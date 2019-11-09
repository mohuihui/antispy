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
 * bitmap.c - Bitmap handling code. Originated from the Linux-NTFS project.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <stddef.h>


#include "types.h"
#include "attrib.h"
#include "bitmap.h"
#include "debug.h"
#include "logging.h"
#include "misc.h"


#pragma warning( disable: 4244 4761 )

//////////////////////////////////////////////////////////////////////////



/**
 * ntfs_bit_set - set a bit in a field of bits
 * @bitmap:	field of bits
 * @bit:	bit to set
 * @new_value:	value to set bit to (0 or 1)
 *
 * Set the bit @bit in the @bitmap to @new_value. Ignore all errors.
 */
void ntfs_bit_set(u8 *bitmap, const u64 bit, const u8 new_value)
{
	if (!bitmap || new_value > 1)
		return;
	if (!new_value)
		bitmap[bit >> 3] &= ~(1 << (bit & 7));
	else
		bitmap[bit >> 3] |= (1 << (bit & 7));
}

/**
 * ntfs_bit_get - get value of a bit in a field of bits
 * @bitmap:	field of bits
 * @bit:	bit to get
 *
 * Get and return the value of the bit @bit in @bitmap (0 or 1).
 * Return -1 on error.
 */
char ntfs_bit_get(const u8 *bitmap, const u64 bit)
{
	if (!bitmap)
		return -1;
	return (bitmap[bit >> 3] >> (bit & 7)) & 1;
}

/**
 * ntfs_bit_get_and_set - get value of a bit in a field of bits and set it
 * @bitmap:	field of bits
 * @bit:	bit to get/set
 * @new_value:	value to set bit to (0 or 1)
 *
 * Return the value of the bit @bit and set it to @new_value (0 or 1).
 * Return -1 on error.
 */
char ntfs_bit_get_and_set(u8 *bitmap, const u64 bit, const u8 new_value)
{
	register u8 old_bit, shift;

	if (!bitmap || new_value > 1)
		return -1;
	shift = bit & 7;
	old_bit = (bitmap[bit >> 3] >> shift) & 1;
	if (new_value != old_bit)
		bitmap[bit >> 3] ^= 1 << shift;
	return old_bit;
}



int 
ntfs_bitmap_set_bits_in_run(
	IN ntfs_attr *na, 
	IN s64 start_bit,
	IN s64 count, 
	IN int value
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/03/13 [13:3:2009 - 10:28]

Routine Description:
  设置指定属性的bitmap位. 参数4(@value)为0表示该属性将被设置为未使用(即标记为删除);
  为1表示该属性将被设置为使用中(即新增文件会用到)
    
Arguments:
  na - 该结构体中包含bitmap
  start_bit - bitmap中指定要设置的起始位置
  count - 要设置的bit数
  value - 0 表示标记为删除; 1表示该属性将被设置为使用中

Return Value:
  0 - 成功; -1 - 失败
    
--*/
{
	s64 bufsize, br;
	u8 *pBufTmp_bark, *pBuf, *buf, *lastbyte_buf, bLittleOffset ;
	int bit, firstbyte, lastbyte, lastbyte_pos, tmp, err, nSectors, wsssssss;

	if (!na || start_bit < 0 || count < 0) { // 参数合法性检测
		errno = EINVAL;
		return -1;
	}

	// 查看此处bit的底3位是否存在
	bit = start_bit & 7;
	if (bit)
		firstbyte = 1;
	else
		firstbyte = 0;

	// Calculate the required buffer size in bytes, capping it at 8kiB.
	bufsize = ((count - (bit ? 8 - bit : 0) + 7) >> 3) + firstbyte;
	if (bufsize > 8192) { bufsize = 8192; }

	// 读写扇区都只能是512的倍数.所以分配起来也一样.计算要读取的扇区数
	wsssssss = ((DWORD)bufsize % 512) > 0 ? 1 : 0 ;
	nSectors =  (DWORD)bufsize / 512 + wsssssss ;

	buf = (u8 *)ntfs_malloc( nSectors * 512 );
	pBuf = buf ;
	if (!buf) { return -1; }
	
	// 初始化内存
	memset(buf, value ? 0xff : 0, nSectors * 512);

	// 存放原始的读出来的数据,不会被修改
	pBufTmp_bark = (u8 *)ntfs_malloc( nSectors * 512 );
	if (!pBufTmp_bark) { return -1; }

	if (bit) 
	{
		// 只读 8-bit, 即一个字节(char)的内容,但ReadFile只能读取512的倍数.
		br = ntfs_attr_pread( na, start_bit >> 3, 1, buf );
		if (br != 512) {
			free(buf);
			errno = EIO;
			return -1;
		}

		// 拷贝一份内容到备份内存中去
		memcpy( (PVOID)pBufTmp_bark, (PVOID)buf, nSectors * 512 );

		//
		// 定位到实际要读的地方
		//
		bLittleOffset = 0 ;
		bLittleOffset = (start_bit >> 3) & 0xff ;
		buf += bLittleOffset ;

		// and set or clear the appropriate bits in it.
		while ( (bit & 7) && count-- ) 
		{
			if (value) { // 标记为已被使用
				*buf |= 1 << bit++;

			} else { // 标记为已被删除

				*buf &= ~(1 << bit++);
			}
		}

		// Update @start_bit to the new position.
		start_bit = (start_bit + 7) & ~7;
	}

	// Loop until @count reaches zero.
	lastbyte = 0;
	lastbyte_buf = NULL;
	bit = count & 7;

	do {

		if (count > 0 && bit) 
		{
			lastbyte_pos = ((count + 7) >> 3) + firstbyte;
			if (!lastbyte_pos) {
				ntfs_log_trace("Eeek! lastbyte is zero. Leaving inconsistent metadata.\n");
				err = EIO;
				goto free_err_out;
			}

			// and it is in the currently loaded bitmap window... 
			if (lastbyte_pos <= bufsize) 
			{
				lastbyte_buf = buf + lastbyte_pos - 1;
				bLittleOffset = ((start_bit + count) >> 3) & 0xff ;

				memcpy( 
					(PVOID)lastbyte_buf, 
					(PVOID)(pBufTmp_bark + bLittleOffset), 
					sizeof(u8) 
					);

// 				br = ntfs_attr_pread( na, (start_bit + count) >> 3, 1, lastbyte_buf );
// 				if (br != 512) {
// 					ntfs_log_trace("Eeek! Read of last byte failed. Leaving inconsistent metadata.\n");
// 					err = EIO;
// 					goto free_err_out;
// 				}

				while (bit && count--) 
				{
					if (value)
						*lastbyte_buf |= 1 << --bit;
					else
						*lastbyte_buf &= ~(1 << --bit);
				}

				// We don't want to come back here... 
				bit = 0;

				// We have a last byte that we have handled.
				lastbyte = 1;
			}
		}

		//
		// unix下的函数可以只写一个字节.但是windows下必须写512的整数倍
		// 那么这里其实只要求写一个bit, 就是置$bitmap中的某个空闲位为FF
		// 但之前读的Buff就是一个扇区的大小, 我们只改变了这个Buff中的
		// 1-bit,现在把这块内容写回去就行了.
		// 

		tmp = (start_bit >> 3) - firstbyte;
		br = ntfs_attr_pwrite( na, tmp, bufsize, pBuf );
		if (br != 512) {
			ntfs_log_trace("Eeek! Failed to write buffer to bitmap. Leaving inconsistent metadata.\n");
			err = EIO;
			goto free_err_out;
		}

		// Update counters.
		tmp = (bufsize - firstbyte - lastbyte) << 3;
		if (firstbyte) 
		{
			firstbyte = 0;

			// Re-set the partial first byte so a subsequent write of the buffer does not have stale, incorrect bits.
			*buf = value ? 0xff : 0;
		}

		start_bit += tmp;
		count -= tmp;
		if (bufsize > (tmp = (count + 7) >> 3))
			bufsize = tmp;

		if (lastbyte && count != 0) {
			ntfs_log_trace("Eeek! Last buffer but count is not zero (= %lli). Leaving inconsistent metadata.\n",(LONG64)count);
			err = EIO;
			goto free_err_out;
		}

	} while (count > 0);

	// OK!
	free(pBuf);
	return 0;

free_err_out:
	free(pBuf);
	errno = err;
	return -1;
}

/**
 * ntfs_bitmap_set_run - set a run of bits in a bitmap
 * @na:		attribute containing the bitmap
 * @start_bit:	first bit to set
 * @count:	number of bits to set
 *
 * Set @count bits starting at bit @start_bit in the bitmap described by the
 * attribute @na.
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 */
int ntfs_bitmap_set_run(ntfs_attr *na, s64 start_bit, s64 count)
{
	return ntfs_bitmap_set_bits_in_run(na, start_bit, count, 1);
}

/**
 * ntfs_bitmap_clear_run - clear a run of bits in a bitmap
 * @na:		attribute containing the bitmap
 * @start_bit:	first bit to clear
 * @count:	number of bits to clear
 *
 * Clear @count bits starting at bit @start_bit in the bitmap described by the
 * attribute @na.
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 */
int ntfs_bitmap_clear_run(ntfs_attr *na, s64 start_bit, s64 count)
{
	ntfs_log_trace("Dealloc from bit 0x%llx, count 0x%llx.\n", (LONG64)start_bit, (LONG64)count);

	return ntfs_bitmap_set_bits_in_run(na, start_bit, count, 0);
}

