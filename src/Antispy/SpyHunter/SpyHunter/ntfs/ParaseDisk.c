/* 
 * Copyright (c) [2010-2019] zhenfei.mzf@gmail.com rights reserved.
 * 
 * AntiSpy is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *     http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
*/
// ParaseDisk.cpp : Defines the entry point for the console application.
//
//#include "stdafx.h"
#include <sys/stat.h>

#include "attrib.h"
#include "inode.h"
#include "volume.h"
#include "dir.h"
#include "unistr.h"
#include "layout.h"
#include "index.h"
#include "utils.h"
#include "version.h"
#include "ntfstime.h"
#include "misc.h"
#include "types.h"
#include "../ParseNTFS.h"

//////////////////////////////////////////////////////////////////////////

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef S_IFBLK
#define S_IFBLK 0x12345678
#endif

#ifndef EOVERFLOW
#define EOVERFLOW 333333
#endif


/*SCSI_DRIVER g_scsi_driver = { FALSE, FALSE };*/


typedef enum {
	FSTYPE_NONE,
	FSTYPE_UNKNOWN,
	FSTYPE_FUSE,
	FSTYPE_FUSEBLK
} fuse_fstype;

typedef struct {
	DWORD filler;
	void *buf;
	ntfs_volume *vol;
} ntfs_fuse_fill_context_t;

typedef enum {
	NF_STREAMS_INTERFACE_NONE,		/* No access to named data streams. */
	NF_STREAMS_INTERFACE_XATTR,		/* Map named data streams to xattrs. */
	NF_STREAMS_INTERFACE_WINDOWS,	/* "file:stream" interface. */
} ntfs_fuse_streams_interface;

typedef struct {
	ntfs_volume *vol;
	int state;
	long free_clusters;
	long free_mft;
	unsigned int uid;
	unsigned int gid;
	unsigned int fmask;
	unsigned int dmask;
	ntfs_fuse_streams_interface streams;
	BOOL ro;				// read only
	BOOL show_sys_files;
	BOOL silent;
	BOOL force;
	BOOL debug;
	BOOL noatime;
	BOOL no_detach;
} ntfs_fuse_context_t;

typedef enum {
	NF_FreeClustersOutdate	= (1 << 0),  /* Information about amount of
										 free clusters is outdated. */
										 NF_FreeMFTOutdate	= (1 << 1),  /* Information about amount of
																		 free MFT records is outdated. */
} ntfs_fuse_state_bits;


static ntfs_fuse_context_t *ctx;
static u32 ntfs_sequence;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


extern int g_current_drive_id ; // 当前正在操作的文件的分区号


VOID 
GetPartitionNumber (
	IN char* szFilePath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/23 [23:3:2009 - 21:47]

Routine Description:
  得到当前文件所在的系统分区号, eg: "c:\1.txt" 对应的分区号为0 ,往后依次递增  
    
Arguments:
  szFilePath - 要操作的文件全路径

--*/
{
	char *ptr = NULL, *Newpath = NULL;


	if ( NULL == szFilePath ) {
		return ;
	}

	Newpath = strdup(szFilePath);
	
	ptr = strstr( Newpath, ":" );
	if ( NULL == ptr ) {
		return ;
	}
	
	*ptr = '\0' ;

	g_current_drive_id = toupper( *(char*)(--ptr) ) - 'C';

	free(Newpath);
	return ;
}



static   void ntfs_fuse_mark_free_space_outdated(void)
{
	/* Mark information about free MFT record and clusters outdated. */
	ctx->state |= (NF_FreeClustersOutdate | NF_FreeMFTOutdate);
}



/**
* ntfs_fuse_is_named_data_stream - check path to be to named data stream
* @path:	path to check
*
* Returns 1 if path is to named data stream or 0 otherwise.
*/
static int 
ntfs_fuse_is_named_data_stream(
	IN const char *path
	)
{
	if ( strchr(path, ':') && ctx->streams == NF_STREAMS_INTERFACE_WINDOWS )
		return 1;

	return 0;
}

static long ntfs_fuse_get_nr_free_mft_records(ntfs_volume *vol, s64 numof_inode)
{
	u8 *buf;
	long nr_free = 0;
	s64 br, total = 0;

	if (!(ctx->state & NF_FreeMFTOutdate))
		return ctx->free_mft;
	buf = (unsigned char *)ntfs_malloc(vol->cluster_size);
	if (!buf)
		return -errno;
	while (1) {
		int i, j;

		br = ntfs_attr_pread(vol->mftbmp_na, total,
			vol->cluster_size, buf);
		if (br <= 0)
			break;
		total += br;
		for (i = 0; i < br; i++)
			for (j = 0; j < 8; j++) {

				if (--numof_inode < 0)
					break;

				if (!((buf[i] >> j) & 1))
					nr_free++;
			}
	}
	free(buf);
	if (!total || br < 0)
		return -errno;
	ctx->free_mft = nr_free;
	ctx->state &= ~(NF_FreeMFTOutdate);
	return nr_free;
}



static long ntfs_fuse_get_nr_free_clusters(ntfs_volume *vol)
{
	u8 *buf;
	long nr_free = 0;
	s64 br, total = 0;

	if (!(ctx->state & NF_FreeClustersOutdate))
		return ctx->free_clusters;
	buf = (unsigned char *)ntfs_malloc(vol->cluster_size);
	if (!buf)
		return -errno;
	while (1) {
		int i, j;

		br = ntfs_attr_pread(vol->lcnbmp_na, total,
			vol->cluster_size, buf);
		if (br <= 0)
			break;
		total += br;
		for (i = 0; i < br; i++)
			for (j = 0; j < 8; j++)
				if (!((buf[i] >> j) & 1))
					nr_free++;
	}
	free(buf);
	if (!total || br < 0)
		return -errno;
	ctx->free_clusters = nr_free;
	ctx->state &= ~(NF_FreeClustersOutdate);
	return nr_free;
}



char* 
ntfs_fuse_SplitStream(
	IN char* stream_name_mbs,
	OUT char** outStreamName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/06 [6:3:2009 - 12:39]

Routine Description:
  分割出文件名&文件流名. 无论如何,调用者需要释放申请的内存 free();

Arguments:
  stream_name_mbs - 要分割的全路径
  outStreamName - 保存文件流名

Return Value:
  返回分割后的文件路径

--*/
{
	int nLength = 0 ;
	char* pdest = NULL ;
	char* szTmp = (char*)malloc( 0x1000 );
	char* szTmpStream = (char*)malloc( 0x1000 );

	if ( NULL == stream_name_mbs || 0 == *stream_name_mbs || NULL == szTmp ) { return NULL; }
	memset( (void*)szTmp, 0, 0x1000 );
	memset( (void*)szTmpStream, 0, 0x1000 );
	*outStreamName = szTmpStream;

	pdest = strrchr( stream_name_mbs, ':' );

	if ( (NULL == pdest) || (*(++pdest) == '\\') ) { 
		strcpy( szTmp, stream_name_mbs );
		return szTmp;

	} else {
		// 表明冒号后面的是流文件,截取之
		strcpy( szTmpStream, pdest );

		nLength = (int)(pdest - stream_name_mbs - 1); 
		if ( 0 == nLength ) { return szTmp; }

		strncpy( szTmp, stream_name_mbs, nLength );
		return szTmp ;
	}

	return NULL ;
}


static int 
ntfs_fuse_parse_path(
    IN const char *org_path, 
    OUT char **path,
	OUT ntfschar **stream_name
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/06 [6:3:2009 - 9:10]

Routine Description:
  分割字符串为 "文件路径" & "文件流".
  此函数会为@*path & @*stream分配内存,调用者需要释放之

Arguments:
  org_path - 要分割的文件全路径
  path - pointer to buffer in which parsed path saved
  stream_name -	pointer to buffer where stream name in unicode saved

Return Value:
  <0	Error occurred, return -errno;
  ==0	    No stream name, @*stream is not allocated and set to AT_UNNAMED.
  >0	Stream name length in unicode characters.

--*/
{
	char *stream_name_mbs;
	int res;
	// 
	// 	stream_name_mbs = strdup(org_path);
	// 	if ( !stream_name_mbs ){ return -errno; }

	stream_name_mbs = (char*) org_path;

	if ( ctx->streams == NF_STREAMS_INTERFACE_WINDOWS ) 
	{
		//	调用者需要释放此子函数分配的2处内存 @path 和 @stream_name_mbs
		*path = ntfs_fuse_SplitStream( stream_name_mbs, &stream_name_mbs );
		if (stream_name_mbs) 
		{
			*stream_name = NULL;
			res = ntfs_mbstoucs(stream_name_mbs, stream_name, 0);
			if (res < 0) { return -errno; }

			return res;
		}

	} else {

		// 反正caller要释放内存,所以这里需要补上
		char* szTmp = (char*)malloc( 0x1000 );

		if ( NULL == szTmp  ) { 
			*path = stream_name_mbs; 
		} else {
			memset( (void*)szTmp, 0, 0x1000 );
			strcpy( szTmp, stream_name_mbs  );
			*path = szTmp;
		}
	}

	*stream_name = AT_UNNAMED;
	return 0;
}



static int ntfs_fuse_readlink(const char *org_path, char *buf, size_t buf_size)
{
	char *path;
	ntfschar *stream_name;
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	INTX_FILE *intx_file = NULL;
	int stream_name_len, res = 0;

	/* Get inode. */
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;
	if (stream_name_len > 0) {
		res = -EINVAL;
		goto exit;
	}
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	/* Sanity checks. */
	if (!(ni->flags & FILE_ATTR_SYSTEM)) {
		res = -EINVAL;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		res = -errno;
		goto exit;
	}
	if (na->data_size <= sizeof(INTX_FILE_TYPES)) {
		res = -EINVAL;
		goto exit;
	}
	if (na->data_size > sizeof(INTX_FILE_TYPES) +
		sizeof(ntfschar) * MAX_PATH) {
			res = -ENAMETOOLONG;
			goto exit;
	}
	/* Receive file content. */
	intx_file = (INTX_FILE *)ntfs_malloc(na->data_size);
	if (!intx_file) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_pread(na, 0, na->data_size, intx_file) != na->data_size) {
		res = -errno;
		goto exit;
	}
	/* Sanity check. */
	if (intx_file->magic != INTX_SYMBOLIC_LINK) {
		res = -EINVAL;
		goto exit;
	}
	/* Convert link from unicode to local encoding. */
	if (ntfs_ucstombs(intx_file->target, (na->data_size -
		offsetof(INTX_FILE, target)) / sizeof(ntfschar),
		&buf, buf_size) < 0) {
			res = -errno;
			goto exit;
	}
exit:
	if (intx_file)
		free(intx_file);
	if (na)
		ntfs_attr_close(na);
	if (ni)
		ntfs_inode_close(ni);
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}


char* ws2ms(LPWSTR szSrc, int len)
{
	char*strDst = NULL;
	char*pBuff=NULL;
	int nLen = 0;
	WCHAR * wsBuf = NULL;

	if ( szSrc==NULL || !len ){
		return strDst;
	}
	
	wsBuf = (WCHAR*)malloc((len + 1) * sizeof(WCHAR));
	if (!wsBuf)
	{
		return strDst;
	}
	memset(wsBuf, 0, (len + 1) * sizeof(WCHAR));
	memcpy(wsBuf, szSrc, len * sizeof(WCHAR));

	nLen = WideCharToMultiByte(CP_ACP,NULL,wsBuf,-1,NULL,0,NULL,FALSE);
	if ( nLen>0 ){
		pBuff=(char*)malloc(nLen+1);
		if ( pBuff ){
			WideCharToMultiByte(CP_ACP,NULL,wsBuf,-1,pBuff,nLen,NULL,FALSE);
			pBuff[nLen]=0;
			strDst = pBuff;
		}
	}
	free(wsBuf);
	return strDst;
}

static int 
ntfs_fuse_filler(
	ntfs_fuse_fill_context_t *fill_ctx,
	const ntfschar *name, 
	const int name_len, 
	const int name_type,
	const s64 pos , 
	const MFT_REF mref,
	const unsigned dt_type
	)
{
	char *filename = NULL;

	if ( name_type == FILE_NAME_DOS || FILE_NAME_POSIX == name_type )
		return 0;

	if ( ntfs_ucstombs(name, name_len, &filename, 0 ) < 0 ) {
		if ((filename = ws2ms((LPWSTR)name, name_len)) == NULL)
		{
			return 0;
		}
	}

	if ( ntfs_fuse_is_named_data_stream( filename ) )
	{
		free( (void*)filename );
		return 0;
	}

	if (MREF(mref) == FILE_root 
		|| MREF(mref) >= FILE_first_user 
		|| ctx->show_sys_files
	   ) 
	{
		char szBBBB[512] = "" ;
		BOOL bIsDiretory = FALSE ;
		
		// 显示当前的文件信息
		
		bIsDiretory = IsDirectory( fill_ctx->vol, mref );
		
		//
		// 打印当前名字
		//
		
		RtlZeroMemory( szBBBB, 512 );
		sprintf( 
			szBBBB,
			"  %s \t %s \n", 
			filename,
			bIsDiretory == TRUE ? "[Diretory]" : ""
			);
		OutputDebugStringA( szBBBB );
		printf( szBBBB );
	}

	free( (void*)filename );
	return 0;
}



static int 
ntfs_enumerate_dir_ex(
	IN const char *path
	)
{
	ntfs_fuse_fill_context_t fill_ctx;
	ntfs_volume *vol;
	ntfs_inode *ni;
	s64 pos = 0;
	int err = 0;

	vol = ctx->vol;
	fill_ctx.vol = vol ;

	// 得到当前文件路径的对应的Inode结构
	ni = ntfs_pathname_to_inode( vol, NULL, path );
	if (!ni) { return -errno; }

	// 关键一句调用
	if ( ntfs_readdir( ni, &pos, &fill_ctx, (ntfs_filldir_t)ntfs_fuse_filler) )
		err = -errno;

	// 收尾工作
	ntfs_inode_close(ni);
	return err;
}




static int 
ntfs_fuse_open(
	IN const char *org_path
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/06 [6:3:2009 - 12:57]

Routine Description:
  测试能否打开对应的文件    

Arguments:
  org_path - 指定要打开的文件全路径

Return Value:
  0 -- 成功; 其他失败

--*/
{
	ntfs_volume *vol;
	ntfs_inode *ni;
	ntfs_attr *na;
	int res = 0;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;

	// 得到文件名 & 文件流名
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) { return stream_name_len; }

	// 获得该文件的分区号
	GetPartitionNumber( path );

	// 得到文件名对应的节点inode
	vol = ctx->vol;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (ni) 
	{
		// 获取该文件对应MFT的80H属性
		na = ntfs_attr_open( ni, AT_DATA, stream_name, stream_name_len );
		if ( na ) 
		{
			// 不支持加密属性
			if ( NAttrEncrypted(na) ) { res = -EACCES; }
			ntfs_attr_close(na);
		} else {
			res = -errno;
		}

		ntfs_inode_close(ni);
	} else {
		res = -errno;
	}

	free(path);
	if (stream_name_len) { free(stream_name); }

	return res;
}



static int 
ntfs_fuse_read(
	IN const char *org_path, 
	OUT char *buf,
	IN size_t size,
	IN off_t offset
	)
/*++

Routine Description:
  从指定文件的偏移@offset处读出大小@size的内容到@buf中  

Arguments:
  org_path - 要读取的文件的全路径
  buf - 保存读出的内容
  size - 要读取的内容长度
  offset - 读取的偏移

Return Value:
  0 -- 成功; 其他失败

--*/
{
	ntfs_volume *vol;
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len, res, total = 0;

	// 得到文件名 & 文件流名
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) { return stream_name_len; }

	// 获得该文件的分区号
	GetPartitionNumber( path );

	// 得到文件名对应的节点inode
	vol = ctx->vol;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}

	// 获取该文件对应MFT的80H属性
	na = ntfs_attr_open( ni, AT_DATA, stream_name, stream_name_len );
	if (!na) {
		res = -errno;
		goto exit;
	}

	// 检测要读取大小的合法性.若内容超出了该属性的范围.则限制一下
	if (offset + size > na->data_size) { size = na->data_size - offset; }


	g_scsi_driver.bUse_SCSI = TRUE ;

	// 开始读取指定偏移处指定大小的内容到Buffer中去
	while (size) 
	{
		res = ntfs_attr_pread( na, offset, size, buf );
		if (res < (s64)size) { ntfs_log_error("ntfs_attr_pread returned less bytes than requested.\n"); }
		if (res <= 0) {
			res = -errno;
			goto exit;
		}
		if ( (res > (int)size) && (res < (int)size + 512) ) // 比如要读0x3e8,那么按照512字节对齐读出的话,就是0x400了.很正常. 表明成功读取
		{ 
			ntfs_log_debug("-- Read OK! -- \n");
			break ;
		}

		size -= res; // res是实际读出的内容大小
		offset += res;
		total += res;

		if ( size < 0 || (size & 0xff000000) ) { break ; }
	}
	res = total; // 保存实际读出的内容大小

	// 收尾工作
exit:
	g_scsi_driver.bUse_SCSI = FALSE ;

	if (na) { ntfs_attr_close(na); }
	if (ni && ntfs_inode_close(ni)) { ntfs_log_perror("Failed to close inode"); }
	if(path) { free(path); }
	if (stream_name_len) { free(stream_name); }

	return res;
}



static int 
ntfs_fuse_write(
	IN const char *org_path, 
	IN const char *buf, 
	IN size_t size,
	IN off_t offset
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/06 [6:3:2009 - 13:06]

Routine Description:
  从指定文件的偏移@offset处写入大小@size的@buf内容  

Arguments:
  org_path - 要写入的文件的全路径
  buf - 保存写入的内容
  size - 要写入的内容长度
  offset - 写入的偏移

Return Value:
  0 -- 成功; 其他失败

--*/
{
	ntfs_volume *vol;
	ntfs_inode *ni = NULL;
	ntfs_attr *na = NULL;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len, res, total = 0;

	// 得到文件名 & 文件流名
	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) {
		res = stream_name_len;
		goto out;
	}

	// 获得该文件的分区号
	GetPartitionNumber( path );

	// 得到文件名对应的节点inode
	vol = ctx->vol;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}

	// 获取该文件对应MFT的80H属性
	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}

	// 开始写入...
	while (size) 
	{
		res = ntfs_attr_pwrite( na, offset, size, buf );
		if (res < (s64)size)
			ntfs_log_error("ntfs_attr_pwrite returned less bytes than requested.\n");
		if (res <= 0) {
			res = -errno;
			goto exit;
		}

		size -= res;
		offset += res;
		total += res;
	}
	res = total;

	// 收尾工作
exit:
	ntfs_fuse_mark_free_space_outdated();
	if (na) { ntfs_attr_close(na); }
	if (ni && ntfs_inode_close(ni)) { ntfs_log_perror("Failed to close inode"); }
	free(path);
	if (stream_name_len) { free(stream_name); }
out:	
	return res;
}



static int 
ntfs_fuse_truncate(
	IN const char *org_path, 
	IN off_t size
	)
{
	ntfs_volume *vol;
	ntfs_inode *ni = NULL;
	ntfs_attr *na;
	int res;
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) { return stream_name_len; }

	// 获得该文件的分区号
	GetPartitionNumber( path );

	vol = ctx->vol;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}

	na = ntfs_attr_open(ni, AT_DATA, stream_name, stream_name_len);
	if (!na) {
		res = -errno;
		goto exit;
	}

	res = ntfs_attr_truncate( na, size );
	if (res) { res = -1; }

	ntfs_fuse_mark_free_space_outdated();
	ntfs_attr_close(na);

exit:
	if (ni && ntfs_inode_close(ni)) { ntfs_log_perror("Failed to close inode"); }
	free(path);
	if (stream_name_len) { free(stream_name); }

	return res;
}



static int 
ntfs_fuse_chmod(
	const char *path
	)
{
	if ( ntfs_fuse_is_named_data_stream(path) ) { return -EINVAL; }

	if (ctx->silent) { return 0; }

	return -1;
}

/*
static int ntfs_fuse_chown(const char *path, uid_t uid, gid_t gid)
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL;
	if (ctx->silent)
		return 0;
	if (uid == ctx->uid && gid == ctx->gid)
		return 0;
	return -EOPNOTSUPP;
}
*/



static int 
ntfs_fuse_create(
	IN const char *org_path, 
	IN dev_t type, 
	IN dev_t dev,
	IN const char *target
	)
{
	char *name;
	ntfschar *uname = NULL, *utarget = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len, utarget_len;

	path = strdup(org_path);
	if (!path)
		return -errno;
	/* Generate unicode filename. */
	name = strrchr(path, '\\');
	name++;
	uname_len = ntfs_mbstoucs(name, &uname, 0);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	*name = 0;
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}
	/* Create object specified in @type. */
	switch (type) {
case S_IFCHR:
case S_IFBLK:
	ni = ntfs_create_device(dir_ni, uname, (u8)uname_len, type,dev);
	break;
case S_IFLNK:
	utarget_len = ntfs_mbstoucs(target, &utarget, 0);
	if (utarget_len < 0) {
		res = -errno;
		goto exit;
	}
	ni = ntfs_create_symlink(dir_ni, uname, (u8)uname_len, utarget, (u8)utarget_len);
	break;
default:
	ni = ntfs_create(dir_ni, uname, (u8)uname_len, type);
	break;
	}
	if (ni)
		ntfs_inode_close(ni);
	else
		res = -errno;
exit:
	free(uname);
	if (dir_ni)
		ntfs_inode_close(dir_ni);
	if (utarget)
		free(utarget);
	free(path);
	return res;
}

static int ntfs_fuse_create_stream(const char *path,
								   ntfschar *stream_name, const int stream_name_len)
{
	ntfs_inode *ni;
	int res = 0;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		if (res == -ENOENT) {
			/*
			* If such file does not exist, create it and try once
			* again to add stream to it.
			*/
			res = ntfs_fuse_create(path, S_IFREG, 0, NULL);
			if (!res)
				return ntfs_fuse_create_stream(path,
				stream_name, stream_name_len);
			else
				res = -errno;
		}
		return res;
	}
	if (ntfs_attr_add(ni, AT_DATA, stream_name, (u8)stream_name_len, NULL, 0))
		res = -errno;
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}



static int 
ntfs_fuse_mknod(
	IN const char *org_path, 
	int mode, 
	dev_t dev
	)
{
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	int res = 0;

	stream_name_len = ntfs_fuse_parse_path(org_path, &path, &stream_name);
	if (stream_name_len < 0) { return stream_name_len; }
//
// unix下的宏:
// _ISLNK(st_mode):是否是一个连接.S_ISREG是否是一个常规文件.S_ISDIR是否是一个目录
// S_ISCHR是否是一个字符设备.S_ISBLK是否是一个块设备S_ISFIFO是否 是一个FIFO文件.
// S_ISSOCK是否是一个SOCKET文件.
//
	if (stream_name_len && !S_ISREG(mode)) {
		res = -EINVAL;
		goto exit;
	}
	if (!stream_name_len)
		res = ntfs_fuse_create(path, S_IFREG, dev, NULL);
	else
		res = ntfs_fuse_create_stream(path, stream_name,
		stream_name_len);
	ntfs_fuse_mark_free_space_outdated();
exit:
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}

static int ntfs_fuse_symlink(const char *to, const char *from)
{
	if (ntfs_fuse_is_named_data_stream(from))
		return -EINVAL; /* n/a for named data streams. */
	ntfs_fuse_mark_free_space_outdated();
	return ntfs_fuse_create(from, S_IFLNK, 0, to);
}



static int ntfs_fuse_link(const char *old_path, const char *new_path)
{
	char *name;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len;

	if (ntfs_fuse_is_named_data_stream(old_path))
		return -EINVAL; /* n/a for named data streams. */
	if (ntfs_fuse_is_named_data_stream(new_path))
		return -EINVAL; /* n/a for named data streams. */
	path = strdup(new_path);
	if (!path)
		return -errno;
	/* Open file for which create hard link. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, old_path);
	if (!ni) {
		res = -errno;
		goto exit;
	}
	/* Generate unicode filename. */
	name = strrchr(path, '\\');
	name++;
	uname_len = ntfs_mbstoucs(name, &uname, 0);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}
	/* Open parent directory. */
	*name = 0;
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}
	ntfs_fuse_mark_free_space_outdated();
	/* Create hard link. */
	if (ntfs_link(ni, dir_ni, uname, (u8)uname_len))
		res = -errno;
exit:
	if (ni)
		ntfs_inode_close(ni);
	free(uname);
	if (dir_ni)
		ntfs_inode_close(dir_ni);
	free(path);
	return res;
}




VOID
ntfs_fuse_Enum(
	IN const char *org_path
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/05 [5:3:2009 - 21:45]

Routine Description:
  枚举当前目录的所有子项

Arguments:
  org_path - 枚举目录的全路径

Return Value:

--*/
{
	ntfs_inode *ni  = NULL;
	char *path;

	// 复制路径名
	path = strdup(org_path);
	if (!path) { return ; }

	// 获得该文件的分区号
	GetPartitionNumber( path );

	// 打开这个文件相关的信息,获取对应的Inode结构体
	ni = ntfs_pathname_to_inode( ctx->vol, NULL, path );
	if (!ni) {
		goto exit ;
	}

	OutputDebugStringA( path );
	printf( path );
	printf( "\n" );

	//
	// 关键一句
	//

//	g_scsi_driver.bUse_SCSI = TRUE ;
	ntfs_inode_Enum_name( ni );
//	g_scsi_driver.bUse_SCSI = FALSE ;

	// 清理工作
exit:
	if (ni) { ntfs_inode_close(ni); }
	free(path);

	return ;
}



static int 
ntfs_fuse_rm(
	IN const char *org_path
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/05 [5:3:2009 - 21:45]

Routine Description:
  删除指定文件

Arguments:
  org_path - 指定要删除文件的全路径

Return Value:

--*/
{
	char *name;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path;
	int res = 0, uname_len;

	// 复制路径名
	path = strdup(org_path);
	if (!path) { return -errno; }

	// 打开这个文件相关的信息,获取对应的Inode结构体
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) {
		res = -errno;
		goto exit;
	}

	// 将路径转换成Unicode形式
	name = strrchr(path, PATH_SEP);
	name++;
	uname_len = ntfs_mbstoucs(name, &uname, 0);
	if (uname_len < 0) {
		res = -errno;
		goto exit;
	}

	// 得到其父目录
	*name = 0; // 截断完整的字符串,只保留其父目录的完整路径
	dir_ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!dir_ni) {
		res = -errno;
		goto exit;
	}

	//
	// 关键一句调用
	//
	if ( ntfs_delete( ni, dir_ni, uname, (u8)uname_len ) ) { res = -errno; }
	ni = NULL;

	// 清理工作
exit:
	if (ni) { ntfs_inode_close(ni); }
	free(uname);
	if (dir_ni) { ntfs_inode_close(dir_ni); }
	free(path);

	return res;
}



static int 
ntfs_fuse_rm_stream(
	IN const char *path, 
	IN ntfschar *stream_name,
	IN const int stream_name_len
	)
{
	ntfs_inode *ni;
	int res = 0;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni) { return -errno; }

	if (ntfs_attr_remove(ni, AT_DATA, stream_name, stream_name_len)) { res = -errno; }

	if (ntfs_inode_close(ni)) { ntfs_log_perror("Failed to close inode"); }

	return res;
}



static int 
ntfs_fuse_unlink(
	IN const char *org_path
	)
{
	char *path = NULL;
	ntfschar *stream_name;
	int stream_name_len;
	int res = 0;

	stream_name_len = ntfs_fuse_parse_path( org_path, &path, &stream_name );
	if (stream_name_len < 0) { return stream_name_len; }

	if ( !stream_name_len ) {
		res = ntfs_fuse_rm( path ); // 删除该指定文件
	} else {
		res = ntfs_fuse_rm_stream(path, stream_name, stream_name_len);
	}

	ntfs_fuse_mark_free_space_outdated();
	free(path);
	if (stream_name_len)
		free(stream_name);
	return res;
}


static int 
ntfs_fuse_safe_rename(
	const char *old_path, 
	const char *new_path, 
	const char *tmp
	)
{
	int ret;

	ntfs_log_trace("Entering");

	ret = ntfs_fuse_link(new_path, tmp);
	if (ret)
		return ret;

	ret = ntfs_fuse_unlink(new_path);
	if (!ret) {

		ret = ntfs_fuse_link(old_path, new_path);
		if (ret)
			goto restore;

		ret = ntfs_fuse_unlink(old_path);
		if (ret) {
			if (ntfs_fuse_unlink(new_path))
				goto err;
			goto restore;
		}
	}

	goto cleanup;

restore:
	if (ntfs_fuse_link(tmp, new_path)) {
err:
		ntfs_log_perror("Rename failed. Existing file '%s' was renamed to '%s'", new_path, tmp);
	} else {
cleanup:
		ntfs_fuse_unlink(tmp);
	}
	return 	ret;
}



static int ntfs_fuse_rename_existing_dest(const char *old_path, const char *new_path)
{
	int ret, len;
	char *tmp;
	const char *ext = ".ntfs-3g-";

	ntfs_log_trace("Entering");

	len = strlen(new_path) + strlen(ext) + 10 + 1; /* wc(str(2^32)) + \0 */
	tmp = (char*)ntfs_malloc(len);
	if (!tmp)
		return -errno;

	ret = _snprintf(tmp, len, "%s%s%010d", new_path, ext, ++ntfs_sequence);
	if (ret != len - 1) {
		ntfs_log_error("snprintf failed: %d != %d\n", ret, len - 1);
		ret = -EOVERFLOW;
	} else
		ret = ntfs_fuse_safe_rename(old_path, new_path, tmp);

	free(tmp);
	return 	ret;
}



static int ntfs_fuse_rename(const char *old_path, const char *new_path)
{
	int ret, stream_name_len;
	char *path = NULL;
	ntfschar *stream_name;
	ntfs_inode *ni;

	ntfs_log_debug("rename: old: '%s'  new: '%s'\n", old_path, new_path);

	stream_name_len = ntfs_fuse_parse_path(new_path, &path, &stream_name);
	if (stream_name_len < 0)
		return stream_name_len;

	// 获得该文件的分区号
	GetPartitionNumber( path );

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (ni) {
		ret = ntfs_check_empty_dir(ni);
		if (ret < 0) {
			ret = -errno;
			ntfs_inode_close(ni);
			goto out;
		}

		ntfs_inode_close(ni);

		ret = ntfs_fuse_rename_existing_dest(old_path, new_path);
		goto out;
	}

	ret = ntfs_fuse_link(old_path, new_path);
	if (ret)
		goto out;

	ret = ntfs_fuse_unlink(old_path);
	if (ret)
		ntfs_fuse_unlink(new_path);
out:
	free(path);
	if (stream_name_len)
		free(stream_name);
	return ret;
}



static int 
ntfs_fuse_mkdir(
	IN const char *path
	)
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ntfs_fuse_mark_free_space_outdated();

	// 获得该文件的分区号
	GetPartitionNumber( (char*)path );

	return ntfs_fuse_create( path, S_IFDIR, 0, NULL );
}



static int 
ntfs_fuse_rmdir(
	IN const char *path
	)
{
	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ntfs_fuse_mark_free_space_outdated();
	return ntfs_fuse_rm(path);
}


static int ntfs_fuse_utime(const char *path, VOID *buf)
{
	ntfs_inode *ni;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL; /* n/a for named data streams. */
	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

// 	if (buf) {
// 		ni->last_access_time = buf->actime;
// 		ni->last_data_change_time = buf->modtime;
// 		ni->last_mft_change_time = buf->modtime;
// 	} else 
	{
		time_t now;

		now = time(NULL);
		ni->last_access_time = now;
		ni->last_data_change_time = now;
		ni->last_mft_change_time = now;
	}
	NInoFileNameSetDirty(ni);
	NInoSetDirty(ni);
	if (ntfs_inode_close(ni)) { ntfs_log_perror("Failed to close inode"); }

	return 0;
}



static int 
ntfs_fuse_bmap(
	IN const char *path, 
	IN size_t blocksize, 
	IN u64 *idx
	)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	LCN lcn;
	int ret, cl_per_bl = ctx->vol->cluster_size / blocksize;

	if (blocksize > ctx->vol->cluster_size)
		return -EINVAL;

	if (ntfs_fuse_is_named_data_stream(path))
		return -EINVAL;

	ni = ntfs_pathname_to_inode(ctx->vol, NULL, path);
	if (!ni)
		return -errno;

	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ret = -errno;
		goto close_inode;
	}

	if (NAttrCompressed(na) || NAttrEncrypted(na) || !NAttrNonResident(na)){
		ret = -EINVAL;
		goto close_attr;
	}

	if (ntfs_attr_map_whole_runlist(na)) {
		ret = -errno;
		goto close_attr;
	}

	lcn = ntfs_rl_vcn_to_lcn(na->rl, *idx / cl_per_bl);
	*idx = (lcn > 0) ? lcn * cl_per_bl + *idx % cl_per_bl : 0;

	ret = 0;

close_attr:
	ntfs_attr_close(na);
close_inode:
	if (ntfs_inode_close(ni))
		ntfs_log_perror("bmap: failed to close inode");
	return ret;
}

#ifdef HAVE_SETXATTR

static const char nf_ns_xattr_preffix[] = "user.";
static const int nf_ns_xattr_preffix_len = 5;

static int ntfs_fuse_listxattr(const char *path, char *list, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_volume *vol;
	ntfs_inode *ni;
	char *to = list;
	int ret = 0;

	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	vol = ctx->vol;
	if (!vol)
		return -ENODEV;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni)
		return -errno;
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) {
		ret = -errno;
		ntfs_inode_close(ni);
		goto exit;
	}
	while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE,
		0, NULL, 0, actx)) {
			char *tmp_name = NULL;
			int tmp_name_len;

			if (!actx->attr->name_length)
				continue;
			tmp_name_len = ntfs_ucstombs((ntfschar *)((u8*)actx->attr +
				le16_to_cpu(actx->attr->name_offset)),
				actx->attr->name_length, &tmp_name, 0);
			if (tmp_name_len < 0) {
				ret = -errno;
				goto exit;
			}
			ret += tmp_name_len + nf_ns_xattr_preffix_len + 1;
			if (size) {
				if ((size_t)ret <= size) {
					strcpy(to, nf_ns_xattr_preffix);
					to += nf_ns_xattr_preffix_len;
					strncpy(to, tmp_name, tmp_name_len);
					to += tmp_name_len;
					*to = 0;
					to++;
				} else {
					free(tmp_name);
					ret = -ERANGE;
					goto exit;
				}
			}
			free(tmp_name);
	}
	if (errno != ENOENT)
		ret = -errno;
exit:
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	ntfs_inode_close(ni);
	ntfs_log_debug("return %d\n", ret);
	return ret;
}



static int ntfs_fuse_getxattr_windows(const char *path, const char *name,
									  char *value, size_t size)
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_volume *vol;
	ntfs_inode *ni;
	char *to = value;
	int ret = 0;

	if (strcmp(name, "ntfs.streams.list"))
		return -EOPNOTSUPP;
	vol = ctx->vol;
	if (!vol)
		return -ENODEV;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni)
		return -errno;
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) {
		ret = -errno;
		ntfs_inode_close(ni);
		goto exit;
	}
	while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE,
		0, NULL, 0, actx)) {
			char *tmp_name = NULL;
			int tmp_name_len;

			if (!actx->attr->name_length)
				continue;
			tmp_name_len = ntfs_ucstombs((ntfschar *)((u8*)actx->attr +
				le16_to_cpu(actx->attr->name_offset)),
				actx->attr->name_length, &tmp_name, 0);
			if (tmp_name_len < 0) {
				ret = -errno;
				goto exit;
			}
			if (ret)
				ret++; /* For space delimiter. */
			ret += tmp_name_len;
			if (size) {
				if ((size_t)ret <= size) {
					/* Don't add space to the beginning of line. */
					if (to != value) {
						*to = ' ';
						to++;
					}
					strncpy(to, tmp_name, tmp_name_len);
					to += tmp_name_len;
				} else {
					free(tmp_name);
					ret = -ERANGE;
					goto exit;
				}
			}
			free(tmp_name);
	}
	if (errno != ENOENT)
		ret = -errno;
exit:
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	ntfs_inode_close(ni);
	return ret;
}



static int ntfs_fuse_getxattr(const char *path, const char *name,
							  char *value, size_t size)
{
	ntfs_volume *vol;
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res, lename_len;

	if (ctx->streams == NF_STREAMS_INTERFACE_WINDOWS)
		return ntfs_fuse_getxattr_windows(path, name, value, size);
	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	if (strncmp(name, nf_ns_xattr_preffix, nf_ns_xattr_preffix_len) ||
		strlen(name) == (size_t)nf_ns_xattr_preffix_len)
		return -5;
	vol = ctx->vol;
	if (!vol)
		return -ENODEV;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni)
		return -errno;
	lename_len = ntfs_mbstoucs(name + nf_ns_xattr_preffix_len, &lename, 0);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (!na) {
		res = -5;
		goto exit;
	}
	if (size) {
		if (size >= na->data_size) {
			res = ntfs_attr_pread(na, 0, na->data_size, value);
			if (res != na->data_size)
				res = -errno;
		} else
			res = -ERANGE;
	} else
		res = na->data_size;
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}



static int ntfs_fuse_setxattr(const char *path, const char *name,
							  const char *value, size_t size, int flags)
{
	ntfs_volume *vol;
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	ntfschar *lename = NULL;
	int res, lename_len;

	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	if (strncmp(name, nf_ns_xattr_preffix, nf_ns_xattr_preffix_len) ||
		strlen(name) == (size_t)nf_ns_xattr_preffix_len)
		return -EACCES;
	vol = ctx->vol;
	if (!vol)
		return -ENODEV;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni)
		return -errno;
	lename_len = ntfs_mbstoucs(name + nf_ns_xattr_preffix_len, &lename, 0);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
	if (na && flags == XATTR_CREATE) {
		res = -EEXIST;
		goto exit;
	}
	ntfs_fuse_mark_free_space_outdated();
	if (!na) {
		if (flags == XATTR_REPLACE) {
			res = -5;
			goto exit;
		}
		if (ntfs_attr_add(ni, AT_DATA, lename, lename_len, NULL, 0)) {
			res = -errno;
			goto exit;
		}
		na = ntfs_attr_open(ni, AT_DATA, lename, lename_len);
		if (!na) {
			res = -errno;
			goto exit;
		}
	}
	res = ntfs_attr_pwrite(na, 0, size, value);
	if (res != (s64) size)
		res = -errno;
	else
		res = 0;
exit:
	if (na)
		ntfs_attr_close(na);
	free(lename);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}



static int ntfs_fuse_removexattr(const char *path, const char *name)
{
	ntfs_volume *vol;
	ntfs_inode *ni;
	ntfschar *lename = NULL;
	int res = 0, lename_len;


	if (ctx->streams != NF_STREAMS_INTERFACE_XATTR)
		return -EOPNOTSUPP;
	if (strncmp(name, nf_ns_xattr_preffix, nf_ns_xattr_preffix_len) ||
		strlen(name) == (size_t)nf_ns_xattr_preffix_len)
		return -5;
	vol = ctx->vol;
	if (!vol)
		return -ENODEV;
	ni = ntfs_pathname_to_inode(vol, NULL, path);
	if (!ni)
		return -errno;
	lename_len = ntfs_mbstoucs(name + nf_ns_xattr_preffix_len, &lename, 0);
	if (lename_len == -1) {
		res = -errno;
		goto exit;
	}
	if (ntfs_attr_remove(ni, AT_DATA, lename, lename_len)) {
		if (errno == ENOENT)
			errno = 5;
		res = -errno;
	}

	ntfs_fuse_mark_free_space_outdated();
exit:
	free(lename);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode");
	return res;
}

#endif /* HAVE_SETXATTR */


static void ntfs_fuse_destroy(void)
{
	if (!ctx)
		return;

	if (ctx->vol) 
	{
		if ( ntfs_umount( ctx->vol, FALSE ) ) { ntfs_log_perror("Failed to unmount volume"); }
	}

	free(ctx);
	ctx = NULL;
	
}

static void ntfs_fuse_destroy2()
{
	ntfs_fuse_destroy();
}


// 
// static struct fuse_operations ntfs_fuse_oper = {
// //	.getattr	= ntfs_fuse_getattr,
// 	.readlink	= ntfs_fuse_readlink,
// //	.readdir	= ntfs_enumerate_dir_ex,
// 	.open		= ntfs_fuse_open,
// 	.read		= ntfs_fuse_read,
// 	.write		= ntfs_fuse_write,
// 	.truncate	= ntfs_fuse_truncate,
// //	.statfs		= ntfs_fuse_statfs,
// 	.chmod		= ntfs_fuse_chmod,
// 	.chown		= ntfs_fuse_chown,
// 	.mknod		= ntfs_fuse_mknod,
// 	.symlink	= ntfs_fuse_symlink,
// 	.link		= ntfs_fuse_link,
// 	.unlink		= ntfs_fuse_unlink,
// 	.rename		= ntfs_fuse_rename,
// 	.mkdir		= ntfs_fuse_mkdir,
// 	.rmdir		= ntfs_fuse_rmdir,
// 	.utime		= ntfs_fuse_utime,
// 	.bmap		= ntfs_fuse_bmap,
// 	.destroy        = ntfs_fuse_destroy2,
// #ifdef HAVE_SETXATTR
// 	.getxattr	= ntfs_fuse_getxattr,
// 	.setxattr	= ntfs_fuse_setxattr,
// 	.removexattr	= ntfs_fuse_removexattr,
// 	.listxattr	= ntfs_fuse_listxattr,
// #endif /* HAVE_SETXATTR */
// };



int ntfs_fuse_init(void)
{
	ctx = (ntfs_fuse_context_t *)ntfs_malloc(sizeof(ntfs_fuse_context_t));
	if (!ctx) { return -1; }

	ctx->state = NF_FreeClustersOutdate | NF_FreeMFTOutdate;
	ctx->fmask = 0;
	ctx->dmask = 0;
	ctx->streams = NF_STREAMS_INTERFACE_NONE ;

	return 0;
}



ntfs_volume *
ntfs_open(
	IN const char *device,
	IN int blkdev
	)
{
	unsigned long flags = 0;

	if (!blkdev)
		flags |= MS_EXCLUSIVE;
	if (ctx->ro)
		flags |= MS_RDONLY;
	if (ctx->noatime)
		flags |= MS_NOATIME;

	ctx->vol = utils_mount_volume(device, flags, ctx->force);
	return ctx->vol;
}



#ifndef HAVE_REALPATH
/* If there is no realpath() on the system, provide a dummy one. */
static char *realpath(const char *path, char *resolved_path)
{
	strncpy(resolved_path, path, PATH_MAX);
	resolved_path[PATH_MAX] = '\0';
	return resolved_path;
}
#endif


int enum_file_by_parse_disk(char* szDevice, char* szPath)
{
	int nCounts = 0 ;
	int use_blkdev = 0;
	char* pBuff = NULL;
//	char* szDevice = NULL;

	if (!szPath)
	{
		return 0;
	}
	
	if ( ntfs_fuse_init() ) { return 2; }

	//
	// 打开指定的盘符. eg. c:
	//
//	ntfs_open( "\\\\.\\PHYSICALDRIVE0", use_blkdev );
	ntfs_open( "c", use_blkdev );

	//
	// 创建 文件 & 文件夹
	//

//	ntfs_fuse_mknod( "\\360Safe.Com.txt",S_IFREG, 0 );
//	ntfs_fuse_mkdir( "C:\\360Safe.FuckingYou" ); //
//	ntfs_fuse_mkdir( "C:\\xnj.FuckingYou" );

	//
	// 删除 文件 & 文件夹
	//

//	ntfs_fuse_rmdir( "\\sudami" ); // 
//	ntfs_fuse_rm( "\\Test\\sudami.txt" );

	//
	// 重新命名目录 & 文件
	//

//	ntfs_fuse_rename( "\\haha.txt", "\\qq.txt" ); // 操作后 该目录损坏,变得不可重命名&删除. 但可打开&读写 
//	ntfs_fuse_rename( "\\sudami", "\\aaaaaaa" );  // 操作后 该目录损坏,变得不可重命名&删除

	//
	// 枚举文件
	//

//	ntfs_fuse_Enum( "c:\\" );
	ntfs_enumerate_dir_ex( "C:\\Windows" );
	//
	// 读 & 写 文件 [Success]
	//
	
	// 写
// 	pBuff = (char*)malloc(0x1000);
// 	strcpy( pBuff, "sudami.360Safe.com " );
// 	
// 	ntfs_fuse_write( "\\360Safe.Com.txt", pBuff, 0x1000, 0 );
// 	free( pBuff );

	// 读
// 	pBuff = (char*)malloc( 0x1000 );
// 	memset( pBuff, 0, 1000 );
// 	
// 	ntfs_fuse_read( "D:\\sudami.txt", pBuff, 0x1000, 0 );
// 	printf( "%s", pBuff );
// 	free( pBuff );

	//
	// 收尾工作
	//
	
	ntfs_fuse_destroy();
	
	return 0;
}