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
 * volume.c - NTFS volume handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2006 Anton Altaparmakov
 * Copyright (c) 2002-2006 Szabolcs Szakacsits
 * Copyright (c) 2004-2005 Richard Russon
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "volume.h"
#include "attrib.h"
#include "mft.h"
#include "bootsect.h"
#include "device.h"
#include "debug.h"
#include "inode.h"
#include "runlist.h"
#include "logfile.h"
#include "dir.h"
#include "logging.h"
#include "misc.h"

#include "win32_io.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#pragma warning( disable: 4244 4129 )

//////////////////////////////////////////////////////////////////////////


ntfs_volume *
ntfs_volume_alloc(void)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 14:06]

Routine Description:
  Create an NTFS volume object and initialise it  
    
--*/
{
	return calloc( 1, sizeof(ntfs_volume) );
}

/**
 * __ntfs_volume_release - Destroy an NTFS volume object
 * @v:
 *
 * Description...
 *
 * Returns:
 */
static void __ntfs_volume_release(ntfs_volume *v)
{
	if (v->lcnbmp_ni && NInoDirty(v->lcnbmp_ni))
		ntfs_inode_sync(v->lcnbmp_ni);
	if (v->vol_ni)
		ntfs_inode_close(v->vol_ni);
	if (v->lcnbmp_na)
		ntfs_attr_close(v->lcnbmp_na);
	if (v->lcnbmp_ni)
		ntfs_inode_close(v->lcnbmp_ni);
	if (v->mft_ni && NInoDirty(v->mft_ni))
		ntfs_inode_sync(v->mft_ni);
	if (v->mftbmp_na)
		ntfs_attr_close(v->mftbmp_na);
	if (v->mft_na)
		ntfs_attr_close(v->mft_na);
	if (v->mft_ni)
		ntfs_inode_close(v->mft_ni);
	if (v->mftmirr_ni && NInoDirty(v->mftmirr_ni))
		ntfs_inode_sync(v->mftmirr_ni);
	if (v->mftmirr_na)
		ntfs_attr_close(v->mftmirr_na);
	if (v->mftmirr_ni)
		ntfs_inode_close(v->mftmirr_ni);
	if (v->dev) {
		struct ntfs_device *dev = v->dev;

		dev->d_ops->sync(dev);
		if (dev->d_ops->close(dev))
			ntfs_log_perror("Failed to close the device");
	}
	free(v->vol_name);
	free(v->upcase);
	free(v->attrdef);
	free(v);
}



static void 
ntfs_attr_setup_flag(
	IN ntfs_inode *ni
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 16:35]

Routine Description:
  读取inode节点包含的0x10这个标准属性,将其file_attributes赋予给该节点的flags
    
--*/
{
	STANDARD_INFORMATION *si;

	si = ntfs_attr_readall(ni, AT_STANDARD_INFORMATION, AT_UNNAMED, 0, NULL);
	if (si) {
		ni->flags = si->file_attributes;
		free(si);
	}

	return ;
}



int 
ntfs_mft_load(
	IN ntfs_volume *vol
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 16:04]

Routine Description:
  Load $MFT from @vol and setup @vol with it. After calling this function the
  volume @vol is ready for use by all read access functions provided by the ntfs library.  

Return Value:
  0 -- 成功; -1 -- 错误
    
--*/
{
	VCN next_vcn, last_vcn, highest_vcn;
	s64 l;
	MFT_RECORD *mb = NULL;
	ntfs_attr_search_ctx *ctx = NULL;
	ATTR_RECORD *a;
	int eo;

	vol->mft_ni = ntfs_inode_allocate(vol);
	mb = (MFT_RECORD *) ntfs_malloc(vol->mft_record_size);
	if (!vol->mft_ni || !mb) {
		ntfs_log_perror("Error allocating memory for $MFT");
		goto error_exit;
	}

	vol->mft_ni->mft_no = 0;
	vol->mft_ni->mrec = mb;

	// 根据该卷元数据$MFT的LCN,得到其内容
	l = ntfs_mst_pread(vol->dev, vol->mft_lcn << vol->cluster_size_bits, 1,vol->mft_record_size, mb);
	if (l != 1) {
		if (l != -1) errno = EIO;
		ntfs_log_perror("Error reading $MFT");
		goto error_exit;
	}

	// 校验
	if (ntfs_is_baad_record(mb->magic)) {
		ntfs_log_error("Incomplete multi sector transfer detected in $MFT.\n");
		goto io_error_exit;
	}
	if (!ntfs_is_mft_record(mb->magic)) {
		ntfs_log_error("$MFT has invalid magic.\n");
		goto io_error_exit;
	}

	// 分配 & 初始化 搜索结构体
	ctx = ntfs_attr_get_search_ctx(vol->mft_ni, NULL);
	if (!ctx) {
		ntfs_log_perror("Failed to allocate attribute search context");
		goto error_exit;
	}

	if (p2n(ctx->attr) < p2n(mb) 
		|| (char*)ctx->attr > (char*)mb + vol->mft_record_size
	   ) 
	{
		ntfs_log_error("$MFT is corrupt.\n");
		goto io_error_exit;
	}

	// 在元数据$MFT内找到$ATTRIBUTE_LIST属性
	if ( ntfs_attr_lookup(AT_ATTRIBUTE_LIST, AT_UNNAMED, 0, 0, 0, NULL, 0, ctx) ) 
	{
		if (errno != ENOENT) {
			ntfs_log_error("$MFT has corrupt attribute list.\n");
			goto io_error_exit;
		}

		goto mft_has_no_attr_list;
	}

	NInoSetAttrList( vol->mft_ni ); // 设置该节点有0x20属性

	// 将该0x20属性的内容拷贝一份至vol结构体中
	l = ntfs_get_attribute_value_length( (ATTR_RECORD *)ctx->attr );
	if (l <= 0 || l > 0x40000) {
		ntfs_log_error("$MFT/$ATTR_LIST invalid length (%lld).\n",(LONG64)l);
		goto io_error_exit;
	}

	vol->mft_ni->attr_list_size = l;
	vol->mft_ni->attr_list = (u8 *) ntfs_malloc((size_t)l);
	if (!vol->mft_ni->attr_list)
		goto error_exit;
	
	l = ntfs_get_attribute_value(vol, ctx->attr, vol->mft_ni->attr_list);
	if (!l) {
		ntfs_log_error("Failed to get value of $MFT/$ATTR_LIST.\n");
		goto io_error_exit;
	}

	if (l != vol->mft_ni->attr_list_size) {
		ntfs_log_error("Partial read of $MFT/$ATTR_LIST (%lld != %u).\n", (LONG64)l, vol->mft_ni->attr_list_size);
		goto io_error_exit;
	}

mft_has_no_attr_list:

	ntfs_attr_setup_flag( vol->mft_ni ); // 将0x10标准属性中的flag赋予inode结构体
		
	// Get an ntfs attribute for $MFT/$DATA and set it up, too.
	vol->mft_na = ntfs_attr_open(vol->mft_ni, AT_DATA, AT_UNNAMED, 0);
	if (!vol->mft_na) {
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}

	// 读取元数据$MFT内的0x80属性的所有扩展属性(run list)
	ntfs_attr_reinit_search_ctx( ctx );
	last_vcn = vol->mft_na->allocated_size >> vol->cluster_size_bits;
	highest_vcn = next_vcn = 0;
	a = NULL;

	//
	// 读取0x80属性的数据运行,记录到vol->mft_na->rl中
	//
	while ( !ntfs_attr_lookup( AT_DATA, AT_UNNAMED, 0, 0, next_vcn, NULL, 0,ctx ) ) 
	{
		runlist_element *nrl;

		a = ctx->attr;
		
		if ( !a->non_resident ) { // $MFT must be non-resident.
			ntfs_log_error("$MFT must be non-resident.\n");
			goto io_error_exit;
		}

		// $MFT must be uncompressed and unencrypted.
		if (a->flags & ATTR_COMPRESSION_MASK || a->flags & ATTR_IS_ENCRYPTED) 
		{
			ntfs_log_error("$MFT must be uncompressed and unencrypted.\n");
			goto io_error_exit;
		}

		// 每次decompress时,便把旧的合并到新的中去了,所以到最后就剩下一个总的vol->mft_na->rl啦
		nrl = ntfs_mapping_pairs_decompress( vol, a, vol->mft_na->rl );
		if (!nrl) {
			ntfs_log_perror("ntfs_mapping_pairs_decompress() failed");
			goto error_exit;
		}
		vol->mft_na->rl = nrl;

		//
		// 0x80属性的数据运行有很多块, 每一块都用VCN来标记它占了多大地方.
		// 那么highest_vcn+1 就是下一个块的起始VCN啦.
		//
		highest_vcn = /**(VCN*)( (char*)a + 24 )*/ sle64_to_cpu(a->X.highest_vcn);
		next_vcn = highest_vcn + 1;

		/* Only one extent or error, which we catch below. */
		if ( next_vcn <= 0 ) { break; }

		/* Avoid endless loops due to corruption. */
		if ( next_vcn < *(VCN*)( (char*)a + 16 ) /*sle64_to_cpu(a->lowest_vcn)*/ ) {
			ntfs_log_error("$MFT has corrupt attribute list.\n");
			goto io_error_exit;
		}
	} // end-of-while

	if (!a) {
		ntfs_log_error("$MFT/$DATA attribute not found.\n");
		goto io_error_exit;
	}

	if ( highest_vcn && highest_vcn != last_vcn - 1 ) 
	{
		ntfs_log_error("Failed to load runlist for $MFT/$DATA.\n");
		ntfs_log_error("highest_vcn = 0x%llx, last_vcn - 1 = 0x%llx\n",(LONG64)highest_vcn, (LONG64)last_vcn - 1);
		goto io_error_exit;
	}

	ntfs_attr_put_search_ctx(ctx); // 释放内存
	ctx = NULL;
	
	vol->mftbmp_na = ntfs_attr_open(vol->mft_ni, AT_BITMAP, AT_UNNAMED, 0);
	if (!vol->mftbmp_na) {
		ntfs_log_perror("Failed to open $MFT/$BITMAP");
		goto error_exit;
	}

	return 0;

io_error_exit:
	errno = EIO;
error_exit:
	eo = errno;
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (vol->mft_na) {
		ntfs_attr_close(vol->mft_na);
		vol->mft_na = NULL;
	}
	if (vol->mft_ni) {
		ntfs_inode_close(vol->mft_ni);
		vol->mft_ni = NULL;
	}
	errno = eo;
	return -1;
}



static int
ntfs_mftmirr_load(
	IN ntfs_volume *vol
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 17:07]

Routine Description:
  Load $MFTMirr from @vol and setup @vol with it. After calling this function
  the volume @vol is ready for use by all write access functions provided by
  the ntfs library (assuming ntfs_mft_load() has been called successfully beforehand).  
    
Return Value:
  Return 0 on success and -1 on error with errno set to the error code.
    
--*/
{
	int err;

	// 打开元数据$MFTMirr,关联到相应的inode节点
	vol->mftmirr_ni = ntfs_inode_open(vol, FILE_MFTMirr);
	if (!vol->mftmirr_ni) {
		ntfs_log_perror("Failed to open inode $MFTMirr");
		return -1;
	}
	
	// 打开元数据$MFTMirr包含的80H属性
	vol->mftmirr_na = ntfs_attr_open(vol->mftmirr_ni, AT_DATA, AT_UNNAMED, 0);
	if (!vol->mftmirr_na) {
		ntfs_log_perror("Failed to open $MFTMirr/$DATA");
		goto error_exit;
	}
	
	//
	if ( ntfs_attr_map_runlist( vol->mftmirr_na, 0 ) < 0 ) {
		ntfs_log_perror("Failed to map runlist of $MFTMirr/$DATA");
		goto error_exit;
	}
	
	return 0;

	// 收尾工作
error_exit:
	err = errno;
	if (vol->mftmirr_na) {
		ntfs_attr_close(vol->mftmirr_na);
		vol->mftmirr_na = NULL;
	}
	ntfs_inode_close(vol->mftmirr_ni);
	vol->mftmirr_ni = NULL;
	errno = err;
	return -1;
}



ntfs_volume *
ntfs_volume_startup(
	IN struct ntfs_device *dev, 
	IN unsigned long flags
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 14:02]

Routine Description:
  allocate and setup an ntfs volume.
  Load, verify, and parse bootsector; load and setup $MFT and $MFTMirr. After
  calling this function, the volume is setup sufficiently to call all read
  and write access functions provided by the library.
    
Arguments:
  dev - device to open
  flags - optional mount flags

Return Value:

    
--*/
{
	LCN mft_zone_size, mft_lcn;
	s64 br;
	ntfs_volume *vol;
	NTFS_BOOT_SECTOR *bs;
	int eo;
#ifdef DEBUG
	const char *OK = "OK\n";
	const char *FAILED = "FAILED\n";
#endif

	if (!dev || !dev->d_ops || !dev->d_name) { // 参数合法性检测
		errno = EINVAL;
		return NULL;
	}

	// 分配内存,存放MBR
	bs = (NTFS_BOOT_SECTOR *) ntfs_malloc( 0x1000 );
	if (!bs) { return NULL; }
	
	// 分配 ntfs_volume 结构体大小的内存
	vol = ntfs_volume_alloc();
	if (!vol) { goto error_exit; }

	// 创建大小写转换表
	vol->upcase_len = 65536;
	vol->upcase = (ntfschar *) ntfs_malloc(vol->upcase_len * sizeof(ntfschar));
	if ( !vol->upcase ) { goto error_exit; }
	ntfs_upcase_table_build( vol->upcase, vol->upcase_len * sizeof(ntfschar) );

	// 设置该卷的状态 (只读/不必更新访问时间)
	if (flags & MS_RDONLY) { NVolSetReadOnly(vol); }
	if (flags & MS_NOATIME) { NVolSetNoATime(vol); }

	// 读取bootsector的内容
	ntfs_log_debug("Reading bootsector... ");

	// 这个open函数是unix下的,需要替换成windows下的API
	if ( dev->d_ops->open( dev, NVolReadOnly(vol) ? 0: 2 ) ) 
//	if ( ntfs_device_win32_open( dev, NVolReadOnly(vol) ? 0: 2 ) )
	{
		ntfs_log_perror("Error opening partition device");
		goto error_exit;
	}

	vol->dev = dev;

	// 从0开始,读取MBR的内容到指针bs中
	br = ntfs_pread( dev, 0, sizeof(NTFS_BOOT_SECTOR), bs );
	if (br == 0) 
	{
		if (br != -1) errno = EINVAL;
		if (!br) ntfs_log_error("Partition is smaller than bootsector size.\n");
		else ntfs_log_perror("Error reading bootsector");
		goto error_exit;
	}

	// 校验NTFS的合法性
	if (!ntfs_boot_sector_is_ntfs(bs)) {
		errno = EINVAL;
		goto error_exit;
	}

	// 解析MBR & BPB,存储一些重要参数到vol结构体中
	if (ntfs_boot_sector_parse(vol, bs) < 0) { goto error_exit; }
	
	free(bs);
	bs = NULL;

	// Now set the device block size to the sector size.
	if ( ntfs_device_block_size_set( vol->dev, vol->sector_size ) ) 
	{
		ntfs_log_debug(
			"Failed to set the device block size to the sector size. This may affect performance "
			"but should be harmless otherwise.  Error: %s\n", strerror(errno)
			);
	}

	// TODO: Make this tunable at mount time. (AIA)
	vol->mft_zone_multiplier = 1;

	// Determine the size of the MFT zone.
	mft_zone_size = vol->nr_clusters;
	switch (vol->mft_zone_multiplier) {  // % of volume size in clusters
	case 4:
		mft_zone_size >>= 1;			 // 50%
		break;
	case 3:
		mft_zone_size = mft_zone_size * 3 >> 3;	// 37.5%
		break;
	case 2:
		mft_zone_size >>= 2;			// 25%
		break;
	/* case 1: */
	default:
		mft_zone_size >>= 3;			// 12.5%
		break;
	}

	// Setup the mft zone.
	vol->mft_zone_start = vol->mft_zone_pos = vol->mft_lcn;
	ntfs_log_debug("mft_zone_pos = 0x%llx\n", (LONG64)vol->mft_zone_pos);

	/*
	 * Calculate the mft_lcn for an unmodified NTFS volume (see mkntfs
	 * source) and if the actual mft_lcn is in the expected place or even
	 * further to the front of the volume, extend the mft_zone to cover the
	 * beginning of the volume as well. This is in order to protect the
	 * area reserved for the mft bitmap as well within the mft_zone itself.
	 * On non-standard volumes we don't protect it as the overhead would be
	 * higher than the speed increase we would get by doing it.
	 */
	mft_lcn = (8192 + 2 * vol->cluster_size - 1) / vol->cluster_size;
	if (mft_lcn * vol->cluster_size < 16 * 1024) {
		mft_lcn = (16 * 1024 + vol->cluster_size - 1) / vol->cluster_size;
	}

	if (vol->mft_zone_start <= mft_lcn) { vol->mft_zone_start = 0; }
	ntfs_log_debug("mft_zone_start = 0x%llx\n", (LONG64)vol->mft_zone_start);

	/*
	 * Need to cap the mft zone on non-standard volumes so that it does
	 * not point outside the boundaries of the volume. We do this by
	 * halving the zone size until we are inside the volume.
	 */
	vol->mft_zone_end = vol->mft_lcn + mft_zone_size;
	while (vol->mft_zone_end >= vol->nr_clusters) {
		mft_zone_size >>= 1;
		vol->mft_zone_end = vol->mft_lcn + mft_zone_size;
	}
	ntfs_log_debug("mft_zone_end = 0x%llx\n", (LONG64)vol->mft_zone_end);

	/*
	 * Set the current position within each data zone to the start of the
	 * respective zone.
	 */
	vol->data1_zone_pos = vol->mft_zone_end;
	ntfs_log_debug("data1_zone_pos = 0x%llx\n", vol->data1_zone_pos);
	vol->data2_zone_pos = 0;
	ntfs_log_debug("data2_zone_pos = 0x%llx\n", vol->data2_zone_pos);

	// 前16个是可使用的元数据,到第23个之间是保留的元数据
	vol->mft_data_pos = 24;

	// Need to setup $MFT so we can use the library read functions.
	// 分析元数据$MFT,将其中的一些信息填充到vol结构体中
	ntfs_log_debug("Loading $MFT... ");
	if ( ntfs_mft_load( vol ) < 0 ) {
		ntfs_log_perror("Failed to load $MFT");
		goto error_exit;
	}

	// Need to setup $MFTMirr so we can use the write functions, too. 
	// 分析元数据$MFTMirr,将其中的一些信息填充到vol结构体中
	ntfs_log_debug("Loading $MFTMirr... ");
	if ( ntfs_mftmirr_load( vol ) < 0 ) {
		ntfs_log_perror("Failed to load $MFTMirr");
		goto error_exit;
	}

	return vol;

	// 收尾工作
error_exit:
	eo = errno;
	free(bs);
	if (vol)
		__ntfs_volume_release(vol);
	errno = eo;
	return NULL;
}



static int 
ntfs_volume_check_logfile(
	IN ntfs_volume *vol
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 20:08]

Routine Description:
  检查在这个卷上的logfile  
    
Return Value:
  Return 0 on success and -1 on error with errno set error code.
    
--*/
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	RESTART_PAGE_HEADER *rp = NULL;
	int err = 0;

	// 打开$FILE_LogFile,得到对应的inode节点
	if ((ni = ntfs_inode_open(vol, FILE_LogFile)) == NULL) {
		ntfs_log_perror("Failed to open inode FILE_LogFile");
		errno = EIO;
		return -1;
	}

	// 得到其80H属性
	if ((na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0)) == NULL) {
		ntfs_log_perror("Failed to open $FILE_LogFile/$DATA");
		err = EIO;
		goto exit;
	}

	if (!ntfs_check_logfile(na, &rp) || !ntfs_is_logfile_clean(na, rp)) { err = 1045; }
	free(rp);

exit:
	if (na) { ntfs_attr_close( na ); }
	ntfs_inode_close( ni );
	if (err) {
		errno = err;
		return -1;
	}

	return 0;
}

/**
 * ntfs_hiberfile_open - Find and open '/hiberfil.sys'
 * @vol:    An ntfs volume obtained from ntfs_mount
 *
 * Return:  inode  Success, hiberfil.sys is valid
 *	    NULL   hiberfil.sys doesn't exist or some other error occurred
 */
static ntfs_inode *ntfs_hiberfile_open(ntfs_volume *vol)
{
	u64 inode;
	ntfs_inode *ni_root;
	ntfs_inode *ni_hibr = NULL;
	ntfschar   *unicode = NULL;
	int unicode_len;
	const char *hiberfile = "hiberfil.sys";

	if (!vol) {
		errno = EINVAL;
		return NULL;
	}

	ni_root = ntfs_inode_open(vol, FILE_root);
	if (!ni_root) {
		ntfs_log_debug("Couldn't open the root directory.\n");
		return NULL;
	}

	unicode_len = ntfs_mbstoucs(hiberfile, &unicode, 0);
	if (unicode_len < 0) {
		ntfs_log_perror("Couldn't convert 'hiberfil.sys' to Unicode");
		goto out;
	}

	inode = ntfs_inode_lookup_by_name(ni_root, unicode, unicode_len);
	if (inode == (u64)-1) {
		ntfs_log_debug("Couldn't find file '%s'.\n", hiberfile);
		goto out;
	}

	inode = MREF(inode);
	ni_hibr = ntfs_inode_open(vol, inode);
	if (!ni_hibr) {
		ntfs_log_debug("Couldn't open inode %lld.\n", (LONG64)inode);
		goto out;
	}
out:
	ntfs_inode_close(ni_root);
	free(unicode);
	return ni_hibr;
}


#define NTFS_HIBERFILE_HEADER_SIZE	4096

/**
 * ntfs_volume_check_hiberfile - check hiberfil.sys whether Windows is
 *                               hibernated on the target volume
 * @vol:    volume on which to check hiberfil.sys
 *
 * Return:  0 if Windows isn't hibernated for sure
 *         -1 otherwise and errno is set to the appropriate value
 */
static int ntfs_volume_check_hiberfile(ntfs_volume *vol)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	int i, bytes_read, ret = -1;
	char *buf = NULL;

	ni = ntfs_hiberfile_open(vol);
	if (!ni) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}

	buf = (char *) ntfs_malloc(NTFS_HIBERFILE_HEADER_SIZE);
	if (!buf)
		goto out;

	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_perror("Failed to open hiberfil.sys data attribute");
		goto out;
	}

	bytes_read = ntfs_attr_pread(na, 0, NTFS_HIBERFILE_HEADER_SIZE, buf);
	if (bytes_read == -1) {
		ntfs_log_perror("Failed to read hiberfil.sys");
		goto out;
	}
	if (bytes_read < NTFS_HIBERFILE_HEADER_SIZE) {
		ntfs_log_error("Hibernated non-system partition, refused to "
			       "mount.\n");
		errno = EPERM;
		goto out;
	}
	if (memcmp(buf, "hibr", 4) == 0) {
		ntfs_log_error("Windows is hibernated, refused to mount.\n");
		errno = EPERM;
		goto out;
	}
	for (i = 0; i < NTFS_HIBERFILE_HEADER_SIZE; i++) {
		if (buf[i]) {
			ntfs_log_error("Windows is hibernated, won't mount.\n");
			errno = EPERM;
			goto out;
		}
	}
        /* All right, all header bytes are zero */
	ret = 0;
out:
	if (na)
		ntfs_attr_close(na);
	free(buf);
	ntfs_inode_close(ni);
	return ret;
}



ntfs_volume *
ntfs_device_mount(
	IN struct ntfs_device *dev, 
	IN unsigned long flags
	)
/*++

Routine Description:
  open ntfs volume. 
  Currently only the following flags are implemented:
    MS_RDONLY	- mount volume read-only
   	MS_NOATIME	- do not update access time
   
  The function opens the device @dev and verifies that it contains a valid
  bootsector. Then, it allocates an ntfs_volume structure and initializes
  some of the values inside the structure from the information stored in the
  bootsector. It proceeds to load the necessary system files and completes
  setting up the structure.

--*/
{
	s64 l;
#ifdef DEBUG
	const char *OK = "OK\n";
	const char *FAILED = "FAILED\n";
#endif	
	ntfs_volume *vol;
	u8 *m = NULL, *m2 = NULL;
	ntfs_attr_search_ctx *ctx = NULL;
	ntfs_inode *ni;
	ntfs_attr *na;
	ATTR_RECORD *a;
	VOLUME_INFORMATION *vinf;
	ntfschar *vname;
	int i, j, eo;
	u32 u;

	// 这个函数做了很多事情...
	vol = ntfs_volume_startup(dev, flags);
	if (!vol) {
		ntfs_log_perror("Failed to startup volume");
		return NULL;
	}

	//////////////////////////
	//                      //
	//   校验$MFT的完整性   //
	//                      //
	//////////////////////////

	// Load data from $MFT and $MFTMirr and compare the contents.
	m  = (u8 *) ntfs_malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	m2 = (u8 *) ntfs_malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	if (!m || !m2) { goto error_exit; }

	l = ntfs_attr_mst_pread( vol->mft_na, 0, vol->mftmirr_size, vol->mft_record_size, m );
	if (l != vol->mftmirr_size) 
	{
		if (l == -1) { 
			ntfs_log_perror("Failed to read $MFT");
		} else {
			ntfs_log_error("Failed to read $MFT, unexpected length (%lld != %d).\n", (LONG64)l, vol->mftmirr_size);
			errno = EIO;
		}

		goto error_exit;
	}

	l = ntfs_attr_mst_pread( vol->mftmirr_na, 0, vol->mftmirr_size, vol->mft_record_size, m2);
	if (l != vol->mftmirr_size) 
	{
		if (l == 4) {
			vol->mftmirr_size = 4;
		} else {

			if (l == -1) {
				ntfs_log_perror("Failed to read $MFTMirr");
			} else {
				ntfs_log_error(
					"Failed to read $MFTMirr unexpected length (%d != %lld).\n",
					vol->mftmirr_size, (LONG64)l
					);
				errno = EIO;
			}

			goto error_exit;
		}
	}

	//
	// 开始验证 $MFT 的合法性
	//
	ntfs_log_debug("Comparing $MFTMirr to $MFT... ");
	for ( i = 0; i < vol->mftmirr_size; ++i ) 
	{
		MFT_RECORD *mrec, *mrec2;
		const char *ESTR[12] = { 
			"$MFT", "$MFTMirr", "$LogFile",
			"$Volume", "$AttrDef", "root directory", "$Bitmap",
			"$Boot", "$BadClus", "$Secure", "$UpCase", "$Extend" 
		};

		const char *s;

		if (i < 12)
			s = ESTR[i];
		else if (i < 16)
			s = "system file";
		else
			s = "mft record";

		mrec = (MFT_RECORD*)(m + i * vol->mft_record_size);
		if (mrec->flags & MFT_RECORD_IN_USE) 
		{
			if (ntfs_is_baad_recordp(mrec))
			{
				ntfs_log_debug("FAILED\n");
				ntfs_log_error("$MFT error: Incomplete multi sector transfer detected in '%s'.\n", s);
				goto io_error_exit;
			}

			if (!ntfs_is_mft_recordp(mrec)) 
			{
				ntfs_log_debug("FAILED\n");
				ntfs_log_error("$MFT error: Invalid mft record for '%s'.\n", s);
				goto io_error_exit;
			}
		}

		mrec2 = (MFT_RECORD*)(m2 + i * vol->mft_record_size);
		if (mrec2->flags & MFT_RECORD_IN_USE)
		{
			if (ntfs_is_baad_recordp(mrec2)) 
			{
				ntfs_log_debug("FAILED\n");
				ntfs_log_error("$MFTMirr error: Incomplete multi sector transfer detected in '%s'.\n", s);
				goto io_error_exit;
			}

			if (!ntfs_is_mft_recordp(mrec2)) 
			{
				ntfs_log_debug("FAILED\n");
				ntfs_log_error("$MFTMirr error: Invalid mft record for '%s'.\n", s);
				goto io_error_exit;
			}
		}

		// 校验元数据 $MFTMirr 和 $MFT,确保相同
		if (memcmp(mrec, mrec2, ntfs_mft_record_get_data_size(mrec))) 
		{
			ntfs_log_error("$MFTMirr does not match $MFT (record %d).\n", i);
			goto io_error_exit;
		}
	}

	free(m2);
	free(m);
	m = m2 = NULL;

	//
	// 分析元数据 $Bitmap.
	//
	ntfs_log_debug("Loading $Bitmap... ");
	vol->lcnbmp_ni = ntfs_inode_open(vol, FILE_Bitmap);
	if (!vol->lcnbmp_ni) {
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}

	// 得到元数据$Bitmap的$DATA属性
	vol->lcnbmp_na = ntfs_attr_open(vol->lcnbmp_ni, AT_DATA, AT_UNNAMED, 0);
	if (!vol->lcnbmp_na) {
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}


	//
	// 分析元数据 $UpCase.
	//
	ntfs_log_debug("Loading $UpCase... ");
	ni = ntfs_inode_open(vol, FILE_UpCase);
	if (!ni) {
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}

	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}

	//
	// 通常大小写转换表的长度为65536 2-byte Unicode characters.但是在不同的
	// 情况下可能低于这个值. 故这里仅检查确保字符串没有溢出
	// Just check we don't overflow 32-bits worth of Unicode characters.
	//
	if ( na->data_size & ~0x1ffffffff ) 
	{
		ntfs_log_error("Error: Upcase table is too big (max 32-bit allowed).\n");
		errno = EINVAL;
		goto error_exit;
	}

	if ( vol->upcase_len != na->data_size >> 1 ) 
	{
		vol->upcase_len = na->data_size >> 1;
		
		free(vol->upcase); // Throw away default table.
		vol->upcase = (ntfschar *) ntfs_malloc( (size_t)na->data_size );
		if (!vol->upcase) {
			goto error_exit;
		}
	}

	// 将元数据$UpCase属性中的$DATA属性内容拷贝到新Buffer中 
	l = ntfs_attr_pread( na, 0, na->data_size, vol->upcase );
	if (l != na->data_size) {
		ntfs_log_error("Failed to read $UpCase, unexpected length (%lld != %lld).\n", (LONG64)l,(LONG64)na->data_size);
		errno = EIO;
		goto error_exit;
	}
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode, leaking memory");

	//
	// 分析元数据 $Volume.
	//
	ntfs_log_debug("Loading $Volume... ");
	vol->vol_ni = ntfs_inode_open(vol, FILE_Volume);
	if (!vol->vol_ni) {
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}

	ctx = ntfs_attr_get_search_ctx( vol->vol_ni, NULL );
	if (!ctx) {
		ntfs_log_perror("Failed to allocate attribute search context");
		goto error_exit;
	}

	// 得到元数据$Volume的70H属性
	if (ntfs_attr_lookup(AT_VOLUME_INFORMATION, AT_UNNAMED, 0, 0, 0, NULL,0, ctx)) 
	{
		ntfs_log_perror("$VOLUME_INFORMATION attribute not found in $Volume");
		goto error_exit;
	}
	a = ctx->attr;

	if (a->non_resident) { // $Volume的70H属性必须是常驻属性
		ntfs_log_error("Attribute $VOLUME_INFORMATION must be resident but it isn't.\n");
		errno = EIO;
		goto error_exit;
	}

	vinf = (VOLUME_INFORMATION*)(le16_to_cpu(a->X.value_offset) + (char*)a);

	// $VOLUME_INFORMATION属性的合法性校验
	if ((char*)vinf + le32_to_cpu(a->X.value_length) > (char*)ctx->mrec + le32_to_cpu(ctx->mrec->bytes_in_use) 
		|| le16_to_cpu(a->X.value_offset) + le32_to_cpu(a->X.value_length) > le32_to_cpu(a->length)
	   ) 
	{
		ntfs_log_error("$VOLUME_INFORMATION in $Volume is corrupt.\n");
		errno = EIO;
		goto error_exit;
	}

	// 取出当前NTFS的版本号
	vol->major_ver = vinf->major_ver;
	vol->minor_ver = vinf->minor_ver;
	vol->flags = vinf->flags;

	// Reinitialize the search context for the $Volume/$VOLUME_NAME lookup.
	ntfs_attr_reinit_search_ctx(ctx);
	if ( ntfs_attr_lookup(AT_VOLUME_NAME, AT_UNNAMED, 0, 0, 0, NULL, 0, ctx) ) 
	{
		if (errno != ENOENT) {
			ntfs_log_perror("Failed to lookup of $VOLUME_NAME in $Volume failed");
			goto error_exit;
		}

		vol->vol_name = (char *) ntfs_malloc(1);
		if (!vol->vol_name) {
			goto error_exit;
		}
		vol->vol_name[0] = '\0';

	} else {

		// 成功读取 70H 属性(卷名)
		a = ctx->attr;
		if (a->non_resident) { // 必须是常驻属性
			ntfs_log_error("$VOLUME_NAME must be resident.\n");
			errno = EIO;
			goto error_exit;
		}

		// 得到卷名的地址和长度
		vname = (ntfschar*)(le16_to_cpu(a->X.value_offset) + (char*)a);
		u = le32_to_cpu(a->X.value_length) / 2;
		
		// 将卷名 unicode 转换成 多字符集
		vol->vol_name = NULL;
		if ( ntfs_ucstombs(vname, u, &vol->vol_name, 0) == -1 ) 
		{
			ntfs_log_perror("Volume name could not be converted to current locale");
			ntfs_log_debug("Forcing name into ASCII by replacing non-ASCII characters with underscores.\n");

			vol->vol_name = (char *) ntfs_malloc(u + 1);
			if (!vol->vol_name) {
				goto error_exit;
			}

			for (j = 0; j < (s32)u; j++) 
			{
				ntfschar uc = le16_to_cpu(vname[j]);
				if (uc > 0xff) { uc = (ntfschar)'_'; }
				vol->vol_name[j] = (char)uc;
			}
			vol->vol_name[u] = '\0';
		}
	}
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;

	//
	// 分析元数据 $AttrDef.
	//
	ntfs_log_debug("Loading $AttrDef... ");
	ni = ntfs_inode_open( vol, FILE_AttrDef );
	if (!ni) {
		ntfs_log_perror("Failed to open $AttrDef");
		goto error_exit;
	}
	
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}

	// 检查是否溢出
	if ( na->data_size > 0xffffffff ) 
	{
		ntfs_log_error("Attribute definition table is too big (max 32-bit allowed).\n");
		errno = EINVAL;
		goto error_exit;
	}
	vol->attrdef_len = na->data_size;
	vol->attrdef = (ATTR_DEF *) ntfs_malloc( (size_t)na->data_size );
	if (!vol->attrdef) {
		
		goto error_exit;
	}

	l = ntfs_attr_pread(na, 0, na->data_size, vol->attrdef);
	if (l != na->data_size) {
		
		ntfs_log_error("Failed to read $AttrDef, unexpected length (%lld != %lld).\n", (LONG64)l,(LONG64)na->data_size);
		errno = EIO;
		goto error_exit;
	}
	
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni)) { ntfs_log_perror("Failed to close inode, leaking memory"); }

	//
	// Check for dirty logfile and hibernated Windows. We care only about read-write mounts.
	//
	if ( !(flags & MS_RDONLY) ) 
	{
		if (ntfs_volume_check_logfile(vol) < 0) { goto error_exit; }
		if (ntfs_volume_check_hiberfile(vol) < 0) { goto error_exit; }
	}

	return vol;

	// 收尾工作
io_error_exit:
	errno = EIO;
error_exit:
	eo = errno;
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	free(m);
	free(m2);
	__ntfs_volume_release(vol);
	errno = eo;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////

struct ntfs_device_operations g_default_io_ops ;

ntfs_volume *
ntfs_mount(
	IN const char *name,
	IN unsigned long flags
	)
/*++

Routine Description:
  This function mounts an ntfs volume. @name should contain the name of the
  device/file to mount as the ntfs volume.  
    
Arguments:
  name - name of device/file to open
  flags - optional mount flags

Return Value:

    
--*/
{
	struct ntfs_device *dev;
	ntfs_volume *vol;

	g_default_io_ops.open	= ntfs_device_win32_open ;
	g_default_io_ops.close	= ntfs_device_win32_close ;
	g_default_io_ops.seek	= ntfs_device_win32_seek ;
	g_default_io_ops.read	= ntfs_device_win32_read ;
	g_default_io_ops.write	= ntfs_device_win32_write ;
	g_default_io_ops.pread  = ntfs_device_win32_pread ;
	g_default_io_ops.pwrite = ntfs_device_win32_pwrite ;
	g_default_io_ops.sync   = ntfs_device_win32_sync ;
	g_default_io_ops.ioctl	= ntfs_device_win32_ioctl ;

	// 分配结构体
	dev = ntfs_device_alloc(name, 0, &g_default_io_ops, NULL);
	if (!dev) { return NULL; }

	// 真正开始做事...
	vol = ntfs_device_mount( dev, flags );
	if (!vol) 
	{
		int eo = errno;
		ntfs_device_free( dev );
		errno = eo;
	}

	return vol;
}


/**
 * ntfs_device_umount - close ntfs volume
 * @vol: address of ntfs_volume structure of volume to close
 * @force: if true force close the volume even if it is busy
 *
 * Deallocate all structures (including @vol itself) associated with the ntfs
 * volume @vol.
 *
 * Note it is up to the caller to destroy the device associated with the volume
 * being unmounted after this function returns.
 *
 * Return 0 on success. On error return -1 with errno set appropriately
 * (most likely to one of EAGAIN, EBUSY or EINVAL). The EAGAIN error means that
 * an operation is in progress and if you try the close later the operation
 * might be completed and the close succeed.
 *
 * If @force is true (i.e. not zero) this function will close the volume even
 * if this means that data might be lost.
 *
 * @vol must have previously been returned by a call to ntfs_device_mount().
 *
 * @vol itself is deallocated and should no longer be dereferenced after this
 * function returns success. If it returns an error then nothing has been done
 * so it is safe to continue using @vol.
 */
int ntfs_device_umount(ntfs_volume *vol,
		const BOOL force)
{
	if (!vol) {
		errno = EINVAL;
		return -1;
	}
	__ntfs_volume_release(vol);
	return 0;
}



int 
ntfs_umount(
	IN ntfs_volume *vol,
	IN const BOOL force
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/05 [5:3:2009 - 20:22]

Routine Description:
  close ntfs volume. 释放所有申请的结构体,内存等

Arguments:
  vol - address of ntfs_volume structure of volume to close
  force - If @force is true (i.e. not zero) this function will close the volume even
    if this means that data might be lost.

Return Value:
  0 -- 成功; -1 -- 失败
    
--*/
{
	struct ntfs_device *dev;

	if (!vol) { // 参数合法性检测
		errno = EINVAL;
		return -1;
	}

	dev = vol->dev;
	__ntfs_volume_release(vol);
	ntfs_device_free(dev);
	return 0;
}

#ifdef HAVE_MNTENT_H

/**
 * realpath - If there is no realpath on the system
 */
static char *realpath(const char *path, char *resolved_path)
{
	strncpy(resolved_path, path, PATH_MAX);
	resolved_path[PATH_MAX] = '\0';
	return resolved_path;
}


/**
 * ntfs_mntent_check - desc
 *
 * If you are wanting to use this, you actually wanted to use
 * ntfs_check_if_mounted(), you just didn't realize. (-:
 *
 * See description of ntfs_check_if_mounted(), below.
 */
static int ntfs_mntent_check(const char *file, unsigned long *mnt_flags)
{
	struct mntent *mnt;
	char *real_file = NULL, *real_fsname = NULL;
	FILE *f;
	int err = 0;

	real_file = (char *) ntfs_malloc(PATH_MAX + 1);
	if (!real_file)
		return -1;
	real_fsname = (char *) ntfs_malloc(PATH_MAX + 1);
	if (!real_fsname) {
		err = errno;
		goto exit;
	}
	if (!realpath(file, real_file)) {
		err = errno;
		goto exit;
	}
	if (!(f = setmntent(MOUNTED, "r"))) {
		err = errno;
		goto exit;
	}
	while ((mnt = getmntent(f))) {
		if (!realpath(mnt->mnt_fsname, real_fsname))
			continue;
		if (!strcmp(real_file, real_fsname))
			break;
	}
	endmntent(f);
	if (!mnt)
		goto exit;
	*mnt_flags = NTFS_MF_MOUNTED;
	if (!strcmp(mnt->mnt_dir, "/"))
		*mnt_flags |= NTFS_MF_ISROOT;
#ifdef HAVE_HASMNTOPT
	if (hasmntopt(mnt, "ro") && !hasmntopt(mnt, "rw"))
		*mnt_flags |= NTFS_MF_READONLY;
#endif
exit:
	free(real_file);
	free(real_fsname);
	if (err) {
		errno = err;
		return -1;
	}
	return 0;
}
#endif /* HAVE_MNTENT_H */

/**
 * ntfs_check_if_mounted - check if an ntfs volume is currently mounted
 * @file:	device file to check
 * @mnt_flags:	pointer into which to return the ntfs mount flags (see volume.h)
 *
 * If the running system does not support the {set,get,end}mntent() calls,
 * just return 0 and set *@mnt_flags to zero.
 *
 * When the system does support the calls, ntfs_check_if_mounted() first tries
 * to find the device @file in /etc/mtab (or wherever this is kept on the
 * running system). If it is not found, assume the device is not mounted and
 * return 0 and set *@mnt_flags to zero.
 *
 * If the device @file is found, set the NTFS_MF_MOUNTED flags in *@mnt_flags.
 *
 * Further if @file is mounted as the file system root ("/"), set the flag
 * NTFS_MF_ISROOT in *@mnt_flags.
 *
 * Finally, check if the file system is mounted read-only, and if so set the
 * NTFS_MF_READONLY flag in *@mnt_flags.
 *
 * On success return 0 with *@mnt_flags set to the ntfs mount flags.
 *
 * On error return -1 with errno set to the error code.
 */
int ntfs_check_if_mounted(const char *file,
		unsigned long *mnt_flags)
{
	*mnt_flags = 0;
#ifdef HAVE_MNTENT_H
	return ntfs_mntent_check(file, mnt_flags);
#else
	return 0;
#endif
}

/**
 * ntfs_version_is_supported - check if NTFS version is supported.
 * @vol:	ntfs volume whose version we're interested in.
 *
 * The function checks if the NTFS volume version is known or not.
 * Version 1.1 and 1.2 are used by Windows NT3.x and NT4.
 * Version 2.x is used by Windows 2000 Betas.
 * Version 3.0 is used by Windows 2000.
 * Version 3.1 is used by Windows XP, Windows Server 2003 and Longhorn.
 *
 * Return 0 if NTFS version is supported otherwise -1 with errno set.
 *
 * The following error codes are defined:
 *	EOPNOTSUPP - Unknown NTFS version
 *	EINVAL	   - Invalid argument
 */
int ntfs_version_is_supported(ntfs_volume *vol)
{
	u8 major, minor;

	if (!vol) {
		errno = EINVAL;
		return -1;
	}

	major = vol->major_ver;
	minor = vol->minor_ver;

	if (NTFS_V1_1(major, minor) || NTFS_V1_2(major, minor))
		return 0;

	if (NTFS_V2_X(major, minor))
		return 0;

	if (NTFS_V3_0(major, minor) || NTFS_V3_1(major, minor))
		return 0;

	errno = /*EOPNOTSUPP*/1045;
	return -1;
}

/**
 * ntfs_logfile_reset - "empty" $LogFile data attribute value
 * @vol:	ntfs volume whose $LogFile we intend to reset.
 *
 * Fill the value of the $LogFile data attribute, i.e. the contents of
 * the file, with 0xff's, thus marking the journal as empty.
 *
 * FIXME(?): We might need to zero the LSN field of every single mft
 * record as well. (But, first try without doing that and see what
 * happens, since chkdsk might pickup the pieces and do it for us...)
 *
 * On success return 0.
 *
 * On error return -1 with errno set to the error code.
 */
int ntfs_logfile_reset(ntfs_volume *vol)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	int eo;

	if (!vol) {
		errno = EINVAL;
		return -1;
	}

	if ((ni = ntfs_inode_open(vol, FILE_LogFile)) == NULL) {
		ntfs_log_perror("Failed to open inode FILE_LogFile.");
		return -1;
	}

	if ((na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0)) == NULL) {
		eo = errno;
		ntfs_log_perror("Failed to open $FILE_LogFile/$DATA");
		goto error_exit;
	}

	if (ntfs_empty_logfile(na)) {
		eo = errno;
		ntfs_log_perror("Failed to empty $FILE_LogFile/$DATA");
		ntfs_attr_close(na);
		goto error_exit;
	}
	ntfs_attr_close(na);
	return ntfs_inode_close(ni);

error_exit:
	ntfs_inode_close(ni);
	errno = eo;
	return -1;
}

/**
 * ntfs_volume_write_flags - set the flags of an ntfs volume
 * @vol:	ntfs volume where we set the volume flags
 * @flags:	new flags
 *
 * Set the on-disk volume flags in the mft record of $Volume and
 * on volume @vol to @flags.
 *
 * Return 0 if successful and -1 if not with errno set to the error code.
 */
int ntfs_volume_write_flags(ntfs_volume *vol, const u16 flags)
{
	ATTR_RECORD *a;
	VOLUME_INFORMATION *c;
	ntfs_attr_search_ctx *ctx;
	int ret = -1;	/* failure */

	if (!vol || !vol->vol_ni) {
		errno = EINVAL;
		return -1;
	}
	/* Get a pointer to the volume information attribute. */
	ctx = ntfs_attr_get_search_ctx(vol->vol_ni, NULL);
	if (!ctx) {
		ntfs_log_perror("Failed to allocate attribute search context");
		return -1;
	}
	if (ntfs_attr_lookup(AT_VOLUME_INFORMATION, AT_UNNAMED, 0, 0, 0, NULL,
			0, ctx)) {
		ntfs_log_error("Attribute $VOLUME_INFORMATION was not found "
			       "in $Volume!\n");
		goto err_out;
	}
	a = ctx->attr;
	/* Sanity check. */
	if (a->non_resident) {
		ntfs_log_error("Attribute $VOLUME_INFORMATION must be resident "
			       "but it isn't.\n");
		errno = EIO;
		goto err_out;
	}
	/* Get a pointer to the value of the attribute. */
	c = (VOLUME_INFORMATION*)(le16_to_cpu(a->X.value_offset) + (char*)a);
	/* Sanity checks. */
	if ((char*)c + le32_to_cpu(a->X.value_length) > (char*)ctx->mrec +
			le32_to_cpu(ctx->mrec->bytes_in_use) ||
			le16_to_cpu(a->X.value_offset) +
			le32_to_cpu(a->X.value_length) > le32_to_cpu(a->length)) {
		ntfs_log_error("Attribute $VOLUME_INFORMATION in $Volume is corrupt!\n");
		errno = EIO;
		goto err_out;
	}
	/* Set the volume flags. */
	vol->flags = c->flags = flags & VOLUME_FLAGS_MASK;
	/* Write them to disk. */
	ntfs_inode_mark_dirty(vol->vol_ni);
	if (ntfs_inode_sync(vol->vol_ni)) {
		ntfs_log_perror("Error writing $Volume");
		goto err_out;
	}
	ret = 0; /* success */
err_out:
	ntfs_attr_put_search_ctx(ctx);
	return ret;
}

