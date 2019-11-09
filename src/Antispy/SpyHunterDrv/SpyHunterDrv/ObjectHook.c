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
#include "ObjectHook.h"
#include "CommonFunction.h"
#include "InitWindows.h"
#include "Process.h"
#include "..\\..\\Common\\Common.h"
#include "libdasm.h"

NTSTATUS GetAllObjectTypes(POBJECT_TYPE_INFO pTypes, ULONG nCnt)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES objectAttributes; 
	UNICODE_STRING unObjectTypes; 
	HANDLE handle;
	PVOID Object = NULL;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	WCHAR szObjectTypes[] = {L'\\', L'O', L'b', L'j', L'e', L'c', L't', L'T', L'y', L'p', L'e', L's', L'\0'};
	PETHREAD pThread = NULL;
	CHAR PreMode = 0;
	ULONG nTypes = 0;
	pfnNtOpenDirectoryObject MzfNtOpenDirectoryObject = (pfnNtOpenDirectoryObject)GetGlobalVeriable(enumNtOpenDirectoryObject);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	WCHAR szDirectory[] = {'D','i','r','e','c','t','o','r','y','\0'};
	WCHAR szMutant[] = {'M','u','t','a','n','t','\0'};
	WCHAR szThread[] = {'T','h','r','e','a','d','\0'};
	WCHAR szFilterCommunicationPort[] = {'F','i','l','t','e','r','C','o','m','m','u','n','i','c','a','t','i','o','n','P','o','r','t','\0'};
	WCHAR szController[] = {'C','o','n','t','r','o','l','l','e','r','\0'};
	WCHAR szProfile[] = {'P','r','o','f','i','l','e','\0'};
	WCHAR szEvent[] = {'E','v','e','n','t','\0'};
	WCHAR szType[] = {'T','y','p','e','\0'};
	WCHAR szSection[] = {'S','e','c','t','i','o','n','\0'};
	WCHAR szEventPair[] = {'E','v','e','n','t','P','a','i','r','\0'};
	WCHAR szSymbolicLink[] = {'S','y','m','b','o','l','i','c','L','i','n','k','\0'};
	WCHAR szDesktop[] = {'D','e','s','k','t','o','p','\0'};
	WCHAR szTimer[] = {'T','i','m','e','r','\0'};
	WCHAR szFile[] = {'F','i','l','e','\0'};
	WCHAR szWindowStation[] = {'W','i','n','d','o','w','S','t','a','t','i','o','n','\0'};
	WCHAR szDriver[] = {'D','r','i','v','e','r','\0'};
	WCHAR szWmiGuid[] = {'W','m','i','G','u','i','d','\0'};
	WCHAR szKeyedEvent[] = {'K','e','y','e','d','E','v','e','n','t','\0'};
	WCHAR szDevice[] = {'D','e','v','i','c','e','\0'};
	WCHAR szToken[] = {'T','o','k','e','n','\0'};
	WCHAR szDebugObject[] = {'D','e','b','u','g','O','b','j','e','c','t','\0'};
	WCHAR szIoCompletion[] = {'I','o','C','o','m','p','l','e','t','i','o','n','\0'};
	WCHAR szProcess[] = {'P','r','o','c','e','s','s','\0'};
	WCHAR szAdapter[] = {'A','d','a','p','t','e','r','\0'};
	WCHAR szKey[] = {'K','e','y','\0'};
	WCHAR szJob[] = {'J','o','b','\0'};
	WCHAR szWaitablePort[] = {'W','a','i','t','a','b','l','e','P','o','r','t','\0'};
	WCHAR szPort[] = {'P','o','r','t','\0'};
	WCHAR szAlpcPort[] = {'A','L','P','C',' ', 'P','o','r','t','\0'};
	WCHAR szCallback[] = {'C','a','l','l','b','a','c','k','\0'};
	WCHAR szFilterConnectionPort[] = {'F','i','l','t','e','r','C','o','n','n','e','c','t','i','o','n','P','o','r','t','\0'};
	WCHAR szSemaphore[] = {'S','e','m','a','p','h','o','r','e','\0'};

	if (!nCnt ||
		!pTypes ||
		!MzfNtOpenDirectoryObject || 
		!MzfPsGetCurrentThread || 
		!MzfNtClose || 
		!MzfMmIsAddressValid || 
		!MzfObReferenceObjectByHandle || 
		!MzfObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	MzfInitUnicodeString(&unObjectTypes, szObjectTypes);
	InitializeObjectAttributes(&objectAttributes, &unObjectTypes, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	
	pThread = MzfPsGetCurrentThread();
	PreMode = ChangePreMode(pThread);

	ntStatus = MzfNtOpenDirectoryObject(&handle, 0, &objectAttributes);
	if ( NT_SUCCESS(ntStatus) )
	{
		ntStatus = MzfObReferenceObjectByHandle(handle, 0, 0, 0, &Object, 0);
		if ( NT_SUCCESS(ntStatus) )
		{
			int i = 0;
			POBJECT_DIRECTORY pOd = NULL;
			ULONG nChars = 0;
			WCHAR *szTypeName = NULL;

			MzfObfDereferenceObject(Object);
			pOd = (POBJECT_DIRECTORY)Object;
		
			__try
			{
				for (i = 0; i < NUMBER_HASH_BUCKETS; i++)
				{
					POBJECT_DIRECTORY_ENTRY pode = pOd->HashBuckets[i];
					for (; pode; pode = pode->ChainLink)
					{
						if (MzfMmIsAddressValid(pode->Object))
						{
							switch (WinVersion)
							{
							case enumWINDOWS_2K:
							case enumWINDOWS_XP:
							case enumWINDOWS_2K3:
							case enumWINDOWS_2K3_SP1_SP2:
							case enumWINDOWS_VISTA:
								{
									POBJECT_TYPE_XP pOt = (POBJECT_TYPE_XP)pode->Object;
									ULONG nLen = 0;

									KdPrint(("Name: %wZ, Object: 0x%08X\n", &pOt->Name, pode->Object));
									nChars = pOt->Name.Length / sizeof(WCHAR);
									szTypeName = pOt->Name.Buffer;
									
									if (nTypes < nCnt)
									{
										pTypes[nTypes].ObjectType = (ULONG)pode->Object;
										nLen = CmpAndGetStringLength(&pOt->Name, MAX_TYPE_NAME);
										wcsncpy(pTypes[nTypes++].szTypeName, szTypeName, nLen);
									}
								}
								break;

							case enumWINDOWS_VISTA_SP1_SP2:
							case enumWINDOWS_7:
							case enumWINDOWS_8:
								{
									POBJECT_TYPE_2008ABOVE pOt = (POBJECT_TYPE_2008ABOVE)pode->Object;
									ULONG nLen = 0;

									KdPrint(("Name: %wZ, Object: 0x%08X\n", &pOt->Name, pode->Object));
									nChars = pOt->Name.Length / sizeof(WCHAR);
									szTypeName = pOt->Name.Buffer;

									if (nTypes < nCnt)
									{
										pTypes[nTypes].ObjectType = (ULONG)pode->Object;
										nLen = CmpAndGetStringLength(&pOt->Name, MAX_TYPE_NAME);
										wcsncpy(pTypes[nTypes++].szTypeName, szTypeName, nLen);
									}
								}
								break;
							}

							if (nChars == wcslen(szDirectory) &&
								!_wcsnicmp(szDirectory, szTypeName, nChars))
							{
								SetGlobalVeriable(enumDirectoryType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szMutant) &&
								!_wcsnicmp(szMutant, szTypeName, nChars))
							{
								SetGlobalVeriable(enumMutantType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szThread) &&
								!_wcsnicmp(szThread, szTypeName, nChars))
							{
								SetGlobalVeriable(enumThreadType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szFilterCommunicationPort) &&
								!_wcsnicmp(szFilterCommunicationPort, szTypeName, nChars))
							{
								SetGlobalVeriable(enumFilterCommunicationPortType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szController) &&
								!_wcsnicmp(szController, szTypeName, nChars))
							{
								SetGlobalVeriable(enumControllerType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szProfile) &&
								!_wcsnicmp(szProfile, szTypeName, nChars))
							{
								SetGlobalVeriable(enumProfileType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szEvent) &&
								!_wcsnicmp(szEvent, szTypeName, nChars))
							{
								SetGlobalVeriable(enumEventType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szType) &&
								!_wcsnicmp(szType, szTypeName, nChars))
							{
								SetGlobalVeriable(enumTypeType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szSection) &&
								!_wcsnicmp(szSection, szTypeName, nChars))
							{
								SetGlobalVeriable(enumSectionType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szEventPair) &&
								!_wcsnicmp(szEventPair, szTypeName, nChars))
							{
								SetGlobalVeriable(enumEventPairType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szSymbolicLink) &&
								!_wcsnicmp(szSymbolicLink, szTypeName, nChars))
							{
								SetGlobalVeriable(enumSymbolicLinkType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szDesktop) &&
								!_wcsnicmp(szDesktop, szTypeName, nChars))
							{
								SetGlobalVeriable(enumDesktopType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szTimer) &&
								!_wcsnicmp(szTimer, szTypeName, nChars))
							{
								SetGlobalVeriable(enumTimerType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szFile) &&
								!_wcsnicmp(szFile, szTypeName, nChars))
							{
								SetGlobalVeriable(enumFileType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szWindowStation) &&
								!_wcsnicmp(szWindowStation, szTypeName, nChars))
							{
								SetGlobalVeriable(enumWindowStationType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szDriver) &&
								!_wcsnicmp(szDriver, szTypeName, nChars))
							{
								SetGlobalVeriable(enumDriverType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szWmiGuid) &&
								!_wcsnicmp(szWmiGuid, szTypeName, nChars))
							{
								SetGlobalVeriable(enumWmiGuidType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szKeyedEvent) &&
								!_wcsnicmp(szKeyedEvent, szTypeName, nChars))
							{
								SetGlobalVeriable(enumKeyedEventType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szDevice) &&
								!_wcsnicmp(szDevice, szTypeName, nChars))
							{
								SetGlobalVeriable(enumDeviceType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szToken) &&
								!_wcsnicmp(szToken, szTypeName, nChars))
							{
								SetGlobalVeriable(enumTokenType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szDebugObject) &&
								!_wcsnicmp(szDebugObject, szTypeName, nChars))
							{
								SetGlobalVeriable(enumDebugObjectType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szIoCompletion) &&
								!_wcsnicmp(szIoCompletion, szTypeName, nChars))
							{
								SetGlobalVeriable(enumIoCompletionType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szProcess) &&
								!_wcsnicmp(szProcess, szTypeName, nChars))
							{
								SetGlobalVeriable(enumProcessType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szAdapter) &&
								!_wcsnicmp(szAdapter, szTypeName, nChars))
							{
								SetGlobalVeriable(enumAdapterType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szKey) &&
								!_wcsnicmp(szKey, szTypeName, nChars))
							{
								SetGlobalVeriable(enumKeyType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szJob) &&
								!_wcsnicmp(szJob, szTypeName, nChars))
							{
								SetGlobalVeriable(enumJobType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szWaitablePort) &&
								!_wcsnicmp(szWaitablePort, szTypeName, nChars))
							{
								SetGlobalVeriable(enumWaitablePortType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szPort) &&
								!_wcsnicmp(szPort, szTypeName, nChars))
							{
								SetGlobalVeriable(enumPortType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szAlpcPort) &&
								!_wcsnicmp(szAlpcPort, szTypeName, nChars))
							{
								KdPrint(("PortType: 0x%08X\n", pode->Object));
								SetGlobalVeriable(enumPortType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szCallback) &&
								!_wcsnicmp(szCallback, szTypeName, nChars))
							{
								SetGlobalVeriable(enumCallbackType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szFilterConnectionPort) &&
								!_wcsnicmp(szFilterConnectionPort, szTypeName, nChars))
							{
								SetGlobalVeriable(enumFilterConnectionPortType, (ULONG)pode->Object);
							}
							else if (nChars == wcslen(szSemaphore) &&
								!_wcsnicmp(szSemaphore, szTypeName, nChars))
							{
								SetGlobalVeriable(enumSemaphoreType, (ULONG)pode->Object);
							}
						}
					}
				}
			}
			__except(1)
			{
				KdPrint(("GetAllObjectTypes catch __except\n"));
			}
		}

		ntStatus = MzfNtClose(handle);
	}
	
	RecoverPreMode(pThread, PreMode);

	return ntStatus;
}

PIMAGE_SECTION_HEADER GetInitSecion(PIMAGE_NT_HEADERS NtHeaders)
{
	ULONG i = 0;
	PIMAGE_SECTION_HEADER NtSection = NULL;
	CHAR szInit[] = {'I','N','I','T','\0'};

	if (!NtHeaders)
	{
		return NULL;
	}

	NtSection = IMAGE_FIRST_SECTION( NtHeaders );
	for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) 
	{
		if (NtSection->Characteristics & IMAGE_SCN_CNT_CODE &&
			!_strnicmp(NtSection->Name, szInit, strlen(szInit))) 
		{
			return NtSection;
		}

		++NtSection;
	}

	KdPrint(("NtSection: 0x%08X\n", NtSection));

	return NULL;
}

BOOL IsCreateObjectTypeCall(ULONG pRelocationAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG pObCreateObjectType = GetGlobalVeriable(enumObCreateObjectType);
	ULONG pObCreateObjectTypeEx = GetGlobalVeriable(enumObCreateObjectTypeEx);
	BOOL bFind = FALSE;
	
//	KdPrint(("IsCreateObjectTypeCall\n"));
	if (MzfMmIsAddressValid &&
		pRelocationAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pRelocationAddress) )
	{
		ULONG Address = 0, RelocationAddress = 0, nCodeLen = 0;

		RelocationAddress = pRelocationAddress - 1;
		
		for (Address = RelocationAddress; 
			Address < RelocationAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_CALL && Inst.opcode == 0xE8)
			{
				ULONG CallAddress = Inst.op1.immediate + Address + 5;

				if (CallAddress == pObCreateObjectType ||
					CallAddress == pObCreateObjectTypeEx)
				{
					bFind = TRUE;
					break;
				}
			}


			if (Inst.type == INSTRUCTION_TYPE_RET ||
				Inst.length == 0)
			{
				break;
			}

			nCodeLen = Inst.length;
		}
	}

	return bFind;
}

void GetTypeTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("TypeType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		ULONG Start = 0, End = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (Version == enumWINDOWS_2K3_SP1_SP2 ||
			Version == enumWINDOWS_VISTA)
		{
			Start = pAddress + 4;
			End = pAddress + 100;
		}
		else
		{
			Start = pAddress - 0x20;
			End = pAddress;
		}

		for (Address = Start; 
			Address < End && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				KdPrint(("ObpDeleteObjectType: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumObpDeleteObjectType, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetSymbolicLinkTypeOriginDispatch( ULONG pAddress )
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("SymbolicLinkType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					KdPrint(("ObpDeleteSymbolicLink: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumObpDeleteSymbolicLink, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("ObpParseSymbolicLink: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumObpParseSymbolicLink, Inst.op2.immediate);
					break;
				}
				
				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetDeviceTypeOriginDispatch( ULONG pAddress )
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

	KdPrint(("DeviceType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;
			
			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);
			
			if (BuildNumber >= 6000)
			{
				if (Inst.type == INSTRUCTION_TYPE_MOV &&
					Inst.op1.type == OPERAND_TYPE_REGISTER && 
					Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
					Inst.op2.immediate > SYSTEM_ADDRESS_START &&
					MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
				{
					KdPrint(("IopGetSetSecurityObject: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopGetSetSecurityObject, Inst.op2.immediate);
				}
			}

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					KdPrint(("IopParseDevice: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopParseDevice, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("IopDeleteDevice: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopDeleteDevice, Inst.op2.immediate);

// 					if (BuildNumber >= 6000 && BuildNumber < 9200)
// 					{
// 						break;
// 					}
				}
				else if (i == 2)
				{
					KdPrint(("IopGetSetSecurityObject: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopGetSetSecurityObject, Inst.op2.immediate);
					break;
				}

				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetDriverTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("DriverType\n"));
	
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		ULONG Start = 0, End = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		KdPrint(("pAddress: 0x%08X\n", pAddress));
		
		if (Version == enumWINDOWS_2K)
		{
			Start = pAddress + 4;
			End = pAddress + 100;
		}
		else
		{
			Start = pAddress - 50;
			End = pAddress;
		}

		for (Address = Start; 
			Address < End && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START// &&
				/*MzfMmIsAddressValid((PVOID)Inst.op2.immediate)*/)
			{
				KdPrint(("IopDeleteDriver: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumIopDeleteDriver, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET || Inst.length <= 0)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetIoCompletionTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	int i = 0;

	KdPrint(("IoCompletionType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				i++;

				if (BuildNumber >= 6000)
				{
					if (i == 1)
					{
						KdPrint(("IopCloseIoCompletion: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumIopCloseIoCompletion, Inst.op2.immediate);
					}
					else if (i == 2)
					{
						KdPrint(("IopDeleteIoCompletion: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumIopDeleteIoCompletion, Inst.op2.immediate);
						break;
					}
				}
				else
				{
					KdPrint(("IopDeleteIoCompletion: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopDeleteIoCompletion, Inst.op2.immediate);
					break;
				}
			}

			if (Inst.type == INSTRUCTION_TYPE_RET || Inst.length <= 0)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetFileTypeOriginDispatch( ULONG pAddress )
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

	KdPrint(("FileType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					KdPrint(("IopCloseFile: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopCloseFile, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("IopDeleteFile: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopDeleteFile, Inst.op2.immediate);
				}
				else if (i == 2)
				{
					KdPrint(("IopParseFile: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopParseFile, Inst.op2.immediate);
				}
				else if (i == 3)
				{
					if (BuildNumber >= 6000 && BuildNumber < 9200)
					{
						KdPrint(("IopQueryName: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumIopQueryName, Inst.op2.immediate);
					//	break;
					}
					else
					{
						KdPrint(("IopGetSetSecurityObject: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumIopFileGetSetSecurityObject, Inst.op2.immediate);
					}
				}
				else if (i == 4)
				{
					KdPrint(("IopQueryName: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumIopQueryName, Inst.op2.immediate);
					break;
				}

				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetPortTypeOriginDispatch( ULONG pAddress )
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	KdPrint(("PortType\n"));
	
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
		
		KdPrint(("BuildNumber: %d\n", BuildNumber));

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					if (BuildNumber >= 6000)
					{
						KdPrint(("enumEtwpOpenRegistrationObject: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumEtwpOpenRegistrationObject, Inst.op2.immediate);
					}
					else
					{
						KdPrint(("LpcpClosePort: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumLpcpClosePort, Inst.op2.immediate);
					}
				}
				else if (i == 1)
				{
					if (BuildNumber >= 6000)
					{
						KdPrint(("enumLpcpClosePort: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumLpcpClosePort, Inst.op2.immediate);
					}
					else
					{
						KdPrint(("LpcpDeletePort: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumLpcpDeletePort, Inst.op2.immediate);
						break;
					}
				}
				else if (i == 2)
				{
					KdPrint(("LpcpDeletePort: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumLpcpDeletePort, Inst.op2.immediate);
					break;
				}

				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetSectionTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("SectionType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress - 0x30; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				KdPrint(("MiSectionDelete: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumMiSectionDelete, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetProcessTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	int i = 0;
	
	KdPrint(("ProcessType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress - 100; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				i++;

				if (BuildNumber >= 6000)
				{
					if (i == 1)
					{
						KdPrint(("PspProcessDelete: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumPspProcessDelete, Inst.op2.immediate);
					}
					else if (i == 2)
					{
						KdPrint(("PspProcessDelete: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumPspProcessOpen, Inst.op2.immediate);
					}
					else if (i == 3)
					{
						KdPrint(("PspProcessDelete: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumPspProcessClose, Inst.op2.immediate);
						break;
					}
				}
				else
				{
					if (i == 1)
					{
						KdPrint(("PspProcessDelete: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumPspProcessDelete, Inst.op2.immediate);
						break;
					}
				}
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetThreadTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	int i = 0 ;

	KdPrint(("ThreadType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress - 100; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				i++;

				if (BuildNumber >= 6000)
				{
					if (i == 1)
					{
						KdPrint(("PspThreadDelete: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumPspThreadDelete, Inst.op2.immediate);
					}
					else if (i == 2)
					{
						KdPrint(("PspThreadOpen: 0x%08X\n", Inst.op2.immediate));
						SetGlobalVeriable(enumPspThreadOpen, Inst.op2.immediate);
						break;
					}
				}
				else
				{
					KdPrint(("PspThreadDelete: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumPspThreadDelete, Inst.op2.immediate);
					break;
				}
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetJobTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("JobType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;

		for (Address = pAddress - 0x30; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					KdPrint(("PspJobDelete: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumPspJobDelete, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("PspJobClose: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumPspJobClose, Inst.op2.immediate);
					break;
				}
				
				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetTokenTypeOriginDispatch( ULONG pAddress )
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("TokenType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				KdPrint(("SepTokenDeleteMethod: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumSepTokenDeleteMethod, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetWmiGuidTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("WmiGuidType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

		for (Address = pAddress - 0x30; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START/* &&*/
				/*MzfMmIsAddressValid((PVOID)Inst.op2.immediate)*/)
			{
				if (i == 0)
				{
					KdPrint(("WmipSecurityMethod: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumWmipSecurityMethod, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("WmipDeleteMethod: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumWmipDeleteMethod, Inst.op2.immediate);

					if (BuildNumber == 9200)
					{
						break;
					}
				}
				else if (i == 2)
				{
					KdPrint(("WmipCloseMethod: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumWmipCloseMethod, Inst.op2.immediate);
					break;
				}

				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetWindowStationTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("WindowStationType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					KdPrint(("ExpWin32CloseProcedure: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumExpWin32CloseProcedure, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("ExpWin32DeleteProcedure: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumExpWin32DeleteProcedure, Inst.op2.immediate);
				}
				else if (i == 2)
				{
					KdPrint(("ExpWin32OkayToCloseProcedure: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumExpWin32OkayToCloseProcedure, Inst.op2.immediate);
				}
				else if (i == 3)
				{
					KdPrint(("ExpWin32ParseProcedure: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumExpWin32ParseProcedure, Inst.op2.immediate);
				}
				else if (i == 4)
				{
					KdPrint(("ExpWin32OpenProcedure: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumExpWin32OpenProcedure, Inst.op2.immediate);
					break;
				}

				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetTimerTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("TimerType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				KdPrint(("TimerDeleteProcedure: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumTimerDeleteProcedure, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetMutantTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("MutantType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				KdPrint(("ExpDeleteMutant: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumExpDeleteMutant, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetProfileTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("ProfileType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress - 100; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START// &&
				/*MzfMmIsAddressValid((PVOID)Inst.op2.immediate)*/)
			{
				KdPrint(("ExpProfileDelete: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumExpProfileDelete, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetKeyTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("KeyType\n"));
	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				if (i == 0)
				{
					KdPrint(("CmpCloseKeyObjec: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumCmpCloseKeyObject, Inst.op2.immediate);
				}
				else if (i == 1)
				{
					KdPrint(("CmpDeleteKeyObject: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumCmpDeleteKeyObject, Inst.op2.immediate);
				}
				else if (i == 2)
				{
					KdPrint(("CmpParseKey: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumCmpParseKey, Inst.op2.immediate);
				}
				else if (i == 3)
				{
					KdPrint(("CmpSecurityMethod: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumCmpSecurityMethod, Inst.op2.immediate);
				}
				else if (i == 4)
				{
					KdPrint(("CmpQueryKeyName: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumCmpQueryKeyName, Inst.op2.immediate);
					break;
				}

				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetDebugObjectTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("DebugObjectType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		int i  = 0;

		for (Address = pAddress - 0x30; 
			Address < pAddress && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START// &&
				/*MzfMmIsAddressValid((PVOID)Inst.op2.immediate)*/)
			{
				if ( i == 0 )
				{
					KdPrint(("xHalLocateHiberRanges: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumxHalLocateHiberRanges, Inst.op2.immediate);
				}
				else if ( i == 1 )
				{
					KdPrint(("DbgkpCloseObject: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumDbgkpCloseObject, Inst.op2.immediate);
					break;
				}
				
				i++;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetCallbackObjectTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("CallbackObjectType\n"));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;

		for (Address = pAddress + 4; 
			Address < pAddress + 100 && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				KdPrint(("xHalLocateHiberRanges: 0x%08X\n", Inst.op2.immediate));
				SetGlobalVeriable(enumxHalLocateHiberRanges, Inst.op2.immediate);
				break;
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void GetDirectoryObjectTypeOriginDispatch(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	KdPrint(("DirectoryObjectType: pAddress: 0x%08X\n", pAddress));

	if (MzfMmIsAddressValid &&
		pAddress > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG Address = 0, nCodeLen = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
		ULONG Start = 0, End = 0;
		int i  = 0;

		if (Version == enumWINDOWS_VISTA || 
			Version == enumWINDOWS_7 ||
			Version == enumWINDOWS_8)
		{
			Start = pAddress + 4;
			End = pAddress + 100;
		}
		else if (Version == enumWINDOWS_VISTA_SP1_SP2)
		{
			Start = pAddress - 20;
			End = pAddress;
		}

		for (Address = Start; 
			Address < End && MzfMmIsAddressValid((PVOID)Address); 
			Address += nCodeLen)
		{
			INSTRUCTION	Inst;

			memset(&Inst, 0, sizeof(INSTRUCTION));
			get_instruction(&Inst, (PBYTE)Address, MODE_32);

			if (Inst.type == INSTRUCTION_TYPE_MOV &&
				Inst.op1.type == OPERAND_TYPE_MEMORY && 
				Inst.op2.type == OPERAND_TYPE_IMMEDIATE &&
				Inst.op2.immediate > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
			{
				i++;

				if (i == 1)
				{
					KdPrint(("ObpCloseDirectoryObject: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumObpCloseDirectoryObject, Inst.op2.immediate);

					if (Version != enumWINDOWS_8)
					{
						break;
					}
				}
				else if (i == 2)
				{
					KdPrint(("ObpDeleteDirectoryObject: 0x%08X\n", Inst.op2.immediate));
					SetGlobalVeriable(enumObpDeleteDirectoryObject, Inst.op2.immediate);
					break;
				}
			}

			if (Inst.type == INSTRUCTION_TYPE_RET)
			{
				break;
			}

			nCodeLen = Inst.length > 0 ? Inst.length : 1;
		}
	}
}

void FindTypeDispatch(ULONG pTypeObject, ULONG pRelocationAddress)
{
	if(!IsCreateObjectTypeCall(pRelocationAddress))
	{
		return;
	}

	if (pTypeObject == GetGlobalVeriable(enumDirectoryType))
	{
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

		if (BuildNumber >= 6000)
		{
			GetDirectoryObjectTypeOriginDispatch(pRelocationAddress);
		}
	}
	else if (pTypeObject == GetGlobalVeriable(enumMutantType))
	{
		GetMutantTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumThreadType))
	{
		GetThreadTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumFilterCommunicationPortType))
	{
	}
	else if (pTypeObject == GetGlobalVeriable(enumControllerType))
	{
		// None.
	}
	else if (pTypeObject == GetGlobalVeriable(enumProfileType))
	{
		GetProfileTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumEventType))
	{
		// None.
	}
	else if (pTypeObject == GetGlobalVeriable(enumTypeType))
	{
		GetTypeTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumSectionType))
	{
		GetSectionTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumEventPairType))
	{
		// None.
	}
	else if (pTypeObject == GetGlobalVeriable(enumSymbolicLinkType))
	{
		GetSymbolicLinkTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumDesktopType))
	{
		// Í¬WindowStationType
	}
	else if (pTypeObject == GetGlobalVeriable(enumTimerType))
	{
		GetTimerTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumFileType))
	{
		GetFileTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumWindowStationType))
	{
		GetWindowStationTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumDriverType))
	{
		GetDriverTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumWmiGuidType))
	{
		GetWmiGuidTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumKeyedEventType))
	{
		// None.
	}
	else if (pTypeObject == GetGlobalVeriable(enumDeviceType))
	{
		GetDeviceTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumTokenType))
	{
		GetTokenTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumDebugObjectType))
	{
		GetDebugObjectTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumIoCompletionType))
	{
		GetIoCompletionTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumProcessType))
	{
		GetProcessTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumAdapterType))
	{
		// None.
	}
	else if (pTypeObject == GetGlobalVeriable(enumKeyType))
	{
		GetKeyTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumJobType))
	{
		GetJobTypeOriginDispatch( pRelocationAddress );
	}
	else if (pTypeObject == GetGlobalVeriable(enumWaitablePortType))
	{
		// º¯ÊýÍ¬Port.
	}
	else if (pTypeObject == GetGlobalVeriable(enumPortType))
	{
		KdPrint(("else if (pTypeObject == GetGlobalVeriable(enumPortType))\n"));
		GetPortTypeOriginDispatch(pRelocationAddress);
	}
	else if (pTypeObject == GetGlobalVeriable(enumCallbackType))
	{
		if (GetGlobalVeriable(enumxHalLocateHiberRanges) == 0)
		{
			GetCallbackObjectTypeOriginDispatch(pRelocationAddress);
		}
	}
	else if (pTypeObject == GetGlobalVeriable(enumFilterConnectionPortType))
	{
	}
	else if (pTypeObject == GetGlobalVeriable(enumSemaphoreType))
	{
		// None.
	}
}

void FindSeDefaultObjectMethod()
{
	ULONG pObGetObjectSecurity = GetGlobalVeriable(enumObGetObjectSecurity);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG SeDefaultObjectMethod = 0;
	
	if (pObGetObjectSecurity && MzfMmIsAddressValid)
	{
		ULONG Address = 0, nCodeLen = 0;
		ULONG CallAddress = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (Version == enumWINDOWS_2K ||
			Version == enumWINDOWS_2K3)
		{
			CallAddress = pObGetObjectSecurity;
		}
		else
		{
			for (Address = pObGetObjectSecurity; 
				Address < pObGetObjectSecurity + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
				Address += nCodeLen)
			{
				INSTRUCTION	Inst;

				memset(&Inst, 0, sizeof(INSTRUCTION));
				get_instruction(&Inst, (PBYTE)Address, MODE_32);

				if (Inst.type == INSTRUCTION_TYPE_CALL && Inst.opcode == 0xE8)
				{
					CallAddress = Inst.op1.immediate + Address + 5;
					break;
				}

				if (Inst.length == 0)
				{
					break;
				}
				else if (Inst.type == INSTRUCTION_TYPE_RET)
				{
					break;
				}

				nCodeLen = Inst.length;
			}
		}

		
		if (CallAddress)
		{
			for (Address = CallAddress; 
				Address < CallAddress + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
				Address += nCodeLen)
			{
				INSTRUCTION	Inst;

				memset(&Inst, 0, sizeof(INSTRUCTION));
				get_instruction(&Inst, (PBYTE)Address, MODE_32);

				if (Inst.type == INSTRUCTION_TYPE_CMP && 
					Inst.op1.type == OPERAND_TYPE_MEMORY && 
					Inst.op2.type == OPERAND_TYPE_IMMEDIATE && 
					MzfMmIsAddressValid((PVOID)Inst.op2.immediate))
				{
					SeDefaultObjectMethod = Inst.op2.immediate;
					break;
				}

				if (Inst.length == 0)
				{
					break;
				}
				else if (Inst.type == INSTRUCTION_TYPE_RET)
				{
					break;
				}

				nCodeLen = Inst.length;
			}
		}
	}

	if (SeDefaultObjectMethod > SYSTEM_ADDRESS_START && 
		MzfMmIsAddressValid((PVOID)SeDefaultObjectMethod))
	{
		KdPrint(("SeDefaultObjectMethod: 0x%08X\n", SeDefaultObjectMethod));
		SetGlobalVeriable(enumSeDefaultObjectMethod, SeDefaultObjectMethod);
	}
}

BOOL InitOriginTypeFunctions()
{
    ULONG TotalCountBytes = 0;
    ULONG_PTR VA;
    ULONG SizeOfBlock;
    ULONG FixupVA;
    USHORT Offset;
    PUSHORT NextOffset = NULL;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	PVOID NewImageBase = (PVOID)GetGlobalVeriable(enumNewKernelBase);
	PIMAGE_SECTION_HEADER InitSection = NULL;
	ULONG pInitSectionAddress = 0, nInitSectionSize = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);

	if (!MzfMmIsAddressValid ||
		!MzfRtlImageDirectoryEntryToData)
	{
		return FALSE;
	}

	KdPrint(("InitOriginTypeFunctions \n"));
	FindSeDefaultObjectMethod();

	__try
	{
		if (!NewImageBase || !MzfMmIsAddressValid)
		{
			return FALSE;
		}

		ImageDosHeader = (PIMAGE_DOS_HEADER)NewImageBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)NewImageBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		NtHeaders = RtlImageNtHeader( NewImageBase );
		if (NtHeaders == NULL) 
		{
			return FALSE;
		}

		InitSection = GetInitSecion(NtHeaders);
		if (!InitSection)
		{
			return FALSE;
		}

		pInitSectionAddress = InitSection->VirtualAddress + (ULONG)NewImageBase;
		nInitSectionSize = InitSection->Misc.VirtualSize;
		KdPrint(("pInitSectionAddress: 0x%08X, Size: 0x%X\n", pInitSectionAddress, nInitSectionSize));

		//
		// Locate the relocation section.
		//

		NextBlock = (PIMAGE_BASE_RELOCATION)MzfRtlImageDirectoryEntryToData(
			NewImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &TotalCountBytes);

		//
		// It is possible for a file to have no relocations, but the relocations
		// must not have been stripped.
		//

		if (!NextBlock || !TotalCountBytes) 
		{
			if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) 
			{
				KdPrint(("Image can't be relocated, no fixup information.\n"));
				return FALSE;
			}
			else 
			{
				return TRUE;
			}
		}

		while (TotalCountBytes)
		{
			SizeOfBlock = NextBlock->SizeOfBlock;
			TotalCountBytes -= SizeOfBlock;
			SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
			SizeOfBlock /= sizeof(USHORT);
			NextOffset = (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));
			
			if (!MzfMmIsAddressValid(NextOffset))
			{
				return FALSE;
			}

			VA = (ULONG_PTR)NewImageBase + NextBlock->VirtualAddress;

			while (SizeOfBlock--) 
			{
				Offset = *NextOffset & (USHORT)0xfff;
				FixupVA = (ULONG)(VA + Offset);

				// INIT:005DE0EC 68 B0 43 5A 00   push offset _CmpKeyObjectType
				if (MzfMmIsAddressValid((PVOID)FixupVA) &&
					MzfMmIsAddressValid((PVOID)(FixupVA - 1)) &&
					*(BYTE *)(FixupVA - 1) == 0x68)
				{
					ULONG pType = *(PULONG)FixupVA;
					if (pType > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)pType))
					{
						ULONG Type = *(PULONG)pType;

						if (Type > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)Type))
						{
							FindTypeDispatch(Type, FixupVA);
						}
					}
				}

				++NextOffset;
				if ( !NextOffset || (ULONG)NextOffset <= SYSTEM_ADDRESS_START || !MzfMmIsAddressValid(NextOffset)) 
				{
					return FALSE;
				}
			}

			NextBlock = (PIMAGE_BASE_RELOCATION)NextOffset;
		}
	}
	__except(1)
	{
		return FALSE;
	}

	SetGlobalVeriable(enumGetObjectTypeDispatchOk, 1);
    return TRUE;
}

ULONG GetObjectProcedure(ULONG Object, ULONG Offset)
{	
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG Procedure = 0, pProcedure = 0;

	if ( MzfMmIsAddressValid && Object && MzfMmIsAddressValid((PVOID)Object))
	{
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

		pProcedure = Object + (BuildNumber < 6001 ? 0x60 : 0x28) + Offset;

		if ( MzfMmIsAddressValid((PVOID)pProcedure) )
		{
			Procedure = *(ULONG *)pProcedure;
		}
	}
	
	return Procedure;
}

ULONG GetObjectTypeDumpProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x2C : 0x30;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectOpenProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x30 : 0x34;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectCloseProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x34 : 0x38;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectDeleteProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x38 : 0x3C;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectParseProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x3C : 0x40;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectSecurityProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x40 : 0x44;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectQueryNameProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x44 : 0x48;

	return GetObjectProcedure(Object, Offset);
}

ULONG GetObjectOkayToCloseProcedure(ULONG Object)
{
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = BuildNumber < 6001 ? 0x48 : 0x4C;

	return GetObjectProcedure(Object, Offset);
}

void CheckObjectTypeInlineHook(POBJECT_HOOK_INOF pObjectHookInfo)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG KernelBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG ReloadKernelBase = GetGlobalVeriable(enumNewKernelBase);

	if (MzfMmIsAddressValid &&
		KernelBase &&
		ReloadKernelBase &&
		pObjectHookInfo)
	{
		ULONG i = 0;

		for ( i = 0; i < 8; i++ )
		{
			ULONG pAddress = 0;
			ULONG NowDispatch = 0, OriginDispatch = 0;

			switch (i)
			{
			case 0:
				NowDispatch = pObjectHookInfo->DumpProcedure;
				OriginDispatch = pObjectHookInfo->OriginDumpProcedure;
				break;

			case 1:
				NowDispatch = pObjectHookInfo->OpenProcedure;
				OriginDispatch = pObjectHookInfo->OriginOpenProcedure;
				break;

			case 2:
				NowDispatch = pObjectHookInfo->CloseProcedure;
				OriginDispatch = pObjectHookInfo->OriginCloseProcedure;
				break;

			case 3:
				NowDispatch = pObjectHookInfo->DeleteProcedure;
				OriginDispatch = pObjectHookInfo->OriginDeleteProcedure;
				break;

			case 4:
				NowDispatch = pObjectHookInfo->ParseProcedure;
				OriginDispatch = pObjectHookInfo->OriginParseProcedure;
				break;

			case 5:
				NowDispatch = pObjectHookInfo->SecurityProcedure;
				OriginDispatch = pObjectHookInfo->OriginSecurityProcedure;
				break;

			case 6:
				NowDispatch = pObjectHookInfo->QueryNameProcedure;
				OriginDispatch = pObjectHookInfo->OriginQueryNameProcedure;
				break;

			case 7:
				NowDispatch = pObjectHookInfo->OkayToCloseProcedure;
				OriginDispatch = pObjectHookInfo->OriginOkayToCloseProcedure;
				break;
			}

			if (NowDispatch &&
				MzfMmIsAddressValid((PVOID)NowDispatch) && 
				OriginDispatch &&
				MzfMmIsAddressValid((PVOID)OriginDispatch) &&
				MzfMmIsAddressValid((PVOID)(OriginDispatch - KernelBase + ReloadKernelBase)))
			{
				if (memcmp((PBYTE)NowDispatch, (PBYTE)(OriginDispatch - KernelBase + ReloadKernelBase), 0x10))
				{
					pAddress = GetInlineHookAddress(NowDispatch, 0x10);

					if (pAddress == 0 || 
						pAddress < SYSTEM_ADDRESS_START || 
						!MzfMmIsAddressValid((PVOID)pAddress))
					{
						pAddress = 1;
					}
				}
			}

			switch (i)
			{
			case 0:
				pObjectHookInfo->InlineDumpProcedure = pAddress;
				break;

			case 1:
				pObjectHookInfo->InlineOpenProcedure = pAddress;
				break;

			case 2:
				pObjectHookInfo->InlineCloseProcedure = pAddress;
				break;

			case 3:
				pObjectHookInfo->InlineDeleteProcedure = pAddress;
				break;

			case 4:
				pObjectHookInfo->InlineParseProcedure = pAddress;
				break;

			case 5:
				pObjectHookInfo->InlineSecurityProcedure = pAddress;
				break;

			case 6:
				pObjectHookInfo->InlineQueryNameProcedure = pAddress;
				break;

			case 7:
				pObjectHookInfo->InlineOkayToCloseProcedure = pAddress;
				break;
			}
		}
	}
}

NTSTATUS EnumObjectHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!GetGlobalVeriable(enumGetObjectTypeDispatchOk))
	{
		InitOriginTypeFunctions();
	}
	
	if (GetGlobalVeriable(enumGetObjectTypeDispatchOk))
	{
		POBJECT_HOOK_INOF pObjectHookInfo = (POBJECT_HOOK_INOF)pOutBuffer;
		ULONG i = 0;
		PULONG pTypes = (PULONG)GetGlobalVeriableAddress(enumDirectoryType);
		ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);

		status = STATUS_SUCCESS;

		for (i = 0; i < eMaxType; i++)
		{
			pObjectHookInfo[i].Type = i;
			pObjectHookInfo[i].TypeObject = pTypes[i];
			pObjectHookInfo[i].DumpProcedure = GetObjectTypeDumpProcedure(pTypes[i]);
			pObjectHookInfo[i].OpenProcedure = GetObjectOpenProcedure(pTypes[i]);
			pObjectHookInfo[i].CloseProcedure = GetObjectCloseProcedure(pTypes[i]);
			pObjectHookInfo[i].DeleteProcedure = GetObjectDeleteProcedure(pTypes[i]);
			pObjectHookInfo[i].ParseProcedure = GetObjectParseProcedure(pTypes[i]);
			pObjectHookInfo[i].SecurityProcedure = GetObjectSecurityProcedure(pTypes[i]);
			pObjectHookInfo[i].QueryNameProcedure = GetObjectQueryNameProcedure(pTypes[i]);
			pObjectHookInfo[i].OkayToCloseProcedure = GetObjectOkayToCloseProcedure(pTypes[i]);

			pObjectHookInfo[i].OriginSecurityProcedure = GetGlobalVeriable(enumSeDefaultObjectMethod);

			switch (i)
			{
			case eDirectoryType:
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumObpCloseDirectoryObject);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumObpDeleteDirectoryObject);
				break;

			case eMutantType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumExpDeleteMutant);
				break;

			case eThreadType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumPspThreadDelete);
				pObjectHookInfo[i].OriginOpenProcedure = GetGlobalVeriable(enumPspThreadOpen);
				break;

			case eFilterCommunicationPortType:
				break;

			case eControllerType:
				break;

			case eProfileType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumExpProfileDelete);
				break;

			case eEventType:
				break;

			case eTypeType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumObpDeleteObjectType);
				break;

			case eSectionType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumMiSectionDelete);
				break;

			case eEventPairType:
				break;

			case eSymbolicLinkType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumObpDeleteSymbolicLink);
				pObjectHookInfo[i].OriginParseProcedure = GetGlobalVeriable(enumObpParseSymbolicLink);
				break;

			case eDesktopType:
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumExpWin32CloseProcedure);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumExpWin32DeleteProcedure);
				pObjectHookInfo[i].OriginOkayToCloseProcedure = GetGlobalVeriable(enumExpWin32OkayToCloseProcedure);
				pObjectHookInfo[i].OriginOpenProcedure = GetGlobalVeriable(enumExpWin32OpenProcedure);
				break;

			case eTimerType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumTimerDeleteProcedure);
				break;

			case eFileType:
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumIopCloseFile);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumIopDeleteFile);
				pObjectHookInfo[i].OriginParseProcedure = GetGlobalVeriable(enumIopParseFile);

				if (BuildNumber >= 6000 && BuildNumber < 9200)
				{
					pObjectHookInfo[i].OriginSecurityProcedure = GetGlobalVeriable(enumIopGetSetSecurityObject);
				}
				else
				{
					pObjectHookInfo[i].OriginSecurityProcedure = GetGlobalVeriable(enumIopFileGetSetSecurityObject);
				}
				
				pObjectHookInfo[i].OriginQueryNameProcedure = GetGlobalVeriable(enumIopQueryName);
				break;

			case eWindowStationType:
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumExpWin32CloseProcedure);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumExpWin32DeleteProcedure);
				pObjectHookInfo[i].OriginParseProcedure = GetGlobalVeriable(enumExpWin32ParseProcedure);
				pObjectHookInfo[i].OriginOkayToCloseProcedure = GetGlobalVeriable(enumExpWin32OkayToCloseProcedure);
				pObjectHookInfo[i].OriginOpenProcedure = GetGlobalVeriable(enumExpWin32OpenProcedure);
				break;

			case eDriverType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumIopDeleteDriver);
				break;

			case eWmiGuidType:
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumWmipCloseMethod);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumWmipDeleteMethod);
				pObjectHookInfo[i].OriginSecurityProcedure = GetGlobalVeriable(enumWmipSecurityMethod);
				break;

			case eKeyedEventType:
				break;

			case eDeviceType:
				pObjectHookInfo[i].OriginParseProcedure = GetGlobalVeriable(enumIopParseDevice);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumIopDeleteDevice);
				pObjectHookInfo[i].OriginSecurityProcedure = GetGlobalVeriable(enumIopGetSetSecurityObject);
				break;

			case eTokenType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumSepTokenDeleteMethod);
				break;

			case eDebugObjectType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumxHalLocateHiberRanges);
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumDbgkpCloseObject);
				break;

			case eIoCompletionType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumIopDeleteIoCompletion);
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumIopCloseIoCompletion);
				break;

			case eProcessType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumPspProcessDelete);
				pObjectHookInfo[i].OriginOpenProcedure = GetGlobalVeriable(enumPspProcessOpen);
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumPspProcessClose);
				break;

			case eAdapterType:
				break;

			case eKeyType:
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumCmpCloseKeyObject);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumCmpDeleteKeyObject);
				pObjectHookInfo[i].OriginParseProcedure = GetGlobalVeriable(enumCmpParseKey);
				pObjectHookInfo[i].OriginSecurityProcedure = GetGlobalVeriable(enumCmpSecurityMethod);
				pObjectHookInfo[i].OriginQueryNameProcedure = GetGlobalVeriable(enumCmpQueryKeyName);
				break;

			case eJobType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumPspJobDelete);
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumPspJobClose);
				break;

			case eWaitablePortType:
				pObjectHookInfo[i].OriginOpenProcedure = GetGlobalVeriable(enumEtwpOpenRegistrationObject);
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumLpcpClosePort);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumLpcpDeletePort);
				break;

			case ePortType:
				pObjectHookInfo[i].OriginOpenProcedure = GetGlobalVeriable(enumEtwpOpenRegistrationObject);
				pObjectHookInfo[i].OriginCloseProcedure = GetGlobalVeriable(enumLpcpClosePort);
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumLpcpDeletePort);
				break;

			case eCallbackType:
				pObjectHookInfo[i].OriginDeleteProcedure = GetGlobalVeriable(enumxHalLocateHiberRanges);
				break;

			case eFilterConnectionPortType:
				break;

			case eSemaphoreType:
				break;
			}
		}

		CheckObjectTypeInlineHook(pObjectHookInfo);
	}
	
	return status;
}

void GetObCreateObjectTypeEx()
{
	ULONG ObCreateObjectType = GetGlobalVeriable(enumObCreateObjectType);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG Address = 0, nCodeLen = 0;

	if (!MzfMmIsAddressValid ||
		!ObCreateObjectType ||
		!MzfMmIsAddressValid((PVOID)ObCreateObjectType))
	{
		return;
	}

	for (Address = ObCreateObjectType; 
		Address < ObCreateObjectType + PAGE_SIZE && MzfMmIsAddressValid((PVOID)Address); 
		Address += nCodeLen)
	{
		INSTRUCTION	Inst;

		memset(&Inst, 0, sizeof(INSTRUCTION));
		get_instruction(&Inst, (PBYTE)Address, MODE_32);

		if (Inst.type == INSTRUCTION_TYPE_CALL)
		{
			ULONG CallAddress = Inst.op1.immediate + Address + Inst.length;

			if (CallAddress > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)CallAddress))
			{
				SetGlobalVeriable(enumObCreateObjectTypeEx, CallAddress);
				KdPrint(("ObCreateObjectTypeEx: 0x%08X\n", CallAddress));
				break;
			}
		}

		if (Inst.type == INSTRUCTION_TYPE_RET ||
			Inst.length == 0)
		{
			break;
		}

		nCodeLen = Inst.length;
	}
}

NTSTATUS EnumAllObjectTypes(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	ULONG nCnt = uOutSize / sizeof(OBJECT_TYPE_INFO);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	
	if (BuildNumber >= 6000)
	{
		ULONG ObCreateObjectTypeEx = GetGlobalVeriable(enumObCreateObjectTypeEx);
		if (!ObCreateObjectTypeEx)
		{
			GetObCreateObjectTypeEx();
		}
	}

	KdPrint(("enter EnumAllObjectTypes\n"));

	return GetAllObjectTypes((POBJECT_TYPE_INFO)pOutBuffer, nCnt);
}

NTSTATUS GetSelfCreateObjectHookInfo(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	POBJECT_HOOK_INOF pHookInfo = (POBJECT_HOOK_INOF)pOutBuffer;
	PCOMMUNICATE_OBJECT_HOOK pCoh = (PCOMMUNICATE_OBJECT_HOOK)pInBuffer;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG Type = pCoh->op.GetSlefCreateType.pType;

	if (!MzfMmIsAddressValid ||
		Type <= SYSTEM_ADDRESS_START ||
		!MzfMmIsAddressValid((PVOID)Type))
	{
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("GetSelfCreateObjectHookInfo-> Type: 0x%08X\n", Type));

	pHookInfo->Type = eUnknowType;
	pHookInfo->TypeObject = Type;
	pHookInfo->DumpProcedure = GetObjectTypeDumpProcedure(Type);
	pHookInfo->OpenProcedure = GetObjectOpenProcedure(Type);
	pHookInfo->CloseProcedure = GetObjectCloseProcedure(Type);
	pHookInfo->DeleteProcedure = GetObjectDeleteProcedure(Type);
	pHookInfo->ParseProcedure = GetObjectParseProcedure(Type);
	pHookInfo->SecurityProcedure = GetObjectSecurityProcedure(Type);
	pHookInfo->QueryNameProcedure = GetObjectQueryNameProcedure(Type);
	pHookInfo->OkayToCloseProcedure = GetObjectOkayToCloseProcedure(Type);

	return STATUS_SUCCESS;
}

NTSTATUS RestoreObjectHook(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_OBJECT_HOOK pCoh = (PCOMMUNICATE_OBJECT_HOOK)pInBuffer;
	ULONG ObjectType = pCoh->op.Restore.ObjectType;
	ULONG OriginProcedure = pCoh->op.Restore.OriginProcedure;
	ENUM_OBJECT_PROCEDURE ObjectProcedureType = pCoh->op.Restore.ObjectProcedureType;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG BuildNumber = GetGlobalVeriable(enumBuildNumber);
	ULONG Offset = 0;

	if (!MzfMmIsAddressValid ||
		!ObjectType ||
		!MzfMmIsAddressValid((PVOID)ObjectType) ||
		ObjectProcedureType == enumProcedureUnknow)
	{	
		return status;
	}

	switch (ObjectProcedureType)
	{
	case enumDumpProcedure:
		Offset = BuildNumber < 6001 ? 0x2C : 0x30;
		break;

	case enumOpenProcedure:
		Offset = BuildNumber < 6001 ? 0x30 : 0x34;
		break;

	case enumCloseProcedure:
		Offset = BuildNumber < 6001 ? 0x34 : 0x38;
		break;

	case enumDeleteProcedure:
		Offset = BuildNumber < 6001 ? 0x38 : 0x3C;
		break;

	case enumParseProcedure:
		Offset = BuildNumber < 6001 ? 0x3C : 0x40;
		break;

	case enumSecurityProcedure:
		Offset = BuildNumber < 6001 ? 0x40 : 0x44;
		break;

	case enumQueryNameProcedure:
		Offset = BuildNumber < 6001 ? 0x44 : 0x48;
		break;

	case enumOkayToCloseProcedure:
		Offset = BuildNumber < 6001 ? 0x48 : 0x4C;
		break;
	}
	
	if (MzfMmIsAddressValid((PVOID)(ObjectType + (BuildNumber < 6001 ? 0x60 : 0x28) + Offset)))
	{
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		WPOFF();
		*(PULONG)(ObjectType + (BuildNumber < 6001 ? 0x60 : 0x28) + Offset) = OriginProcedure;
		WPON();
		
		KeLowerIrql(OldIrql);

		status = STATUS_SUCCESS;
	}
	
	return status;
}