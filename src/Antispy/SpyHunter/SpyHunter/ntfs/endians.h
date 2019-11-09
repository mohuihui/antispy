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
/*
 * endians.h - Definitions related to handling of byte ordering. 
 */

#ifndef _NTFS_ENDIANS_H
#define _NTFS_ENDIANS_H

/*
 * Notes:
 *	We define the conversion functions including typecasts since the
 * defaults don't necessarily perform appropriate typecasts.
 *	Also, using our own functions means that we can change them if it
 * turns out that we do need to use the unaligned access macros on
 * architectures requiring aligned memory accesses...
 */


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"


//////////////////////////////////////////////////////////////////////////


#define __ntfs_bswap_constant_16(x)		\
	  (u16)((((u16)(x) & 0xff00) >> 8) |	\
		(((u16)(x) & 0x00ff) << 8))

#define __ntfs_bswap_constant_32(x)			\
	  (u32)((((u32)(x) & 0xff000000) >> 24) |	\
		(((u32)(x) & 0x00ff0000) >>  8) |	\
		(((u32)(x) & 0x0000ff00) <<  8) |	\
		(((u32)(x) & 0x000000ff) << 24))

#define __ntfs_bswap_constant_64(x)				\
	  (u64)((((u64)(x) & 0xff00000000000000) >> 56) |	\
		(((u64)(x) & 0x00ff000000000000) >> 40) |	\
		(((u64)(x) & 0x0000ff0000000000) >> 24) |	\
		(((u64)(x) & 0x000000ff00000000) >>  8) |	\
		(((u64)(x) & 0x00000000ff000000) <<  8) |	\
		(((u64)(x) & 0x0000000000ff0000) << 24) |	\
		(((u64)(x) & 0x000000000000ff00) << 40) |	\
		(((u64)(x) & 0x00000000000000ff) << 56))


#define bswap_16(x) __ntfs_bswap_constant_16(x)
#define bswap_32(x) __ntfs_bswap_constant_32(x)
#define bswap_64(x) __ntfs_bswap_constant_64(x)


//////////////////////////////////////////////////////////////////////////

// #if defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
// 
#define __le16_to_cpu(x) (x)
#define __le32_to_cpu(x) (x)
#define __le64_to_cpu(x) (x)

#define __cpu_to_le16(x) (x)
#define __cpu_to_le32(x) (x)
#define __cpu_to_le64(x) (x)

#define __constant_le16_to_cpu(x) bswap_16(x)
#define __constant_le32_to_cpu(x) bswap_32(x)
#define __constant_le64_to_cpu(x) bswap_64(x)

#define __constant_cpu_to_le16(x) bswap_16(x)
#define __constant_cpu_to_le32(x) /*bswap_32*/(x)
#define __constant_cpu_to_le64(x) bswap_64(x)
 
// 
// #elif defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)

// #define __le16_to_cpu(x) bswap_16(x)
// #define __le32_to_cpu(x) bswap_32(x)
// #define __le64_to_cpu(x) bswap_64(x)

// 
// #define __cpu_to_le16(x) bswap_16(x)
// #define __cpu_to_le32(x) bswap_32(x)
// #define __cpu_to_le64(x) bswap_64(x)
// 
// #define __constant_le16_to_cpu(x) __ntfs_bswap_constant_16((u16)(x))
// #define __constant_le32_to_cpu(x) __ntfs_bswap_constant_32((u32)(x))
// #define __constant_le64_to_cpu(x) __ntfs_bswap_constant_64((u64)(x))
// 
// #define __constant_cpu_to_le16(x) __ntfs_bswap_constant_16((u16)(x))
// #define __constant_cpu_to_le32(x) __ntfs_bswap_constant_32((u32)(x))
// #define __constant_cpu_to_le64(x) __ntfs_bswap_constant_64((u64)(x))

// #endif


//////////////////////////////////////////////////////////////////////////


/* Unsigned from LE to CPU conversion. */

#define le16_to_cpu(x)		(u16)__le16_to_cpu((u16)(x))
#define le32_to_cpu(x)		(u32)__le32_to_cpu((u32)(x))
#define le64_to_cpu(x)		(u64)__le64_to_cpu((u64)(x))

#define le16_to_cpup(x)		(u16)__le16_to_cpu(*(const u16*)(x))
#define le32_to_cpup(x)		(u32)__le32_to_cpu(*(const u32*)(x))
#define le64_to_cpup(x)		(u64)__le64_to_cpu(*(const u64*)(x))

/* Signed from LE to CPU conversion. */

#define sle16_to_cpu(x)		(s16)__le16_to_cpu((s16)(x))
#define sle32_to_cpu(x)		(s32)__le32_to_cpu((s32)(x))
#define sle64_to_cpu(x)		(s64)__le64_to_cpu((s64)(x))

#define sle16_to_cpup(x)	(s16)__le16_to_cpu(*(s16*)(x))
#define sle32_to_cpup(x)	(s32)__le32_to_cpu(*(s32*)(x))
#define sle64_to_cpup(x)	(s64)__le64_to_cpu(*(s64*)(x))

/* Unsigned from CPU to LE conversion. */

#define cpu_to_le16(x)		(u16)__cpu_to_le16((u16)(x))
#define cpu_to_le32(x)		(u32)__cpu_to_le32((u32)(x))
#define cpu_to_le64(x)		(u64)__cpu_to_le64((u64)(x))

#define cpu_to_le16p(x)		(u16)__cpu_to_le16(*(u16*)(x))
#define cpu_to_le32p(x)		(u32)__cpu_to_le32(*(u32*)(x))
#define cpu_to_le64p(x)		(u64)__cpu_to_le64(*(u64*)(x))

/* Signed from CPU to LE conversion. */

#define cpu_to_sle16(x)		(s16)__cpu_to_le16((s16)(x))
#define cpu_to_sle32(x)		(s32)__cpu_to_le32((s32)(x))
#define cpu_to_sle64(x)		(s64)__cpu_to_le64((s64)(x))

#define cpu_to_sle16p(x)	(s16)__cpu_to_le16(*(s16*)(x))
#define cpu_to_sle32p(x)	(s32)__cpu_to_le32(*(s32*)(x))
#define cpu_to_sle64p(x)	(s64)__cpu_to_le64(*(s64*)(x))

/* Constant endianness conversion defines. */

#define const_le16_to_cpu(x)	(u16)__constant_le16_to_cpu((s16)(x))
#define const_le32_to_cpu(x)	(s32)__constant_le32_to_cpu((s32)(x))
#define const_le64_to_cpu(x)	(s64)__constant_le64_to_cpu((s64)(x))

#define const_cpu_to_le16(x)	(u16)__constant_cpu_to_le16((u16)(x))
#define const_cpu_to_le32(x)	(u32)__constant_cpu_to_le32((u32)(x))
#define const_cpu_to_le64(x)	(u64)__constant_cpu_to_le64((u64)(x))

#endif /* defined _NTFS_ENDIANS_H */
