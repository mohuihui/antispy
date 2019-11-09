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
 * mft.c - Mft record handling code. Originated from the Linux-NTFS project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <time.h>
#include <stddef.h>

#include "compat.h"
#include "types.h"
#include "device.h"
#include "debug.h"
#include "bitmap.h"
#include "attrib.h"
#include "inode.h"
#include "volume.h"
#include "layout.h"
#include "lcnalloc.h"
#include "mft.h"
#include "logging.h"
#include "misc.h"

#pragma warning( disable: 4244 )

//////////////////////////////////////////////////////////////////////////


int 
ntfs_mft_records_read(
	IN const ntfs_volume *vol,
	IN const MFT_REF mref,
	IN const s64 count,
	OUT MFT_RECORD *b
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/01 [1:3:2009 - 14:40]

Routine Description:
  read records from the mft from disk. If any of the records exceed the initialized
  size of the $MFT/$DATA attribute, i.e. they cannot possibly be allocated mft records,
  assume this is a bug and return error code ESPIPE.
    
Arguments:
  vol - 该卷结构体的一个指针
  mref - 要在该卷中查找的MFT
  count - 要读取的MFT的数量
  b - 保存查找到的MFT,其副本的地址

Return Value:
  0 - 成功; -1 - 失败
    
--*/
{
	s64 br;
	VCN m;

	ntfs_log_trace("Entering for inode 0x%llx.\n", MREF(mref));
	if (!vol || !vol->mft_na || !b || count < 0) {
		errno = EINVAL;
		return -1;
	}
	m = MREF(mref);

	// Refuse to read non-allocated mft records.  
	if ( m + count > vol->mft_na->initialized_size >> vol->mft_record_size_bits ) 
	{
		errno = ESPIPE;
		ntfs_log_perror(
			"Trying to read non-allocated mft records (%lld > %lld)",
			(LONG64)m + count,
			(LONG64)vol->mft_na->initialized_size >> vol->mft_record_size_bits );

		return -1;
	}

	// 关键的一句调用
	br = ntfs_attr_mst_pread( vol->mft_na, m << vol->mft_record_size_bits, count, vol->mft_record_size, b );
	if ( br != count ) 
	{
		if (br != -1) errno = EIO;
		if (br >= 0) ntfs_log_debug("Error: partition is smaller than it should be!\n");
		else ntfs_log_perror("Error reading $Mft record(s)");

		return -1;
	}

	return 0;
}

/**
 * ntfs_mft_records_write - write mft records to disk
 * @vol:	volume to write to
 * @mref:	starting mft record number to write
 * @count:	number of mft records to write
 * @b:		data buffer containing the mft records to write
 *
 * Write @count mft records starting at @mref from data buffer @b to volume
 * @vol. Return 0 on success or -1 on error, with errno set to the error code.
 *
 * If any of the records exceed the initialized size of the $MFT/$DATA
 * attribute, i.e. they cannot possibly be allocated mft records, assume this
 * is a bug and return error code ESPIPE.
 *
 * Before the mft records are written, they are mst protected. After the write,
 * they are deprotected again, thus resulting in an increase in the update
 * sequence number inside the data buffer @b.
 *
 * If any mft records are written which are also represented in the mft mirror
 * $MFTMirr, we make a copy of the relevant parts of the data buffer @b into a
 * temporary buffer before we do the actual write. Then if at least one mft
 * record was successfully written, we write the appropriate mft records from
 * the copied buffer to the mft mirror, too.
 */
int ntfs_mft_records_write(const ntfs_volume *vol, const MFT_REF mref,
		const s64 count, MFT_RECORD *b)
{
	s64 bw;
	VCN m;
	void *bmirr = NULL;
	int cnt = 0, res = 0;

	ntfs_log_trace("Entering for inode 0x%llx.\n", MREF(mref));
	if (!vol || !vol->mft_na || vol->mftmirr_size <= 0 || !b || count < 0) {
		errno = EINVAL;
		return -1;
	}
	m = MREF(mref);
	/* Refuse to write non-allocated mft records. */
	if (m + count > vol->mft_na->initialized_size >>
			vol->mft_record_size_bits) {
		errno = ESPIPE;
		ntfs_log_perror("Trying to write non-allocated mft records "
				"(%lld > %lld)", (LONG64)m + count,
				(LONG64)vol->mft_na->initialized_size >>
				vol->mft_record_size_bits);
		return -1;
	}
	if (m < vol->mftmirr_size) {
		if (!vol->mftmirr_na) {
			errno = EINVAL;
			return -1;
		}
		cnt = vol->mftmirr_size - m;
		if (cnt > count)
			cnt = count;
		bmirr = (void *) ntfs_malloc(cnt * vol->mft_record_size);
		if (!bmirr)
			return -1;
		memcpy(bmirr, b, cnt * vol->mft_record_size);
	}
	bw = ntfs_attr_mst_pwrite(vol->mft_na, m << vol->mft_record_size_bits,
			count, vol->mft_record_size, b);
	if (bw != count) {
		if (bw != -1)
			errno = EIO;
		if (bw >= 0)
			ntfs_log_debug("Error: partial write while writing $Mft "
					"record(s)!\n");
		else
			ntfs_log_perror("Error writing $Mft record(s)");
		res = errno;
	}
	if (bmirr && bw > 0) {
		if (bw < cnt)
			cnt = bw;
		bw = ntfs_attr_mst_pwrite(vol->mftmirr_na,
				m << vol->mft_record_size_bits, cnt,
				vol->mft_record_size, bmirr);
		if (bw != cnt) {
			if (bw != -1)
				errno = EIO;
			ntfs_log_debug("Error: failed to sync $MFTMirr! Run "
					"chkdsk.\n");
			res = errno;
		}
	}
	free(bmirr);
	if (!res)
		return res;
	errno = res;
	return -1;
}



int 
ntfs_file_record_read(
	IN const ntfs_volume *vol,
	IN const MFT_REF mref,
	OUT MFT_RECORD **mrec, 
	OUT ATTR_RECORD **attr
	)
/*++

Routine Description:
  从指定的分区卷中读取指定的MFT,分配一块内存存储之,并将其地址存入到结构体的
  一个指针中. 若参数4 @attr存在,那么它将保存该MFT中第一个属性的地址.
  当处理完毕后,调用者需释放*@mrec所指向的内存.

  The read mft record is checked for having the magic FILE,
  and for having a matching sequence number (if MSEQNO(*@mref) != 0).
  If either of these fails, -1 is returned and errno is set to EIO. If you get
  this, but you still want to read the mft record (e.g. in order to correct
  it), use ntfs_mft_record_read() directly.
    
Arguments:
  vol - 该卷结构体的一个指针
  mref - 要在该卷中查找的MFT
  mrec - 保存查找到的MFT,其副本的地址
  attr - 保存查找到的MFT中第一个属性的地址

Return Value:
  0 - 成功; -1 - 失败
    
--*/
{
	MFT_RECORD *m;
	ATTR_RECORD *a;
	int err;

	if (!vol || !mrec) {
		errno = EINVAL;
		return -1;
	}
	m = *mrec;
	if (!m) // 若原来的这个指针没有申请内存,则申请一份
	{
		m = (MFT_RECORD *) ntfs_malloc(vol->mft_record_size);
		if ( !m ) { return -1; }
	}

	if ( ntfs_mft_record_read(vol, mref, m) )  // 关键的一句调用
	{
		err = errno;
		goto read_failed;
	}

	if ( !ntfs_is_file_record(m->magic) ) { goto file_corrupt; } // 验证是否为MFT
	if ( MSEQNO(mref) && MSEQNO(mref) != le16_to_cpu(m->sequence_number) ) { goto file_corrupt; } // 当前MFT所代表的文件正被占用

	a = (ATTR_RECORD*)((char*)m + le16_to_cpu(m->attrs_offset)); // 得到该MFT第一个属性的地址
	if ( p2n(a) < p2n(m) || (char*)a > (char*)m + vol->mft_record_size ) { goto file_corrupt; }

	*mrec = m;
	if (attr) { *attr = a; } // 若参数4存在,则将该MFT第一个属性的地址 赋给这个指针
	return 0; // 成功返回

file_corrupt:
	ntfs_log_debug("ntfs_file_record_read(): file is corrupt.\n");
	err = EIO;
read_failed:
	if (m != *mrec) { free(m); }
	errno = err;
	return -1; // 返回失败
}


/**
 * ntfs_mft_record_layout - layout an mft record into a memory buffer
 * @vol:	volume to which the mft record will belong
 * @mref:	mft reference specifying the mft record number
 * @mrec:	destination buffer of size >= @vol->mft_record_size bytes
 *
 * Layout an empty, unused mft record with the mft reference @mref into the
 * buffer @m.  The volume @vol is needed because the mft record structure was
 * modified in NTFS 3.1 so we need to know which volume version this mft record
 * will be used on.
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 */
int ntfs_mft_record_layout(const ntfs_volume *vol, const MFT_REF mref,
		MFT_RECORD *mrec)
{
	ATTR_RECORD *a;

	if (!vol || !mrec) {
		errno = EINVAL;
		return -1;
	}
	/* Aligned to 2-byte boundary. */
	if (vol->major_ver < 3 || (vol->major_ver == 3 && !vol->minor_ver))
		mrec->usa_ofs = cpu_to_le16((sizeof(MFT_RECORD_OLD) + 1) & ~1);
	else {
		/* Abort if mref is > 32 bits. */
		if (MREF(mref) & 0x0000ffff00000000) {
			ntfs_log_debug("Mft reference exceeds 32 bits!\n");
			errno = ERANGE;
			return -1;
		}
		mrec->usa_ofs = cpu_to_le16((sizeof(MFT_RECORD) + 1) & ~1);
		/*
		 * Set the NTFS 3.1+ specific fields while we know that the
		 * volume version is 3.1+.
		 */
		mrec->reserved = cpu_to_le16(0);
		mrec->mft_record_number = cpu_to_le32(MREF(mref));
	}
	mrec->magic = magic_FILE;
	if (vol->mft_record_size >= NTFS_BLOCK_SIZE)
		mrec->usa_count = cpu_to_le16(vol->mft_record_size /
				NTFS_BLOCK_SIZE + 1);
	else {
		mrec->usa_count = cpu_to_le16(1);
		ntfs_log_error("Sector size is bigger than MFT record size.  "
				"Setting usa_count to 1.  If Windows chkdsk "
				"reports this as corruption, please email %s "
				"stating that you saw this message and that "
				"the file system created was corrupt.  "
				"Thank you.\n", NTFS_DEV_LIST);
	}
	/* Set the update sequence number to 1. */
	*(u16*)((u8*)mrec + le16_to_cpu(mrec->usa_ofs)) = cpu_to_le16(1);
	mrec->lsn = cpu_to_le64(0);
	mrec->sequence_number = cpu_to_le16(1);
	mrec->link_count = cpu_to_le16(0);
	/* Aligned to 8-byte boundary. */
	mrec->attrs_offset = cpu_to_le16((le16_to_cpu(mrec->usa_ofs) +
			(le16_to_cpu(mrec->usa_count) << 1) + 7) & ~7);
	mrec->flags = cpu_to_le16(0);
	/*
	 * Using attrs_offset plus eight bytes (for the termination attribute),
	 * aligned to 8-byte boundary.
	 */
	mrec->bytes_in_use = cpu_to_le32((le16_to_cpu(mrec->attrs_offset) + 8 +
			7) & ~7);
	mrec->bytes_allocated = cpu_to_le32(vol->mft_record_size);
	mrec->base_mft_record = cpu_to_le64((MFT_REF)0);
	mrec->next_attr_instance = cpu_to_le16(0);
	a = (ATTR_RECORD*)((u8*)mrec + le16_to_cpu(mrec->attrs_offset));
	a->type = AT_END;
	a->length = cpu_to_le32(0);
	/* Finally, clear the unused part of the mft record. */
	memset((u8*)a + 8, 0, vol->mft_record_size - ((u8*)a + 8 - (u8*)mrec));
	return 0;
}

/**
 * ntfs_mft_record_format - format an mft record on an ntfs volume
 * @vol:	volume on which to format the mft record
 * @mref:	mft reference specifying mft record to format
 *
 * Format the mft record with the mft reference @mref in $MFT/$DATA, i.e. lay
 * out an empty, unused mft record in memory and write it to the volume @vol.
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 */
int ntfs_mft_record_format(const ntfs_volume *vol, const MFT_REF mref)
{
	MFT_RECORD *m;
	int err;

	if (!vol || !vol->mft_na) {
		errno = EINVAL;
		return -1;
	}
	m = ntfs_calloc(vol->mft_record_size);
	if (!m)
		return -1;
	if (ntfs_mft_record_layout(vol, mref, m)) {
		err = errno;
		free(m);
		errno = err;
		return -1;
	}
	if (ntfs_mft_record_write(vol, mref, m)) {
		err = errno;
		free(m);
		errno = err;
		return -1;
	}
	free(m);
	return 0;
}

static const char *es = "  Leaving inconsistent metadata.  Run chkdsk.";

/**
 * ntfs_ffz - Find the first unset (zero) bit in a word
 * @word:
 *
 * Description...
 *
 * Returns:
 */
unsigned int ntfs_ffz(unsigned int word)
{
	return ffs(~word) - 1;
}

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/**
 * ntfs_mft_bitmap_find_free_rec - find a free mft record in the mft bitmap
 * @vol:	volume on which to search for a free mft record
 * @base_ni:	open base inode if allocating an extent mft record or NULL
 *
 * Search for a free mft record in the mft bitmap attribute on the ntfs volume
 * @vol.
 *
 * If @base_ni is NULL start the search at the default allocator position.
 *
 * If @base_ni is not NULL start the search at the mft record after the base
 * mft record @base_ni.
 *
 * Return the free mft record on success and -1 on error with errno set to the
 * error code.  An error code of ENOSPC means that there are no free mft
 * records in the currently initialized mft bitmap.
 */
static int ntfs_mft_bitmap_find_free_rec(ntfs_volume *vol, ntfs_inode *base_ni)
{
	s64 pass_end, ll, data_pos, pass_start, ofs, bit;
	ntfs_attr *mftbmp_na;
	u8 *buf, *byte;
	unsigned int size;
	u8 pass, b;
	u8 dwLittleOffset ;

	mftbmp_na = vol->mftbmp_na;
	/*
	 * Set the end of the pass making sure we do not overflow the mft
	 * bitmap.
	 */
	size = PAGE_SIZE;
	pass_end = vol->mft_na->allocated_size >> vol->mft_record_size_bits;
	ll = mftbmp_na->initialized_size << 3;
	if (pass_end > ll)
		pass_end = ll;
	pass = 1;
	if (!base_ni)
		data_pos = vol->mft_data_pos;
	else
		data_pos = base_ni->mft_no + 1;
	if (data_pos < 24)
		data_pos = 24;
	if (data_pos >= pass_end) {
		data_pos = 24;
		pass = 2;
		/* This happens on a freshly formatted volume. */
		if (data_pos >= pass_end) {
			errno = ENOSPC;
			return -1;
		}
	}
	pass_start = data_pos;
	buf = (u8 *) ntfs_malloc(PAGE_SIZE);
	if (!buf)
		return -1;
	
	ntfs_log_debug("Starting bitmap search: pass %u, pass_start 0x%llx, "
			"pass_end 0x%llx, data_pos 0x%llx.\n", pass,
			(LONG64)pass_start, (LONG64)pass_end,
			(LONG64)data_pos);
#ifdef DEBUG
	byte = NULL;
	b = 0;
#endif
	/* Loop until a free mft record is found. */
	for (; pass <= 2; size = PAGE_SIZE) 
	{
		/* Cap size to pass_end. */
		ofs = data_pos >> 3;
		ll = ((pass_end + 7) >> 3) - ofs;
		if (size > ll)
			size = ll;

	
		// offset必须是一个扇区大小(0x200 - 512 )的倍数. 
		dwLittleOffset = 0 ;
		dwLittleOffset = (u8)(ofs & 0xff) ;

		// 读出来的是512的倍数,而不是要求的size大小
		ll = ntfs_attr_pread(mftbmp_na, ofs, size, buf);
		if (ll < 0) {
			ntfs_log_error("Failed to read mft bitmap attribute, aborting.\n");
			free(buf);
			return -1;
		}
		ntfs_log_debug("Read 0x%llx bytes.\n", (LONG64)ll);

		buf += dwLittleOffset;

		/* If we read at least one byte, search @buf for a zero bit. */
		if (ll) 
		{
		//	size = ll << 3;
			size = size << 3;
			bit = data_pos & 7;
			data_pos &= ~7;
// 			ntfs_log_debug("Before inner for loop: size 0x%x, "
// 					"data_pos 0x%llx, bit 0x%llx, "
// 					"*byte 0x%hhx, b %u.\n", size,
// 					(LONG64)data_pos, (LONG64)bit,
// 					byte ? *byte : -1, b);

			for (; bit < size && data_pos + bit < pass_end; bit &= ~7, bit += 8) 
			{
				byte = buf + (bit >> 3);
				if (*byte == 0xff)
					continue;

				/* Note: ffz() result must be zero based. */
				b = ntfs_ffz((unsigned long)*byte);
				if (b < 8 && b >= (bit & 7)) {
					buf -= dwLittleOffset;
					free(buf);
					return data_pos + (bit & ~7) + b;
				}
			}
			ntfs_log_debug("After inner for loop: size 0x%x, "
					"data_pos 0x%llx, bit 0x%llx, "
					"*byte 0x%hhx, b %u.\n", size,
					(LONG64)data_pos, (LONG64)bit,
					byte ? *byte : -1, b);

			data_pos += size;
			/*
			 * If the end of the pass has not been reached yet,
			 * continue searching the mft bitmap for a zero bit.
			 */
			if (data_pos < pass_end)
				continue;
		}
		/* Do the next pass. */
		pass++;
		if (pass == 2) 
		{
			/*
			 * Starting the second pass, in which we scan the first
			 * part of the zone which we omitted earlier.
			 */
			pass_end = pass_start;
			data_pos = pass_start = 24;
			ntfs_log_debug("pass %i, pass_start 0x%llx, pass_end 0x%llx.\n", pass, (LONG64)pass_start,(LONG64)pass_end);
			if (data_pos >= pass_end)
				break;
		}
	}
	/* No free mft records in currently initialized mft bitmap. */
	free(buf);
	errno = ENOSPC;
	return -1;
}

/**
 * ntfs_mft_bitmap_extend_allocation - extend mft bitmap attribute by a cluster
 * @vol:	volume on which to extend the mft bitmap attribute
 *
 * Extend the mft bitmap attribute on the ntfs volume @vol by one cluster.
 *
 * Note:  Only changes allocated_size, i.e. does not touch initialized_size or
 * data_size.
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
static int ntfs_mft_bitmap_extend_allocation(ntfs_volume *vol)
{
	LCN lcn;
	s64 ll = 0; /* silence compiler warning */
	ntfs_attr *mftbmp_na, *lcnbmp_na;
	runlist_element *rl, *rl2 = NULL; /* silence compiler warning */
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m = NULL; /* silence compiler warning */
	ATTR_RECORD *a = NULL; /* silence compiler warning */
	int ret, mp_size;
	u32 old_alen = 0; /* silence compiler warning */
	u8 b, tb;
	struct {
		u8 added_cluster:1;
		u8 added_run:1;
		u8 mp_rebuilt:1;
	} status = { 0, 0, 0 };

	mftbmp_na = vol->mftbmp_na;
	lcnbmp_na = vol->lcnbmp_na;
	/*
	 * Determine the last lcn of the mft bitmap.  The allocated size of the
	 * mft bitmap cannot be zero so we are ok to do this.
	 */
	rl = ntfs_attr_find_vcn(mftbmp_na, (mftbmp_na->allocated_size - 1) >>
			vol->cluster_size_bits);
	if (!rl || !rl->length || rl->lcn < 0) {
		ntfs_log_error("Failed to determine last allocated "
				"cluster of mft bitmap attribute.\n");
		if (rl)
			errno = EIO;
		return -1;
	}
	lcn = rl->lcn + rl->length;
	/*
	 * Attempt to get the cluster following the last allocated cluster by
	 * hand as it may be in the MFT zone so the allocator would not give it
	 * to us.
	 */
	ret = (int)ntfs_attr_pread(lcnbmp_na, lcn >> 3, 1, &b);
	if (ret < 0) {
		ntfs_log_error("Failed to read from lcn bitmap.\n");
		return -1;
	}
	ntfs_log_debug("Read %i byte%s.\n", ret, ret == 1 ? "" : "s");
	tb = 1 << (lcn & 7);
	if (ret == 1 && b != 0xff && !(b & tb)) {
		/* Next cluster is free, allocate it. */
		b |= tb;
		ret = (int)ntfs_attr_pwrite(lcnbmp_na, lcn >> 3, 1, &b);
		if (ret < 1) {
			ntfs_log_error("Failed to write to lcn "
					"bitmap.\n");
			if (!ret)
				errno = EIO;
			return -1;
		}
		/* Update the mft bitmap runlist. */
		rl->length++;
		rl[1].vcn++;
		status.added_cluster = 1;
		ntfs_log_debug("Appending one cluster to mft bitmap.\n");
	} else {
		/* Allocate a cluster from the DATA_ZONE. */
		rl2 = ntfs_cluster_alloc(vol, rl[1].vcn, 1, lcn, DATA_ZONE);
		if (!rl2) {
			ntfs_log_error("Failed to allocate a cluster for "
					"the mft bitmap.\n");
			return -1;
		}
		rl = ntfs_runlists_merge(mftbmp_na->rl, rl2);
		if (!rl) {
			ret = errno;
			ntfs_log_error("Failed to merge runlists for mft "
					"bitmap.\n");
			if (ntfs_cluster_free_from_rl(vol, rl2))
				ntfs_log_error("Failed to deallocate "
						"cluster.%s\n", es);
			free(rl2);
			errno = ret;
			return -1;
		}
		mftbmp_na->rl = rl;
		status.added_run = 1;
		ntfs_log_debug("Adding one run to mft bitmap.\n");
		/* Find the last run in the new runlist. */
		for (; rl[1].length; rl++)
			;
	}
	/*
	 * Update the attribute record as well.  Note: @rl is the last
	 * (non-terminator) runlist element of mft bitmap.
	 */
	ctx = ntfs_attr_get_search_ctx(mftbmp_na->ni, NULL);
	if (!ctx) {
		ntfs_log_error("Failed to get search context.\n");
		goto undo_alloc;
	}
	if (ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			mftbmp_na->name_len, 0, rl[1].vcn, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find last attribute extent of "
				"mft bitmap attribute.\n");
		goto undo_alloc;
	}
	m = ctx->mrec;
	a = ctx->attr;
	ll = sle64_to_cpu(a->X.lowest_vcn);
	rl2 = ntfs_attr_find_vcn(mftbmp_na, ll);
	if (!rl2 || !rl2->length) {
		ntfs_log_error("Failed to determine previous last "
				"allocated cluster of mft bitmap attribute.\n");
		if (rl2)
			errno = EIO;
		goto undo_alloc;
	}
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll);
	if (mp_size <= 0) {
		ntfs_log_error("Get size for mapping pairs failed for "
				"mft bitmap attribute extent.\n");
		goto undo_alloc;
	}
	/* Expand the attribute record if necessary. */
	old_alen = le32_to_cpu(a->length);
	if (ntfs_attr_record_resize(m, a, mp_size +
			le16_to_cpu(a->X.mapping_pairs_offset))) {
		// TODO: Deal with this by moving this extent to a new mft
		// record or by starting a new extent in a new mft record.
		ntfs_log_error("Not enough space in this mft record to "
				"accommodate extended mft bitmap attribute "
				"extent.  Cannot handle this yet.\n");
		errno = /*EOPNOTSUPP*/1045;
		goto undo_alloc;
	}
	status.mp_rebuilt = 1;
	/* Generate the mapping pairs array directly into the attr record. */
	if (ntfs_mapping_pairs_build(vol, (u8*)a +
			le16_to_cpu(a->X.mapping_pairs_offset), mp_size, rl2, ll,NULL)) 
	{
		ntfs_log_error("Failed to build mapping pairs array for mft bitmap attribute.\n");
		errno = EIO;
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->X.highest_vcn = cpu_to_sle64(rl[1].vcn - 1);
	/*
	 * We now have extended the mft bitmap allocated_size by one cluster.
	 * Reflect this in the ntfs_attr structure and the attribute record.
	 */
	if (a->X.lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		if (ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
				mftbmp_na->name_len, 0, 0, NULL, 0, ctx)) {
			ntfs_log_error("Failed to find first attribute "
					"extent of mft bitmap attribute.\n");
			goto restore_undo_alloc;
		}
		a = ctx->attr;
	}
	mftbmp_na->allocated_size += vol->cluster_size;
	a->X.allocated_size = cpu_to_sle64(mftbmp_na->allocated_size);
	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	return 0;
restore_undo_alloc:
	ret = errno;
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			mftbmp_na->name_len, 0, rl[1].vcn, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find last attribute extent of "
				"mft bitmap attribute.%s\n", es);
		ntfs_attr_put_search_ctx(ctx);
		mftbmp_na->allocated_size += vol->cluster_size;
		/*
		 * The only thing that is now wrong is ->allocated_size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		errno = ret;
		return -1;
	}
	m = ctx->mrec;
	a = ctx->attr;
	a->X.highest_vcn = cpu_to_sle64(rl[1].vcn - 2);
	errno = ret;
undo_alloc:
	ret = errno;
	if (status.added_cluster) {
		/* Truncate the last run in the runlist by one cluster. */
		rl->length--;
		rl[1].vcn--;
	} else if (status.added_run) {
		lcn = rl->lcn;
		/* Remove the last run from the runlist. */
		rl->lcn = rl[1].lcn;
		rl->length = 0;
	}
	/* Deallocate the cluster. */
	if (ntfs_bitmap_clear_bit(lcnbmp_na, lcn))
		ntfs_log_error("Failed to free cluster.%s\n", es);
	if (status.mp_rebuilt) 
	{
		if (ntfs_mapping_pairs_build(vol, (u8*)a +
				le16_to_cpu(a->X.mapping_pairs_offset),
				old_alen - le16_to_cpu(a->X.mapping_pairs_offset),
				rl2, ll, NULL))
			ntfs_log_error("Failed to restore mapping pairs array.%s\n", es);

		if (ntfs_attr_record_resize(m, a, old_alen))
			ntfs_log_error("Failed to restore attribute record.%s\n", es);

		ntfs_inode_mark_dirty(ctx->ntfs_ino);
	}

	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	errno = ret;
	return -1;
}

/**
 * ntfs_mft_bitmap_extend_initialized - extend mft bitmap initialized data
 * @vol:	volume on which to extend the mft bitmap attribute
 *
 * Extend the initialized portion of the mft bitmap attribute on the ntfs
 * volume @vol by 8 bytes.
 *
 * Note:  Only changes initialized_size and data_size, i.e. requires that
 * allocated_size is big enough to fit the new initialized_size.
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
static int ntfs_mft_bitmap_extend_initialized(ntfs_volume *vol)
{
	s64 old_data_size, old_initialized_size, ll;
	ntfs_attr *mftbmp_na;
	ntfs_attr_search_ctx *ctx;
	ATTR_RECORD *a;
	int err;

	mftbmp_na = vol->mftbmp_na;
	ctx = ntfs_attr_get_search_ctx(mftbmp_na->ni, NULL);
	if (!ctx) {
		ntfs_log_error("Failed to get search context.\n");
		return -1;
	}
	if (ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			mftbmp_na->name_len, 0, 0, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find first attribute extent of "
				"mft bitmap attribute.\n");
		err = errno;
		goto put_err_out;
	}
	a = ctx->attr;
	old_data_size = mftbmp_na->data_size;
	old_initialized_size = mftbmp_na->initialized_size;
	mftbmp_na->initialized_size += 8;
	a->X.initialized_size = cpu_to_sle64(mftbmp_na->initialized_size);
	if (mftbmp_na->initialized_size > mftbmp_na->data_size) {
		mftbmp_na->data_size = mftbmp_na->initialized_size;
		a->X.data_size = cpu_to_sle64(mftbmp_na->data_size);
	}
	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	/* Initialize the mft bitmap attribute value with zeroes. */
	ll = 0;
	ll = ntfs_attr_pwrite(mftbmp_na, old_initialized_size, 8, &ll);
	if (ll == 8) {
		ntfs_log_debug("Wrote eight initialized bytes to mft bitmap.\n");
		return 0;
	}
	ntfs_log_error("Failed to write to mft bitmap.\n");
	err = errno;
	if (ll >= 0)
		err = EIO;
	/* Try to recover from the error. */
	ctx = ntfs_attr_get_search_ctx(mftbmp_na->ni, NULL);
	if (!ctx) {
		ntfs_log_error("Failed to get search context.%s\n", es);
		goto err_out;
	}
	if (ntfs_attr_lookup(mftbmp_na->type, mftbmp_na->name,
			mftbmp_na->name_len, 0, 0, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find first attribute extent of "
				"mft bitmap attribute.%s\n", es);
put_err_out:
		ntfs_attr_put_search_ctx(ctx);
		goto err_out;
	}
	a = ctx->attr;
	mftbmp_na->initialized_size = old_initialized_size;
	a->X.initialized_size = cpu_to_sle64(old_initialized_size);
	if (mftbmp_na->data_size != old_data_size) {
		mftbmp_na->data_size = old_data_size;
		a->X.data_size = cpu_to_sle64(old_data_size);
	}
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	ntfs_log_debug("Restored status of mftbmp: allocated_size 0x%llx, "
			"data_size 0x%llx, initialized_size 0x%llx.\n",
			(LONG64)mftbmp_na->allocated_size,
			(LONG64)mftbmp_na->data_size,
			(LONG64)mftbmp_na->initialized_size);
err_out:
	errno = err;
	return -1;
}

/**
 * ntfs_mft_data_extend_allocation - extend mft data attribute
 * @vol:	volume on which to extend the mft data attribute
 *
 * Extend the mft data attribute on the ntfs volume @vol by 16 mft records
 * worth of clusters or if not enough space for this by one mft record worth
 * of clusters.
 *
 * Note:  Only changes allocated_size, i.e. does not touch initialized_size or
 * data_size.
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
static int ntfs_mft_data_extend_allocation(ntfs_volume *vol)
{
	LCN lcn;
	VCN old_last_vcn;
	s64 min_nr, nr, ll = 0; /* silence compiler warning */
	ntfs_attr *mft_na;
	runlist_element *rl, *rl2;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m = NULL; /* silence compiler warning */
	ATTR_RECORD *a = NULL; /* silence compiler warning */
	int err, mp_size;
	u32 old_alen = 0; /* silence compiler warning */
	BOOL mp_rebuilt = FALSE;

	ntfs_log_debug("Extending mft data allocation.\n");
	mft_na = vol->mft_na;
	/*
	 * Determine the preferred allocation location, i.e. the last lcn of
	 * the mft data attribute.  The allocated size of the mft data
	 * attribute cannot be zero so we are ok to do this.
	 */
	rl = ntfs_attr_find_vcn(mft_na,
			(mft_na->allocated_size - 1) >> vol->cluster_size_bits);
	if (!rl || !rl->length || rl->lcn < 0) {
		ntfs_log_error("Failed to determine last allocated "
				"cluster of mft data attribute.\n");
		if (rl)
			errno = EIO;
		return -1;
	}
	lcn = rl->lcn + rl->length;
	ntfs_log_debug("Last lcn of mft data attribute is 0x%llx.\n", (LONG64)lcn);
	/* Minimum allocation is one mft record worth of clusters. */
	min_nr = vol->mft_record_size >> vol->cluster_size_bits;
	if (!min_nr)
		min_nr = 1;
	/* Want to allocate 16 mft records worth of clusters. */
	nr = vol->mft_record_size << 4 >> vol->cluster_size_bits;
	if (!nr)
		nr = min_nr;
	ntfs_log_debug("Trying mft data allocation with default cluster count "
			"%lli.\n", (LONG64)nr);
	old_last_vcn = rl[1].vcn;
	do {
		rl2 = ntfs_cluster_alloc(vol, old_last_vcn, nr, lcn, MFT_ZONE);
		if (rl2)
			break;
		if (errno != ENOSPC || nr == min_nr) {
			ntfs_log_error("Failed to allocate the minimal "
					"number of clusters (%lli) for the "
					"mft data attribute.\n", (LONG64)nr);
			return -1;
		}
		/*
		 * There is not enough space to do the allocation, but there
		 * might be enough space to do a minimal allocation so try that
		 * before failing.
		 */
		nr = min_nr;
		ntfs_log_debug("Retrying mft data allocation with minimal cluster "
				"count %lli.\n", (LONG64)nr);
	} while (1);
	rl = ntfs_runlists_merge(mft_na->rl, rl2);
	if (!rl) {
		err = errno;
		ntfs_log_error("Failed to merge runlists for mft data "
				"attribute.\n");
		if (ntfs_cluster_free_from_rl(vol, rl2))
			ntfs_log_error("Failed to deallocate clusters "
					"from the mft data attribute.%s\n", es);
		free(rl2);
		errno = err;
		return -1;
	}
	mft_na->rl = rl;
	ntfs_log_debug("Allocated %lli clusters.\n", nr);
	/* Find the last run in the new runlist. */
	for (; rl[1].length; rl++)
		;
	/* Update the attribute record as well. */
	ctx = ntfs_attr_get_search_ctx(mft_na->ni, NULL);
	if (!ctx) {
		ntfs_log_error("Failed to get search context.\n");
		goto undo_alloc;
	}
	if (ntfs_attr_lookup(mft_na->type, mft_na->name, mft_na->name_len, 0,
			rl[1].vcn, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find last attribute extent of "
				"mft data attribute.\n");
		goto undo_alloc;
	}
	m = ctx->mrec;
	a = ctx->attr;
	ll = sle64_to_cpu(a->X.lowest_vcn);
	rl2 = ntfs_attr_find_vcn(mft_na, ll);
	if (!rl2 || !rl2->length) {
		ntfs_log_error("Failed to determine previous last "
				"allocated cluster of mft data attribute.\n");
		if (rl2)
			errno = EIO;
		goto undo_alloc;
	}
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll);
	if (mp_size <= 0) {
		ntfs_log_error("Get size for mapping pairs failed for "
				"mft data attribute extent.\n");
		goto undo_alloc;
	}
	/* Expand the attribute record if necessary. */
	old_alen = le32_to_cpu(a->length);
	if (ntfs_attr_record_resize(m, a,
			mp_size + le16_to_cpu(a->X.mapping_pairs_offset))) {
		// TODO: Deal with this by moving this extent to a new mft
		// record or by starting a new extent in a new mft record.
		// Note: Use the special reserved mft records and ensure that
		// this extent is not required to find the mft record in
		// question.
		errno = 1045;
		ntfs_log_perror("Not enough space to extended mft data "
				"attribute.\n");
		goto undo_alloc;
	}
	mp_rebuilt = TRUE;
	/*
	 * Generate the mapping pairs array directly into the attribute record.
	 */
	if (ntfs_mapping_pairs_build(vol,
			(u8*)a + le16_to_cpu(a->X.mapping_pairs_offset), mp_size,
			rl2, ll, NULL)) 
	{
		ntfs_log_error("Failed to build mapping pairs array of mft data attribute.\n");
		errno = EIO;
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->X.highest_vcn = cpu_to_sle64(rl[1].vcn - 1);
	/*
	 * We now have extended the mft data allocated_size by nr clusters.
	 * Reflect this in the ntfs_attr structure and the attribute record.
	 * @rl is the last (non-terminator) runlist element of mft data
	 * attribute.
	 */
	if (a->X.lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		if (ntfs_attr_lookup(mft_na->type, mft_na->name,
				mft_na->name_len, 0, 0, NULL, 0, ctx)) 
		{
			ntfs_log_error("Failed to find first attribute extent of mft data attribute.\n");
			goto restore_undo_alloc;
		}
		a = ctx->attr;
	}
	mft_na->allocated_size += nr << vol->cluster_size_bits;
	a->X.allocated_size = cpu_to_sle64(mft_na->allocated_size);
	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	return 0;
restore_undo_alloc:
	err = errno;
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(mft_na->type, mft_na->name, mft_na->name_len, 0,
			rl[1].vcn, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find last attribute extent of "
				"mft data attribute.%s\n", es);
		ntfs_attr_put_search_ctx(ctx);
		mft_na->allocated_size += nr << vol->cluster_size_bits;
		/*
		 * The only thing that is now wrong is ->allocated_size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		errno = err;
		return -1;
	}
	m = ctx->mrec;
	a = ctx->attr;
	a->X.highest_vcn = cpu_to_sle64(old_last_vcn - 1);
	errno = err;
undo_alloc:
	err = errno;
	if (ntfs_cluster_free(vol, mft_na, old_last_vcn, -1) < 0)
		ntfs_log_error("Failed to free clusters from mft data "
				"attribute.%s\n", es);
	if (ntfs_rl_truncate(&mft_na->rl, old_last_vcn))
		ntfs_log_error("Failed to truncate mft data attribute "
				"runlist.%s\n", es);
	if (mp_rebuilt) {
		if (ntfs_mapping_pairs_build(vol, (u8*)a +
				le16_to_cpu(a->X.mapping_pairs_offset),
				old_alen - le16_to_cpu(a->X.mapping_pairs_offset),
				rl2, ll, NULL))
			ntfs_log_error("Failed to restore mapping pairs "
					"array.%s\n", es);
		if (ntfs_attr_record_resize(m, a, old_alen))
			ntfs_log_error("Failed to restore attribute "
					"record.%s\n", es);
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	errno = err;
	return -1;
}

/**
 * ntfs_mft_record_alloc - allocate an mft record on an ntfs volume
 * @vol:	volume on which to allocate the mft record
 * @base_ni:	open base inode if allocating an extent mft record or NULL
 *
 * Allocate an mft record in $MFT/$DATA of an open ntfs volume @vol.
 *
 * If @base_ni is NULL make the mft record a base mft record and allocate it at
 * the default allocator position.
 *
 * If @base_ni is not NULL make the allocated mft record an extent record,
 * allocate it starting at the mft record after the base mft record and attach
 * the allocated and opened ntfs inode to the base inode @base_ni.
 *
 * On success return the now opened ntfs (extent) inode of the mft record.
 *
 * On error return NULL with errno set to the error code.
 *
 * To find a free mft record, we scan the mft bitmap for a zero bit.  To
 * optimize this we start scanning at the place specified by @base_ni or if
 * @base_ni is NULL we start where we last stopped and we perform wrap around
 * when we reach the end.  Note, we do not try to allocate mft records below
 * number 24 because numbers 0 to 15 are the defined system files anyway and 16
 * to 24 are special in that they are used for storing extension mft records
 * for the $DATA attribute of $MFT.  This is required to avoid the possibility
 * of creating a run list with a circular dependence which once written to disk
 * can never be read in again.  Windows will only use records 16 to 24 for
 * normal files if the volume is completely out of space.  We never use them
 * which means that when the volume is really out of space we cannot create any
 * more files while Windows can still create up to 8 small files.  We can start
 * doing this at some later time, it does not matter much for now.
 *
 * When scanning the mft bitmap, we only search up to the last allocated mft
 * record.  If there are no free records left in the range 24 to number of
 * allocated mft records, then we extend the $MFT/$DATA attribute in order to
 * create free mft records.  We extend the allocated size of $MFT/$DATA by 16
 * records at a time or one cluster, if cluster size is above 16kiB.  If there
 * is not sufficient space to do this, we try to extend by a single mft record
 * or one cluster, if cluster size is above the mft record size, but we only do
 * this if there is enough free space, which we know from the values returned
 * by the failed cluster allocation function when we tried to do the first
 * allocation.
 *
 * No matter how many mft records we allocate, we initialize only the first
 * allocated mft record, incrementing mft data size and initialized size
 * accordingly, open an ntfs_inode for it and return it to the caller, unless
 * there are less than 24 mft records, in which case we allocate and initialize
 * mft records until we reach record 24 which we consider as the first free mft
 * record for use by normal files.
 *
 * If during any stage we overflow the initialized data in the mft bitmap, we
 * extend the initialized size (and data size) by 8 bytes, allocating another
 * cluster if required.  The bitmap data size has to be at least equal to the
 * number of mft records in the mft, but it can be bigger, in which case the
 * superfluous bits are padded with zeroes.
 *
 * Thus, when we return successfully (return value non-zero), we will have:
 *	- initialized / extended the mft bitmap if necessary,
 *	- initialized / extended the mft data if necessary,
 *	- set the bit corresponding to the mft record being allocated in the
 *	  mft bitmap,
 *	- open an ntfs_inode for the allocated mft record, and we will
 *	- return the ntfs_inode.
 *
 * On error (return value zero), nothing will have changed.  If we had changed
 * anything before the error occurred, we will have reverted back to the
 * starting state before returning to the caller.  Thus, except for bugs, we
 * should always leave the volume in a consistent state when returning from
 * this function.
 *
 * Note, this function cannot make use of most of the normal functions, like
 * for example for attribute resizing, etc, because when the run list overflows
 * the base mft record and an attribute list is used, it is very important that
 * the extension mft records used to store the $DATA attribute of $MFT can be
 * reached without having to read the information contained inside them, as
 * this would make it impossible to find them in the first place after the
 * volume is dismounted.  $MFT/$BITMAP probably does not need to follow this
 * rule because the bitmap is not essential for finding the mft records, but on
 * the other hand, handling the bitmap in this special way would make life
 * easier because otherwise there might be circular invocations of functions
 * when reading the bitmap but if we are careful, we should be able to avoid
 * all problems.
 */
ntfs_inode *ntfs_mft_record_alloc(ntfs_volume *vol, ntfs_inode *base_ni)
{
	s64 ll, bit, old_data_initialized, old_data_size;
	ntfs_attr *mft_na, *mftbmp_na;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	ntfs_inode *ni;
	int err;
	u16 seq_no, usn;

	if (base_ni)
		ntfs_log_trace("Entering (allocating an extent mft record for base mft record 0x%llx).\n",(LONG64)base_ni->mft_no);
	else
		ntfs_log_trace("Entering (allocating a base mft record).\n");

	if (!vol || !vol->mft_na || !vol->mftbmp_na) {
		errno = EINVAL;
		return NULL;
	}

	mft_na = vol->mft_na;
	mftbmp_na = vol->mftbmp_na;

	// 空闲的位图
	bit = ntfs_mft_bitmap_find_free_rec(vol, base_ni);
	if (bit >= 0) {
		ntfs_log_debug("Found free record (#1), bit 0x%llx.\n",(LONG64)bit);
		goto found_free_rec;
	}
	if (errno != ENOSPC)
		return NULL;
	/*
	 * No free mft records left.  If the mft bitmap already covers more
	 * than the currently used mft records, the next records are all free,
	 * so we can simply allocate the first unused mft record.
	 * Note: We also have to make sure that the mft bitmap at least covers
	 * the first 24 mft records as they are special and whilst they may not
	 * be in use, we do not allocate from them.
	 */
	ll = mft_na->initialized_size >> vol->mft_record_size_bits;
	if (mftbmp_na->initialized_size << 3 > ll &&
			mftbmp_na->initialized_size > 3) {
		bit = ll;
		if (bit < 24)
			bit = 24;
		ntfs_log_debug("Found free record (#2), bit 0x%llx.\n",(LONG64)bit);
		goto found_free_rec;
	}
	/*
	 * The mft bitmap needs to be expanded until it covers the first unused
	 * mft record that we can allocate.
	 * Note: The smallest mft record we allocate is mft record 24.
	 */
	ntfs_log_debug("Status of mftbmp before extension: allocated_size 0x%llx, "
			"data_size 0x%llx, initialized_size 0x%llx.\n",
			(LONG64)mftbmp_na->allocated_size,
			(LONG64)mftbmp_na->data_size,
			(LONG64)mftbmp_na->initialized_size);
	if (mftbmp_na->initialized_size + 8 > mftbmp_na->allocated_size) {
		/* Need to extend bitmap by one more cluster. */
		ntfs_log_debug("mftbmp: initialized_size + 8 > allocated_size.\n");
		if (ntfs_mft_bitmap_extend_allocation(vol))
			goto err_out;
		ntfs_log_debug("Status of mftbmp after allocation extension: "
				"allocated_size 0x%llx, data_size 0x%llx, "
				"initialized_size 0x%llx.\n",
				(LONG64)mftbmp_na->allocated_size,
				(LONG64)mftbmp_na->data_size,
				(LONG64)mftbmp_na->initialized_size);
	}
	/*
	 * We now have sufficient allocated space, extend the initialized_size
	 * as well as the data_size if necessary and fill the new space with
	 * zeroes.
	 */
	bit = mftbmp_na->initialized_size << 3;
	if (ntfs_mft_bitmap_extend_initialized(vol))
		goto err_out;
	ntfs_log_debug("Status of mftbmp after initialized extension: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.\n",
			(LONG64)mftbmp_na->allocated_size,
			(LONG64)mftbmp_na->data_size,
			(LONG64)mftbmp_na->initialized_size);
	ntfs_log_debug("Found free record (#3), bit 0x%llx.\n", (LONG64)bit);
found_free_rec:
	/* @bit is the found free mft record, allocate it in the mft bitmap. */
	ntfs_log_debug("At found_free_rec.\n");
	if (ntfs_bitmap_set_bit(mftbmp_na, bit)) {
		ntfs_log_error("Failed to allocate bit in mft bitmap.\n");
		goto err_out;
	}
	ntfs_log_debug("Set bit 0x%llx in mft bitmap.\n", (LONG64)bit);
	/* The mft bitmap is now uptodate.  Deal with mft data attribute now. */
	ll = (bit + 1) << vol->mft_record_size_bits;
	if (ll <= mft_na->initialized_size) {
		ntfs_log_debug("Allocated mft record already initialized.\n");
		goto mft_rec_already_initialized;
	}
	ntfs_log_debug("Initializing allocated mft record.\n");
	/*
	 * The mft record is outside the initialized data.  Extend the mft data
	 * attribute until it covers the allocated record.  The loop is only
	 * actually traversed more than once when a freshly formatted volume is
	 * first written to so it optimizes away nicely in the common case.
	 */
	ntfs_log_debug("Status of mft data before extension: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.\n",
			(LONG64)mft_na->allocated_size,
			(LONG64)mft_na->data_size,
			(LONG64)mft_na->initialized_size);
	while (ll > mft_na->allocated_size) {
		if (ntfs_mft_data_extend_allocation(vol))
			goto undo_mftbmp_alloc;
		ntfs_log_debug("Status of mft data after allocation extension: "
				"allocated_size 0x%llx, data_size 0x%llx, "
				"initialized_size 0x%llx.\n",
				(LONG64)mft_na->allocated_size,
				(LONG64)mft_na->data_size,
				(LONG64)mft_na->initialized_size);
	}
	old_data_initialized = mft_na->initialized_size;
	old_data_size = mft_na->data_size;
	/*
	 * Extend mft data initialized size (and data size of course) to reach
	 * the allocated mft record, formatting the mft records along the way.
	 * Note: We only modify the ntfs_attr structure as that is all that is
	 * needed by ntfs_mft_record_format().  We will update the attribute
	 * record itself in one fell swoop later on.
	 */
	while (ll > mft_na->initialized_size) {
		s64 ll2 = mft_na->initialized_size >> vol->mft_record_size_bits;
		mft_na->initialized_size += vol->mft_record_size;
		if (mft_na->initialized_size > mft_na->data_size)
			mft_na->data_size = mft_na->initialized_size;
		ntfs_log_debug("Initializing mft record 0x%llx.\n", (LONG64)ll2);
		err = ntfs_mft_record_format(vol, ll2);
		if (err) {
			ntfs_log_error("Failed to format mft record.\n");
			goto undo_data_init;
		}
	}
	/* Update the mft data attribute record to reflect the new sizes. */
	ctx = ntfs_attr_get_search_ctx(mft_na->ni, NULL);
	if (!ctx) {
		ntfs_log_error("Failed to get search context.\n");
		goto undo_data_init;
	}
	if (ntfs_attr_lookup(mft_na->type, mft_na->name, mft_na->name_len, 0,
			0, NULL, 0, ctx)) {
		ntfs_log_error("Failed to find first attribute extent of "
				"mft data attribute.\n");
		ntfs_attr_put_search_ctx(ctx);
		goto undo_data_init;
	}
	a = ctx->attr;
	a->X.initialized_size = cpu_to_sle64(mft_na->initialized_size);
	a->X.data_size = cpu_to_sle64(mft_na->data_size);
	/* Ensure the changes make it to disk. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	ntfs_log_debug("Status of mft data after mft record initialization: "
			"allocated_size 0x%llx, data_size 0x%llx, "
			"initialized_size 0x%llx.\n",
			(LONG64)mft_na->allocated_size,
			(LONG64)mft_na->data_size,
			(LONG64)mft_na->initialized_size);
	/* Sanity checks. */
	if (mft_na->data_size > mft_na->allocated_size || mft_na->initialized_size > mft_na->data_size)
		ntfs_log_error("mft_na sanity checks failed");
	// BUG_ON(mft_na->initialized_size > mft_na->data_size);
	// BUG_ON(mft_na->data_size > mft_na->allocated_size);
	/* Sync MFT to minimize data loss if there won't be clean unmount. */
	if (ntfs_inode_sync(mft_na->ni)) {
		ntfs_log_error("Failed to sync $MFT.");
		goto undo_data_init;
	}
mft_rec_already_initialized:
	/*
	 * We now have allocated and initialized the mft record.  Need to read
	 * it from disk and re-format it, preserving the sequence number if it
	 * is not zero as well as the update sequence number if it is not zero
	 * or -1 (0xffff).
	 */
	m = (MFT_RECORD *) ntfs_malloc(vol->mft_record_size);
	if (!m)
		goto undo_mftbmp_alloc;
	
	if (ntfs_mft_record_read(vol, bit, m)) {
		err = errno;
		ntfs_log_error("Failed to read mft record.\n");
		free(m);
		errno = err;
		goto undo_mftbmp_alloc;
	}
	/* Sanity check that the mft record is really not in use. */
	if (ntfs_is_file_record(m->magic) && (m->flags & MFT_RECORD_IN_USE)) {
		ntfs_log_error("Mft record 0x%llx was marked unused in "
				"mft bitmap but is marked used itself.  "
				"Corrupt filesystem or library bug!  "
				"Run chkdsk immediately!\n", (LONG64)bit);
		free(m);
		errno = EIO;
		goto undo_mftbmp_alloc;
	}
	seq_no = m->sequence_number;
	usn = *(u16*)((u8*)m + le16_to_cpu(m->usa_ofs));
	if (ntfs_mft_record_layout(vol, bit, m)) {
		err = errno;
		ntfs_log_error("Failed to re-format mft record.\n");
		free(m);
		errno = err;
		goto undo_mftbmp_alloc;
	}
	if (le16_to_cpu(seq_no))
		m->sequence_number = seq_no;
	seq_no = le16_to_cpu(usn);
	if (seq_no && seq_no != 0xffff)
		*(u16*)((u8*)m + le16_to_cpu(m->usa_ofs)) = usn;
	/* Set the mft record itself in use. */
	m->flags |= MFT_RECORD_IN_USE;
	/* Now need to open an ntfs inode for the mft record. */
	ni = ntfs_inode_allocate(vol);
	if (!ni) {
		err = errno;
		ntfs_log_error("Failed to allocate buffer for inode.\n");
		free(m);
		errno = err;
		goto undo_mftbmp_alloc;
	}
	ni->mft_no = bit;
	ni->mrec = m;
	/*
	 * If we are allocating an extent mft record, make the opened inode an
	 * extent inode and attach it to the base inode.  Also, set the base
	 * mft record reference in the extent inode.
	 */
	if (base_ni) {
		ni->nr_extents = -1;
		ni->base_ni = base_ni;
		m->base_mft_record = MK_LE_MREF(base_ni->mft_no,
				le16_to_cpu(base_ni->mrec->sequence_number));
		/*
		 * Attach the extent inode to the base inode, reallocating
		 * memory if needed.
		 */
		if (!(base_ni->nr_extents & 3)) {
			ntfs_inode **extent_nis;
			int i;

			i = (base_ni->nr_extents + 4) * sizeof(ntfs_inode *);
			extent_nis = (ntfs_inode **) ntfs_malloc(i);
			if (!extent_nis) {
				free(m);
				free(ni);
				goto undo_mftbmp_alloc;
			}
			if (base_ni->extent_nis) {
				memcpy(extent_nis, base_ni->extent_nis,
						i - 4 * sizeof(ntfs_inode *));
				free(base_ni->extent_nis);
			}
			base_ni->extent_nis = extent_nis;
		}
		base_ni->extent_nis[base_ni->nr_extents++] = ni;
	}
	/* Make sure the allocated inode is written out to disk later. */
	ntfs_inode_mark_dirty(ni);
	/* Initialize time, allocated and data size in ntfs_inode struct. */
	ni->data_size = ni->allocated_size = 0;
	ni->flags = 0;
	ni->creation_time = ni->last_data_change_time =
			ni->last_mft_change_time =
			ni->last_access_time = time(NULL);
	/* Update the default mft allocation position if it was used. */
	if (!base_ni)
		vol->mft_data_pos = bit + 1;
	/* Return the opened, allocated inode of the allocated mft record. */
	ntfs_log_debug("Returning opened, allocated %sinode 0x%llx.\n",
			base_ni ? "extent " : "", (LONG64)bit);
	return ni;
undo_data_init:
	mft_na->initialized_size = old_data_initialized;
	mft_na->data_size = old_data_size;
undo_mftbmp_alloc:
	err = errno;
	if (ntfs_bitmap_clear_bit(mftbmp_na, bit))
		ntfs_log_error("Failed to clear bit in mft bitmap.%s\n", es);
	errno = err;
err_out:
	if (!errno)
		errno = EIO;
	return NULL;
}



int 
ntfs_mft_record_free(
	IN ntfs_volume *vol,
	IN ntfs_inode *ni
	)
{
	u64 mft_no;
	int err;
	u16 seq_no, old_seq_no;

	ntfs_log_trace("Entering for inode 0x%llx.\n", (LONG64) ni->mft_no);
	if (!vol || !vol->mftbmp_na || !ni) {
		errno = EINVAL;
		return -1;
	}

	mft_no = ni->mft_no;

	// 在本文件MFT偏移16H处 标记为0 -- 表示已删除
	ni->mrec->flags &= ~MFT_RECORD_IN_USE;

	// Increment the sequence number, skipping zero, if it is not zero.
	old_seq_no = ni->mrec->sequence_number;
	seq_no = le16_to_cpu(old_seq_no);
	if (seq_no == 0xffff)
		seq_no = 1;
	else if (seq_no)
		seq_no++;
	ni->mrec->sequence_number = cpu_to_le16(seq_no);

	//
	// 真正的写入磁盘
	//

	ntfs_inode_mark_dirty(ni);
	if (ntfs_inode_sync(ni)) {
		err = errno;
		goto sync_rollback;
	}

	//
	// Clear the bit in the $MFT/$BITMAP corresponding to this record.
	// 清理掉该MFT在bitmap中的标记 -- 设置为0表示该MFT未被使用
	//

	if (ntfs_bitmap_clear_bit(vol->mftbmp_na, mft_no)) {
		err = errno;
		goto bitmap_rollback;
	}

	// 收尾工作
	if (!ntfs_inode_close(ni))
		return 0;
	err = errno;

bitmap_rollback:
	if (ntfs_bitmap_set_bit(vol->mftbmp_na, mft_no))
		ntfs_log_debug("Eeek! Rollback failed in ntfs_mft_record_free().  Leaving inconsistent metadata!\n");
sync_rollback:
	ni->mrec->flags |= MFT_RECORD_IN_USE;
	ni->mrec->sequence_number = old_seq_no;
	ntfs_inode_mark_dirty(ni);
	errno = err;
	return -1;
}

/**
 * ntfs_mft_usn_dec - Decrement USN by one
 * @mrec:	pointer to an mft record
 *
 * On success return 0 and on error return -1 with errno set.
 */
int ntfs_mft_usn_dec(MFT_RECORD *mrec)
{
	u16 usn, *usnp;

	if (!mrec) {
		errno = EINVAL;
		return -1;
	}
	usnp = (u16 *)((char *)mrec + le16_to_cpu(mrec->usa_ofs));
	usn = le16_to_cpup(usnp);
	if (usn-- <= 1)
		usn = 0xfffe;
	*usnp = cpu_to_le16(usn);

	return 0;
}

