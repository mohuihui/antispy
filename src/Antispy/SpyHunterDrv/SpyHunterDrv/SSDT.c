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
#include "SSDT.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "libdasm.h"
#include "peload.h"

PVOID GetFunctionNameFromNtdll(PVOID pModuleBase)
{
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS NtDllHeader;
	IMAGE_OPTIONAL_HEADER opthdr;
	DWORD* arrayOfFunctionAddresses;
	DWORD* arrayOfFunctionNames;
	WORD* arrayOfFunctionOrdinals;
	DWORD functionOrdinal;
	DWORD Base, x, functionAddress, position;
	char* functionName;
	IMAGE_EXPORT_DIRECTORY *pExportTable;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	PVOID pSsdtFuncionsNameBuffer = NULL;
	ULONG nServiceCnt = GetGlobalVeriable(enumSsdtFunctionsCnt);
	ULONG nCnt = 0;

	KdPrint(("nServiceCnt: %d\n", nServiceCnt));

	if (pModuleBase && nServiceCnt && MzfExAllocatePoolWithTag && MzfExFreePoolWithTag && MzfMmIsAddressValid)
	{
		pSsdtFuncionsNameBuffer = MzfExAllocatePoolWithTag(NonPagedPool, nServiceCnt * 100, MZFTAG);
		if (pSsdtFuncionsNameBuffer)
		{
			memset(pSsdtFuncionsNameBuffer, 0, nServiceCnt * 100);
		}
	}

	KdPrint(("pSsdtFuncionsNameBuffer: 0x%08X\n", pSsdtFuncionsNameBuffer));

	if (!pSsdtFuncionsNameBuffer)
	{
		return NULL;
	}
	
	__try
	{
		pDosHeader = (PIMAGE_DOS_HEADER)pModuleBase;
		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			MzfExFreePoolWithTag(pSsdtFuncionsNameBuffer, 0);
			pSsdtFuncionsNameBuffer = NULL;
			return NULL;
		}

		NtDllHeader = (PIMAGE_NT_HEADERS)((ULONG)pDosHeader + pDosHeader->e_lfanew);
		if (NtDllHeader->Signature != IMAGE_NT_SIGNATURE)
		{
			MzfExFreePoolWithTag(pSsdtFuncionsNameBuffer, 0);
			pSsdtFuncionsNameBuffer = NULL;
			return NULL;
		}

		opthdr = NtDllHeader->OptionalHeader;
		pExportTable = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)pModuleBase + opthdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress); //得到导出表
		arrayOfFunctionAddresses = (DWORD*)( (BYTE*)pModuleBase + pExportTable->AddressOfFunctions);  //地址表
		arrayOfFunctionNames = (DWORD*)((BYTE*)pModuleBase + pExportTable->AddressOfNames);         //函数名表
		arrayOfFunctionOrdinals = (WORD*)( (BYTE*)pModuleBase + pExportTable->AddressOfNameOrdinals);

		Base = pExportTable->Base;

		for(x = 0; x < pExportTable->NumberOfFunctions; x++) //在整个导出表里扫描
		{
			char szNt[] = {'n', 't', '\0'};
			functionName = (char*)( (BYTE*)pModuleBase + arrayOfFunctionNames[x]);
			
			if (functionName && MmIsAddressValid(functionName) && strlen(functionName) > strlen(szNt) && !_strnicmp(functionName, szNt, strlen(szNt)))
			{
				ULONG nFunLen = strlen(functionName);
				if (nFunLen >= 100)
				{
					nFunLen = 99;
				}

				functionOrdinal = arrayOfFunctionOrdinals[x] + Base - 1; 
				functionAddress = (DWORD)((BYTE*)pModuleBase + arrayOfFunctionAddresses[functionOrdinal]);

				if (MzfMmIsAddressValid((PVOID)(functionAddress + 1)))
				{
					position  = *((WORD*)(functionAddress + 1));  //得到服务号
					if (position < nServiceCnt)
					{
						strncpy( (PCHAR)((ULONG)pSsdtFuncionsNameBuffer + 100 * position), functionName, nFunLen);
						KdPrint(("%d: %s\n", position, functionName));
					}
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		MzfExFreePoolWithTag(pSsdtFuncionsNameBuffer, 0);
		pSsdtFuncionsNameBuffer = NULL;
		KdPrint(("EXCEPTION_EXECUTE_HANDLER[%08x]", GetExceptionCode()));
	}

	return pSsdtFuncionsNameBuffer;
}

BOOL GetSsdtFuncionsName()
{
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	HANDLE hNtdll = NULL;
	PVOID pSsdtFuncionsNameBuffer = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	BOOL bRet = FALSE;
	ULONG Length = 0;
	LARGE_INTEGER FileSize;
	PVOID FileBuffer = NULL;
	IO_STATUS_BLOCK IoStatus;
	PBYTE ImageBase = NULL;
	WCHAR szNtdll[] = {
		'\\','S','y','s','t','e','m','R','o','o','t','\\','S','y','s','t','e','m','3','2','\\','n','t','d','l','l','.','d','l','l','\0'
	};

	if (!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag)
	{
		return FALSE;
	}

	status = KernelOpenFile(szNtdll, &hNtdll, SYNCHRONIZE | FILE_EXECUTE, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	status = KernelGetFileSize(hNtdll, &FileSize);
	if (!NT_SUCCESS(status))
	{
		MzfCloseHandle(hNtdll);
		return FALSE;
	}

	Length = FileSize.LowPart;
	FileBuffer = MzfExAllocatePoolWithTag(PagedPool, Length, MZFTAG);
	if (!FileBuffer)
	{
		ULONG i = 1;
		do 
		{
			ULONG nLen = Length + 2048 * i;
			FileBuffer = MzfExAllocatePoolWithTag(PagedPool, nLen, 0); 
		} while (FileBuffer == NULL && ++i < 10);
	}

	if (FileBuffer == NULL)
	{
		MzfCloseHandle(hNtdll);
		return FALSE;
	}

	status = KernelReadFile(hNtdll, NULL, Length, FileBuffer, &IoStatus);
	if (!NT_SUCCESS(status))
	{
		MzfCloseHandle(hNtdll);
		MzfExFreePoolWithTag(FileBuffer, 0);
		return FALSE;
	}

	MzfCloseHandle(hNtdll);

	if(!ImageFile(FileBuffer, &ImageBase))
	{
		KdPrint(("ImageFile failed\n"));
		MzfExFreePoolWithTag(FileBuffer, 0);
		return FALSE;
	}

	MzfExFreePoolWithTag(FileBuffer, 0);

	pSsdtFuncionsNameBuffer = GetFunctionNameFromNtdll(ImageBase);
	if (pSsdtFuncionsNameBuffer)
	{
		SetGlobalVeriable(enumSsdtFuncionsNameBuffer, (ULONG)pSsdtFuncionsNameBuffer);
		bRet = TRUE;
	}
	
	if (ImageBase)
	{
		MzfExFreePoolWithTag(ImageBase, 0);
	}

	return bRet;
}

//
// 根据索引值,得到SSDT函数的inline hook地址
//
ULONG GetSSDTInlineAddress(ULONG nIndex)
{
	ULONG pAddress = 0;
	PULONG pNewSsdt = (PULONG)GetGlobalVeriable(enumReloadKiServiceTable);
	PULONG pNowSsdt = (PULONG)GetGlobalVeriable(enumOriginKiServiceTable);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (pNewSsdt && pNowSsdt && 
		MzfMmIsAddressValid && 
		MzfMmIsAddressValid(pNewSsdt) && 
		MzfMmIsAddressValid(pNowSsdt) &&
		MzfMmIsAddressValid((PVOID)pNewSsdt[nIndex]) &&
		MzfMmIsAddressValid((PVOID)pNowSsdt[nIndex])) 
	{
		ULONG pOrigin = pNewSsdt[nIndex];
		ULONG pNow = pNowSsdt[nIndex];

		if (memcmp((PBYTE)pOrigin, (PBYTE)pNow, 0x10))
		{
			pAddress = GetInlineHookAddress(pNow, 0x10);

			if (pAddress == 0 || 
				pAddress < SYSTEM_ADDRESS_START || 
				!MzfMmIsAddressValid((PVOID)pAddress))
			{
				pAddress = 1;
			}
		}
	}

	return pAddress;
}

//
// 真正枚举SSDT钩子的函数
//
NTSTATUS PspEnumSsdtHookInfo(PALL_SSDT_HOOK psh, ULONG nCnt)
{
	PULONG pOriginSsdt = (PULONG)GetGlobalVeriable(enumOriginKiServiceTable);
	PServiceDescriptorTableEntry_t pSsdt = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
	ULONG nFuncCnt = GetGlobalVeriable(enumSsdtFunctionsCnt);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	PVOID pSsdtName = (PVOID)GetGlobalVeriable(enumSsdtFuncionsNameBuffer);
	
	if (!pSsdtName)
	{
		GetSsdtFuncionsName();
		pSsdtName = (PVOID)GetGlobalVeriable(enumSsdtFuncionsNameBuffer);
	}

	KdPrint(("pSsdtName: 0x%08X\n", pSsdtName));

	if (!MzfRtlInitAnsiString ||
		!MzfRtlAnsiStringToUnicodeString ||
		!MzfRtlFreeUnicodeString ||
		!psh ||
		!pSsdtName)
	{
		return status;
	}

	if ( pOriginSsdt && pSsdt && nFuncCnt )
	{
		ULONG i = 0;
		for (i = 0; i < nFuncCnt; i++)
		{
			ULONG pOrigin = pOriginSsdt[i];
			ULONG pNow = pSsdt->ServiceTableBase[i];
			ANSI_STRING anName;
			UNICODE_STRING unName;
			ULONG nCurCnt = psh->nCnt;

			if (nCnt > nCurCnt)
			{
				psh->SsdtHook[nCurCnt].nIndex = i;
				psh->SsdtHook[nCurCnt].pNowAddress = pNow;
				psh->SsdtHook[nCurCnt].pOriginAddress = pOrigin;
				psh->SsdtHook[nCurCnt].pInlineHookAddress = GetSSDTInlineAddress(i);

				MzfRtlInitAnsiString(&anName, (PCHAR)pSsdtName + i * 100);
				if (NT_SUCCESS(MzfRtlAnsiStringToUnicodeString(&unName, &anName, TRUE)))
				{
					wcsncpy(psh->SsdtHook[nCurCnt].szFunctionName, unName.Buffer, unName.Length / sizeof(WCHAR));
					MzfRtlFreeUnicodeString(&unName);
				}
			}

			psh->nCnt++;
		}

		status = STATUS_SUCCESS;
	}

	return status;
}

//
// 枚举SSDT的hook信息
//
NTSTATUS EnumSsdtHookInfo(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PALL_SSDT_HOOK psh = (PALL_SSDT_HOOK)pOutBuffer;
	ULONG nCnt = (uOutSize - sizeof(ALL_SSDT_HOOK)) / sizeof(SSDT_HOOK_INFO);

	// 参数检查
	if (!pInBuffer || uInSize != sizeof(OPERATE_TYPE) ||
		!pOutBuffer || uOutSize < sizeof(ALL_SSDT_HOOK))
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (NT_SUCCESS(PspEnumSsdtHookInfo(psh, nCnt)))
	{
		if (nCnt >= psh->nCnt)
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

ULONG PspGetSdtInlineHookAddress(ULONG pAddress)
{
	ULONG nRet = 0;
	ULONG nLen = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!pAddress ||
		!MzfMmIsAddressValid ||
		!MzfMmIsAddressValid((PVOID)pAddress))
	{
		return 0;
	}
	
	for (nLen = 0; nLen < 100; nLen++)
	{
		if (!MzfMmIsAddressValid((PVOID)(pAddress + nLen)))
		{
			break;
		}
	}

	nRet = GetInlineHookAddress(pAddress, nLen);
	return nRet;
}

NTSTATUS GetSdtInlineHookAddress(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCOMMUNICATE_SSDT pcs = (PCOMMUNICATE_SSDT)pInBuffer;
	ULONG pAddress = pcs->op.GetInlineAddress.pAddress;
	KdPrint(("enter EnumSsdtInlineHookInfo\n"));
	*(PULONG)pOutBuffer = PspGetSdtInlineHookAddress(pAddress);
	return status;
}

NTSTATUS RestoreSsdtOrShadowHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCOMMUNICATE_SSDT pcs = (PCOMMUNICATE_SSDT)pInBuffer;
	ULONG nIndex = pcs->op.Restore.nIndex;
	ULONG bSsdt = pcs->op.Restore.bSsdt;
	KdPrint(("enter RestoreSsdtOrShadowHook\n"));

	if (bSsdt)
	{
		ULONG nSsdtCnt = GetGlobalVeriable(enumSsdtFunctionsCnt);
		if (nIndex < nSsdtCnt)
		{
			PULONG pOriginSsdt = (PULONG)GetGlobalVeriable(enumOriginKiServiceTable);
			PServiceDescriptorTableEntry_t pSsdt = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
			PULONG pNewSsdt = (PULONG)GetGlobalVeriable(enumReloadKiServiceTable);
			ULONG pOrigin = pNewSsdt[nIndex];
			ULONG pNow = pOriginSsdt[nIndex];

			if (pSsdt->ServiceTableBase[nIndex] != pOriginSsdt[nIndex])
			{
				ExchangeAddress(&pSsdt->ServiceTableBase[nIndex], pOriginSsdt[nIndex]);
			}
		
			if (memcmp((PBYTE)pOrigin, (PBYTE)pNow, 0x10))
			{
				KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
				WPOFF();
				memcpy((PVOID)pNow, (PVOID)pOrigin, 0x10);
				WPON();
				KeLowerIrql(OldIrql);
			}
		}
	}
	else 
	{
		ULONG nShadowSsdtCnt = GetGlobalVeriable(enumShadowSsdtFunctionsCnt);
		if (nIndex < nShadowSsdtCnt)
		{
			PULONG pOriginShadowSsdt = (PULONG)GetGlobalVeriable(enumOriginShadowKiServiceTable);
			PServiceDescriptorTableEntry_t pShadowSsdt = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowShadowSSDT);
			PULONG pNewShadowSsdt = (PULONG)GetGlobalVeriable(enumReloadShadowKiServiceTable);
			ULONG pOrigin = pNewShadowSsdt[nIndex];
			ULONG pNow = pOriginShadowSsdt[nIndex];

			if (pShadowSsdt->ServiceTableBase[nIndex] != pOriginShadowSsdt[nIndex])
			{
				ExchangeAddress(&pShadowSsdt->ServiceTableBase[nIndex], pOriginShadowSsdt[nIndex]);
			}

			if (memcmp((PBYTE)pOrigin, (PBYTE)pNow, 0x10))
			{
				KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
				WPOFF();
				memcpy((PVOID)pNow, (PVOID)pOrigin, 0x10);
				WPON();
				KeLowerIrql(OldIrql);
			}
		}
	}

	return status;
}