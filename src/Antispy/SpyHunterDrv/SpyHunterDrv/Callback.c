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
#include "Callback.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "ldasm.h"

/////////////////////////////////////////////////////

VOID
create_process_notify (
					   IN HANDLE  ParentId,
					   IN HANDLE  ProcessId,
					   IN BOOLEAN  Create
					   )
{
	KdPrint(("fuck\n"));
}

/////////////////////////////////////////////////////

VOID
load_image_notify (
				   IN PUNICODE_STRING  FullImageName,
				   IN HANDLE  ProcessId,
				   IN PIMAGE_INFO  ImageInfo
				   )
{
	KdPrint(("fuck\n"));
}

/////////////////////////////////////////////////////

VOID
create_thread_notify (
					  IN HANDLE  ProcessId,
					  IN HANDLE  ThreadId,
					  IN BOOLEAN  Create
					  )
{
	KdPrint(("fuck\n"));
}

/////////////////////////////////////////////////////

ULONG find_PspSetXxxNotifyRoutine_by_valid_address(
											ULONG pValidAddress, 
											PVOID pFindAddress
											)
{
	ULONG i = 0, pNotifyItem = 0;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!pValidAddress ||
		!pFindAddress ||
		!MzfMmIsAddressValid)
	{
		return 0;
	}

	if ( pValidAddress > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)pValidAddress) )
	{
		for ( i = 0; i < 64; i++ )
		{
			if ( !MzfMmIsAddressValid( (PVOID)(pValidAddress + i * sizeof(ULONG))) )
			{
				break;
			}

			pNotifyItem = *(PULONG)(pValidAddress + i * sizeof(ULONG));

			if (Version == enumWINDOWS_2K)
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem))) )
				{
					break;
				}
			}
			else 
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG)))) )
				{
					break;
				}
			}

			if (Version == enumWINDOWS_2K)
			{
				if ( pNotifyItem == (ULONG)pFindAddress )
				{
					return pValidAddress;
				}
			}
			else 
			{
				if ( *(PULONG)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG)) == (ULONG)pFindAddress )
				{
					return pValidAddress;
				}
			}
		}
	}

	return 0;
}	

/////////////////////////////////////////////////////

ULONG find_PspSetxxxNotifyRoutine_address(
									PVOID pAddress, 
									PVOID pFindAddress
									)
{
	ULONG i = 0, pNotifyItem = 0;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	if (!pAddress ||
		!pFindAddress ||
		!MzfMmIsAddressValid)
	{
		return 0;
	}

	for ( i = 0; i < 0x100; i++ )
	{
		ULONG pValidAddress = 0, uAddress = 0;

		if (!MzfMmIsAddressValid((PBYTE)pAddress + i))
		{
			break;
		}

		pValidAddress = *(PULONG)((PBYTE)pAddress + i);

		if ( pValidAddress > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)pValidAddress) )
		{
			uAddress = find_PspSetXxxNotifyRoutine_by_valid_address(pValidAddress, pFindAddress);

			if (uAddress)
			{
				return uAddress;
			}
		}
	}

	return 0;
}

/////////////////////////////////////////////////////

void GetPspLoadImageNotifyRoutine()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG PspLoadImageNotifyRoutine = 0;
	pfnPsSetLoadImageNotifyRoutine MzfPsSetLoadImageNotifyRoutine = (pfnPsSetLoadImageNotifyRoutine)GetGlobalVeriable(enumPsSetLoadImageNotifyRoutine);
	pfnPsRemoveLoadImageNotifyRoutine MzfPsRemoveLoadImageNotifyRoutine = (pfnPsRemoveLoadImageNotifyRoutine)GetGlobalVeriable(enumPsRemoveLoadImageNotifyRoutine);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (MzfPsSetLoadImageNotifyRoutine && MzfPsRemoveLoadImageNotifyRoutine)
	{
		status = MzfPsSetLoadImageNotifyRoutine(load_image_notify);

		if (NT_SUCCESS(status))
		{
			PspLoadImageNotifyRoutine = find_PspSetxxxNotifyRoutine_address(
				MzfPsSetLoadImageNotifyRoutine, 
				load_image_notify
				);

			if (PspLoadImageNotifyRoutine && MzfMmIsAddressValid((PVOID)(PspLoadImageNotifyRoutine)))
			{
				KdPrint(("方法一：PspLoadImageNotifyRoutine: 0x%08X\n", PspLoadImageNotifyRoutine));
				SetGlobalVeriable(enumPspLoadImageNotifyRoutine, PspLoadImageNotifyRoutine);
			}

			MzfPsRemoveLoadImageNotifyRoutine(load_image_notify);
		}
	}

	if (MzfPsSetLoadImageNotifyRoutine &&
		!PspLoadImageNotifyRoutine)
	{
		ULONG pStart = (ULONG)MzfPsSetLoadImageNotifyRoutine;
		ULONG pEnd = (ULONG)MzfPsSetLoadImageNotifyRoutine + PAGE_SIZE;
		ULONG i = 0, nCodeLen = 0;
		BYTE btSign = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (Version == enumWINDOWS_2K)
		{
			btSign = 0xBA;
		}
		else if (Version == enumWINDOWS_XP ||
			Version == enumWINDOWS_2K3 ||
			Version == enumWINDOWS_2K3_SP1_SP2 ||
			Version == enumWINDOWS_VISTA ||
			Version == enumWINDOWS_VISTA_SP1_SP2 ||
			Version == enumWINDOWS_7)
		{
			btSign = 0xbe;
		}
		else if (Version == enumWINDOWS_8)
		{
			btSign = 0xBB;
		}

		if (btSign == 0)
		{
			return;
		}

		for (i = 0; i < pEnd; i += nCodeLen)
		{
			PCHAR OpCode = 0;

			if (!MzfMmIsAddressValid((PVOID)((ULONG)MzfPsSetLoadImageNotifyRoutine + i)))
			{
				return;
			}

			nCodeLen = SizeOfCode( (PVOID)((ULONG)MzfPsSetLoadImageNotifyRoutine + i), &OpCode);
			if (nCodeLen == 0)
			{
				return;
			}
			else if (nCodeLen == 5 && *(PBYTE)((ULONG)MzfPsSetLoadImageNotifyRoutine + i) == btSign)
			{
				PspLoadImageNotifyRoutine = *(PULONG)((ULONG)MzfPsSetLoadImageNotifyRoutine + i + 1);

				if (PspLoadImageNotifyRoutine && MzfMmIsAddressValid((PVOID)(PspLoadImageNotifyRoutine)))
				{
					KdPrint(("方法二：PspLoadImageNotifyRoutine: 0x%08X\n", PspLoadImageNotifyRoutine));
					SetGlobalVeriable(enumPspLoadImageNotifyRoutine, PspLoadImageNotifyRoutine);
					break;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////

ULONG GetPsSetCreateProcessNotifyRoutineCommon(ULONG PsSetCreateProcessNotifyRoutineAddress)
{
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
	ULONG pAddress = 0;

	switch (Version)
	{
	case enumWINDOWS_2K:
	case enumWINDOWS_XP:
	case enumWINDOWS_2K3:
	case enumWINDOWS_2K3_SP1_SP2:
	case enumWINDOWS_VISTA:
		pAddress = PsSetCreateProcessNotifyRoutineAddress;
		break;

	case enumWINDOWS_VISTA_SP1_SP2:
	case enumWINDOWS_7:
	case enumWINDOWS_8:
		{
			ULONG i = 0, nCodeLen = 0, pEnd = PsSetCreateProcessNotifyRoutineAddress + PAGE_SIZE;
			pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
			
			if (!MzfMmIsAddressValid)
			{
				break;
			}

			for (i = 0; i < pEnd; i += nCodeLen)
			{
				PCHAR OpCode = 0;

				if (!MzfMmIsAddressValid((PVOID)((ULONG)PsSetCreateProcessNotifyRoutineAddress + i)))
				{
					return 0;
				}

				nCodeLen = SizeOfCode( (PVOID)((ULONG)PsSetCreateProcessNotifyRoutineAddress + i), &OpCode);
				if (nCodeLen == 0)
				{
					return 0;
				}
				else if (nCodeLen == 5 && *(PBYTE)((ULONG)PsSetCreateProcessNotifyRoutineAddress + i) == 0xe8)
				{
					pAddress = PsSetCreateProcessNotifyRoutineAddress + 
						*(PULONG)((ULONG)PsSetCreateProcessNotifyRoutineAddress + i + 1) + 5;

					break;
				}
			}
		}
		break;
	}

	return pAddress;
}

VOID GetPspCreateProcessNotifyRoutine()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG PspCreateProcessNotifyRoutine = 0;
	pfnPsSetCreateProcessNotifyRoutine MzfPsSetCreateProcessNotifyRoutine = (pfnPsSetCreateProcessNotifyRoutine)GetGlobalVeriable(enumPsSetCreateProcessNotifyRoutine);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	
	ULONG pAddress = 0;

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	pAddress = GetPsSetCreateProcessNotifyRoutineCommon((ULONG)MzfPsSetCreateProcessNotifyRoutine);

	if (pAddress)
	{
		status = MzfPsSetCreateProcessNotifyRoutine(create_process_notify, FALSE);
		if (NT_SUCCESS(status))
		{
			PspCreateProcessNotifyRoutine = find_PspSetxxxNotifyRoutine_address(
				(PVOID)pAddress, 
				create_process_notify
				);

			if (PspCreateProcessNotifyRoutine && MzfMmIsAddressValid((PVOID)(PspCreateProcessNotifyRoutine)) )
			{
				KdPrint(("方法一：PspCreateProcessNotifyRoutine: 0x%08X\n", PspCreateProcessNotifyRoutine));
				SetGlobalVeriable(enumPspCreateProcessNotifyRoutine, PspCreateProcessNotifyRoutine);
			}

			MzfPsSetCreateProcessNotifyRoutine(create_process_notify, TRUE);
		}
	}

	if (pAddress && 
		!PspCreateProcessNotifyRoutine)
	{
		ULONG pStart = (ULONG)pAddress;
		ULONG pEnd = (ULONG)pAddress + PAGE_SIZE;
		ULONG i = 0, nCodeLen = 0;
		BYTE btSign = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (Version == enumWINDOWS_2K)
		{
			btSign = 0xBA;
		}
		else if (Version == enumWINDOWS_XP ||
			Version == enumWINDOWS_2K3 ||
			Version == enumWINDOWS_2K3_SP1_SP2 ||
			Version == enumWINDOWS_VISTA )
		{
			btSign = 0xbf;
		}
		else if (Version == enumWINDOWS_VISTA_SP1_SP2 ||
			Version == enumWINDOWS_7)
		{
			btSign = 0xc7;
		}
		else if (Version == enumWINDOWS_8)
		{
			btSign =  0xB8;
		}

		if (btSign == 0)
		{
			return;
		}

		for (i = 0; i < pEnd; i += nCodeLen)
		{
			PCHAR OpCode = 0;

			if (!MzfMmIsAddressValid((PVOID)((ULONG)pAddress + i)))
			{
				return;
			}

			nCodeLen = SizeOfCode( (PVOID)((ULONG)pAddress + i), &OpCode);
			if (nCodeLen == 0)
			{
				return;
			}
			else if ((nCodeLen == 5 || nCodeLen == 7) && *(PBYTE)((ULONG)pAddress + i) == btSign)
			{
				if (Version == enumWINDOWS_VISTA_SP1_SP2 ||
					Version == enumWINDOWS_7)
				{
					PspCreateProcessNotifyRoutine = *(PULONG)((ULONG)pAddress + i + 3);
				}
				else
				{
					PspCreateProcessNotifyRoutine = *(PULONG)((ULONG)pAddress + i + 1);
				}

				if (PspCreateProcessNotifyRoutine && MzfMmIsAddressValid((PVOID)(PspCreateProcessNotifyRoutine)))
				{
					KdPrint(("方法二：PspCreateProcessNotifyRoutine: 0x%08X\n", PspCreateProcessNotifyRoutine));
					SetGlobalVeriable(enumPspCreateProcessNotifyRoutine, PspCreateProcessNotifyRoutine);
					break;
				}
			}
		}
	}
}

VOID GetPspCreateThreadNotifyRoutine()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG PspCreateThreadNotifyRoutine = 0;
	pfnPsSetCreateThreadNotifyRoutine MzfPsSetCreateThreadNotifyRoutine = (pfnPsSetCreateThreadNotifyRoutine)GetGlobalVeriable(enumPsSetCreateThreadNotifyRoutine);
	pfnPsRemoveCreateThreadNotifyRoutine MzfPsRemoveCreateThreadNotifyRoutine = (pfnPsRemoveCreateThreadNotifyRoutine)GetGlobalVeriable(enumPsRemoveCreateThreadNotifyRoutine);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (MzfPsSetCreateThreadNotifyRoutine && MzfPsRemoveCreateThreadNotifyRoutine)
	{
		status = MzfPsSetCreateThreadNotifyRoutine(create_thread_notify);
		if (NT_SUCCESS(status))
		{
			PspCreateThreadNotifyRoutine = find_PspSetxxxNotifyRoutine_address(
				MzfPsSetCreateThreadNotifyRoutine, 
				create_thread_notify
				);

			if (PspCreateThreadNotifyRoutine && MzfMmIsAddressValid((PVOID)(PspCreateThreadNotifyRoutine)))
			{
				KdPrint(("方法一：PspCreateThreadNotifyRoutine: 0x%08X\n", PspCreateThreadNotifyRoutine));
				SetGlobalVeriable(enumPspCreateThreadNotifyRoutine, PspCreateThreadNotifyRoutine);
			}

			MzfPsRemoveCreateThreadNotifyRoutine(create_thread_notify);
		}
	}

	if (MzfPsSetCreateThreadNotifyRoutine && 
		!PspCreateThreadNotifyRoutine)
	{
		ULONG pStart = (ULONG)MzfPsSetCreateThreadNotifyRoutine;
		ULONG pEnd = (ULONG)MzfPsSetCreateThreadNotifyRoutine + PAGE_SIZE;
		ULONG i = 0, nCodeLen = 0;
		BYTE btSign = 0;
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);
		
		if (Version == enumWINDOWS_2K)
		{
			btSign = 0xBA;
		}
		else if (Version == enumWINDOWS_XP ||
			Version == enumWINDOWS_2K3 ||
			Version == enumWINDOWS_2K3_SP1_SP2 ||
			Version == enumWINDOWS_VISTA ||
			Version == enumWINDOWS_VISTA_SP1_SP2 ||
			Version == enumWINDOWS_7)
		{
			btSign = 0xbe;
		}
		else if (Version == enumWINDOWS_8)
		{
			btSign = 0xBB;
		}

		if (btSign == 0)
		{
			return;
		}

		for (i = 0; i < pEnd; i += nCodeLen)
		{
			PCHAR OpCode = 0;

			if (!MzfMmIsAddressValid((PVOID)((ULONG)MzfPsSetCreateThreadNotifyRoutine + i)))
			{
				return;
			}

			nCodeLen = SizeOfCode( (PVOID)((ULONG)MzfPsSetCreateThreadNotifyRoutine + i), &OpCode);
			if (nCodeLen == 0)
			{
				return;
			}
			else if (nCodeLen == 5 && *(PBYTE)((ULONG)MzfPsSetCreateThreadNotifyRoutine + i) == btSign)
			{
				PspCreateThreadNotifyRoutine = *(PULONG)((ULONG)MzfPsSetCreateThreadNotifyRoutine + i + 1);

				if (PspCreateThreadNotifyRoutine && MzfMmIsAddressValid((PVOID)(PspCreateThreadNotifyRoutine)))
				{
					KdPrint(("方法二：PspCreateThreadNotifyRoutine: 0x%08X\n", PspCreateThreadNotifyRoutine));
					SetGlobalVeriable(enumPspCreateThreadNotifyRoutine, PspCreateThreadNotifyRoutine);
					break;
				}
			}
		}
	}
}

VOID GetIopNotifyShutdownQueueHead()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG IopNotifyShutdownQueueHead = 0;
	pfnIoRegisterShutdownNotification MzfIoRegisterShutdownNotification  = (pfnIoRegisterShutdownNotification)GetGlobalVeriable(enumIoRegisterShutdownNotification );
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG pStart = 0, pEnd = 0;
	ULONG i = 0, nCodeLen = 0;
	BYTE btSign = 0;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	if (!MzfMmIsAddressValid ||
		!MzfIoRegisterShutdownNotification)
	{
		return;
	}

	pStart = (ULONG)MzfIoRegisterShutdownNotification;
	pEnd = (ULONG)MzfIoRegisterShutdownNotification + PAGE_SIZE;
	
	if (Version == enumWINDOWS_2K ||
		Version == enumWINDOWS_XP ||
		Version == enumWINDOWS_2K3 ||
		Version == enumWINDOWS_2K3_SP1_SP2)
	{
		btSign = 0xB9;
	}
	else if (Version == enumWINDOWS_VISTA ||
		Version == enumWINDOWS_VISTA_SP1_SP2 ||
		Version == enumWINDOWS_7 ||
		Version == enumWINDOWS_8)
	{
		btSign = 0xbf;
	}

	if (btSign == 0)
	{
		return;
	}

	for (i = 0; i < pEnd; i += nCodeLen)
	{
		PCHAR OpCode = 0;

		if (!MzfMmIsAddressValid((PVOID)((ULONG)MzfIoRegisterShutdownNotification + i)))
		{
			return;
		}

		nCodeLen = SizeOfCode( (PVOID)((ULONG)MzfIoRegisterShutdownNotification + i), &OpCode);
		if (nCodeLen == 0)
		{
			return;
		}
		else if (nCodeLen == 5 && *(PBYTE)((ULONG)MzfIoRegisterShutdownNotification + i) == btSign)
		{
			IopNotifyShutdownQueueHead = *(PULONG)((ULONG)MzfIoRegisterShutdownNotification + i + 1);

			if (IopNotifyShutdownQueueHead && MzfMmIsAddressValid((PVOID)(IopNotifyShutdownQueueHead)))
			{
				KdPrint(("方法1：IopNotifyShutdownQueueHead: 0x%08X\n", IopNotifyShutdownQueueHead));
				SetGlobalVeriable(enumIopNotifyShutdownQueueHead, IopNotifyShutdownQueueHead);
				break;
			}
		}
	}
	
}

VOID GetCmpCallBackVector()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG CmpCallBackVector = 0;
	pfnCmUnRegisterCallback MzfCmUnRegisterCallback  = (pfnCmUnRegisterCallback)GetGlobalVeriable(enumCmUnRegisterCallback);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG pStart = 0, pEnd = 0;
	ULONG i = 0, nCodeLen = 0;
	BYTE btSign = 0;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	if (!MzfMmIsAddressValid ||
		!MzfCmUnRegisterCallback)
	{
		return;
	}

	pStart = (ULONG)MzfCmUnRegisterCallback;
	pEnd = (ULONG)MzfCmUnRegisterCallback + PAGE_SIZE;

	if (Version == enumWINDOWS_2K ||
		Version == enumWINDOWS_XP ||
		Version == enumWINDOWS_2K3 ||
		Version == enumWINDOWS_2K3_SP1_SP2)
	{
		btSign = 0xBB;
	}
	else if (Version == enumWINDOWS_VISTA ||
		Version == enumWINDOWS_VISTA_SP1_SP2 ||
		Version == enumWINDOWS_8)
	{
		btSign = 0xbe;
	}
	else if (Version == enumWINDOWS_7)
	{
		btSign = 0xbf;
	}

	if (!btSign)
	{
		return;
	}

	for (i = 0; i < pEnd; i += nCodeLen)
	{
		PCHAR OpCode = 0;

		if (!MzfMmIsAddressValid((PVOID)((ULONG)MzfCmUnRegisterCallback + i)))
		{
			return;
		}

		nCodeLen = SizeOfCode( (PVOID)((ULONG)MzfCmUnRegisterCallback + i), &OpCode);
		if (nCodeLen == 0)
		{
			return;
		}
		else if (nCodeLen == 5 && *(PBYTE)((ULONG)MzfCmUnRegisterCallback + i) == btSign)
		{
			CmpCallBackVector = *(PULONG)((ULONG)MzfCmUnRegisterCallback + i + 1);

			if (CmpCallBackVector && MzfMmIsAddressValid((PVOID)(CmpCallBackVector)))
			{
				KdPrint(("方法1：CmpCallBackVector: 0x%08X\n", CmpCallBackVector));
				SetGlobalVeriable(enumCmpCallBackVector, CmpCallBackVector);
				break;
			}
		}
	}
}

void GetProessCallbackNotify(PGET_CALLBACK pGetCallback)
{
	ULONG PspCreateProcessNotifyRoutine = GetGlobalVeriable(enumPspCreateProcessNotifyRoutine);
	if (!PspCreateProcessNotifyRoutine)
	{
		GetPspCreateProcessNotifyRoutine();
		PspCreateProcessNotifyRoutine = GetGlobalVeriable(enumPspCreateProcessNotifyRoutine);
	}

	if (PspCreateProcessNotifyRoutine)
	{
		ULONG i = 0;
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (!MzfMmIsAddressValid)
		{
			return;
		}

		for ( i = 0; i < 64; i++ )
		{
			ULONG pNotifyItem = 0, pValidAddress = 0;

			if ( !MzfMmIsAddressValid( (PVOID)(PspCreateProcessNotifyRoutine + i * sizeof(ULONG))) )
			{
				break;
			}

			pNotifyItem = *(PULONG)(PspCreateProcessNotifyRoutine + i * sizeof(ULONG));

			if (Version == enumWINDOWS_2K)
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem))) )
				{
					break;
				}
			}
			else 
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG)))) )
				{
					break;
				}
			}

			if (Version == enumWINDOWS_2K)
			{
				pValidAddress = pNotifyItem;
			}
			else 
			{
				pValidAddress = *(PULONG)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG));
			}

			if (pValidAddress && MzfMmIsAddressValid((PVOID)pValidAddress))
			{
				if (pGetCallback->nCnt > pGetCallback->nRetCnt)
				{
					pGetCallback->Callbacks[pGetCallback->nRetCnt].type = enumCreateProcess;
					pGetCallback->Callbacks[pGetCallback->nRetCnt].pCallbackAddress = pValidAddress;
					pGetCallback->Callbacks[pGetCallback->nRetCnt].Note = pNotifyItem;
				}

				pGetCallback->nRetCnt++;
			}
		}
	}
}

void GetThreadCallbackNotify(PGET_CALLBACK pGetCallback)
{
	ULONG PspCreateThreadNotifyRoutine = GetGlobalVeriable(enumPspCreateThreadNotifyRoutine);
	if (!PspCreateThreadNotifyRoutine)
	{
		GetPspCreateThreadNotifyRoutine();
		PspCreateThreadNotifyRoutine = GetGlobalVeriable(enumPspCreateThreadNotifyRoutine);
	}

	if (PspCreateThreadNotifyRoutine)
	{
		ULONG i = 0;
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (!MzfMmIsAddressValid)
		{
			return;
		}

		for ( i = 0; i < 64; i++ )
		{
			ULONG pNotifyItem = 0, pValidAddress = 0;

			if ( !MzfMmIsAddressValid( (PVOID)(PspCreateThreadNotifyRoutine + i * sizeof(ULONG))) )
			{
				break;
			}

			pNotifyItem = *(PULONG)(PspCreateThreadNotifyRoutine + i * sizeof(ULONG));

			if (Version == enumWINDOWS_2K)
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem))) )
				{
					break;
				}
			}
			else 
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG)))) )
				{
					break;
				}
			}

			if (Version == enumWINDOWS_2K)
			{
				pValidAddress = pNotifyItem;
			}
			else 
			{
				pValidAddress = *(PULONG)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG));
			}

			if (pValidAddress && MzfMmIsAddressValid((PVOID)pValidAddress))
			{
				if (pGetCallback->nCnt > pGetCallback->nRetCnt)
				{
					pGetCallback->Callbacks[pGetCallback->nRetCnt].type = enumCreateThread;
					pGetCallback->Callbacks[pGetCallback->nRetCnt].pCallbackAddress = pValidAddress;
					pGetCallback->Callbacks[pGetCallback->nRetCnt].Note = pNotifyItem;
				}

				pGetCallback->nRetCnt++;
			}
		}
	}
}

void GetLoadImageCallbackNotify(PGET_CALLBACK pGetCallback)
{
	ULONG PspLoadImageNotifyRoutine = GetGlobalVeriable(enumPspLoadImageNotifyRoutine);
	if (!PspLoadImageNotifyRoutine)
	{
		GetPspLoadImageNotifyRoutine();
		PspLoadImageNotifyRoutine = GetGlobalVeriable(enumPspLoadImageNotifyRoutine);
	}

	if (PspLoadImageNotifyRoutine)
	{
		ULONG i = 0;
		pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (!MzfMmIsAddressValid)
		{
			return;
		}

		for ( i = 0; i < 64; i++ )
		{
			ULONG pNotifyItem = 0, pValidAddress = 0;

			if ( !MzfMmIsAddressValid( (PVOID)(PspLoadImageNotifyRoutine + i * sizeof(ULONG))) )
			{
				break;
			}

			pNotifyItem = *(PULONG)(PspLoadImageNotifyRoutine + i * sizeof(ULONG));

			if (Version == enumWINDOWS_2K)
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem))) )
				{
					break;
				}
			}
			else 
			{
				if ( !(pNotifyItem > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG)))) )
				{
					break;
				}
			}

			if (Version == enumWINDOWS_2K)
			{
				pValidAddress = pNotifyItem;
			}
			else 
			{
				pValidAddress = *(PULONG)(pNotifyItem & 0xFFFFFFF8 + sizeof(ULONG));
			}

			if (pValidAddress && MzfMmIsAddressValid((PVOID)pValidAddress))
			{
				if (pGetCallback->nCnt > pGetCallback->nRetCnt)
				{
					pGetCallback->Callbacks[pGetCallback->nRetCnt].type = enumLoadImage;
					pGetCallback->Callbacks[pGetCallback->nRetCnt].pCallbackAddress = pValidAddress;
					pGetCallback->Callbacks[pGetCallback->nRetCnt].Note = pNotifyItem;
				}

				pGetCallback->nRetCnt++;
			}
		}
	}
}

ULONG GetShutdownDispatch(PDEVICE_OBJECT pDevice)
{
	PDRIVER_OBJECT pDriver = NULL;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	ULONG ShutdownDispatch = 0;

	if (!MzfMmIsAddressValid)
	{
		return 0;
	}

	if (pDevice && MzfMmIsAddressValid((PVOID)pDevice))
	{
		pDriver = pDevice->DriverObject;
		if (pDriver && MzfMmIsAddressValid((PVOID)pDriver))
		{
			ShutdownDispatch = (ULONG)pDriver->MajorFunction[IRP_MJ_SHUTDOWN];
		}
	}

	return ShutdownDispatch;
}

// 获取关机回调
void GetShutdownCallbackNotify(PGET_CALLBACK pGetCallback)
{
	PLIST_ENTRY IopNotifyShutdownQueueHead = (PLIST_ENTRY)GetGlobalVeriable(enumIopNotifyShutdownQueueHead);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (!IopNotifyShutdownQueueHead)
	{
		GetIopNotifyShutdownQueueHead();
		IopNotifyShutdownQueueHead = (PLIST_ENTRY)GetGlobalVeriable(enumIopNotifyShutdownQueueHead);
	}

	if (IopNotifyShutdownQueueHead && MzfMmIsAddressValid(IopNotifyShutdownQueueHead))
	{
		PLIST_ENTRY pEntry = IopNotifyShutdownQueueHead->Flink;
		while (MzfMmIsAddressValid(pEntry) && pEntry != IopNotifyShutdownQueueHead)
		{
			ULONG pValidAddress = (ULONG)pEntry + sizeof(LIST_ENTRY);
		
			if (pValidAddress && 
				MzfMmIsAddressValid((PVOID)pValidAddress))
			{
				ULONG pDeviceObject = *(PULONG)pValidAddress;

				if (pDeviceObject && 
					MzfMmIsAddressValid((PVOID)pDeviceObject))
				{
					if (pGetCallback->nCnt > pGetCallback->nRetCnt)
					{
						pGetCallback->Callbacks[pGetCallback->nRetCnt].type = enumShutdown;
						pGetCallback->Callbacks[pGetCallback->nRetCnt].pCallbackAddress = GetShutdownDispatch((PDEVICE_OBJECT)pDeviceObject);
						pGetCallback->Callbacks[pGetCallback->nRetCnt].Note = pDeviceObject;
					}

					pGetCallback->nRetCnt++;
				}
			}

			pEntry = pEntry->Flink;
		}
	}
}

// 获取注册表回调
void GetRegstryCallbackNotify(PGET_CALLBACK pGetCallback)
{
	ULONG CmpCallBackVector = GetGlobalVeriable(enumCmpCallBackVector);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!MzfMmIsAddressValid)
	{
		return;
	}

	if (!CmpCallBackVector)
	{
		GetCmpCallBackVector();
		CmpCallBackVector = GetGlobalVeriable(enumCmpCallBackVector);
	}

	if (CmpCallBackVector && MzfMmIsAddressValid((PVOID)CmpCallBackVector))
	{
		WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

		if (Version == enumWINDOWS_XP ||
			Version == enumWINDOWS_2K3_SP1_SP2 ||
			Version == enumWINDOWS_2K3)
		{
			ULONG nSize = 0;
			ULONG pEntry = CmpCallBackVector;

			while (nSize < 0x190 && MzfMmIsAddressValid((PVOID)pEntry))
			{
				ULONG pNote = *(PULONG)pEntry;

				if (pNote && 
					MzfMmIsAddressValid((PVOID)pNote))
				{
					ULONG pValidAddress = 0;

					if ( !(pNote > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)(pNote & 0xFFFFFFF8 + sizeof(ULONG)))) )
					{
						break;
					}

					pValidAddress = *(PULONG)(pNote & 0xFFFFFFF8 + sizeof(ULONG));
			
					if (pGetCallback->nCnt > pGetCallback->nRetCnt)
					{
						pGetCallback->Callbacks[pGetCallback->nRetCnt].type = enumCmCallBack;
						pGetCallback->Callbacks[pGetCallback->nRetCnt].pCallbackAddress = pValidAddress;
						pGetCallback->Callbacks[pGetCallback->nRetCnt].Note = pNote;
					}

					pGetCallback->nRetCnt++;
				}

				nSize += sizeof(ULONG);
				pEntry += sizeof(ULONG);
			}
		}
		else if (Version == enumWINDOWS_VISTA ||
			Version == enumWINDOWS_VISTA_SP1_SP2 ||
			Version == enumWINDOWS_7 ||
			Version == enumWINDOWS_8)
		{
			PLIST_ENTRY pFirstEntry = *(PLIST_ENTRY *)CmpCallBackVector;
			PLIST_ENTRY pEntry = pFirstEntry;

			while ((ULONG)pEntry > SYSTEM_ADDRESS_START &&
				MzfMmIsAddressValid((PVOID)pEntry) &&
				pEntry->Flink != (PLIST_ENTRY)pFirstEntry)
			{
				if (MzfMmIsAddressValid((PVOID)((ULONG)pEntry + 0x1C)))
				{
					ULONG pValidAddress = *(PULONG)((ULONG)pEntry + 0x1C);

					if (pGetCallback->nCnt > pGetCallback->nRetCnt)
					{
						pGetCallback->Callbacks[pGetCallback->nRetCnt].type = enumCmCallBack;
						pGetCallback->Callbacks[pGetCallback->nRetCnt].pCallbackAddress = pValidAddress;
						pGetCallback->Callbacks[pGetCallback->nRetCnt].Note = (ULONG)pEntry;
					}

					pGetCallback->nRetCnt++;
				}
				
				pEntry = pEntry->Flink;
			}
		}
	}
}

NTSTATUS GetAllCallbackNotify(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{	
	NTSTATUS status = STATUS_SUCCESS;
	PGET_CALLBACK pGetCallback = (PGET_CALLBACK)pOutBuffer;
	
	KdPrint(("GetAllCallbackNotify\n"));

	GetProessCallbackNotify(pGetCallback);
	GetThreadCallbackNotify(pGetCallback);
	GetLoadImageCallbackNotify(pGetCallback);
	GetRegstryCallbackNotify(pGetCallback);
	GetShutdownCallbackNotify(pGetCallback);
	
	if (pGetCallback->nRetCnt > pGetCallback->nCnt)
	{
		status = STATUS_BUFFER_TOO_SMALL;
		KdPrint(("STATUS_BUFFER_TOO_SMALL: 0x%08X, DosError: %d\n", status, RtlNtStatusToDosError(status)));
	}

	if (dwRet)
	{
		*dwRet = uOutSize;
	}

	return status;
}

NTSTATUS RemoveCreateProcessCallback(ULONG Callback)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsSetCreateProcessNotifyRoutine MzfPsSetCreateProcessNotifyRoutine = (pfnPsSetCreateProcessNotifyRoutine)GetGlobalVeriable(enumPsSetCreateProcessNotifyRoutine);

	if (!Callback ||
		!MzfMmIsAddressValid ||
		!MzfPsSetCreateProcessNotifyRoutine ||
		!MzfMmIsAddressValid((PVOID)Callback))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	return MzfPsSetCreateProcessNotifyRoutine(
		(PCREATE_PROCESS_NOTIFY_ROUTINE)Callback,
		TRUE
		);
}

NTSTATUS RemoveCreateThreadCallback(ULONG Callback)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsRemoveCreateThreadNotifyRoutine MzfPsRemoveCreateThreadNotifyRoutine = (pfnPsRemoveCreateThreadNotifyRoutine)GetGlobalVeriable(enumPsRemoveCreateThreadNotifyRoutine);

	if (!Callback ||
		!MzfMmIsAddressValid ||
		!MzfPsRemoveCreateThreadNotifyRoutine ||
		!MzfMmIsAddressValid((PVOID)Callback))
	{
		return STATUS_UNSUCCESSFUL;
	}

	return MzfPsRemoveCreateThreadNotifyRoutine( (PCREATE_THREAD_NOTIFY_ROUTINE)Callback );
}

NTSTATUS RemoveShutdownCallback(PDEVICE_OBJECT pDeviceObject)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnIoUnregisterShutdownNotification MzfIoUnregisterShutdownNotification = (pfnIoUnregisterShutdownNotification)GetGlobalVeriable(enumIoUnregisterShutdownNotification);

	if (!pDeviceObject ||
		!MzfMmIsAddressValid ||
		!MzfIoUnregisterShutdownNotification ||
		!MzfMmIsAddressValid((PVOID)pDeviceObject))
	{
		return STATUS_UNSUCCESSFUL;
	}

	MzfIoUnregisterShutdownNotification( pDeviceObject );

	return STATUS_SUCCESS;
}

NTSTATUS RemoveLoadImageCallback(ULONG Callback)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnPsRemoveLoadImageNotifyRoutine MzfPsRemoveLoadImageNotifyRoutine = (pfnPsRemoveLoadImageNotifyRoutine)GetGlobalVeriable(enumPsRemoveLoadImageNotifyRoutine);

	if (!Callback ||
		!MzfMmIsAddressValid ||
		!MzfPsRemoveLoadImageNotifyRoutine ||
		!MzfMmIsAddressValid((PVOID)Callback))
	{
		return STATUS_UNSUCCESSFUL;
	}

	return MzfPsRemoveLoadImageNotifyRoutine( (PLOAD_IMAGE_NOTIFY_ROUTINE)Callback );
}

LARGE_INTEGER XpGetRegstryCallbackCookie(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	LARGE_INTEGER Cookie;

	Cookie.QuadPart = 0;

	if (!MzfMmIsAddressValid)
	{
		return Cookie;
	}

	if (pAddress && MzfMmIsAddressValid((PVOID)pAddress))
	{
		ULONG pItem = pAddress & 0xFFFFFFF8;
		KdPrint(("pItem: 0x%08X\n", pItem));

		if (MzfMmIsAddressValid((PVOID)pItem) &&
			MzfMmIsAddressValid((PVOID)(pItem + 8)))
		{
			ULONG pCookie = *(PULONG)(pItem + 8);

			KdPrint(("pCookie: 0x%08X\n", pCookie));

			if (MzfMmIsAddressValid((PVOID)pCookie))
			{
				Cookie.LowPart = *(PULONG)pCookie;
				Cookie.HighPart = *(PULONG)(pCookie + sizeof(ULONG));
			}
		}
	}

	return Cookie;
}

LARGE_INTEGER Win7GetRegstryCallbackCookie(ULONG pAddress)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	LARGE_INTEGER Cookie;

	Cookie.QuadPart = 0;

	if (!MzfMmIsAddressValid)
	{
		return Cookie;
	}

	if (pAddress && MzfMmIsAddressValid((PVOID)pAddress) && MzfMmIsAddressValid((char *)pAddress + 0x37))
	{
		Cookie.LowPart = *((PULONG)pAddress + 4);
		Cookie.HighPart = *((PULONG)pAddress + 5);
	}

	return Cookie;
}

NTSTATUS RemoveRegistryCallback(ULONG pNote)
{
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);
	pfnCmUnRegisterCallback MzfCmUnRegisterCallback= (pfnCmUnRegisterCallback)GetGlobalVeriable(enumCmUnRegisterCallback);
	LARGE_INTEGER Cookie;
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	Cookie.QuadPart = 0;

	if (!pNote ||
		!MzfMmIsAddressValid ||
		!MzfCmUnRegisterCallback ||
		!MzfMmIsAddressValid((PVOID)pNote))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	if (Version == enumWINDOWS_XP ||
		Version == enumWINDOWS_2K3_SP1_SP2 ||
		Version == enumWINDOWS_2K3)
	{
		Cookie = XpGetRegstryCallbackCookie(pNote);
	}
	else
	{
		Cookie = Win7GetRegstryCallbackCookie(pNote);
	}

	if (Cookie.LowPart == 0 && Cookie.HighPart == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("LowPart: 0x%X, HighPart: 0x%X\n", Cookie.LowPart, Cookie.HighPart));
	return MzfCmUnRegisterCallback( Cookie );
}

NTSTATUS RemoveCallbackNotify(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{	
	NTSTATUS status = STATUS_SUCCESS;
	PCOMMUNICATE_CALLBACK pCc = (PCOMMUNICATE_CALLBACK)pInBuffer;
	CALLBACK_TYPE Type = pCc->Type;
	ULONG Note = pCc->Note;
	ULONG Callback = pCc->Callback;
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (!Callback ||
		!MzfMmIsAddressValid ||
		!MzfMmIsAddressValid((PVOID)Callback))
	{
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("Type: %d, Note: 0x%08X, Callback: 0x%08X\n", Type, Note, Callback));

	switch (Type)
	{
	case enumCreateProcess:
		status = RemoveCreateProcessCallback(Callback);
		break;

	case enumCreateThread:
		status = RemoveCreateThreadCallback(Callback);
		break;

	case enumLoadImage:
		status = RemoveLoadImageCallback(Callback);
		break;

	case enumShutdown:
		status = RemoveShutdownCallback((PDEVICE_OBJECT)Note); 
		break;

	case enumCmCallBack:
		status = RemoveRegistryCallback(Note);
		break;
	}

	return status;
}