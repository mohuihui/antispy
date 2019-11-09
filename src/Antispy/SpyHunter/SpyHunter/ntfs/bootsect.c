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
/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/03/03 [3:3:2009 - 15:02]
* MODULE : F:\Tmp\磁盘操作\code\ntfs-3g-1.0\libntfs-3g\bootsect.c
* 
* Description:
*   
*   分析主分区的MBR                     
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <stddef.h>

/*#include "layout.h*/
#include "compat.h"
#include "bootsect.h"
#include "debug.h"
#include "logging.h"


//////////////////////////////////////////////////////////////////////////


BOOL 
ntfs_boot_sector_is_ntfs(
	IN NTFS_BOOT_SECTOR *b
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 14:22]

Routine Description:
  检查这块Buffer是否是真的NTFS的MBR. Buffer应该 >= 512 bytes

Return Value:
  TRUE - 是合法的NTFS MBR;  FALSE - 错误
    
--*/
{
	u32 i;
	BOOL ret = FALSE;

	ntfs_log_debug("Beginning bootsector check.\n");
	ntfs_log_debug("Checking OEMid, NTFS signature.\n");

	// 检查"NTFS"标记
	if (b->oem_id != /*cpu_to_le64*/(0x202020205346544e)) { /* "NTFS    " */
		ntfs_log_error("NTFS signature is missing.\n");
		goto not_ntfs;
	}

	if ( *(PULONG)((char*)b+3) != 0x5346544e ) { goto not_ntfs; }
	if ( *(PULONG)((char*)b+7) != 0x20202020 ) { goto not_ntfs; }

	// 每个扇区的字节数范围应该为( 256~4096 ) 
//	ntfs_log_debug("Checking bytes per sector.\n");
	if (/*le16_to_cpu*/(b->bpb.bytes_per_sector) <  256 
		|| /*le16_to_cpu*/(b->bpb.bytes_per_sector) > 4096
	   ) 
	{
		ntfs_log_error("Unexpected bytes per sector value (%d).\n",  le16_to_cpu(b->bpb.bytes_per_sector));
		goto not_ntfs;
	}

	// 每个簇的扇区数只能是下面几种
//	ntfs_log_debug("Checking sectors per cluster.\n");
	switch (b->bpb.sectors_per_cluster) 
	{
	case 1: case 2: case 4: case 8: case 16: case 32: case 64: case 128:
		break;
	default:
		ntfs_log_error("Unexpected sectors per cluster value (%d).\n",b->bpb.sectors_per_cluster);
		goto not_ntfs;
	}

	// 一个簇的大小应该 <=65535
	ntfs_log_debug("Checking cluster size.\n");
	i = (u32)/*le16_to_cpu*/(b->bpb.bytes_per_sector) * b->bpb.sectors_per_cluster;
	if (i > 65536) {
		ntfs_log_error("Unexpected cluster size (%d).\n", i);
		goto not_ntfs;
	}

	// 检查BPB结构中的其他一些部分,必须为0
	ntfs_log_debug("Checking reserved fields are zero.\n");
	if (/*le16_to_cpu*/(b->bpb.reserved_sectors) 
		|| /*le16_to_cpu*/(b->bpb.root_entries) 
		|| /*le16_to_cpu*/(b->bpb.sectors) 
		|| /*le16_to_cpu*/(b->bpb.sectors_per_fat) 
		|| /*le32_to_cpu*/(b->bpb.large_sectors) 
		|| b->bpb.fats
	   ) 
	{
		ntfs_log_error("Reserved fields aren't zero "
			       "(%d, %d, %d, %d, %d, %d).\n",
			       le16_to_cpu(b->bpb.reserved_sectors),
			       le16_to_cpu(b->bpb.root_entries),
			       le16_to_cpu(b->bpb.sectors),
			       le16_to_cpu(b->bpb.sectors_per_fat),
			       le32_to_cpu(b->bpb.large_sectors),
			       b->bpb.fats);
		goto not_ntfs;
	}

	// 检查每个MFT所拥有的簇数
	ntfs_log_debug("Checking clusters per mft record.\n");
	if ((u8)b->clusters_per_mft_record < 0xe1
		|| (u8)b->clusters_per_mft_record > 0xf7
	   ) 
	{
		switch (b->clusters_per_mft_record) {
		case 1: case 2: case 4: case 8: case 0x10: case 0x20: case 0x40:
			break;
		default:
			ntfs_log_error("Unexpected clusters per mft record (%d).\n", b->clusters_per_mft_record);
			goto not_ntfs;
		}
	}

	// 检查
	ntfs_log_debug("Checking clusters per index block.\n");
	if ((u8)b->clusters_per_index_record < 0xe1 ||
	    (u8)b->clusters_per_index_record > 0xf7) {
		switch (b->clusters_per_index_record) {
		case 1: case 2: case 4: case 8: case 0x10: case 0x20: case 0x40:
			break;
		default:
			ntfs_log_error("Unexpected clusters per index record (%d).\n", b->clusters_per_index_record);
			goto not_ntfs;
		}
	}

	// MBR以 'AA55'结束
	if (b->end_of_sector_marker != cpu_to_le16(0xaa55))
		ntfs_log_debug("Warning: Bootsector has invalid end of sector marker.\n");

	ntfs_log_debug("Bootsector check completed successfully.\n");

	ret = TRUE;
not_ntfs:
	return ret;
}



int 
ntfs_boot_sector_parse(
	IN ntfs_volume *vol, 
	IN const NTFS_BOOT_SECTOR *bs
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/03 [3:3:2009 - 15:04]

Routine Description:
  setup an ntfs volume from an ntfs boot sector   

Return Value:
  0 -- 成功; -1 -- 失败
    
--*/
{
	s64 sectors;
	u8  sectors_per_cluster;
	s8  c;

	errno = EINVAL;

	vol->sector_size = le16_to_cpu(bs->bpb.bytes_per_sector); // 一般为512 bytes
	vol->sector_size_bits = ffs(vol->sector_size) - 1;
	ntfs_log_debug("SectorSize = 0x%x\n", vol->sector_size);
	ntfs_log_debug("SectorSizeBits = %u\n", vol->sector_size_bits);

	sectors_per_cluster = bs->bpb.sectors_per_cluster;
	ntfs_log_debug("SectorsPerCluster = 0x%x\n", sectors_per_cluster);
	if (sectors_per_cluster & (sectors_per_cluster - 1)) {
		ntfs_log_error("sectors_per_cluster (%d) is not a power of 2.\n", sectors_per_cluster);
		return -1;
	}
	
	sectors = sle64_to_cpu(bs->number_of_sectors);
	ntfs_log_debug("NumberOfSectors = %lld\n", sectors);
	if (!sectors) {
		ntfs_log_error("Volume size is set to zero.\n");
		return -1;
	}

	// unix函数
	if (vol->dev->d_ops->seek(
			vol->dev, (sectors - 1) << vol->sector_size_bits,SEEK_SET) 
			== -1
	   )
	{
		ntfs_log_error("Perhaps the volume is a RAID/LDM but it wasn't "
			       "setup yet, or the\nwrong device was used, "
			       "or the partition table is incorrect.\n" );
		return -1;
	}
	
	vol->nr_clusters =  sectors >> (ffs(sectors_per_cluster) - 1);

	vol->mft_lcn = sle64_to_cpu(bs->mft_lcn);
	vol->mftmirr_lcn = sle64_to_cpu(bs->mftmirr_lcn);
	ntfs_log_debug("MFT LCN = 0x%llx\n", vol->mft_lcn);
	ntfs_log_debug("MFTMirr LCN = 0x%llx\n", vol->mftmirr_lcn);

	if (vol->mft_lcn > vol->nr_clusters 
		|| vol->mftmirr_lcn > vol->nr_clusters
	   )
	{
		ntfs_log_error(
			"$MFT LCN (%lld) or $MFTMirr LCN (%lld) is greater than the number of clusters (%lld).\n",
			(LONG64)vol->mft_lcn, (LONG64)vol->mftmirr_lcn,
			(LONG64)vol->nr_clusters);
		
		return -1;
	}
	
	vol->cluster_size = sectors_per_cluster * vol->sector_size;
	if (vol->cluster_size & (vol->cluster_size - 1)) {
		ntfs_log_error("cluster_size (%d) is not a power of 2.\n", vol->cluster_size);
		return -1;
	}

	vol->cluster_size_bits = ffs(vol->cluster_size) - 1;

	c = bs->clusters_per_mft_record;
	ntfs_log_debug("ClusterSize = 0x%x\n", (unsigned)vol->cluster_size);
	ntfs_log_debug("ClusterSizeBits = %u\n", vol->cluster_size_bits);
	ntfs_log_debug("ClustersPerMftRecord = 0x%x\n", c);
	/*
	 * When clusters_per_mft_record is negative, it means that it is to
	 * be taken to be the negative base 2 logarithm of the mft_record_size
	 * min bytes. Then:
	 *	 mft_record_size = 2^(-clusters_per_mft_record) bytes.
	 */
	if (c < 0)
		vol->mft_record_size = 1 << -c;
	else
		vol->mft_record_size = c << vol->cluster_size_bits;
	if (vol->mft_record_size & (vol->mft_record_size - 1)) {
		ntfs_log_error("mft_record_size (%d) is not a power of 2.\n",
			       vol->mft_record_size);
		return -1;
	}

	vol->mft_record_size_bits = ffs(vol->mft_record_size) - 1;
	ntfs_log_debug("MftRecordSize = 0x%x\n", (unsigned)vol->mft_record_size);
	ntfs_log_debug("MftRecordSizeBits = %u\n", vol->mft_record_size_bits);
	
	c = bs->clusters_per_index_record;
	ntfs_log_debug("ClustersPerINDXRecord = 0x%x\n", c);
	if (c < 0)
		vol->indx_record_size = 1 << -c;
	else
		vol->indx_record_size = c << vol->cluster_size_bits;
	vol->indx_record_size_bits = ffs(vol->indx_record_size) - 1;
	ntfs_log_debug("INDXRecordSize = 0x%x\n", (unsigned)vol->indx_record_size);
	ntfs_log_debug("INDXRecordSizeBits = %u\n", vol->indx_record_size_bits);
	/*
	 * Work out the size of the MFT mirror in number of mft records. If the
	 * cluster size is less than or equal to the size taken by four mft
	 * records, the mft mirror stores the first four mft records. If the
	 * cluster size is bigger than the size taken by four mft records, the
	 * mft mirror contains as many mft records as will fit into one
	 * cluster.
	 */
	if (vol->cluster_size <= 4 * vol->mft_record_size)
		vol->mftmirr_size = 4;
	else
		vol->mftmirr_size = vol->cluster_size / vol->mft_record_size;
	return 0;
}

