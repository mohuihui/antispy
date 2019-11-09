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
// Copyright (C) 2002, Matt Conover (mconover@gmail.com)
#ifndef MISC_H
#define MISC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// NOTE: start is inclusive, end is exclusive (as in start <= x < end)
#define IS_IN_RANGE(x, s, e) \
( \
	((ULONG_PTR)(x) == (ULONG_PTR)(s) && (ULONG_PTR)(x) == (ULONG_PTR)(e)) || \
	((ULONG_PTR)(x) >= (ULONG_PTR)(s) && (ULONG_PTR)(x) < (ULONG_PTR)(e)) \
)

#if _MSC_VER >= 1400
#pragma warning(disable:4996)
#endif

#if defined(_WIN64)
	#define VALID_ADDRESS_MAX 0x7FFEFFFFFFFFFFFF // Win64 specific
	typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
	#define VALID_ADDRESS_MAX 0x7FFEFFFF // Win32 specific
	typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#ifndef DECLSPEC_ALIGN
	#if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
		#define DECLSPEC_ALIGN(x) __declspec(align(x))
	#else
		#define DECLSPEC_ALIGN(x)
	#endif
#endif

#define VALID_ADDRESS_MIN 0x10000    // Win32 specific
#define IS_VALID_ADDRESS(a) IS_IN_RANGE(a, VALID_ADDRESS_MIN, VALID_ADDRESS_MAX+1)

BOOL IsHexChar(BYTE ch);
BYTE *HexToBinary(char *Input, DWORD InputLength, DWORD *OutputLength);

#ifdef __cplusplus
}
#endif
#endif // MISC_H
