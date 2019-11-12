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
#include "InitWindows.h"
#include "CommonFunction.h"
#include "HookPort.h"
#include "Peload.h"
#include "ReloadWin32k.h"
#include "FileSystem.h"
#include "IatEat.h"
#include "ReloadHalDll.h"
#include "ModifiedCode.h"
#include "SelfProtectSSDT.h"
#include "FakeFunc.h"

// extern LIST_ENTRY TempProcessListHead;				// ��������ͷ
// extern KSPIN_LOCK TempProcessSpinLock;				// ��������

#define MAX_VERIABLE 3000
ULONG GlobalVeriable[MAX_VERIABLE] = {0};
//LIST_ENTRY GlobalList[100];

VOID SetGlobalVeriable(VERIABLE_INDEX Index, ULONG Value)
{
	GlobalVeriable[Index] = Value;
}

ULONG GetGlobalVeriable(VERIABLE_INDEX Index)
{
	return GlobalVeriable[Index];
}

ULONG *GetGlobalVeriable_Address(VERIABLE_INDEX Index)
{
	return &GlobalVeriable[Index];
}

ULONG GetGlobalVeriableAddress(VERIABLE_INDEX Index)
{
	return ((ULONG)GlobalVeriable + Index * sizeof(ULONG));
}

WIN_VERSION GetWindowsVersion()
{
	RTL_OSVERSIONINFOEXW osverinfo = { sizeof(osverinfo) }; 
	pfnRtlGetVersion pRtlGetVersion = NULL;
	WIN_VERSION WinVersion;
	WCHAR szRtlGetVersion[] = {'R','t','l','G','e','t','V','e','r','s','i','o','n','\0'};

	pRtlGetVersion = GetFunctionAddressByName(/*L"RtlGetVersion"*/szRtlGetVersion); 

	if (pRtlGetVersion)
	{
		pRtlGetVersion((PRTL_OSVERSIONINFOW)&osverinfo); 
	} 
	else 
	{
		PsGetVersion(&osverinfo.dwMajorVersion, &osverinfo.dwMinorVersion, &osverinfo.dwBuildNumber, NULL);
	}

	KdPrint(("build number: %d\n", osverinfo.dwBuildNumber));
	SetGlobalVeriable(enumBuildNumber, (ULONG)(osverinfo.dwBuildNumber));
	
	if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 0) 
	{
		KdPrint(("WINDOWS_2K\n"));
		WinVersion = enumWINDOWS_2K;
	} 
	else if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 1) 
	{
		KdPrint(("WINDOWS_xp\n"));
		WinVersion = enumWINDOWS_XP;
	} 
	else if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 2) 
	{
		if (osverinfo.wServicePackMajor == 0) 
		{ 
			KdPrint(("WINDOWS_2K3\n"));
			WinVersion = enumWINDOWS_2K3;
		} 
		else 
		{
			KdPrint(("WINDOWS_2K3_sp1_sp2\n"));
			WinVersion = enumWINDOWS_2K3_SP1_SP2;
		}
	} 
	else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 0) 
	{
		if (osverinfo.dwBuildNumber == 6000)
		{
			KdPrint(("WINDOWS_VISTA\n"));
			WinVersion = enumWINDOWS_VISTA;
		}
		else if (osverinfo.dwBuildNumber == 6001 || osverinfo.dwBuildNumber == 6002)
		{
			KdPrint(("WINDOWS_VISTA_SP1_SP2\n"));
			WinVersion = enumWINDOWS_VISTA_SP1_SP2;
		}
	}
	else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 1)
	{
		KdPrint(("WINDOWS 7\n"));
		WinVersion = enumWINDOWS_7;
	}
	else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 2)
	{
		KdPrint(("WINDOWS 8\n"));
		switch (osverinfo.dwBuildNumber)
		{
		case 8250:
		case 8400:
		case 9200:
			WinVersion = enumWINDOWS_8;
			break;
		}
	}
	else
	{
		KdPrint(("WINDOWS_UNKNOW\n"));
		WinVersion = enumWINDOWS_UNKNOW;
	}

	return WinVersion;
}

//*************************************************

// Returns:   VOID
// Qualifier: ��ʼ��ȫ�ֱ�����һЩ�ṹƫ�ƣ�Show ssdt�������ȡ�

//*************************************************
VOID InitGlobalVariable()
{
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);

// 	InitializeListHead(&TempProcessListHead);
// 	KeInitializeSpinLock(&TempProcessSpinLock);
	
	SetGlobalVeriable(enumIdleThreadOffset_KPRCB, 0xc);
	SetGlobalVeriable(enumProcessOffset_KAPC_STATE, 0x10);
	SetGlobalVeriable(enumObjectHeaderSize, 0x18);
	SetGlobalVeriable(enumObjectTypeOffset_OBJECT_HEADER, 0x8);
	SetGlobalVeriable(enumIoDriverObjectType, (ULONG)*IoDriverObjectType);
	SetGlobalVeriable(enumIoDeviceObjectType, (ULONG)*IoDeviceObjectType);
	SetGlobalVeriable(enumIoFileObjectType, (ULONG)*IoFileObjectType);
	SetGlobalVeriable(enumPsInitialSystemProcessAddress, (ULONG)&PsInitialSystemProcess);
	SetGlobalVeriable(enumPsProcessType, (ULONG)*PsProcessType);
	SetGlobalVeriable(enumPsThreadType, (ULONG)*PsThreadType);

	SetGlobalVeriable(enumNtReadFileIndex, *(DWORD*)((PBYTE)ZwReadFile + 1));
	SetGlobalVeriable(enumNtWriteFileIndex, *(DWORD*)((PBYTE)ZwWriteFile + 1));

	switch (WinVersion)
	{
	case enumWINDOWS_2K:
		{
			// ���̱���
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0x128);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x1b0);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x1c8);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x5c);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x34);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0x9c);
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x10);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0x120); 

			// �̱߳���
			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x50);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x1e0);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x234);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x230);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1a4);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x20);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x33);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x4c);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x2d);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0x134);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x22c);
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x270);  
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x240); 
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x18);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x1c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0xd0);
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x1ad);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x1ac);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x194);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0x38);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x80);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x110);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x32);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x30);

			// ssdt table service id
			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0x9c);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0x97);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0x6a);			
			SetGlobalVeriable(enumNtCloseIndex, 0x18);						
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0x61); 
			SetGlobalVeriable(enumNtQueryObjectIndex, 0x8e);  
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0xe7);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0x6b);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0x88);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0xe1);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0xdd);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0xb5);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0xc5);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x47);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0x77);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xa);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0xe0);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0);	
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 110);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 149);
			SetGlobalVeriable(enumNtOpenKeyIndex, 103);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 60);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 61);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 53);
			SetGlobalVeriable(enumNtCreateKeyIndex, 35);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 215);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 55);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 164);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 240);
			SetGlobalVeriable(enumNtRenameKeyIndex, 0);  // todo

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 302);
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 466);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 437);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 639);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 470);
		}
		break;	

	case enumWINDOWS_XP:
		{
			// ���̱���
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0xc4);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x1b0);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x14c);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x60);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x34);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x138);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0x84);	
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x8);	 
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x13c);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0xbc); 

			// �̱߳���
			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x50);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x1ec);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x228);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x224);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1b0);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x20);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x33);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x4c);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x2d);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0x140); 
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x220);
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x190);  
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x22c);
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x18);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x1c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0xd4);
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x248);
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x24c);
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x1b9);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x1b8);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x250);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x11c);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0x38);
			
			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x100);
			
			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x118);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x32);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x30);

			// ssdt table service id
			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0xb2);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0xad);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0x7a);				
			SetGlobalVeriable(enumNtCloseIndex, 0x19);						
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0x71); 
			SetGlobalVeriable(enumNtQueryObjectIndex, 0xa3);   
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 267);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 123);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 156);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0x102);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0xfe);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0xce);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0xe3);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x53);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0x89);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xb);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0x101);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0xcd);
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0xfd);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0x7f);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0xaa);
			SetGlobalVeriable(enumNtOpenKeyIndex, 0x77);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x47);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x49);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 63);
			SetGlobalVeriable(enumNtCreateKeyIndex, 41);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 247);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 65);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 186);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 277);
			SetGlobalVeriable(enumNtRenameKeyIndex, 192);

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 483);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 453);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 586);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 490);

			// self protection
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 312);
			SetGlobalVeriable(enumNtUserDestroyWindowIndex, 355);
			SetGlobalVeriable(enumNtUserFindWindowExIndex, 378);
			SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 404);
			SetGlobalVeriable(enumNtUserMessageCallIndex, 460);
			SetGlobalVeriable(enumNtUserPostMessageIndex, 475);
			SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 476);
			SetGlobalVeriable(enumNtUserSetParentIndex, 529);
			SetGlobalVeriable(enumNtUserSetWindowLongIndex, 544);
			SetGlobalVeriable(enumNtUserShowWindowIndex, 555);
			SetGlobalVeriable(enumNtUserWindowFromPointIndex, 592);

			SetGlobalVeriable(enumNtDuplicateObjectIndex, 68);
			SetGlobalVeriable(enumNtOpenThreadIndex, 128);

			// forbid 
			SetGlobalVeriable(enumNtCreateProcessIndex, 0x2f);
			SetGlobalVeriable(enumNtCreateProcessExIndex, 0x30);
			SetGlobalVeriable(enumNtCreateThreadIndex, 53);
			SetGlobalVeriable(enumNtCreateFileIndex, 37);
			SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 558);
			SetGlobalVeriable(enumNtLoadKey2Index, 99);
			SetGlobalVeriable(enumNtReplaceKeyIndex, 193);
			SetGlobalVeriable(enumNtRestoreKeyIndex, 204);
			SetGlobalVeriable(enumNtInitiatePowerActionIndex, 93);
			SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 241);
			SetGlobalVeriable(enumNtUserCallOneParamIndex, 323);
			SetGlobalVeriable(enumNtShutdownSystemIndex, 0xF9);
			SetGlobalVeriable(enumNtSetSystemTimeIndex, 242);
			SetGlobalVeriable(enumNtUserLockWorkStationIndex, 457);
			SetGlobalVeriable(enumNtLoadDriverIndex, 97);
			SetGlobalVeriable(enumNtCreateSectionIndex, 50);

			// ��ֹ����
			SetGlobalVeriable(enumNtGdiGetDCPointIndex, 0xAA);
			SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 0xA7);
			SetGlobalVeriable(enumNtGdiBitBltIndex, 0xD);
			SetGlobalVeriable(enumNtGdiStretchBltIndex, 0x124);
			SetGlobalVeriable(enumNtGdiMaskBltIndex, 0xE3);
			SetGlobalVeriable(enumNtGdiPlgBltIndex, 0xED);
			SetGlobalVeriable(enumNtGdiTransparentBltIndex, 298);
			SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 7);
			SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 389);
		}
		break;

	case enumWINDOWS_2K3:
		{
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0xc4);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x190);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x128);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x60);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x34);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x114);
			SetGlobalVeriable(enumWaitListHeadOffset_KPRCB, 0x920);
			SetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB, 0x930);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0x84);	
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x8);	 
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x118);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0xbc); 

			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x50);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x1f4);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x230);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x22c);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1ac);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x30);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x5b);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x2c);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0x115);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x228);
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x170);  
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x234);
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x18);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x1c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x20);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0x70);
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x250);
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x254);
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x1bb);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x1ba);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x248);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x258);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0x38);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x100);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x100);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x32);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x30);

			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0xba);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0xb5);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0x80);				
			SetGlobalVeriable(enumNtCloseIndex, 0x1b);						
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0x77);
			SetGlobalVeriable(enumNtQueryObjectIndex, 0xaa);   
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x115);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0x81);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xa3);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0x10b);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0x107);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0xd6);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0xec);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x57);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0x8f);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xc);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0x10a);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0xd5);
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0x106);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0x85);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0xb2);
			SetGlobalVeriable(enumNtOpenKeyIndex, 0x7d);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x4b);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x4d);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 66);
			SetGlobalVeriable(enumNtCreateKeyIndex, 43);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 256);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 68);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 0xc2);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 0x11f);
			SetGlobalVeriable(enumNtRenameKeyIndex, 200);

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 481);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 452);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 582);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 488);

			// self protection
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 311);
			SetGlobalVeriable(enumNtUserDestroyWindowIndex, 354);
			SetGlobalVeriable(enumNtUserFindWindowExIndex, 377);
			SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 403);
			SetGlobalVeriable(enumNtUserMessageCallIndex, 459);
			SetGlobalVeriable(enumNtUserPostMessageIndex, 474);
			SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 475);
			SetGlobalVeriable(enumNtUserSetParentIndex, 526);
			SetGlobalVeriable(enumNtUserSetWindowLongIndex, 540);
			SetGlobalVeriable(enumNtUserShowWindowIndex, 551);
			SetGlobalVeriable(enumNtUserWindowFromPointIndex, 588);

			SetGlobalVeriable(enumNtDuplicateObjectIndex, 71);
			SetGlobalVeriable(enumNtOpenThreadIndex, 134);

			// forbid 
			SetGlobalVeriable(enumNtCreateProcessIndex, 49);
			SetGlobalVeriable(enumNtCreateProcessExIndex, 50);
			SetGlobalVeriable(enumNtCreateThreadIndex, 55);
			SetGlobalVeriable(enumNtCreateFileIndex, 39);
			SetGlobalVeriable(enumNtLoadKey2Index, 103);
			SetGlobalVeriable(enumNtReplaceKeyIndex, 201);
			SetGlobalVeriable(enumNtRestoreKeyIndex, 212);
			SetGlobalVeriable(enumNtInitiatePowerActionIndex, 97);
			SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 250);
			SetGlobalVeriable(enumNtShutdownSystemIndex, 258);
			SetGlobalVeriable(enumNtSetSystemTimeIndex, 251);
			SetGlobalVeriable(enumNtLoadDriverIndex, 101);
			SetGlobalVeriable(enumNtCreateSectionIndex, 52);
			
			SetGlobalVeriable(enumNtUserCallOneParamIndex, 322);
			SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 554);
			SetGlobalVeriable(enumNtUserLockWorkStationIndex, 454);

			// ��ֹ����
			SetGlobalVeriable(enumNtGdiGetDCPointIndex, 169);
			SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 166);
			SetGlobalVeriable(enumNtGdiBitBltIndex, 13);
			SetGlobalVeriable(enumNtGdiStretchBltIndex, 291);
			SetGlobalVeriable(enumNtGdiMaskBltIndex, 226);
			SetGlobalVeriable(enumNtGdiPlgBltIndex, 236);
			SetGlobalVeriable(enumNtGdiTransparentBltIndex, 297);
			SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 7);
			SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 388);
		}
		break;

	case enumWINDOWS_2K3_SP1_SP2:
		{
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0xd4);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x1a0);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x138);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x60);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x124);
			SetGlobalVeriable(enumWaitListHeadOffset_KPRCB, 0x9f0);
			SetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB, 0x930);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0x94);	 
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x8);	
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x128);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0xcc);

			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x50);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x1e4);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x220);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x21c);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1a8);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x74);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x5b);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x48);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x4c);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0xd7);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x218); 
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x224); 
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x180);   
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x18);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x1c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x20);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0x70);
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x240); 
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x244); 
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x150);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x14f);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x248);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x250);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0x38);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x200);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x108);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x32);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x30);

			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0xba);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0xb5);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0x80);			
			SetGlobalVeriable(enumNtCloseIndex, 0x1b);					
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0x77); 
			SetGlobalVeriable(enumNtQueryObjectIndex, 0xaa);  
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x115);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0x81);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xa3);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0x10b);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0x107);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0xd6);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0xec);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x57);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0x8f);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xc);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0x10a);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0xd5);
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0x106);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0x85);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0xb2);
			SetGlobalVeriable(enumNtOpenKeyIndex, 0x7d);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x4b);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x4d);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 0x42);
			SetGlobalVeriable(enumNtCreateKeyIndex, 0x2b);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 0x100);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 0x44);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 0xc2);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 0x11f);
			SetGlobalVeriable(enumNtRenameKeyIndex, 200);

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 311);
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 481);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 452);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 582);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 488);

			// self protection
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 311);
			SetGlobalVeriable(enumNtUserDestroyWindowIndex, 354);
			SetGlobalVeriable(enumNtUserFindWindowExIndex, 377);
			SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 403);
			SetGlobalVeriable(enumNtUserMessageCallIndex, 459);
			SetGlobalVeriable(enumNtUserPostMessageIndex, 474);
			SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 475);
			SetGlobalVeriable(enumNtUserSetParentIndex, 526);
			SetGlobalVeriable(enumNtUserSetWindowLongIndex, 540);
			SetGlobalVeriable(enumNtUserShowWindowIndex, 551);
			SetGlobalVeriable(enumNtUserWindowFromPointIndex, 588);

			SetGlobalVeriable(enumNtDuplicateObjectIndex, 71);
			SetGlobalVeriable(enumNtOpenThreadIndex, 134);

			// forbid 
			SetGlobalVeriable(enumNtCreateProcessIndex, 49);
			SetGlobalVeriable(enumNtCreateProcessExIndex, 50);
			SetGlobalVeriable(enumNtCreateThreadIndex, 55);
			SetGlobalVeriable(enumNtCreateFileIndex, 39);
			SetGlobalVeriable(enumNtLoadKey2Index, 103);
			SetGlobalVeriable(enumNtReplaceKeyIndex, 201);
			SetGlobalVeriable(enumNtRestoreKeyIndex, 212);
			SetGlobalVeriable(enumNtInitiatePowerActionIndex, 97);
			SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 250);
			SetGlobalVeriable(enumNtShutdownSystemIndex, 258);
			SetGlobalVeriable(enumNtSetSystemTimeIndex, 251);
			SetGlobalVeriable(enumNtLoadDriverIndex, 101);
			SetGlobalVeriable(enumNtCreateSectionIndex, 52);

			SetGlobalVeriable(enumNtUserCallOneParamIndex, 322);
			SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 554);
			SetGlobalVeriable(enumNtUserLockWorkStationIndex, 454);

			// ��ֹ����
			SetGlobalVeriable(enumNtGdiGetDCPointIndex, 169);
			SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 166);
			SetGlobalVeriable(enumNtGdiBitBltIndex, 13);
			SetGlobalVeriable(enumNtGdiStretchBltIndex, 291);
			SetGlobalVeriable(enumNtGdiMaskBltIndex, 226);
			SetGlobalVeriable(enumNtGdiPlgBltIndex, 236);
			SetGlobalVeriable(enumNtGdiTransparentBltIndex, 297);
			SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 7);
			SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 388);
		}
		break;

	case enumWINDOWS_VISTA:
		{
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0xdc);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x188);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x124);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x70);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x38);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x110);
			SetGlobalVeriable(enumWaitListHeadOffset_KPRCB, 0x1a20);
			SetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB, 0x1a60);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0x9c);	
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x8);
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x114);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0xd4);

			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x50);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x20c);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x240);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x1f8);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1c4);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x84);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x4f);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x58);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x5c);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0xe7);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x144);
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x248);
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x168);   
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x2c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x30);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0x80);
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x260);
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x264);
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x16c);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x16b);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x268);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x238);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0x38);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x200);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x118);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x32);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x30);

			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0xfd);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0xf8);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0xc2);			
			SetGlobalVeriable(enumNtCloseIndex, 0x30);						
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0xb7); 
			SetGlobalVeriable(enumNtQueryObjectIndex, 0xed);  
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x160);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0xc3);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xe6);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0x153);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0x14f);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0x119);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0x134);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x93);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0xd2);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xc);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0x152);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0x118);
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0x14e);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0xc8);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0xf5);
			SetGlobalVeriable(enumNtOpenKeyIndex, 0xbd);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x85);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x88);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 0x7b);
			SetGlobalVeriable(enumNtCreateKeyIndex, 0x40);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 0x148);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 0x7e);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 0x105);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 0x16a);
			SetGlobalVeriable(enumNtRenameKeyIndex, 267);

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 504);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 471);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 610);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 512);

			// self protection
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 322);
			SetGlobalVeriable(enumNtUserDestroyWindowIndex, 366);
			SetGlobalVeriable(enumNtUserFindWindowExIndex, 391);
			SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 418);
			SetGlobalVeriable(enumNtUserMessageCallIndex, 479);
			SetGlobalVeriable(enumNtUserPostMessageIndex, 497);
			SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 498);
			SetGlobalVeriable(enumNtUserSetParentIndex, 550);
			SetGlobalVeriable(enumNtUserSetWindowLongIndex, 566);
			SetGlobalVeriable(enumNtUserShowWindowIndex, 579);
			SetGlobalVeriable(enumNtUserWindowFromPointIndex, 617);

			SetGlobalVeriable(enumNtDuplicateObjectIndex, 129);
			SetGlobalVeriable(enumNtOpenThreadIndex, 201);

			// forbid 
			SetGlobalVeriable(enumNtCreateProcessIndex, 72);
			SetGlobalVeriable(enumNtCreateProcessExIndex, 73);
			SetGlobalVeriable(enumNtCreateThreadIndex, 78);
			SetGlobalVeriable(enumNtCreateFileIndex, 60);
			SetGlobalVeriable(enumNtLoadKey2Index, 167);
			SetGlobalVeriable(enumNtReplaceKeyIndex, 268);
			SetGlobalVeriable(enumNtRestoreKeyIndex, 279);
			SetGlobalVeriable(enumNtInitiatePowerActionIndex, 161);
			SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 322);
			SetGlobalVeriable(enumNtShutdownSystemIndex, 330);
			SetGlobalVeriable(enumNtSetSystemTimeIndex, 323);
			SetGlobalVeriable(enumNtLoadDriverIndex, 165);
			SetGlobalVeriable(enumNtCreateSectionIndex, 75);
			SetGlobalVeriable(enumNtCreateUserProcessIndex, 389);
			SetGlobalVeriable(enumNtCreateThreadExIndex, 388);

			SetGlobalVeriable(enumNtUserCallOneParamIndex, 332);
			SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 582);
			SetGlobalVeriable(enumNtUserLockWorkStationIndex, 475);

			// ��ֹ����
			SetGlobalVeriable(enumNtGdiGetDCPointIndex, 175);
			SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 172);
			SetGlobalVeriable(enumNtGdiBitBltIndex, 13);
			SetGlobalVeriable(enumNtGdiStretchBltIndex, 301);
			SetGlobalVeriable(enumNtGdiMaskBltIndex, 235);
			SetGlobalVeriable(enumNtGdiPlgBltIndex, 245);
			SetGlobalVeriable(enumNtGdiTransparentBltIndex, 307);
			SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 7);
			SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 403);
		}
		break;

	case enumWINDOWS_VISTA_SP1_SP2:
		{
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0xdc);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x188);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x124);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x70);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x38);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x110);
			SetGlobalVeriable(enumWaitListHeadOffset_KPRCB, 0x1aa0);
			SetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB, 0x1ae0);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0x9c);	
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x8);
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x114);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0xd4);

			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x50);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x20c);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x240);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x1f8);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1c4);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x84);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x4f);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x58);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x5c);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0xe7);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x144); 
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x248); 
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x168);   
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x2c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x30);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0x80);
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x260);
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x264);
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x16c);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x13c);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x268);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x238);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x200);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x118);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x32);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x30);

			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0xfd);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0xf8);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0xc2);			
			SetGlobalVeriable(enumNtCloseIndex, 0x30);					
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0xb7); 
			SetGlobalVeriable(enumNtQueryObjectIndex, 0xed);  
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x15c);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0xc3);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xe6);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0x14f);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0x14b);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0x11a);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0x130);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x93);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0xd2);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xc);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0x14e);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0x119);
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0x14a);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0xc8);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0xf5);
			SetGlobalVeriable(enumNtOpenKeyIndex, 0xbd);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x85);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x88);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 0x7b);
			SetGlobalVeriable(enumNtCreateKeyIndex, 0x40);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 0x144);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 0x7e);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 0x105);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 0x166);
			SetGlobalVeriable(enumNtRenameKeyIndex, 267);

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 504);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 471);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 610);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 512);

			// self protection
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 322);
			SetGlobalVeriable(enumNtUserDestroyWindowIndex, 366);
			SetGlobalVeriable(enumNtUserFindWindowExIndex, 391);
			SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 418);
			SetGlobalVeriable(enumNtUserMessageCallIndex, 479);
			SetGlobalVeriable(enumNtUserPostMessageIndex, 497);
			SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 498);
			SetGlobalVeriable(enumNtUserSetParentIndex, 550);
			SetGlobalVeriable(enumNtUserSetWindowLongIndex, 566);
			SetGlobalVeriable(enumNtUserShowWindowIndex, 579);
			SetGlobalVeriable(enumNtUserWindowFromPointIndex, 617);

			SetGlobalVeriable(enumNtDuplicateObjectIndex, 129);
			SetGlobalVeriable(enumNtOpenThreadIndex, 201);

			// forbid 
			SetGlobalVeriable(enumNtCreateProcessIndex, 72);
			SetGlobalVeriable(enumNtCreateProcessExIndex, 73);
			SetGlobalVeriable(enumNtCreateThreadIndex, 78);
			SetGlobalVeriable(enumNtCreateFileIndex, 60);
			SetGlobalVeriable(enumNtLoadKey2Index, 167);
			SetGlobalVeriable(enumNtReplaceKeyIndex, 268);
			SetGlobalVeriable(enumNtRestoreKeyIndex, 280);
			SetGlobalVeriable(enumNtInitiatePowerActionIndex, 161);
			SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 322);
			SetGlobalVeriable(enumNtShutdownSystemIndex, 330);
			SetGlobalVeriable(enumNtSetSystemTimeIndex, 319);
			SetGlobalVeriable(enumNtLoadDriverIndex, 165);
			SetGlobalVeriable(enumNtCreateSectionIndex, 75);
			SetGlobalVeriable(enumNtCreateUserProcessIndex, 383);
			SetGlobalVeriable(enumNtCreateThreadExIndex, 382);

			SetGlobalVeriable(enumNtUserCallOneParamIndex, 332);
			SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 582);
			SetGlobalVeriable(enumNtUserLockWorkStationIndex, 475);

			// ��ֹ����
			SetGlobalVeriable(enumNtGdiGetDCPointIndex, 175);
			SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 172);
			SetGlobalVeriable(enumNtGdiBitBltIndex, 13);
			SetGlobalVeriable(enumNtGdiStretchBltIndex, 301);
			SetGlobalVeriable(enumNtGdiMaskBltIndex, 235);
			SetGlobalVeriable(enumNtGdiPlgBltIndex, 245);
			SetGlobalVeriable(enumNtGdiTransparentBltIndex, 307);
			SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 7);
			SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 403);
		}
		break;

	case enumWINDOWS_7:
		{
			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0xf4);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x1a8);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x140);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x74);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x40);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x128);
			SetGlobalVeriable(enumWaitListHeadOffset_KPRCB, 0x31e0);
			SetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB, 0x3220);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0xb4);	
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x8);	
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x12c);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0xec);

			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x2c);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x22c);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x260);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x218);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1e0);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0x88);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x57);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x64);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x68);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0x13a);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x150);
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x268); 
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x188);  
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x28);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x2c);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x30);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0x84);
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x280);
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x284);
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x188);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x140);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x288);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x278);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x100);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x120);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0x6d);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x6e);

			SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0x10b);
			SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0x105);
			SetGlobalVeriable(enumNtOpenProcessIndex, 0xbe);				
			SetGlobalVeriable(enumNtCloseIndex, 0x32);				
			SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0xaf); 
			SetGlobalVeriable(enumNtQueryObjectIndex, 0xf8); 
			SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x181);
			SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0xbf);
			SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xed);
			SetGlobalVeriable(enumNtTerminateThreadIndex, 0x173);
			SetGlobalVeriable(enumNtSuspendThreadIndex, 0x16f);
			SetGlobalVeriable(enumNtResumeThreadIndex, 0x130);
			SetGlobalVeriable(enumNtSetInformationObjectIndex, 0x14c);
			SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x83);
			SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0xd7);
			SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0xc);
			SetGlobalVeriable(enumNtTerminateProcessIndex, 0x172);
			SetGlobalVeriable(enumNtResumeProcessIndex, 0x12f);
			SetGlobalVeriable(enumNtSuspendProcessIndex, 0x16e);
			SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0xc5);
			SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0x102);
			SetGlobalVeriable(enumNtOpenKeyIndex, 0xb6);
			SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x74);
			SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x77);
			SetGlobalVeriable(enumNtDeleteKeyIndex, 103);
			SetGlobalVeriable(enumNtCreateKeyIndex, 70);
			SetGlobalVeriable(enumNtSetValueKeyIndex, 358);
			SetGlobalVeriable(enumNtDeleteValueKeyIndex, 106);
			SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 277);
			SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 399);
			SetGlobalVeriable(enumNtRenameKeyIndex, 290);

			// shadow ssdt table service id
			SetGlobalVeriable(enumNtUserQueryWindowIndex, 515);
			SetGlobalVeriable(enumNtUserKillTimerIndex, 482);
			SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 623);
			SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 523);

			// self protection
			SetGlobalVeriable(enumNtUserBuildHwndListIndex, 323);
			SetGlobalVeriable(enumNtUserDestroyWindowIndex, 371);
			SetGlobalVeriable(enumNtUserFindWindowExIndex, 396);
			SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 423);
			SetGlobalVeriable(enumNtUserMessageCallIndex, 490);
			SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 509);
			SetGlobalVeriable(enumNtUserSetParentIndex, 560);
			SetGlobalVeriable(enumNtUserSetWindowLongIndex, 578);
			SetGlobalVeriable(enumNtUserShowWindowIndex, 591);
			SetGlobalVeriable(enumNtUserWindowFromPointIndex, 629);
			SetGlobalVeriable(enumNtUserPostMessageIndex, 508);

			SetGlobalVeriable(enumNtDuplicateObjectIndex, 111);
			SetGlobalVeriable(enumNtOpenThreadIndex, 198);

			// forbid 
			SetGlobalVeriable(enumNtCreateProcessIndex, 79);
			SetGlobalVeriable(enumNtCreateProcessExIndex, 80);
			SetGlobalVeriable(enumNtCreateThreadIndex, 87);
			SetGlobalVeriable(enumNtCreateFileIndex, 66);
			SetGlobalVeriable(enumNtLoadKey2Index, 157);
			SetGlobalVeriable(enumNtReplaceKeyIndex, 292);
			SetGlobalVeriable(enumNtRestoreKeyIndex, 302);
			SetGlobalVeriable(enumNtInitiatePowerActionIndex, 150);
			SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 351);
			SetGlobalVeriable(enumNtShutdownSystemIndex, 360);
			SetGlobalVeriable(enumNtSetSystemTimeIndex, 352);
			SetGlobalVeriable(enumNtLoadDriverIndex, 155);
			SetGlobalVeriable(enumNtCreateSectionIndex, 84);
			SetGlobalVeriable(enumNtCreateUserProcessIndex, 93);
			SetGlobalVeriable(enumNtCreateThreadExIndex, 88);

			SetGlobalVeriable(enumNtUserCallOneParamIndex, 333);
			SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 594);
			SetGlobalVeriable(enumNtUserLockWorkStationIndex, 486);

			// ��ֹ����
			SetGlobalVeriable(enumNtGdiGetDCPointIndex, 175);
			SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 172);
			SetGlobalVeriable(enumNtGdiBitBltIndex, 14);
			SetGlobalVeriable(enumNtGdiStretchBltIndex, 302);
			SetGlobalVeriable(enumNtGdiMaskBltIndex, 237);
			SetGlobalVeriable(enumNtGdiPlgBltIndex, 247);
			SetGlobalVeriable(enumNtGdiTransparentBltIndex, 308);
			SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 7);
			SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 408);
		}
		break;

	case enumWINDOWS_8:
		{	
			ULONG BuildNumber = 0;

			SetGlobalVeriable(enumObjectTableOffset_EPROCESS, 0x150);
			SetGlobalVeriable(enumPebOffset_EPROCESS, 0x140);
			SetGlobalVeriable(enumInheritedFromUniqueProcessIdOffset_EPROCESS, 0x134);
			SetGlobalVeriable(enumWaitListEntryOffset_KTHREAD, 0x9c);
			SetGlobalVeriable(enumApcStateOffset_KTHREAD, 0x70);
			SetGlobalVeriable(enumSectionObjectOffset_EPROCESS, 0x11c);
			SetGlobalVeriable(enumWaitListHeadOffset_KPRCB, 0x3ae0);
			SetGlobalVeriable(enumDispatcherReadyListHeadOffset_KPRCB, 0x3b20);
			SetGlobalVeriable(enumUniqueProcessIdOffset_EPROCESS, 0xb4);
			SetGlobalVeriable(enumUniqueProcessIdOffset_HANDLE_TABLE, 0x18);	
			SetGlobalVeriable(enumSectionBaseAddressOffset_EPROCESS, 0x120);
			SetGlobalVeriable(enumDebugportOffset_EPROCESS, 0x154);

			SetGlobalVeriable(enumThreadListHeadOffset_KPROCESS, 0x2c);
			SetGlobalVeriable(enumCidOffset_ETHREAD, 0x214);
			SetGlobalVeriable(enumWin32StartAddressOffset_ETHREAD, 0x244);
			SetGlobalVeriable(enumStartAddressOffset_ETHREAD, 0x200);
			SetGlobalVeriable(enumThreadListEntryOffset_KTHREAD, 0x1d4);
			SetGlobalVeriable(enumTebOffset_KTHREAD, 0xa8);
			SetGlobalVeriable(enumPriorityOffset_KTHREAD, 0x87);
			SetGlobalVeriable(enumContextSwitchesOffset_KTHREAD, 0x8c);
			SetGlobalVeriable(enumStateOffset_KTHREAD, 0x90);
			SetGlobalVeriable(enumPreviousModeOffset_KTHREAD, 0x15a);
			SetGlobalVeriable(enumThreadsProcessOffset_ETHREAD, 0x150); 
			SetGlobalVeriable(enumThreadListEntryOffset_ETHREAD, 0x24c); 
			SetGlobalVeriable(enumThreadListHeadOffset_EPROCESS, 0x194); 
			SetGlobalVeriable(enumInitialStackOffset_KTHREAD, 0x20);
			SetGlobalVeriable(enumStackLimitOffset_KTHREAD, 0x24);
			SetGlobalVeriable(enumKernelStackOffset_KTHREAD, 0x48);
			SetGlobalVeriable(enumKernelApcDisableOffset_KTHREAD, 0x13c); 
			SetGlobalVeriable(enumCrossThreadFlagsOffset_ETHREAD, 0x268); 
			SetGlobalVeriable(enumActiveExWorkerOffset_ETHREAD, 0x26c); 
			SetGlobalVeriable(enumSuspendCountOffset_KTHREAD, 0x18c);
			SetGlobalVeriable(enumFreezeCountOffset_KTHREAD, 0x5c);
			SetGlobalVeriable(enumSameThreadApcFlags_Offset_ETHREAD, 0x268);

			// ����ģ�����
			SetGlobalVeriable(enumVadRootOffset_EPROCESS, 0x274);

			// ö������ģ���õ��ı���
			SetGlobalVeriable(enumTypeListOffset_OBJECT_TYPE, 0);

			// DPC Timer
			SetGlobalVeriable(enumTIMER_TABLE_SIZE, 0x100);

			// Worker Thread
			SetGlobalVeriable(enumQueueListEntryOffset_KTHREAD, 0x140);

			// Direct IO
			SetGlobalVeriable(enumIoplOffset_KPROCESS, 0);
			SetGlobalVeriable(enumIopmOffsetOffset_KPROCESS, 0x6e);

			BuildNumber = GetGlobalVeriable(enumBuildNumber);
			if (BuildNumber == 9200)
			{
				SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0x8f);
				SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0x95);
				SetGlobalVeriable(enumNtOpenProcessIndex, 0xdd);		
				SetGlobalVeriable(enumNtCloseIndex, 0x174);				
				SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0xec);
				SetGlobalVeriable(enumNtQueryObjectIndex, 0xa2);
				SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x13);
				SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0xdc);
				SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xad);
				SetGlobalVeriable(enumNtTerminateThreadIndex, 0x22);
				SetGlobalVeriable(enumNtSuspendThreadIndex, 0x26);
				SetGlobalVeriable(enumNtResumeThreadIndex, 0x68);
				SetGlobalVeriable(enumNtSetInformationObjectIndex, 0x4b);
				SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x118);
				SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0xc3);
				SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0x19e); 
				SetGlobalVeriable(enumNtTerminateProcessIndex, 0x23); 
				SetGlobalVeriable(enumNtResumeProcessIndex, 0x69);
				SetGlobalVeriable(enumNtSuspendProcessIndex, 0x27);
				SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0xd6);
				SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0x98);
				SetGlobalVeriable(enumNtOpenKeyIndex, 0xe5);
				SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x12a);
				SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x127);
				SetGlobalVeriable(enumNtDeleteKeyIndex, 0x139);
				SetGlobalVeriable(enumNtCreateKeyIndex, 0x15e);
				SetGlobalVeriable(enumNtSetValueKeyIndex, 0x30);
				SetGlobalVeriable(enumNtDeleteValueKeyIndex, 0x136);
				SetGlobalVeriable(enumNtReadVirtualMemoryIndex, 0x83);
				SetGlobalVeriable(enumNtWriteVirtualMemoryIndex, 2);
				SetGlobalVeriable(enumNtRenameKeyIndex, 118);

				// shadow ssdt table service id	
				SetGlobalVeriable(enumNtUserQueryWindowIndex, 0x1E2);
				SetGlobalVeriable(enumNtUserKillTimerIndex, 0x205);
				SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 0x28f);
				SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 0x273);

				// self protection
				SetGlobalVeriable(enumNtUserBuildHwndListIndex, 360);
				SetGlobalVeriable(enumNtUserDestroyWindowIndex, 375);
				SetGlobalVeriable(enumNtUserFindWindowExIndex, 459);
				SetGlobalVeriable(enumNtUserGetForegroundWindowIndex, 429);
				SetGlobalVeriable(enumNtUserMessageCallIndex, 508);
				SetGlobalVeriable(enumNtUserPostThreadMessageIndex, 489);
				SetGlobalVeriable(enumNtUserPostMessageIndex, 490);
				SetGlobalVeriable(enumNtUserSetParentIndex, 590);
				SetGlobalVeriable(enumNtUserSetWindowLongIndex, 566);
				SetGlobalVeriable(enumNtUserShowWindowIndex, 553);
				SetGlobalVeriable(enumNtUserWindowFromPointIndex, 649);
				
				SetGlobalVeriable(enumNtDuplicateObjectIndex, 303);
				SetGlobalVeriable(enumNtOpenThreadIndex, 213);

				// forbid 
				SetGlobalVeriable(enumNtCreateProcessIndex, 341);
				SetGlobalVeriable(enumNtCreateProcessExIndex, 340);
				SetGlobalVeriable(enumNtCreateThreadIndex, 333);
				SetGlobalVeriable(enumNtCreateFileIndex, 355);
				SetGlobalVeriable(enumNtLoadKey2Index, 254);
				SetGlobalVeriable(enumNtReplaceKeyIndex, 116);
				SetGlobalVeriable(enumNtRestoreKeyIndex, 106);
				SetGlobalVeriable(enumNtInitiatePowerActionIndex, 261);
				SetGlobalVeriable(enumNtSetSystemPowerStateIndex, 55);
				SetGlobalVeriable(enumNtShutdownSystemIndex, 46);
				SetGlobalVeriable(enumNtSetSystemTimeIndex, 54);
				SetGlobalVeriable(enumNtLoadDriverIndex, 256);
				SetGlobalVeriable(enumNtCreateSectionIndex, 336);
				SetGlobalVeriable(enumNtCreateUserProcessIndex, 325);
				SetGlobalVeriable(enumNtCreateThreadExIndex, 332);

				SetGlobalVeriable(enumNtUserCallOneParamIndex, 350);
				SetGlobalVeriable(enumNtUserSwitchDesktopIndex, 547);
				SetGlobalVeriable(enumNtUserLockWorkStationIndex, 513);

				// ��ֹ����
				SetGlobalVeriable(enumNtGdiGetDCPointIndex, 139);
				SetGlobalVeriable(enumNtGdiGetDCDwordIndex, 142);
				SetGlobalVeriable(enumNtGdiBitBltIndex, 302);
				SetGlobalVeriable(enumNtGdiStretchBltIndex, 12);
				SetGlobalVeriable(enumNtGdiMaskBltIndex, 77);
				SetGlobalVeriable(enumNtGdiPlgBltIndex, 67);
				SetGlobalVeriable(enumNtGdiTransparentBltIndex, 6);
				SetGlobalVeriable(enumNtGdiAlphaBlendIndex, 309);
				SetGlobalVeriable(enumNtUserGetClipboardDataIndex, 447);
			}
// 			else
// 			{
// 				SetGlobalVeriable(enumNtQueryVirtualMemoryIndex, 0x90);
// 				SetGlobalVeriable(enumNtQuerySystemInformationIndex, 0x96);
// 				SetGlobalVeriable(enumNtOpenProcessIndex, 0xde);		
// 				SetGlobalVeriable(enumNtCloseIndex, 0x175);				
// 				SetGlobalVeriable(enumNtOpenDirectoryObjectIndex, 0xed);
// 				SetGlobalVeriable(enumNtQueryObjectIndex, 0xa3);
// 				SetGlobalVeriable(enumNtUnmapViewOfSectionIndex, 0x13);
// 				SetGlobalVeriable(enumNtOpenProcessTokenIndex, 0xdd);
// 				SetGlobalVeriable(enumNtQueryInformationTokenIndex, 0xae);
// 				SetGlobalVeriable(enumNtTerminateThreadIndex, 0x22);
// 				SetGlobalVeriable(enumNtSuspendThreadIndex, 0x26);
// 				SetGlobalVeriable(enumNtResumeThreadIndex, 0x69);
// 				SetGlobalVeriable(enumNtSetInformationObjectIndex, 0x4c);
// 				SetGlobalVeriable(enumNtFreeVirtualMemoryIndex, 0x119);
// 				SetGlobalVeriable(enumNtProtectVirtualMemoryIndex, 0xc4);
// 				SetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex, 0x19f); 
// 				SetGlobalVeriable(enumNtTerminateProcessIndex, 0x23); 
// 				SetGlobalVeriable(enumNtResumeProcessIndex, 0x6a);
// 				SetGlobalVeriable(enumNtSuspendProcessIndex, 0x27);
// 				SetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex, 0xd7);
// 				SetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex, 0x99);
// 				SetGlobalVeriable(enumNtOpenKeyIndex, 0xe6);
// 				SetGlobalVeriable(enumNtEnumerateKeyIndex, 0x12b);
// 				SetGlobalVeriable(enumNtEnumerateValueKeyIndex, 0x128);
// 
// 				// shadow ssdt table service id	
// 				SetGlobalVeriable(enumNtUserBuildHwndListIndex, 0x168);
// 				SetGlobalVeriable(enumNtUserQueryWindowIndex, 0x1E1);
// 				SetGlobalVeriable(enumNtUserKillTimerIndex, 0x203);
// 				SetGlobalVeriable(enumNtUserValidateTimerCallbackIndex, 0x28C);
// 				SetGlobalVeriable(enumNtUserRegisterHotKeyIndex, 0x270);
// 			}
		}
		break;

	case enumWINDOWS_UNKNOW:
		break;
	}

// 	InlieHookKiFastCallEntry();
// 	HookSwapContext();

// 	if (!InlieHookKiFastCallEntry())
// 	{
// 		if (HookSwapContext())
// 		{
// 			KdPrint(("HookSwapContext success\n"));
// 		}
// 	}
// 	else
// 	{
// 		KdPrint(("InlieHookKiFastCallEntry success\n"));
// 	}
}

VOID InitShadowSSDTFunctions()
{
	PServiceDescriptorTableEntry_t pOriginShadowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumOriginShadowSSDT); 
	PServiceDescriptorTableEntry_t pReloadShadowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumReloadShadowSSDT);
	PServiceDescriptorTableEntry_t pNowShadowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowShadowSSDT);
	PServiceDescriptorTableEntry_t pTable = NULL;

	ULONG pNtUserBuildHwndList = 0;
	ULONG pNtUserQueryWindow = 0;
	ULONG pNtUserKillTimer = 0;
	ULONG pNtUserValidateTimerCallback = 0;
	ULONG pNtUserRegisterHotKey = 0;
	ULONG pNtGdiGetDCPoint = 0;
	ULONG pNtGdiGetDCDword = 0;
	ULONG pNtUserWindowFromPoint = 0;

	ULONG nNtUserBuildHwndListIndex = GetGlobalVeriable(enumNtUserBuildHwndListIndex);
	ULONG nNtUserQueryWindowIndex = GetGlobalVeriable(enumNtUserQueryWindowIndex);
	ULONG nNtUserKillTimerIndex  = GetGlobalVeriable(enumNtUserKillTimerIndex);
	ULONG nNtUserValidateTimerCallbackIndex = GetGlobalVeriable(enumNtUserValidateTimerCallbackIndex);
	ULONG nNtUserRegisterHotKeyIndex = GetGlobalVeriable(enumNtUserRegisterHotKeyIndex);
	ULONG nNtGdiGetDCPointIndex = GetGlobalVeriable(enumNtGdiGetDCPointIndex);
	ULONG nNtGdiGetDCDwordIndex = GetGlobalVeriable(enumNtGdiGetDCDwordIndex);
	ULONG nNtUserWindowFromPointIndex = GetGlobalVeriable(enumNtUserWindowFromPointIndex);

	KAPC_STATE as;
	pfnKeStackAttachProcess MzfKeStackAttachProcess = (pfnKeStackAttachProcess)GetGlobalVeriable(enumKeStackAttachProcess);
	pfnKeUnstackDetachProcess MzfKeUnstackDetachProcess = (pfnKeUnstackDetachProcess)GetGlobalVeriable(enumKeUnstackDetachProcess);
	PEPROCESS pEprocess = (PEPROCESS)GetGlobalVeriable(enumCsrssEprocess);
	if (!pEprocess)
	{
		WCHAR szCsrss[] = {'c','s','r','s','s','.','e','x','e','\0'};
		pEprocess = LookupProcessByName(szCsrss);
	}

	if (!pEprocess || !MzfKeStackAttachProcess || !MzfKeUnstackDetachProcess)
	{
		return;
	}

	MzfKeStackAttachProcess(pEprocess, &as);

	if (pReloadShadowSSDT && pReloadShadowSSDT->ServiceTableBase)
	{
		pTable = pReloadShadowSSDT;
	}
	else if (pOriginShadowSSDT && pOriginShadowSSDT->ServiceTableBase)
	{
		pTable = pOriginShadowSSDT;
	}
	else if (pNowShadowSSDT && pNowShadowSSDT->ServiceTableBase)
	{
		pTable = pNowShadowSSDT;
	}

	if (pTable)
	{
		pNtUserBuildHwndList = pTable->ServiceTableBase[nNtUserBuildHwndListIndex];
		KdPrint(("pNtUserBuildHwndList: 0x%08X\n", pNtUserBuildHwndList));
		SetGlobalVeriable(enumNtUserBuildHwndList, pNtUserBuildHwndList);

		pNtUserQueryWindow = pTable->ServiceTableBase[nNtUserQueryWindowIndex];
		KdPrint(("pNtUserQueryWindow: 0x%08X\n", pNtUserQueryWindow));
		SetGlobalVeriable(enumNtUserQueryWindow, pNtUserQueryWindow);

		pNtUserKillTimer = pTable->ServiceTableBase[nNtUserKillTimerIndex];
		KdPrint(("pNtUserKillTimer: 0x%08X\n", pNtUserKillTimer));
		SetGlobalVeriable(enumNtUserKillTimer, pNtUserKillTimer);

		pNtUserValidateTimerCallback = pTable->ServiceTableBase[nNtUserValidateTimerCallbackIndex];
		KdPrint(("pNtUserValidateTimerCallback: 0x%08X\n", pNtUserValidateTimerCallback));
		SetGlobalVeriable(enumNtUserValidateTimerCallback, pNtUserValidateTimerCallback);

		pNtUserRegisterHotKey = pTable->ServiceTableBase[nNtUserRegisterHotKeyIndex];
		KdPrint(("pNtUserRegisterHotKey: 0x%08X\n", pNtUserRegisterHotKey));
		SetGlobalVeriable(enumNtUserRegisterHotKey, pNtUserRegisterHotKey);

		pNtGdiGetDCPoint = pTable->ServiceTableBase[nNtGdiGetDCPointIndex];
		KdPrint(("pNtGdiGetDCPoint: 0x%08X\n", pNtGdiGetDCPoint));
		SetGlobalVeriable(enumNtGdiGetDCPoint, pNtGdiGetDCPoint);

		pNtGdiGetDCDword = pTable->ServiceTableBase[nNtGdiGetDCDwordIndex];
		KdPrint(("pNtGdiGetDCDword: 0x%08X\n", pNtGdiGetDCDword));
		SetGlobalVeriable(enumNtGdiGetDCDword, pNtGdiGetDCDword);

		pNtUserWindowFromPoint = pTable->ServiceTableBase[nNtUserWindowFromPointIndex];
		KdPrint(("pNtUserWindowFromPoint: 0x%08X\n", pNtUserWindowFromPoint));
		SetGlobalVeriable(enumNtUserWindowFromPoint, pNtUserWindowFromPoint);
	}

	MzfKeUnstackDetachProcess(&as);
}

VOID InitSSDTFunctions()
{
	PServiceDescriptorTableEntry_t pOriginSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumOriginSSDT); 
	PServiceDescriptorTableEntry_t pReloadSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumReloadSSDT);
	PServiceDescriptorTableEntry_t pNowSSDT = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
	PServiceDescriptorTableEntry_t pTable = NULL;

	ULONG pNtQuerySystemInformation = 0;
	ULONG pNtOpenProcess = 0;
	ULONG pNtQueryVirtualMemory = 0;
	ULONG pNtClose = 0;
	ULONG pNtOpenDirectoryObject = 0;
	ULONG pNtQueryObject = 0;
	ULONG pNtOpenProcessToken = 0;
	ULONG pNtQueryInformationToken = 0;
	ULONG pNtUnmapViewOfSection = 0;
	ULONG pNtTerminateThread = 0;
	ULONG pNtResumeThread = 0;
	ULONG pNtSuspendThread = 0;
	ULONG pNtSetInformationObject = 0;
	ULONG pNtFreeVirtualMemory = 0;
	ULONG pNtProtectVirtualMemory = 0;
	ULONG pNtAdjustPrivilegesToken = 0;
	ULONG pNtTerminateProcess = 0;
	ULONG pNtResumeProcess = 0;
	ULONG pNtSuspendProcess = 0;
	ULONG pNtOpenSymbolicLinkObject = 0;
	ULONG pNtQuerySymbolicLinkObject = 0;
	ULONG pNtOpenKey = 0;
	ULONG pNtEnumerateKey = 0;
	ULONG pNtEnumerateValueKey = 0;
	ULONG pNtOpenFile = 0;
	ULONG pNtCreateSection = 0;
	ULONG pNtMapViewOfSection = 0;
	ULONG pNtDeleteKey = 0;
	ULONG pNtCreateKey = 0;
	ULONG pNtSetValueKey = 0;
	ULONG pNtDeleteValueKey = 0;
	ULONG pNtReadVirtualMemory = 0;
	ULONG pNtWriteVirtualMemory = 0;
	ULONG pNtRenameKey = 0;
	ULONG pNtShutdownSystem = 0;
	ULONG pNtCreateFile = 0;
	ULONG pNtReadFile = 0;
	ULONG pNtWriteFile = 0;

	ULONG nNtQuerySystemInformationIndex = GetGlobalVeriable(enumNtQuerySystemInformationIndex);
	ULONG nNtOpenProcessIndex = GetGlobalVeriable(enumNtOpenProcessIndex);
	ULONG nNtQueryVirtualMemoryIndex = GetGlobalVeriable(enumNtQueryVirtualMemoryIndex);
	ULONG nNtCloseIndex = GetGlobalVeriable(enumNtCloseIndex);
	ULONG nNtOpenDirectoryObjectIndex = GetGlobalVeriable(enumNtOpenDirectoryObjectIndex);
	ULONG nNtQueryObjectIndex = GetGlobalVeriable(enumNtQueryObjectIndex);
	ULONG nNtOpenProcessTokenIndex = GetGlobalVeriable(enumNtOpenProcessTokenIndex);
	ULONG nNtQueryInformationTokenIndex = GetGlobalVeriable(enumNtQueryInformationTokenIndex);
	ULONG nNtUnmapViewOfSectionIndex = GetGlobalVeriable(enumNtUnmapViewOfSectionIndex);
	ULONG nNtTerminateThreadIndex = GetGlobalVeriable(enumNtTerminateThreadIndex);
	ULONG nNtResumeThreadIndex = GetGlobalVeriable(enumNtResumeThreadIndex);
	ULONG nNtSuspendThreadIndex = GetGlobalVeriable(enumNtSuspendThreadIndex);
	ULONG nNtSetInformationObjectIndex = GetGlobalVeriable(enumNtSetInformationObjectIndex);
	ULONG nNtFreeVirtualMemoryIndex = GetGlobalVeriable(enumNtFreeVirtualMemoryIndex);
	ULONG nNtProtectVirtualMemoryIndex = GetGlobalVeriable(enumNtProtectVirtualMemoryIndex);
	ULONG nNtAdjustPrivilegesTokenIndex = GetGlobalVeriable(enumNtAdjustPrivilegesTokenIndex);
	ULONG nNtTerminateProcessIndex = GetGlobalVeriable(enumNtTerminateProcessIndex);
	ULONG nNtResumeProcessIndex = GetGlobalVeriable(enumNtResumeProcessIndex);
	ULONG nNtSuspendProcessIndex = GetGlobalVeriable(enumNtSuspendProcessIndex);
	ULONG nNtOpenSymbolicLinkObjectIndex = GetGlobalVeriable(enumNtOpenSymbolicLinkObjectIndex);
	ULONG nNtQuerySymbolicLinkObjectIndex = GetGlobalVeriable(enumNtQuerySymbolicLinkObjectIndex);
	ULONG nNtEnumerateValueKeyIndex = GetGlobalVeriable(enumNtEnumerateValueKeyIndex);
	ULONG nNtOpenKeyIndex = GetGlobalVeriable(enumNtOpenKeyIndex);
	ULONG nNtEnumerateKeyIndex = GetGlobalVeriable(enumNtEnumerateKeyIndex);
	ULONG nNtOpenFileIndex = GetGlobalVeriable(enumNtOpenFileIndex);
	ULONG nNtCreateSectionIndex = GetGlobalVeriable(enumNtCreateSectionIndex);
	ULONG nNtMapViewOfSectionIndex  = GetGlobalVeriable(enumNtMapViewOfSectionIndex);
	ULONG nNtDeleteKeyIndex = GetGlobalVeriable(enumNtDeleteKeyIndex);
	ULONG nNtCreateKeyIndex = GetGlobalVeriable(enumNtCreateKeyIndex);
	ULONG nNtSetValueKeyIndex = GetGlobalVeriable(enumNtSetValueKeyIndex);
	ULONG nNtDeleteValueKeyIndex = GetGlobalVeriable(enumNtDeleteValueKeyIndex);
	ULONG nNtReadVirtualMemoryIndex = GetGlobalVeriable(enumNtReadVirtualMemoryIndex);
	ULONG nNtWriteVirtualMemoryIndex = GetGlobalVeriable(enumNtWriteVirtualMemoryIndex);
	ULONG nNtRenameKeyIndex = GetGlobalVeriable(enumNtRenameKeyIndex);
	ULONG nNtShutdownSystemIndex = GetGlobalVeriable(enumNtShutdownSystemIndex);
	ULONG nNtCreateFileIndex = GetGlobalVeriable(enumNtCreateFileIndex);
	ULONG nNtReadFileIndex = GetGlobalVeriable(enumNtReadFileIndex);
	ULONG nNtWriteFileIndex = GetGlobalVeriable(enumNtWriteFileIndex);

	if (pReloadSSDT && pReloadSSDT->ServiceTableBase)
	{
		pTable = pReloadSSDT;
	}
	else if (pOriginSSDT && pOriginSSDT->ServiceTableBase)
	{
		pTable = pOriginSSDT;
	}
	else if (pNowSSDT && pNowSSDT->ServiceTableBase)
	{
		pTable = pNowSSDT;
	}

	if (pTable)
	{
		pNtQuerySystemInformation = pTable->ServiceTableBase[nNtQuerySystemInformationIndex];
		KdPrint(("pNtQuerySystemInformation: 0x%08X\n", pNtQuerySystemInformation));
		SetGlobalVeriable(enumNtQuerySystemInformation, pNtQuerySystemInformation);

		pNtOpenProcess = pTable->ServiceTableBase[nNtOpenProcessIndex];
		KdPrint(("pNtOpenProcess: 0x%08X\n", pNtOpenProcess));
		SetGlobalVeriable(enumNtOpenProcess, pNtOpenProcess);

		pNtQueryVirtualMemory = pTable->ServiceTableBase[nNtQueryVirtualMemoryIndex];
		KdPrint(("pNtQueryVirtualMemory: 0x%08X\n", pNtQueryVirtualMemory));
		SetGlobalVeriable(enumNtQueryVirtualMemory, pNtQueryVirtualMemory);

		pNtClose = pTable->ServiceTableBase[nNtCloseIndex];
		KdPrint(("pNtClose: 0x%08X\n", pNtClose));
		SetGlobalVeriable(enumNtClose, pNtClose);

		pNtOpenDirectoryObject = pTable->ServiceTableBase[nNtOpenDirectoryObjectIndex];
		KdPrint(("pNtOpenDirectoryObject: 0x%08X\n", pNtOpenDirectoryObject));
		SetGlobalVeriable(enumNtOpenDirectoryObject, pNtOpenDirectoryObject);

		pNtQueryObject = pTable->ServiceTableBase[nNtQueryObjectIndex];
		KdPrint(("pNtQueryObject: 0x%08X\n", pNtQueryObject));
		SetGlobalVeriable(enumNtQueryObject, pNtQueryObject);

		pNtOpenProcessToken = pTable->ServiceTableBase[nNtOpenProcessTokenIndex];
		KdPrint(("pNtOpenProcessToken: 0x%08X\n", pNtOpenProcessToken));
		SetGlobalVeriable(enumNtOpenProcessToken, pNtOpenProcessToken);

		pNtQueryInformationToken = pTable->ServiceTableBase[nNtQueryInformationTokenIndex];
		KdPrint(("pNtQueryInformationToken: 0x%08X\n", pNtQueryInformationToken));
		SetGlobalVeriable(enumNtQueryInformationToken, pNtQueryInformationToken);

		pNtUnmapViewOfSection = pTable->ServiceTableBase[nNtUnmapViewOfSectionIndex];
		KdPrint(("pNtUnmapViewOfSection: 0x%08X\n", pNtUnmapViewOfSection));
		SetGlobalVeriable(enumNtUnmapViewOfSection, pNtUnmapViewOfSection);

		pNtTerminateThread = pTable->ServiceTableBase[nNtTerminateThreadIndex];
		KdPrint(("pNtTerminateThread: 0x%08X\n", pNtTerminateThread));
		SetGlobalVeriable(enumNtTerminateThread, pNtTerminateThread);

		pNtResumeThread = pTable->ServiceTableBase[nNtResumeThreadIndex];
		KdPrint(("pNtResumeThread: 0x%08X\n", pNtResumeThread));
		SetGlobalVeriable(enumNtResumeThread, pNtResumeThread);

		pNtSuspendThread = pTable->ServiceTableBase[nNtSuspendThreadIndex];
		KdPrint(("pNtSuspendThread: 0x%08X\n", pNtSuspendThread));
		SetGlobalVeriable(enumNtSuspendThread, pNtSuspendThread);

		pNtSetInformationObject = pTable->ServiceTableBase[nNtSetInformationObjectIndex];
		KdPrint(("pNtSetInformationObject: 0x%08X\n", pNtSetInformationObject));
		SetGlobalVeriable(enumNtSetInformationObject, pNtSetInformationObject);

		pNtFreeVirtualMemory = pTable->ServiceTableBase[nNtFreeVirtualMemoryIndex];
		KdPrint(("pNtFreeVirtualMemory: 0x%08X\n", pNtFreeVirtualMemory));
		SetGlobalVeriable(enumNtFreeVirtualMemory, pNtFreeVirtualMemory);

		pNtProtectVirtualMemory = pTable->ServiceTableBase[nNtProtectVirtualMemoryIndex];
		KdPrint(("pNtProtectVirtualMemory: 0x%08X\n", pNtProtectVirtualMemory));
		SetGlobalVeriable(enumNtProtectVirtualMemory, pNtProtectVirtualMemory);

		pNtAdjustPrivilegesToken = pTable->ServiceTableBase[nNtAdjustPrivilegesTokenIndex];
		KdPrint(("pNtAdjustPrivilegesToken: 0x%08X\n", pNtAdjustPrivilegesToken));
		SetGlobalVeriable(enumNtAdjustPrivilegesToken, pNtAdjustPrivilegesToken);

		pNtTerminateProcess = pTable->ServiceTableBase[nNtTerminateProcessIndex];
		KdPrint(("pNtTerminateProcess: 0x%08X\n", pNtTerminateProcess));
		SetGlobalVeriable(enumNtTerminateProcess, pNtTerminateProcess);

		pNtResumeProcess = pTable->ServiceTableBase[nNtResumeProcessIndex];
		KdPrint(("pNtResumeProcess: 0x%08X\n", pNtResumeProcess));
		SetGlobalVeriable(enumNtResumeProcess, pNtResumeProcess);

		pNtSuspendProcess = pTable->ServiceTableBase[nNtSuspendProcessIndex];
		KdPrint(("pNtSuspendProcess: 0x%08X\n", pNtSuspendProcess));
		SetGlobalVeriable(enumNtSuspendProcess, pNtSuspendProcess);

		pNtOpenSymbolicLinkObject = pTable->ServiceTableBase[nNtOpenSymbolicLinkObjectIndex];
		KdPrint(("pNtOpenSymbolicLinkObject: 0x%08X\n", pNtOpenSymbolicLinkObject));
		SetGlobalVeriable(enumNtOpenSymbolicLinkObject, pNtOpenSymbolicLinkObject);

		pNtQuerySymbolicLinkObject = pTable->ServiceTableBase[nNtQuerySymbolicLinkObjectIndex];
		KdPrint(("pNtQuerySymbolicLinkObject: 0x%08X\n", pNtQuerySymbolicLinkObject));
		SetGlobalVeriable(enumNtQuerySymbolicLinkObject, pNtQuerySymbolicLinkObject);
		
		pNtOpenKey = pTable->ServiceTableBase[nNtOpenKeyIndex];
		KdPrint(("pNtOpenKey: 0x%08X\n", pNtOpenKey));
		SetGlobalVeriable(enumNtOpenKey, pNtOpenKey);
		
		pNtEnumerateKey = pTable->ServiceTableBase[nNtEnumerateKeyIndex];
		KdPrint(("pNtEnumerateKey: 0x%08X\n", pNtEnumerateKey));
		SetGlobalVeriable(enumNtEnumerateKey, pNtEnumerateKey);
		
		pNtEnumerateValueKey = pTable->ServiceTableBase[nNtEnumerateValueKeyIndex];
		KdPrint(("pNtEnumerateValueKey: 0x%08X\n", pNtEnumerateValueKey));
		SetGlobalVeriable(enumNtEnumerateValueKey, pNtEnumerateValueKey);

		pNtOpenFile = pTable->ServiceTableBase[nNtOpenFileIndex];
		KdPrint(("pNtOpenFile: 0x%08X\n", pNtOpenFile));
		SetGlobalVeriable(enumNtOpenFile, pNtOpenFile);

		pNtCreateSection = pTable->ServiceTableBase[nNtCreateSectionIndex];
		KdPrint(("pNtCreateSection: 0x%08X\n", pNtCreateSection));
		SetGlobalVeriable(enumNtCreateSection, pNtCreateSection);

		pNtMapViewOfSection = pTable->ServiceTableBase[nNtMapViewOfSectionIndex];
		KdPrint(("pNtMapViewOfSection: 0x%08X\n", pNtMapViewOfSection));
		SetGlobalVeriable(enumNtMapViewOfSection, pNtMapViewOfSection);

		pNtDeleteKey = pTable->ServiceTableBase[nNtDeleteKeyIndex];
		KdPrint(("pNtDeleteKey: 0x%08X\n", pNtDeleteKey));
		SetGlobalVeriable(enumNtDeleteKey, pNtDeleteKey);

		pNtCreateKey = pTable->ServiceTableBase[nNtCreateKeyIndex];
		KdPrint(("pNtCreateKey: 0x%08X\n", pNtCreateKey));
		SetGlobalVeriable(enumNtCreateKey, pNtCreateKey);

		pNtSetValueKey = pTable->ServiceTableBase[nNtSetValueKeyIndex];
		KdPrint(("pNtSetValueKey: 0x%08X\n", pNtSetValueKey));
		SetGlobalVeriable(enumNtSetValueKey, pNtSetValueKey); 

		pNtDeleteValueKey = pTable->ServiceTableBase[nNtDeleteValueKeyIndex];
		KdPrint(("pNtDeleteValueKey: 0x%08X\n", pNtDeleteValueKey));
		SetGlobalVeriable(enumNtDeleteValueKey, pNtDeleteValueKey); 

		pNtReadVirtualMemory = pTable->ServiceTableBase[nNtReadVirtualMemoryIndex];
		KdPrint(("pNtReadVirtualMemory: 0x%08X\n", pNtReadVirtualMemory));
		SetGlobalVeriable(enumNtReadVirtualMemory, pNtReadVirtualMemory); 

		pNtWriteVirtualMemory = pTable->ServiceTableBase[nNtWriteVirtualMemoryIndex];
		KdPrint(("pNtWriteVirtualMemory: 0x%08X\n", pNtWriteVirtualMemory));
		SetGlobalVeriable(enumNtWriteVirtualMemory, pNtWriteVirtualMemory); 

		pNtRenameKey = pTable->ServiceTableBase[nNtRenameKeyIndex];
		KdPrint(("pNtRenameKey: 0x%08X\n", pNtRenameKey));
		SetGlobalVeriable(enumNtRenameKey, pNtRenameKey); 

		pNtShutdownSystem = pTable->ServiceTableBase[nNtShutdownSystemIndex];
		KdPrint(("pNtShutdownSystem: 0x%08X\n", pNtShutdownSystem));
		SetGlobalVeriable(enumNtShutdownSystem, pNtShutdownSystem); 

		pNtCreateFile = pTable->ServiceTableBase[nNtCreateFileIndex];
		KdPrint(("NtCreateFile: 0x%08X\n", pNtCreateFile));
		SetGlobalVeriable(enumNtCreateFile, pNtCreateFile); 

		pNtReadFile = pTable->ServiceTableBase[nNtReadFileIndex];
		KdPrint(("NtReadFile: 0x%08X\n", pNtReadFile));
		SetGlobalVeriable(enumNtReadFile, pNtReadFile); 

		pNtWriteFile = pTable->ServiceTableBase[nNtWriteFileIndex];
		KdPrint(("NtWriteFile: 0x%08X\n", pNtWriteFile));
		SetGlobalVeriable(enumNtWriteFile, pNtWriteFile); 
	}
}

VOID InitCommonFunctions()
{
	ULONG NewBase = GetGlobalVeriable(enumNewKernelBase);
	ULONG OldBase = GetGlobalVeriable(enumOriginKernelBase);
	ULONG bReloadNtosKrnlSuccess = GetGlobalVeriable(enumReloadNtosKrnlSuccess);
	ULONG Base = bReloadNtosKrnlSuccess ? NewBase : OldBase;
	
	PVOID pMmIsAddressValid = NULL;
	PVOID pMmGetVirtualForPhysical = NULL;
	PVOID pMmGetPhysicalAddress = NULL;
	PVOID pObQueryNameString = NULL;
	PVOID pObfDereferenceObject = NULL;
	PVOID pIoVolumeDeviceToDosName = NULL;
	PVOID pIoGetCurrentProcess = NULL;
	PVOID pObOpenObjectByPointer = NULL;
	PVOID pObReferenceObjectByPointer = NULL;
	PVOID pPsLookupProcessByProcessId = NULL;
	PVOID pIoThreadToProcess = NULL;
	PVOID pExAllocatePoolWithTag = NULL;
	PVOID pExFreePoolWithTag = NULL;
	PVOID pKeSetEvent = NULL;
	PVOID pKeQueryActiveProcessors = NULL;
	PVOID pKeInitializeEvent = NULL;
	PVOID pKeInitializeDpc = NULL;
	PVOID pKeSetTargetProcessorDpc = NULL;
	PVOID pKeSetImportanceDpc = NULL;
	PVOID pKeInsertQueueDpc = NULL;
	PVOID pKeWaitForSingleObject = NULL;
	PVOID pKeStackAttachProcess = NULL;
	PVOID pKeUnstackDetachProcess = NULL;
	PVOID pProbeForRead = NULL;
	PVOID pObReferenceObjectByHandle = NULL;
	PVOID pObGetObjectType = NULL;
	PVOID pKeGetCurrentThread = NULL;
	PVOID pPsGetProcessId = NULL;
	PVOID pPsGetThreadId = NULL;
	PVOID pIoQueryFileDosDeviceName = NULL;
	PVOID pMmIsNonPagedSystemAddressValid = NULL;
	PVOID pIoAllocateMdl = NULL;
	PVOID pMmBuildMdlForNonPagedPool = NULL;
	PVOID pMmMapLockedPagesSpecifyCache = NULL;
	PVOID pMmUnmapLockedPages = NULL;
	PVOID pIoFreeMdl = NULL;
	PVOID pProbeForWrite = NULL;
	PVOID pIoCreateFile = NULL;
	PVOID pObMakeTemporaryObject = NULL;
	PVOID pPsTerminateSystemThread = NULL;
	PVOID pPsCreateSystemThread = NULL;
	PVOID pIoDeleteDevice = NULL;
	PVOID pObOpenObjectByName = NULL;
	PVOID pPsSetLoadImageNotifyRoutine = NULL;
	PVOID pPsSetCreateProcessNotifyRoutine = NULL;
	PVOID pPsSetCreateThreadNotifyRoutine = NULL;
	PVOID pPsRemoveCreateThreadNotifyRoutine = NULL;
	PVOID pPsRemoveLoadImageNotifyRoutine = NULL;
	PVOID pIoRegisterShutdownNotification = NULL;
	PVOID pCmUnRegisterCallback = NULL;
	PVOID pIoUnregisterShutdownNotification = NULL;
	PVOID pKeUpdateSystemTime = NULL;
	PVOID pKeCancelTimer = NULL;
	PVOID pIoInitializeTimer = NULL;
	PVOID pIoStartTimer = NULL;
	PVOID pIoStopTimer = NULL;
	PVOID pExQueueWorkItem = NULL;
	PVOID pKe386IoSetAccessProcess = NULL;
	PVOID pIoGetDeviceObjectPointer = NULL;
	PVOID pObReferenceObjectByName = NULL;
	PVOID pRtlInitAnsiString = NULL;
	PVOID pRtlAnsiStringToUnicodeString = NULL;
	PVOID pRtlFreeUnicodeString = NULL;
	PVOID pPsGetCurrentProcessId = NULL;
	PVOID pIoFreeIrp = NULL;
 	PVOID pMmFlushImageSection = NULL;
	PVOID pIoCreateDriver = NULL;
	PVOID pObCreateObjectType = NULL;
	PVOID pObGetObjectSecurity = NULL;
	PVOID pObCreateObjectTypeEx = NULL;
	PVOID pIofCompleteRequest = NULL;
	PVOID pKeInitializeApc = NULL;
	PVOID pKeInsertQueueApc = NULL;
	PVOID pIoBuildDeviceIoControlRequest = NULL;
	PVOID pInterlockedIncrement = NULL;
	PVOID pInterlockedDecrement = NULL;
	PVOID pPsLookupThreadByThreadId = NULL;
	PVOID pRtlImageDirectoryEntryToData = NULL;
	PVOID pPsGetProcessCreateTimeQuadPart = NULL;

	CHAR szMmIsAddressValid[] = {'M','m','I','s','A','d','d','r','e','s','s','V','a','l','i','d','\0'};
	CHAR szMmGetVirtualForPhysical[] = {'M','m','G','e','t','V','i','r','t','u','a','l','F','o','r','P','h','y','s','i','c','a','l','\0'};
	CHAR szMmGetPhysicalAddress[] = {'M','m','G','e','t','P','h','y','s','i','c','a','l','A','d','d','r','e','s','s','\0'};
	CHAR szObQueryNameString[] = {'O','b','Q','u','e','r','y','N','a','m','e','S','t','r','i','n','g','\0'};
	CHAR szObfDereferenceObject[] = {'O','b','f','D','e','r','e','f','e','r','e','n','c','e','O','b','j','e','c','t','\0'};
	CHAR szIoVolumeDeviceToDosName[] = {'I','o','V','o','l','u','m','e','D','e','v','i','c','e','T','o','D','o','s','N','a','m','e','\0'};
	CHAR szRtlVolumeDeviceToDosName[] = {'R','t','l','V','o','l','u','m','e','D','e','v','i','c','e','T','o','D','o','s','N','a','m','e','\0'};
	CHAR szIoGetCurrentProcess[] = {'I','o','G','e','t','C','u','r','r','e','n','t','P','r','o','c','e','s','s','\0'};
	CHAR szObOpenObjectByPointer[] = {'O','b','O','p','e','n','O','b','j','e','c','t','B','y','P','o','i','n','t','e','r','\0'};
	CHAR szObReferenceObjectByPointer[] = {'O','b','R','e','f','e','r','e','n','c','e','O','b','j','e','c','t','B','y','P','o','i','n','t','e','r','\0'};
	CHAR szPsLookupProcessByProcessId[] = {'P','s','L','o','o','k','u','p','P','r','o','c','e','s','s','B','y','P','r','o','c','e','s','s','I','d','\0'};
	CHAR szIoThreadToProcess[] = {'I','o','T','h','r','e','a','d','T','o','P','r','o','c','e','s','s','\0'};
	CHAR szExAllocatePoolWithTag[] = {'E','x','A','l','l','o','c','a','t','e','P','o','o','l','W','i','t','h','T','a','g','\0'};
	CHAR szExFreePoolWithTag[] = {'E','x','F','r','e','e','P','o','o','l','W','i','t','h','T','a','g','\0'};
	CHAR szKeSetEvent[] = {'K','e','S','e','t','E','v','e','n','t','\0'};
	CHAR szKeQueryActiveProcessors[] = {'K','e','Q','u','e','r','y','A','c','t','i','v','e','P','r','o','c','e','s','s','o','r','s','\0'};
	CHAR szKeInitializeEvent[] = {'K','e','I','n','i','t','i','a','l','i','z','e','E','v','e','n','t','\0'};
	CHAR szKeInitializeDpc[] = {'K','e','I','n','i','t','i','a','l','i','z','e','D','p','c','\0'};
	CHAR szKeSetTargetProcessorDpc[] = {'K','e','S','e','t','T','a','r','g','e','t','P','r','o','c','e','s','s','o','r','D','p','c','\0'};
	CHAR szKeSetImportanceDpc[] = {'K','e','S','e','t','I','m','p','o','r','t','a','n','c','e','D','p','c','\0'};
	CHAR szKeInsertQueueDpc[] = {'K','e','I','n','s','e','r','t','Q','u','e','u','e','D','p','c','\0'};
	CHAR szKeWaitForSingleObject[] = {'K','e','W','a','i','t','F','o','r','S','i','n','g','l','e','O','b','j','e','c','t','\0'};
	CHAR szKeStackAttachProcess[] = {'K','e','S','t','a','c','k','A','t','t','a','c','h','P','r','o','c','e','s','s','\0'};
	CHAR szKeUnstackDetachProcess[] = {'K','e','U','n','s','t','a','c','k','D','e','t','a','c','h','P','r','o','c','e','s','s','\0'};
	CHAR szProbeForRead[] = {'P','r','o','b','e','F','o','r','R','e','a','d','\0'};
	CHAR szObReferenceObjectByHandle[] = {'O','b','R','e','f','e','r','e','n','c','e','O','b','j','e','c','t','B','y','H','a','n','d','l','e','\0'};
	CHAR szObGetObjectType[] = {'O','b','G','e','t','O','b','j','e','c','t','T','y','p','e','\0'};
	CHAR szKeGetCurrentThread[] = {'K','e','G','e','t','C','u','r','r','e','n','t','T','h','r','e','a','d','\0'};
	CHAR szPsGetProcessId[] = {'P','s','G','e','t','P','r','o','c','e','s','s','I','d','\0'};
	CHAR szPsGetThreadId[] = {'P','s','G','e','t','T','h','r','e','a','d','I','d','\0'};
	CHAR szIoQueryFileDosDeviceName[] = {'I','o','Q','u','e','r','y','F','i','l','e','D','o','s','D','e','v','i','c','e','N','a','m','e','\0'};
	CHAR szMmIsNonPagedSystemAddressValid[] = {'M','m','I','s','N','o','n','P','a','g','e','d','S','y','s','t','e','m','A','d','d','r','e','s','s','V','a','l','i','d','\0'};
	CHAR szIoAllocateMdl[] = {'I','o','A','l','l','o','c','a','t','e','M','d','l','\0'};
	CHAR szMmBuildMdlForNonPagedPool[] = {'M','m','B','u','i','l','d','M','d','l','F','o','r','N','o','n','P','a','g','e','d','P','o','o','l','\0'};
	CHAR szMmMapLockedPagesSpecifyCache[] = {'M','m','M','a','p','L','o','c','k','e','d','P','a','g','e','s','S','p','e','c','i','f','y','C','a','c','h','e','\0'};
	CHAR szMmUnmapLockedPages[] = {'M','m','U','n','m','a','p','L','o','c','k','e','d','P','a','g','e','s','\0'};
	CHAR szIoFreeMdl[] = {'I','o','F','r','e','e','M','d','l','\0'};
	CHAR szProbeForWrite[] = {'P','r','o','b','e','F','o','r','W','r','i','t','e','\0'};
	CHAR szIoCreateFile[] = {'I','o','C','r','e','a','t','e','F','i','l','e','\0'};
	CHAR szObMakeTemporaryObject[] = {'O','b','M','a','k','e','T','e','m','p','o','r','a','r','y','O','b','j','e','c','t','\0'};
	CHAR szPsTerminateSystemThread[] = {'P','s','T','e','r','m','i','n','a','t','e','S','y','s','t','e','m','T','h','r','e','a','d','\0'};
	CHAR szPsCreateSystemThread[] = {'P','s','C','r','e','a','t','e','S','y','s','t','e','m','T','h','r','e','a','d','\0'};
	CHAR szIoDeleteDevice[] = {'I','o','D','e','l','e','t','e','D','e','v','i','c','e','\0'};
	CHAR szObOpenObjectByName[] = {'O','b','O','p','e','n','O','b','j','e','c','t','B','y','N','a','m','e','\0'};
	CHAR szPsSetCreateProcessNotifyRoutine[] = {'P','s','S','e','t','C','r','e','a','t','e','P','r','o','c','e','s','s','N','o','t','i','f','y','R','o','u','t','i','n','e','\0'};
	CHAR szPsSetLoadImageNotifyRoutine[] = {'P','s','S','e','t','L','o','a','d','I','m','a','g','e','N','o','t','i','f','y','R','o','u','t','i','n','e','\0'};
	CHAR szPsSetCreateThreadNotifyRoutine[] = {'P','s','S','e','t','C','r','e','a','t','e','T','h','r','e','a','d','N','o','t','i','f','y','R','o','u','t','i','n','e','\0'};
	CHAR szPsRemoveCreateThreadNotifyRoutine[] = {'P','s','R','e','m','o','v','e','C','r','e','a','t','e','T','h','r','e','a','d','N','o','t','i','f','y','R','o','u','t','i','n','e','\0'};
	CHAR szPsRemoveLoadImageNotifyRoutine[] = {'P','s','R','e','m','o','v','e','L','o','a','d','I','m','a','g','e','N','o','t','i','f','y','R','o','u','t','i','n','e','\0'};
	CHAR szIoRegisterShutdownNotification[] = {'I','o','R','e','g','i','s','t','e','r','S','h','u','t','d','o','w','n','N','o','t','i','f','i','c','a','t','i','o','n','\0'};
	CHAR szCmUnRegisterCallback[] = {'C','m','U','n','R','e','g','i','s','t','e','r','C','a','l','l','b','a','c','k','\0'};
	CHAR szIoUnregisterShutdownNotification[] = {'I','o','U','n','r','e','g','i','s','t','e','r','S','h','u','t','d','o','w','n','N','o','t','i','f','i','c','a','t','i','o','n','\0'};
	CHAR szKeUpdateSystemTime[] = {'K','e','U','p','d','a','t','e','S','y','s','t','e','m','T','i','m','e','\0'};
	CHAR szKeCancelTimer[] = {'K','e','C','a','n','c','e','l','T','i','m','e','r','\0'};
	CHAR szIoInitializeTimer[] = {'I','o','I','n','i','t','i','a','l','i','z','e','T','i','m','e','r','\0'};
	CHAR szIoStartTimer[] = {'I','o','S','t','a','r','t','T','i','m','e','r','\0'};
	CHAR szIoStopTimer[] = {'I','o','S','t','o','p','T','i','m','e','r','\0'};
	CHAR szExQueueWorkItem[] = {'E','x','Q','u','e','u','e','W','o','r','k','I','t','e','m','\0'};
	CHAR szKe386IoSetAccessProcess[] = {'K','e','3','8','6','I','o','S','e','t','A','c','c','e','s','s','P','r','o','c','e','s','s','\0'};
	CHAR szIoGetDeviceObjectPointer[] = {'I','o','G','e','t','D','e','v','i','c','e','O','b','j','e','c','t','P','o','i','n','t','e','r','\0'};
	CHAR szObReferenceObjectByName[] = {'O','b','R','e','f','e','r','e','n','c','e','O','b','j','e','c','t','B','y','N','a','m','e','\0'};
	CHAR szRtlInitAnsiString[] = {'R','t','l','I','n','i','t','A','n','s','i','S','t','r','i','n','g','\0'};
	CHAR szRtlAnsiStringToUnicodeString[] = {'R','t','l','A','n','s','i','S','t','r','i','n','g','T','o','U','n','i','c','o','d','e','S','t','r','i','n','g','\0'};
	CHAR szRtlFreeUnicodeString[] = {'R','t','l','F','r','e','e','U','n','i','c','o','d','e','S','t','r','i','n','g','\0'};
	CHAR szPsGetCurrentProcessId[] = {'P','s','G','e','t','C','u','r','r','e','n','t','P','r','o','c','e','s','s','I','d','\0'};
	CHAR szIoFreeIrp[] = {'I','o','F','r','e','e','I','r','p','\0'};
	CHAR szMmFlushImageSection[] = {'M','m','F','l','u','s','h','I','m','a','g','e','S','e','c','t','i','o','n','\0'};
	CHAR szIoCreateDriver[] = {'I','o','C','r','e','a','t','e','D','r','i','v','e','r','\0'};
	CHAR szObGetObjectSecurity[] = {'O','b','G','e','t','O','b','j','e','c','t','S','e','c','u','r','i','t','y','\0'};
	CHAR szObCreateObjectType[] = {'O','b','C','r','e','a','t','e','O','b','j','e','c','t','T','y','p','e','\0'};
	CHAR szIofCompleteRequest[] = {'I','o','f','C','o','m','p','l','e','t','e','R','e','q','u','e','s','t','\0'};
	CHAR szKeInitializeApc[] = {'K','e','I','n','i','t','i','a','l','i','z','e','A','p','c','\0'};
	CHAR szKeInsertQueueApc[] = {'K','e','I','n','s','e','r','t','Q','u','e','u','e','A','p','c','\0'};
	CHAR szIoBuildDeviceIoControlRequest[] = {'I','o','B','u','i','l','d','D','e','v','i','c','e','I','o','C','o','n','t','r','o','l','R','e','q','u','e','s','t','\0'};
	CHAR szInterlockedIncrement[] = {'_','I','n','t','e','r','l','o','c','k','e','d','I','n','c','r','e','m','e','n','t','\0'};
	CHAR szInterlockedDecrement[] = {'_','I','n','t','e','r','l','o','c','k','e','d','D','e','c','r','e','m','e','n','t','\0'};
	CHAR szInterlockedIncrement1[] = {'I','n','t','e','r','l','o','c','k','e','d','I','n','c','r','e','m','e','n','t','\0'};
	CHAR szInterlockedDecrement1[] = {'I','n','t','e','r','l','o','c','k','e','d','D','e','c','r','e','m','e','n','t','\0'};
	CHAR szPsLookupThreadByThreadId[] = {'P','s','L','o','o','k','u','p','T','h','r','e','a','d','B','y','T','h','r','e','a','d','I','d','\0'};
	CHAR szRtlImageDirectoryEntryToData[] = {'R','t','l','I','m','a','g','e','D','i','r','e','c','t','o','r','y','E','n','t','r','y','T','o','D','a','t','a','\0'};
	CHAR szPsGetProcessCreateTimeQuadPart[] = {'P','s','G','e','t','P','r','o','c','e','s','s','C','r','e','a','t','e','T','i','m','e','Q','u','a','d','P','a','r','t','\0'};

	KdPrint(("Base: 0x%08X\n", Base));
	pMmIsAddressValid = FindExportedRoutineByName((PVOID)Base, /*"MmIsAddressValid"*/szMmIsAddressValid);
	KdPrint(("pMmIsAddressValid: 0x%08X\n", pMmIsAddressValid));
	SetGlobalVeriable(enumMmIsAddressValid, (ULONG)pMmIsAddressValid);

	pMmGetVirtualForPhysical = FindExportedRoutineByName((PVOID)Base, /*"MmGetVirtualForPhysical"*/szMmGetVirtualForPhysical);
	KdPrint(("pMmGetVirtualForPhysical: 0x%08X\n", pMmGetVirtualForPhysical));
	SetGlobalVeriable(enumMmGetVirtualForPhysical, (ULONG)pMmGetVirtualForPhysical);

	pMmGetPhysicalAddress = FindExportedRoutineByName((PVOID)Base, /*"MmGetPhysicalAddress"*/szMmGetPhysicalAddress);
	KdPrint(("pMmGetPhysicalAddress: 0x%08X\n", pMmGetPhysicalAddress));
	SetGlobalVeriable(enumMmGetPhysicalAddress, (ULONG)pMmGetPhysicalAddress);

	pObQueryNameString = FindExportedRoutineByName((PVOID)Base, /*"ObQueryNameString"*/szObQueryNameString);
	KdPrint(("pObQueryNameString: 0x%08X\n", pObQueryNameString));
	SetGlobalVeriable(enumObQueryNameString, (ULONG)pObQueryNameString);

	pObfDereferenceObject = FindExportedRoutineByName((PVOID)Base, /*"ObfDereferenceObject"*/szObfDereferenceObject);
	KdPrint(("pObfDereferenceObject: 0x%08X\n", pObfDereferenceObject));
	SetGlobalVeriable(enumObfDereferenceObject, (ULONG)pObfDereferenceObject);

	pIoVolumeDeviceToDosName = FindExportedRoutineByName((PVOID)Base, /*"IoVolumeDeviceToDosName"*/szIoVolumeDeviceToDosName);
	if (!pIoVolumeDeviceToDosName)
	{
		pIoVolumeDeviceToDosName = FindExportedRoutineByName((PVOID)Base, /*"RtlVolumeDeviceToDosName"*/szRtlVolumeDeviceToDosName);
	}
	KdPrint(("pIoVolumeDeviceToDosName: 0x%08X\n", pIoVolumeDeviceToDosName));
	SetGlobalVeriable(enumIoVolumeDeviceToDosName, (ULONG)pIoVolumeDeviceToDosName);

	pIoGetCurrentProcess = FindExportedRoutineByName((PVOID)Base, /*"IoGetCurrentProcess"*/szIoGetCurrentProcess);
	KdPrint(("pIoGetCurrentProcess: 0x%08X\n", pIoGetCurrentProcess));
	SetGlobalVeriable(enumIoGetCurrentProcess, (ULONG)pIoGetCurrentProcess);

	pObOpenObjectByPointer = FindExportedRoutineByName((PVOID)Base, /*"ObOpenObjectByPointer"*/szObOpenObjectByPointer);
	KdPrint(("pObOpenObjectByPointer: 0x%08X\n", pObOpenObjectByPointer));
	SetGlobalVeriable(enumObOpenObjectByPointer, (ULONG)pObOpenObjectByPointer);

	pObReferenceObjectByPointer = FindExportedRoutineByName((PVOID)Base, /*"ObReferenceObjectByPointer"*/szObReferenceObjectByPointer);
	KdPrint(("pObReferenceObjectByPointer: 0x%08X\n", pObReferenceObjectByPointer));
	SetGlobalVeriable(enumObReferenceObjectByPointer, (ULONG)pObReferenceObjectByPointer);

	pPsLookupProcessByProcessId = FindExportedRoutineByName((PVOID)Base, /*"PsLookupProcessByProcessId"*/szPsLookupProcessByProcessId);
	KdPrint(("pPsLookupProcessByProcessId: 0x%08X\n", pPsLookupProcessByProcessId));
	SetGlobalVeriable(enumPsLookupProcessByProcessId, (ULONG)pPsLookupProcessByProcessId);

	pIoThreadToProcess = FindExportedRoutineByName((PVOID)Base, /*"IoThreadToProcess"*/szIoThreadToProcess);
	KdPrint(("pIoThreadToProcess: 0x%08X\n", pIoThreadToProcess));
	SetGlobalVeriable(enumIoThreadToProcess, (ULONG)pIoThreadToProcess);

	pExAllocatePoolWithTag = FindExportedRoutineByName((PVOID)Base, /*"ExAllocatePoolWithTag"*/szExAllocatePoolWithTag);
	KdPrint(("pExAllocatePoolWithTag: 0x%08X\n", pExAllocatePoolWithTag));
	SetGlobalVeriable(enumExAllocatePoolWithTag, (ULONG)pExAllocatePoolWithTag);

	pExFreePoolWithTag = FindExportedRoutineByName((PVOID)Base, /*"ExFreePoolWithTag"*/szExFreePoolWithTag);
	KdPrint(("pExFreePoolWithTag: 0x%08X\n", pExFreePoolWithTag));
	SetGlobalVeriable(enumExFreePoolWithTag, (ULONG)pExFreePoolWithTag);

	pKeSetEvent = FindExportedRoutineByName((PVOID)Base, /*"KeSetEvent"*/szKeSetEvent);
	KdPrint(("pKeSetEvent: 0x%08X\n", pKeSetEvent));
	SetGlobalVeriable(enumKeSetEvent, (ULONG)pKeSetEvent);

	pKeQueryActiveProcessors = FindExportedRoutineByName((PVOID)Base, /*"KeQueryActiveProcessors"*/szKeQueryActiveProcessors);
	KdPrint(("pKeQueryActiveProcessors: 0x%08X\n", pKeQueryActiveProcessors));
	SetGlobalVeriable(enumKeQueryActiveProcessors, (ULONG)pKeQueryActiveProcessors);

	pKeInitializeEvent = FindExportedRoutineByName((PVOID)Base, /*"KeInitializeEvent"*/szKeInitializeEvent);
	KdPrint(("pKeInitializeEvent: 0x%08X\n", pKeInitializeEvent));
	SetGlobalVeriable(enumKeInitializeEvent, (ULONG)pKeInitializeEvent);

	pKeInitializeDpc = FindExportedRoutineByName((PVOID)Base, /*"KeInitializeDpc"*/szKeInitializeDpc);
	KdPrint(("pKeInitializeDpc: 0x%08X\n", pKeInitializeDpc));
	SetGlobalVeriable(enumKeInitializeDpc, (ULONG)pKeInitializeDpc);

	pKeSetTargetProcessorDpc = FindExportedRoutineByName((PVOID)Base, /*"KeSetTargetProcessorDpc"*/szKeSetTargetProcessorDpc);
	KdPrint(("pKeSetTargetProcessorDpc: 0x%08X\n", pKeSetTargetProcessorDpc));
	SetGlobalVeriable(enumKeSetTargetProcessorDpc, (ULONG)pKeSetTargetProcessorDpc);

	pKeSetImportanceDpc = FindExportedRoutineByName((PVOID)Base, /*"KeSetImportanceDpc"*/szKeSetImportanceDpc);
	KdPrint(("pKeSetImportanceDpc: 0x%08X\n", pKeSetImportanceDpc));
	SetGlobalVeriable(enumKeSetImportanceDpc, (ULONG)pKeSetImportanceDpc);

	pKeInsertQueueDpc = FindExportedRoutineByName((PVOID)Base, /*"KeInsertQueueDpc"*/szKeInsertQueueDpc);
	KdPrint(("pKeInsertQueueDpc: 0x%08X\n", pKeInsertQueueDpc));
	SetGlobalVeriable(enumKeInsertQueueDpc, (ULONG)pKeInsertQueueDpc);

	pKeWaitForSingleObject = FindExportedRoutineByName((PVOID)Base, /*"KeWaitForSingleObject"*/szKeWaitForSingleObject);
	KdPrint(("pKeWaitForSingleObject: 0x%08X\n", pKeWaitForSingleObject));
	SetGlobalVeriable(enumKeWaitForSingleObject, (ULONG)pKeWaitForSingleObject);

	pKeStackAttachProcess = FindExportedRoutineByName((PVOID)Base, /*"KeStackAttachProcess"*/szKeStackAttachProcess);
	KdPrint(("pKeStackAttachProcess: 0x%08X\n", pKeStackAttachProcess));
	SetGlobalVeriable(enumKeStackAttachProcess, (ULONG)pKeStackAttachProcess);

	pKeUnstackDetachProcess = FindExportedRoutineByName((PVOID)Base, /*"KeUnstackDetachProcess"*/szKeUnstackDetachProcess);
	KdPrint(("pKeUnstackDetachProcess: 0x%08X\n", pKeUnstackDetachProcess));
	SetGlobalVeriable(enumKeUnstackDetachProcess, (ULONG)pKeUnstackDetachProcess);

	pProbeForRead = FindExportedRoutineByName((PVOID)Base, /*"ProbeForRead"*/szProbeForRead);
	KdPrint(("pProbeForRead: 0x%08X\n", pProbeForRead));
	SetGlobalVeriable(enumProbeForRead, (ULONG)pProbeForRead);

	pObReferenceObjectByHandle = FindExportedRoutineByName((PVOID)Base, /*"ObReferenceObjectByHandle"*/szObReferenceObjectByHandle);
	KdPrint(("pObReferenceObjectByHandle: 0x%08X\n", pObReferenceObjectByHandle));
	SetGlobalVeriable(enumObReferenceObjectByHandle, (ULONG)pObReferenceObjectByHandle);

	pObGetObjectType = FindExportedRoutineByName((PVOID)Base, /*"ObGetObjectType"*/szObGetObjectType);
	KdPrint(("pObGetObjectType: 0x%08X\n", pObGetObjectType));
	SetGlobalVeriable(enumObGetObjectType, (ULONG)pObGetObjectType);

	pKeGetCurrentThread = FindExportedRoutineByName((PVOID)Base, /*"KeGetCurrentThread"*/szKeGetCurrentThread);
	KdPrint(("pKeGetCurrentThread: 0x%08X\n", pKeGetCurrentThread));
	SetGlobalVeriable(enumPsGetCurrentThread, (ULONG)pKeGetCurrentThread);

	pPsGetProcessId = FindExportedRoutineByName((PVOID)Base, /*"PsGetProcessId"*/szPsGetProcessId);
	KdPrint(("pPsGetProcessId: 0x%08X\n", pPsGetProcessId));
	SetGlobalVeriable(enumPsGetProcessId, (ULONG)pPsGetProcessId);

	pPsGetThreadId = FindExportedRoutineByName((PVOID)Base,/* "PsGetThreadId"*/szPsGetThreadId);
	KdPrint(("pPsGetThreadId: 0x%08X\n", pPsGetThreadId));
	SetGlobalVeriable(enumPsGetThreadId, (ULONG)pPsGetThreadId);

	pIoQueryFileDosDeviceName = FindExportedRoutineByName((PVOID)Base, /*"IoQueryFileDosDeviceName"*/szIoQueryFileDosDeviceName);
	KdPrint(("pIoQueryFileDosDeviceName: 0x%08X\n", pIoQueryFileDosDeviceName));
	SetGlobalVeriable(enumIoQueryFileDosDeviceName, (ULONG)pIoQueryFileDosDeviceName);

	pMmIsNonPagedSystemAddressValid = FindExportedRoutineByName((PVOID)Base, /*"MmIsNonPagedSystemAddressValid"*/szMmIsNonPagedSystemAddressValid);
	KdPrint(("pMmIsNonPagedSystemAddressValid: 0x%08X\n", pMmIsNonPagedSystemAddressValid));
	SetGlobalVeriable(enumMmIsNonPagedSystemAddressValid, (ULONG)pMmIsNonPagedSystemAddressValid);

	pIoAllocateMdl = FindExportedRoutineByName((PVOID)Base, /*"IoAllocateMdl"*/szIoAllocateMdl);
	KdPrint(("pIoAllocateMdl: 0x%08X\n", pIoAllocateMdl));
	SetGlobalVeriable(enumIoAllocateMdl, (ULONG)pIoAllocateMdl);

	pMmBuildMdlForNonPagedPool = FindExportedRoutineByName((PVOID)Base, /*"MmBuildMdlForNonPagedPool"*/szMmBuildMdlForNonPagedPool);
	KdPrint(("pMmBuildMdlForNonPagedPool: 0x%08X\n", pMmBuildMdlForNonPagedPool));
	SetGlobalVeriable(enumMmBuildMdlForNonPagedPool, (ULONG)pMmBuildMdlForNonPagedPool);

	pMmMapLockedPagesSpecifyCache = FindExportedRoutineByName((PVOID)Base, /*"MmMapLockedPagesSpecifyCache"*/szMmMapLockedPagesSpecifyCache);
	KdPrint(("pMmMapLockedPagesSpecifyCache: 0x%08X\n", pMmMapLockedPagesSpecifyCache));
	SetGlobalVeriable(enumMmMapLockedPagesSpecifyCache, (ULONG)pMmMapLockedPagesSpecifyCache);

	pMmUnmapLockedPages = FindExportedRoutineByName((PVOID)Base, /*"MmUnmapLockedPages"*/szMmUnmapLockedPages);
	KdPrint(("pMmUnmapLockedPages: 0x%08X\n", pMmUnmapLockedPages));
	SetGlobalVeriable(enumMmUnmapLockedPages, (ULONG)pMmUnmapLockedPages);

	pIoFreeMdl = FindExportedRoutineByName((PVOID)Base, /*"IoFreeMdl"*/szIoFreeMdl);
	KdPrint(("pIoFreeMdl: 0x%08X\n", pIoFreeMdl));
	SetGlobalVeriable(enumIoFreeMdl, (ULONG)pIoFreeMdl);

	pProbeForWrite = FindExportedRoutineByName((PVOID)Base, /*"ProbeForWrite"*/szProbeForWrite);
	KdPrint(("pProbeForWrite: 0x%08X\n", pProbeForWrite));
	SetGlobalVeriable(enumProbeForWrite, (ULONG)pProbeForWrite);

	pIoCreateFile = FindExportedRoutineByName((PVOID)Base, /*"IoCreateFile"*/szIoCreateFile);
	KdPrint(("pIoCreateFile: 0x%08X\n", pIoCreateFile));
	SetGlobalVeriable(enumIoCreateFile, (ULONG)pIoCreateFile);

	pObMakeTemporaryObject = FindExportedRoutineByName((PVOID)Base, /*"ObMakeTemporaryObject"*/szObMakeTemporaryObject);
	KdPrint(("pObMakeTemporaryObject: 0x%08X\n", pObMakeTemporaryObject));
	SetGlobalVeriable(enumObMakeTemporaryObject, (ULONG)pObMakeTemporaryObject);

	pPsTerminateSystemThread = FindExportedRoutineByName((PVOID)Base, /*"PsTerminateSystemThread"*/szPsTerminateSystemThread);
	KdPrint(("pPsTerminateSystemThread: 0x%08X\n", pPsTerminateSystemThread));
	SetGlobalVeriable(enumPsTerminateSystemThread, (ULONG)pPsTerminateSystemThread);

	pPsCreateSystemThread = FindExportedRoutineByName((PVOID)Base, /*"PsCreateSystemThread"*/szPsCreateSystemThread);
	KdPrint(("pPsCreateSystemThread: 0x%08X\n", pPsCreateSystemThread));
	SetGlobalVeriable(enumPsCreateSystemThread, (ULONG)pPsCreateSystemThread);

	pIoDeleteDevice = FindExportedRoutineByName((PVOID)Base, /*"IoDeleteDevice"*/szIoDeleteDevice);
	KdPrint(("pIoDeleteDevice: 0x%08X\n", pIoDeleteDevice));
	SetGlobalVeriable(enumIoDeleteDevice, (ULONG)pIoDeleteDevice);

	pObOpenObjectByName = FindExportedRoutineByName((PVOID)Base, /*"ObOpenObjectByName"*/szObOpenObjectByName);
	KdPrint(("pObOpenObjectByName: 0x%08X\n", pObOpenObjectByName));
	SetGlobalVeriable(enumObOpenObjectByName, (ULONG)pObOpenObjectByName);

	pPsSetCreateProcessNotifyRoutine = FindExportedRoutineByName((PVOID)Base, /*"PsSetCreateProcessNotifyRoutine"*/szPsSetCreateProcessNotifyRoutine);
	KdPrint(("pPsSetCreateProcessNotifyRoutine: 0x%08X\n", pPsSetCreateProcessNotifyRoutine));
	SetGlobalVeriable(enumPsSetCreateProcessNotifyRoutine, (ULONG)pPsSetCreateProcessNotifyRoutine);

	pPsSetLoadImageNotifyRoutine = FindExportedRoutineByName((PVOID)Base, /*"PsSetLoadImageNotifyRoutine"*/szPsSetLoadImageNotifyRoutine);
	KdPrint(("pPsSetLoadImageNotifyRoutine: 0x%08X\n", pPsSetLoadImageNotifyRoutine));
	SetGlobalVeriable(enumPsSetLoadImageNotifyRoutine, (ULONG)pPsSetLoadImageNotifyRoutine);

	pPsSetCreateThreadNotifyRoutine = FindExportedRoutineByName((PVOID)Base, /*"PsSetCreateThreadNotifyRoutine"*/szPsSetCreateThreadNotifyRoutine);
	KdPrint(("pPsSetCreateThreadNotifyRoutine: 0x%08X\n", pPsSetCreateThreadNotifyRoutine));
	SetGlobalVeriable(enumPsSetCreateThreadNotifyRoutine, (ULONG)pPsSetCreateThreadNotifyRoutine);

	pPsRemoveCreateThreadNotifyRoutine = FindExportedRoutineByName((PVOID)Base, /*"PsRemoveCreateThreadNotifyRoutine"*/szPsRemoveCreateThreadNotifyRoutine);
	KdPrint(("pPsRemoveCreateThreadNotifyRoutine: 0x%08X\n", pPsRemoveCreateThreadNotifyRoutine));
	SetGlobalVeriable(enumPsRemoveCreateThreadNotifyRoutine, (ULONG)pPsRemoveCreateThreadNotifyRoutine);

	pPsRemoveLoadImageNotifyRoutine = FindExportedRoutineByName((PVOID)Base, /*"PsRemoveLoadImageNotifyRoutine"*/szPsRemoveLoadImageNotifyRoutine);
	KdPrint(("pPsRemoveLoadImageNotifyRoutine: 0x%08X\n", pPsRemoveLoadImageNotifyRoutine));
	SetGlobalVeriable(enumPsRemoveLoadImageNotifyRoutine, (ULONG)pPsRemoveLoadImageNotifyRoutine);

	pIoRegisterShutdownNotification = FindExportedRoutineByName((PVOID)Base, /*"IoRegisterShutdownNotification"*/szIoRegisterShutdownNotification);
	KdPrint(("pIoRegisterShutdownNotification: 0x%08X\n", pIoRegisterShutdownNotification));
	SetGlobalVeriable(enumIoRegisterShutdownNotification, (ULONG)pIoRegisterShutdownNotification);

	pCmUnRegisterCallback = FindExportedRoutineByName((PVOID)Base, /*"CmUnRegisterCallback"*/szCmUnRegisterCallback);
	KdPrint(("pCmUnRegisterCallback: 0x%08X\n", pCmUnRegisterCallback));
	SetGlobalVeriable(enumCmUnRegisterCallback, (ULONG)pCmUnRegisterCallback);

	pIoUnregisterShutdownNotification = FindExportedRoutineByName((PVOID)Base, /*"IoUnregisterShutdownNotification"*/szIoUnregisterShutdownNotification);
	KdPrint(("pIoUnregisterShutdownNotification: 0x%08X\n", pIoUnregisterShutdownNotification));
	SetGlobalVeriable(enumIoUnregisterShutdownNotification, (ULONG)pIoUnregisterShutdownNotification);

	pKeUpdateSystemTime = FindExportedRoutineByName((PVOID)Base, /*"KeUpdateSystemTime"*/szKeUpdateSystemTime);
	KdPrint(("pKeUpdateSystemTime: 0x%08X\n", pKeUpdateSystemTime));
	SetGlobalVeriable(enumKeUpdateSystemTime, (ULONG)pKeUpdateSystemTime);

	pKeCancelTimer = FindExportedRoutineByName((PVOID)Base, /*"KeCancelTimer"*/szKeCancelTimer);
	KdPrint(("pKeCancelTimer: 0x%08X\n", pKeCancelTimer));
	SetGlobalVeriable(enumKeCancelTimer, (ULONG)pKeCancelTimer);

	pIoInitializeTimer = FindExportedRoutineByName((PVOID)Base, /*"IoInitializeTimer"*/szIoInitializeTimer);
	KdPrint(("pIoInitializeTimer: 0x%08X\n", pIoInitializeTimer));
	SetGlobalVeriable(enumIoInitializeTimer, (ULONG)pIoInitializeTimer);

	pIoStartTimer = FindExportedRoutineByName((PVOID)Base, /*"IoStartTimer"*/szIoStartTimer);
	KdPrint(("pIoStartTimer: 0x%08X\n", pIoStartTimer));
	SetGlobalVeriable(enumIoStartTimer, (ULONG)pIoStartTimer);

	pIoStopTimer = FindExportedRoutineByName((PVOID)Base, /*"IoStopTimer"*/szIoStopTimer);
	KdPrint(("pIoStopTimer: 0x%08X\n", pIoStopTimer));
	SetGlobalVeriable(enumIoStopTimer, (ULONG)pIoStopTimer);

	pExQueueWorkItem = FindExportedRoutineByName((PVOID)Base, /*"ExQueueWorkItem"*/szExQueueWorkItem);
	KdPrint(("pExQueueWorkItem: 0x%08X\n", pExQueueWorkItem));
	SetGlobalVeriable(enumExQueueWorkItem, (ULONG)pExQueueWorkItem);

	pKe386IoSetAccessProcess = FindExportedRoutineByName((PVOID)Base, /*"Ke386IoSetAccessProcess"*/szKe386IoSetAccessProcess);
	KdPrint(("pKe386IoSetAccessProcess: 0x%08X\n", pKe386IoSetAccessProcess));
	SetGlobalVeriable(enumKe386IoSetAccessProcess, (ULONG)pKe386IoSetAccessProcess);

	pIoGetDeviceObjectPointer = FindExportedRoutineByName((PVOID)Base, /*"IoGetDeviceObjectPointer"*/szIoGetDeviceObjectPointer);
	KdPrint(("pIoGetDeviceObjectPointer: 0x%08X\n", pIoGetDeviceObjectPointer));
	SetGlobalVeriable(enumIoGetDeviceObjectPointer, (ULONG)pIoGetDeviceObjectPointer);

	pObReferenceObjectByName = FindExportedRoutineByName((PVOID)Base, /*"ObReferenceObjectByName"*/szObReferenceObjectByName);
	KdPrint(("pObReferenceObjectByName: 0x%08X\n", pObReferenceObjectByName));
	SetGlobalVeriable(enumObReferenceObjectByName, (ULONG)pObReferenceObjectByName);

	pRtlInitAnsiString = FindExportedRoutineByName((PVOID)Base, /*"RtlInitAnsiString"*/szRtlInitAnsiString);
	KdPrint(("pRtlInitAnsiString: 0x%08X\n", pRtlInitAnsiString));
	SetGlobalVeriable(enumRtlInitAnsiString, (ULONG)pRtlInitAnsiString);

	pRtlAnsiStringToUnicodeString = FindExportedRoutineByName((PVOID)Base, /*"RtlAnsiStringToUnicodeString"*/szRtlAnsiStringToUnicodeString);
	KdPrint(("pRtlAnsiStringToUnicodeString: 0x%08X\n", pRtlAnsiStringToUnicodeString));
	SetGlobalVeriable(enumRtlAnsiStringToUnicodeString, (ULONG)pRtlAnsiStringToUnicodeString);

	pRtlFreeUnicodeString = FindExportedRoutineByName((PVOID)Base,/* "RtlFreeUnicodeString"*/szRtlFreeUnicodeString);
	KdPrint(("pRtlFreeUnicodeString: 0x%08X\n", pRtlFreeUnicodeString));
	SetGlobalVeriable(enumRtlFreeUnicodeString, (ULONG)pRtlFreeUnicodeString);

	pPsGetCurrentProcessId = FindExportedRoutineByName((PVOID)Base, /*"PsGetCurrentProcessId"*/szPsGetCurrentProcessId);
	KdPrint(("pPsGetCurrentProcessId: 0x%08X\n", pPsGetCurrentProcessId));
	SetGlobalVeriable(enumPsGetCurrentProcessId, (ULONG)pPsGetCurrentProcessId);

	pIoFreeIrp = FindExportedRoutineByName((PVOID)Base, /*"IoFreeIrp"*/szIoFreeIrp);
	KdPrint(("pIoFreeIrp: 0x%08X\n", pIoFreeIrp));
	SetGlobalVeriable(enumIoFreeIrp, (ULONG)pIoFreeIrp);

	pMmFlushImageSection = FindExportedRoutineByName((PVOID)Base, /*"MmFlushImageSection"*/szMmFlushImageSection);
	KdPrint(("pMmFlushImageSection: 0x%08X\n", pMmFlushImageSection));
	SetGlobalVeriable(enumMmFlushImageSection, (ULONG)pMmFlushImageSection);

	pIoCreateDriver = FindExportedRoutineByName((PVOID)Base, /*"IoCreateDriver"*/szIoCreateDriver);
	KdPrint(("pIoCreateDriver: 0x%08X\n", pIoCreateDriver));
	SetGlobalVeriable(enumIoCreateDriver, (ULONG)pIoCreateDriver);

	pObGetObjectSecurity = FindExportedRoutineByName((PVOID)Base, /*"ObGetObjectSecurity"*/szObGetObjectSecurity);
	KdPrint(("pObGetObjectSecurity: 0x%08X\n", pObGetObjectSecurity));
	SetGlobalVeriable(enumObGetObjectSecurity, (ULONG)pObGetObjectSecurity);

	pObCreateObjectType = FindExportedRoutineByName((PVOID)Base, /*"ObCreateObjectType"*/szObCreateObjectType);
	KdPrint(("pObCreateObjectType: 0x%08X\n", pObCreateObjectType));
	SetGlobalVeriable(enumObCreateObjectType, (ULONG)pObCreateObjectType);

	pIofCompleteRequest = FindExportedRoutineByName((PVOID)Base, /*"IofCompleteRequest"*/szIofCompleteRequest);
	KdPrint(("pIofCompleteRequest: 0x%08X\n", pIofCompleteRequest));
	SetGlobalVeriable(enumIofCompleteRequest, (ULONG)pIofCompleteRequest);

	pKeInitializeApc = FindExportedRoutineByName((PVOID)Base, /*"KeInitializeApc"*/szKeInitializeApc);
	KdPrint(("KeInitializeApc: 0x%08X\n", pKeInitializeApc));
	SetGlobalVeriable(enumKeInitializeApc, (ULONG)pKeInitializeApc);

	pKeInsertQueueApc = FindExportedRoutineByName((PVOID)Base, /*"KeInsertQueueApc"*/szKeInsertQueueApc);
	KdPrint(("pKeInsertQueueApc: 0x%08X\n", pKeInsertQueueApc));
	SetGlobalVeriable(enumKeInsertQueueApc, (ULONG)pKeInsertQueueApc);

	pIoBuildDeviceIoControlRequest = FindExportedRoutineByName((PVOID)Base, /*"KeInsertQueueApc"*/szIoBuildDeviceIoControlRequest);
	KdPrint(("pIoBuildDeviceIoControlRequest: 0x%08X\n", pIoBuildDeviceIoControlRequest));
	SetGlobalVeriable(enumIoBuildDeviceIoControlRequest, (ULONG)pIoBuildDeviceIoControlRequest);

	pInterlockedIncrement = FindExportedRoutineByName((PVOID)Base, szInterlockedIncrement);
	if (pInterlockedIncrement == NULL)
	{
		pInterlockedIncrement = FindExportedRoutineByName((PVOID)Base, szInterlockedIncrement1);
	}
	KdPrint(("InterlockedIncrement: 0x%08X\n", pInterlockedIncrement));
	SetGlobalVeriable(enumInterlockedIncrement, (ULONG)pInterlockedIncrement);

	pInterlockedDecrement = FindExportedRoutineByName((PVOID)Base, szInterlockedDecrement);
	if (pInterlockedDecrement == NULL)
	{
		pInterlockedDecrement = FindExportedRoutineByName((PVOID)Base, szInterlockedDecrement1);
	}
	KdPrint(("InterlockedDecrement: 0x%08X\n", pInterlockedDecrement));
	SetGlobalVeriable(enumInterlockedDecrement, (ULONG)pInterlockedDecrement);

	pPsLookupThreadByThreadId = FindExportedRoutineByName((PVOID)Base, szPsLookupThreadByThreadId);
	KdPrint(("PsLookupThreadByThreadId: 0x%08X\n", pPsLookupThreadByThreadId));
	SetGlobalVeriable(enumPsLookupThreadByThreadId, (ULONG)pPsLookupThreadByThreadId);

	pRtlImageDirectoryEntryToData = FindExportedRoutineByName((PVOID)Base, szRtlImageDirectoryEntryToData);
	KdPrint(("RtlImageDirectoryEntryToData: 0x%08X\n", pRtlImageDirectoryEntryToData));
	SetGlobalVeriable(enumRtlImageDirectoryEntryToData, (ULONG)pRtlImageDirectoryEntryToData);

	pPsGetProcessCreateTimeQuadPart = FindExportedRoutineByName((PVOID)Base, szPsGetProcessCreateTimeQuadPart);
	KdPrint(("PsGetProcessCreateTimeQuadPart: 0x%08X\n", pPsGetProcessCreateTimeQuadPart));
	SetGlobalVeriable(enumPsGetProcessCreateTimeQuadPart, (ULONG)pPsGetProcessCreateTimeQuadPart);
	
}

/*************************************************
Returns:   NULL
Qualifier: ��ȡ����ģ��Ļ�ַ�ʹ�С
*************************************************/
void GetCommonDriversSizeAndBase()
{
	PKLDR_DATA_TABLE_ENTRY entry = NULL, firstentry = NULL;
	WCHAR szNtfs[] = {'n','t','f','s','.','s','y','s','\0'};
	WCHAR szFastfat[] = {'F','a','s','t','f','a','t','.','s','y','s','\0'};
	WCHAR szKbdClass[] = {'K','b','d','c','l','a','s','s','.','s','y','s','\0'};
	WCHAR szMouClass[] = {'M','o','u','c','l','a','s','s','.','s','y','s','\0'};
	WCHAR szClasspnp[] = {'C','l','a','s','s','p','n','p','.','s','y','s','\0'};
	WCHAR szAtapi[] = {'A','t','a','p','i','.','s','y','s','\0'};
	WCHAR szAcpi[] = {'A','c','p','i','.','s','y','s','\0'};
	WCHAR szTcpip[] = {'T','c','p','i','p','.','s','y','s','\0'};
	WCHAR szAtaPort[] = {'A','t','a','P','o','r','t','.','s','y','s','\0'};

	firstentry = entry = (PKLDR_DATA_TABLE_ENTRY)GetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY);

	if (entry)
	{
		__try
		{
			do 
			{
				if (entry->BaseDllName.Length && entry->BaseDllName.Buffer)
				{
					ULONG nLen = entry->BaseDllName.Length;
					ULONG nCharCnt = nLen / sizeof(WCHAR);

					// Ntfs
					if (wcslen(szNtfs) == nCharCnt &&
						!_wcsnicmp(szNtfs, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumNtfsBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumNtfsSize, (ULONG)entry->SizeOfImage);
					}
					// acpi
					else if (wcslen(szAcpi) == nCharCnt &&
						!_wcsnicmp(szAcpi, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumAcpiBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumAcpiSize, (ULONG)entry->SizeOfImage);
					} 
					// atapi
					else if (wcslen(szAtapi) == nCharCnt &&
						!_wcsnicmp(szAtapi, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumAtapiBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumAtapiSize, (ULONG)entry->SizeOfImage);
					}
					// Fastfat
					else if (wcslen(szFastfat) == nCharCnt &&
						!_wcsnicmp(szFastfat, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumFastfatBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumFastfatSize, (ULONG)entry->SizeOfImage);
					}
					// KbdClass
					else if (wcslen(szKbdClass) == nCharCnt &&
						!_wcsnicmp(szKbdClass, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumKbdclassBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumKbdclassSize, (ULONG)entry->SizeOfImage);
					}
					// MouClass
					else if (wcslen(szMouClass) == nCharCnt &&
						!_wcsnicmp(szMouClass, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumMouclassBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumMouclassSize, (ULONG)entry->SizeOfImage);
					}
					// Classpnp
					else if (wcslen(szClasspnp) == nCharCnt &&
						!_wcsnicmp(szClasspnp, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumClasspnpBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumClasspnpSize, (ULONG)entry->SizeOfImage);
					}
					// tcpip
					else if (wcslen(szTcpip) == nCharCnt &&
						!_wcsnicmp(szTcpip, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumTcpipBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumTcpipSize, (ULONG)entry->SizeOfImage);
					}
					// AtaPort
					else if (wcslen(szAtaPort) == nCharCnt &&
						!_wcsnicmp(szAtaPort, entry->BaseDllName.Buffer, nCharCnt))
					{
						SetGlobalVeriable(enumAtaPortBase, (ULONG)entry->DllBase);
						SetGlobalVeriable(enumAtaPortSize, (ULONG)entry->SizeOfImage);
					}
				}

				entry = (PKLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink;

			} while(entry && entry != firstentry);	
		}
		__except(1)
		{}
	}
}

//*************************************************
// Returns:   BOOL
// Qualifier: ͨ��AntiSpy���������ҵ�ntoskrnl��KLDR_DATA_TABLE_ENTRY
// Parameter: PDRIVER_OBJECT DriverObject - AntiSpy��������
//*************************************************
BOOL GetKernelLdrDataTableEntry(PDRIVER_OBJECT DriverObject)
{
	BOOL bRet = FALSE;
	if (DriverObject)
	{
		PKLDR_DATA_TABLE_ENTRY entry = NULL, firstentry = NULL;
		WCHAR szNtoskrnl[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
		int nLen = wcslen(szNtoskrnl) * sizeof(WCHAR);

		firstentry = entry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

		while((PKLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink != firstentry)
		{
			KdPrint(("BaseDllName: %wZ, FullDllName: %wZ\n", &(entry->BaseDllName), &(entry->FullDllName)));

			if (entry->BaseDllName.Buffer && 
				nLen == entry->BaseDllName.Length &&
				MmIsAddressValid((PVOID)entry->BaseDllName.Buffer) &&
				MmIsAddressValid((PVOID)&entry->BaseDllName.Buffer[entry->BaseDllName.Length / sizeof(WCHAR) - 1]) &&
				!_wcsnicmp(szNtoskrnl, entry->BaseDllName.Buffer, nLen / sizeof(WCHAR)))
			{
				SetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY, (ULONG)entry);
				bRet = TRUE;
				break;
			}

			entry = (PKLDR_DATA_TABLE_ENTRY)entry->InLoadOrderLinks.Flink;
		}
	}

	return bRet;
}

void InitSlefProtectShadowFunctions()
{
	WIN_VERSION Version = GetGlobalVeriable(enumWindowsVersion);

	SetGlobalVeriable(enumFakeNtUserGetForegroundWindow, (ULONG)Fake_NtUserGetForegroundWindow);
	
	if (Version == enumWINDOWS_8)
	{
		SetGlobalVeriable(enumFakeNtUserBuildHwndList, (ULONG)Fake_NtUserBuildHwndList_Win8);
	}
	else
	{
		SetGlobalVeriable(enumFakeNtUserBuildHwndList, (ULONG)Fake_NtUserBuildHwndList);
	}

	SetGlobalVeriable(enumFakeNtUserDestroyWindow, (ULONG)Fake_NtUserDestroyWindow);
	SetGlobalVeriable(enumFakeNtUserShowWindow, (ULONG)Fake_NtUserShowWindow);
	SetGlobalVeriable(enumFakeNtUserSetWindowLong, (ULONG)Fake_NtUserSetWindowLong);
	SetGlobalVeriable(enumFakeNtUserSetParent, (ULONG)Fake_NtUserSetParent);
	SetGlobalVeriable(enumFakeNtUserQueryWindow, (ULONG)Fake_NtUserQueryWindow);
	SetGlobalVeriable(enumFakeNtUserPostThreadMessage, (ULONG)Fake_NtUserPostThreadMessage);
	SetGlobalVeriable(enumFakeNtUserPostMessage, (ULONG)Fake_NtUserPostMessage);
	SetGlobalVeriable(enumFakeNtUserFindWindowEx, (ULONG)Fake_NtUserFindWindowEx);
	SetGlobalVeriable(enumFakeNtUserMessageCall, (ULONG)Fake_NtUserMessageCall);
	SetGlobalVeriable(enumFakeNtUserWindowFromPoint, (ULONG)Fake_NtUserWindowFromPoint);
}

void InitSlefProtectSSDTFunctions()
{
	SetGlobalVeriable(enumFakeNtOpenProcess, (ULONG)Fake_NtOpenProcess);
	SetGlobalVeriable(enumFakeNtOpenThread, (ULONG)Fake_NtOpenThread);
	SetGlobalVeriable(enumFakeNtTerminateProcess, (ULONG)Fake_NtTerminateProcess);
	SetGlobalVeriable(enumFakeNtTerminateThread, (ULONG)Fake_NtTerminateThread);
	SetGlobalVeriable(enumFakeNtDuplicateObject, (ULONG)Fake_NtDuplicateObject);
}

// �´ε��ǽ�ֹxxϵ�к���
void InitForbidFunctions()
{
	ULONG nBuild = GetGlobalVeriable(enumBuildNumber);

	SetGlobalVeriable(enumFakeNtCreateProcess, (ULONG)Fake_NtCreateProcess);
	SetGlobalVeriable(enumFakeNtCreateProcessEx, (ULONG)Fake_NtCreateProcessEx);
	SetGlobalVeriable(enumFakeNtCreateThread, (ULONG)Fake_NtCreateThread);
	SetGlobalVeriable(enumFakeNtCreateFile, (ULONG)Fake_NtCreateFile);
	SetGlobalVeriable(enumFakeNtCreateKey, (ULONG)Fake_NtCreateKey);
	SetGlobalVeriable(enumFakeNtLoadKey2, (ULONG)Fake_NtLoadKey2);
	SetGlobalVeriable(enumFakeNtReplaceKey, (ULONG)Fake_NtReplaceKey);
	SetGlobalVeriable(enumFakeNtRestoreKey, (ULONG)Fake_NtRestoreKey);
	SetGlobalVeriable(enumFakeNtSetValueKey, (ULONG)Fake_NtSetValueKey);
	SetGlobalVeriable(enumFakeNtCreateSection, (ULONG)Fake_NtCreateSection);
	SetGlobalVeriable(enumFakeNtInitiatePowerAction, (ULONG)Fake_NtInitiatePowerAction);
	SetGlobalVeriable(enumFakeNtSetSystemPowerState, (ULONG)Fake_NtSetSystemPowerState);
	SetGlobalVeriable(enumFakeNtShutdownSystem, (ULONG)Fake_NtShutdownSystem);
	SetGlobalVeriable(enumFakeNtSetSystemTime, (ULONG)Fake_NtSetSystemTime);
	SetGlobalVeriable(enumFakeNtUserLockWorkStation, (ULONG)Fake_NtUserLockWorkStation);
	SetGlobalVeriable(enumFakeNtLoadDriver, (ULONG)Fake_NtLoadDriver);
	
	if (nBuild < 6000)
	{
		SetGlobalVeriable(enumFakeNtUserCallOneParam, (ULONG)Fake_NtUserCallOneParam);
	}
	else if (nBuild >= 6000)
	{
		SetGlobalVeriable(enumFakeNtUserCallOneParam, (ULONG)Fake_NtUserCallNoParam);
		SetGlobalVeriable(enumFakeNtCreateUserProcess, (ULONG)Fake_NtCreateUserProcess);
		SetGlobalVeriable(enumFakeNtCreateThreadEx, (ULONG)Fake_NtCreateThreadEx);
	}

	if (nBuild < 6000)
	{
		SetGlobalVeriable(enumFakeNtUserSwitchDesktop, (ULONG)Fake_NtUserSwitchDesktop_xp);
	}
	else if (nBuild >= 6000 && nBuild < 9200)
	{
		SetGlobalVeriable(enumFakeNtUserSwitchDesktop, (ULONG)Fake_NtUserSwitchDesktop_Win7);
	}
	else if (nBuild == 9200)
	{
		SetGlobalVeriable(enumFakeNtUserSwitchDesktop, (ULONG)Fake_NtUserSwitchDesktop_Win8);
	}
}

// �´ε��ǽ�ֹxxϵ�к���
void InitForbidScreenCapture()
{
	ULONG nBuild = GetGlobalVeriable(enumBuildNumber);

	SetGlobalVeriable(enumFakeNtGdiBitBlt, (ULONG)myNtGdiBitBlt);
	SetGlobalVeriable(enumFakeNtGdiStretchBlt, (ULONG)myNtGdiStretchBlt);
	SetGlobalVeriable(enumFakeNtGdiMaskBlt, (ULONG)myNtGdiMaskBlt);
	SetGlobalVeriable(enumFakeNtGdiPlgBlt, (ULONG)myNtGdiPlgBlt);
	SetGlobalVeriable(enumFakeNtGdiAlphaBlend, (ULONG)newNtGdiAlphaBlend);
	SetGlobalVeriable(enumFakeNtGdiTransparentBlt, (ULONG)newNtGdiTransparentBlt);
	SetGlobalVeriable(enumFakeNtUserGetClipboardData, (ULONG)newNtUserGetClipboardData);

// 	if (nBuild < 6000)
// 	{
// 		SetGlobalVeriable(enumFakeNtUserCallOneParam, (ULONG)Fake_NtUserCallOneParam);
// 	}
// 	else if (nBuild >= 6000)
// 	{
// 		SetGlobalVeriable(enumFakeNtUserCallOneParam, (ULONG)Fake_NtUserCallNoParam);
// 		SetGlobalVeriable(enumFakeNtCreateUserProcess, (ULONG)Fake_NtCreateUserProcess);
// 		SetGlobalVeriable(enumFakeNtCreateThreadEx, (ULONG)Fake_NtCreateThreadEx);
// 	}
// 
// 	if (nBuild < 6000)
// 	{
// 		SetGlobalVeriable(enumFakeNtUserSwitchDesktop, (ULONG)Fake_NtUserSwitchDesktop_xp);
// 	}
// 	else if (nBuild >= 6000 && nBuild < 9200)
// 	{
// 		SetGlobalVeriable(enumFakeNtUserSwitchDesktop, (ULONG)Fake_NtUserSwitchDesktop_Win7);
// 	}
// 	else if (nBuild == 9200)
// 	{
// 		SetGlobalVeriable(enumFakeNtUserSwitchDesktop, (ULONG)Fake_NtUserSwitchDesktop_Win8);
// 	}
}

BOOL InitWindows(PDRIVER_OBJECT DriverObject)
{
	BOOL bRet = FALSE;
	WIN_VERSION WinVersion = enumWINDOWS_UNKNOW;
	
	if (!DriverObject)
	{
		return FALSE;
	}
	
	memset(&GlobalVeriable, 0, MAX_VERIABLE * sizeof(ULONG));

	WinVersion = GetWindowsVersion();

	if (WinVersion != enumWINDOWS_UNKNOW && 
		GetKernelLdrDataTableEntry(DriverObject)) // ���ȵõ�ntoskrnl.exe��KLDR_DATA_TABLE_ENTRY, ����ö��ɶ�Ķ����������ֹ���ǵ�������ժ���ˡ�
	{	
		PGEN_CACHE pVMemCache = NULL;
		PEPROCESS process = PsGetCurrentProcess();
		PVOID pKeServiceDescriptorTable = NULL;
		CHAR szKeServiceDescriptorTable[] = 
			{'K','e','S','e','r','v','i','c','e','D','e','s','c','r','i','p','t','o','r','T','a','b','l','e','\0'};
	//	UNICODE_STRING unKeServiceDescriptorTable;
		
		SetGlobalVeriable(enumSpyHunterDriverObject, (ULONG)DriverObject);
		SetGlobalVeriable(enumSystemProcess, (ULONG)process);
		SetGlobalVeriable(enumWindowsVersion, WinVersion);

	//	RtlInitUnicodeString(&unKeServiceDescriptorTable, szKeServiceDescriptorTable);

		pKeServiceDescriptorTable = MzfGetSystemRoutineAddress(szKeServiceDescriptorTable);/*MmGetSystemRoutineAddress(&unKeServiceDescriptorTable);*/
		if (!pKeServiceDescriptorTable)
		{
			KdPrint(("Get pKeServiceDescriptorTable error\n"));
			return FALSE;
		}

		SetGlobalVeriable(enumNowSSDT, (ULONG)pKeServiceDescriptorTable);
		SetGlobalVeriable(enumSsdtFunctionsCnt, ((PServiceDescriptorTableEntry_t)pKeServiceDescriptorTable)->NumberOfServices);

		KdPrint(("pKeServiceDescriptorTable: 0x%08X\n", pKeServiceDescriptorTable));
		KdPrint(("KeServiceDescriptorTable: 0x%08X\n", KeServiceDescriptorTable));

		// ��ʼ��ȫ�ֱ���
		InitGlobalVariable();

		// reload kernel and save the ssdt table
		if (!ReloadNtoskrnl())
		{
			KdPrint(("ReloadNtoskrnl error\n"));
			return FALSE;
		}

		// reload kernel ok.
		SetGlobalVeriable(enumReloadNtosKrnlSuccess, 1);

		// ��ʼ�����õĺ���
		InitCommonFunctions();

		// ��ʼ��SSDT���еĺ���
		InitSSDTFunctions();
		
		// �о�����ʡ��~
	//	ReloadHalDll();

		// �õ�HAL.DLL�Ļ�ַ�ʹ�С
		GetHalDllInfo();

		// reload win32k.sys
		ReloadWin32k();

		// ��ʼ��shodow ssdt table
		InitShadowSSDTFunctions();
		
		// ��ȡ���õ�������ַ�ʹ�С
		GetCommonDriversSizeAndBase();

		// ��ʼ��FSD�������Ϣ
 		InitFileSystem(enumNtfs);
 		InitFileSystem(enumFastfat);
		
		InitSlefProtectShadowFunctions();
		InitSlefProtectSSDTFunctions();
		InitForbidFunctions();
		
		// ��ֹ����
		if (VMemCacheInit(&pVMemCache))
		{
			SetGlobalVeriable(enumVMemCache, (ULONG)pVMemCache);
			InitForbidScreenCapture();
			KdPrint(("VMemCacheInit success\n"));
		}

		bRet = TRUE;
	}
	
	return bRet;
}

VOID 
UnInitProcessVariable()
{
	//	PsSetCreateProcessNotifyRoutine(RemoveDieProcessNotify, TRUE);

	// 	UnInlieHookKiFastCallEntry();
	// 	UnHookSwapContext();

	// 	// ��ս�������
	// 	while (!IsListEmpty(&ProcessListHead))
	// 	{
	// 		PLIST_ENTRY pListEntry = NULL;
	// 		PPROCESS_INFO pInfo = NULL;
	// 		KIRQL oldIrql;
	// 
	// 		KeAcquireSpinLock(&ProcessSpinLock, &oldIrql);
	// 		pListEntry = RemoveHeadList(&ProcessListHead);
	// 		KeReleaseSpinLock(&ProcessSpinLock, oldIrql);
	// 		pInfo = CONTAINING_RECORD(pListEntry, PROCESS_INFO, NextEntry);
	// 
	// 		ExFreePool(pInfo);
	// 	}
	// 
	// 	// ���Temp��������
	// 	while (!IsListEmpty(&TempProcessListHead))
	// 	{
	// 		PLIST_ENTRY pListEntry = NULL;
	// 		PTEMP_PROCESS_INFO pInfo = NULL;
	// 		KIRQL oldIrql;
	// 
	// 		KeAcquireSpinLock(&TempProcessSpinLock, &oldIrql);
	// 		pListEntry = RemoveHeadList(&TempProcessListHead);
	// 		KeReleaseSpinLock(&TempProcessSpinLock, oldIrql);
	// 		pInfo = CONTAINING_RECORD(pListEntry, TEMP_PROCESS_INFO, NextEntry);
	// 
	// 		ExFreePool(pInfo);
	// 	}
}

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr)+(DWORD)(addValue))

/*++

Routine Description:
	����ڴ��е�ϵͳģ��,(����ntoskrnl.exe)�Ƿ�ʹ����ϵ��ļ�CheckSum��һ�µ�.
	��Ҫ��ֹ���������ϵͳ����ȥ��ȡԭʼ�Ĵ�����Ϣ��������.

Arguments:
	nOldBase - ϵͳ�еĵ�ַ
	nMapedBase - �Լ�ӳ������ļ��õ��ĵ�ַ

Return Value:
	STATUS_UNSUCCESSFUL - ��һ��.
	STATUS_SUCCESS - һ��.

---*/
NTSTATUS VerifyCheckSum(ULONG nOldBase, ULONG nMapedBase)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (nOldBase > 0 && nMapedBase > 0)
	{
		PIMAGE_DOS_HEADER pDosHeader;
		PIMAGE_NT_HEADERS pNTHeader;
		ULONG nCheckSumOld = 0, nCheckSumReload = 0;
		
		KdPrint(("VerifyCheckSum-> nOldBase: 0x%08X, nMapedBase: 0x%08X\n", nOldBase, nMapedBase));

		__try 
		{	
			// �õ��ڴ��ļ���checksum
			pDosHeader = (PIMAGE_DOS_HEADER)nOldBase;
			if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) 
			{
				return status;
			}

			pNTHeader = MakePtr(PIMAGE_NT_HEADERS, nOldBase, pDosHeader->e_lfanew);
			if(pNTHeader->Signature != IMAGE_NT_SIGNATURE) 
			{
				return status;
			}
		
			nCheckSumOld = pNTHeader->OptionalHeader.CheckSum;

			// �õ��Լ�map�ļ���checksum
			pDosHeader = (PIMAGE_DOS_HEADER)nMapedBase;
			if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) 
			{
				return status;
			}

			pNTHeader = MakePtr(PIMAGE_NT_HEADERS, nMapedBase, pDosHeader->e_lfanew);
			if(pNTHeader->Signature != IMAGE_NT_SIGNATURE) 
			{
				return status;
			}

			nCheckSumReload = pNTHeader->OptionalHeader.CheckSum;

			if (nCheckSumReload > 0				&& 
				nCheckSumOld  > 0				&&
				nCheckSumOld == nCheckSumReload)
			{
				KdPrint(("CheckSum is ok.\n"));
				status = STATUS_SUCCESS;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			KdPrint(("VerifyCheckSum exception\n"));
			status = STATUS_UNSUCCESSFUL;
		}
	}

	return status;
}

NTSTATUS MapSectionAndCheckSum(WCHAR* szPath, DWORD dwOrginBase)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	UNICODE_STRING usKrnlPath;
	OBJECT_ATTRIBUTES ObjectAttributes; 
	HANDLE hFile = NULL;
	HANDLE hSection = NULL; 
	IO_STATUS_BLOCK IoStatusBlock; 
	PVOID BaseAddress = NULL; 
	SIZE_T size = 0;  

	if (!szPath || !dwOrginBase)
	{
		return ntStatus;
	}

	MzfInitUnicodeString(&usKrnlPath, szPath);

	KdPrint(("CheckSum Path: %wZ\n", &usKrnlPath));

	InitializeObjectAttributes(&ObjectAttributes,
		&usKrnlPath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
		NULL,
		NULL); 

	ntStatus = ZwOpenFile(&hFile, 
		SYNCHRONIZE | FILE_EXECUTE, 
		&ObjectAttributes, 
		&IoStatusBlock,   
		FILE_SHARE_READ,   
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (NT_SUCCESS(ntStatus))
	{
		ObjectAttributes.ObjectName = 0;  

		ntStatus = ZwCreateSection(&hSection, 
			SECTION_MAP_EXECUTE | SECTION_MAP_READ | SECTION_QUERY, 
			&ObjectAttributes, 
			0,
			PAGE_EXECUTE, 
			SEC_IMAGE, 
			hFile
			);  

		if (NT_SUCCESS(ntStatus))
		{
			ntStatus = ZwMapViewOfSection(hSection, 
				NtCurrentProcess(), 
				&BaseAddress, 
				0, 
				0x1000, 
				0, 
				&size, 
				ViewShare, 
				MEM_TOP_DOWN, 
				PAGE_READWRITE
				);   

			if (NT_SUCCESS(ntStatus))
			{
				// У�����ڴ��е�ģ��ʹ����ϵ��ļ��Ƿ�һ��
				ntStatus = VerifyCheckSum((ULONG)dwOrginBase, (ULONG)BaseAddress);
				if (NT_SUCCESS(ntStatus))
				{
					KdPrint(("VerifyCheckSum ok\n"));
				}
				else
				{
					ntStatus = STATUS_INVALID_DISPOSITION;
				}

				ZwUnmapViewOfSection(NtCurrentProcess(),BaseAddress);
			}//ZwMapViewOfSection Success
			ZwClose(hSection); 
		}//ZwCreateSection Success
		ZwClose( hFile ); 
	}//Open File Success

	return ntStatus;
}

NTSTATUS CheckUpdate()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG KernelModuleBase = 0, KernelModuleSize = 0, NewImageBase = 0;
	WCHAR *szNtoskrnlPath = ExAllocatePoolWithTag(PagedPool, MAX_PATH * sizeof(WCHAR), MZFTAG);

	if (!szNtoskrnlPath)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	memset(szNtoskrnlPath, 0, MAX_PATH * sizeof(WCHAR));

	if (!GetNtoskrnlInfo(szNtoskrnlPath, &KernelModuleBase, &KernelModuleSize))
	{
		ExFreePool(szNtoskrnlPath);
		KdPrint(("GetNtoskrnlInfo failed\n"));
		return STATUS_UNSUCCESSFUL;
	}

	KdPrint(("Ntoskrnl Path: %S, Base: 0x%08X, Size: 0x%X\n", szNtoskrnlPath, KernelModuleBase, KernelModuleSize));
	
	status = MapSectionAndCheckSum(szNtoskrnlPath, KernelModuleBase);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(szNtoskrnlPath);
		return status;
	}

	ExFreePool(szNtoskrnlPath);

// 	KernelModuleBase = KernelModuleSize = 0;
// 	if (!GetWin32kInfo(szNtoskrnlPath, &KernelModuleBase, &KernelModuleSize))
// 	{
// 		KdPrint(("GetWin32kInfo failed\n"));
// 		return STATUS_UNSUCCESSFUL;
// 	}
// 	KdPrint(("Win32k Path: %S, Base: 0x%08X, Size: 0x%X\n", szNtoskrnlPath, KernelModuleBase, KernelModuleSize));
// 
// 	status = MapSectionAndCheckSum(szWin32kPath, KernelModuleBase);
// 	if (!NT_SUCCESS(status))
// 	{
// 		return status;
// 	}

	return STATUS_SUCCESS;
}