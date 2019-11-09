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
/**
 * dir.c - Directory handling code. Originated from the Linux-NTFS project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <stddef.h>

#include <sys/stat.h>

#include "types.h"
#include "debug.h"
#include "attrib.h"
#include "inode.h"
#include "dir.h"
#include "volume.h"
#include "mft.h"
#include "index.h"
#include "ntfstime.h"
#include "lcnalloc.h"
#include "logging.h"
#include "misc.h"
#include "security.h"

#pragma warning( disable: 4244 4018 )

//////////////////////////////////////////////////////////////////////////

#ifndef S_IFBLK
#define S_IFBLK 0x12345678
#endif

#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)   //目录文件
#define S_IFIFO 0x1000 

//////////////////////////////////////////////////////////////////////////


/*
 * The little endian Unicode strings "$I30", "$SII", "$SDH", "$O"
 *  and "$Q" as global constants.
 */
ntfschar NTFS_INDEX_I30[5] = { /*const_cpu_to_le16*/('$'), /*const_cpu_to_le16*/('I'),
		/*const_cpu_to_le16*/('3'), /*const_cpu_to_le16*/('0'),
		/*const_cpu_to_le16*/('\0') };
ntfschar NTFS_INDEX_SII[5] = { const_cpu_to_le16('$'), const_cpu_to_le16('S'),
		const_cpu_to_le16('I'), const_cpu_to_le16('I'),
		const_cpu_to_le16('\0') };
ntfschar NTFS_INDEX_SDH[5] = { const_cpu_to_le16('$'), const_cpu_to_le16('S'),
		const_cpu_to_le16('D'), const_cpu_to_le16('H'),
		const_cpu_to_le16('\0') };
ntfschar NTFS_INDEX_O[3] = { const_cpu_to_le16('$'), const_cpu_to_le16('O'),
		const_cpu_to_le16('\0') };
ntfschar NTFS_INDEX_Q[3] = { const_cpu_to_le16('$'), const_cpu_to_le16('Q'),
		const_cpu_to_le16('\0') };
ntfschar NTFS_INDEX_R[3] = { const_cpu_to_le16('$'), const_cpu_to_le16('R'),
		const_cpu_to_le16('\0') };


//////////////////////////////////////////////////////////////////////////

// void DbgPrint(char* formatstr, ...)
// {
// 	char buf[1024];
// 	va_list argptr;
// 	va_start(argptr, formatstr);
// 	
// 	wvsprintfA(buf,formatstr,argptr);
// 	OutputDebugStringA(buf);
// 
// 	return ;
// }



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+              枚举当前目录的所有文件(夹)                   +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
IsDirectory (
    IN ntfs_volume *vol, 
	IN const u64 mref
//	OUT char* szStreamName
	)
{
	u64 inum ;
	ntfs_inode *ni = NULL ;
	ntfs_attr_search_ctx *ctx = NULL;
	INDEX_ROOT *ir = NULL;
	ntfs_attr *na = NULL;
	BOOL bResult = FALSE ;
	char* s = NULL ;

	if ( !mref || !vol ) {
		return FALSE ;
	}

	// 开打这个MFT,存储信息到对应的inode结构体中. 则此时的ni就充当了下个截断字符串的父目录
	inum = MREF(mref);
	ni = ntfs_inode_open(vol, inum);
	if (!ni) {
		goto _close_ ;
	}

	ctx = ntfs_attr_get_search_ctx( ni, NULL );
	if (!ctx) {
		goto _close_ ;
	}
	
	if ( ntfs_attr_lookup( AT_INDEX_ROOT, NTFS_INDEX_I30, 4, CASE_SENSITIVE, 0, NULL, 0, ctx ) ) {
		goto _close_ ;
	}

	
	ir = (INDEX_ROOT*)((u8*)ctx->attr + le16_to_cpu(ctx->attr->X.value_offset));
	if ( NULL == ir ) {
		goto _close_ ;
	}

	if ( AT_FILE_NAME == ir->type ) { // 表明是目录
		bResult = TRUE ;
	}

	//
	// 取得90H属性中的名字,若存在且不为$I30,表明是流文件
	//
// 
// 	if ( FALSE == bResult )
// 	{
// 		s = ntfs_attr_name_get( 
// 			(ntfschar*)( (u8*)ctx->attr + ctx->attr->name_offset ), 
// 			(int)ctx->attr->name_length 
// 			);
// 	}
	

	//
	// 打开80H属性,获取名字. 若存在,即为流文件
	//

// 	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
// 	if (!na) {
// 		goto _close_;
// 	}

	  
_close_:
	if ( ni ) { 
		ntfs_inode_close(ni);
	}

	if (ctx) {
		ntfs_attr_put_search_ctx( ctx );
	}

	return bResult ;
}


VOID
ntfs_inode_Enum_name(
	IN ntfs_inode *dir_ni
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 0:32]

Routine Description:
  遍历当前目录下的所有文件(夹)
    
Arguments:
  dir_ni - 要遍历的节点

--*/
{
	VCN vcn;
	s64 br;
	u64 CurrentMref = 0;
	ntfs_volume *vol = dir_ni->vol;
	ntfs_attr_search_ctx *ctx;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_ALLOCATION *ia;
	u8 *index_end;
	ntfs_attr *ia_na;
	u32 index_block_size, index_vcn_size;
	u8 index_vcn_size_bits;
	char szBBBB[512] = "", *s = NULL ;
	BOOL bIsDiretory = FALSE ;

	ntfs_log_trace("Entering\n");

	if (!dir_ni) {
		errno = EINVAL;
		return ;
	}

	// 初始化查找结构体
	ctx = ntfs_attr_get_search_ctx(dir_ni, NULL);
	if (!ctx) { return ; }

	// 在此MFT中找到90H类型属性(索引根目)
	if ( ntfs_attr_lookup( AT_INDEX_ROOT, NTFS_INDEX_I30, 4, CASE_SENSITIVE, 0, NULL, 0, ctx ) ) 
	{
		ntfs_log_perror("Index root attribute missing in directory inode 0x%llx", (ULONG64)dir_ni->mft_no);
		goto put_err_out;
	}

	// 得到90H类型属性的内容
	ir = (INDEX_ROOT*)( (u8*)ctx->attr + le16_to_cpu(ctx->attr->X.value_offset) );

	// 90H类型属性包含的AOH属性的大小
	index_block_size = le32_to_cpu( ir->index_block_size );
	if (index_block_size < NTFS_BLOCK_SIZE || index_block_size & (index_block_size - 1)) 
	{
		ntfs_log_debug("Index block size %u is invalid.\n", (unsigned)index_block_size);
		goto put_err_out;
	}

	index_end = (u8*)&ir->index + le32_to_cpu(ir->index.index_length);

	// 0x90属性的第一个index entry
	ie = (INDEX_ENTRY*)((u8*)&ir->index + le32_to_cpu(ir->index.entries_offset));

	//
	// 遍历0x90属性的所有index entry
	//
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) 
	{
		if ((u8*)ie < (u8*)ctx->mrec
			|| (u8*)ie + sizeof(INDEX_ENTRY_HEADER) > index_end 
			|| (u8*)ie + le16_to_cpu(ie->key_length) > index_end
		   )
		{
			goto put_err_out;
		}
	
		// 索引项的最后一个节点是没有名字的. 但是它包含一个指向子节点的指针.
		if (ie->ie_flags & INDEX_ENTRY_END) { break; }

		CurrentMref = le64_to_cpu(ie->indexed_file);
		bIsDiretory = IsDirectory( vol, CurrentMref );
		
		//
		// 打印当前名字
		//

		s = ntfs_attr_name_get( 
			(ntfschar*)&ie->key.file_name.file_name, 
			(int)ie->key.file_name.file_name_length
			);
		
	//	DbgPrint( "  %s\n", s ) ;
		RtlZeroMemory( szBBBB, 512 );
		sprintf( 
			szBBBB,
			"  %s \t %s \n", 
			s,
			bIsDiretory == TRUE ? "[Diretory]" : ""
			);
		OutputDebugString( szBBBB );
		printf( szBBBB );
		
		ntfs_attr_name_free( &s );
	}

	// 在当前的索引项中没有找到,那么进入到它的子节点中继续查找
	if ( !(ie->ie_flags & INDEX_ENTRY_NODE) ) 
	{
		// 若当前索引项没有子节点,成功返回
		ntfs_attr_put_search_ctx(ctx);
		return ;
	} 

	////////////////////////////
	//						  //				
	// 开始在AOH属性中干活... //
	//						  //
	////////////////////////////

	// 打开0xA0属性
	ia_na = ntfs_attr_open(dir_ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
	if (!ia_na) 
	{
		ntfs_log_perror(
			"Failed to open index allocation attribute. Directory inode 0x%llx is corrupt or driver bug",
			(ULONG64)dir_ni->mft_no );

		goto put_err_out;
	}

	// 分配A0H属性大小的内存
	ia = (INDEX_ALLOCATION *)ntfs_malloc(index_block_size);
	if (!ia) {
		ntfs_attr_close(ia_na);
		goto put_err_out;
	}

	// Determine the size of a vcn in the directory index.
	if (vol->cluster_size <= index_block_size) {
		index_vcn_size = vol->cluster_size;
		index_vcn_size_bits = vol->cluster_size_bits;
	} else {
		index_vcn_size = vol->sector_size;
		index_vcn_size_bits = vol->sector_size_bits;
	}

	//
	// Get the starting vcn of the index_block holding the child node.
	// 也就是说,90H属性里面没有我们要找的名字,但是90属性有子节点,在其最后一个
	// index entry块中的最后8字节,存储这它拥有的子节点的VCN.而这个VCN在本MFT
	// 的A0H属性中,定位到该VCN的位置即可
	//
	vcn = sle64_to_cpup((u8*)ie + le16_to_cpu(ie->length) - 8);

descend_into_child_node: // 递归 (进入到子节点中)

	// 读A0H属性中指定的VCN (即读出该VCN所指向的index block).保存到ia中
	br = ntfs_attr_mst_pread( ia_na, vcn << index_vcn_size_bits, 1, index_block_size, ia );
	if (br != 1) 
	{
		if (br != -1) { errno = EIO; }
		ntfs_log_perror( "Failed to read vcn 0x%llx", (ULONG64)vcn );
		goto close_err_out;
	}

	if ( sle64_to_cpu(ia->index_block_vcn) != vcn ) // 合法性校验
	{
		errno = EIO;
		goto close_err_out;
	}

	// 合法性校验
	// 0x18是A0H属性的头部INDEX_BLOCK的大小,再加上它内容的大小,即为该block的总大小
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 != index_block_size) 
	{
		errno = EIO;
		goto close_err_out;
	}

	// 得到本Index_block的结束处
	index_end = (u8*)&ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8*)ia + index_block_size) 
	{
		errno = EIO;
		goto close_err_out;
	}

	// 0xA0属性的第一个index entry
	ie = (INDEX_ENTRY*)( (u8*)&ia->index + le32_to_cpu(ia->index.entries_offset) );

	//
	// 遍历0xA0属性的所有index entry
	//
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) 
	{
		if ((u8*)ie < (u8*)ia 
			|| (u8*)ie + sizeof(INDEX_ENTRY_HEADER) > index_end 
			|| (u8*)ie + le16_to_cpu(ie->key_length) > index_end 
		   ) 
		{
			ntfs_log_debug(
				"Index entry out of bounds in directory inode 0x%llx.\n",
				(ULONG64)dir_ni->mft_no );

			errno = EIO;
			goto close_err_out;
		}

		if (ie->ie_flags & INDEX_ENTRY_END) { break; }

		//
		// 打印当前名字
		//

		CurrentMref = le64_to_cpu(ie->indexed_file);
		bIsDiretory = IsDirectory( vol, CurrentMref );
		
		//
		// 打印当前名字
		//
		
		s = ntfs_attr_name_get( 
			(ntfschar*)&ie->key.file_name.file_name, 
			(int)ie->key.file_name.file_name_length
			);
		
		//	DbgPrint( "  %s\n", s ) ;
		RtlZeroMemory( szBBBB, 512 );
		sprintf( 
			szBBBB,
			"  %s \t %s \n", 
			s,
			bIsDiretory == TRUE ? "[Diretory]" : ""
			);
		OutputDebugString( szBBBB );
		printf( szBBBB );

		ntfs_attr_name_free( &s );
	}

	/*
	 * We have finished with this index buffer without success. Check for
	 * the presence of a child node.
	 */
	if ( ie->ie_flags & INDEX_ENTRY_NODE )
	{
		if ( (ia->index.ih_flags & NODE_MASK) == LEAF_NODE ) 
		{
			ntfs_log_debug(
				"Index entry with child node found in a leaf node in directory inode 0x%llx.\n",
				(ULONG64)dir_ni->mft_no );

			goto close_err_out;
		}

		// 该子节点又存在子节点,递归遍历之
		vcn = sle64_to_cpup((u8*)ie + le16_to_cpu(ie->length) - 8);
		if (vcn >= 0) { goto descend_into_child_node; }

		ntfs_log_debug("Negative child node vcn in directory inode 0x%llx.\n", (ULONG64)dir_ni->mft_no);
		goto close_err_out;
	}

	// 收尾工作
	free(ia);
	ntfs_attr_close(ia_na);
	ntfs_attr_put_search_ctx(ctx);
	
	return ;

put_err_out:

eo_put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	return ;

close_err_out:
	free(ia);
	ntfs_attr_close(ia_na);
	goto eo_put_err_out;
}







/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


u64 
ntfs_inode_lookup_by_name(
	IN ntfs_inode *dir_ni, 
	IN const ntfschar *uname,
	IN const int uname_len
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 0:32]

Routine Description:
  find an inode in a directory given its name
  在目录中递归查找出包含指定名字的节点. 尽管是区分大小写来查找,但当没有找到时,
  也会采用不区分大小写的方式进行查找. If the volume is mounted with the case
  sensitive flag set, then we only allow exact matches.
    
Arguments:
  dir_ni - ntfs inode of the directory in which to search for the name
  uname - 要查找的unicode字符串名字
  uname_len - 要查找的unicode字符串长度,不包含结束符'\0'
    
Return Value:
  If the name is found in the directory, the corresponding inode number (>= 0)
  is returned as a mft reference in cpu format, On error, return -1 with errno
  set to the error code.

--*/
{
	VCN vcn;
	u64 mref = 0;
	s64 br;
	ntfs_volume *vol = dir_ni->vol;
	ntfs_attr_search_ctx *ctx;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_ALLOCATION *ia;
	u8 *index_end;
	ntfs_attr *ia_na;
	int eo, rc;
	u32 index_block_size, index_vcn_size;
	u8 index_vcn_size_bits;

	ntfs_log_trace("Entering\n");

	if (!dir_ni || !dir_ni->mrec || !uname || uname_len <= 0) {
		errno = EINVAL;
		return -1;
	}

	// 初始化查找结构体
	ctx = ntfs_attr_get_search_ctx(dir_ni, NULL);
	if (!ctx) { return -1; }

	// 在此MFT中找到90H类型属性(索引根目)
	if ( ntfs_attr_lookup( AT_INDEX_ROOT, NTFS_INDEX_I30, 4, CASE_SENSITIVE, 0, NULL, 0, ctx ) ) 
	{
		ntfs_log_perror("Index root attribute missing in directory inode 0x%llx", (ULONG64)dir_ni->mft_no);
		goto put_err_out;
	}

	// 得到90H类型属性的内容
	ir = (INDEX_ROOT*)( (u8*)ctx->attr + le16_to_cpu(ctx->attr->X.value_offset) );

	// 90H类型属性包含的AOH属性的大小
	index_block_size = le32_to_cpu( ir->index_block_size );
	if (index_block_size < NTFS_BLOCK_SIZE || index_block_size & (index_block_size - 1)) 
	{
		ntfs_log_debug("Index block size %u is invalid.\n", (unsigned)index_block_size);
		goto put_err_out;
	}

	index_end = (u8*)&ir->index + le32_to_cpu(ir->index.index_length);

	// 0x90属性的第一个index entry
	ie = (INDEX_ENTRY*)((u8*)&ir->index + le32_to_cpu(ir->index.entries_offset));

	//
	// 遍历0x90属性的所有index entry
	//
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) 
	{
		if ((u8*)ie < (u8*)ctx->mrec
			|| (u8*)ie + sizeof(INDEX_ENTRY_HEADER) > index_end 
			|| (u8*)ie + le16_to_cpu(ie->key_length) > index_end
		   )
		{
			goto put_err_out;
		}
	
		// 索引项的最后一个节点是没有名字的. 但是它包含一个指向子节点的指针.
		if (ie->ie_flags & INDEX_ENTRY_END) { break; }
	
		rc = ntfs_names_collate(uname, uname_len,
				(ntfschar*)&ie->key.file_name.file_name,
				ie->key.file_name.file_name_length, 1,
				IGNORE_CASE, vol->upcase, vol->upcase_len);
		//
		// 若校对过程中返回-1,表明要查找的名字比当前entry中的名字要短,那么肯定不匹配
		// but we might need to descend into the B+tree so we just break out of the loop.
		//
		if (rc == -1) { break; }

		// 若返回值>0,表明当前不匹配,继续下一个
		if (rc) { continue; }

		// 虽然忽略大小写后,能匹配.但是我们要进一步的匹配,即大小写敏感
		rc = ntfs_names_collate(uname, uname_len,
				(ntfschar*)&ie->key.file_name.file_name,
				ie->key.file_name.file_name_length, 1,
				CASE_SENSITIVE, vol->upcase, vol->upcase_len);

		if (rc == -1) { break; }
		if (rc) { continue; }

		// 完全匹配,OK! 返回当前这个匹配的索引块的MFT
		mref = le64_to_cpu(ie->indexed_file);
		ntfs_attr_put_search_ctx(ctx);
		return mref;
	}

	// 在当前的索引项中没有找到,那么进入到它的子节点中继续查找
	if ( !(ie->ie_flags & INDEX_ENTRY_NODE) ) 
	{
		// 若当前索引项没有子节点,就彻底失败了...
		ntfs_attr_put_search_ctx(ctx);
		if (mref) { return mref; }

		ntfs_log_debug("Entry not found.\n");
		errno = ENOENT;
		return -1;
	} 

	////////////////////////////
	//						  //				
	// 开始在AOH属性中干活... //
	//						  //
	////////////////////////////

	// 打开0xA0属性
	ia_na = ntfs_attr_open(dir_ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
	if (!ia_na) 
	{
		ntfs_log_perror(
			"Failed to open index allocation attribute. Directory inode 0x%llx is corrupt or driver bug",
			(ULONG64)dir_ni->mft_no );

		goto put_err_out;
	}

	// 分配A0H属性大小的内存
	ia = (INDEX_ALLOCATION *)ntfs_malloc(index_block_size);
	if (!ia) {
		ntfs_attr_close(ia_na);
		goto put_err_out;
	}

	// Determine the size of a vcn in the directory index.
	if (vol->cluster_size <= index_block_size) {
		index_vcn_size = vol->cluster_size;
		index_vcn_size_bits = vol->cluster_size_bits;
	} else {
		index_vcn_size = vol->sector_size;
		index_vcn_size_bits = vol->sector_size_bits;
	}

	//
	// Get the starting vcn of the index_block holding the child node.
	// 也就是说,90H属性里面没有我们要找的名字,但是90属性有子节点,在其最后一个
	// index entry块中的最后8字节,存储这它拥有的子节点的VCN.而这个VCN在本MFT
	// 的A0H属性中,定位到该VCN的位置即可
	//
	vcn = sle64_to_cpup((u8*)ie + le16_to_cpu(ie->length) - 8);

descend_into_child_node: // 递归 (进入到子节点中)

	// 读A0H属性中指定的VCN (即读出该VCN所指向的index block).保存到ia中
	br = ntfs_attr_mst_pread( ia_na, vcn << index_vcn_size_bits, 1, index_block_size, ia );
	if (br != 1) 
	{
		if (br != -1) { errno = EIO; }
		ntfs_log_perror( "Failed to read vcn 0x%llx", (ULONG64)vcn );
		goto close_err_out;
	}

	if ( sle64_to_cpu(ia->index_block_vcn) != vcn ) // 合法性校验
	{
		errno = EIO;
		goto close_err_out;
	}

	// 合法性校验
	// 0x18是A0H属性的头部INDEX_BLOCK的大小,再加上它内容的大小,即为该block的总大小
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 != index_block_size) 
	{
		errno = EIO;
		goto close_err_out;
	}

	// 得到本Index_block的结束处
	index_end = (u8*)&ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8*)ia + index_block_size) 
	{
		errno = EIO;
		goto close_err_out;
	}

	// 0xA0属性的第一个index entry
	ie = (INDEX_ENTRY*)( (u8*)&ia->index + le32_to_cpu(ia->index.entries_offset) );

	//
	// 遍历0xA0属性的所有index entry
	//
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) 
	{
		if ((u8*)ie < (u8*)ia 
			|| (u8*)ie + sizeof(INDEX_ENTRY_HEADER) > index_end 
			|| (u8*)ie + le16_to_cpu(ie->key_length) > index_end 
		   ) 
		{
			ntfs_log_debug(
				"Index entry out of bounds in directory inode 0x%llx.\n",
				(ULONG64)dir_ni->mft_no );

			errno = EIO;
			goto close_err_out;
		}

		if (ie->ie_flags & INDEX_ENTRY_END) { break; }
	
		rc = ntfs_names_collate(uname, uname_len,
				(ntfschar*)&ie->key.file_name.file_name,
				ie->key.file_name.file_name_length, 1,
				IGNORE_CASE, vol->upcase, vol->upcase_len);

		if (rc == -1) { break; }
		if (rc) { continue; }
	
		rc = ntfs_names_collate(uname, uname_len,
				(ntfschar*)&ie->key.file_name.file_name,
				ie->key.file_name.file_name_length, 1,
				CASE_SENSITIVE, vol->upcase, vol->upcase_len);
		
		if (rc == -1) { break; }
		if (rc) { continue; }
		
		mref = le64_to_cpu(ie->indexed_file);
		free( ia );
		ntfs_attr_close(ia_na);
		ntfs_attr_put_search_ctx(ctx);
		return mref;
	}

	/*
	 * We have finished with this index buffer without success. Check for
	 * the presence of a child node.
	 */
	if ( ie->ie_flags & INDEX_ENTRY_NODE )
	{
		if ( (ia->index.ih_flags & NODE_MASK) == LEAF_NODE ) 
		{
			ntfs_log_debug(
				"Index entry with child node found in a leaf node in directory inode 0x%llx.\n",
				(ULONG64)dir_ni->mft_no );

			errno = EIO;
			goto close_err_out;
		}

		// 该子节点又存在子节点,递归遍历之
		vcn = sle64_to_cpup((u8*)ie + le16_to_cpu(ie->length) - 8);
		if (vcn >= 0) { goto descend_into_child_node; }

		ntfs_log_debug("Negative child node vcn in directory inode 0x%llx.\n", (ULONG64)dir_ni->mft_no);
		errno = EIO;
		goto close_err_out;
	}

	// 收尾工作
	free(ia);
	ntfs_attr_close(ia_na);
	ntfs_attr_put_search_ctx(ctx);
	
	if (mref) { return mref; }
	ntfs_log_debug("Entry not found.\n");
	errno = ENOENT;
	return -1;

put_err_out:
	eo = EIO;
	ntfs_log_debug("Corrupt directory. Aborting lookup.\n");

eo_put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	errno = eo;
	return -1;

close_err_out:
	eo = errno;
	free(ia);
	ntfs_attr_close(ia_na);
	goto eo_put_err_out;
}



ntfs_inode *
ntfs_pathname_to_inode(
	IN ntfs_volume *vol, 
	IN ntfs_inode *parent,
	IN const char *pathname
	)
/*++

Routine Description:
  找到@pathname对应的结构体inode  
    
Arguments:
  vol - An ntfs volume obtained from ntfs_mount
  parent - A directory inode to begin the search (may be NULL).若为空,
		   那么就从根目录开始查找
  pathname - Pathname to be located.ASCII格式
    
--*/
{
	u64 inum;
	int len, err = 0;
	char *p, *q;
	ntfs_inode *ni;
	ntfs_inode *result = NULL;
	ntfschar *unicode = NULL;
	char *ascii = NULL;

	if (!vol || !pathname) {
		errno = EINVAL;
		return NULL;
	}
	
	ntfs_log_trace("path: '%s'\n", pathname);
	
	if (parent) {
		ni = parent;

	} else {
		// 若参数2为空,那么就从根目录开始查找
		ni = ntfs_inode_open( vol, FILE_root );
		if (!ni) 
		{
			ntfs_log_debug("Couldn't open the inode of the root directory.\n");
			err = EIO;
			goto close;
		}
	}

	unicode = ntfs_calloc( MAX_PATH );
	ascii = strdup( pathname );
	if (!unicode || !ascii)
	{
		ntfs_log_debug("Out of memory.\n");
		err = ENOMEM;
		goto close;
	}

	p = ascii;

	// 是从根目录开始搜,去掉 c: 这个2个字节
	if ( (NULL == parent) && p && *p && *(++p) == ':' ) {
		p++ ;
	}

	// 去掉路径开头出的 '/'
	while (p && *p && *p == PATH_SEP) {
		p++; 
	}

	//
	// 比如完整路径为"c:\\sudami\\a.txt". 这个循环是依次截取每一段的内容
	//
	while (p && *p) 
	{
		// Find the end of the first token.
		q = strchr(p, PATH_SEP);
		if (q != NULL) {
			*q = '\0';
			q++;
		}

		// 将多字符串转换成2-byte的unicode字符
		len = ntfs_mbstoucs( p, &unicode, NTFS_MAX_NAME_LEN );
		if (len < 0) {
			ntfs_log_debug("Couldn't convert name to Unicode: %s.\n", p);
			err = errno;
			goto close;
		}

		// 得到这个截断的字符串对应的MFT
		inum = ntfs_inode_lookup_by_name( ni, unicode, len );
		if (inum == (u64) -1) 
		{
			ntfs_log_debug("Couldn't find name '%s' in pathname '%s'.\n", p, pathname);
			err = ENOENT;
			goto close;
		}

		if (ni != parent) { ntfs_inode_close(ni); }

		// 开打这个MFT,存储信息到对应的inode结构体中. 则此时的ni就充当了下个截断字符串的父目录
		inum = MREF(inum);
		ni = ntfs_inode_open(vol, inum);
		if (!ni) {
			ntfs_log_debug("Cannot open inode %llu: %s.\n", (ULONG64)inum, p);
			err = EIO;
			goto close;
		}

		p = q;
		while (p && *p && *p == PATH_SEP) { p++; }
	} // end-of-while

	// 最终定位到指定文件所对应的inode结构体了,返回之
	result = ni;
	ni = NULL;

close:
	if (ni && (ni != parent)) { ntfs_inode_close(ni); }
	free(ascii);
	free(unicode);
	if (err) { errno = err; }
	return result;
}

/*
 * The little endian Unicode string ".." for ntfs_readdir().
 */
static const ntfschar dotdot[3] = { const_cpu_to_le16('.'),
				   const_cpu_to_le16('.'),
				   const_cpu_to_le16('\0') };

/*
 * union index_union -
 * More helpers for ntfs_readdir().
 */
typedef union {
	INDEX_ROOT *ir;
	INDEX_ALLOCATION *ia;
} index_union;

/**
 * enum INDEX_TYPE -
 * More helpers for ntfs_readdir().
 */
typedef enum {
	INDEX_TYPE_ROOT,	/* index root */
	INDEX_TYPE_ALLOCATION	/* index allocation */
} INDEX_TYPE;



static int 
ntfs_filldir(
	ntfs_inode *dir_ni, 
	s64 *pos,
	u8 ivcn_bits,
	const INDEX_TYPE index_type, 
	/*index_union*/void *iuTmp, 
	INDEX_ENTRY *ie,
	void *dirent, 
	ntfs_filldir_t filldir
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/26 [26:3:2009 - 14:05]

Routine Description:
  Pass information specifying the current directory entry @ie to the @filldir callback.   
    
Arguments:
  dir_ni - 当前目录的inode结构
  pos - current position in directory
  ivcn_bits - log(2) of index vcn size
  index_type - 指定参数五 @iuTmp为 index root 或 index allocation
  iuTmp - index root or index block to which @ie belongs
  ie - 当前的 index entry
  dirent - context for filldir callback supplied by the caller
  filldir - filldir callback supplied by the caller

Return Value:

--*/
{
	FILE_NAME_ATTR *fn = &ie->key.file_name;
	unsigned dt_type;
	
	ntfs_log_trace("Entering.\n");
	
	// Advance the position even if going to skip the entry.
	if ( index_type == INDEX_TYPE_ALLOCATION ) 
	{
		INDEX_ALLOCATION *iu = (INDEX_ALLOCATION *)iuTmp ;

		*pos = (u8*)ie - (u8*)iu + (sle64_to_cpu(iu->index_block_vcn) << ivcn_bits) + dir_ni->vol->mft_record_size ;
	} 
	else 
	{ /* if (index_type == INDEX_TYPE_ROOT) */
		INDEX_ROOT *iu = (INDEX_ROOT *)iuTmp ;
		*pos = (u8*)ie - (u8*)iu;
	}

	// Skip root directory self reference entry.
	if ( MREF_LE(ie->indexed_file) == FILE_root )
		return 0 ;

	if ( ie->key.file_name.file_attributes & FILE_ATTR_I30_INDEX_PRESENT )
		dt_type = NTFS_DT_DIR ;
	else
		dt_type = NTFS_DT_REG ;

	return filldir(
			dirent, 
			fn->file_name,			// 当前文件名
			fn->file_name_length,	// 当前文件名长度
			fn->file_name_type,		// 当前文件名类型
			*pos,					// 位置
			le64_to_cpu( ie->indexed_file ), 
			dt_type					// "目录 | 文件" 标记
			);
}



static MFT_REF 
ntfs_mft_get_parent_ref(
	IN ntfs_inode *ni
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/26 [26:3:2009 - 13:38]

Routine Description:
  得到当前目录的父目录的MFT | 即打开该目录的30H属性,偏移18H处便是

  [ Note this only makes sense for directories, since files can be hard linked
  from multiple directories and there is no way for us to tell which one is
  being looked for. Technically directories can have hard links, too, but we 
  consider that as illegal as Linux/UNIX do not support directory hard links. ]
    
--*/
{
	MFT_REF mref;
	ntfs_attr_search_ctx *ctx;
	FILE_NAME_ATTR *fn;
	int eo;

	ntfs_log_trace("Entering.\n");
	
	if (!ni) {
		errno = EINVAL;
		return ERR_MREF(-1);
	}

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx)
		return ERR_MREF(-1);

	// 打开30H属性
	if (ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0, 0, 0, NULL, 0, ctx)) {
		ntfs_log_debug("No file name found in inode 0x%llx. Corrupt inode.\n", (ULONG64)ni->mft_no);
		goto err_out;
	}

	// 一定是常驻属性才行
	if (ctx->attr->non_resident) {
		ntfs_log_debug("File name attribute must be resident. Corrupt inode 0x%llx.\n", (ULONG64)ni->mft_no);
		goto io_err_out;
	}

	fn = (FILE_NAME_ATTR*)((u8*)ctx->attr + le16_to_cpu(ctx->attr->X.value_offset));

	if ((u8*)fn +	le32_to_cpu(ctx->attr->X.value_length) > (u8*)ctx->attr + le32_to_cpu(ctx->attr->length))
	{
		ntfs_log_debug("Corrupt file name attribute in inode 0x%llx.\n", (ULONG64)ni->mft_no);
		goto io_err_out;
	}

	// OK!
	mref = le64_to_cpu(fn->parent_directory);
	ntfs_attr_put_search_ctx(ctx);
	return mref;

io_err_out:
	errno = EIO;
err_out:
	eo = errno;
	ntfs_attr_put_search_ctx(ctx);
	errno = eo;
	return ERR_MREF(-1);
}



int 
ntfs_readdir(
	ntfs_inode *dir_ni, 
	s64 *pos,
	void *dirent, 
	ntfs_filldir_t filldir
	)
/*++

Routine Description:
  解析目录 | 解析$bitmap中被标记为"正在使用中" 的index root & index blocks.
  每找到一个directory entry,传递给callback函数 - filldir ,让其后续处理.
  按照VCN递增模式解析Index blocks, 故找到的文件名不是按名子顺序排列的.

  [ Parse the index root and the index blocks that are marked in use in the
  index bitmap and hand each found directory entry to the @filldir callback
  supplied by the caller. Index blocks are parsed in ascending vcn order,
  from which follows that the directory entries are not returned sorted.]
    
Arguments:
  dir_ni - 当前目录的inode结构
  pos -	current position in directory
  dirent - context for filldir callback supplied by the caller
  filldir - filldir callback supplied by the caller

Return Value:
  0 -- 成功; -1 -- 失败
    
--*/
{
	s64 i_size, br, ia_pos, bmp_pos, ia_start;
	ntfs_volume *vol;
	ntfs_attr *ia_na, *bmp_na = NULL;
	ntfs_attr_search_ctx *ctx = NULL;
	u8 *index_end, *bmp = NULL;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_ALLOCATION *ia = NULL;
	int rc, ir_pos, bmp_buf_size, bmp_buf_pos, eo;
	u32 index_block_size, index_vcn_size;
	u8 index_block_size_bits, index_vcn_size_bits;

	ntfs_log_trace("Entering.\n");
	
	if (!dir_ni || !pos || !filldir) {
		errno = EINVAL;
		return -1;
	}

	if (!(dir_ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)) {
		errno = ENOTDIR;
		return -1;
	}

	vol = dir_ni->vol;

	ntfs_log_trace("Entering for inode 0x%llx, *pos 0x%llx.\n", (ULONG64)dir_ni->mft_no, (LONG64)*pos);

	// 打开A0H属性
	ia_na = ntfs_attr_open(dir_ni, AT_INDEX_ALLOCATION, NTFS_INDEX_I30, 4);
	if (!ia_na) 
	{
		if (errno != ENOENT) {
			ntfs_log_perror("Failed to open index allocation attribute. Directory inode 0x%llx is corrupt or bug", (ULONG64)dir_ni->mft_no);
			return -1;
		}

		i_size = 0;
	} else {
		i_size = ia_na->data_size;
	}

	rc = 0;

	// Are we at end of dir yet?
	if ( *pos >= i_size + vol->mft_record_size )
		goto done;

	// Emulate . and .. for all directories. 

	//
	// 处理2个特殊目录 "." & ".."
	//

	if ( !*pos ) 
	{
		// 目录为 ".", 即表示当前目录
		rc = filldir(
				dirent,
				dotdot,				// 当前文件名
				1,					// 当前文件名长度
				FILE_NAME_POSIX,	// 当前文件名类型
				*pos,
				MK_MREF(dir_ni->mft_no, le16_to_cpu(dir_ni->mrec->sequence_number)),
				NTFS_DT_DIR			// "目录" 标记
				);

		if (rc) goto done;
		++*pos ;
	}

	if ( *pos == 1 ) 
	{
		MFT_REF parent_mref;

		// 得到当前目录的父目录的MFT
		parent_mref = ntfs_mft_get_parent_ref( dir_ni );
		if ( parent_mref == ERR_MREF(-1) )
		{
			ntfs_log_perror("Parent directory not found");
			goto dir_err_out;
		}

		// 目录为 "..", 即表示当前目录的父目录
		rc = filldir(
			dirent, 
			dotdot,				// 当前文件名
			2,					// 当前文件名长度
			FILE_NAME_POSIX,	// 当前文件名类型
			*pos,
			parent_mref,
			NTFS_DT_DIR			// "目录" 标记
			);

		if (rc) goto done;
		++*pos ;
	}

	//
	// 开始枚举 90H属性 & A0H属性
	//

	ctx = ntfs_attr_get_search_ctx(dir_ni, NULL);
	if (!ctx) goto err_out;

	// Get the offset into the index root attribute.
	ir_pos = (int)*pos;

	// 打开90H属性
	if ( ntfs_attr_lookup(AT_INDEX_ROOT, NTFS_INDEX_I30, 4, CASE_SENSITIVE, 0, NULL, 0, ctx) )
	{
		ntfs_log_debug("Index root attribute missing in directory inode 0x%llx.\n", (ULONG64)dir_ni->mft_no);
		goto dir_err_out;
	}

	// 跳过90H属性的标准头,到达 索引根(INDEX_ROOT)
	ir = (INDEX_ROOT*) ( (u8*)ctx->attr + le16_to_cpu(ctx->attr->X.value_offset) );

	// Determine the size of a vcn in the directory index.
	// 90H类型属性包含的AOH属性的大小
	index_block_size = le32_to_cpu(ir->index_block_size);
	if (index_block_size < NTFS_BLOCK_SIZE || index_block_size & (index_block_size - 1)) 
	{
		goto dir_err_out;
	}

	index_block_size_bits = (u8)ffs(index_block_size) - 1;
	if ( vol->cluster_size <= index_block_size ) {
		index_vcn_size = vol->cluster_size;
		index_vcn_size_bits = vol->cluster_size_bits;
	} else {
		index_vcn_size = vol->sector_size;
		index_vcn_size_bits = vol->sector_size_bits;
	}

	// 是否要直接跳到A0H属性去
	if ( *pos >= vol->mft_record_size ) 
	{
		ntfs_attr_put_search_ctx(ctx);
		ctx = NULL;
		goto skip_index_root;
	}

	index_end = (u8*)&ir->index + le32_to_cpu(ir->index.index_length);

	// 0x90属性的第一个index entry
	ie = (INDEX_ENTRY*)((u8*)&ir->index + le32_to_cpu(ir->index.entries_offset));

	//
	// 遍历0x90属性的所有index entry
	//

	for ( ;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length)) ) 
	{
		ntfs_log_debug("In index root, offset 0x%x.\n", (u8*)ie - (u8*)ir);

		// 越界检查
		if ((u8*)ie < (u8*)ctx->mrec 
			|| (u8*)ie + sizeof(INDEX_ENTRY_HEADER) > index_end
			|| (u8*)ie + le16_to_cpu(ie->key_length) > index_end
		   )
		{
			goto dir_err_out;
		}

		// 索引项的最后一个节点是没有名字的. 但是它包含一个指向子节点的指针.
		if ( ie->ie_flags & INDEX_ENTRY_END )
			break;

		// Skip index root entry if continuing previous readdir.
		if ( ir_pos > (u8*)ie - (u8*)ir )
			continue;

		//
		// Submit the directory entry to ntfs_filldir(), which will invoke the filldir() callback as appropriate.
		//

		rc = ntfs_filldir(
			dir_ni, 
			pos, 
			index_vcn_size_bits, 
			INDEX_TYPE_ROOT,
			(PVOID)ir,
			ie,
			dirent, 
			filldir
			);

		if (rc) 
		{
			ntfs_attr_put_search_ctx(ctx);
			ctx = NULL;
			goto done;
		}
	}

	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;

	// 若没有A0H属性,表明枚举结束
	if (!ia_na)
		goto EOD;

	// Advance *pos to the beginning of the index allocation.
	*pos = vol->mft_record_size;

skip_index_root:

	///////////////////////////
	//						 //
	// 主要精力放到A0H属性中 //
	//						 //
	///////////////////////////

	if (!ia_na)
		goto done;

	// 为当前的 index block 分配内存
	ia = (INDEX_ALLOCATION *)ntfs_malloc(index_block_size);
	if (!ia)
		goto err_out;

	// 打开B0H属性
	bmp_na = ntfs_attr_open( dir_ni, AT_BITMAP, NTFS_INDEX_I30, 4 );
	if (!bmp_na) {
		ntfs_log_perror("Failed to open index bitmap attribute");
		goto dir_err_out;
	}

	// Get the offset into the index allocation attribute.
	ia_pos = *pos - vol->mft_record_size;

	bmp_pos = ia_pos >> index_block_size_bits;
	if (bmp_pos >> 3 >= bmp_na->data_size) {
		ntfs_log_debug("Current index position exceeds index bitmap size.\n");
		goto dir_err_out;
	}

	bmp_buf_size = min(bmp_na->data_size - (bmp_pos >> 3), 4096);
	bmp = (u8 *) ntfs_malloc(bmp_buf_size);
	if (!bmp)
		goto err_out;

	// 读取bitmap中指定位置的内容
	br = ntfs_attr_pread(bmp_na, bmp_pos >> 3, bmp_buf_size, bmp);
	if (br != bmp_buf_size) 
	{
		if (br != -1)
			errno = EIO;
		ntfs_log_perror("Failed to read from index bitmap attribute");
		goto err_out;
	}

	bmp_buf_pos = 0;

	// If the index block is not in use find the next one that is.
	while ( !(bmp[bmp_buf_pos >> 3] & (1 << (bmp_buf_pos & 7))) )
	{
find_next_index_buffer:
		bmp_pos++;
		bmp_buf_pos++;

		// 若到了bitmap的结尾处,跳出
		if (bmp_pos >> 3 >= bmp_na->data_size)
			goto EOD;

		ia_pos = bmp_pos << index_block_size_bits;
		if (bmp_buf_pos >> 3 < bmp_buf_size)
			continue;

		// Read next chunk from the index bitmap. 
		if ((bmp_pos >> 3) + bmp_buf_size > bmp_na->data_size)
			bmp_buf_size = bmp_na->data_size - (bmp_pos >> 3);

		br = ntfs_attr_pread(bmp_na, bmp_pos >> 3, bmp_buf_size, bmp);
		if (br != bmp_buf_size) 
		{
			if (br != -1)
				errno = EIO;
			ntfs_log_perror("Failed to read from index bitmap attribute");
			goto err_out;
		}
	}

	ntfs_log_debug( "Handling index block 0x%llx.\n", (LONG64)bmp_pos );

	// 从bmp_pos 位置读取index block 
	br = ntfs_attr_mst_pread( ia_na, bmp_pos << index_block_size_bits, 1, index_block_size, ia );
	if (br != 1) 
	{
		if (br != -1)
			errno = EIO;
		ntfs_log_perror("Failed to read index block");
		goto err_out;
	}

	ia_start = ia_pos & ~(s64)(index_block_size - 1);
	if ( sle64_to_cpu(ia->index_block_vcn) != ia_start >> index_vcn_size_bits )
	{
		ntfs_log_debug(
			"Actual VCN (0x%llx) of index buffer is different from expected VCN (0x%llx) in inode 0x%llx.\n",
			(LONG64) sle64_to_cpu(ia->index_block_vcn),
			(LONG64) ia_start >> index_vcn_size_bits,
			(ULONG64) dir_ni->mft_no
			);

		goto dir_err_out;
	}

	// 合法性校验. 0x18是A0H属性的头部INDEX_BLOCK的大小,再加上它内容的大小,即为该block的总大小
	if ( le32_to_cpu(ia->index.allocated_size) + 0x18 != index_block_size ) 
	{
		ntfs_log_debug(
			"Index buffer (VCN 0x%llx) of directory inode 0x%llx has a size (%u) differing from the directory specified size (%u).\n", 
			(LONG64) ia_start >> index_vcn_size_bits,
			(ULONG64) dir_ni->mft_no,
			(unsigned) le32_to_cpu(ia->index.allocated_size) + 0x18, 
			(unsigned) index_block_size 
			);

		goto dir_err_out;
	}

	// 得到本Index_block的结束处
	index_end = (u8*)&ia->index + le32_to_cpu(ia->index.index_length);
	if ( index_end > (u8*)ia + index_block_size ) 
	{
		ntfs_log_debug(
			"Size of index buffer (VCN 0x%llx) of directory inode 0x%llx exceeds maximum size.\n",
				(LONG64)ia_start >> index_vcn_size_bits,
				(ULONG64)dir_ni->mft_no
				);

		goto dir_err_out;
	}

	// 0xA0属性的第一个index entry
	ie = (INDEX_ENTRY*) ( (u8*)&ia->index + le32_to_cpu(ia->index.entries_offset) );

	//
	// 遍历0xA0属性的所有index entry
	//

	for ( ;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length)) ) 
	{
		ntfs_log_debug("In index allocation, offset 0x%llx.\n",(LONG64)ia_start + ((u8*)ie - (u8*)ia));
		
		// 越界检查
		if ((u8*)ie < (u8*)ia 
			|| (u8*)ie + sizeof(INDEX_ENTRY_HEADER) > index_end 
			|| (u8*)ie + le16_to_cpu(ie->key_length) > index_end
		   )
		{
			ntfs_log_debug("Index entry out of bounds in directory inode 0x%llx.\n", (ULONG64)dir_ni->mft_no);
			goto dir_err_out;
		}

		// 索引项的最后一个节点是没有名字的. 但是它包含一个指向子节点的指针.
		if (ie->ie_flags & INDEX_ENTRY_END)
			break;

		// Skip index entry if continuing previous readdir.
		if (ia_pos - ia_start > (u8*)ie - (u8*)ia)
			continue;

		//
		// Submit the directory entry to ntfs_filldir(), which will invoke the filldir() callback as appropriate.
		//

		rc = ntfs_filldir( dir_ni, pos, index_vcn_size_bits, INDEX_TYPE_ALLOCATION, (PVOID)ia, ie, dirent, filldir );
		if (rc)
			goto done;
	}

	goto find_next_index_buffer;

EOD:
	// We are finished, set *pos to EOD.
	*pos = i_size + vol->mft_record_size;
done:
	free(ia);
	free(bmp);
	if (bmp_na)
		ntfs_attr_close(bmp_na);
	if (ia_na)
		ntfs_attr_close(ia_na);
#ifdef DEBUG
	if (!rc)
		ntfs_log_debug("EOD, *pos 0x%llx, returning 0.\n", (LONG64)*pos);
	else
		ntfs_log_debug("filldir returned %i, *pos 0x%llx, returning 0.\n", rc, (LONG64)*pos);
#endif
	return 0;
dir_err_out:
	errno = EIO;
err_out:
	eo = errno;
	ntfs_log_trace("failed.\n");
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	free(ia);
	free(bmp);
	if (bmp_na)
		ntfs_attr_close(bmp_na);
	if (ia_na)
		ntfs_attr_close(ia_na);
	errno = eo;
	return -1;
}


/**
 * __ntfs_create - create object on ntfs volume
 * @dir_ni:	ntfs inode for directory in which create new object
 * @name:	unicode name of new object
 * @name_len:	length of the name in unicode characters
 * @type:	type of the object to create
 * @dev:	major and minor device numbers (obtained from makedev())
 * @target:	target in unicode (only for symlinks)
 * @target_len:	length of target in unicode characters
 *
 * Internal, use ntfs_create{,_device,_symlink} wrappers instead.
 *
 * @type can be:
 *	S_IFREG		to create regular file
 *	S_IFDIR		to create directory
 *	S_IFBLK		to create block device
 *	S_IFCHR		to create character device
 *	S_IFLNK		to create symbolic link
 *	S_IFIFO		to create FIFO
 *	S_IFSOCK	to create socket
 * other values are invalid.
 *
 * @dev is used only if @type is S_IFBLK or S_IFCHR, in other cases its value
 * ignored.
 *
 * @target and @target_len are used only if @type is S_IFLNK, in other cases
 * their value ignored.
 *
 * Return opened ntfs inode that describes created object on success or NULL
 * on error with errno set to the error code.
 */
static ntfs_inode *
__ntfs_create(
	IN ntfs_inode *dir_ni,
	IN ntfschar *name, 
	IN u8 name_len, 
	IN dev_t type,
	IN dev_t dev,
	IN ntfschar *target, 
	IN u8 target_len
	)
{
	ntfs_inode *ni;
	int rollback_data = 0, rollback_sd = 0;
	FILE_NAME_ATTR *fn = NULL;
	STANDARD_INFORMATION *si = NULL;
	int err, fn_len, si_len;

	ntfs_log_trace("Entering.\n");
	if (!dir_ni || !name || !name_len) { 
		ntfs_log_error("Invalid arguments.\n");
		errno = EINVAL;
		return NULL;
	}

	//
	// 在$MFT:$bitmap中找到一个空闲位.设置为FF.
	// 并在这个所指向的空闲区域分配一个新的MFT
	//

	ni = ntfs_mft_record_alloc(dir_ni->vol, NULL);
	if (!ni) {
		ntfs_log_perror("Could not allocate new MFT record");
		return NULL;
	}

	//
	// 创建 & 写入标准属性(10H) STANDARD_INFORMATION
	//

	si_len = offsetof(STANDARD_INFORMATION, v1_end);
	si = ntfs_calloc(si_len);
	if (!si) {
		err = errno;
		goto err_out;
	}
	si->creation_time = utc2ntfs(ni->creation_time);
	si->last_data_change_time = utc2ntfs(ni->last_data_change_time);
	si->last_mft_change_time = utc2ntfs(ni->last_mft_change_time);
	si->last_access_time = utc2ntfs(ni->last_access_time);
	if (!S_ISREG(type) && !S_ISDIR(type)) {
		si->file_attributes = FILE_ATTR_SYSTEM;
		ni->flags = FILE_ATTR_SYSTEM;
	}

	// 将新建的10H属性填入预先找到的Buffer中
	if (ntfs_attr_add(ni, AT_STANDARD_INFORMATION, AT_UNNAMED, 0, (u8*)si, si_len)) 
	{
		err = errno;
		ntfs_log_error("Failed to add STANDARD_INFORMATION attribute.\n");
		goto err_out;
	}
	
	//
	// 创建 & 写入安全描述符属性(50H) SECURITY_DESCRIPTOR
	//

	if (ntfs_sd_add_everyone(ni)) 
	{
		err = errno;
		goto err_out;
	}
	rollback_sd = 1;

	if (S_ISDIR(type)) 
	{
		//
		// 若是创建文件夹,需要新增 INDEX_ROOT(90H)属性
		//

		INDEX_ROOT *ir = NULL;
		INDEX_ENTRY *ie;
		int ir_len, index_len;

		// Create INDEX_ROOT attribute. 
		index_len = sizeof(INDEX_HEADER) + sizeof(INDEX_ENTRY_HEADER);
		ir_len = offsetof(INDEX_ROOT, index) + index_len;
		ir = ntfs_calloc(ir_len);
		if (!ir) {
			err = errno;
			goto err_out;
		}
		ir->type = AT_FILE_NAME;
		ir->collation_rule = COLLATION_FILE_NAME;
		ir->index_block_size = cpu_to_le32(ni->vol->indx_record_size);
		if ( ni->vol->cluster_size <= ni->vol->indx_record_size ) {
			ir->clusters_per_index_block = ni->vol->indx_record_size >> ni->vol->cluster_size_bits;
		} else {
			ir->clusters_per_index_block = ni->vol->indx_record_size >> ni->vol->sector_size_bits;
		}

		ir->index.entries_offset = cpu_to_le32(sizeof(INDEX_HEADER));
		ir->index.index_length = cpu_to_le32(index_len);
		ir->index.allocated_size = cpu_to_le32(index_len);
		ie = (INDEX_ENTRY*)((u8*)ir + sizeof(INDEX_ROOT));
		ie->length = cpu_to_le16(sizeof(INDEX_ENTRY_HEADER));
		ie->key_length = 0;
		ie->ie_flags = INDEX_ENTRY_END;

		// 将新建的 INDEX_ROOT(90H) 属性填入预先找到的Buffer中
		if (ntfs_attr_add(ni, AT_INDEX_ROOT, NTFS_INDEX_I30, 4, (u8*)ir, ir_len))
		{
			err = errno;
			free(ir);
			ntfs_log_error("Failed to add INDEX_ROOT attribute.\n");
			goto err_out;
		}
		free(ir);

	} else {

		//
		// 创建的是文件
		//

		INTX_FILE *data;
		int data_len;

		// 填充80H属性
		switch (type)
		{
			case S_IFBLK:
			case S_IFCHR:
				data_len = offsetof(INTX_FILE, device_end);
				data = (void *)ntfs_malloc(data_len);
				if (!data) {
					err = errno;
					goto err_out;
				}
				data->major = cpu_to_le64(/*major(dev)*/0);
				data->minor = cpu_to_le64(/*minor(dev)*/0);
				if (type == S_IFBLK)
					data->magic = INTX_BLOCK_DEVICE;
				if (type == S_IFCHR)
					data->magic = INTX_CHARACTER_DEVICE;
				break;
			case S_IFLNK:
				data_len = sizeof(INTX_FILE_TYPES) +
						target_len * sizeof(ntfschar);
				data = (INTX_FILE *)ntfs_malloc(data_len);
				if (!data) {
					err = errno;
					goto err_out;
				}
				data->magic = INTX_SYMBOLIC_LINK;
				memcpy(data->target, target,
						target_len * sizeof(ntfschar));
				break;
			case S_IFSOCK:
				data = NULL;
				data_len = 1;
				break;
			default: /* FIFO or regular file. */
				data = NULL;
				data_len = 0;
				break;
		}

		// 将新建的 DATA(80H) 属性填入预先找到的Buffer中
		if (ntfs_attr_add(ni, AT_DATA, AT_UNNAMED, 0, (u8*)data, data_len)) {
			err = errno;
			ntfs_log_error("Failed to add DATA attribute.\n");
			free(data);
			goto err_out;
		}
		rollback_data = 1;
		free(data);
	}

	//
	// 创建 & 写入文件名属性(30H) FILE_NAME
	//

	fn_len = sizeof(FILE_NAME_ATTR) + name_len * sizeof(ntfschar);
	fn = ntfs_calloc(fn_len);
	if (!fn) {
		err = errno;
		goto err_out;
	}
	fn->parent_directory = MK_LE_MREF(dir_ni->mft_no,le16_to_cpu(dir_ni->mrec->sequence_number));
	fn->file_name_length = name_len;
	fn->file_name_type = FILE_NAME_POSIX;
	if (S_ISDIR(type))
		fn->file_attributes = FILE_ATTR_I30_INDEX_PRESENT;
	if (!S_ISREG(type) && !S_ISDIR(type))
		fn->file_attributes = FILE_ATTR_SYSTEM;
	fn->creation_time = utc2ntfs(ni->creation_time);
	fn->last_data_change_time = utc2ntfs(ni->last_data_change_time);
	fn->last_mft_change_time = utc2ntfs(ni->last_mft_change_time);
	fn->last_access_time = utc2ntfs(ni->last_access_time);
	memcpy(fn->file_name, name, name_len * sizeof(ntfschar));

	// 将新建的 FILE_NAME(30H) 属性填入预先找到的Buffer中
	if (ntfs_attr_add(ni, AT_FILE_NAME, AT_UNNAMED, 0, (u8*)fn, fn_len)) {
		err = errno;
		ntfs_log_error("Failed to add FILE_NAME attribute.\n");
		goto err_out;
	}

	// Add FILE_NAME attribute to index.
	if (ntfs_index_add_filename(dir_ni, fn, MK_MREF(ni->mft_no,le16_to_cpu(ni->mrec->sequence_number)))) {
		err = errno;
		ntfs_log_perror("Failed to add entry to the index");
		goto err_out;
	}

	//
	// 填充这个新MFT的2个必要的值: 硬连接数 && 目录标志(0x3).
	//

	ni->mrec->link_count = cpu_to_le16(1);
	if (S_ISDIR(type))
		ni->mrec->flags |= MFT_RECORD_IS_DIRECTORY;
	ntfs_inode_mark_dirty(ni);

	//
	// Done!
	//

	free(fn);
	free(si);
	ntfs_log_trace("Done.\n");
	return ni;
err_out:
	ntfs_log_trace("Failed.\n");

	if (rollback_sd)
		ntfs_attr_remove(ni, AT_SECURITY_DESCRIPTOR, AT_UNNAMED, 0);
	
	if (rollback_data)
		ntfs_attr_remove(ni, AT_DATA, AT_UNNAMED, 0);
	/*
	 * Free extent MFT records (should not exist any with current
	 * ntfs_create implementation, but for any case if something will be
	 * changed in the future).
	 */
	while (ni->nr_extents)
	{
		if (ntfs_mft_record_free(ni->vol, *(ni->extent_nis))) {
			err = errno;
			ntfs_log_error("Failed to free extent MFT record. Leaving inconsistent metadata.\n");
		}
	}

	if (ntfs_mft_record_free(ni->vol, ni)) { ntfs_log_error("Failed to free MFT record. metadata. Run chkdsk.\n"); }
	free(fn);
	free(si);
	errno = err;
	return NULL;
}

/**
 * Some wrappers around __ntfs_create() ...
 */

ntfs_inode *ntfs_create(ntfs_inode *dir_ni, ntfschar *name, u8 name_len, dev_t type)
{
	if (type != S_IFREG && type != S_IFDIR && type != S_IFIFO && type != S_IFSOCK) {
		ntfs_log_error("Invalid arguments.\n");
		return NULL;
	}

	return __ntfs_create(dir_ni, name, name_len, type, 0, NULL, 0);
}



ntfs_inode *ntfs_create_device(ntfs_inode *dir_ni, ntfschar *name, u8 name_len, dev_t type, dev_t dev)
{
	if (type != S_IFCHR && type != S_IFBLK) {
		ntfs_log_error("Invalid arguments.\n");
		return NULL;
	}
	return __ntfs_create(dir_ni, name, name_len, type, dev, NULL, 0);
}



ntfs_inode *ntfs_create_symlink(ntfs_inode *dir_ni, ntfschar *name, u8 name_len, ntfschar *target, u8 target_len)
{
	if (!target || !target_len) {
		ntfs_log_error("Invalid arguments.\n");
		return NULL;
	}
	return __ntfs_create(dir_ni, name, name_len, S_IFLNK, 0,
			target, target_len);
}



int ntfs_check_empty_dir(ntfs_inode *ni)
{
	ntfs_attr *na;
	int ret = 0;
	
	if (!(ni->mrec->flags & MFT_RECORD_IS_DIRECTORY))
		return 0;

	na = ntfs_attr_open(ni, AT_INDEX_ROOT, NTFS_INDEX_I30, 4);
	if (!na) {
		errno = EIO;
		ntfs_log_perror("Failed to open directory");
		return -1;
	}
	
	// Non-empty directory?
	if ((na->data_size != sizeof(INDEX_ROOT) + sizeof(INDEX_ENTRY_HEADER)))
	{
		// Both ENOTEMPTY and EEXIST are ok. We use the more common.
		errno = EEXIST;
		ntfs_log_debug("Directory is not empty\n");
		ret = -1;
	}
	
	ntfs_attr_close(na);
	return ret;
}



static int 
ntfs_check_unlinkable_dir(
	IN ntfs_inode *ni, 
	IN FILE_NAME_ATTR *fn
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/05 [5:3:2009 - 22:57]

Routine Description:
  检查  
    
Arguments:

     - 

Return Value:

    
--*/
{
	int link_count = le16_to_cpu(ni->mrec->link_count);
	int ret;
	
	ret = ntfs_check_empty_dir( ni );
	if (!ret || errno != EEXIST)
		return ret;
	/* 
	 * Directory is non-empty, so we can unlink only if there is more than
	 * one "real" hard link, i.e. links aren't different DOS and WIN32 names
	 */
	if ((link_count == 1) || 
	    (link_count == 2 && fn->file_name_type == FILE_NAME_DOS)) {
		errno = EEXIST;
		ntfs_log_debug("Non-empty directory without hard links\n");
		goto no_hardlink;
	}
	
	ret = 0;
no_hardlink:	
	return ret;
}



int 
ntfs_delete(
	IN ntfs_inode *ni, 
	IN ntfs_inode *dir_ni, 
	IN ntfschar *name, 
	IN u8 name_len
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/05 [5:3:2009 - 21:58]

Routine Description:
  在ntfs分区上删除指定文件/文件夹  
    
Arguments:
  ni - 要删除的文件的节点
  dir_ni - 要删除的文件的父节点
  name - unicode name of the object to delete
  name_len- length of the name in unicode characters

Return Value:
  0 -- 成功; -1 -- 失败
    
--*/
{
	ntfs_attr_search_ctx *actx = NULL;
	ntfs_index_context *ictx = NULL;
	FILE_NAME_ATTR *fn = NULL;
	BOOL looking_for_dos_name = FALSE, looking_for_win32_name = FALSE;
	BOOL case_sensitive_match = TRUE;
	int err = 0;

	ntfs_log_trace("Entering.\n");
	
	if (!ni || !dir_ni || !name || !name_len) { // 参数合法性检测
		ntfs_log_error("Invalid arguments.\n");
		errno = EINVAL;
		goto err_out;
	}

	if (ni->nr_extents == -1) { ni = ni->base_ni; }
	if (dir_ni->nr_extents == -1) { dir_ni = dir_ni->base_ni; }

	/*
	 * Search for FILE_NAME attribute with such name. If it's in POSIX or
	 * WIN32_AND_DOS namespace, then simply remove it from index and inode.
	 * If filename in DOS or in WIN32 namespace, then remove DOS name first,
	 * only then remove WIN32 name.
	 */
	actx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!actx) { goto err_out; }

search:
	while ( !ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0, CASE_SENSITIVE,0, NULL, 0, actx) ) 
	{
		char *s;
		BOOL case_sensitive = IGNORE_CASE;

		errno = 0;
		fn = (FILE_NAME_ATTR*)((u8*)actx->attr + le16_to_cpu(actx->attr->X.value_offset));
		s = ntfs_attr_name_get( fn->file_name, fn->file_name_length );
		ntfs_log_trace(
			"name: '%s'  type: %d  dos: %d  win32: %d case: %d\n", 
			s, fn->file_name_type, looking_for_dos_name, 
			looking_for_win32_name, case_sensitive_match
			);

		ntfs_attr_name_free( &s );
		if ( looking_for_dos_name ) 
		{
			if ( fn->file_name_type == FILE_NAME_DOS )
				break;
			else
				continue;
		}

		if ( looking_for_win32_name ) 
		{
			if ( fn->file_name_type == FILE_NAME_WIN32 )
				break;
			else
				continue;
		}
		
		// Ignore hard links from other directories
		if ( dir_ni->mft_no != MREF_LE(fn->parent_directory) ) 
		{
			ntfs_log_debug("MFT record numbers don't match (%llu != %llu)\n", dir_ni->mft_no,  MREF_LE(fn->parent_directory));
			continue;
		}
		     
		if (fn->file_name_type == FILE_NAME_POSIX || case_sensitive_match)
			case_sensitive = CASE_SENSITIVE;
		
		if ( ntfs_names_are_equal(fn->file_name, fn->file_name_length,
					 name, name_len, case_sensitive, 
					 ni->vol->upcase, ni->vol->upcase_len)
		   )
		{
			// 找到了,进一步检查是否是win32类型的名字
			if (fn->file_name_type == FILE_NAME_WIN32) 
			{
				looking_for_dos_name = TRUE;
				ntfs_attr_reinit_search_ctx( actx );
				continue;
			}

			if (fn->file_name_type == FILE_NAME_DOS) { looking_for_dos_name = TRUE; }
			break;
		}
	} // end-of-while

	if ( errno ) 
	{
		// 若大小写敏感的查找失败了. 则忽略大小写再次进行查找
		if ( errno == ENOENT && case_sensitive_match ) 
		{
			case_sensitive_match = FALSE;
			ntfs_attr_reinit_search_ctx( actx );
			goto search;
		}

		goto err_out;
	}
	
	if ( ntfs_check_unlinkable_dir( ni, fn ) < 0 ) { goto err_out; }
		
	ictx = ntfs_index_ctx_get( dir_ni, NTFS_INDEX_I30, 4 );
	if (!ictx) { goto err_out; }
	
	if ( ntfs_index_lookup( fn, le32_to_cpu(actx->attr->X.value_length), ictx ) ) { goto err_out; }
	
	if ( ((FILE_NAME_ATTR*)ictx->data)->file_attributes & FILE_ATTR_REPARSE_POINT ) 
	{
	//	errno = EOPNOTSUPP;
		goto err_out;
	}
	
	//
	// 开始删除
	//

	if (ntfs_index_rm(ictx)) { // 1. 删除父目录中记录该文件的INDEX_BLOCK [写入磁盘]
		goto err_out; 
	} 

	// 这里操作的是自己申请的内存.填充完毕后该MFT后再写入磁盘
	if (ntfs_attr_record_rm(actx)) { // 2.1 删除该文件MFT中的3OH属性 [写入内存]
		goto err_out; 
	}
	
	// 递减 硬连接数
	ni->mrec->link_count = cpu_to_le16(le16_to_cpu(ni->mrec->link_count) - 1);
	
	ntfs_inode_mark_dirty(ni);
	if (looking_for_dos_name)
	{
		looking_for_dos_name = FALSE;
		looking_for_win32_name = TRUE;
		ntfs_attr_reinit_search_ctx(actx);
		goto search;
	}

	/* TODO: Update object id, quota and securiry indexes if required. */
	/*
	 * If hard link count is not equal to zero then we are done. In other
	 * case there are no reference to this inode left, so we should free all
	 * non-resident attributes and mark all MFT record as not in use.
	 */
	if (ni->mrec->link_count)
		goto out;
	ntfs_attr_reinit_search_ctx(actx);

	//
	// 2.2 清理掉该文件MFT中的非常驻属性
	//

	while (!ntfs_attrs_walk(actx))
	{
		if (actx->attr->non_resident) 
		{
			runlist *rl;

			// 得到该非常驻属性的LCN
			rl = ntfs_mapping_pairs_decompress(ni->vol, actx->attr,NULL);
			if (!rl) {
				err = errno;
				ntfs_log_error("Failed to decompress runlist.Leaving inconsistent metadata.\n");
				continue;
			}

			// 清理之 [写入磁盘,仅仅在$bitmap中将这些run对应的簇标记为0]
			if ( ntfs_cluster_free_from_rl(ni->vol, rl) ) {
				err = errno;
				ntfs_log_error("Failed to free clusters.  Leaving inconsistent metadata.\n");
				continue;
			}

			free(rl);
		}
	}

	if (errno != ENOENT) {
		err = errno;
		ntfs_log_error("Attribute enumeration failed.Probably leaving inconsistent metadata.\n");
	}

	// All extents should be attached after attribute walk.
	while (ni->nr_extents) 
	{
		if (ntfs_mft_record_free(ni->vol, *(ni->extent_nis))) 
		{
			err = errno;
			ntfs_log_error("Failed to free extent MFT record. Leaving inconsistent metadata.\n");
		}
	}

	//
	// 3. 填充完毕,把修改后的数据真正的写入磁盘
	//

	if (ntfs_mft_record_free(ni->vol, ni)) 
	{
		err = errno;
		ntfs_log_error("Failed to free base MFT record. Leaving inconsistent metadata.\n");
	}
	ni = NULL;

	// 收尾工作
out:
	if (actx)
		ntfs_attr_put_search_ctx(actx);

//	if (ictx) { ntfs_index_ctx_put(ictx); }

	if (ni)
		ntfs_inode_close(ni);

	if (err) {
		errno = err;
		ntfs_log_debug("Could not delete file: %s\n", strerror(errno));
		return -1;
	}

	ntfs_log_trace("Done.\n");
	return 0;
err_out:
	err = errno;
	goto out;
}



/**
 * ntfs_link - create hard link for file or directory
 * @ni:		ntfs inode for object to create hard link
 * @dir_ni:	ntfs inode for directory in which new link should be placed
 * @name:	unicode name of the new link
 * @name_len:	length of the name in unicode characters
 *
 * NOTE: At present we allow creating hardlinks to directories, we use them
 * in a temporary state during rename. But it's defenitely bad idea to have
 * hard links to directories as a result of operation.
 * FIXME: Create internal  __ntfs_link that allows hard links to a directories
 * and external ntfs_link that do not. Write ntfs_rename that uses __ntfs_link.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
int ntfs_link(ntfs_inode *ni, ntfs_inode *dir_ni, ntfschar *name, u8 name_len)
{
	FILE_NAME_ATTR *fn = NULL;
	int fn_len, err;

	ntfs_log_trace("Entering.\n");
	
	if (!ni || !dir_ni || !name || !name_len || ni->mft_no == dir_ni->mft_no) 
	{
		err = EINVAL;
		ntfs_log_perror("ntfs_link wrong arguments");
		goto err_out;
	}

	/* Create FILE_NAME attribute. */
	fn_len = sizeof(FILE_NAME_ATTR) + name_len * sizeof(ntfschar);
	fn = ntfs_calloc(fn_len);
	if (!fn) {
		err = errno;
		goto err_out;
	}
	fn->parent_directory = MK_LE_MREF(dir_ni->mft_no,le16_to_cpu(dir_ni->mrec->sequence_number));
	fn->file_name_length = name_len;
	fn->file_name_type = FILE_NAME_POSIX;
	fn->file_attributes = ni->flags;
	if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY) { fn->file_attributes |= FILE_ATTR_I30_INDEX_PRESENT; }
	fn->allocated_size = cpu_to_sle64(ni->allocated_size);
	fn->data_size = cpu_to_sle64(ni->data_size);
	fn->creation_time = utc2ntfs(ni->creation_time);
	fn->last_data_change_time = utc2ntfs(ni->last_data_change_time);
	fn->last_mft_change_time = utc2ntfs(ni->last_mft_change_time);
	fn->last_access_time = utc2ntfs(ni->last_access_time);
	memcpy(fn->file_name, name, name_len * sizeof(ntfschar));

	// Add FILE_NAME attribute to index.
	if (ntfs_index_add_filename(dir_ni, fn, MK_MREF(ni->mft_no,le16_to_cpu(ni->mrec->sequence_number)))) 
	{
		err = errno;
		ntfs_log_perror("Failed to add filename to the index");
		goto err_out;
	}

	// Add FILE_NAME attribute to inode.
	if (ntfs_attr_add(ni, AT_FILE_NAME, AT_UNNAMED, 0, (u8*)fn, fn_len)) {
		ntfs_index_context *ictx;

		err = errno;
		ntfs_log_error("Failed to add FILE_NAME attribute.\n");

		/* Try to remove just added attribute from index. */
		ictx = ntfs_index_ctx_get(dir_ni, NTFS_INDEX_I30, 4);
		if (!ictx)
			goto rollback_failed;

		if (ntfs_index_lookup(fn, fn_len, ictx)) {
			ntfs_index_ctx_put(ictx);
			goto rollback_failed;
		}

		if (ntfs_index_rm(ictx)) {
			ntfs_index_ctx_put(ictx);
			goto rollback_failed;
		}

		goto err_out;
	}

	// Increment hard links count.
	ni->mrec->link_count = cpu_to_le16(le16_to_cpu(ni->mrec->link_count) + 1);

	//
	// Done! 
	//

	ntfs_inode_mark_dirty(ni);
	free(fn);
	ntfs_log_trace("Done.\n");
	return 0;
rollback_failed:
	ntfs_log_error("Rollback failed. Leaving inconsistent metadata.\n");
err_out:
	free(fn);
	errno = err;
	ntfs_log_perror("Hard link failed");
	return -1;
}

