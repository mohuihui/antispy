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
#ifndef _INIT_WINDOWS_H_
#define _INIT_WINDOWS_H_

#include <ntifs.h>
#include <windef.h>

/////////////////////////////////////////////////////////

typedef enum WIN_VERSION {
	enumWINDOWS_UNKNOW,
	enumWINDOWS_2K,
	enumWINDOWS_XP,
	enumWINDOWS_2K3,
	enumWINDOWS_2K3_SP1_SP2,
	enumWINDOWS_VISTA,
	enumWINDOWS_VISTA_SP1_SP2,
	enumWINDOWS_7,
	enumWINDOWS_8
} WIN_VERSION;

/////////////////////////////////////////////////////////

typedef enum _VERIABLE_INDEX_
{	
	enumRandomKey,
	enumSameThreadApcFlags_Offset_ETHREAD,
	enumPsGetProcessCreateTimeQuadPart,
	enumAtapiParseDiskDriverObject,
	enumReloadAtapiStartIoDispatch,
	enumNtReadFileIndex,
	enumPsLookupThreadByThreadId,
	enumOriginAtapiStartIoDispatch,
	enumIoBuildDeviceIoControlRequest,
	enumBackupDriverPath,
	enumAntiSpyPid,
	enumAntiSpyEprocess,
	enumRtlInitAnsiString,
	enumRtlAnsiStringToUnicodeString,
	enumRtlFreeUnicodeString,
	enumCsrssEprocess,
	enumPsGetCurrentProcessId,
	enumIoFreeIrp,
	enumMmFlushImageSection,
	enumFastfatBase,
	enumFastfatSize,
	enumNtfsBase,
	enumKeInitializeApc,
	enumKeInsertQueueApc,
	enumActiveExWorkerOffset_ETHREAD,
	enumNtfsSize,
	enumExfatBase,
	enumNtCreateFile,
	enumNtWriteFile,
	enumNtReadFile,
	enumExfatSize,
	enumPsThreadType,
	enumPsProcessType,
	enumThreadListHeadOffset_KPROCESS,
	enumCidOffset_ETHREAD,
	enumStartAddressOffset_ETHREAD,
	enumKdVersionBlock,
	enumVadRootOffset_EPROCESS,
	enumPreviousModeOffset_KTHREAD,
	enumStateOffset_KTHREAD,
	enumWindowsVersion,	
	enumPriorityOffset_KTHREAD,
	enumContextSwitchesOffset_KTHREAD,
	enumTebOffset_KTHREAD,
	enumThreadListEntryOffset_KTHREAD,
	enumThreadListEntryOffset_ETHREAD,
	enumThreadsProcessOffset_ETHREAD,
	enumWin32StartAddressOffset_ETHREAD,
	enumBuildNumber,
	enumMmPfnDatabase,
	enumIdleThreadOffset_KPRCB,
	enumPspCidTable,
	enumNtoskrnlPath,
	enumWaitListHeadOffset_KPRCB,
	enumDispatcherReadyListHeadOffset_KPRCB,
	enumWaitInListHead,
	enumThreadListHeadOffset_EPROCESS,
	enumProcessOffset_KAPC_STATE,
	enumObjectHeaderSize,
	enumObjectTypeOffset_OBJECT_HEADER,
	enumSectionObjectOffset_EPROCESS,
	enumDispatcherReadyListHead,
	enumSpyHunterDriverObject,
	enumObjectTableOffset_EPROCESS,
	enumUniqueProcessIdOffset_HANDLE_TABLE,
	enumUniqueProcessIdOffset_EPROCESS,
	enumPebOffset_EPROCESS,
	enumSystemProcess,
	enumInheritedFromUniqueProcessIdOffset_EPROCESS,
	enumWaitListEntryOffset_KTHREAD,
	enumApcStateOffset_KTHREAD,
	enumOriginKernelBase,
	enumKernelModuleSize,
	enumNewKernelBase,					// 新内核的基地址，卸载时需要释放
	enumNewKernelBaseWithoutFix,		// 新内核的基地址, 没有修复过，卸载时需要释放
	enumOriginKiServiceTable,			// 系统原始的SSDT->ServiceTable，也是比较干净的，但可能会被inline hook，卸载时需要释放
	enumOriginSSDT,						// 系统原始的SSDT表, 卸载时需要释放
	enumReloadKiServiceTable,			// Reload Ntos之后,重定位出来的SSDT->ServiceTable,为了防止hook,一般调这里的函数,卸载时需要释放
	enumReloadSSDT,						// Reload Ntos之后,重定位出来的SSDT表,卸载时需要释放
	enumReloadNtosKrnlSuccess,			// 表明Reload Ntos是否成功
	enumNowSSDT,						// 系统中的SSDT表

	enumSsdtFunctionsCnt,
	enumShadowSsdtFunctionsCnt,
	enumSsdtFuncionsNameBuffer,			// 保存ssdt函数名的一段不可分页内存，卸载时需要释放。

	enumOriginWin32kBase,				// 原始的Win32k.sys基地址
	enumNewWin32kBase,					// Reload的Win32k.sys基地址
	enumWin32kSize,						// win32k.sys的大小
	enumReloadWin32kSuccess,			// Reload Win32k.sys是否成功
	enumNowShadowSSDT,					// 当前系统中的Shadow SSDT表
	enumOriginShadowKiServiceTable,		// 系统原始的，未被SSDT hook的Shadow SSDT->ServiceTable，卸载时需要释放
	enumOriginShadowSSDT,				// 系统原始的，未被SSDT hook的Shadow SSDT表，卸载时需要释放
	enumReloadShadowKiServiceTable,		// Reload的Shadow SSDT->ServiceTable，卸载时需要释放
	enumReloadShadowSSDT,				// Reload的Shadow SSDT表，卸载时需要释放
	
	enumMmIsAddressValid,
	enumMmGetVirtualForPhysical,
	enumMmGetPhysicalAddress,
	enumObQueryNameString,
	enumObfDereferenceObject,
	enumIoVolumeDeviceToDosName,
	enumIoQueryFileDosDeviceName,
	enumObOpenObjectByPointer,
	enumObReferenceObjectByPointer,
	enumPsGetThreadId,
	enumPsLookupProcessByProcessId,
	enumIoGetCurrentProcess,
	enumIoThreadToProcess,
	enumExAllocatePoolWithTag,
	enumExFreePoolWithTag,
	enumKeSetEvent,
	enumKeQueryActiveProcessors,
	enumKeInitializeEvent,
	enumKeInitializeDpc,
	enumKeSetTargetProcessorDpc,
	enumKeSetImportanceDpc,
	enumKeInsertQueueDpc,
	enumKeWaitForSingleObject,
	enumKeStackAttachProcess,
	enumKeUnstackDetachProcess,
	enumProbeForRead,
	enumObReferenceObjectByHandle,
	enumObGetObjectType,
	enumPsGetCurrentThread,
	enumPsGetProcessId,
	enumNtQuerySystemInformation,
	enumNtQuerySystemInformationIndex,
	enumNtOpenProcess,
	enumNtQueryVirtualMemory,
	enumNtQueryVirtualMemoryIndex,
	enumNtClose,
	enumNtCloseIndex,
	enumNtOpenDirectoryObject,
	enumNtOpenDirectoryObjectIndex,
	enumNtQueryObject,
	enumNtQueryObjectIndex,
	enumMmIsNonPagedSystemAddressValid,
	enumIoAllocateMdl,
	enumMmBuildMdlForNonPagedPool,
	enumMmMapLockedPagesSpecifyCache,
	enumMmUnmapLockedPages,
	enumIoFreeMdl,
	enumProbeForWrite,
	
// { shadow hook
	enumNtUserGetForegroundWindowIndex,  // 这三个变量的顺序不能乱
	enumOriginNtUserGetForegroundWindow,
	enumFakeNtUserGetForegroundWindow,

	enumNtUserBuildHwndListIndex,
	enumOriginNtUserBuildHwndList,
	enumFakeNtUserBuildHwndList,

	enumNtUserDestroyWindowIndex,
	enumOriginNtUserDestroyWindow,
	enumFakeNtUserDestroyWindow,

	enumNtUserFindWindowExIndex,
	enumOriginNtUserFindWindowEx,
	enumFakeNtUserFindWindowEx,

	enumNtUserMessageCallIndex,
	enumOriginNtUserMessageCall,
	enumFakeNtUserMessageCall,

	enumNtUserPostThreadMessageIndex,
	enumOriginNtUserPostThreadMessage,
	enumFakeNtUserPostThreadMessage,
	
	enumNtUserQueryWindowIndex,
	enumOriginNtUserQueryWindow,
	enumFakeNtUserQueryWindow,
	
	enumNtUserSetParentIndex,
	enumOriginNtUserSetParent,
	enumFakeNtUserSetParent,

	enumNtUserSetWindowLongIndex,
	enumOriginNtUserSetWindowLong,
	enumFakeNtUserSetWindowLong,

	enumNtUserShowWindowIndex,
	enumOriginNtUserShowWindow,
	enumFakeNtUserShowWindow,

	enumNtUserWindowFromPointIndex,
	enumOriginNtUserWindowFromPoint,
	enumFakeNtUserWindowFromPoint,

	enumNtUserAttachThreadInputIndex,
	enumOriginNtUserAttachThreadInput,
	enumFakeNtUserAttachThreadInput,

	enumNtUserPostMessageIndex,
	enumOriginNtUserPostMessage,
	enumFakeNtUserPostMessage,

// }

// { 防止屏幕截图
	enumNtGdiBitBltIndex,
	enumOriginNtGdiBitBlt,
	enumFakeNtGdiBitBlt,

	enumNtGdiStretchBltIndex,
	enumOriginNtGdiStretchBlt,
	enumFakeNtGdiStretchBlt,

	enumNtGdiMaskBltIndex,
	enumOriginNtGdiMaskBlt,
	enumFakeNtGdiMaskBlt,

	enumNtGdiPlgBltIndex,
	enumOriginNtGdiPlgBlt,
	enumFakeNtGdiPlgBlt,

	enumNtGdiAlphaBlendIndex,
	enumOriginNtGdiAlphaBlend,
	enumFakeNtGdiAlphaBlend,

	enumNtGdiTransparentBltIndex,
	enumOriginNtGdiTransparentBlt,
	enumFakeNtGdiTransparentBlt,

	enumNtUserGetClipboardDataIndex,
	enumOriginNtUserGetClipboardData,
	enumFakeNtUserGetClipboardData,

	enumNtGdiGetDCPointIndex,
	enumNtGdiGetDCPoint,

	enumNtGdiGetDCDwordIndex,
	enumNtGdiGetDCDword,
	
	enumNtUserWindowFromPoint,
	enumVMemCache,
// }

	enumNtUserBuildHwndList,
	enumNtUserQueryWindow,
	enumNtUserKillTimer,
	enumNtUserKillTimerIndex,
	enumNtUserValidateTimerCallbackIndex,
	enumNtUserValidateTimerCallback,
	enumNtUserRegisterHotKey,
	enumNtUserRegisterHotKeyIndex,

	enumInterlockedIncrement,
	enumInterlockedDecrement,
	enumAddend,				// 这个是被递加和递减的计数值
	enumFakeWnd,

	enumNtOpenProcessToken,
	enumNtOpenProcessTokenIndex,
	enumNtQueryInformationToken,
	enumNtQueryInformationTokenIndex,
	enumNtUnmapViewOfSection,
	enumNtUnmapViewOfSectionIndex,
	enumNtTerminateThread,

	enumgptmrFirst,                // win32k的一个全局变量，用来枚举进程的定时器
	enumgphkFirst,				   // win32k的一个全局变量，用来枚举系统热键
	enumInitialStackOffset_KTHREAD,
	enumStackLimitOffset_KTHREAD,
	enumKernelStackOffset_KTHREAD,
	enumKernelApcDisableOffset_KTHREAD,
	enumCrossThreadFlagsOffset_ETHREAD,
	enumNtSuspendThread,
	enumNtSuspendThreadIndex,
	enumNtResumeThread,
	enumNtResumeThreadIndex,
	enumSuspendCountOffset_KTHREAD,
	enumFreezeCountOffset_KTHREAD,
	enumNtSetInformationObject,
	enumNtSetInformationObjectIndex,
	enumNtFreeVirtualMemoryIndex,
	enumNtFreeVirtualMemory,
	enumNtProtectVirtualMemory,
	enumFileTypeIndex,
	enumNtProtectVirtualMemoryIndex,
	enumNtAdjustPrivilegesToken,
	enumNtAdjustPrivilegesTokenIndex,
	enumNtTerminateProcess,
// 	enumAntiSpyEprocess,

// { ssdt hook
	enumNtOpenProcessIndex,
	enumOriginNtOpenProcess,
	enumFakeNtOpenProcess,

	enumNtOpenThreadIndex,
	enumOriginNtOpenThread,
	enumFakeNtOpenThread,

	enumNtTerminateProcessIndex,
	enumOriginNtTerminateProcess,
	enumFakeNtTerminateProcess,

	enumNtTerminateThreadIndex,
	enumOriginNtTerminateThread,
	enumFakeNtTerminateThread,

	enumNtDuplicateObjectIndex,
	enumOriginNtDuplicateObject,
	enumFakeNtDuplicateObject,
// }

// { forbid functions

	enumNtCreateProcessIndex,
	enumOriginNtCreateProcess,
	enumFakeNtCreateProcess,

	enumNtCreateProcessExIndex,
	enumOriginNtCreateProcessEx,
	enumFakeNtCreateProcessEx,

	enumNtCreateThreadIndex,
	enumOriginNtCreateThread,
	enumFakeNtCreateThread,

	enumNtCreateFileIndex,
	enumOriginNtCreateFile,
	enumFakeNtCreateFile,

	enumNtUserSwitchDesktopIndex,
	enumOriginNtUserSwitchDesktop,
	enumFakeNtUserSwitchDesktop,

	enumNtCreateKeyIndex,
	enumOriginNtCreateKey,
	enumFakeNtCreateKey,

	enumNtLoadKey2Index,
	enumOriginNtLoadKey2,
	enumFakeNtLoadKey2,

	enumNtReplaceKeyIndex,
	enumOriginNtReplaceKey,
	enumFakeNtReplaceKey,

	enumNtRestoreKeyIndex,
	enumOriginNtRestoreKey,
	enumFakeNtRestoreKey,
	
	enumNtSetValueKeyIndex,
	enumOriginNtSetValueKey,
	enumFakeNtSetValueKey,

	enumCanSetValues,			// 是否可以更改值的内容
	enumIsBackupDriver,			// 是否备份驱动文件
	enumBackupDriverEvent,		// 用来同步备份驱动的事件

	enumNtCreateSectionIndex,
	enumOriginNtCreateSection,
	enumFakeNtCreateSection,

	enumNtInitiatePowerActionIndex,
	enumOriginNtInitiatePowerAction,
	enumFakeNtInitiatePowerAction,

	enumNtSetSystemPowerStateIndex,
	enumOriginNtSetSystemPowerState,
	enumFakeNtSetSystemPowerState,

	enumNtUserCallOneParamIndex,
	enumOriginNtUserCallOneParam,
	enumFakeNtUserCallOneParam,

	enumNtShutdownSystemIndex,
	enumOriginNtShutdownSystem,
	enumFakeNtShutdownSystem,

	enumNtSetSystemTimeIndex,
	enumOriginNtSetSystemTime,
	enumFakeNtSetSystemTime,

	enumNtUserLockWorkStationIndex,
	enumOriginNtUserLockWorkStation,
	enumFakeNtUserLockWorkStation,

	enumNtLoadDriverIndex,
	enumOriginNtLoadDriver,
	enumFakeNtLoadDriver,

	enumRtlImageDirectoryEntryToData,

	enumKeUserModeCallback,
	enumKeUserModeCallbackIatAddress,

	enumNtCreateUserProcessIndex,
	enumOriginNtCreateUserProcess,
	enumFakeNtCreateUserProcess,

	enumNtCreateThreadExIndex,
	enumOriginNtCreateThreadEx,
	enumFakeNtCreateThreadEx,
// }

	enumNtShutdownSystem,
	enumNtSuspendProcess,
	enumNtSuspendProcessIndex,
	enumNtResumeProcess,
	enumNtResumeProcessIndex,
	enumPspTerminateThreadByPointer,
	enumTypeListOffset_OBJECT_TYPE,
	enumIoDeviceObjectType,
	enumIoDriverObjectType,
	enumDirectoryObjectType,
	enumNtoskrnl_KLDR_DATA_TABLE_ENTRY,
	enumNtOpenSymbolicLinkObject,
	enumNtOpenSymbolicLinkObjectIndex,
	enumNtQuerySymbolicLinkObject,
	enumNtQuerySymbolicLinkObjectIndex,
	enumIoCreateFile,
	enumIoFileObjectType,
	enumObMakeTemporaryObject,
	enumPsTerminateSystemThread,
	enumPsCreateSystemThread,
	enumIoDeleteDevice,
	enumSymbolicLinkObjectType,
	enumObOpenObjectByName,
	enumNtOpenKeyIndex,
	enumNtOpenKey,
	enumNtEnumerateKeyIndex,
	enumNtEnumerateKey,
	enumNtEnumerateValueKey,
	enumNtEnumerateValueKeyIndex,
	enumPsSetCreateProcessNotifyRoutine,
	enumPspCreateProcessNotifyRoutine,
	enumPspLoadImageNotifyRoutine,
	enumPsSetLoadImageNotifyRoutine,
	enumPsSetCreateThreadNotifyRoutine,
	enumPspCreateThreadNotifyRoutine,
	enumPsRemoveCreateThreadNotifyRoutine,
	enumPsRemoveLoadImageNotifyRoutine,
	enumIoRegisterShutdownNotification,
	enumIopNotifyShutdownQueueHead,
	enumCmUnRegisterCallback,
	enumCmpCallBackVector,
	enumIoUnregisterShutdownNotification,
	enumKeUpdateSystemTime,
	enumKiTimerTableListHead,
	enumExTimerObjectType,
	enumTIMER_TABLE_SIZE,
	enumKeCancelTimer,
	enumIoInitializeTimer,
	enumIoStartTimer,
	enumIoStopTimer,
	enumIopTimerQueueHead,
	enumExWorkerQueue,
	enumExQueueWorkItem,
	enumQueueListEntryOffset_KTHREAD,
	enumIoplOffset_KPROCESS,
	enumIopmOffsetOffset_KPROCESS,
	enumKe386IoSetAccessProcess,
	enumIoGetDeviceObjectPointer,
	enumObReferenceObjectByName,
	enumPsInitialSystemProcessAddress,
	enumNtOpenFile,
	enumNtCreateSection,
	enumNtMapViewOfSection,
	enumNtOpenFileIndex,
//	enumNtCreateSectionIndex,
	enumNtMapViewOfSectionIndex,
	
	enumNtfsImpMmFlushImageSection,			// NtfsImpMmFlushImageSection函数导入地址
	enumFastfatImpMmFlushImageSection,
	enumOriginNtfsImpMmFlushImageSection,	// 系统中原本的NtfsImpMmFlushImageSection函数地址，可能被hook的地址
	enumOriginFastfatImpMmFlushImageSection,

	enumpIofCompleteRequest,
	enumIopfCompleteRequest,
	enumOriginIopfCompleteRequest,			// 系统中原本的IopfCompleteRequest函数地址，可能被hook的地址

	enumIofCompleteRequest,
	enumNtfsImpIofCompleteRequest,			// NtfsImpIofCompleteRequest函数导入地址
	enumFastfatImpIofCompleteRequest,
	enumOriginNtfsImpIofCompleteRequest,	// 系统中原本的NtfsImpIofCompleteRequest函数地址，可能被hook的地址
	enumOriginFastfatImpIofCompleteRequest,

	enumNtfsDriverObject,
	enumFastfatDriverObject,
	enumNtfsOriginalDispatchs,	// Ntfs原始的Dispatchs例程，卸载时需要释放。
	enumNtfsReloadDispatchs,	// Ntfs Reload的Dispatchs例程，卸载时需要释放。
	enumIopInvalidDeviceRequest,
	enumIoCreateDriver,
	enumReloadNtfsBase,			// 卸载时需要释放

	enumFastfatOriginalDispatchs,	// Fastfat原始的Dispatchs例程，卸载时需要释放。
	enumFastfatReloadDispatchs,		// Fastfat Reload的Dispatchs例程，卸载时需要释放。
	enumReloadFastfatBase,			// 卸载时需要释放

	enumKbdclassBase,
	enumKbdclassSize,
	enumKbdClassDriverObject,
	enumReloadKbdClassBase,			// 卸载时需要释放
	enumKbdClassOriginalDispatchs,	// KbdClass原始的Dispatchs例程，卸载时需要释放。
	enumKbdClassReloadDispatchs,	// KbdClass Reload的Dispatchs例程，卸载时需要释放。
	
	enumMouclassBase,
	enumMouclassSize,
	enumMouClassDriverObject,
	enumReloadMouClassBase,			// 卸载时需要释放
	enumMouClassOriginalDispatchs,	// MouClass原始的Dispatchs例程，卸载时需要释放。
	enumMouClassReloadDispatchs,	// MouClass Reload的Dispatchs例程，卸载时需要释放。

	enumClasspnpBase,
	enumClasspnpSize,
	enumClasspnpDriverObject,
	enumReloadClasspnpBase,			// 卸载时需要释放
	enumClasspnpOriginalDispatchs,	// Classpnp原始的Dispatchs例程，卸载时需要释放。
	enumClasspnpReloadDispatchs,	// Classpnp Reload的Dispatchs例程，卸载时需要释放。

	enumAtaPortSize,
	enumAtaPortBase,

	enumAtapiSize,
	enumAtapiBase,
	enumAtapiDriverObject,
	enumReloadAtapiBase,		// 卸载时需要释放
	enumAtapiOriginalDispatchs,	// Atapi原始的Dispatchs例程，卸载时需要释放。
	enumAtapiReloadDispatchs,   // Atapi Reload的Dispatchs例程，卸载时需要释放。

	enumAcpiSize,
	enumAcpiBase,
	enumAcpiDriverObject,
	enumReloadAcpiBase,			// 卸载时需要释放
	enumAcpiOriginalDispatchs,	// Acpi原始的Dispatchs例程，卸载时需要释放。
	enumAcpiReloadDispatchs,	// Acpi Reload的Dispatchs例程，卸载时需要释放。

	enumTcpipBase,
	enumTcpipSize,
	enumTcpipDriverObject,
	enumReloadTcpipBase,			// 卸载时需要释放
	enumTcpipOriginalDispatchs,		// Tcpip原始的Dispatchs例程，卸载时需要释放。
	enumTcpipReloadDispatchs,		// Tcpip Reload的Dispatchs例程，卸载时需要释放。

	enumOriginIdtTable,		// 原始的Idt表函数地址，卸载时需要释放
	enumReloadIdtTable,		// Reload的Idt表函数地址，卸载时需要释放

	enumDirectoryType,		 // 系统中全部的类型
	enumMutantType,
	enumThreadType,
	enumFilterCommunicationPortType,
	enumControllerType,
	enumProfileType,
	enumEventType,
	enumTypeType,
	enumSectionType,
	enumEventPairType,
	enumSymbolicLinkType,
	enumDesktopType,
	enumTimerType,
	enumFileType,
	enumWindowStationType,
	enumDriverType,
	enumWmiGuidType,
	enumKeyedEventType,
	enumDeviceType,
	enumTokenType,
	enumDebugObjectType,
	enumIoCompletionType,
	enumProcessType,
	enumAdapterType,
	enumKeyType,
	enumJobType,
	enumWaitablePortType,
	enumPortType,
	enumCallbackType,
	enumFilterConnectionPortType,
	enumSemaphoreType,

	enumObGetObjectSecurity,
	enumObCreateObjectType,
	enumSeDefaultObjectMethod,

	enumObpDeleteObjectType, // ObpTypeObjectType

	enumObpDeleteSymbolicLink, // ObpSymbolicLinkObjectType
	enumObpParseSymbolicLink,

	enumIopParseDevice, // IoDeviceObjectType
	enumIopDeleteDevice,
	enumIopGetSetSecurityObject,

	enumIopDeleteDriver, // IoDriverObjectType

	enumIopDeleteIoCompletion, // IoCompletionObjectType
	enumIopCloseIoCompletion,

	enumIopCloseFile,   // IoFileObjectType
	enumIopDeleteFile,
	enumIopParseFile,
	enumIopFileGetSetSecurityObject,
	enumIopQueryName,

	enumLpcpClosePort, // LpcWaitablePortObjectType || LpcPortObjectType
	enumLpcpDeletePort,
	enumEtwpOpenRegistrationObject,

	enumMiSectionDelete, // MmSectionObjectType
	
	enumPspProcessDelete, // PsProcessType
	enumPspProcessOpen,
	enumPspProcessClose,

	enumPspThreadDelete, // PsThreadType
	enumPspThreadOpen,

	enumPspJobDelete, // PsJobType
	enumPspJobClose,

	enumSepTokenDeleteMethod, // SeTokenObjectType

	enumWmipSecurityMethod, // WmipGuidObjectType
	enumWmipDeleteMethod,
	enumWmipCloseMethod,

	enumExpWin32CloseProcedure, // ExWindowStationObjectType || ExDesktopObjectType
	enumExpWin32DeleteProcedure,
	enumExpWin32OkayToCloseProcedure,
	enumExpWin32ParseProcedure, // ExDesktopObjectType 无此函数
	enumExpWin32OpenProcedure,
	
	enumTimerDeleteProcedure, // ExTimerObjectType

	enumExpDeleteMutant, // ExMutantObjectType

	enumExpProfileDelete, // ExProfileObjectType

	enumCmpCloseKeyObject, // CmpKeyObjectType
	enumCmpDeleteKeyObject,
	enumCmpParseKey,
	enumCmpSecurityMethod,
	enumCmpQueryKeyName,

	enumxHalLocateHiberRanges, // DbgkDebugObjectType || ExCallbackObjectType 的Delete Procedure
	enumDbgkpCloseObject,
	
	enumObpCloseDirectoryObject, // ObpDirectoryObjectType
	enumObpDeleteDirectoryObject,

	enumGetObjectTypeDispatchOk, // 是否已经获取了上述信息

	enumKiFastCallEntry,

	enumNewHalDllBaseWithoutFix,		// 新Hal.Dll的基地址, 没有修复过，卸载时需要释放
	enumOriginHalDllBase,				// 原始的Hal.Dll基地址
	enumNewHalDllBase,					// Reload的Hal.Dll基地址
	enumHalDllSize,						// Hal.Dll的大小

	enumObCreateObjectTypeEx,

	enumSectionBaseAddressOffset_EPROCESS,

	enumNtDeleteKeyIndex,
	enumNtDeleteKey,
	enumNtCreateKey,
// 	enumNtCreateKeyIndex,
	enumNtSetValueKey,
//	enumNtSetValueKeyIndex,
	enumNtDeleteValueKey,
	enumNtDeleteValueKeyIndex,

	enumDebugportOffset_EPROCESS,

	enumNtReadVirtualMemory,
	enumNtWriteVirtualMemory,
	enumNtReadVirtualMemoryIndex,
	enumNtWriteVirtualMemoryIndex,
	enumNtRenameKeyIndex,
	enumNtRenameKey,
	
	enumNtWriteFileIndex,
}VERIABLE_INDEX;	

VOID SetGlobalVeriable(VERIABLE_INDEX Index, ULONG Value);
ULONG GetGlobalVeriable(VERIABLE_INDEX Index);
ULONG GetGlobalVeriableAddress(VERIABLE_INDEX Index);
ULONG *GetGlobalVeriable_Address(VERIABLE_INDEX Index);

/////////////////////////////////////////////////////////

BOOL InitWindows(PDRIVER_OBJECT DriverObject);
VOID UnInitProcessVariable();
VOID InitSSDTFunctions();
NTSTATUS CheckUpdate();

/////////////////////////////////////////////////////////

#endif



