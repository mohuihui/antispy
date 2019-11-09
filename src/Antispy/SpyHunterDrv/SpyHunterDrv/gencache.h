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
#ifndef __GENCACHE_H
#define __GENCACHE_H
//--------------------------------------------------------------------------
#define		HANDLE_CACHE_KEY_FUNC( __pid__, __handle__ )	( ( ( (ULONGLONG) ( (ULONG) ( __handle__ ) & 0xffffffff ) ) << 32 ) | ( (ULONG) ( __pid__ ) & 0xffffffff ) )
#define		HANDLE_CACHE_PID_MASK							0xffffffff

// callback on-alloc
typedef VOID ( *t_fGenCacheItemAlloc ) (
										IN ULONGLONG Key,
										IN PVOID pData
										);
// callback on-free
typedef VOID ( *t_fGenCacheItemFree ) (
									   IN ULONGLONG Key,
									   IN PVOID pData
									   );

typedef struct _GEN_CACHE
{
	ULONG					m_HashSize;
	ULONG					m_DataSize;

	t_fGenCacheItemAlloc	m_cbAlloc;
	t_fGenCacheItemFree		m_cbFree;

	FAST_MUTEX		m_CacheLock;		// global cache lock
	LIST_ENTRY		m_HashTable[1];
} GEN_CACHE, *PGEN_CACHE;

BOOLEAN GenCacheInit(
	IN ULONG HashSize,
	IN ULONG DataSize,
	IN t_fGenCacheItemAlloc cbAlloc,
	IN t_fGenCacheItemFree cbFree,
	OUT PGEN_CACHE *p_pGenCache
	);
VOID GenCacheDone(
	IN OUT PGEN_CACHE *p_pCache
	);
PVOID GenCacheGet(
	IN PGEN_CACHE pCache,
	IN ULONGLONG Key,
	IN BOOLEAN bCreateNew,
	OUT PBOOLEAN p_bNewReturned
	);
VOID GenCacheFree(
	IN PGEN_CACHE pCache,
	IN ULONGLONG Key
	);
VOID GenCacheCleanupByKeyMask(
	IN PGEN_CACHE pCache,
	IN ULONGLONG Key,
	IN ULONGLONG KeyMask
	);
//--------------------------------------------------------------------------
#endif