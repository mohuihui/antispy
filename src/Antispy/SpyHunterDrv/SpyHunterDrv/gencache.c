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
#include <fltKernel.h>
#include "gencache.h"

#define GEN_CACHE_SIGNATURE				0x33784095

// hash func
#define GEN_HASH_FUNC( __x__, __s__ )	(ULONG) ( (ULONGLONG) ( __x__ ) & ( __s__ - 1 ) )

typedef struct _GEN_CACHE_ITEM
{
	LIST_ENTRY		m_List;
	ULONG			m_Signature;
	ULONGLONG		m_Key;
	UCHAR			m_Data[1];
} GEN_CACHE_ITEM, *PGEN_CACHE_ITEM;

BOOLEAN GenCacheInit(
	IN ULONG HashSize,
	IN ULONG DataSize,
	IN t_fGenCacheItemAlloc cbAlloc,
	IN t_fGenCacheItemFree cbFree,
	OUT PGEN_CACHE *p_pGenCache
	)
{
	PGEN_CACHE pCache;
	ULONG i;

	pCache = (PGEN_CACHE) ExAllocatePoolWithTag(
		NonPagedPool,
		FIELD_OFFSET( GEN_CACHE, m_HashTable ) + HashSize * sizeof( LIST_ENTRY ),
		'mZft'
		);
	if ( !pCache )
		return FALSE;

	pCache->m_HashSize = HashSize;
	pCache->m_DataSize = DataSize;
	pCache->m_cbAlloc = cbAlloc;
	pCache->m_cbFree = cbFree;
	ExInitializeFastMutex( &pCache->m_CacheLock );

	for ( i = 0; i < HashSize; i++ )
		InitializeListHead( &pCache->m_HashTable[i] );

	*p_pGenCache = pCache;

	return TRUE;
}

VOID GenCacheDone(
	__inout PGEN_CACHE *p_pCache
	)
{
	PGEN_CACHE pCache;
	ULONG Hash;

	ASSERT( p_pCache );
	pCache = *p_pCache;
	ASSERT( pCache );

	ExAcquireFastMutex( &pCache->m_CacheLock );

	for ( Hash = 0; Hash < pCache->m_HashSize; Hash++ )
	{
		PLIST_ENTRY pListHead;
		PGEN_CACHE_ITEM pCacheItem;

		pListHead = &pCache->m_HashTable[Hash];

		pCacheItem = (PGEN_CACHE_ITEM) pListHead->Flink;
		while ( pCacheItem != (PGEN_CACHE_ITEM) pListHead )
		{
			PGEN_CACHE_ITEM pTmpCacheItem;

			ASSERT( GEN_CACHE_SIGNATURE == pCacheItem->m_Signature );	

			pTmpCacheItem = (PGEN_CACHE_ITEM) pCacheItem->m_List.Flink;

			// call pre-free callback
			if ( pCache->m_cbFree )
			{
				pCache->m_cbFree(
					pCacheItem->m_Key,
					&pCacheItem->m_Data
					);
			}

			ExFreePool( pCacheItem );
			pCacheItem = pTmpCacheItem;
		}
	}

	ExReleaseFastMutex( &pCache->m_CacheLock );

	ExFreePool( *p_pCache );

	*p_pCache = NULL;
}

PVOID GenCacheGet(
	IN PGEN_CACHE pCache,
	IN ULONGLONG Key,
	IN BOOLEAN bCreateNew,
	OUT PBOOLEAN p_bNewReturned
  )
{
	ULONG Hash;
	PLIST_ENTRY pListHead;
	PGEN_CACHE_ITEM pCacheItem;
	PGEN_CACHE_ITEM pRetItem = NULL;
	BOOLEAN bNewReturned = FALSE;

	ASSERT( pCache );
	ASSERT( p_bNewReturned );

	Hash = GEN_HASH_FUNC( Key, pCache->m_HashSize );

	ExAcquireFastMutex( &pCache->m_CacheLock );

	pListHead = &pCache->m_HashTable[Hash];

	for ( pCacheItem = (PGEN_CACHE_ITEM) pListHead->Flink;
		pCacheItem != (PGEN_CACHE_ITEM) pListHead;
		pCacheItem = (PGEN_CACHE_ITEM) pCacheItem->m_List.Flink 
		)
	{
		ASSERT( GEN_CACHE_SIGNATURE == pCacheItem->m_Signature );	

		if ( Key == pCacheItem->m_Key )
		{
			pRetItem = pCacheItem;
			break;
		}	
	}

	if ( !pRetItem && bCreateNew )
	{
		pRetItem = (PGEN_CACHE_ITEM) ExAllocatePoolWithTag(
			NonPagedPool,
			FIELD_OFFSET( GEN_CACHE_ITEM, m_Data ) + pCache->m_DataSize,
			'mZft'
			);
		if ( pRetItem )
		{
			pRetItem->m_Signature = GEN_CACHE_SIGNATURE;
			pRetItem->m_Key = Key;

			InsertTailList( pListHead, &pRetItem->m_List );

			if ( pCache->m_cbAlloc )
			{
				pCache->m_cbAlloc(
					pRetItem->m_Key,
					&pRetItem->m_Data
					);
			}

			bNewReturned = TRUE;
		}
	}

	ExReleaseFastMutex( &pCache->m_CacheLock );

	*p_bNewReturned = bNewReturned;

	return pRetItem ? &pRetItem->m_Data : NULL;
}

VOID GenCacheFree(
	IN PGEN_CACHE pCache,
	IN ULONGLONG Key
	)
{
	ULONG Hash;
	PLIST_ENTRY pListHead;
	PGEN_CACHE_ITEM pCacheItem;
	PGEN_CACHE_ITEM pFoundItem = NULL;

	ASSERT( pCache );

	Hash = GEN_HASH_FUNC( Key, pCache->m_HashSize );

	ExAcquireFastMutex( &pCache->m_CacheLock );

	pListHead = &pCache->m_HashTable[Hash];

	for (	pCacheItem = (PGEN_CACHE_ITEM) pListHead->Flink;
			pCacheItem != (PGEN_CACHE_ITEM) pListHead;
			pCacheItem = (PGEN_CACHE_ITEM) pCacheItem->m_List.Flink 
		)
	{
		ASSERT( GEN_CACHE_SIGNATURE == pCacheItem->m_Signature );	

		if ( Key == pCacheItem->m_Key )
		{
			pFoundItem = pCacheItem;
			break;
		}	
	}

	if ( pFoundItem )	
	{
		if ( pCache->m_cbFree )
		{
			pCache->m_cbFree(
				pCacheItem->m_Key,
				&pCacheItem->m_Data
				);
		}

		RemoveEntryList( &pFoundItem->m_List );		
		ExFreePool( pFoundItem );
	}

	ExReleaseFastMutex( &pCache->m_CacheLock );
}

VOID GenCacheCleanupByKeyMask(
	IN PGEN_CACHE pCache,
	IN ULONGLONG Key,
	IN ULONGLONG KeyMask
	)
{
	ULONG Hash;
	PLIST_ENTRY pListHead;
	PGEN_CACHE_ITEM pCacheItem;

	ASSERT( pCache );

	Hash = GEN_HASH_FUNC( Key, pCache->m_HashSize );

	ExAcquireFastMutex( &pCache->m_CacheLock );

	pListHead = &pCache->m_HashTable[Hash];

	pCacheItem = (PGEN_CACHE_ITEM) pListHead->Flink;
	while ( pCacheItem != (PGEN_CACHE_ITEM) pListHead )
	{
		PGEN_CACHE_ITEM pNextItem;

		ASSERT( GEN_CACHE_SIGNATURE == pCacheItem->m_Signature );	

		pNextItem = (PGEN_CACHE_ITEM) pCacheItem->m_List.Flink;

		if ( ( KeyMask & Key ) == ( KeyMask & pCacheItem->m_Key ) )
		{
			// call pre-free callback
			if ( pCache->m_cbFree )
			{
				pCache->m_cbFree(
					pCacheItem->m_Key,
					&pCacheItem->m_Data
					);
			}

			RemoveEntryList( &pCacheItem->m_List );		
			ExFreePool( pCacheItem );
		}

		pCacheItem = pNextItem;
	}

	ExReleaseFastMutex( &pCache->m_CacheLock );
}
