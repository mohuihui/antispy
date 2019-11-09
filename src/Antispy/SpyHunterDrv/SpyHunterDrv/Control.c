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
/********************************************************************
	created:	2012/06/06	13:36
	file:		Control.c
	author:		莫灰灰
	blog：		http://blog.csdn.net/hu3167343

	purpose:	
*********************************************************************/

#include "Control.h"
#include "..\\..\\Common\\Common.h"
#include "InitWindows.h"
#include "Process.h"
#include "ProcessModule.h"
#include "ProcessThread.h"
#include "ProcessHandle.h"
#include "processmemory.h"
#include "ProcessWnd.h"
#include "ProcessTimer.h"
#include "HotKeys.h"
#include "ProcessPrivileges.h"
#include "Driver.h"
#include "Reg.h"
#include "Callback.h"
#include "DpcTimer.h"
#include "IoTimer.h"
#include "WorkerThread.h"
#include "DebugRegister.h"
#include "DirectIo.h"
#include "Filter.h"
#include "SSDT.h"
#include "ShadowSSDT.h"
#include "fsd.h"
#include "DispatchHook.h"
#include "kbdclass.h"
#include "mouclass.h"
#include "classpnp.h"
#include "Atapi.h"
#include "Acpi.h"
#include "Tcpip.h"
#include "Idt.h"
#include "CommonFunction.h"
#include "ObjectHook.h"
#include "KernelEntry.h"
#include "IatEat.h"
#include "ModifiedCode.h"
#include "MessageHook.h"
#include "DeleteFile.h"
#include "Disassmbly.h"
#include "ProcessFunc.h"
#include "FileOperation.h"
#include "Port.h"
#include "KernelMemory.h"
#include "SelfProtectSSDT.h"
#include "FakeFunc.h"
#include "Power.h"
#include "FileFunc.h"
#include "MBR.h"
#include "ParseNTFS.h"

NTSTATUS ComVersion(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);

typedef NTSTATUS (*pfnFunction)(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);

pfnFunction FunctionArray[] = {
	NULL,
	ReadSector,
	SetBackupDriverEvent,
	SetBackupDriverPath,
	ReadMBR,
	MzfReadFileEx,
	WriteMBR,
	MzfWriteFileEx,
	MzfCreateFileEx,
	Forbids,
	Powers,
	ModifyKernelMemory,
	QueryLockedRegistryHandle,
	SetProcessPid,
	StartOrStopSelfProtect,
	SetFileNormalAttributeDispatch,
	CloseFileHandleDispatch,
	QueryFileLockInformationDispatch,
	CreateFileDispatch,
	QueryDirectoryFileDispatch,
	ComVersion,
	KernelQueryVirtualMemory,
	ListProcess,							// 枚举进程
	KillProcess,							// 杀进程
	DumpProcess,							// 拷贝进程内存
	ProcessMenu,							// 进程右键菜单,判断进程是否可挂起等
	SuspendOrResumeProcess,					// 挂起或者恢复进程运行
	ListProcessModule,						// 枚举进程模块
	DumpDllModule,							// 拷贝模块内存
	UnloadDllModule,						// 卸载进程模块					
	ListProcessThread,						// 枚举进程线程
	KillThread,								// 杀线程
	SuspendOrResumeThread,					// 挂起或者恢复线程运行
	GetThreadSuspendCount,					// 获取线程的挂起计数
	ListProcessHandle,						// 枚举进程句柄
	KillHandle,								// 杀句柄
	ListProcessMemory,						// 枚举进程内存
	FreeProcessMemory,						// 释放内存
	ModifyMemoryProtection,					// 更改内存保护属性
	ZeroProcessMemory,						// 清零内存
	ListProcessWnds,						// 枚举进程窗口
	ListProcessTimers,						// 枚举进程定时器
	RemoveTimer,							// 移除定时器
	ModifyTimeOutValue,						// 修改定时器超时时间
	ListHotKeys,							// 枚举进程热键
	RemoveHotKey,							// 移除hotkey
	ListProessPrivileges,					// 枚举进程权限
	AdjustProcessTokenPrivileges,			// 调整进程权限
	EnumDrivers,							// 枚举驱动
	EnumerateKey,
	EnumerateKeyValues,
	DumpDriverMemory,
	UnloadDriver,
	GetAllCallbackNotify,
	RemoveCallbackNotify,
	EnumDpcTimer,
	RemoveDpcTimer,
	EnumIoTimer,
	StartOrStopIoTimer,
	RemoveIoTimer,
	EnumWorkerThread,
	EnumDebugRegisters,
	GetDirectIoProcess,
	DisableDirectIo,
	EnumFilterDriver,
	EnumSsdtHookInfo,
	GetSdtInlineHookAddress,
	EnumShadowSsdtHookInfo,
	RestoreSsdtOrShadowHook,
	GetFsdDispatch,
	GetInlineAddress,
	RestoreDispatchHooks,
	GetKbdClassDispatch,
	GetMouClassDispatch,
	GetClasspnpDispatch,
	GetAtapiDispatch,
	GetAcpiDispatch,
	GetTcpipDispatch,
	EnumIdtHook,
	EnumObjectHook,
	EnumAllObjectTypes,
	GetSelfCreateObjectHookInfo,
	EnumKernelEntryHook,
	EnumIatEatHook,
	EnumExportFunctionsPatchs,
	EnumSsdtInlineHookInfo,
	EnumShadowSsdtInlineHookInfo,
	EnumModulePatchs,
	EnumMessageHook,
	RestoreObjectHook,
	RestoreIatEatHook,
	RestoreModifiedCode,
	RegOpenKey,
	RegEnumerateValueKey,
	RegEnumerateKey,
	RegDeleteKey,
	RegCreateKey,
	RegSetValueKey,
	RegDeleteValueKey,
	RegRenameKey,
	KernelDeleteFile,
	Disassmbly,
	RecoverRegisters,
	KernelOpenProcess,
	KernelReadProcessMemory,
	KernelWriteProcessMemory,
	KernelSuspendProcess,
	KernelResumeProcess,
	KernelVirtualProtectEx,
	EnumPort,
	NULL,
};

pfnFunction GetFunction(OPERATE_TYPE type)
{
	if (type > enumUnKnow && type < enumFunctionMax)
	{
		return FunctionArray[type];
	}

	return NULL;
}

NTSTATUS ComVersion(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WCHAR g_szPeVersion[] = {'2','.','0','\0'};
	PVERSION_INFO pVersion = (PVERSION_INFO)pInBuffer;

	KdPrint(("pVersion->szVersion: %S\n", pVersion->szVersion));

	if (pVersion->szVersion && 
		wcslen(pVersion->szVersion) == wcslen(g_szPeVersion) &&
		!_wcsnicmp(pVersion->szVersion, g_szPeVersion, wcslen(g_szPeVersion))
		)
	{
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS CommunicatNeitherControl(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	OPERATE_TYPE type = enumUnKnow;
	pfnFunction pFunc = NULL;

	if (!MzfProbeForRead || !MzfProbeForWrite)
	{
		return status;
	}

	__try
	{
		if (uInSize < sizeof(ULONG))
		{
			return STATUS_INVALID_PARAMETER;
		}
		
		MzfProbeForRead(pInBuffer, uInSize, 1);

		if (uOutSize > 0)
		{
			MzfProbeForWrite(pOutBuffer, uOutSize, 1);
		}
	}
	__except(1)
	{
		return STATUS_UNSUCCESSFUL;
	}

	type = *(OPERATE_TYPE *)pInBuffer;
	pFunc = GetFunction(type);
	if (!pFunc)
	{
		return status;
	}

	status = pFunc(pInBuffer, uInSize, pOutBuffer, uOutSize, dwRet);

	return status;
}