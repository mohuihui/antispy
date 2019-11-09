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
#include "CommonFunction.h"
#include "ldasm.h"
#include "libdasm.h"
#include "..\\..\\Common\\Common.h"

BOOL IsUnicodeStringValid(PUNICODE_STRING unString)
{
	BOOL bRet = FALSE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return TRUE;
	}

	__try
	{
		if (unString->Length > 0	&&
			unString->Buffer		&&
			MzfMmIsAddressValid(unString->Buffer) &&
			MzfMmIsAddressValid(&unString->Buffer[unString->Length / sizeof(WCHAR) - 1]))
		{
			bRet = TRUE;
		}

	}
	__except(1)
	{	
		KdPrint(("IsUnicodeStringValid catch __except\n"));
		bRet = FALSE;
	}
	
	return bRet;
}

ULONG CmpAndGetStringLength(PUNICODE_STRING unString, ULONG nLen)
{
	ULONG nRet = 0;
	nRet = nLen > unString->Length / sizeof(WCHAR) ? unString->Length / sizeof(WCHAR) : nLen - 1;
	return nRet;
}

// VOID mzfwcscpy(WCHAR* strDes, WCHAR* strSrc)
// {
// 	ULONG i = 0;
// 	WCHAR *strS = NULL;
// 
// 	strS = strSrc;
// 
// 	while (*(WORD *)strS != 0)
// 	{
// 		i++;
// 		strS++;
// 	}
// 
// 	memcpy(strDes, strSrc, i * sizeof(WCHAR));
// }
// 
// VOID mzfwcscat(WCHAR* strDes, WCHAR* strSrc)
// {
// 	ULONG nLen = wcslen(strDes);
//  	mzfwcscpy(strDes + nLen, strSrc);
// }

PVOID 
GetFunctionAddressByName(
						 WCHAR *szFunction
						 )
{
	UNICODE_STRING unFunction;
	PVOID pAddr = NULL;

	if (szFunction)
	{
		RtlInitUnicodeString(&unFunction, szFunction);
		pAddr = MmGetSystemRoutineAddress(&unFunction);
	}

	return pAddr;
}

ULONG 
IsPageValid(
			ULONG uAddr
			) 
{ 
	ULONG uInfo;
	ULONG uCr4;
	ULONG uPdeAddr;
	ULONG uPteAddr;

	_asm
	{
		cli
		push eax

		_emit 0x0F
		_emit 0x20
		_emit 0xE0  // mov eax,cr4

		mov [uCr4], eax
		pop eax

		sti
	}

	uInfo = uCr4 & 0x20;
	if( uInfo != 0 )
	{
		uPdeAddr = (uAddr >> 21) * 8 - 0x3FA00000;
	}
	else
	{
		uPdeAddr = (uAddr >> 22) * 4 - 0x3FD00000;
	}

	if( (*(PULONG)uPdeAddr & 0x1) != 0 )
	{
		if( (*(PULONG)uPdeAddr & 0x80) != 0 )
		{
			return VALID_PAGE;
		}
		else
		{
			if( uInfo != 0 )
			{
				uPteAddr = (uAddr >> 12) *8 - 0x40000000;
			}
			else
			{
				uPteAddr = (uAddr >> 12) * 4 - 0x40000000;
			}

			if( (*(PULONG)uPteAddr & 0x1) != 0 )
			{
				return VALID_PAGE;
			}
			else
			{
				return PTEINVALID;
			}
		}
	}
	else
	{
		return PDEINVALID;
	}
} 

BOOL ValidateUnicodeString(PUNICODE_STRING usStr)
{
	ULONG i = 0;

	__try
	{
		if (!MmIsAddressValid(usStr))
		{
			return FALSE;
		}

		if (usStr->Buffer == NULL || usStr->Length == 0)
		{
			return FALSE;
		}

		for (i = 0; i < usStr->Length / sizeof(WCHAR); i++)
		{
			if (!MmIsAddressValid((WCHAR*)usStr->Buffer + i))
			{
				return FALSE;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return TRUE;
}

ULONG GetUnicodeStringLen(PUNICODE_STRING usStr)
{
	ULONG i = 0;

	__try
	{
		if (!MmIsAddressValid(usStr))
		{
			return 0;
		}

		if (usStr->Buffer == NULL || usStr->Length == 0)
		{
			return 0;
		}

		for (i = 0; i < usStr->Length / sizeof(WCHAR); i++)
		{
			if (!MmIsAddressValid((WCHAR*)usStr->Buffer + i))
			{
				return i - 1;
			}
		}

		i = usStr->Length / sizeof(WCHAR);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		i = 0;
	}

	return i;
}

POBJECT_TYPE KeGetObjectType(PVOID pObject)
{
	POBJECT_TYPE pObjectType = NULL;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid || !pObject || !MzfMmIsAddressValid(pObject))
	{
		return NULL;
	}

	switch(WinVersion)
	{
	case enumWINDOWS_2K:
	case enumWINDOWS_XP:
	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
	case enumWINDOWS_VISTA:
	case enumWINDOWS_VISTA_SP1_SP2:
		{
			ULONG SizeOfObjectHeader = 0, ObjectTypeOffset = 0, ObjectTypeAddress = 0;
			
			SizeOfObjectHeader = GetGlobalVeriable(enumObjectHeaderSize);
			ObjectTypeOffset = GetGlobalVeriable(enumObjectTypeOffset_OBJECT_HEADER);

			ObjectTypeAddress = (ULONG)pObject - SizeOfObjectHeader + ObjectTypeOffset;

			if (MzfMmIsAddressValid((PVOID)ObjectTypeAddress))
			{ 
				pObjectType = *(POBJECT_TYPE*)ObjectTypeAddress;
			}
		}
		break;

	case enumWINDOWS_7:
	case enumWINDOWS_8:
		{
			pfnObGetObjectType ObGetObjectType = (pfnObGetObjectType)GetGlobalVeriable(enumObGetObjectType);
			if (ObGetObjectType)
			{
				pObjectType = ObGetObjectType(pObject);
			}
		}
		break;
	}

	return pObjectType;
}

BOOL IsProcessDie(PEPROCESS pEprocess)
{
	BOOL bDie = FALSE;
	ULONG ObjectTableOffset = GetGlobalVeriable(enumObjectTableOffset_EPROCESS);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid &&
		pEprocess && 
		MzfMmIsAddressValid(pEprocess) &&
		MzfMmIsAddressValid((PVOID)((ULONG)pEprocess + ObjectTableOffset)))
	{
		PVOID pObjectTable = *(PVOID*)((ULONG)pEprocess + ObjectTableOffset );

		if (  !pObjectTable || !MzfMmIsAddressValid(pObjectTable) )
		{
			KdPrint(("process is die\n"));
			bDie = TRUE;
		}
	}
	else
	{
		bDie = TRUE;
	}

	return bDie;
}

BOOL IsRealProcess(PEPROCESS pEprocess) 
{ 
	POBJECT_TYPE pObjectType; 
	ULONG ObjectTypeAddress; 
	BOOL bRet = FALSE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);

	if (ProcessType && MzfMmIsAddressValid && pEprocess && MzfMmIsAddressValid((PVOID)(pEprocess)))
	{ 
		pObjectType = KeGetObjectType((PVOID)pEprocess);
		if (pObjectType && 
			ProcessType == pObjectType &&
			!IsProcessDie(pEprocess))
		{
			bRet = TRUE; 
		}
	} 

	return bRet; 
} 

BOOL IsPe(PVOID FileBuffer)
{
	PIMAGE_DOS_HEADER ImageDosHeader;
	PIMAGE_NT_HEADERS ImageNtHeaders;
	PIMAGE_SECTION_HEADER ImageSectionHeader;

	if (!FileBuffer)
	{
		return FALSE;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)FileBuffer + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}
	}
	__except(1)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL GetPathByFileObject(PFILE_OBJECT pFileObject, WCHAR* szPath)
{
	BOOL bGetPath = FALSE;
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnObQueryNameString MzfObQueryNameString = (pfnObQueryNameString)GetGlobalVeriable(enumObQueryNameString);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfExFreePoolWithTag || !MzfExAllocatePoolWithTag || !MzfObQueryNameString || !MzfMmIsAddressValid)
	{
		return FALSE;
	}

	__try
	{
		if (pFileObject && MzfMmIsAddressValid(pFileObject) && szPath)
		{
			POBJECT_NAME_INFORMATION wcFilePath = NULL;
			pfnIoQueryFileDosDeviceName MzfIoQueryFileDosDeviceName = (pfnIoQueryFileDosDeviceName)GetGlobalVeriable(enumIoQueryFileDosDeviceName);

			if (MzfIoQueryFileDosDeviceName)
			{
				if (NT_SUCCESS(MzfIoQueryFileDosDeviceName(pFileObject, &wcFilePath)))
				{
					if (IsUnicodeStringValid(&wcFilePath->Name))
					{
						ULONG nLen = CmpAndGetStringLength(&wcFilePath->Name, MAX_PATH);
						wcsncpy(szPath, wcFilePath->Name.Buffer, nLen);
						bGetPath = TRUE;
					}

					MzfExFreePoolWithTag(wcFilePath, 0);
				}
			}

			if (!bGetPath)
			{
				pfnIoVolumeDeviceToDosName MzfIoVolumeDeviceToDosName = (pfnIoVolumeDeviceToDosName)GetGlobalVeriable(enumIoVolumeDeviceToDosName);
				if (MzfIoVolumeDeviceToDosName)
				{
					NTSTATUS status = STATUS_UNSUCCESSFUL;
					ULONG uRet= 0;
					PVOID pBuffer = MzfExAllocatePoolWithTag(PagedPool, 0x1000, MZFTAG);

					if (pBuffer)
					{
						// ObQueryNameString : \Device\HarddiskVolume1\Program Files\VMware\VMware Tools\VMwareTray.exe
						memset(pBuffer, 0, 0x1000);
						status = MzfObQueryNameString(pFileObject, (POBJECT_NAME_INFORMATION)pBuffer, 0x1000, &uRet);
						if (NT_SUCCESS(status))
						{
							POBJECT_NAME_INFORMATION pTemp = (POBJECT_NAME_INFORMATION)pBuffer;
							WCHAR szHarddiskVolume[100] = {L'\\', L'D', L'e', L'v', L'i', L'c', L'e', L'\\', L'H', L'a', L'r', L'd', L'd', L'i', L's', L'k', L'V', L'o', L'l', L'u', L'm', L'e', L'\0'};

							if (IsUnicodeStringValid(&pTemp->Name))
							{
								if (pTemp->Name.Length / sizeof(WCHAR) > wcslen(szHarddiskVolume) &&
									!_wcsnicmp(pTemp->Name.Buffer, szHarddiskVolume, wcslen(szHarddiskVolume)))
								{
									// 如果是以 "\\Device\\HarddiskVolume" 这样的形式存在的，那么再查询其卷名。
									UNICODE_STRING pDosName;

									if (NT_SUCCESS(MzfIoVolumeDeviceToDosName(pFileObject->DeviceObject, &pDosName)))
									{
										if (IsUnicodeStringValid(&pDosName))
										{
											ULONG nLen = CmpAndGetStringLength(&pDosName, MAX_PATH);
											wcsncpy(szPath, pDosName.Buffer, nLen);
											nLen = MAX_PATH - nLen > pTemp->Name.Length / sizeof(WCHAR) - wcslen(szHarddiskVolume) - 1 ? pTemp->Name.Length / sizeof(WCHAR) - wcslen(szHarddiskVolume) - 1 : MAX_PATH - nLen - 1;
											wcsncat(szPath, pTemp->Name.Buffer + wcslen(szHarddiskVolume) + 1, nLen);
											bGetPath = TRUE;
										}	

										MzfExFreePoolWithTag(pDosName.Buffer, 0);
									}
								}
								else
								{
									// 如果不是以 "\\Device\\HarddiskVolume" 这样的形式开头的，那么直接复制名称。
									ULONG nLen = CmpAndGetStringLength(&pTemp->Name, MAX_PATH);
									wcsncpy(szPath, pTemp->Name.Buffer, nLen);
									bGetPath = TRUE;
								}
							}
						}

						MzfExFreePoolWithTag(pBuffer, 0);
					}
				}
			}
		}
	}
	__except(1)
	{
		KdPrint(("GetPathByFileObject catch __except\n"));
		bGetPath = FALSE;
	}

	return bGetPath;
}

PIMAGE_SECTION_HEADER
NzfSectionTableFromVirtualAddress (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) 
	{
        if ((ULONG)Address >= NtSection->VirtualAddress &&
            (ULONG)Address < NtSection->VirtualAddress + NtSection->SizeOfRawData) 
		{
            return NtSection;
		}

        ++NtSection;
	}

    return NULL;
}

PVOID
MzfAddressInSectionTable (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )
{
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = NzfSectionTableFromVirtualAddress( NtHeaders,
                                                   Base,
                                                   Address
                                                 );
    if (NtSection != NULL) {
        return( ((PCHAR)Base + ((ULONG_PTR)Address - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }
    else {
        return( NULL );
        }
}

PVOID
MzfImageDirectoryEntryToData32 (
								 IN PVOID Base,
								 IN BOOLEAN MappedAsImage,
								 IN USHORT DirectoryEntry,
								 OUT PULONG Size,
								 PIMAGE_NT_HEADERS32 NtHeaders
								 )
{
	ULONG DirectoryAddress;

	if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
		return( NULL );
	}

	if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
		return( NULL );
	}

	if (Base < MM_HIGHEST_USER_ADDRESS) {
		if ((PVOID)((PCHAR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
			return( NULL );
		}
	}

	*Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
	if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
		return( (PVOID)((PCHAR)Base + DirectoryAddress) );
	}

	return( MzfAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
MzfImageDirectoryEntryToData64 (
								 IN PVOID Base,
								 IN BOOLEAN MappedAsImage,
								 IN USHORT DirectoryEntry,
								 OUT PULONG Size,
								 PIMAGE_NT_HEADERS64 NtHeaders
								 )
{
	ULONG DirectoryAddress;

	if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
		return( NULL );
	}

	if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
		return( NULL );
	}

	if (Base < MM_HIGHEST_USER_ADDRESS) {
		if ((PVOID)((PCHAR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
			return( NULL );
		}
	}

	*Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
	if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
		return( (PVOID)((PCHAR)Base + DirectoryAddress) );
	}

	return( MzfAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}

NTSTATUS
NTAPI
RtlImageNtHeaderEx(
    ULONG Flags,
    PVOID Base,
    ULONG64 Size,
    OUT PIMAGE_NT_HEADERS * OutHeaders
    )
{
    PIMAGE_NT_HEADERS NtHeaders = 0;
    ULONG e_lfanew = 0;
    BOOLEAN RangeCheck = 0;
    NTSTATUS Status = 0;
    const ULONG ValidFlags = 
        RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK;

    if (OutHeaders != NULL) {
        *OutHeaders = NULL;
    }
    if (OutHeaders == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if ((Flags & ~ValidFlags) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Base == NULL || Base == (PVOID)(LONG_PTR)-1) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    RangeCheck = ((Flags & RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) == 0);
    if (RangeCheck) {
        if (Size < sizeof(IMAGE_DOS_HEADER)) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    //
    // Exception handling is not available in the boot loader, and exceptions
    // were not historically caught here in kernel mode. Drivers are considered
    // trusted, so we can't get an exception here due to a bad file, but we
    // could take an inpage error.
    //
#define EXIT goto Exit
    if (((PIMAGE_DOS_HEADER)Base)->e_magic != IMAGE_DOS_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }
    e_lfanew = ((PIMAGE_DOS_HEADER)Base)->e_lfanew;
    if (RangeCheck) {
        if (e_lfanew >= Size
#define SIZEOF_PE_SIGNATURE 4
            || e_lfanew >= (MAXULONG - SIZEOF_PE_SIGNATURE - sizeof(IMAGE_FILE_HEADER))
            || (e_lfanew + SIZEOF_PE_SIGNATURE + sizeof(IMAGE_FILE_HEADER)) >= Size
            ) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
    }

    NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + e_lfanew);

    //
    // In kernelmode, do not cross from usermode address to kernelmode address.
    //
    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)NtHeaders >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
        //
        // Note that this check is slightly overeager since IMAGE_NT_HEADERS has
        // a builtin array of data_directories that may be larger than the image
        // actually has. A better check would be to add FileHeader.SizeOfOptionalHeader,
        // after ensuring that the FileHeader does not cross the u/k boundary.
        //
        if ((PVOID)((PCHAR)NtHeaders + sizeof (IMAGE_NT_HEADERS)) >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
    }

    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }
    Status = STATUS_SUCCESS;

Exit:
    if (NT_SUCCESS(Status)) {
        *OutHeaders = NtHeaders;
    }
    return Status;
}


PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
				 PVOID Base
				 )
{
	PIMAGE_NT_HEADERS NtHeaders = NULL;
	(VOID)RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, Base, 0, &NtHeaders);
	return NtHeaders;
}

PVOID
MzfImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )
{
	PVOID pRet = NULL;

	__try
	{
		PIMAGE_NT_HEADERS NtHeaders;

		if (LDR_IS_DATAFILE(Base)) {
			Base = LDR_DATAFILE_TO_VIEW(Base);
			MappedAsImage = FALSE;
		}

		NtHeaders = RtlImageNtHeader(Base);

		if (!NtHeaders)
			return NULL;

		if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
			return (MzfImageDirectoryEntryToData32(Base,
				MappedAsImage,
				DirectoryEntry,
				Size,
				(PIMAGE_NT_HEADERS32)NtHeaders));
		} else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
			return (MzfImageDirectoryEntryToData64(Base,
				MappedAsImage,
				DirectoryEntry,
				Size,
				(PIMAGE_NT_HEADERS64)NtHeaders));
		} else {
			return (NULL);
		}
	}
	__except(1)
	{
		pRet = NULL;
	}

	return pRet;
}


PVOID
FindExportedRoutineByName (
    IN PVOID DllBase,
    IN CHAR* RoutineName
    )

/*++

Routine Description:

    This function searches the argument module looking for the requested
    exported function name.

Arguments:

    DllBase - Supplies the base address of the requested module.

    AnsiImageRoutineName - Supplies the ANSI routine name being searched for.

Return Value:

    The virtual address of the requested routine or NULL if not found.

--*/

{
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;
    ULONG ExportSize;
    PVOID FunctionAddress;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

	if (!DllBase || !RoutineName)
	{
		return NULL;
	}

	__try{

		ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) MzfImageDirectoryEntryToData (
			DllBase,
			TRUE,
			IMAGE_DIRECTORY_ENTRY_EXPORT,
			&ExportSize);

		if (ExportDirectory == NULL) {
			return NULL;
		}

		//
		// Initialize the pointer to the array of RVA-based ansi export strings.
		//

		NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);

		//
		// Initialize the pointer to the array of USHORT ordinal numbers.
		//

		NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

		//
		// Lookup the desired name in the name table using a binary search.
		//

		Low = 0;
		Middle = 0;
		High = ExportDirectory->NumberOfNames - 1;

		while (High >= Low) {

			//
			// Compute the next probe index and compare the import name
			// with the export name entry.
			//

			Middle = (Low + High) >> 1;

			Result = strcmp (RoutineName,
				(PCHAR)DllBase + NameTableBase[Middle]);

			if (Result < 0) {
				High = Middle - 1;
			}
			else if (Result > 0) {
				Low = Middle + 1;
			}
			else {
				break;
			}
		}

		//
		// If the high index is less than the low index, then a matching
		// table entry was not found. Otherwise, get the ordinal number
		// from the ordinal table.
		//

		if (High < Low) {
			return NULL;
		}

		OrdinalNumber = NameOrdinalTableBase[Middle];

		//
		// If the OrdinalNumber is not within the Export Address Table,
		// then this image does not implement the function.  Return not found.
		//

		if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
			return NULL;
		}

		//
		// Index into the array of RVA export addresses by ordinal number.
		//

		Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

		FunctionAddress = (PVOID)((PCHAR)DllBase + Addr[OrdinalNumber]);
	}
	__except(1)
	{
		FunctionAddress = NULL;
	}
 
    //
    // Forwarders are not used by the kernel and HAL to each other.
    //

//     ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
//             (FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));

    return FunctionAddress;
}

PVOID
MzfGetSystemRoutineAddress (
    CHAR* SystemRoutineName
    )

/*++

Routine Description:

    This function returns the address of the argument function pointer if
    it is in the kernel or HAL, NULL if it is not.

Arguments:

    SystemRoutineName - Supplies the name of the desired routine.

Return Value:

    Non-NULL function pointer if successful.  NULL if not.

Environment:

    Kernel mode, PASSIVE_LEVEL, arbitrary process context.

--*/

{
    PVOID FunctionAddress = NULL;
	WCHAR szNtoskrnl[] = {'n', 't', 'o', 's', 'k', 'r', 'n', 'l', '.', 'e', 'x', 'e', '\0'};
	WCHAR szHal[] = {'h', 'a', 'l', '.', 'd', 'l', 'l', '\0'};
	PLDR_DATA_TABLE_ENTRY FirstEntry = (PLDR_DATA_TABLE_ENTRY)GetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY);
	PLDR_DATA_TABLE_ENTRY LdrEntry = FirstEntry;

	if (!FirstEntry || !SystemRoutineName)
	{
		KdPrint(("if (!DriverSection || !SystemRoutineName) error\n"));
		return NULL;
	}

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

	__try
	{
		do 
		{
			BOOL Found = FALSE;

			if ((DWORD)LdrEntry->DllBase >= SYSTEM_ADDRESS_START	&&
				LdrEntry->BaseDllName.Length > 0					&&
				LdrEntry->BaseDllName.Buffer != NULL)
			{
				if (LdrEntry->BaseDllName.Length == wcslen(szNtoskrnl) * sizeof(WCHAR) &&
					!_wcsnicmp(LdrEntry->BaseDllName.Buffer, szNtoskrnl, wcslen(szNtoskrnl))) 
				{
					Found = TRUE;
				}
				else if (LdrEntry->BaseDllName.Length == wcslen(szHal) * sizeof(WCHAR) &&
					!_wcsnicmp(LdrEntry->BaseDllName.Buffer, szHal, wcslen(szHal)))
				{
					Found = TRUE;
				}

				if (Found == TRUE) {

					FunctionAddress = FindExportedRoutineByName (LdrEntry->DllBase, SystemRoutineName);

					if (FunctionAddress != NULL) 
					{
						break;
					}
				}
			}

			LdrEntry = (PLDR_DATA_TABLE_ENTRY)LdrEntry->InLoadOrderLinks.Flink;

		} while (LdrEntry && FirstEntry != LdrEntry);
	}
	__except(1)
	{
		FunctionAddress = NULL;
	}

    return FunctionAddress;
}

//////////////////////////////////////////////////////////////////// 

VOID Win2kGetKiWaitAndKiWaitOutListHead()
{
	CHAR szKeWaitForSingleObject[100] = {'K', 'e', 'W', 'a', 'i', 't', 'F', 'o', 'r', 'S', 'i', 'n', 'g', 'l', 'e', 'O', 'b', 'j', 'e', 'c', 't'};
	ULONG OldBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG NewBase = GetGlobalVeriable(enumNewKernelBase);
	ULONG bReloadSuccess = GetGlobalVeriable(enumReloadNtosKrnlSuccess);
	ULONG Base = bReloadSuccess ? NewBase : OldBase;
	PBYTE cPtr = FindExportedRoutineByName((PVOID)Base, szKeWaitForSingleObject);
	ULONG Length = 0, uStart = 0, uEnd = 0;
	PLIST_ENTRY WaitInListHead = NULL, DispatcherReadyListHead = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("reload KeWaitForSingleObject: 0x%08X\n", cPtr));
	KdPrint(("old KeWaitForSingleObject: 0x%08X\n", KeWaitForSingleObject));

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	uStart = (ULONG)cPtr;
	uEnd = (ULONG)cPtr + PAGE_SIZE;

	for (; uStart < uEnd; uStart += Length)
	{
		PCHAR pCode;

		if (!MzfMmIsAddressValid((PVOID)uStart))
		{
			break;
		}

		//		Length = GetOpCodeSize(cPtr);
		Length = SizeOfCode((PVOID)uStart, &pCode);

		if (!Length) 
		{
			break;
		}

		if (*(PBYTE)uStart == 0xB9 && *((PBYTE)uStart + 5) == 0x84 && *((PBYTE)uStart + 24) == 0xB9)
		{
			WaitInListHead = *(PLIST_ENTRY *)(uStart + 1);
			DispatcherReadyListHead = *(PLIST_ENTRY *)(uStart + 25);
			break;
		}
	}

	KdPrint(("Win2kGetKiWaitAndKiWaitOutListHead-> WaitInListHead: 0x%08X, DispatcherReadyListHead: 0x%08X\n", WaitInListHead, DispatcherReadyListHead));
	SetGlobalVeriable(enumWaitInListHead, (ULONG)WaitInListHead);
	SetGlobalVeriable(enumDispatcherReadyListHead, (ULONG)DispatcherReadyListHead);
}

//////////////////////////////////////////////////////////////////// 

VOID XPGetKiWaitListHead()
{
	CHAR szKeDelayExecutionThread[100] = {'K', 'e', 'D', 'e', 'l', 'a', 'y', 'E', 'x', 'e', 'c', 'u', 't', 'i', 'o', 'n', 'T', 'h', 'r', 'e', 'a', 'd'};
	ULONG OldBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG NewBase = GetGlobalVeriable(enumNewKernelBase);
	ULONG bReloadSuccess = GetGlobalVeriable(enumReloadNtosKrnlSuccess);
	ULONG Base = bReloadSuccess ? NewBase : OldBase;
	PVOID cPtr = FindExportedRoutineByName((PVOID)Base, szKeDelayExecutionThread);
	ULONG uStart = 0, uEnd = 0;
	PLIST_ENTRY WaitInListHead = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (cPtr)
	{
		uStart = (ULONG)cPtr;
		uEnd = (ULONG)cPtr + PAGE_SIZE;

		for (; uStart < uEnd; uStart++)
		{
			if (MzfMmIsAddressValid((PVOID)uStart))
			{
				if (*(PUSHORT)uStart == 0x3C7 && *(PUSHORT)(uStart + 6) == 0x4389)
				{
					WaitInListHead = *(PLIST_ENTRY *)(uStart + 2);
					break;
				}
			}
		}
	}

	KdPrint(("XPGetKiWaitListHead-> WaitInListHead:0x%08X\n ", WaitInListHead));
	SetGlobalVeriable(enumWaitInListHead, (ULONG)WaitInListHead);
}

//////////////////////////////////////////////////////////////////// 

VOID AboveWin2k3GetKiWaitAndKiWaitOutListHead()
{
	ULONG uKprcb = 0;
	PLIST_ENTRY WaitInListHead = NULL, DispatcherReadyListHead = NULL;
	ULONG WaitListHeadOffset = GetGlobalVeriable(enumWaitListHeadOffset_KPRCB);
	ULONG DispatcherReadyListHeadOffset = GetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB);

	KdPrint(("WaitListHeadOffset: %d \n", WaitListHeadOffset));
	KdPrint(("DispatcherReadyListHeadOffset : %d", DispatcherReadyListHeadOffset));
	__asm{
		push eax
		mov eax, fs:[0x20]
		mov uKprcb, eax
		pop eax
	}

	if (uKprcb)
	{
		WaitInListHead = (PLIST_ENTRY)(uKprcb + WaitListHeadOffset);
		DispatcherReadyListHead = (PLIST_ENTRY)(uKprcb + DispatcherReadyListHeadOffset);
	}

	KdPrint(("AboveWin2k3GetKiWaitAndKiWaitOutListHead-> WaitInListHead: 0x%08X, DispatcherReadyListHead: 0x%08X\n", WaitInListHead, DispatcherReadyListHead));
	SetGlobalVeriable(enumWaitInListHead, (ULONG)WaitInListHead);
	SetGlobalVeriable(enumDispatcherReadyListHead, (ULONG)DispatcherReadyListHead);
}

//////////////////////////////////////////////////////////////////// 

VOID GetWaitListHeadAndDispatcherReadyListHead()
{
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

	switch(WinVersion)
	{
	case enumWINDOWS_2K:
		{
			Win2kGetKiWaitAndKiWaitOutListHead();
		}
		break;

	case enumWINDOWS_XP:
		{
			XPGetKiWaitListHead();
		}
		break;

	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
	case enumWINDOWS_VISTA:
	case enumWINDOWS_VISTA_SP1_SP2:
	case enumWINDOWS_7:
	case enumWINDOWS_8:
		{
			AboveWin2k3GetKiWaitAndKiWaitOutListHead();
		}
		break;
	}
}

BOOL GetPspCidTableByLookupFunction()
{
	ULONG i = 0;
	USHORT sign = 0;
	ULONG uCodeSize = 0;
	ULONG PspCidTable = 0;
	BOOL bRet = FALSE;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	ULONG pFuncAddr = GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("enter GetPspCidTableByLookupFunction\n"));
	if (pFuncAddr && MzfMmIsAddressValid)
	{
		switch(WinVersion)
		{
		case enumWINDOWS_2K:
		case enumWINDOWS_XP:
		case enumWINDOWS_2K3:
		case enumWINDOWS_2K3_SP1_SP2:
			{
				sign = 0x35ff;
			}
			break;

		case enumWINDOWS_VISTA:
		case enumWINDOWS_VISTA_SP1_SP2:
		case enumWINDOWS_7:
			{	
				sign = 0x3d8b;
			}
			break;
		}

		if (sign == 0)
		{
			return FALSE;
		}

		for (i = (ULONG)pFuncAddr; i < (ULONG)pFuncAddr + PAGE_SIZE; i += uCodeSize)
		{
			if (MzfMmIsAddressValid((PVOID)i))
			{
				PCHAR pCode;
				//			uCodeSize = GetOpCodeSize((PBYTE)i);
				uCodeSize = SizeOfCode((PVOID)i, &pCode);

				if (uCodeSize == 0 || *pCode == 0xc3 || *pCode == 0xc2)
				{
					break;
				}

				if (sign == *(PUSHORT)i && 0xe8 == *(PBYTE)(i + 6))
				{  
					PspCidTable = *(PULONG)(i + 2);
					bRet = TRUE;
					break;
				}  
			}
		}
	}

	KdPrint(("GetPspCidTableByLookupFunction -> PspCidTable: 0x%08X\n", PspCidTable));
	SetGlobalVeriable(enumPspCidTable, PspCidTable);
	return bRet;
}

VOID 
Win8GetPspCidTableByLookupFunction()
{
	ULONG i = 0;
	USHORT sign = 0;
	ULONG uCodeSize = 0;
	ULONG PspReferenceCidTableEntry = 0;
	ULONG PspCidTable = 0;
	ULONG pFuncAddr = GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("enter Win8GetPspCidTableByLookupFunction\n"));
	if (pFuncAddr && MzfMmIsAddressValid)
	{
		for (i = (ULONG)pFuncAddr; i < (ULONG)pFuncAddr + 100; i += uCodeSize)
		{
			if (MzfMmIsAddressValid((PVOID)i))
			{
				PCHAR pCode;

				uCodeSize = SizeOfCode((PVOID)i, &pCode);
				if (uCodeSize == 0)
				{
					break;
				}

				if (*(PBYTE)i == 0xe8		 &&
					*(PBYTE)(i - 2) == 0x6a	 &&
					*(PBYTE)(i + 5) == 0x8b  &&
					*(PBYTE)(i + 7) == 0x85)
				{
					PspReferenceCidTableEntry = i + *(PULONG)(i + 1) + 5;
					break;
				}
			}
		}

		if (PspReferenceCidTableEntry && MzfMmIsAddressValid((PVOID)PspReferenceCidTableEntry))
		{
			uCodeSize = 0;
			for (i = (ULONG)PspReferenceCidTableEntry; i < (ULONG)PspReferenceCidTableEntry + 100; i += uCodeSize)
			{
				if (MzfMmIsAddressValid((PVOID)i))
				{
					PCHAR pCode;

					uCodeSize = SizeOfCode((PVOID)i, &pCode);
					if (uCodeSize == 0)
					{
						break;
					}

					if (*(PUSHORT)i == 0x158b			 &&
						*(PUSHORT)(i - 2) == 0xec8b	 &&
						*(PBYTE)(i + 6) == 0x83)
					{
						PspCidTable = *(PULONG)(i + 2);
						break;
					}
				}
			}
		}
	}

	KdPrint(("Win8GetPspCidTableByLookupFunction -> PspCidTable: 0x%08X\n", PspCidTable));
	SetGlobalVeriable(enumPspCidTable, PspCidTable);
}

//////////////////////////////////////////////////////////////////// 

VOID GetKdVersionBlock()
{
	ULONG KdVersionBlock = 0;

	__asm{
		mov eax, fs:[0x1c]
		test eax, eax
		jz _exit_
		mov eax, [eax+0x34]
		test eax, eax
		jz _exit_
		mov KdVersionBlock, eax
_exit_:
	}

	SetGlobalVeriable(enumKdVersionBlock, KdVersionBlock);
}

//////////////////////////////////////////////////////////////////// 

VOID
DpcRoutine(
		   __in struct _KDPC  *Dpc,
		   __in_opt PVOID  DeferredContext,
		   __in_opt PVOID  SystemArgument1,
		   __in_opt PVOID  SystemArgument2
		   )
{
	PKEVENT pEvent = (PKEVENT)DeferredContext;
	pfnKeSetEvent MzfKeSetEvent = (pfnKeSetEvent)GetGlobalVeriable(enumKeSetEvent);

	GetKdVersionBlock();

	if (pEvent)
	{
		MzfKeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);
	}
}

VOID InsertDpcToFindKdVersionBlock()
{
	KAFFINITY CpuAffinity;
	ULONG ulCpuCnt = 0;
	ULONG i = 0;
	pfnKeQueryActiveProcessors MzfKeQueryActiveProcessors = (pfnKeQueryActiveProcessors)GetGlobalVeriable(enumKeQueryActiveProcessors);
	pfnKeInitializeEvent MzfKeInitializeEvent = (pfnKeInitializeEvent)GetGlobalVeriable(enumKeInitializeEvent);
	pfnKeInitializeDpc MzfKeInitializeDpc = (pfnKeInitializeDpc)GetGlobalVeriable(enumKeInitializeDpc);
	pfnKeSetTargetProcessorDpc MzfKeSetTargetProcessorDpc = (pfnKeSetTargetProcessorDpc)GetGlobalVeriable(enumKeSetTargetProcessorDpc);
	pfnKeSetImportanceDpc MzfKeSetImportanceDpc = (pfnKeSetImportanceDpc)GetGlobalVeriable(enumKeSetImportanceDpc);
	pfnKeInsertQueueDpc MzfKeInsertQueueDpc = (pfnKeInsertQueueDpc)GetGlobalVeriable(enumKeInsertQueueDpc);
	pfnKeWaitForSingleObject MzfKeWaitForSingleObject = (pfnKeWaitForSingleObject)GetGlobalVeriable(enumKeWaitForSingleObject);
	pfnKeSetEvent MzfKeSetEvent = (pfnKeSetEvent)GetGlobalVeriable(enumKeSetEvent);

	if (MzfKeQueryActiveProcessors &&
		MzfKeInitializeEvent && 
		MzfKeInitializeDpc &&
		MzfKeSetTargetProcessorDpc && 
		MzfKeSetImportanceDpc && 
		MzfKeInsertQueueDpc && 
		MzfKeWaitForSingleObject &&
		MzfKeSetEvent)
	{
		CpuAffinity = MzfKeQueryActiveProcessors();
		for (i = 0; i < 32; i++)
		{
			if ( (CpuAffinity >> i) & 1 )
			{
				ulCpuCnt++;
			}
		}

		if ( ulCpuCnt == 1 ) 
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			GetKdVersionBlock();
			KeLowerIrql(OldIrql);
		}
		else  
		{
			CpuAffinity = MzfKeQueryActiveProcessors();

			for (i = 0; i < 32; i++) 
			{
				if ((CpuAffinity >> i) & 1) 
				{
					KEVENT Event;
					KDPC Dpc;

					MzfKeInitializeEvent(&Event, NotificationEvent, FALSE);
					MzfKeInitializeDpc(&Dpc, DpcRoutine, &Event);
					MzfKeSetTargetProcessorDpc(&Dpc, (CCHAR)i);
					MzfKeSetImportanceDpc(&Dpc, HighImportance);
					MzfKeInsertQueueDpc(&Dpc, NULL, NULL);

					if (MzfKeWaitForSingleObject(&Event, 0, 0, 0, 0) == STATUS_SUCCESS)
					{
						ULONG KdVersionBlock = GetGlobalVeriable(enumKdVersionBlock);
						if (KdVersionBlock)
						{
							break;
						}
					}
				}
			}
		}
	}
}


VOID GetPspCidTableByKdVersionBlock()
{
	ULONG PspCidTable = 0, KdVersionBlock = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	InsertDpcToFindKdVersionBlock();

	KdVersionBlock = GetGlobalVeriable(enumKdVersionBlock);

	if (KdVersionBlock && MzfMmIsAddressValid((PVOID)KdVersionBlock))
	{
		PspCidTable = *(PULONG)(KdVersionBlock + 0x80);
	}

	KdPrint(("GetPspCidTableByKdVersionBlock-> PspCidTable: 0x%08X\n", PspCidTable));
	SetGlobalVeriable(enumPspCidTable, PspCidTable);
}

////////////////////////////////////////////////////////////////////////////

VOID GetPspCidTable()
{
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

	KdPrint(("enter GetPspCidTable\n"));
	if (WinVersion == enumWINDOWS_8)
	{
		Win8GetPspCidTableByLookupFunction();
	}
	else
	{
		if (!GetPspCidTableByLookupFunction() && WinVersion != enumWINDOWS_2K)
		{
			GetPspCidTableByKdVersionBlock();
		}
	}
}

VOID MzfInitUnicodeString(IN OUT PUNICODE_STRING DestinationString,
					 IN PCWSTR SourceString)
{
	ULONG DestSize;

	if(SourceString)
	{
		DestSize = wcslen(SourceString) * sizeof(WCHAR);
		DestinationString->Length = (USHORT)DestSize;
		DestinationString->MaximumLength = (USHORT)DestSize + sizeof(WCHAR);
	}
	else
	{
		DestinationString->Length = 0;
		DestinationString->MaximumLength = 0;
	}

	DestinationString->Buffer = (PWCHAR)SourceString;
}

BOOL IsRealThread(PETHREAD pThread)
{
	POBJECT_TYPE pObjectType; 
	ULONG ObjectTypeAddress; 
	BOOL bRet = FALSE;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	POBJECT_TYPE ThreadType = (POBJECT_TYPE)GetGlobalVeriable(enumPsThreadType);

	__try
	{
		if (ThreadType && MzfMmIsAddressValid && pThread && MzfMmIsAddressValid((PVOID)(pThread)))
		{ 
			pObjectType = KeGetObjectType((PVOID)pThread);
			if (pObjectType && ThreadType == pObjectType)
			{
				ULONG InitialStack = *(PULONG)((ULONG)pThread + GetGlobalVeriable(enumInitialStackOffset_KTHREAD));
				ULONG KernelStack = *(PULONG)((ULONG)pThread + GetGlobalVeriable(enumKernelStackOffset_KTHREAD));
				ULONG StackLimit = *(PULONG)((ULONG)pThread + GetGlobalVeriable(enumStackLimitOffset_KTHREAD));
				ULONG Teb = *(PULONG)((ULONG)pThread + GetGlobalVeriable(enumTebOffset_KTHREAD));
				PLIST_ENTRY ThreadList1 = *(PLIST_ENTRY *)((ULONG)pThread + GetGlobalVeriable(enumThreadListEntryOffset_ETHREAD));
				PLIST_ENTRY ThreadList2 = *(PLIST_ENTRY *)((ULONG)pThread + GetGlobalVeriable(enumThreadListEntryOffset_KTHREAD));

				if ((ULONG)ThreadList1 > SYSTEM_ADDRESS_START &&
					MzfMmIsAddressValid((PVOID)ThreadList1) &&
					(ULONG)(ThreadList1->Flink) > SYSTEM_ADDRESS_START &&
					MzfMmIsAddressValid(ThreadList1->Flink) &&
					(ULONG)ThreadList2 > SYSTEM_ADDRESS_START &&
					MzfMmIsAddressValid((PVOID)ThreadList2) &&
					(ULONG)(ThreadList2->Flink) > SYSTEM_ADDRESS_START &&
					MzfMmIsAddressValid(ThreadList2->Flink) &&
					Teb < SYSTEM_ADDRESS_START && 
					// Teb > &&	// 系统线程TEB为0
					InitialStack > SYSTEM_ADDRESS_START && 
					KernelStack > SYSTEM_ADDRESS_START && 
					StackLimit > SYSTEM_ADDRESS_START &&
					InitialStack > StackLimit && 
					KernelStack >= StackLimit && 
					KernelStack < InitialStack)
				{
					ULONG ThreadsProcess = *(PULONG)((ULONG)pThread + GetGlobalVeriable(enumThreadsProcessOffset_ETHREAD));
					if ( ThreadsProcess > SYSTEM_ADDRESS_START &&
						MzfMmIsAddressValid((PVOID)ThreadsProcess) &&
						!(ThreadsProcess & 7) &&
						MzfMmIsAddressValid((PVOID)(ThreadsProcess + GetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS)))  )// &&
						//	IsRealProcess((PEPROCESS)ThreadsProcess))
					{
						bRet = TRUE;	
					}
					else
					{
						KdPrint(("thread: 0x%08X   Is not A Real Thread\n", pThread));				
					}
				}
			}
		} 
	}
	__except(1)
	{
		bRet = FALSE;
	}

	return bRet;
}

BOOL IsRealDriverObject(PDRIVER_OBJECT DriverObject)
{
	BOOL bRet = FALSE;
	POBJECT_TYPE DriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);
	POBJECT_TYPE DeviceObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDeviceObjectType);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!DriverObjectType ||
		!DeviceObjectType ||
		!MzfMmIsAddressValid)
	{
		return bRet;
	}

	__try
	{
		if (DriverObject->Type == 4 && 
			DriverObject->Size == sizeof(DRIVER_OBJECT) &&
			KeGetObjectType(DriverObject) == DriverObjectType &&
			MzfMmIsAddressValid(DriverObject->DriverSection) &&
			(ULONG)DriverObject->DriverSection > SYSTEM_ADDRESS_START &&
			!(DriverObject->DriverSize & 0x1F) &&
			DriverObject->DriverSize < SYSTEM_ADDRESS_START &&
			!((ULONG)(DriverObject->DriverStart) & 0xFFF) &&
			(ULONG)DriverObject->DriverStart > SYSTEM_ADDRESS_START
			)
		{
			PDEVICE_OBJECT DeviceObject = DriverObject->DeviceObject;
			if (DeviceObject)
			{
				if (MzfMmIsAddressValid(DeviceObject) &&
					KeGetObjectType(DeviceObject) == DeviceObjectType &&
					DeviceObject->Type == 3 && 
					DeviceObject->Size >= sizeof(DEVICE_OBJECT))
				{
					bRet = TRUE;
				}
			}
			else
			{
				bRet = TRUE;
			}
		}
	}
	__except(1)
	{
		bRet = FALSE;
	}

	return bRet;
}

HANDLE MapFileAsSection(PUNICODE_STRING FileName, PVOID *ModuleBase)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	HANDLE  hSection, hFile;
	DWORD dwKSDT = 0;
	PVOID BaseAddress = NULL;
	SIZE_T size = 0;
	IO_STATUS_BLOCK iosb;
	OBJECT_ATTRIBUTES oa;
	pfnNtOpenFile MzfNtOpenFile = (pfnNtOpenFile)GetGlobalVeriable(enumNtOpenFile);
	pfnNtCreateSection MzfNtCreateSection = (pfnNtCreateSection)GetGlobalVeriable(enumNtCreateSection);
	pfnNtMapViewOfSection MzfNtMapViewOfSection = (pfnNtMapViewOfSection)GetGlobalVeriable(enumNtMapViewOfSection);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;

	if (!FileName ||
		!ModuleBase || 
		!MzfPsGetCurrentThread ||
		!MzfNtOpenFile ||
		!MzfNtCreateSection ||
		!MzfNtMapViewOfSection ||
		!MzfNtClose)
	{
		return NULL;
	}

	InitializeObjectAttributes(&oa, FileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	*ModuleBase = NULL;
	
	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	status = MzfNtOpenFile(
		&hFile, 
		SYNCHRONIZE, 
		&oa, 
		&iosb, 
		FILE_SHARE_READ, 
		FILE_SYNCHRONOUS_IO_NONALERT);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("MzfNtOpenFile error\n"));
		return NULL;
	}

 	oa.ObjectName = 0;

	status = MzfNtCreateSection(
		&hSection, 
		SECTION_ALL_ACCESS, 
		&oa, 
		0,
		PAGE_EXECUTE, 
		SEC_IMAGE, 
		hFile);

	if(!NT_SUCCESS(status))
	{
		MzfNtClose(hFile);
		KdPrint(("ZwCreateSection failed:%d\n", RtlNtStatusToDosError(status)));
		return NULL;
	}

	status = MzfNtMapViewOfSection(
		hSection, 
		NtCurrentProcess(),
		&BaseAddress, 
		0,
		1000, 
		0,
		&size,
		(SECTION_INHERIT)1,
		MEM_TOP_DOWN, 
		PAGE_READWRITE); 

	if(!NT_SUCCESS(status))
	{
		MzfNtClose(hFile);
		MzfNtClose(hSection);
		KdPrint(("MzfNtMapViewOfSection failed:%d\n", RtlNtStatusToDosError(status)));
		return NULL;
	}

	MzfNtClose(hFile);

	__try
	{
		*ModuleBase = BaseAddress;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
	
	RecoverPreMode(pThread, PreMode);
	return hSection;
}

NTSTATUS MzfCloseHandle(HANDLE hHandle)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	
	if (hHandle && MzfPsGetCurrentThread && MzfNtClose)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);
		status = MzfNtClose(hHandle);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

ULONG GetInlineHookAddress(ULONG pAddress, ULONG nLen)
{
	ULONG pRetAddress = 0;
	ULONG nCodeLen = 0;
	PBYTE pCodeAddress = (PBYTE)pAddress;
	ULONG pMove = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid	|| 
		!pAddress				|| 
		!nLen					|| 
		!MzfMmIsAddressValid((PVOID)pAddress))
	{
		return 0;
	}

	__try
	{
		do 
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)pCodeAddress + nCodeLen, MODE_32);

			pRetAddress = 0;

			switch (Inst.type)
			{
			case INSTRUCTION_TYPE_MOV:
				{
					if (Inst.op2.immediate > SYSTEM_ADDRESS_START && 
						MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
					{
						pMove = Inst.op2.immediate;

						if (nLen <= 5)
						{
							pRetAddress = pMove;
						}
					}
				}
				break;

			case INSTRUCTION_TYPE_JMP:
				{
					//	KdPrint(("ff25: Inst.op1.displacement: 0x%08X, Inst.op1.immediate: 0x%08X\n", Inst.op1.displacement, Inst.op1.immediate));

					// ff25 8c1d9cb2  - jmp dword ptr [b29c1d8c] // 绝对跳转
					if(Inst.op1.type == OPERAND_TYPE_MEMORY &&
						Inst.length == 6 &&
						MzfMmIsAddressValid((PVOID)Inst.op1.displacement))
					{
						KdPrint(("ff25: Inst.op1.displacement: 0x%08X, Inst.op1.immediate: 0x%08X\n", Inst.op1.displacement, Inst.op1.immediate));
						pRetAddress = Inst.op1.displacement;
					}

					// jmp eax
					else if (Inst.length == 2 &&
						Inst.op1.type == OPERAND_TYPE_REGISTER &&
						MzfMmIsAddressValid((PVOID)pMove))
					{
						pRetAddress = pMove;	
					}

					// e9 ef9d4a30   jmp Hookport+0xcf44 (b22e9f44) // 远距离相对跳转
					// eb 1d         jmp 011eefe6					// 近距离相对跳转
					else if(Inst.op1.type == OPERAND_TYPE_IMMEDIATE &&
						(Inst.length == 5 || Inst.length == 2) &&
						MzfMmIsAddressValid((PVOID)((ULONG)pCodeAddress + nCodeLen + Inst.op1.immediate + Inst.length)))
					{
						pRetAddress = (ULONG)pCodeAddress + nCodeLen + Inst.op1.immediate + Inst.length;
					}
				}
				break;

			case INSTRUCTION_TYPE_CALL:
				{
					// FF15 12510026	CALL NEAR [26005112]  // 绝对Call
					if(Inst.length == 6 && 
						Inst.op1.type == OPERAND_TYPE_MEMORY &&
						MzfMmIsAddressValid((PVOID)Inst.op1.displacement))
					{
						pRetAddress = Inst.op1.displacement;
					}

					// E8 0502FCFF     call KiDeliverApc
					else if(Inst.length == 5 &&
						Inst.op1.type == OPERAND_TYPE_IMMEDIATE &&
						MzfMmIsAddressValid((PVOID)((ULONG)pCodeAddress + nCodeLen + Inst.op1.immediate + 5)))
					{
						pRetAddress = (ULONG)pCodeAddress + nCodeLen + Inst.op1.immediate + 5;
					}

					//			mov eax, offset SbieDrv+0x1374e (b0b7e74e)
					// ffd0		call eax 
					else if (Inst.length == 2 &&
						Inst.op1.type == OPERAND_TYPE_REGISTER &&
						MzfMmIsAddressValid((PVOID)pMove)) 
					{
						pRetAddress = pMove;
					}

	// 				else if (Inst.opcode == 0x9A) // 这个可能有点问题，因为没看到具体实例
	// 				{
	// 					pRetAddress = (ULONG)(pCodeAddress + nCodeLen) +Inst.op1.immediate + 5;
	// 				}
				}
				break;

			case INSTRUCTION_TYPE_PUSH:
				{
				//	KdPrint(("push ret-> immediate: 0x%08X, displacement: 0x%08X\n", Inst.op1.immediate, Inst.op1.displacement));
					// push xxx
					// ret 
					if (Inst.op1.immediate > SYSTEM_ADDRESS_START &&
						MzfMmIsAddressValid((PVOID)Inst.op1.immediate))
					{
						INSTRUCTION	Instb;

						memset(&Instb, 0, sizeof(INSTRUCTION));
						get_instruction(&Instb, (BYTE *)((pCodeAddress + nCodeLen + Inst.length)), MODE_32);

						if(Instb.type == INSTRUCTION_TYPE_RET)
						{
							pRetAddress = Inst.op1.immediate;
						}
					}
				}
				break;
			}

			if (Inst.length == 0)
			{
				break;
			}

			nCodeLen += Inst.length;

		} while ((!pRetAddress || !MzfMmIsAddressValid((PVOID)pRetAddress)) && nCodeLen < nLen);
	}
	__except(1)
	{
		pRetAddress = 0;
	}

	return pRetAddress;
}

ULONG PspGetDispatchInlineHookAddress(ULONG pAddress)
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

	for (nLen = 0; nLen < 0x100; nLen++)
	{
		if (!MzfMmIsAddressValid((PVOID)(pAddress + nLen)))
		{
			break;
		}
	}

	nRet = GetInlineHookAddress(pAddress, nLen);
	return nRet;
}

NTSTATUS GetInlineAddress(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_SUCCESS;
	PCOMMUNICATE_DISPATCH pcs = (PCOMMUNICATE_DISPATCH)pInBuffer;
	ULONG pAddress = pcs->op.GetInlineAddress.pAddress;

	KdPrint(("enter GetInlineAddress\n"));
	*(PULONG)pOutBuffer = PspGetDispatchInlineHookAddress(pAddress);

	return status;
}

LONG ExchangeAddress(PULONG pTargetAddress, ULONG Value)
{
	LONG nOrigin = 0;
	KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
	WPOFF();
	nOrigin = InterlockedExchange(pTargetAddress, Value);
	WPON();
	KeLowerIrql(OldIrql);
	return nOrigin;
}

void SofeCopyMemory(ULONG pDesAddress, PVOID pSrcAddress, ULONG nLen)
{
	BOOL bOk = TRUE;

	if (!pDesAddress || 
		!pSrcAddress || 
		!nLen ||
		!MmIsAddressValid((PVOID)pDesAddress) ||
		!MmIsAddressValid((PVOID)pSrcAddress) )
	{
		return;
	}

	__try
	{
		ULONG i = 0;

		for (i = 0; i < nLen; i++)
		{
			if (!MmIsAddressValid((PBYTE)pDesAddress + i) ||
				!MmIsAddressValid((PBYTE)pSrcAddress + i))
			{
				return;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bOk = FALSE;
	}

	if (bOk)
	{
		PMDL pMdl = NULL;
		PVOID pRet = NULL;

		__try
		{
			pMdl = IoAllocateMdl((PVOID)(pDesAddress & 0xFFFFF000), (pDesAddress & 0xFFF + nLen + 0xFFF), FALSE, FALSE, NULL);
			if ( pMdl )
			{
				MmBuildMdlForNonPagedPool(pMdl);
				pRet = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);
				if ( pRet )
				{
					KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
					memcpy((PVOID)((ULONG)pRet + (pDesAddress & 0xFFF)), pSrcAddress, nLen);
					KeLowerIrql(OldIrql);
					MmUnmapLockedPages(pRet, pMdl);
				}

				IoFreeMdl(pMdl);
			}
		}
		__except(1)
		{
			if (pMdl)
				IoFreeMdl(pMdl);

			if (pRet)
				MmUnmapLockedPages(pRet, pMdl);
		}
 	}
}

void MemCpy(PVOID pDes, PVOID pSrc, ULONG nLen)
{
	if (!pDes || !pSrc || !nLen)
	{
		return;
	}

	KdPrint(("MY MemCpy\n"));

	SofeCopyMemory((ULONG)pDes, pSrc, nLen);
}

/*************************************************
Returns:   BOOL
Qualifier: 从指定模块的导入表中找到另一个模块导入的函数地址
Parameter: IN PVOID pBase - 模块的首地址
Parameter: IN ULONG ulModuleSize - 模块的大小
Parameter: IN PCHAR ImportDllName - 导入的模块名
Parameter: IN PCHAR ImportApiName - 导入的函数名
Parameter: OUT PULONG ImportFunOffset - 返回导入函数的偏移地址
*************************************************/
BOOL 
FindIatFunction(
		IN PVOID pBase,
		IN ULONG ulModuleSize,
		IN PCHAR ImportDllName,
		IN PCHAR ImportApiName,
		OUT PULONG ImportFunOffset
		)
{
	BOOL bRet = FALSE;
	
	if (pBase && 
		ImportDllName && 
		ImportApiName && 
		ImportApiName && 
		ImportFunOffset &&
		KeGetCurrentIrql() ==  PASSIVE_LEVEL)
	{
		__try
		{
			ULONG ulRet = 0;
			ULONG *OriginalFirstThunk = NULL; 
			ULONG *FirstThunk = NULL; 
			PIMAGE_IMPORT_DESCRIPTOR pImportModuleDirectory = 
				(PIMAGE_IMPORT_DESCRIPTOR)MzfImageDirectoryEntryToData(pBase, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulRet);

			if (pImportModuleDirectory)
			{
				size_t nDllNameLen = strlen(ImportDllName);
				size_t nAPINameLen = strlen(ImportApiName);

				while (pImportModuleDirectory && pImportModuleDirectory->Name)
				{
					PCHAR strModuleName = (PCHAR)((PBYTE)pBase + pImportModuleDirectory->Name);

					if (!_strnicmp(strModuleName, ImportDllName, nDllNameLen))
					{
						PIMAGE_IMPORT_BY_NAME pImageImportByName = NULL;
						ULONG i = 0;

						OriginalFirstThunk    = (ULONG *)((CHAR *)pBase + pImportModuleDirectory->OriginalFirstThunk);
						FirstThunk            = (ULONG *)((CHAR *)pBase + pImportModuleDirectory->FirstThunk);

						for (i = 0; FirstThunk[i] && OriginalFirstThunk[i]; i++)
						{								
							if ( (PVOID)OriginalFirstThunk[i] < (PVOID)pBase )
							{
								pImageImportByName =  (PIMAGE_IMPORT_BY_NAME)(OriginalFirstThunk[i] + (PBYTE)pBase);
							}
							else
							{

								pImageImportByName = (PIMAGE_IMPORT_BY_NAME)OriginalFirstThunk[i];
							}

							if (pImageImportByName && 
								(PBYTE)pImageImportByName > (PBYTE)pBase && 
								(PBYTE)pImageImportByName < (PBYTE)((PBYTE)pBase + ulModuleSize))
							{
								if ( !strncmp((PCHAR)&pImageImportByName->Name, ImportApiName, nAPINameLen))
								{     										
//									KdPrint(("Find %s IAT Addr:%p Func Addr:%p", ImportApiName, &FirstThunk[i], FirstThunk[i]));
//									KdPrint(("&FirstThunk[i] offset: 0x%08X", (ULONG)&FirstThunk[i] - (ULONG)pBase));
									*ImportFunOffset = (ULONG)&FirstThunk[i] - (ULONG)pBase;
									bRet = TRUE;
									break;   
								}                                             
							}                              
						}

						break;
					}

					pImportModuleDirectory++;
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

/*************************************************
Returns:   
Qualifier: 根据映像基址得到模块执行入口，即DriverEntry
Parameter: PVOID ImageBase - 映像基址
Parameter: DWORD * pOutDriverEntry - 返回的映像入口地址
Parameter: ENTRY_POINT_TYPE EntryType - 映像入口类型 
					1.enumEntryPointHead - 直接EntryPoint即是DriverEntry
					2.enumEntryPointJmp - EntryPoint第一个JMP到的地址才是真正的DriverEntry
*************************************************/
BOOL GetDriverEntryPoint(IN PVOID ImageBase, OUT DWORD *pOutDriverEntry, IN ENTRY_POINT_TYPE EntryType)
{
	BOOL bRet = FALSE;
	PIMAGE_NT_HEADERS NtHeaders = NULL;
	ULONG_PTR EntryPoint = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!ImageBase || !pOutDriverEntry || !MzfMmIsAddressValid || enumEntryNone == EntryType)
	{
		return FALSE;
	}

	NtHeaders = RtlImageNtHeader(ImageBase);
	if (NtHeaders)
	{
		DWORD dwEntryPoint = 0;
		DWORD dwCurAddress = 0;
		DWORD Length = 0;
		PUCHAR pOpcode;

		EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
		EntryPoint += (ULONG_PTR)ImageBase;
		dwEntryPoint = (DWORD)EntryPoint;

		if (EntryType == enumEntryPointHead)
		{
			*pOutDriverEntry = dwEntryPoint;
			bRet = TRUE;
		}
		else if (EntryType == enumEntryPointJmp)
		{
			for(dwCurAddress = dwEntryPoint; dwCurAddress < dwEntryPoint + PAGE_SIZE; dwCurAddress += Length)
			{
				Length = SizeOfCode((PUCHAR)dwCurAddress, &pOpcode);
				if(Length == 5 && *(PBYTE)dwCurAddress == 0xE9)
				{  
					ULONG pJmpAddress = dwCurAddress + *(PULONG)(dwCurAddress + 1) + 5;

					if (pJmpAddress > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)pJmpAddress))
					{
						*pOutDriverEntry = pJmpAddress;
						bRet = TRUE;
						break;
					}
				}
			}
		}
	}

	return bRet;
}

/*************************************************
Returns:   
Qualifier: 根据对象名取得对象
Parameter: WCHAR * szDriverName - 对象名
Parameter: PDRIVER_OBJECT * pDriverObject - 返回的对象名对应的对象
*************************************************/
NTSTATUS GetDriverObjectByName(IN WCHAR *szDriverName, OUT PDRIVER_OBJECT *pDriverObject)
{
	PDRIVER_OBJECT DriverObject;
	UNICODE_STRING ObjectName;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	pfnObReferenceObjectByName MzfObReferenceObjectByName = (pfnObReferenceObjectByName)GetGlobalVeriable(enumObReferenceObjectByName);
	POBJECT_TYPE DriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);

	if (!MzfObReferenceObjectByName ||
		!szDriverName || 
		!pDriverObject ||
		!DriverObjectType)
	{
		return Status;
	}
	
	MzfInitUnicodeString(&ObjectName, szDriverName);
	Status = MzfObReferenceObjectByName(&ObjectName, 
		OBJ_CASE_INSENSITIVE,
		NULL,
		0,
		DriverObjectType,
		KernelMode,
		NULL,
		&DriverObject);

	if (NT_SUCCESS(Status))
	{
		*pDriverObject = DriverObject;
	}
	else
	{
		*pDriverObject = NULL;
		KdPrint(("ObReferenceObjectByName failed:%d", RtlNtStatusToDosError(Status)));
	}

	return Status;
}

void GetIopInvalidDeviceRequest()
{
	ULONG pIoCreateDriver = GetGlobalVeriable(enumIoCreateDriver);
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	KdPrint(("enter GetIopInvalidDeviceRequest\n"));
	KdPrint(("IoCreateDriver: 0x%08X\n", pIoCreateDriver));
	if (pIoCreateDriver && MzfMmIsAddressValid)
	{
		ULONG Address = 0, nCodeLen = 0; 
		for (Address = pIoCreateDriver; Address < pIoCreateDriver + PAGE_SIZE; Address += nCodeLen)
		{
			PCHAR pCode;

			if (!MzfMmIsAddressValid((PVOID)Address))
			{
				break;
			}

			nCodeLen = SizeOfCode((PVOID)Address, &pCode);
			if (nCodeLen == 0)
			{
				break;
			}

			if (Version == enumWINDOWS_2K)
			{
				if (nCodeLen == 7 && *(PBYTE)Address == 0xc7 && MzfMmIsAddressValid((PVOID)(*(PULONG)(Address + 3))))
				{
					SetGlobalVeriable(enumIopInvalidDeviceRequest, *(PULONG)(Address + 3));
					break;
				}
			}
			else
			{
				if (nCodeLen == 5 && *(PBYTE)Address == 0xb8 && MzfMmIsAddressValid((PVOID)(*(PULONG)(Address + 1))))
				{
					SetGlobalVeriable(enumIopInvalidDeviceRequest, *(PULONG)(Address + 1));
					break;
				}
			}
		}
	}
}

/*************************************************
Returns:   void
Qualifier: 将原始的Dispatch数组空余的部分用IopInvalidDeviceRequest填充
Parameter: PULONG pOriginDispatchs - 原始的Dispatch数组
*************************************************/
void FixOriginDispatch(PULONG pOriginDispatchs)
{
	ULONG IopInvalidDeviceRequest = GetGlobalVeriable(enumIopInvalidDeviceRequest);
	ULONG i = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!IopInvalidDeviceRequest)
	{
		GetIopInvalidDeviceRequest();
		IopInvalidDeviceRequest = GetGlobalVeriable(enumIopInvalidDeviceRequest);
	}

 	KdPrint(("IopInvalidDeviceRequest: 0x%08X\n", IopInvalidDeviceRequest));

	for (i = 0; i < 28; i++)
	{
		if (!pOriginDispatchs[i])
		{
			pOriginDispatchs[i] = IopInvalidDeviceRequest;
		}
	}
}

/*************************************************
Returns:   void
Qualifier: 用reload之後的IopInvalidDeviceRequest地址，填充ReloadDispatch數組
Parameter: PULONG pReloadDispatchs - reload之后的Dispatch數組
*************************************************/
void FixReloadDispatch(PULONG pReloadDispatchs)
{
	ULONG IopInvalidDeviceRequest = GetGlobalVeriable(enumIopInvalidDeviceRequest);
	ULONG OriginBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG ReloadBase = GetGlobalVeriable(enumNewKernelBase);
	ULONG i = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!IopInvalidDeviceRequest)
	{
		GetIopInvalidDeviceRequest();
		IopInvalidDeviceRequest = GetGlobalVeriable(enumIopInvalidDeviceRequest);
	}

	IopInvalidDeviceRequest = IopInvalidDeviceRequest - OriginBase + ReloadBase;
	KdPrint(("Reload IopInvalidDeviceRequest: 0x%08X\n", IopInvalidDeviceRequest));

	for (i = 0; i < 28; i++)
	{
		if (!pReloadDispatchs[i])
		{
			pReloadDispatchs[i] = IopInvalidDeviceRequest;
		}
	}
}

PIMAGE_SECTION_HEADER GetSecionAddress(PIMAGE_NT_HEADERS NtHeaders, CHAR *szSectionName)
{
	ULONG i = 0;
	PIMAGE_SECTION_HEADER NtSection = NULL;

	if (!NtHeaders || !szSectionName)
	{
		return NULL;
	}

	NtSection = IMAGE_FIRST_SECTION( NtHeaders );
	for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) 
	{
		if (!_strnicmp(NtSection->Name, szSectionName, strlen(szSectionName))) 
		{
			return NtSection;
		}

		++NtSection;
	}

	KdPrint(("NtSection: 0x%08X\n", NtSection));

	return NULL;
}

//*************************************************
// Returns:   返回模块的基址
// Qualifier: 通过模块名，找到模块的基址和大小
// Parameter: WCHAR * szModuleName - 宽字节型的模块名。 eg. hal.dll
// Parameter: DWORD * nModuleSize - 返回模块的大小
//*************************************************
PVOID LookupKernelModuleByNameW(WCHAR* szModuleName, DWORD* nModuleSize)
{
	PLDR_DATA_TABLE_ENTRY FristEntry, LdrEntry;
// 	WCHAR *Buffer = NULL;
	UNICODE_STRING UniKernelModuleName;
/*	UNICODE_STRING ModuleName;*/
	int Lentgh = 0, Index = 0;
	DWORD dwSize = 0;
	PVOID pDllBase = NULL;

	FristEntry = LdrEntry = (PLDR_DATA_TABLE_ENTRY)GetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY);

	if (!FristEntry || !szModuleName)
	{
		return NULL;
	}

// 	Buffer = (WCHAR *)ExAllocatePoolWithTag(PagedPool, MAX_PATH_WCHAR, MZFTAG);
// 
// 	if (!Buffer)
// 	{
// 		return NULL;
// 	}
// 
// 	RtlZeroMemory(Buffer, MAX_PATH_WCHAR);

	RtlInitUnicodeString(&UniKernelModuleName, szModuleName);

	__try
	{
		do 
		{
// 			if ((ULONG)LdrEntry->DllBase > SYSTEM_ADDRESS_START &&
// 				LdrEntry->FullDllName.Length > 0 &&
// 				LdrEntry->FullDllName.Buffer != NULL &&
// 				MmIsAddressValid(&LdrEntry->FullDllName.Buffer[LdrEntry->FullDllName.Length / sizeof(WCHAR) - 1]))
// 			{
// 				Lentgh = LdrEntry->FullDllName.Length / sizeof(WCHAR);
// 
// 				for (Index = Lentgh - 1; Index > 0; Index--)
// 				{
// 					if (LdrEntry->FullDllName.Buffer[Index] == L'\\')
// 					{
// 						break;
// 					}
// 				}
// 
// 				if (LdrEntry->FullDllName.Buffer[Index] == L'\\')
// 				{
// 					RtlCopyMemory(Buffer, &(LdrEntry->FullDllName.Buffer[Index + 1]), (Lentgh - Index - 1) * sizeof(WCHAR));
// 					ModuleName.Buffer = Buffer;
// 					ModuleName.Length = (Lentgh - Index - 1) * sizeof(WCHAR);
// 					ModuleName.MaximumLength = MAX_PATH_WCHAR;
// 				}
// 				else
// 				{
// 					RtlCopyMemory(Buffer, LdrEntry->FullDllName.Buffer, Lentgh * sizeof(WCHAR));
// 					ModuleName.Buffer = Buffer;
// 					ModuleName.Length = Lentgh * sizeof(WCHAR);
// 					ModuleName.MaximumLength = MAX_PATH_WCHAR;
// 
// 				}
// 
// 				if (RtlEqualUnicodeString(&ModuleName, &UniKernelModuleName, TRUE))
// 				{
// 					dwSize = LdrEntry->SizeOfImage;
// 					pDllBase = LdrEntry->DllBase;
// 					break;
// 				}
// 			}	

			if ((ULONG)LdrEntry->DllBase > SYSTEM_ADDRESS_START &&
				LdrEntry->BaseDllName.Length > 0 &&
				LdrEntry->BaseDllName.Buffer != NULL &&
				MmIsAddressValid(LdrEntry->BaseDllName.Buffer) &&
				MmIsAddressValid(&LdrEntry->BaseDllName.Buffer[LdrEntry->BaseDllName.Length / sizeof(WCHAR) - 1]))
			{
				if (RtlEqualUnicodeString(&LdrEntry->BaseDllName, &UniKernelModuleName, TRUE))
				{
					dwSize = LdrEntry->SizeOfImage;
					pDllBase = LdrEntry->DllBase;
					break;
				}
			}	

			LdrEntry = (PLDR_DATA_TABLE_ENTRY)LdrEntry->InLoadOrderLinks.Flink;

		} while (LdrEntry && FristEntry != LdrEntry);
	}
	__except(1)
	{
		pDllBase = NULL;
		dwSize = 0;
	}

//	ExFreePool(Buffer);

	if (nModuleSize)
	{
		*nModuleSize = dwSize;
	}

	return pDllBase;
}

//*************************************************
// Returns:   返回模块的基址
// Qualifier: 通过模块名，找到模块的基址和大小
// Parameter: CHAR * szModuleName - 多字节型的模块名。 eg. hal.dll
// Parameter: DWORD * nModuleSize - 返回模块的大小
//*************************************************
PVOID LookupKernelModuleByName(IN CHAR* szModuleName, OUT DWORD* nModuleSize)
{
	PLDR_DATA_TABLE_ENTRY FirstEntry,LdrEntry;
	ANSI_STRING AnsiKernelModuleName;
	UNICODE_STRING UniKernelModuleName;
	//UNICODE_STRING ModuleName;
//	WCHAR *Buffer = NULL;
	int Lentgh, Index;
	PVOID pDllBase = NULL;
	DWORD dwSize = 0;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnRtlInitAnsiString MzfRtlInitAnsiString = (pfnRtlInitAnsiString)GetGlobalVeriable(enumRtlInitAnsiString);
	pfnRtlAnsiStringToUnicodeString MzfRtlAnsiStringToUnicodeString = (pfnRtlAnsiStringToUnicodeString)GetGlobalVeriable(enumRtlAnsiStringToUnicodeString);
	pfnRtlFreeUnicodeString MzfRtlFreeUnicodeString = (pfnRtlFreeUnicodeString)GetGlobalVeriable(enumRtlFreeUnicodeString);
	
	FirstEntry = LdrEntry = (PLDR_DATA_TABLE_ENTRY)GetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY);

	if (!szModuleName || 
		!FirstEntry || 
		!MzfExAllocatePoolWithTag || 
		!MzfMmIsAddressValid || 
		!MzfExFreePoolWithTag || 
		!MzfRtlInitAnsiString || 
		!MzfRtlAnsiStringToUnicodeString || 
		!MzfRtlFreeUnicodeString)
	{
		return FALSE;
	}

// 	Buffer = MzfExAllocatePoolWithTag(PagedPool, MAX_PATH_WCHAR, MZFTAG);
// 	if (Buffer == NULL)
// 	{
// 		return NULL;
// 	}
	
//	RtlZeroMemory(Buffer, MAX_PATH_WCHAR);
	MzfRtlInitAnsiString(&AnsiKernelModuleName, szModuleName);
	MzfRtlAnsiStringToUnicodeString(&UniKernelModuleName, &AnsiKernelModuleName, TRUE);

	__try
	{
		
		do 
		{
			/*if ((DWORD)LdrEntry->DllBase >= SYSTEM_ADDRESS_START &&
				LdrEntry->FullDllName.Length > 0 &&
				LdrEntry->FullDllName.Buffer != NULL)
			{
				if (MzfMmIsAddressValid(&LdrEntry->FullDllName.Buffer[LdrEntry->FullDllName.Length/2-1]))
				{
					Lentgh = LdrEntry->FullDllName.Length / sizeof(WCHAR);
					for (Index = Lentgh - 1; Index > 0; Index--)
					{
						if (LdrEntry->FullDllName.Buffer[Index] == '\\')
						{
							break;
						}
					}

					if (LdrEntry->FullDllName.Buffer[Index] == '\\')
					{
						RtlCopyMemory(Buffer, &(LdrEntry->FullDllName.Buffer[Index + 1]), (Lentgh - Index - 1) * sizeof(WCHAR));
						ModuleName.Buffer = Buffer;
						ModuleName.Length = (Lentgh - Index - 1) * sizeof(WCHAR);
						ModuleName.MaximumLength = MAX_PATH_WCHAR;
					}
					else
					{
						RtlCopyMemory(Buffer, LdrEntry->FullDllName.Buffer, Lentgh * sizeof(WCHAR));
						ModuleName.Buffer = Buffer;
						ModuleName.Length = Lentgh * sizeof(WCHAR);
						ModuleName.MaximumLength = MAX_PATH_WCHAR;

					}

					if (RtlEqualUnicodeString(&ModuleName, &UniKernelModuleName, TRUE))
					{
						pDllBase = LdrEntry->DllBase;
						dwSize = LdrEntry->SizeOfImage;
						break;
					}
				}
			}	*/

			if ((DWORD)LdrEntry->DllBase >= SYSTEM_ADDRESS_START &&
				LdrEntry->BaseDllName.Length > 0 &&
				LdrEntry->BaseDllName.Buffer != NULL &&
				MzfMmIsAddressValid(LdrEntry->BaseDllName.Buffer) &&
				MzfMmIsAddressValid(&LdrEntry->BaseDllName.Buffer[LdrEntry->BaseDllName.Length/2-1]))
			{
				if (RtlEqualUnicodeString(&LdrEntry->BaseDllName, &UniKernelModuleName, TRUE))
				{
					pDllBase = LdrEntry->DllBase;
					dwSize = LdrEntry->SizeOfImage;
					break;
				}
			}	

			LdrEntry = (PLDR_DATA_TABLE_ENTRY)LdrEntry->InLoadOrderLinks.Flink;

		} while (LdrEntry && FirstEntry != LdrEntry);	
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		pDllBase = NULL;
		dwSize = 0;
	}

	MzfRtlFreeUnicodeString(&UniKernelModuleName);
//	MzfExFreePoolWithTag(Buffer, 0);
	
	if (nModuleSize)
	{
		*nModuleSize = dwSize;
	}

	return pDllBase;
}

PVOID MyQuerySystemInformation(SYSTEM_INFORMATION_CLASS nClass)
{	
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtQuerySystemInformation MzfNtQuerySystemInformation = (pfnNtQuerySystemInformation)GetGlobalVeriable(enumNtQuerySystemInformation);
	pfnPsGetCurrentProcessId MzfPsGetCurrentProcessId = (pfnPsGetCurrentProcessId)GetGlobalVeriable(enumPsGetCurrentProcessId);
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pBuffer = NULL;
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	ULONG ulRet = PAGE_SIZE * 4;
	BOOL bOk = FALSE;

	if (!MzfPsGetCurrentProcessId ||
		!MzfNtQuerySystemInformation ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!MzfPsGetCurrentThread)
	{
		return pBuffer;
	}

	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	do 
	{
		pBuffer = MzfExAllocatePoolWithTag(PagedPool, ulRet, MZFTAG);
		if (pBuffer)
		{
			memset(pBuffer, 0, ulRet);
			status = MzfNtQuerySystemInformation(nClass, pBuffer, ulRet, &ulRet);
			if (NT_SUCCESS(status))
			{
				bOk = TRUE;
				break;
			}

			MzfExFreePoolWithTag(pBuffer, 0);
			pBuffer = NULL;
			ulRet *= 2;
		}
	} while (status == STATUS_INFO_LENGTH_MISMATCH);

	RecoverPreMode(pThread, PreMode);

	if (!bOk && pBuffer)
	{
		MzfExFreePoolWithTag(pBuffer, 0);
		pBuffer = NULL;
	}

	return pBuffer;
}

BOOL IsAddressValid(PVOID pAddress, ULONG nLen)
{	
	BOOL bRet = FALSE;

	__try
	{
		if (nLen == 0)
		{
			bRet = MmIsAddressValid(pAddress);
		}
		else if (nLen > 0)
		{
			ULONG i = 0;
			for (i = 0; i < nLen; i++)
			{
				bRet = MmIsAddressValid((PVOID)((PBYTE)pAddress + i));
				if (bRet == FALSE)
				{
					break;
				}
			}
		}
	}
	__except(1)
	{
		bRet = FALSE;
	}

	return bRet;
}

NTSTATUS SafeCopyMemory_R0_to_R3(PVOID SrcAddr, PVOID DstAddr, ULONG Size)
{
	PMDL  pSrcMdl = NULL, pDstMdl = NULL;
	PUCHAR pSrcAddress = NULL, pDstAddress = NULL;
	NTSTATUS st = STATUS_UNSUCCESSFUL;

	pSrcMdl = IoAllocateMdl(SrcAddr, Size, FALSE, FALSE, NULL);
	if (!pSrcMdl)
	{
		return st;
	}

	MmBuildMdlForNonPagedPool(pSrcMdl);

	pSrcAddress = MmGetSystemAddressForMdlSafe(pSrcMdl, NormalPagePriority);
	if (!pSrcAddress)
	{
		IoFreeMdl(pSrcMdl);
		return st;
	}

	pDstMdl = IoAllocateMdl(DstAddr, Size, FALSE, FALSE, NULL);
	if (!pDstMdl)
	{
		IoFreeMdl(pSrcMdl);
		return st;
	}

	__try
	{
		MmProbeAndLockPages(pDstMdl, UserMode, IoWriteAccess);
		pDstAddress = MmGetSystemAddressForMdlSafe(pDstMdl, NormalPagePriority);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	if (pDstAddress)
	{
		RtlCopyMemory(pDstAddress, pSrcAddress, Size);
		MmUnlockPages(pDstMdl);
		st = STATUS_SUCCESS;
	}

	IoFreeMdl(pDstMdl);      
	IoFreeMdl(pSrcMdl);

	return st;
}


//
// SrcAddr - 源地址，是R3地址
// DstAddr - 目的地址，是R0地址
// Size - 大小
//
NTSTATUS SafeCopyMemory_R3_to_R0(ULONG SrcAddr, ULONG DstAddr, ULONG Size)
{
	ULONG Remaining = 0, Temp = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nRemainSize = PAGE_SIZE - (SrcAddr & 0xFFF);
	ULONG nCopyedSize = 0;

	if (!SrcAddr ||
		!DstAddr ||
		!Size)
	{
		return status;
	}

	KdPrint(("SafeCopyMemory_R3_to_R0 -> SrcAddr: 0x%08X, DstAddr: 0x%08X, Size: 0x%08X\n", SrcAddr, DstAddr, Size));
	
	while ( nCopyedSize < Size )
	{
		PMDL pSrcMdl = NULL;
		PVOID pMappedSrc = NULL;

		if ( Size - nCopyedSize < nRemainSize )
		{
			nRemainSize = Size - nCopyedSize;
		}

		pSrcMdl = IoAllocateMdl( (PVOID)(SrcAddr & 0xFFFFF000), PAGE_SIZE, FALSE, FALSE, NULL );
		if ( pSrcMdl )
		{
			__try
			{
				MmProbeAndLockPages(pSrcMdl, UserMode, IoReadAccess);
				pMappedSrc = MmGetSystemAddressForMdlSafe(pSrcMdl, NormalPagePriority);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
		
		if (pMappedSrc)
		{
			RtlCopyMemory((PVOID)DstAddr, (PVOID)((ULONG)pMappedSrc + (SrcAddr & 0xFFF)), nRemainSize);
			MmUnlockPages( pSrcMdl );
		}
		
		if (pSrcMdl)
		{
			IoFreeMdl(pSrcMdl);
		}
		
		if ( nCopyedSize )
		{
			nRemainSize = PAGE_SIZE;
		}

		nCopyedSize += nRemainSize;
		SrcAddr += nRemainSize;
		DstAddr += nRemainSize;
	}

	status = STATUS_SUCCESS;

	return status;
}

NTSTATUS SafeCopyProcessModules(PEPROCESS pEprocess, ULONG nBase, ULONG nSize, PVOID pOutBuffer)
{
	BOOL bAttach = FALSE;
	KAPC_STATE ks;
	PVOID pBuffer = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);

	if ( nSize == 0 || pOutBuffer == NULL || pEprocess == NULL)
	{
		return status;
	}

	if (!MzfExAllocatePoolWithTag || 
		!MzfExFreePoolWithTag || 
		!MzfKeStackAttachProcess || 
		!MzfKeUnstackDetachProcess || 
		!MzfIoGetCurrentProcess)
	{
		return status;
	}

	pBuffer = MzfExAllocatePoolWithTag(NonPagedPool, nSize, MZFTAG);
	if (!pBuffer)
	{
		return status;
	}

	memset(pBuffer, 0, nSize);

	if (pEprocess != MzfIoGetCurrentProcess())
	{
		MzfKeStackAttachProcess(pEprocess, &ks);
		bAttach = TRUE;
	}

	status = SafeCopyMemory_R3_to_R0(nBase, (ULONG)pBuffer, nSize);

	if (bAttach)
	{
		MzfKeUnstackDetachProcess(&ks);
		bAttach = FALSE;
	}

	if (NT_SUCCESS(status))
	{
		status = SafeCopyMemory_R0_to_R3(pBuffer, pOutBuffer, nSize);
	}
	
	if (pBuffer)
	{
		MzfExFreePoolWithTag(pBuffer, 0);
		pBuffer = NULL;
	}

	return status;
}