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
#include "HotKeys.h"
#include "InitWindows.h"
#include "ldasm.h"
#include "..\\..\\Common\\Common.h"

//
// xp系统获取热键的链表
//
VOID GetgphkFirstWin2kXp()
{
	ULONG pNtUserRegisterHotKey = GetGlobalVeriable(enumNtUserRegisterHotKey);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG n0xE8 = 0, nCodeLen = 0, i = 0, pRegisterHotKey = 0, gphkFirst = 0;
	PCHAR OpCode = NULL;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

	if (!pNtUserRegisterHotKey || !MzfMmIsAddressValid)
	{
		return;
	}

	for (i = pNtUserRegisterHotKey; i < pNtUserRegisterHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(PBYTE)OpCode == 0xe8)
		{
			n0xE8++;
			if (WinVersion == enumWINDOWS_2K && n0xE8 == 4)
			{
				pRegisterHotKey = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pRegisterHotKey: 0x%08X\n", pRegisterHotKey));
				break;
			}
			else if (WinVersion == enumWINDOWS_XP && n0xE8 == 3)
			{
				pRegisterHotKey = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pRegisterHotKey: 0x%08X\n", pRegisterHotKey));
				break;
			}
		}
	}

	if (!pRegisterHotKey || !MzfMmIsAddressValid((PVOID)pRegisterHotKey))
	{
		return;
	}

	for (i = pRegisterHotKey; i < pRegisterHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(PBYTE)OpCode == 0xa1)
		{
			gphkFirst = *((PULONG)(i + 1));;
			KdPrint(("gphkFirst: 0x%08X\n", gphkFirst));
			break;
		}
	}

	if (gphkFirst && gphkFirst > SYSTEM_ADDRESS_START)
	{
		SetGlobalVeriable(enumgphkFirst, gphkFirst);
	}
}

//
// win7获得热键链表
//
VOID GetHotKeyHashArrayWin2k3ToWin7()
{
	ULONG pNtUserRegisterHotKey = GetGlobalVeriable(enumNtUserRegisterHotKey);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG n0xE8 = 0, nCodeLen = 0, i = 0, pRegisterHotKey = 0, pHKInsertHashElement = 0, pHotKeyHashArray = 0;
	PCHAR OpCode = NULL;
	ULONG nBuildNumer = GetGlobalVeriable(enumBuildNumber);

	if (!pNtUserRegisterHotKey || !MzfMmIsAddressValid)
	{
		return;
	}

	for (i = pNtUserRegisterHotKey; i < pNtUserRegisterHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(PBYTE)OpCode == 0xe8)
		{
			n0xE8++;
			if (nBuildNumer == 3790 && n0xE8 == 3)
			{
				pRegisterHotKey = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pRegisterHotKey: 0x%08X\n", pRegisterHotKey));
				break;
			}
			else if (nBuildNumer > 3790 && n0xE8 == 4)
			{
				pRegisterHotKey = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pRegisterHotKey: 0x%08X\n", pRegisterHotKey));
				break;
			}
		}
	}

	if (!pRegisterHotKey || !MzfMmIsAddressValid((PVOID)pRegisterHotKey))
	{
		return;
	}

	n0xE8 = 0;

	for (i = pRegisterHotKey; i < pRegisterHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(PBYTE)OpCode == 0xe8)
		{
			n0xE8++;
			if (nBuildNumer == 3790 && n0xE8 == 5)
			{
				pHKInsertHashElement = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pHKInsertHashElement: 0x%08X\n", pHKInsertHashElement));
				break;
			}
			else if (nBuildNumer > 3790 && n0xE8 == 6)
			{
				pHKInsertHashElement = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pHKInsertHashElement: 0x%08X\n", pHKInsertHashElement));
				break;
			}
		}
	}

	if (!pHKInsertHashElement || !MzfMmIsAddressValid((PVOID)pHKInsertHashElement))
	{
		return;
	}

	for (i = pHKInsertHashElement; i < pHKInsertHashElement + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 7 && *(PBYTE)OpCode == 0x8D)
		{
			pHotKeyHashArray = *(PULONG)(i + 3);
			KdPrint(("pHotKeyHashArray: 0x%08X\n", pHotKeyHashArray));
			break;
		}
	}

	if (pHotKeyHashArray && pHotKeyHashArray > SYSTEM_ADDRESS_START)
	{
		SetGlobalVeriable(enumgphkFirst, pHotKeyHashArray);
	}
}

//
// win8下获得热键列表
//
VOID GetHotKeyHashArrayWin8()
{
	ULONG pNtUserRegisterHotKey = GetGlobalVeriable(enumNtUserRegisterHotKey);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG n0xE8 = 0, nCodeLen = 0, i = 0, pRegisterHotKey = 0, pFindHotKey = 0, pHotKeyHashArray = 0;
	PCHAR OpCode = NULL;
	ULONG nBuildNumer = GetGlobalVeriable(enumBuildNumber);

	if (!pNtUserRegisterHotKey || !MzfMmIsAddressValid)
	{
		return;
	}

	for (i = pNtUserRegisterHotKey; i < pNtUserRegisterHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(PBYTE)OpCode == 0xe8)
		{
			n0xE8++;
			if (n0xE8 == 3)
			{
				pRegisterHotKey = i + *(PULONG)(i + 1) + 5;
				KdPrint(("pRegisterHotKey: 0x%08X\n", pRegisterHotKey));
				break;
			}
		}
	}

	if (!pRegisterHotKey || !MzfMmIsAddressValid((PVOID)pRegisterHotKey))
	{
		return;
	}

	n0xE8 = 0;

	for (i = pRegisterHotKey; i < pRegisterHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 5 && *(PBYTE)OpCode == 0xe8)
		{
			n0xE8++;
			if (n0xE8 == 2)
			{
				pFindHotKey = i + *(PULONG)(i + 1) + 5;
				KdPrint(("FindHotKey: 0x%08X\n", pFindHotKey));
				break;
			}
		}
	}

	if (!pFindHotKey || !MzfMmIsAddressValid((PVOID)pFindHotKey))
	{
		return;
	}

	for (i = pFindHotKey; i < pFindHotKey + PAGE_SIZE; i += nCodeLen)
	{
		if (!MzfMmIsAddressValid((PVOID)i))
		{
			return;
		}

		nCodeLen = SizeOfCode((PVOID)i, &OpCode);
		if (!nCodeLen)
		{
			return;
		}

		if (nCodeLen == 7 && *(PBYTE)OpCode == 0x8b)
		{
			pHotKeyHashArray = *(PULONG)(i + 3);
			KdPrint(("pHotKeyHashArray: 0x%08X\n", pHotKeyHashArray));
			break;
		}
	}

	if (pHotKeyHashArray && pHotKeyHashArray > SYSTEM_ADDRESS_START)
	{
		SetGlobalVeriable(enumgphkFirst, pHotKeyHashArray);
	}
}

//
// xp系统下枚举进程热键
//
NTSTATUS EnumHotKeysWin2kXp(ULONG gphkFirst, PALL_HOTKEYS pHotKeyInfo, ULONG nCnt)
{
	ULONG HotKeyItem = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsGetThreadId MzfPsGetThreadId = (pfnPsGetThreadId)GetGlobalVeriable(enumPsGetThreadId);
	ULONG IdOffset, fsModifiersOffset, vkOffset, nNextOffset;
	PHOTKEY_ITEM pHotKeyItems = (PHOTKEY_ITEM)pHotKeyInfo->HotKeys;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG nMaxCnt = PAGE_SIZE;
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid	|| 
		!gphkFirst				||
		!pHotKeyInfo			||
		!MzfMmIsAddressValid((PVOID)gphkFirst))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	IdOffset = 0x10;
	fsModifiersOffset = 0x8;
	vkOffset = 0xc;
	nNextOffset = 0x14;

	OldIrql = KeRaiseIrqlToDpcLevel();

	for (HotKeyItem = *(PULONG)gphkFirst; 
		HotKeyItem && MzfMmIsAddressValid((PVOID)HotKeyItem) && nMaxCnt--; 
		HotKeyItem = *(PULONG)(HotKeyItem + nNextOffset))
	{
		ULONG pThread = *(PULONG)HotKeyItem;
		if (pThread && MzfMmIsAddressValid((PVOID)pThread))
		{
			pThread = *(PULONG)pThread;
			if (MzfMmIsAddressValid((PVOID)pThread))
			{
				ULONG nCurCnt = pHotKeyInfo->nCnt;
				if (nCnt > nCurCnt)
				{
					pHotKeyItems[nCurCnt].HotKeyObject = HotKeyItem;
					pHotKeyItems[nCurCnt].nId = *(PULONG)(HotKeyItem + IdOffset);
					pHotKeyItems[nCurCnt].fsModifiers = *(PULONG)(HotKeyItem + fsModifiersOffset);
					pHotKeyItems[nCurCnt].vk = *(PULONG)(HotKeyItem + vkOffset);

					if (MzfPsGetThreadId)
					{
						pHotKeyItems[nCurCnt].tid = (ULONG)MzfPsGetThreadId((PETHREAD)pThread);
					}
					else
					{
						pHotKeyItems[nCurCnt].tid = *(PULONG)(GetGlobalVeriable(enumCidOffset_ETHREAD) + pThread + sizeof(ULONG));
					}

					pHotKeyItems[nCurCnt].pEprocess = *(PULONG)(pThread + GetGlobalVeriable(enumThreadsProcessOffset_ETHREAD));
				}
				
				pHotKeyInfo->nCnt++;
			}
		}
	}
	
	KeLowerIrql(OldIrql);
	
	return status;
}

//
// win7枚举热键
//
NTSTATUS EnumHotKeysWin2k3ToWin8(ULONG pHotKeyHashArray, PALL_HOTKEYS pHotKeyInfo, ULONG nCnt)
{
	ULONG HotKeyItem = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsGetThreadId MzfPsGetThreadId = (pfnPsGetThreadId)GetGlobalVeriable(enumPsGetThreadId);
	ULONG IdOffset, fsModifiersOffset, vkOffset, nNextOffset;
	PHOTKEY_ITEM pHotKeyItems = (PHOTKEY_ITEM)pHotKeyInfo->HotKeys;
	ULONG i = 0;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid	|| 
		!pHotKeyHashArray		||
		!pHotKeyInfo			||
		!MzfMmIsAddressValid((PVOID)pHotKeyHashArray))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	if (nBuildNumber == 3790)
	{
		IdOffset = 0x10;
		fsModifiersOffset = 0x8;
		vkOffset = 0xc;
		nNextOffset = 0x14;
	}
	else if (nBuildNumber > 3790)
	{
		IdOffset = 0x14;
		fsModifiersOffset = 0xc;
		vkOffset = 0x10;
		nNextOffset = 0x18;
	}
	
	OldIrql = KeRaiseIrqlToDpcLevel();

	for (i = 0; i < 0x7f; i++)
	{
		ULONG nMaxCnt = PAGE_SIZE;

		for (HotKeyItem = *(PULONG)(pHotKeyHashArray + 4 * i ); 
			HotKeyItem && MzfMmIsAddressValid((PVOID)HotKeyItem) && nMaxCnt--; 
			HotKeyItem = *(PULONG)(HotKeyItem + nNextOffset))
		{
			ULONG pThread = *(PULONG)HotKeyItem;
			if (pThread && MzfMmIsAddressValid((PVOID)pThread))
			{
				pThread = *(PULONG)pThread;
				if (MzfMmIsAddressValid((PVOID)pThread))
				{
					ULONG nCnrCnt = pHotKeyInfo->nCnt;
					if (nCnt > nCnrCnt)
					{
						pHotKeyItems[nCnrCnt].HotKeyObject = HotKeyItem;
						pHotKeyItems[nCnrCnt].nId = *(PULONG)(HotKeyItem + IdOffset);
						pHotKeyItems[nCnrCnt].fsModifiers = *(PULONG)(HotKeyItem + fsModifiersOffset);
						pHotKeyItems[nCnrCnt].vk = *(PULONG)(HotKeyItem + vkOffset);

						if (MzfPsGetThreadId)
						{
							pHotKeyItems[nCnrCnt].tid = (ULONG)MzfPsGetThreadId((PETHREAD)pThread);
						}
						else
						{
							pHotKeyItems[nCnrCnt].tid = *(PULONG)(GetGlobalVeriable(enumCidOffset_ETHREAD) + pThread + sizeof(ULONG));
						}

						pHotKeyItems[nCnrCnt].pEprocess = *(PULONG)(pThread + GetGlobalVeriable(enumThreadsProcessOffset_ETHREAD));
						KdPrint(("HotKeyItem: 0x%08X, process: 0x%08X\n", HotKeyItem, pHotKeyItems[nCnrCnt].pEprocess));
					}

					pHotKeyInfo->nCnt++;
				}
			}
		}
	}

	KeLowerIrql(OldIrql);

	return status;
}

//
// 获取热键的列表
//
VOID GetHotKeyListHead()
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

	if (BuildNumber < 3790)
	{
		GetgphkFirstWin2kXp();
	}
	else if (BuildNumber >= 3790 && BuildNumber < 8000)
	{
		GetHotKeyHashArrayWin2k3ToWin7();
	}
	else if (BuildNumber > 8000)
	{
		GetHotKeyHashArrayWin8();
	}
}

//
// 枚举进程热键
//
NTSTATUS ListHotKeys(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG gphkFirst = GetGlobalVeriable(enumgphkFirst);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PALL_HOTKEYS pHotKeyInfo = (PALL_HOTKEYS)pOutBuffer;
	ULONG nCnt = (nOutLen - sizeof(ALL_HOTKEYS)) / sizeof(HOTKEY_ITEM);

	//
	// 参数检查
	//
	if (!pInBuffer || 
		uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer ||
		nOutLen < sizeof(ALL_HOTKEYS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (!gphkFirst)
	{
		GetHotKeyListHead();
		gphkFirst = GetGlobalVeriable(enumgphkFirst);
	}

	if (gphkFirst)
	{
		if (BuildNumber < 3790)
		{
			status = EnumHotKeysWin2kXp(gphkFirst, pHotKeyInfo, nCnt);
		}
		else if (BuildNumber >= 3790)
		{
			status = EnumHotKeysWin2k3ToWin8(gphkFirst, pHotKeyInfo, nCnt);
		}
	}

	if (NT_SUCCESS(status))
	{
		if (nCnt >= pHotKeyInfo->nCnt)
		{
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	return status;
}

//
// xp系统下移除热键
//
NTSTATUS RemoveHotKeyWin2kXp(ULONG gphkFirst, ULONG pHotKeyObject)
{
	ULONG HotKeyItem = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG nNextOffset = 0x14;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	KIRQL OldIrql;
	ULONG nMaxCnt = PAGE_SIZE;

	if (!MzfMmIsAddressValid					|| 
		!gphkFirst								||
		!MzfMmIsAddressValid((PVOID)gphkFirst)	||
		pHotKeyObject < SYSTEM_ADDRESS_START	||
		!MzfMmIsAddressValid((PVOID)pHotKeyObject))
	{
		return STATUS_UNSUCCESSFUL;
	}

	OldIrql = KeRaiseIrqlToDpcLevel();

	for (HotKeyItem = gphkFirst; 
		*(PDWORD)HotKeyItem && MzfMmIsAddressValid((PVOID)(*(PDWORD)HotKeyItem)) && nMaxCnt--; 
		HotKeyItem = *(PDWORD)HotKeyItem + nNextOffset)
	{
		if (*(PDWORD)HotKeyItem == pHotKeyObject)
		{
			KdPrint(("find pHotKeyObject\n"));
			*(PDWORD)HotKeyItem = *(PDWORD)(*(PDWORD)HotKeyItem + nNextOffset);
			status = STATUS_SUCCESS;
			break;
		}
	}
	
	KeLowerIrql(OldIrql);

	return status;
}

//
// win7系统移除热键
//
NTSTATUS RemoveHotKeysWin2k3ToWin8(ULONG pHotKeyHashArray, ULONG pHotKeyObject)
{
	ULONG HotKeyItem = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsGetThreadId MzfPsGetThreadId = (pfnPsGetThreadId)GetGlobalVeriable(enumPsGetThreadId);
	ULONG nNextOffset = 0;
	BOOL bOk = TRUE;
	ULONG i = 0, j = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nBuildNumber = GetGlobalVeriable(enumBuildNumber);
	KIRQL OldIrql;

	if (!MzfMmIsAddressValid	|| 
		!pHotKeyHashArray		||
		!pHotKeyObject			||
		!MzfMmIsAddressValid((PVOID)pHotKeyHashArray))
	{
		return status;
	}

	if (nBuildNumber == 3790)
	{
		nNextOffset = 0x14;
	}
	else if (nBuildNumber > 3790)
	{
		nNextOffset = 0x18;
	}

	OldIrql = KeRaiseIrqlToDpcLevel();

	for (i = 0; i < 0x7f; i++)
	{
		ULONG nMax = PAGE_SIZE;

		for (HotKeyItem = *(PULONG)(pHotKeyHashArray + 4 * i ); 
			HotKeyItem && MzfMmIsAddressValid((PVOID)HotKeyItem) && nMax--; 
			HotKeyItem = *(PULONG)(HotKeyItem + nNextOffset))
		{
			if (HotKeyItem == pHotKeyObject)
			{
				*(PULONG)(pHotKeyHashArray + 4 * i ) = *(PULONG)(HotKeyItem + nNextOffset);
				status = STATUS_SUCCESS;
				break;
			}
		}

		if (NT_SUCCESS(status))
		{
			break;
		}
	}

	KeLowerIrql(OldIrql);

	return status;
}

//
// 移除hotkey
//
NTSTATUS RemoveHotKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG gphkFirst = GetGlobalVeriable(enumgphkFirst);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG pObject = 0;

	//
	// 检查参数
	//
	if (!pInBuffer ||
		uInSize != sizeof(COMMUNICATE_HOTKEY))
	{
		return STATUS_INVALID_PARAMETER;
	}

	pObject = ((PCOMMUNICATE_HOTKEY)pInBuffer)->op.Remove.HotKeyObject;

	if (!gphkFirst)
	{
		GetHotKeyListHead();
		gphkFirst = GetGlobalVeriable(enumgphkFirst);
	}

	if (gphkFirst && pObject)
	{
		if (BuildNumber < 3790)
		{
			status = RemoveHotKeyWin2kXp(gphkFirst, pObject);
		}
		else if (BuildNumber >= 3790)
		{
			status = RemoveHotKeysWin2k3ToWin8(gphkFirst, pObject);
		}
	}

	return status;
}