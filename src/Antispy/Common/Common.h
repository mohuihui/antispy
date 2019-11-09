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
#ifndef _COMMON_H_
#define _COMMON_H_

#define DEVICE_NAME			L"\\Device\\SpyHunter"
#define SYMLINK_NAME		L"\\DosDevices\\SpyHunter"
#define WIN32_DEVICE_NAME	L"\\\\.\\SpyHunter"

#define IOCTL_BASE          0x800

#define MY_NEITHER_CTL_CODE(i)		\
		CTL_CODE					\
		(							\
			FILE_DEVICE_UNKNOWN,	\
			IOCTL_BASE + i + 0x100,	\
			METHOD_NEITHER,			\
			FILE_ANY_ACCESS			\
		)

#define IOCTL_NEITHER_CONTROL    MY_NEITHER_CTL_CODE(1)

typedef enum _OPERATE_TYPE_
{
	enumUnKnow = 0,
	enumReadSector,
	enumSetBackupDriverEvent,
	enumSetBackupDriverPath,
	enumReadMBR,
	enumReadFileEx,
	enumWriteMBR,
	enumWriteFileEx,
	enumCreateFileEx,
	enumForbidTypes,
	enumPowerTypes,
	enumModifyKernelAddress,
	enumQueryRegLockedInfo,
	enumSetPid,
	enumStartSelfProtect,
	enumSetFileNormalAttribute,
	enumCloseFileHandle,
	enumQueryFileLockInfo,
	enumCreateFile,
	enumQueryDirectoryFile,
	enumCmpVersion,
	enumQueryVirtualMemory,
	enumListProcess,						// 枚举进程
	enumKillProcess,						// 杀死进程
	enumDumpProcess,						// 拷贝进程内存
	enumGetProcessSuspendStatus,			// 获取进程的挂起和恢复状态
	enumSuspendOrResumeProcess,				// 挂起或者恢复进程
	enumListProcessModule,					// 枚举进程模块
	enumDumpDllModule,						// 拷贝模块内存
	enumUnloadDllModule,					// 卸载模块
	enumListProcessThread,					// 枚举进程线程
	enumKillThread,							// 杀线程
	enumResumeOrSuspendThread,				// 挂起或者恢复线程
	enumGetThreadSuspendCnt,				// 获取线程的挂起计数
	enumListProcessHandle,					// 枚举进程句柄
	enumCloseHandle,						// 关闭句柄
	enumListProcessMemory,					// 枚举进程内存
	enumFreeMemory,	
	enumMemoryModifyProtection,
	enumZeroMemory,							// 清零内存
	enumListProcessWnd,
	enumListProcessTimer,
	enumRemoveTimer,
	enumModifyTimeOut,
	enumListHotKey,
	enumRemoveHotKey,
	enumListProcessPrivileges,
	enumAdjustTokenPrivileges,
	enumListDriver,
	enumEnumerateKey,
	enumEnumerateKeyValue,
	enumDumpDriver,
	enumUnloadDriver,
	enumListCallback,
	enumRemoveCallback,
	enumListDpcTimer,
	enumRemoveDpcTimer,
	enumListIoTimer,
	enumStopOrStartIoTimer,
	enumRemoveIoTimer,
	enumListWorkerThread,
	enumListDebugRegister,
	enumGetDirectIoProcess,
	enumDisableDirectIo,
	enumListFilter,
	enumListSsdtHooks,
	enumSDTGetInlineHookAddress,
	enumListShadowSsdtHooks,
	enumRestoreSdt,
	enumFsdHooks,
	enumGetInlineAddress,
	enumRestoreDispatchHook,
	enumKbdClassHook,
	enumMouClassHook,
	enumClasspnpHook,
	enumAtapiHook,
	enumAcpiHook,
	enumTcpipHook,
	enumIdtHook,
	enumObjectHook,
	enumGetObjectTypes,
	enumSelfCreateObjectHook,
	enumKernelEntryHook,
	enumIatEatHook,
	enumExportFunctionsPatchs,					// 枚举导出函数被修改
	enumSsdtInlineHookInfo,
	enumShadowSsdtInlineHookInfo,
	enumAllOfMoudleInlineHookInfo,
	enumMessageHook,
	enumRestoreObjectHook,
	enumRestoreIatEatHook,
	enumRestoreModifyCode,
	enumOpenKey,
	enumEnumValue,
	enumEnumKey,
	enumDeleteKey,
	enumCreateKey,
	enumSetValueKey,
	enumDeleteValueKey,
	enumRenameKey,
	enumDeleteFile,
	enumDisassembly,
	enumRecoverDebugRegister,
	enumOpenProcess,
	enumReadProcessMemory,
	enumWriteProcessMemory,
	enumSuspendProcess,
	enumResumeProcess,
	enumVirtualProtectEx,
	enumEnumPort,
	enumFunctionMax
} OPERATE_TYPE;

//////////////////////////////////////////////////////////////////

typedef enum _ENUM_IS_MICROSOFT_APP_
{
	enumMiciosoftApp,
	enumNotMiciosoftApp,
}ENUM_IS_MICROSOFT_APP;

///////////////////////////////////////////////////////////

typedef struct _VERSION_INFO_
{
	OPERATE_TYPE OpType;
	WCHAR szVersion[100];
}VERSION_INFO, *PVERSION_INFO;

////////////// 以下结构用来枚举进程 ////////////////////

typedef struct _PROCESS_INFO_
{
	ULONG ulPid;
	ULONG ulParentPid;
	ULONG ulEprocess;
	ULONGLONG ulCreateTime;
	WCHAR szPath[MAX_PATH];
}PROCESS_INFO, *PPROCESS_INFO;

typedef struct _ALL_PROCESSES_
{
	ULONG nCnt;
	PROCESS_INFO ProcessInfo[1];
}ALL_PROCESSES, *PALL_PROCESSES;

typedef struct _COMMUNICATE_PROCESS_
{
	OPERATE_TYPE OpType;

	union {
		struct { 
			ULONG nPid;
			ULONG pEprocess;
		} KillProcess;
	
		struct { 
			ULONG nPid; 
			ULONG pEprocess;
		} DumpProcess;

		struct { 
			ULONG nPid; 
			ULONG pEprocess;
		} GetProcessSuspendStatus;

		struct { 
			ULONG nPid; 
			ULONG pEprocess;
			ULONG bSuspend;
		} SuspendOrResumeProcess;

	} op;
}COMMUNICATE_PROCESS, *PCOMMUNICATE_PROCESS;

typedef struct _RESUME_OR_SUSPEND_
{
	ULONG bSuspend;
	ULONG bResume;
}RESUME_OR_SUSPEND, *PRESUME_OR_SUSPEND;

typedef enum _SUSPEND_OR_RESUME_TYPE_
{
	enumAllFalse,
	enumAllOk,
	enumSuspend,
	enumResume,
}SUSPEND_OR_RESUME_TYPE;

////////// 以下结构用来枚举进程DLL模块 /////////////

typedef struct _MODULE_INFO_
{
	ULONG Base;
	ULONG Size;
	WCHAR Path[MAX_PATH]; 
}MODULE_INFO, *PMODULE_INFO;

typedef struct _ALL_MODULES_
{
	ULONG nCnt;
	MODULE_INFO Modules[1];
}ALL_MODULES, *PALL_MODULES;

typedef struct _COMMUNICATE_PROCESS_MODULE_
{
	OPERATE_TYPE OpType;
	
	ULONG nPid;
	ULONG pEprocess;

	union {

		struct{
			ULONG Base;
			ULONG Size;
		}Dump;
		
		struct{
			ULONG Base;
			ULONG LdrpHashTable;
		}Unload;

	}op;

}COMMUNICATE_PROCESS_MODULE, *PCOMMUNICATE_PROCESS_MODULE;

/////////////// 以下结构用来枚举进程线程 /////////////////

typedef struct _COMMUNICATE_THREAD_
{
	OPERATE_TYPE OpType;

	ULONG nPid;
	ULONG pEprocess;

	union{

		struct{
			ULONG pThread;	
		}Kill;

		struct{
			ULONG pThread;
			BOOL bSuspend;
		}Suspend;
		
		struct{
			ULONG pThread;
		}GetSuspendCnt;

	}op;

}COMMUNICATE_THREAD, *PCOMMUNICATE_THREAD;

typedef struct _THREAD_INFO_
{
	ULONG pThread;
	ULONG Tid;
	ULONG Teb;
	ULONG Priority;
	ULONG Win32StartAddress;
	ULONG ContextSwitches;
	ULONG State;
}THREAD_INFO, *PTHREAD_INFO;

typedef struct _ALL_THREADS_
{
	ULONG nCnt;
	THREAD_INFO Threads[1];
}ALL_THREADS, *PALL_THREADS;

////////// 以下结构用来枚举进程句柄 /////////////

typedef struct _COMMUNICATE_HANDLE_
{
	OPERATE_TYPE OpType;

	ULONG nPid; 
	ULONG pEprocess;

	union {

		struct { 
			ULONG hHandle;
			ULONG pHandleObject;
			ULONG bForceKill;
		} Close;

	} op;

}COMMUNICATE_HANDLE, *PCOMMUNICATE_HANDLE;

#define MAX_OBJECT_NAME 50

typedef struct _HANDLE_INFO_
{
	ULONG Handle;
	ULONG Object;
	ULONG ReferenceCount;
	WCHAR ObjectName[MAX_OBJECT_NAME];
	WCHAR HandleName[MAX_PATH];
}HANDLE_INFO, *PHANDLE_INFO;

typedef struct _PROCESS_HANDLES_
{
	ULONG nCnt;
	HANDLE_INFO Handles[1];
}ALL_HANDLES, *PALL_HANDLES;

////////// 以下结构用来枚举进程内存 /////////////

typedef struct _COMMUNICATE_MEMORY_
{
	OPERATE_TYPE OpType;

	union {
		struct { 
			ULONG nPid; 
			ULONG pEprocess;
		} GetMemory;

		struct { 
			ULONG nPid; 
			ULONG pEprocess;
			ULONG Base;
			ULONG Size;
		} FreeMemory;

		struct { 
			ULONG nPid; 
			ULONG pEprocess;
			ULONG Base;
			ULONG Size;
			ULONG ModifyType;
		} ModifyProtection;

		struct { 
			ULONG nPid; 
			ULONG pEprocess;
			ULONG Base;
			ULONG Size;
		} ZeroMemory;

	} op;
}COMMUNICATE_MEMORY, *PCOMMUNICATE_MEMORY;

typedef struct _MEMORY_INFO_
{
	ULONG Base;
	ULONG Size;
	ULONG Protect;
	ULONG State;
	ULONG Type;
}MEMORY_INFO, *PMEMORY_INFO;

typedef struct _ALL_MEMORYS_
{
	ULONG nCnt;
	MEMORY_INFO Memorys[1];
}ALL_MEMORYS, *PALL_MEMORYS;

////////// 以下结构用来枚举进程窗口 /////////////

typedef struct _WND_INFO_
{
	HWND  hWnd;
	ULONG uPid;
	ULONG uTid;
}WND_INFO, *PWND_INFO;

typedef struct _ALL_WNDS_
{
	ULONG nCnt;
	WND_INFO WndInfo[1];
}ALL_WNDS, *PALL_WNDS;

////////// 以下结构用来枚举进程定时器 /////////////

typedef struct _TIMER_INFO_
{
	ULONG TimerObject;  // 定时器对象
	ULONG pCallback;	// 定时器回调函数
	ULONG nInterval;    // 时间间隔
	ULONG tid;			// 所属线程ID
	ULONG pEprocess;    // 所属进程eprocess
	
}TIMER_INFO, *PTIMER_INFO;

typedef struct _ALL_TIMERS_
{
	ULONG nCnt;
	TIMER_INFO timers[1];
}ALL_TIMERS, *PALL_TIMERS;

typedef struct _COMMUNICATE_TIMER_
{
	OPERATE_TYPE OpType;

	union {
		struct { 
			ULONG TimerObject;
		} Remove;

		struct { 
			ULONG TimerObject;
			ULONG TimeOut;
		} ModifyTimeOut;

	} op;
}COMMUNICATE_TIMER, *PCOMMUNICATE_TIMER;

////////// 以下结构用来枚举系统热键 /////////////

typedef struct _HOTKEY_ITEM_
{
	ULONG HotKeyObject;  
	ULONG nId;	
	ULONG fsModifiers;   
	ULONG vk;
	ULONG tid;			// 所属线程ID
	ULONG pEprocess;    // 所属进程eprocess
}HOTKEY_ITEM, *PHOTKEY_ITEM;

typedef struct _ALL_HOTKEYS_
{
	ULONG nCnt;
	HOTKEY_ITEM HotKeys[1];
}ALL_HOTKEYS, *PALL_HOTKEYS;

typedef struct _COMMUNICATE_HOTKEY_
{
	OPERATE_TYPE OpType;

	union {
		struct { 
			ULONG HotKeyObject;
		} Remove;
	} op;
}COMMUNICATE_HOTKEY, *PCOMMUNICATE_HOTKEY;

////////// 以下结构用来枚举进程权限 /////////////

typedef struct _COMMUNICATE_PRIVILEGES_
{
	OPERATE_TYPE OpType;
	union {
		struct { 
			ULONG nPid;
			ULONG pEprocess;
		} GetPrivilege;

		struct {
			ULONG nPid;
			ULONG pEprocess;
			PTOKEN_PRIVILEGES pPrivilegeState;
		} AdjustPrivilege;
	} op;
}COMMUNICATE_PRIVILEGES, *PCOMMUNICATE_PRIVILEGES;


////////// 以下结构用来枚举内核模块 /////////////

typedef enum _DRIVER_TYPE_
{
	enumHide,
	enumHaveOrder,
	enumSuspiciousPe,
}DRIVER_TYPE;

typedef struct _DRIVER_INFO_
{
	DRIVER_TYPE DriverType;
	ULONG nLodeOrder;
	ULONG nBase;
	ULONG nSize;
	ULONG nDriverObject;
	WCHAR szDriverPath[MAX_PATH];
}DRIVER_INFO, *PDRIVER_INFO;

typedef struct _ALL_DRIVERS_
{
	ULONG nCnt;
	DRIVER_INFO Drivers[1];
}ALL_DRIVERS, *PALL_DRIVERS;

typedef struct _COMMUNICATE_DRIVER_
{
	OPERATE_TYPE OpType;
	union {

		struct { 
			ULONG nSize;
			ULONG pBase;
		} DumpMemory;

		struct {
			ULONG DriverObject;
		} Unload;
	} op;

}COMMUNICATE_DRIVER, *PCOMMUNICATE_DRIVER;

////////// 以下结构用来得到Reg信息 /////////////



typedef struct _COMMUNICATE_REG_
{
	OPERATE_TYPE OpType;
	WCHAR szKey[MAX_PATH * 2];
}COMMUNICATE_REG, *PCOMMUNICATE_REG;


////////// 以下结构用来枚举内核回调 /////////////

typedef enum _CALLBACK_TYPE_
{
	enumCreateProcess,
	enumCreateThread,
	enumLoadImage,
	enumShutdown,
	enumCmCallBack,
}CALLBACK_TYPE;

typedef struct _CALLBACK_INFO_
{
	CALLBACK_TYPE type;
	ULONG pCallbackAddress;
	ULONG Note;
}CALLBACK_INFO, *PCALLBACK_INFO;

typedef struct _GET_CALLBACK_
{
	ULONG nCnt;
	ULONG nRetCnt;
	CALLBACK_INFO Callbacks[1];
}GET_CALLBACK, *PGET_CALLBACK;

typedef struct _COMMUNICATE_CALLBACK_
{
	OPERATE_TYPE OpType;
	CALLBACK_TYPE Type;
	ULONG Callback;
	ULONG Note;
}COMMUNICATE_CALLBACK, *PCOMMUNICATE_CALLBACK;


////////// 以下结构用来枚举DPC Timer /////////////

typedef struct _DPC_TIMER_
{
	ULONG TimerObject;
	ULONG Period;			// 周期
	ULONG TimeDispatch;
	ULONG Dpc;
}DPC_TIMER, *PDPC_TIMER;

typedef struct _DPC_TIMER_INFO_
{
	ULONG nCnt;
	ULONG nRetCnt;
	DPC_TIMER DpcTimer[1];
}DPC_TIMER_INFO, *PDPC_TIMER_INFO;

typedef struct _COMMUNICATE_DPCTIMER_
{
	OPERATE_TYPE OpType;
	ULONG pTimer;
}COMMUNICATE_DPCTIMER, *PCOMMUNICATE_DPCTIMER;


////////// 以下结构用来枚举IO Timer /////////////

typedef struct _IO_TIMERS_
{
	ULONG TimerObject;
	ULONG DeviceObject;
	ULONG TimeDispatch;
	ULONG TimerEntry;
	ULONG Status;
}IO_TIMERS, *PIO_TIMERS;

typedef struct _IO_TIMER_INFO_
{
	ULONG nCnt;
	ULONG nRetCnt;
	IO_TIMERS IoTimer[1];
}IO_TIMER_INFO, *PIO_TIMER_INFO;

typedef struct _COMMUNICATE_IO_TIMER_
{
	OPERATE_TYPE OpType;

	union {
		struct { 
			ULONG bStart;
			ULONG pDeviceObject;
		} StopOrStartTimer;

		struct {
			ULONG TimerEntry;
		} Remove;
	} op;

}COMMUNICATE_IO_TIMER, *PCOMMUNICATE_IO_TIMER;


////////// 以下结构用来枚举WORKER THREAD /////////////

typedef struct _WORKER_THREAD_
{
	ULONG ThreadObject;
	ULONG Dispatch;
	ULONG Type;
}WORKER_THREAD, *PWORKER_THREAD;

typedef struct _WORKER_THREAD_INFO_
{
	ULONG nCnt;
	ULONG nRetCnt;
	WORKER_THREAD Thread[1];
}WORKER_THREAD_INFO, *PWORKER_THREAD_INFO;


////////// 以下结构用来枚举Debug Register /////////////

typedef struct _DEBUG_REGISTER_
{
	ULONG DR0;
	ULONG DR1;
	ULONG DR2;
	ULONG DR3;
	ULONG DR6;
	ULONG DR7;
}DEBUG_REGISTER, *PDEBUG_REGISTER;

typedef struct _DEBUG_REGISTER_INFO_
{
	ULONG nCnt;
	DEBUG_REGISTER DebugRegister[1];
}DEBUG_REGISTER_INFO, *PDEBUG_REGISTER_INFO;

typedef enum _DRX_REG
{
	enumDRUnknow,
	enumDR0,
	enumDR1,
	enumDR2,
	enumDR3,
	enumDRAll
}DRX_REG;

typedef struct _RECOVER_DEBUG_REGISTER
{
	OPERATE_TYPE OpType;
	DRX_REG RegType;
}RECOVER_DEBUG_REGISTER, *PRECOVER_DEBUG_REGISTER;

////////// 以下结构用来枚举可以Direct IO的进程 /////////////

typedef enum _DIRECT_IO_TYPE_
{
	enumOK = 0,
	enumIOPL = 0x1,
	enumTSSLarge = 0x2,
	enumIOPLOffsetChange = 0x4,
}DIRECT_IO_TYPE;

typedef struct _DIRECT_IO_PROCESS_INFO_
{
	DIRECT_IO_TYPE Type;
	ULONG Pid;
	ULONG pEprocess;
	WCHAR szPath[MAX_PATH];
}DIRECT_IO_PROCESS_INFO, *PDIRECT_IO_PROCESS_INFO;

typedef struct _DIRECT_IO_PROCESS_
{
	ULONG nCnt;
	ULONG nRetCnt;
	DIRECT_IO_PROCESS_INFO Process[1];
}DIRECT_IO_PROCESS, *PDIRECT_IO_PROCESS;

typedef struct _COMMUNICATE_DIRECTIO_
{
	OPERATE_TYPE OpType;

	union {
		struct { 
			DIRECT_IO_TYPE Type;
			ULONG pid;
			ULONG pEprocess;
		} Disable;
	} op;

}COMMUNICATE_DIRECTIO, *PCOMMUNICATE_DIRECTIO;

////////// 以下结构用来枚举过滤驱动 /////////////

typedef enum _FILTER_TYPE_
{
	enumUnkonw,
	enumFile,
	enumDisk,
	enumVolume,
	enumKeyboard,
	enumMouse,
	enumI8042prt,
	enumTcpip,
	enumNDIS,
	enumPnpManager
}FILTER_TYPE;

typedef struct _FILTER_INFO_
{
	FILTER_TYPE Type;
	ULONG pDeviceObject;
	WCHAR szDriverName[100];
	WCHAR szAttachedDriverName[100];
	WCHAR szPath[MAX_PATH];
}FILTER_INFO, *PFILTER_INFO;

typedef struct _FILTER_DRIVER_
{
	ULONG nCnt;
	ULONG nRetCnt;
	FILTER_INFO Filter[1];
}FILTER_DRIVER, *PFILTER_DRIVER;


////////// 以下结构用来枚举过SSDT HOOK /////////////

typedef enum _HOOKED_
{
	enumNoneHook,
	enumHooked,
};

typedef struct _SSDT_HOOK_INFO_  // ssdt hook
{
	ULONG nIndex;
	ULONG pOriginAddress;
	ULONG pNowAddress;
	ULONG pInlineHookAddress;
	WCHAR szFunctionName[100];
}SSDT_HOOK_INFO, *PSSDT_HOOK_INFO;

typedef struct _ALL_SSDT_HOOK_ // ssdt hook
{
	ULONG nCnt;
	SSDT_HOOK_INFO SsdtHook[1];
}ALL_SSDT_HOOK, *PALL_SSDT_HOOK;

typedef struct _SHADOW_SSDT_HOOK_INFO_ // shadow ssdt hook
{
	ULONG nIndex;
	ULONG pOriginAddress;
	ULONG pNowAddress;
	ULONG pInlineHookAddress;
}SHADOW_SSDT_HOOK_INFO, *PSHADOW_SSDT_HOOK_INFO;

typedef struct _SHADOW_SSDT_HOOK_ // shadow ssdt hook
{
	ULONG nCnt;
	ULONG nRetCnt;
	SHADOW_SSDT_HOOK_INFO ShadowSsdtHook[1];
}SHADOW_SSDT_HOOK, *PSHADOW_SSDT_HOOK;

typedef struct _COMMUNICATE_SSDT_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG pAddress;
		} GetInlineAddress;

		struct {
			ULONG bSsdt;
			ULONG nIndex;
		} Restore;
	} op;

}COMMUNICATE_SSDT, *PCOMMUNICATE_SSDT;


////////// 以下结构用来枚举过FSD HOOK /////////////

typedef enum _FILE_SYSTEM_TYPE_
{
	enumNtfs,
	enumFastfat,
	enumExfat,
}FILE_SYSTEM_TYPE;

typedef struct _COMMUNICATE_DISPATCH_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG pAddress;
		} GetInlineAddress;

		struct {
			FILE_SYSTEM_TYPE FsdType;
			ULONG nIndex;
		} Restore;
	} op;

}COMMUNICATE_DISPATCH, *PCOMMUNICATE_DISPATCH;


////////// 以下结构用来恢复Dispatch hook /////////////

typedef enum _DISPATCH_TYPE_
{
	DispatchUnKnow = 0,
	DispatchNtfs,
	DispatchFastfat,
	DispatchExfat,
	DispatchAcpi,
	DispatchAtapi,
	DispatchDisk,
	DispatchKbdclass,
	DispatchMouclass,
	DispatchTcpip,
}DISPATCH_TYPE;

typedef struct _DISPATCH_HOOK_INFO_
{
	ULONG nIndex;
	ULONG pOriginAddress;
	ULONG pNowAddress;
	ULONG pInlineHookAddress;
}DISPATCH_HOOK_INFO, *PDISPATCH_HOOK_INFO;

typedef struct _DISPATCH_HOOK_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG pAddress;
		} GetInlineAddress;

		struct {
			DISPATCH_TYPE DispatchType;
			ULONG nIndex;
		} Restore;
	} op;

}DISPATCH_HOOK, *PDISPATCH_HOOK;

/////////////////////////////////////////////

typedef struct _IDT_HOOK_INFO_
{
	ULONG nIndex;
	ULONG pNowAddress;
	ULONG pOriginAddress;
	ULONG pInlineHookAddress;
}IDT_HOOK_INFO, *PIDT_HOOK_INFO;

/////////////////////////////////////////////

typedef enum 
{
	eDirectoryType,
	eMutantType,
	eThreadType,
	eFilterCommunicationPortType,
	eControllerType,
	eProfileType,
	eEventType,
	eTypeType,
	eSectionType,
	eEventPairType,
	eSymbolicLinkType,
	eDesktopType,
	eTimerType,
	eFileType,
	eWindowStationType,
	eDriverType,
	eWmiGuidType,
	eKeyedEventType,
	eDeviceType,
	eTokenType,
	eDebugObjectType,
	eIoCompletionType,
	eProcessType,
	eAdapterType,
	eKeyType,
	eJobType,
	eWaitablePortType,
	ePortType,
	eCallbackType,
	eFilterConnectionPortType,
	eSemaphoreType,
	eMaxType,
	eUnknowType,
}ENUM_OBJECT_TYPE;

typedef struct _OBJECT_HOOK_INOF_
{
	ENUM_OBJECT_TYPE Type;			// 对象索引
	ULONG TypeObject;				// 类型对象

	ULONG DumpProcedure;			// Now Procedure
	ULONG OpenProcedure;
	ULONG CloseProcedure;
	ULONG DeleteProcedure;
	ULONG ParseProcedure;
	ULONG SecurityProcedure;
	ULONG QueryNameProcedure;
	ULONG OkayToCloseProcedure;

	ULONG OriginDumpProcedure;		// Origin Procedure
	ULONG OriginOpenProcedure;
	ULONG OriginCloseProcedure;
	ULONG OriginDeleteProcedure;
	ULONG OriginParseProcedure;
	ULONG OriginSecurityProcedure;
	ULONG OriginQueryNameProcedure;
	ULONG OriginOkayToCloseProcedure;
	
	ULONG InlineDumpProcedure;			// Inline Procedure
	ULONG InlineOpenProcedure;
	ULONG InlineCloseProcedure;
	ULONG InlineDeleteProcedure;
	ULONG InlineParseProcedure;
	ULONG InlineSecurityProcedure;
	ULONG InlineQueryNameProcedure;
	ULONG InlineOkayToCloseProcedure;
}OBJECT_HOOK_INOF, *POBJECT_HOOK_INOF;

#define MAX_TYPE_NAME 50

typedef struct _OBJECT_TYPE_INFO
{
	ULONG ObjectType;
	WCHAR szTypeName[MAX_TYPE_NAME];
}OBJECT_TYPE_INFO, *POBJECT_TYPE_INFO;

typedef enum 
{
	enumProcedureUnknow,
	enumDumpProcedure,
	enumOpenProcedure,
	enumCloseProcedure,
	enumDeleteProcedure,
	enumParseProcedure,
	enumSecurityProcedure,
	enumQueryNameProcedure,
	enumOkayToCloseProcedure
}ENUM_OBJECT_PROCEDURE;

typedef struct _COMMUNICATE_OBJECT_HOOK_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG pType;
		} GetSlefCreateType;

		struct {
			ULONG ObjectType;
			ENUM_OBJECT_PROCEDURE ObjectProcedureType;
			ULONG OriginProcedure;
		} Restore;

	} op;

}COMMUNICATE_OBJECT_HOOK, *PCOMMUNICATE_OBJECT_HOOK;

/////////////////////////////////////////////

#define MAX_PATCHED_LEN 50

typedef struct _KERNEL_ENTRY_HOOK_INFO
{
	ULONG PatchLen;
	ULONG OriginAddress; // 被Pathc的地址
	ULONG HookAddress;   // Hook后执行的地址
	BYTE NowBytes[MAX_PATCHED_LEN];
	BYTE OriginBytes[MAX_PATCHED_LEN];
}KERNEL_ENTRY_HOOK_INFO, *PKERNEL_ENTRY_HOOK_INFO;


/////////////////////////////////////////////////////

typedef enum 
{
	enumIatEatAllNot,
	enumIat,
	enumEat,
}IAT_EAT_HOOK_TYPE;

#define MAX_FUNCTION_LEN 50

typedef struct _IAT_EAT_HOOK_INFO
{
	IAT_EAT_HOOK_TYPE HookType;
	ULONG HookAddress;
	ULONG NowAddress;
	ULONG OriginAddress;
	WCHAR szFunction[MAX_FUNCTION_LEN];
}IAT_EAT_HOOK_INFO, *PIAT_EAT_HOOK_INFO;

typedef struct _ALL_IAT_EAT_HOOK
{
	ULONG nCnt;
	IAT_EAT_HOOK_INFO hooks[1];
}ALL_IAT_EAT_HOOK, *PALL_IAT_EAT_HOOK;

typedef struct _COMMUNICATE_IAT_EAT_HOOK
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG Base;
			ULONG Size;
			WCHAR szPath[MAX_PATH];
		} Get;

		struct {
			ULONG HookAddress;
			ULONG OriginDispatch;
		} Restore;

	} op;

}COMMUNICATE_IAT_EAT_HOOK, *PCOMMUNICATEIAT_IAT_EAT_HOOK;

////////////////////////////////////////////////////////

typedef struct _EXPORT_FUNCTION_PATCH_INFO
{
	ULONG PatchLen;
	ULONG FunctionAddress;	// 被hook的函数地址
	ULONG PatchedAddress;	// 被Patch的地址
	ULONG HookAddress;		// Hook后执行的地址
	BYTE NowBytes[MAX_PATCHED_LEN];
	BYTE OriginBytes[MAX_PATCHED_LEN];
	WCHAR szFunction[MAX_FUNCTION_LEN];
}EXPORT_FUNCTION_PATCH_INFO, *PEXPORT_FUNCTION_PATCH_INFO;

typedef struct _EXPORT_FUNCTION_PATCHS_
{
	ULONG nCnt;
	EXPORT_FUNCTION_PATCH_INFO ExportPatch[1];
}ALL_EXPORT_FUNCTION_PATCHS, *PALL_EXPORT_FUNCTION_PATCHS;

typedef struct _SHADOW_INLINE_INFO
{
	ULONG nIndex;           // Shadow函数索引
	ULONG PatchLen;
	ULONG FunctionAddress;	// 被hook的函数地址
	ULONG PatchedAddress;	// 被Patch的地址
	ULONG HookAddress;		// Hook后执行的地址
	BYTE NowBytes[MAX_PATCHED_LEN];
	BYTE OriginBytes[MAX_PATCHED_LEN];
}SHADOW_INLINE_INFO, *PSHADOW_INLINE_INFO;

typedef struct _ALL_SHADOW_INLINE_
{
	ULONG nCnt;
	SHADOW_INLINE_INFO ShadowHooks[1];
}ALL_SHADOW_INLINE, *PALL_SHADOW_INLINE;

typedef struct _MODULE_PATCH_INFO
{
	ULONG PatchLen;
	ULONG PatchedAddress;	// 被Patch的地址
	ULONG HookAddress;		// Hook后执行的地址
	BYTE NowBytes[MAX_PATCHED_LEN];
	BYTE OriginBytes[MAX_PATCHED_LEN];
}MODULE_PATCH_INFO, *PMODULE_PATCH_INFO;

typedef struct _ALL_OF_MODULE_PATCHS_
{
	ULONG nCnt;
	MODULE_PATCH_INFO ModulePatchs[1];
}ALL_OF_MODULE_PATCHS, *PALL_OF_MODULE_PATCHS;

typedef struct _COMMUNICATE_EXPORT_FUNCTION_HOOK
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG Base;
			ULONG Size;
			WCHAR szPath[MAX_PATH];
		} Get;
	} op;

}COMMUNICATE_EXPORT_FUNCTION_HOOK, *PCOMMUNICATE_EXPORT_FUNCTION_HOOK;

typedef struct _MODIFIED_CODE_INFO_
{
	ULONG PatchLen;
	ULONG PatchedAddress;	// 被Patch的地址
	BYTE OriginBytes[MAX_PATCHED_LEN];
}MODIFIED_CODE_INFO, *PMODIFIED_CODE_INFO;

typedef struct _COMMUNICATE_MODIFY_CODE
{
	OPERATE_TYPE OpType;

	union {
		struct {
			MODIFIED_CODE_INFO ModifyCodeInfo;
		} Restore;
	} op;

}COMMUNICATE_MODIFY_CODE, *PCOMMUNICATE_MODIFY_CODE;

//////////////////////////////////////////////

typedef enum  _HOOK_TYPE{
	R_WH_MSGFILTER = -1,
	R_WH_JOURNALRECORD = 0,
	R_WH_JOURNALPLAYBACK = 1,
	R_WH_KEYBOARD = 2,
	R_WH_GETMESSAGE = 3,
	R_WH_CALLWNDPROC = 4,
	R_WH_CBT = 5,
	R_WH_SYSMSGFILTER = 6,
	R_WH_MOUSE = 7,
	R_WH_HARDWARE = 8,
	R_WH_DEBUG = 9,
	R_WH_SHELL = 10,
	R_WH_FOREGROUNDIDLE = 11,
	R_WH_CALLWNDPROCRET = 12,
	R_WH_KEYBOARD_LL = 13,
	R_WH_MOUSE_LL = 14
}HOOK_TYPE;

#define MAX_MODULE_NAME_LEN 50

typedef struct _MESSAGE_HOOK_INFO
{
	HOOK_TYPE iHook;      
	HANDLE Handle;
	ULONG offPfn;
	DWORD ihmod;
	ULONG flags;
	ULONG Pid;
	ULONG Tid;
	WCHAR ModuleName[MAX_MODULE_NAME_LEN];
	WCHAR szPath[MAX_PATH];
}MESSAGE_HOOK_INFO, *PMESSAGE_HOOK_INFO;

typedef struct _MESSAGE_HOOK_
{
	ULONG nCnt;
	ULONG nRetCnt;
	MESSAGE_HOOK_INFO MessageHookInfo[1];
}MESSAGE_HOOK, *PMESSAGE_HOOK;

typedef struct _COMMUNICATE_MESSAGE_HOOK_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ULONG UserRegisterWowHandlers;
		} Get;
	} op;

}COMMUNICATE_MESSAGE_HOOK, *PCOMMUNICATE_MESSAGE_HOOK;

////////////////////////////////////////////////////////////

typedef struct _MZF_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} MZF_UNICODE_STRING, *PMZF_UNICODE_STRING;

typedef struct _MZF_OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PMZF_UNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} MZF_OBJECT_ATTRIBUTES, *PMZF_OBJECT_ATTRIBUTES;

typedef struct _MZF_CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} MZF_CLIENT_ID;
typedef MZF_CLIENT_ID *PMZF_CLIENT_ID;

#define MzfInitializeObjectAttributes( p, n, a, r, s ) { \
	(p)->Length = sizeof( MZF_OBJECT_ATTRIBUTES );          \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
}

typedef struct _ENUM_VALUE_
{
	PULONG RetLength;
	PVOID pValueInfo;
}ENUM_VALUE, *PENUM_VALUE;

typedef struct _CREATE_KEY_RETURN_
{
	PHANDLE pKeyHandle;
	PULONG  Disposition;
}CREATE_KEY_RETURN, *PCREATE_KEY_RETURN;

typedef struct _COMMUNICATE_KEY_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			ACCESS_MASK DesiredAccess;
			PMZF_OBJECT_ATTRIBUTES ObjectAttributes;
		} Open;

		struct {
			HANDLE hKey;
			ULONG Index;
			ULONG InformationClass;
			ULONG Length;
		} Enum;

		struct {
			HANDLE hKey;
		} Delete;

		struct {
			HANDLE hKey;
			PMZF_UNICODE_STRING ValueName;
			ULONG Type;
			PVOID Data;
			ULONG DataSize;
		} SetValue;

		struct {
			HANDLE hKey;
			PMZF_UNICODE_STRING ValueName;
		} DeleteValue;

		struct {
			HANDLE hKey;
			PMZF_UNICODE_STRING NewName;
		} RenameKey;
	} op;
	
}COMMUNICATE_KEY, *PCOMMUNICATE_KEY;

////////// 以下结构用来文件操作 /////////////

typedef struct _COMMUNICATE_FILE_
{
	OPERATE_TYPE OpType;

	union {
		struct {
			WCHAR *szPath;
		} Delete;

		struct {
			WCHAR *szPath;
			ULONG nPathLen; // 字节大小
			ACCESS_MASK DesiredAccess;
			ULONG  FileAttributes;
			ULONG  ShareAccess;
			ULONG  CreateDisposition;
			ULONG  CreateOptions;
		} Create;

		struct {
			HANDLE hDirectory;
			PVOID  FileInformation;
			ULONG  Length;
			ULONG  FileInformationClass;
			BOOLEAN  ReturnSingleEntry;
			BOOLEAN  RestartScan;
			PULONG pWrittenLen;
		} QueryDirectoryFile;

		struct {
			WCHAR *szPath;
			ULONG nPathLen; // 字节大小
			BOOL bDirectory;
		} QueryFileLock;

		struct {
			ULONG nPid;
			ULONG pEprocess;
			ULONG nHandle;
		} CloseHandle;

		struct {
			HANDLE hFile;
			ULONG Attribute;
		}SetFileAttribute; 

	} op;

}COMMUNICATE_FILE, *PCOMMUNICATE_FILE;

typedef struct _LOCKED_FILES
{	
	ULONG nPid;
	ULONG nHandle;
	WCHAR szFilePath[MAX_PATH * 2];
}LOCKED_FILES, *PLOCKED_FILES;

typedef struct _LOCKED_FILE_INFO
{	
	ULONG nRetCount;
	LOCKED_FILES files[1];
}LOCKED_FILE_INFO, *PLOCKED_FILE_INFO;


////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_DISASSEMBLY_
{
	OPERATE_TYPE OpType;
	ULONG nPid;
	ULONG pEprocess;
	ULONG Base;
	ULONG Size;
}COMMUNICATE_DISASSEMBLY, *PCOMMUNICATE_DISASSEMBLY;

////////////////////////////////////////////////////////////

// typedef struct _COMMUNICATE_DISASSEMBLY_
// {
// 	OPERATE_TYPE OpType;
// 	ULONG nPid;
// 	ULONG pEprocess;
// 	ULONG Base;
// 	ULONG Size;
// }COMMUNICATE_DISASSEMBLY, *PCOMMUNICATE_DISASSEMBLY;

typedef struct _COMMUNICATE_PROCES
{
	OPERATE_TYPE OpType;
	union {
		struct {
			ACCESS_MASK  DesiredAccess;
			PMZF_OBJECT_ATTRIBUTES  ObjectAttributes;
			PMZF_CLIENT_ID  ClientId;
		} Open;

		struct {
			IN HANDLE hProcess;
			IN PVOID lpBaseAddress;
			IN PVOID lpBuffer;
			IN SIZE_T nSize;
		} ReadMemory;

		struct {
			IN HANDLE hProcess;
			IN PVOID lpBaseAddress;
			IN PVOID lpBuffer;
			IN SIZE_T nSize;
		} WriteMemory;

		struct {
			IN HANDLE hProcess;
		} SuspendOrResume;

		struct {
			IN HANDLE hProcess;
			IN LPVOID lpAddress;
			IN SIZE_T dwSize;
			IN DWORD flNewProtect;
		} VirtualProtect;

		struct {
			IN HANDLE ProcessHandle;
			IN PVOID BaseAddress;
			IN ULONG MemoryInformationClass;
			OUT PVOID MemoryInformation;
			IN ULONG MemoryInformationLength;
		}QueryVirtualMemory;

	} op;
	
}COMMUNICATE_PROCES, *PCOMMUNICATE_PROCES;

////////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_UNLOAD_LdrpHashTable_
{
	OPERATE_TYPE OpType;
	ULONG nPid;
	ULONG pEprocess;
	ULONG Base;
	ULONG LdrpHashTable;
}COMMUNICATE_UNLOAD_LdrpHashTable, *PCOMMUNICATE_UNLOAD_LdrpHashTable;

////////////////////////////////////////////////////////////////

typedef enum _PORT_TYPE
{
	enumTcp,
	enumUdp,
}PORT_TYPE;

typedef struct _PORT_INFO_
{
	PORT_TYPE nPortType;
	ULONG nConnectState;
	ULONG nLocalAddress;
	ULONG nLocalPort;
	ULONG nRemoteAddress;
	ULONG nRemotePort;
	ULONG nPid;
}PORT_INFO, *PPORT_INFO;

typedef struct _COMMUNICATE_PORT_
{
	ULONG nCnt;					// 总共有几项
	PORT_INFO Ports[1];
}COMMUNICATE_PORT, *PCOMMUNICATE_PORT;

/////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_MODIFY_KERNEL_ADDRESS_
{
	OPERATE_TYPE OpType;
	ULONG nAddress;
	ULONG nLen;
	PBYTE pContent;
}COMMUNICATE_MODIFY_KERNEL_ADDRESS, *PCOMMUNICATE_MODIFY_KERNEL_ADDRESS;

////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_PID_
{
	OPERATE_TYPE OpType;
	ULONG nPid;
}COMMUNICATE_PID, *PCOMMUNICATE_PID;

////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_REG_LOCK_
{
	OPERATE_TYPE OpType;

	WCHAR *szRegPath; // 注册表路径
	ULONG nPathLen; // 路径字节大小

}COMMUNICATE_REG_LOCK, *PCOMMUNICATE_REG_LOCK;

typedef struct _LOCKED_REGS
{	
	ULONG nPid;
	ULONG nHandle;
	WCHAR szRegPath[MAX_PATH];
}LOCKED_REGS, *PLOCKED_REGS;

typedef struct _LOCKED_REG_INFO
{	
	ULONG nRetCount;
	LOCKED_REGS regs[1];
}LOCKED_REG_INFO, *PLOCKED_REG_INFO;

////////////////////////////////////////////////////////////

#define SELF_PROTECT_SSDT		1
#define SELF_PROTECT_SHADOW		2

typedef struct _COMMUNICATE_SELF_PROTECT_
{
	OPERATE_TYPE OpType;

	union {

		struct {
			ULONG nPid;
		} SetPid;

		struct {
			ULONG nMask; // 1.ssdt, 2.shadow, 3.all
			ULONG bStart;
		} StartOrStop;

	} op;
}COMMUNICATE_SELF_PROTECT, *PCOMMUNICATE_SELF_PROTECT;

////////////////////////////////////////////////////////////

typedef enum _FORBID_TYPE_
{
	eForbid_Unknow,
	eForbid_CreateProcess,
	eForbid_CreateThread,
	eForbid_CreateFile,
	eForbid_SwitchDesktop,
	eForbid_CreateKey,
	eForbid_SetKeyValue,
	eForbid_CreateImage,
	eForbid_Shutdown,
	eForbid_ModifySystemTime,
	eForbid_LockWorkStation,
	eForbid_LoadDriver,
	eForbid_InjectModule,
	eForbid_ScreenCapture,
	eForbid_BackupDriver,
}FORBID_TYPE;

typedef struct _COMMUNICATE_FORBID_
{
	OPERATE_TYPE OpType;
	FORBID_TYPE ForbidType;
	BOOL bDeny;
}COMMUNICATE_FORBID, *PCOMMUNICATE_FORBID;

////////////////////////////////////////////////////////////

typedef enum _POWER_TYPE_
{
	ePower_Unknow,
	ePower_Reboot,
	ePower_Force_Reboot,
	ePower_PowerOff,
}POWER_TYPE;

typedef struct _COMMUNICATE_POWER_
{
	OPERATE_TYPE OpType;
	POWER_TYPE PowerType;
}COMMUNICATE_POWER, *PCOMMUNICATE_POWER;

////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_FILE_OPERATION_
{
	OPERATE_TYPE OpType;

	union {

		struct {
			WCHAR *szFilePath;
			ULONG nPathLen;  // in bytes
			ACCESS_MASK  DesiredAccess;
			ULONG  FileAttributes;
			ULONG  ShareAccess;
			ULONG  CreateDisposition;
			ULONG  CreateOptions;
		} Create;

		struct {
			HANDLE hFile;
			PVOID pBuffer;
			ULONG nReadBytes;
		} Read;

		struct {
			HANDLE hFile;
			PVOID pBuffer;
			ULONG nWriteBytes;
		} Write;

	} op;

}COMMUNICATE_FILE_OPERATION, *PCOMMUNICATE_FILE_OPERATION;

////////////////////////////////////////////////////////////

typedef struct _COMMUNICATE_MBR_
{
	OPERATE_TYPE OpType;

	union {

		struct {
			WCHAR *szDiskName;
			PVOID pBuffer;
			ULONG nReadBytes;
		} Read;

		struct {
			WCHAR *szDiskName;
			PVOID pBuffer;
			ULONG nWriteBytes;
		} Write;

	} op;

}COMMUNICATE_MBR, *PCOMMUNICATE_MBR;

////////////////////////////////////////////////////////////

typedef struct _BACKUP_DRIVER_PATH_
{
	OPERATE_TYPE OpType;
	WCHAR *szPath;
	DWORD dwBytes;
}BACKUP_DRIVER_PATH, *PBACKUP_DRIVER_PATH;

typedef struct _BACKUP_DRIVER_EVENT_
{
	OPERATE_TYPE OpType;
	HANDLE hEvent;
}BACKUP_DRIVER_EVENT, *PBACKUP_DRIVER_EVENT;

////////////////////////////////////////////////////////////

typedef struct _READ_SECTORS_CONTEXT_ {
	OPERATE_TYPE OpType;
	ULONG64 nSectorNum;			// 扇区号
	int nSectorCounts;			// 扇区数
} READ_SECTORS_CONTEXT, *PREAD_SECTORS_CONTEXT;

////////////////////////////////////////////////////////////

#endif
