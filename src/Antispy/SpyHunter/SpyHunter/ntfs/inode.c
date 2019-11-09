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
 * inode.c - Inode handling code. Originated from the Linux-NTFS project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <stddef.h>


#include "compat.h"
#include "types.h"
#include "attrib.h"
#include "inode.h"
#include "debug.h"
#include "mft.h"
#include "attrlist.h"
#include "runlist.h"
#include "lcnalloc.h"
#include "index.h"
#include "dir.h"
#include "ntfstime.h"
#include "logging.h"
#include "misc.h"

//////////////////////////////////////////////////////////////////////////


void 
ntfs_inode_mark_dirty(
	IN ntfs_inode *ni
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 12:31]

Routine Description:
  Set the inode @ni dirty so it is written out later (at the latest at
  ntfs_inode_close() time). If @ni is an extent inode, set the base inode
  dirty, too. This function cannot fail.  
    
--*/
{
	NInoSetDirty(ni);
	if (ni->nr_extents == -1) { NInoSetDirty(ni->base_ni); }

	return ;
}



static ntfs_inode *
__ntfs_inode_allocate(
	IN ntfs_volume *vol
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 12:28]

Routine Description:
  分配一个inode结构体,成功则将参数1赋予给该结构的一个变量

Return Value:
  返回这个新申请的结构体指针
    
--*/
{
	ntfs_inode *ni;

	ni = (ntfs_inode*)calloc( 1, sizeof(ntfs_inode) );
	if (ni) { ni->vol = vol; }

	return ni;
}


ntfs_inode *ntfs_inode_allocate(ntfs_volume *vol)
{
	return __ntfs_inode_allocate(vol);
}



static int 
__ntfs_inode_release(
	IN ntfs_inode *ni
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 12:35]

Routine Description:
  释放掉一个inode结构
    
--*/
{
	if ( NInoDirty(ni) ) { ntfs_log_debug("Eeek. Discarding dirty inode!\n"); }
	if ( NInoAttrList(ni) && ni->attr_list ) { free( ni->attr_list ); }
	free( ni->mrec );
	free( ni );
	return 0;
}



ntfs_inode *
ntfs_inode_open( 
	IN ntfs_volume *vol, 
	IN const MFT_REF mref
	)
/*++

Routine Description:
  open an inode ready for access.
  Allocate an ntfs_inode structure and initialize it for the given inode
  specified by @mref. @mref specifies the inode number / mft record to read,
  including the sequence number, which can be 0 if no sequence number checking
  is to be performed.
 
  Then, allocate a buffer for the mft record, read the mft record from the
  volume @vol, and attach it to the ntfs_inode structure (->mrec). The
  mft record is mst deprotected and sanity checked for validity and we abort
  if deprotection or checks fail.
 
  Finally, search for an attribute list attribute in the mft record and if one
  is found, load the attribute list attribute value and attach it to the
  ntfs_inode structure (->attr_list). Also set the NI_AttrList bit to indicate this.
    
Arguments:
  vol - volume to get the inode from
  mref - inode number / mft record number to open

Return Value:
  Return a pointer to the ntfs_inode structure on success or NULL on error,
  with errno set to the error code.
    
--*/
{
	s64 l;
	ntfs_inode *ni;
	ntfs_attr_search_ctx *ctx;
	int err = 0;
	STANDARD_INFORMATION *std_info;

	ntfs_log_trace("Entering for inode 0x%llx.\n", MREF(mref));
	if (!vol) {
		errno = EINVAL;
		return NULL;
	}

	// 分配一个节点
	ni = __ntfs_inode_allocate( vol );
	if ( !ni ) { return NULL; }

	// 得到指定的MFT,将其内容拷贝到 ni->mrec 中
	if ( ntfs_file_record_read( vol, mref, &ni->mrec, NULL ) ) { goto err_out; }
	if (!(ni->mrec->flags & MFT_RECORD_IN_USE)) 
	{
		err = ENOENT;
		goto err_out;
	}

	ni->mft_no = MREF(mref);
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx) { goto err_out; }

	// 查找该MFT中的标准属性 0x10,记录其中的部分内容到节点中
	if ( ntfs_attr_lookup(AT_STANDARD_INFORMATION, AT_UNNAMED, 0, CASE_SENSITIVE, 0, NULL, 0, ctx) ) 
	{
		err = errno;
		ntfs_log_trace("Failed to receive STANDARD_INFORMATION attribute.\n");
		goto put_err_out;
	}

	std_info = (STANDARD_INFORMATION *)((u8 *)ctx->attr + le16_to_cpu(ctx->attr->X.value_offset));
	ni->flags = std_info->file_attributes;
	ni->creation_time = ntfs2utc(std_info->creation_time);
	ni->last_data_change_time = ntfs2utc(std_info->last_data_change_time);
	ni->last_mft_change_time = ntfs2utc(std_info->last_mft_change_time);
	ni->last_access_time = ntfs2utc(std_info->last_access_time);

	// 查找该MFT中的属性列表属性 0x20,记录其中的部分内容到节点中
	if (ntfs_attr_lookup(AT_ATTRIBUTE_LIST, AT_UNNAMED, 0, 0, 0, NULL, 0, ctx))
	{
		if ( errno != ENOENT ) { goto put_err_out; }
	
		goto get_size;
	}

	NInoSetAttrList( ni );

	// 得到该MFT中属性内容的大小
	l = ntfs_get_attribute_value_length( ctx->attr );
	if (!l) { goto put_err_out; }
	if (l > 0x40000) 
	{
		err = EIO;
		goto put_err_out;
	}

	ni->attr_list_size = l;

	// 得到该MFT中属性内容,拷贝到新Buffer中
	ni->attr_list = (u8 *) ntfs_malloc(ni->attr_list_size);
	if ( !ni->attr_list ) { goto put_err_out; }

	l = ntfs_get_attribute_value( vol, ctx->attr, ni->attr_list );
	if ( !l ) { goto put_err_out; }
	if ( l != ni->attr_list_size ) {
		err = EIO;
		goto put_err_out;
	}

get_size:
	// 得到该MFT中数据属性(0x80)
	if (ntfs_attr_lookup(AT_DATA, AT_UNNAMED, 0, 0, 0, NULL, 0, ctx)) 
	{
		if (errno != ENOENT) { goto put_err_out; }

		// 是文件夹或特殊文件
		ni->data_size = ni->allocated_size = 0;

	} else { // 正常的数据属性(0x80)

		if ( ctx->attr->non_resident ) 
		{
			// 非常驻属性
			ni->data_size = sle64_to_cpu(ctx->attr->X.data_size);
			if (ctx->attr->flags & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE)) {
				ni->allocated_size = sle64_to_cpu( ctx->attr->X.compressed_size );

			} else {
				ni->allocated_size = sle64_to_cpu( ctx->attr->X.allocated_size );
			}

		} else {

			// 常驻属性
			ni->data_size = le32_to_cpu(ctx->attr->X.value_length);
			ni->allocated_size = (ni->data_size + 7) & ~7;
		}
	}

	// 收尾清理工作
	ntfs_attr_put_search_ctx(ctx);
	return ni;

put_err_out:
	if (!err) { err = errno; }
	ntfs_attr_put_search_ctx(ctx);

err_out:
	if (!err) { err = errno; }
	__ntfs_inode_release(ni);
	errno = err;
	return NULL;
}

/**
 * ntfs_inode_close - close an ntfs inode and free all associated memory
 * @ni:		ntfs inode to close
 *
 * Make sure the ntfs inode @ni is clean.
 *
 * If the ntfs inode @ni is a base inode, close all associated extent inodes,
 * then deallocate all memory attached to it, and finally free the ntfs inode
 * structure itself.
 *
 * If it is an extent inode, we disconnect it from its base inode before we
 * destroy it.
 *
 * Return 0 on success or -1 on error with errno set to the error code. On
 * error, @ni has not been freed. The user should attempt to handle the error
 * and call ntfs_inode_close() again. The following error codes are defined:
 *
 *	EBUSY	@ni and/or its attribute list runlist is/are dirty and the
 *		attempt to write it/them to disk failed.
 *	EINVAL	@ni is invalid (probably it is an extent inode).
 *	EIO	I/O error while trying to write inode to disk.
 */
int ntfs_inode_close(ntfs_inode *ni)
{
	if (!ni)
		return 0;

	/* If we have dirty metadata, write it out. */
	if (NInoDirty(ni) || NInoAttrListDirty(ni)) {
		if (ntfs_inode_sync(ni)) {
			if (errno != EIO)
				errno = EBUSY;
			return -1;
		}
	}
	/* Is this a base inode with mapped extent inodes? */
	if (ni->nr_extents > 0) {
		while (ni->nr_extents > 0) {
			if (ntfs_inode_close(ni->extent_nis[0])) {
				if (errno != EIO)
					errno = EBUSY;
				return -1;
			}
		}
	} else if (ni->nr_extents == -1) {
		ntfs_inode **tmp_nis;
		ntfs_inode *base_ni;
		s32 i;

		/*
		 * If the inode is an extent inode, disconnect it from the
		 * base inode before destroying it.
		 */
		base_ni = ni->base_ni;
		for (i = 0; i < base_ni->nr_extents; ++i) {
			tmp_nis = base_ni->extent_nis;
			if (tmp_nis[i] != ni)
				continue;
			/* Found it. Disconnect. */
			memmove(tmp_nis + i, tmp_nis + i + 1,
					(base_ni->nr_extents - i - 1) *
					sizeof(ntfs_inode *));
			/* Buffer should be for multiple of four extents. */
			if ((--base_ni->nr_extents) & 3) {
				i = -1;
				break;
			}
			/*
			 * ElectricFence is unhappy with realloc(x,0) as free(x)
			 * thus we explicitly separate these two cases.
			 */
			if (base_ni->nr_extents) {
				/* Resize the memory buffer. */
				tmp_nis = realloc(tmp_nis, base_ni->nr_extents *
						  sizeof(ntfs_inode *));
				/* Ignore errors, they don't really matter. */
				if (tmp_nis)
					base_ni->extent_nis = tmp_nis;
			} else if (tmp_nis)
				free(tmp_nis);
			/* Allow for error checking. */
			i = -1;
			break;
		}
		if (i != -1)
			ntfs_log_debug("Extent inode was not attached to base inode! "
					"Weird! Continuing regardless.\n");
	}
	return __ntfs_inode_release(ni);
}

/**
 * ntfs_extent_inode_open - load an extent inode and attach it to its base
 * @base_ni:	base ntfs inode
 * @mref:	mft reference of the extent inode to load (in little endian)
 *
 * First check if the extent inode @mref is already attached to the base ntfs
 * inode @base_ni, and if so, return a pointer to the attached extent inode.
 *
 * If the extent inode is not already attached to the base inode, allocate an
 * ntfs_inode structure and initialize it for the given inode @mref. @mref
 * specifies the inode number / mft record to read, including the sequence
 * number, which can be 0 if no sequence number checking is to be performed.
 *
 * Then, allocate a buffer for the mft record, read the mft record from the
 * volume @base_ni->vol, and attach it to the ntfs_inode structure (->mrec).
 * The mft record is mst deprotected and sanity checked for validity and we
 * abort if deprotection or checks fail.
 *
 * Finally attach the ntfs inode to its base inode @base_ni and return a
 * pointer to the ntfs_inode structure on success or NULL on error, with errno
 * set to the error code.
 *
 * Note, extent inodes are never closed directly. They are automatically
 * disposed off by the closing of the base inode.
 */
ntfs_inode *
ntfs_extent_inode_open(
	IN ntfs_inode *base_ni,
	IN const MFT_REF mref
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 12:18]

Routine Description:
    
    
Arguments:

     - 

Return Value:

    
--*/
{
	u64 mft_no = MREF_LE(mref);
	ntfs_inode *ni;
	ntfs_inode **extent_nis;
	int i;

	if (!base_ni) {
		errno = EINVAL;
		return NULL;
	}
	ntfs_log_trace("Opening extent inode 0x%llx (base mft record 0x%llx).\n",
			(ULONG64)mft_no,
			(ULONG64)base_ni->mft_no);
	// Is the extent inode already open and attached to the base inode?
	if ( base_ni->nr_extents > 0 ) 
	{
		extent_nis = base_ni->extent_nis;
		for (i = 0; i < base_ni->nr_extents; i++) {
			u16 seq_no;

			ni = extent_nis[i];
			if (mft_no != ni->mft_no) { continue; }

			// 校验该MFT偏移0x10处的序列号
			seq_no = MSEQNO_LE(mref);
			if ( seq_no && seq_no != le16_to_cpu( ni->mrec->sequence_number ) ) 
			{
				ntfs_log_debug("Found stale extent mft reference! "
					"Corrupt file system. Run chkdsk.\n");
				errno = EIO;
				return NULL;
			}

			// We are done, return the extent inode.
			return ni;
		}
	}

	// Wasn't there, we need to load the extent inode. */
	ni = __ntfs_inode_allocate(base_ni->vol);
	if (!ni)
		return NULL;
	if (ntfs_file_record_read(base_ni->vol, le64_to_cpu(mref), &ni->mrec,
			NULL))
		goto err_out;
	ni->mft_no = mft_no;
	ni->nr_extents = -1;
	ni->base_ni = base_ni;
	/* Attach extent inode to base inode, reallocating memory if needed. */
	if (!(base_ni->nr_extents & 3)) {
		i = (base_ni->nr_extents + 4) * sizeof(ntfs_inode *);

		extent_nis = (ntfs_inode **)ntfs_malloc(i);
		if (!extent_nis)
			goto err_out;
		if (base_ni->nr_extents) {
			memcpy(extent_nis, base_ni->extent_nis,
					i - 4 * sizeof(ntfs_inode *));
			free(base_ni->extent_nis);
		}
		base_ni->extent_nis = extent_nis;
	}
	base_ni->extent_nis[base_ni->nr_extents++] = ni;
	return ni;
err_out:
	i = errno;
	__ntfs_inode_release(ni);
	errno = i;
	ntfs_log_perror("Failed to open extent inode");
	return NULL;
}

/**
 * ntfs_inode_attach_all_extents - attach all extents for target inode
 * @ni:		opened ntfs inode for which perform attach
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
int ntfs_inode_attach_all_extents(ntfs_inode *ni)
{
	ATTR_LIST_ENTRY *ale;
	u64 prev_attached = 0;

	if (!ni) {
		ntfs_log_trace("Invalid arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (ni->nr_extents == -1)
		ni = ni->base_ni;

	ntfs_log_trace("Entering for inode 0x%llx.\n", (LONG64) ni->mft_no);

	/* Inode haven't got attribute list, thus nothing to attach. */
	if (!NInoAttrList(ni))
		return 0;

	if (!ni->attr_list) {
		ntfs_log_trace("Corrupt in-memory struct.\n");
		errno = EINVAL;
		return -1;
	}

	/* Walk through attribute list and attach all extents. */
	errno = 0;
	ale = (ATTR_LIST_ENTRY *)ni->attr_list;
	while ((u8*)ale < ni->attr_list + ni->attr_list_size) {
		if (ni->mft_no != MREF_LE(ale->mft_reference) &&
				prev_attached != MREF_LE(ale->mft_reference)) {
			if (!ntfs_extent_inode_open(ni,
					MREF_LE(ale->mft_reference))) {
				ntfs_log_trace("Couldn't attach extent inode.\n");
				return -1;
			}
			prev_attached = MREF_LE(ale->mft_reference);
		}
		ale = (ATTR_LIST_ENTRY *)((u8*)ale + le16_to_cpu(ale->length));
	}
	return 0;
}

/**
 * ntfs_inode_sync_standard_information - update standard information attribute
 * @ni:		ntfs inode to update standard information
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
static int ntfs_inode_sync_standard_information(ntfs_inode *ni)
{
	ntfs_attr_search_ctx *ctx;
	STANDARD_INFORMATION *std_info;
	int err;

	ntfs_log_trace("Entering for inode 0x%llx.\n", (LONG64) ni->mft_no);

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx)
		return -1;
	if (ntfs_attr_lookup(AT_STANDARD_INFORMATION, AT_UNNAMED,
				0, CASE_SENSITIVE, 0, NULL, 0, ctx)) {
		err = errno;
		ntfs_log_trace("Failed to receive STANDARD_INFORMATION "
				"attribute.\n");
		ntfs_attr_put_search_ctx(ctx);
		errno = err;
		return -1;
	}
	std_info = (STANDARD_INFORMATION *)((u8 *)ctx->attr +
			le16_to_cpu(ctx->attr->X.value_offset));
	std_info->file_attributes = ni->flags;
	std_info->creation_time = utc2ntfs(ni->creation_time);
	std_info->last_data_change_time = utc2ntfs(ni->last_data_change_time);
	std_info->last_mft_change_time = utc2ntfs(ni->last_mft_change_time);
	std_info->last_access_time = utc2ntfs(ni->last_access_time);
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	return 0;
}

/**
 * ntfs_inode_sync_file_name - update FILE_NAME attributes
 * @ni:		ntfs inode to update FILE_NAME attributes
 *
 * Update all FILE_NAME attributes for inode @ni in the index.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
static int ntfs_inode_sync_file_name(ntfs_inode *ni)
{
	ntfs_attr_search_ctx *ctx = NULL;
	ntfs_index_context *ictx;
	ntfs_inode *index_ni;
	FILE_NAME_ATTR *fn;
	int err = 0;

	ntfs_log_trace("Entering for inode 0x%llx.\n", (LONG64) ni->mft_no);

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx) {
		err = errno;
		ntfs_log_trace("Failed to get attribute search context.\n");
		goto err_out;
	}
	/* Walk through all FILE_NAME attributes and update them. */
	while (!ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, 0, 0, NULL, 0, ctx)) {
		fn = (FILE_NAME_ATTR *)((u8 *)ctx->attr +
				le16_to_cpu(ctx->attr->X.value_offset));
		if (MREF_LE(fn->parent_directory) == ni->mft_no) {
			/*
			 * WARNING: We cheater here and obtain 2 attribute
			 * search contexts for one inode (first we obtained
			 * above, second will be obtained inside
			 * ntfs_index_lookup), it's acceptable for library,
			 * but will lock kernel.
			 */
			index_ni = ni;
		} else {
			index_ni = ntfs_inode_open( ni->vol, le64_to_cpu(fn->parent_directory) );
		}
		if (!index_ni) {
			if (!err)
				err = errno;
			ntfs_log_trace("Failed to open inode with index.\n");
			continue;
		}
		ictx = ntfs_index_ctx_get(index_ni, NTFS_INDEX_I30, 4);
		if (!ictx) {
			if (!err)
				err = errno;
			ntfs_log_trace("Failed to get index context.\n");
			ntfs_inode_close(index_ni);
			continue;
		}
		if (ntfs_index_lookup(fn, sizeof(FILE_NAME_ATTR), ictx)) {
			if (!err) {
				if (errno == ENOENT)
					err = EIO;
				else
					err = errno;
			}
			ntfs_log_trace("Index lookup failed.\n");
			ntfs_index_ctx_put(ictx);
			ntfs_inode_close(index_ni);
			continue;
		}
		/* Update flags and file size. */
		fn = (FILE_NAME_ATTR *)ictx->data;
		fn->file_attributes =
				(fn->file_attributes & ~FILE_ATTR_VALID_FLAGS) |
				(ni->flags & FILE_ATTR_VALID_FLAGS);
		fn->allocated_size = cpu_to_sle64(ni->allocated_size);
		fn->data_size = cpu_to_sle64(ni->data_size);
		fn->creation_time = utc2ntfs(ni->creation_time);
		fn->last_data_change_time = utc2ntfs(ni->last_data_change_time);
		fn->last_mft_change_time = utc2ntfs(ni->last_mft_change_time);
		fn->last_access_time = utc2ntfs(ni->last_access_time);
		ntfs_index_entry_mark_dirty(ictx);
		ntfs_index_ctx_put(ictx);
		if (ni != index_ni)
			ntfs_inode_close(index_ni);
	}
	/* Check for real error occurred. */
	if (errno != ENOENT) {
		err = errno;
		ntfs_log_trace("Attribute lookup failed.\n");
		goto err_out;
	}
	ntfs_attr_put_search_ctx(ctx);
	if (err) {
		errno = err;
		return -1;
	}
	return 0;
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	errno = err;
	return -1;
}

/**
 * ntfs_inode_sync - write the inode (and its dirty extents) to disk
 * @ni:		ntfs inode to write
 *
 * Write the inode @ni to disk as well as its dirty extent inodes if such
 * exist and @ni is a base inode. If @ni is an extent inode, only @ni is
 * written completely disregarding its base inode and any other extent inodes.
 *
 * For a base inode with dirty extent inodes if any writes fail for whatever
 * reason, the failing inode is skipped and the sync process is continued. At
 * the end the error condition that brought about the failure is returned. Thus
 * the smallest amount of data loss possible occurs.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 * The following error codes are defined:
 *	EINVAL	- Invalid arguments were passed to the function.
 *	EBUSY	- Inode and/or one of its extents is busy, try again later.
 *	EIO	- I/O error while writing the inode (or one of its extents).
 */
int ntfs_inode_sync(ntfs_inode *ni)
{
	int err = 0;

	if (!ni) {
		errno = EINVAL;
		return -1;
	}

	ntfs_log_trace("Entering for inode 0x%llx.\n", (LONG64) ni->mft_no);

	/* Update STANDARD_INFORMATION. */
	if ((ni->mrec->flags & MFT_RECORD_IN_USE) && ni->nr_extents != -1 &&
			ntfs_inode_sync_standard_information(ni)) {
		if (!err || errno == EIO) {
			err = errno;
			if (err != EIO)
				err = EBUSY;
		}
		ntfs_log_trace("Failed to sync standard information.\n");
	}

	/* Update FILE_NAME's in the index. */
	if ((ni->mrec->flags & MFT_RECORD_IN_USE)
		&& (ni->nr_extents != -1) 
		&& /*(NInoFileNameTestAndClearDirty(ni))*/NInoFileNameTestAndClearDirty_func(ni) 
		&& ntfs_inode_sync_file_name(ni)
	   ) 
	{
		if (!err || errno == EIO) {
			err = errno;
			if (err != EIO)
				err = EBUSY;
		}

		ntfs_log_trace("Failed to sync FILE_NAME attributes.\n");
		NInoFileNameSetDirty(ni);
	}

	/* Write out attribute list from cache to disk. */
	if ((ni->mrec->flags & MFT_RECORD_IN_USE) 
		&& ni->nr_extents != -1 
		&& NInoAttrList(ni) 
		&& /*NInoAttrListTestAndClearDirty(ni)*/ NInoAttrListTestAndClearDirty_func(ni)
	   ) 
	{
		ntfs_attr *na;

		na = ntfs_attr_open(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
		if (!na) 
		{
			if (!err || errno == EIO) 
			{
				err = errno;
				if (err != EIO)
					err = EBUSY;
				ntfs_log_trace("Attribute list sync failed (open failed).\n");
			}

			NInoAttrListSetDirty(ni);

		} else 
		{
			if (na->data_size == ni->attr_list_size)
			{
				if (ntfs_attr_pwrite(na, 0, ni->attr_list_size,
							ni->attr_list) !=
							ni->attr_list_size)
				{
					if (!err || errno == EIO) 
					{
						err = errno;
						if (err != EIO) { err = EBUSY; }
						ntfs_log_trace("Attribute list sync failed (write failed).\n");
					}
					NInoAttrListSetDirty(ni);
				}
			} else {
				err = EIO;
				ntfs_log_trace("Attribute list sync failed (invalid size).\n");
				NInoAttrListSetDirty(ni);
			}
			ntfs_attr_close(na);
		}
	}

	/* Write this inode out to the $MFT (and $MFTMirr if applicable). */
//	if ( NInoTestAndClearDirty(ni) ) 
	if ( NInoTestAndClearDirty_func(ni) ) 
	{
		if (ntfs_mft_record_write(ni->vol, ni->mft_no, ni->mrec)) 
		{
			if (!err || errno == EIO) {
				err = errno;
				if (err != EIO)
					err = EBUSY;
			}

			NInoSetDirty(ni);
			ntfs_log_trace("Base MFT record sync failed.\n");
		}
	}

	/* If this is a base inode with extents write all dirty extents, too. */
	if (ni->nr_extents > 0) 
	{
		s32 i;

		for (i = 0; i < ni->nr_extents; ++i) 
		{
			ntfs_inode *eni;

			eni = ni->extent_nis[i];
		//	if ( NInoTestAndClearDirty(eni) ) 
			if ( NInoTestAndClearDirty_func(eni) )
			{
				if (ntfs_mft_record_write(eni->vol, eni->mft_no, eni->mrec)) 
				{
					if (!err || errno == EIO) {
						err = errno;
						if (err != EIO)
							err = EBUSY;
					}

					NInoSetDirty(eni);
					ntfs_log_trace("Extent MFT record sync failed.\n");
				}
			}
		}
	}

	if (!err) { return 0;}
	errno = err;
	return -1;
}

/**
 * ntfs_inode_add_attrlist - add attribute list to inode and fill it
 * @ni: opened ntfs inode to which add attribute list
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 * The following error codes are defined:
 *	EINVAL	- Invalid arguments were passed to the function.
 *	EEXIST	- Attribute list already exist.
 *	EIO	- Input/Ouput error occurred.
 *	ENOMEM	- Not enough memory to perform add.
 */
int ntfs_inode_add_attrlist(ntfs_inode *ni)
{
	int err;
	ntfs_attr_search_ctx *ctx;
	u8 *al = NULL, *aln;
	int al_len = 0;
	ATTR_LIST_ENTRY *ale = NULL;
	ntfs_attr *na;

	if (!ni) {
		ntfs_log_trace("Invalid arguments.\n");
		errno = EINVAL;
		return -1;
	}

	ntfs_log_trace("Entering for inode 0x%llx.\n", (LONG64) ni->mft_no);

	if (NInoAttrList(ni) || ni->nr_extents) {
		ntfs_log_trace("Inode already has got attribute list.\n");
		errno = EEXIST;
		return -1;
	}

	/* Form attribute list. */
	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx) {
		err = errno;
		ntfs_log_trace("Couldn't get search context.\n");
		goto err_out;
	}

	/* Walk through all attributes. */
	while (!ntfs_attr_lookup(AT_UNUSED, NULL, 0, 0, 0, NULL, 0, ctx)) 
	{
		
		int ale_size;
		
		if (ctx->attr->type == AT_ATTRIBUTE_LIST) 
		{
			err = EIO;
			ntfs_log_trace("Eeek! Attribute list already present.\n");
			goto put_err_out;
		}
		
		ale_size = (sizeof(ATTR_LIST_ENTRY) + sizeof(ntfschar) * ctx->attr->name_length + 7) & ~7;
		al_len += ale_size;
		
		aln = realloc(al, al_len);
		if (!aln) {
			err = errno;
			ntfs_log_perror("Failed to realloc %d bytes", al_len);
			goto put_err_out;
		}
		ale = (ATTR_LIST_ENTRY *)(aln + ((u8 *)ale - al));
		al = aln;
		
		memset(ale, 0, ale_size);
		
		/* Add attribute to attribute list. */
		ale->type = ctx->attr->type;
		ale->length = cpu_to_le16((sizeof(ATTR_LIST_ENTRY) +
			sizeof(ntfschar) * ctx->attr->name_length + 7) & ~7);
		ale->name_length = ctx->attr->name_length;
		ale->name_offset = (u8 *)ale->name - (u8 *)ale;
		if (ctx->attr->non_resident)
			ale->lowest_vcn = ctx->attr->X.lowest_vcn;
		else
			ale->lowest_vcn = 0;
		ale->mft_reference = MK_LE_MREF(ni->mft_no,
			le16_to_cpu(ni->mrec->sequence_number));
		ale->instance = ctx->attr->instance;
		memcpy(ale->name, (u8 *)ctx->attr +
				le16_to_cpu(ctx->attr->name_offset),
				ctx->attr->name_length * sizeof(ntfschar));
		ale = (ATTR_LIST_ENTRY *)(al + al_len);
	}
	/* Check for real error occurred. */
	if (errno != ENOENT) {
		err = errno;
		ntfs_log_trace("Attribute lookup failed.\n");
		goto put_err_out;
	}

	/* Set in-memory attribute list. */
	ni->attr_list = al;
	ni->attr_list_size = al_len;
	NInoSetAttrList(ni);
	NInoAttrListSetDirty(ni);

	/* Free space if there is not enough it for $ATTRIBUTE_LIST. */
	if (le32_to_cpu(ni->mrec->bytes_allocated) -
			le32_to_cpu(ni->mrec->bytes_in_use) <
			offsetof(ATTR_RECORD, X.resident_end)) {
		if (ntfs_inode_free_space(ni,
				offsetof(ATTR_RECORD, X.resident_end))) {
			/* Failed to free space. */
			err = errno;
			ntfs_log_trace("Failed to free space for $ATTRIBUTE_LIST.\n");
			goto rollback;
		}
	}

	/* Add $ATTRIBUTE_LIST to mft record. */
	if (ntfs_resident_attr_record_add(ni,
				AT_ATTRIBUTE_LIST, NULL, 0, NULL, 0, 0) < 0) {
		err = errno;
		ntfs_log_trace("Couldn't add $ATTRIBUTE_LIST to MFT record.\n");
		goto rollback;
	}

	/* Resize it. */
	na = ntfs_attr_open(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
	if (!na) {
		err = errno;
		ntfs_log_trace("Failed to open just added $ATTRIBUTE_LIST.\n");
		goto remove_attrlist_record;
	}
	if (ntfs_attr_truncate(na, al_len)) {
		err = errno;
		ntfs_log_trace("Failed to resize just added $ATTRIBUTE_LIST.\n");
		ntfs_attr_close(na);
		goto remove_attrlist_record;;
	}
	
	ntfs_attr_put_search_ctx(ctx);
	ntfs_attr_close(na);
	return 0;

remove_attrlist_record:
	/* Prevent ntfs_attr_recorm_rm from freeing attribute list. */
	ni->attr_list = NULL;
	NInoClearAttrList(ni);
	/* Remove $ATTRIBUTE_LIST record. */
	ntfs_attr_reinit_search_ctx(ctx);
	if (!ntfs_attr_lookup(AT_ATTRIBUTE_LIST, NULL, 0,
				CASE_SENSITIVE, 0, NULL, 0, ctx)) {
		if (ntfs_attr_record_rm(ctx))
			ntfs_log_trace("Rollback failed. Failed to remove attribute list record.\n");
	} else
		ntfs_log_trace("Rollback failed. Couldn't find attribute list record.\n");
	/* Setup back in-memory runlist. */
	ni->attr_list = al;
	ni->attr_list_size = al_len;
	NInoSetAttrList(ni);
rollback:
	/*
	 * Scan attribute list for attributes that placed not in the base MFT
	 * record and move them to it.
	 */
	ntfs_attr_reinit_search_ctx(ctx);
	ale = (ATTR_LIST_ENTRY*)al;
	while ((u8*)ale < al + al_len) 
	{
		if (MREF_LE(ale->mft_reference) != ni->mft_no)
		{
			if (!ntfs_attr_lookup(ale->type, ale->name,
						ale->name_length,
						CASE_SENSITIVE,
						sle64_to_cpu(ale->lowest_vcn),
						NULL, 0, ctx)) {
				if (ntfs_attr_record_move_to(ctx, ni))
					ntfs_log_trace("Rollback failed. Couldn't back attribute to base MFT record.\n");
			} else {
				ntfs_log_trace("Rollback failed. ntfs_attr_lookup failed.\n");
			}

			ntfs_attr_reinit_search_ctx(ctx);
		}

		ale = (ATTR_LIST_ENTRY*)((u8*)ale + le16_to_cpu(ale->length));
	}
	/* Remove in-memory attribute list. */
	ni->attr_list = NULL;
	ni->attr_list_size = 0;
	NInoClearAttrList(ni);
	NInoAttrListClearDirty(ni);
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
err_out:
	free(al);
	errno = err;
	return -1;
}



int 
ntfs_inode_free_space(
	IN ntfs_inode *ni, 
	IN int size
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/06 [6:3:2009 - 15:48]

Routine Description:
  free space in the MFT record of inode  
    
Arguments:
  ni - ntfs inode in which MFT record free space
  size - amount of space needed to free

Return Value:
  0 -- 成功; -1 -- 失败
    
--*/
{
	ntfs_attr_search_ctx *ctx;
	int freed, err;

	if (!ni || size < 0) {
		ntfs_log_trace("Invalid arguments.\n");
		errno = EINVAL;
		return -1;
	}

	ntfs_log_trace("Entering for inode 0x%llx, size %d.\n",(LONG64) ni->mft_no, size);

	freed = (le32_to_cpu(ni->mrec->bytes_allocated) - le32_to_cpu(ni->mrec->bytes_in_use));

	if (size <= freed)
		return 0;

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx) {
		err = errno;
		ntfs_log_trace("Failed to get attribute search context.\n");
		errno = err;
		return -1;
	}

	/*
	 * Chkdsk complain if $STANDARD_INFORMATION is not in the base MFT
	 * record. FIXME: I'm not sure in this, need to recheck. For now simply
	 * do not move $STANDARD_INFORMATION at all.
	 *
	 * Also we can't move $ATTRIBUTE_LIST from base MFT_RECORD, so position
	 * search context on first attribute after $STANDARD_INFORMATION and
	 * $ATTRIBUTE_LIST.
	 *
	 * Why we reposition instead of simply skip this attributes during
	 * enumeration? Because in case we have got only in-memory attribute
	 * list ntfs_attr_lookup will fail when it will try to find
	 * $ATTRIBUTE_LIST.
	 */
	if ( ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, CASE_SENSITIVE, 0, NULL, 0, ctx) )
	{
		if (errno != ENOENT) {
			err = errno;
			ntfs_log_trace("Attribute lookup failed.\n");
			goto put_err_out;
		}

		if (ctx->attr->type == AT_END) {
			err = ENOSPC;
			goto put_err_out;
		}
	}

	while (1) 
	{
		int record_size;

		/*
		 * Check whether attribute is from different MFT record. If so,
		 * find next, because we don't need such.
		 */
		while (ctx->ntfs_ino->mft_no != ni->mft_no) 
		{
			if (ntfs_attr_lookup(AT_UNUSED, NULL, 0, CASE_SENSITIVE,0, NULL, 0, ctx))
			{
				err = errno;
				if (errno != ENOENT) {
					ntfs_log_trace("Attribute lookup failed.\n");
				} else
					err = ENOSPC;
				goto put_err_out;
			}
		}

		record_size = le32_to_cpu(ctx->attr->length);

		/* Move away attribute. */
		if (ntfs_attr_record_move_away(ctx, 0)) {
			err = errno;
			ntfs_log_trace("Failed to move out attribute.\n");
			break;
		}
		freed += record_size;

		/* Check whether we done. */
		if (size <= freed) {
			ntfs_attr_put_search_ctx(ctx);
			return 0;
		}

		/*
		 * Reposition to first attribute after $STANDARD_INFORMATION and
		 * $ATTRIBUTE_LIST (see comments upwards).
		 */
		ntfs_attr_reinit_search_ctx(ctx);
		if (ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, CASE_SENSITIVE, 0, NULL, 0, ctx)) 
		{
			if (errno != ENOENT) {
				err = errno;
				ntfs_log_trace("Attribute lookup failed.\n");
				break;
			}
			if (ctx->attr->type == AT_END) {
				err = ENOSPC;
				break;
			}
		}
	}

put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	if (err == ENOSPC) { ntfs_log_trace("No attributes left that can be moved out.\n"); }
	errno = err;
	return -1;
}



void 
ntfs_inode_update_atime(
	IN ntfs_inode *ni
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/02 [2:3:2009 - 9:49]

Routine Description:
  更新该文件对应属性数据的访问时间. 不要更新系统文件的访问时间
    
--*/
{
	//
	// 要更新的MFT属性时间的要求:
	// 可写,有时间的概念,不是系统的前15个元数据(排除$5,根目录)
	//
	if (!NVolReadOnly( ni->vol ) 
		&& !NVolNoATime( ni->vol ) 
		&& ( ni->mft_no >= FILE_first_user || ni->mft_no == FILE_root )
	   ) 
	{
		ni->last_access_time = time(NULL);
		NInoFileNameSetDirty( ni );
		NInoSetDirty( ni );
	}

	return ;
}

/**
 * ntfs_inode_update_time - update all times for ntfs inode
 * @ni:		ntfs inode for which update times
 *
 * This function updates last access, mft and data change times. Usually
 * get called when user write not metadata to inode. Do not update time for
 * system files.
 */
void ntfs_inode_update_time(ntfs_inode *ni)
{
	if (!NVolReadOnly(ni->vol) && 
	    (ni->mft_no >= FILE_first_user || ni->mft_no == FILE_root)) {
		time_t now;

		now = time(NULL);
		ni->last_data_change_time = now;
		ni->last_mft_change_time = now;
		NInoFileNameSetDirty(ni);
		NInoSetDirty(ni);
	}
}

/**
 * ntfs_inode_badclus_bad - check for $Badclus:$Bad data attribute
 * @mft_no:		mft record number where @attr is present
 * @attr:		attribute record used to check for the $Bad attribute
 *
 * Check if the mft record given by @mft_no and @attr contains the bad sector
 * list. Please note that mft record numbers describing $Badclus extent inodes
 * will not match the current $Badclus:$Bad check.
 * 
 * On success return 1 if the file is $Badclus:$Bad, otherwise return 0.
 * On error return -1 with errno set to the error code.
 */
int ntfs_inode_badclus_bad(u64 mft_no, ATTR_RECORD *attr)
{
	int len, ret = 0;
	ntfschar *ustr;

	if (!attr) {
		ntfs_log_error("Invalid argument.\n");
		errno = EINVAL;
		return -1;
	}
	
	if (mft_no != FILE_BadClus)
	       	return 0;

	if (attr->type != AT_DATA)
	       	return 0;

	if ((ustr = ntfs_str2ucs("$Bad", &len)) == NULL) {
		ntfs_log_perror("Couldn't convert '$Bad' to Unicode");
		return -1;
	}

	if (ustr && ntfs_names_are_equal(ustr, len,
			(ntfschar *)((u8 *)attr + le16_to_cpu(attr->name_offset)),
			attr->name_length, 0, NULL, 0))
		ret = 1;

	ntfs_ucsfree(ustr);

	return ret;
}


//////////////////////////////////////////////////////////////////////////

BOOL NInoFileNameTestAndClearDirty_func( ntfs_inode *ni )
{
	BOOL old_state = FALSE ;

	old_state = test_bit( NI_FileNameDirty, ni->state );
	clear_bit( NI_FileNameDirty, ni->state );	

	return old_state;	
}


BOOL NInoAttrListTestAndClearDirty_func( ntfs_inode *ni )
{
	BOOL old_state = FALSE ;

	old_state = test_bit( NI_AttrListDirty, ni->state );
	clear_bit( NI_AttrListDirty, ni->state );	

	return old_state;	
}


BOOL NInoTestAndClearDirty_func( ntfs_inode *ni )
{
	BOOL old_state = FALSE ;

	old_state = test_bit( NI_Dirty, ni->state );
	clear_bit( NI_Dirty, ni->state );	

	return old_state;
}