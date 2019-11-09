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
#pragma once

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

//////////////////////////////////////////////////////////////////////////

// 保存磁盘的物理信息，例如磁头数，柱面数等
typedef struct {
	HANDLE handle;
	s64 pos;		/* Logical current position on the volume. */
	s64 part_start;
	s64 part_length;
	int part_hidden_sectors;
	s64 geo_size, geo_cylinders;
	DWORD geo_sectors, geo_heads;
	HANDLE vol_handle;
} win32_fd;



//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


/*extern*/ int ntfs_device_win32_open(struct ntfs_device *dev, int flags);

/*extern*/ int ntfs_device_win32_close(struct ntfs_device *dev);

/*extern*/ s64 ntfs_device_win32_seek(struct ntfs_device *dev, s64 offset, int whence);

/*extern*/ s64 ntfs_device_win32_read(struct ntfs_device *dev, void *b, s64 count);

/*extern*/ s64 ntfs_device_win32_write(struct ntfs_device *dev, const void *b, s64 count);

/*extern*/ s64 ntfs_device_win32_pread(struct ntfs_device *dev, void *b, s64 count, s64 offset);

/*extern*/ s64 ntfs_device_win32_pwrite(struct ntfs_device *dev, const void *b, s64 count, s64 offset);

/*extern*/ int ntfs_device_win32_sync(struct ntfs_device *dev);

/*extern*/ int ntfs_device_win32_ioctl(struct ntfs_device *dev, int request, void *argp);

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////

