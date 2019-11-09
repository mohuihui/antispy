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
#ifndef _STRUCT_H_
#define _STRUCT_H_

#include <ntifs.h>
#include <windef.h>
#include <ntiologc.h>
#include <ntimage.h>

/////////////////////////////////////////////////////////////////////////

#define		MZFTAG					'MzF'
#define		MAX_PATH_WCHAR			MAX_PATH * sizeof(WCHAR)
#define		SYSTEM_ADDRESS_START	0x80000000
#define		MAX_IDT					0x100

/////////////////////////////////////////////////////////////////////////

#define LDRP_RELOCATION_INCREMENT   0x1
#define LDRP_RELOCATION_FINAL       0x2
#define RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK (0x00000001)

#define LDR_IS_DATAFILE(x)              (((ULONG_PTR)(x)) &  (ULONG_PTR)1)
#define LDR_DATAFILE_TO_VIEW(x) ((PVOID)(((ULONG_PTR)(x)) & ~(ULONG_PTR)1))

#define		SEC_IMAGE				0x1000000
#define		MEM_IMAGE				SEC_IMAGE 

#define NUMBER_HASH_BUCKETS 37

/////////////////////////////////////////////////////////////////////////

extern POBJECT_TYPE *IoDriverObjectType;
extern POBJECT_TYPE *IoDeviceObjectType;
extern POBJECT_TYPE *LpcPortObjectType;
extern POBJECT_TYPE *PsProcessType;
extern POBJECT_TYPE *CmpKeyObjectType;
extern POBJECT_TYPE *IoFileObjectType;
extern POBJECT_TYPE *ExEventObjectType;
extern POBJECT_TYPE *ExSemaphoreObjectType;
extern POBJECT_TYPE *TmTransactionManagerObjectType;
extern POBJECT_TYPE *TmResourceManagerObjectType;
extern POBJECT_TYPE *TmEnlistmentObjectType;
extern POBJECT_TYPE *TmTransactionObjectType;
extern POBJECT_TYPE *PsThreadType;
extern POBJECT_TYPE *SeTokenObjectType;
extern POBJECT_TYPE *MmSectionObjectType;

/////////////////////////////////////////////////////////////////////////

#pragma pack(1)	
typedef struct ServiceDescriptorEntry {
	unsigned int *ServiceTableBase;			// 系统服务表
	unsigned int *ServiceCounterTableBase; 
	unsigned int NumberOfServices;			// 系统服务函数个数
	unsigned char *ParamTableBase;			// 参数表
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;
#pragma pack()

__declspec(dllimport) ServiceDescriptorTableEntry_t KeServiceDescriptorTable;

///////////////////////////////////////////////

typedef struct _CONTROL_AREA
{
	PVOID Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _SEGMENT
{
	struct _CONTROL_AREA *ControlArea;
	ULONG TotalNumberOfPtes;
	ULONG NonExtendedPtes;
	ULONG Spare0;
} SEGMENT, *PSEGMENT;

typedef struct _SECTION_OBJECT
{
	PVOID StartingVa;
	PVOID EndingVa;
	PVOID Parent;
	PVOID LeftChild;
	PVOID RightChild;
	PSEGMENT Segment;
} SECTION_OBJECT, *PSECTION_OBJECT;

typedef struct _HANDLE_TABLE_ENTRY {
	union {
		PVOID Object;
		ULONG ObAttributes;
		PVOID InfoTable;
		ULONG_PTR Value;
	};
	union {
		union {
			ACCESS_MASK GrantedAccess;
			struct {

				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};
		LONG NextFreeTableEntry;
	};
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _AUX_ACCESS_DATA {
	PPRIVILEGE_SET PrivilegesUsed;
	GENERIC_MAPPING GenericMapping;
	ACCESS_MASK AccessesToAudit;
	ACCESS_MASK MaximumAuditMask;
	ULONG Unknown[41];
} AUX_ACCESS_DATA, *PAUX_ACCESS_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union {
		struct {
			ULONG TimeDateStamp;
		};
		struct {
			PVOID LoadedImports;
		};
	};
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG  Reserved[2];  
	ULONG  Base;        
	ULONG  Size;         
	ULONG  Flags;        
	USHORT Index;       
	USHORT Unknown;     
	USHORT LoadCount;   
	USHORT ModuleNameOffset;
	CHAR   ImageName[256];   
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _tagSysModuleList {
	ULONG ulCount;
	SYSTEM_MODULE_INFORMATION smi[1];
} MODULES, *PMODULES;

///////////////////////////////////////////////////////

typedef struct _SEGMENT_WIN8_
{
	PVOID ControlArea;				// +0x000 
	ULONG TotalNumberOfPtes;		// +0x004 
	ULONG SegmentFlags;				// +0x008 
	ULONG NumberOfCommittedPages;	// +0x00c 
	LARGE_INTEGER SizeOfSegment;	// +0x010 
	PVOID BasedAddress;				// +0x018 
	ULONG SegmentLock;				// +0x01c 
	ULONG u1;						// +0x020 
	ULONG SystemImageBase;			// +0x024 			
}SEGMENT_WIN8, *PSEGMENT_WIN8;

typedef struct _CONTROL_AREA_WIN8_
{
	struct _SEGMENT_WIN8_ *Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA_WIN8, *PCONTROL_AREA_WIN8;

typedef struct _SUBSECTION_WIN8_
{
	PCONTROL_AREA_WIN8 ControlArea;
}SUBSECTION_WIN8, *PSUBSECTION_WIN8;

typedef struct _MMVAD_WIN8_
{
	struct _MMVAD_WIN8_ *Parent;
	struct _MMVAD_WIN8_ *LeftChild;
	struct _MMVAD_WIN8_ *RightChild;
	ULONG StartingVpn;
	ULONG EndingVpn;
	ULONG PushLock;
	ULONG u;
	ULONG u1;
	ULONG EventList;
	ULONG ReferenceCuont;
	ULONG u2;
	PSUBSECTION_WIN8 Subsection;
}MMVAD_WIN8, *PMMVAD_WIN8;

/////////////////////////////////////////////////

typedef struct _SEGMENT_VISTA_
{
	PVOID ControlArea;					// +0x000
	ULONG TotalNumberOfPtes;			// +0x004
	ULONG NonExtendedPtes;				// +0x008
	ULONG NumberOfCommittedPages;		// +0x00c
	LARGE_INTEGER SizeOfSegment;		// +0x010
	PVOID BasedAddress;					// +0x018
	ULONG SegmentLock;					// +0x01c
	ULONG SegmentFlags;					// +0x020
	ULONG u1;							// +0x024
	ULONG SystemImageBase;				// +0x028
}SEGMENT_VISTA, *PSEGMENT_VISTA;

typedef struct _CONTROL_AREA_VISTA_
{
	struct _SEGMENT_VISTA_ *Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA_VISTA, *PCONTROL_AREA_VISTA;

typedef struct _SUBSECTION_
{
	PCONTROL_AREA_VISTA ControlArea;
}SUBSECTION, *PSUBSECTION;

typedef struct _MMVAD_VISTA_
{
	struct _MMVAD_VISTA_ *Parent;
	struct _MMVAD_VISTA_ *LeftChild;
	struct _MMVAD_VISTA_ *RightChild;
	ULONG StartingVpn;
	ULONG EndingVpn;
	ULONG NumberGenericTableElements;
	ULONG PushLock;
	ULONG u5;
	ULONG u2;
	PSUBSECTION Subsection;
}MMVAD_VISTA, *PMMVAD_VISTA;

/////////////////////////////////////////////////

typedef struct _SEGMENT_2003_
{
	PVOID ControlArea;
	ULONG TotalNumberOfPtes;
	ULONG NonExtendedPtes;
	ULONG WritableUserReferences;
	LARGE_INTEGER SizeOfSegment;
	LARGE_INTEGER SegmentPteTemplate;
	ULONG NumberOfCommittedPages;
	PVOID ExtendInfo;
	PVOID SegmentFlags;
	PVOID BasedAddress;
}SEGMENT_2003, *PSEGMENT_2003;

typedef struct _CONTROL_AREA_2003_
{
	struct _SEGMENT_2003_ *Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA_2003, *PCONTROL_AREA_2003;

typedef struct _MMVAD_2003_
{
	struct _MMVAD_2003_ *Parent;
	struct _MMVAD_2003_ *LeftChild;
	struct _MMVAD_2003_ *RightChild;
	ULONG StartingVpn;
	ULONG EndingVpn;
	ULONG NumberGenericTableElements;
	PCONTROL_AREA_2003 ControlArea;
}MMVAD_2003, *PMMVAD_2003;

/////////////////////////////////////////////////

typedef struct _SEGMENT_XP_
{
	PVOID ControlArea;					// +0x000
	ULONG TotalNumberOfPtes;			// +0x004
	ULONG NonExtendedPtes;				// +0x008
	ULONG WritableUserReferences;		// +0x00c
	LARGE_INTEGER SizeOfSegment;		// +0x010 
	LARGE_INTEGER SegmentPteTemplate;	// +0x018
	ULONG NumberOfCommittedPages;		// +0x020
	PVOID ExtendInfo;					// +0x024
	PVOID SystemImageBase;				// +0x028
	PVOID BasedAddress;					// +0x02c
}SEGMENT_XP, *PSEGMENT_XP;

typedef struct _CONTROL_AREA_XP_
{
	struct _SEGMENT_XP_ *Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA_XP, *PCONTROL_AREA_XP;

typedef struct _MMVAD_XP_
{
	ULONG StartingVpn;
	ULONG EndingVpn;
	struct _MMVAD_XP_ *Parent;
	struct _MMVAD_XP_ *LeftChild;
	struct _MMVAD_XP_ *RightChild;
	ULONG NumberGenericTableElements;
	PCONTROL_AREA_XP ControlArea;
}MMVAD_XP, *PMMVAD_XP;

/////////////////////////////////////////////////

typedef struct _SEGMENT_2000_
{
	PVOID ControlArea;				// +0x000
	ULONG TotalNumberOfPtes;		// +0x004
	ULONG NonExtendedPtes;			// +0x008
	ULONG WritableUserReferences;	// +0x00c
	LARGE_INTEGER SizeOfSegment;	// +0x010 
	ULONG SegmentPteTemplate;		// +0x018
	ULONG NumberOfCommittedPages;	// +0x01c
	PVOID ExtendInfo;				// +0x020
	PVOID SystemImageBase;			// +0x024
	PVOID BasedAddress;				// +0x028
}SEGMENT_2000, *PSEGMENT_2000;

typedef struct _CONTROL_AREA_2000_
{
	struct _SEGMENT_2000_ *Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA_2000, *PCONTROL_AREA_2000;

typedef struct _MMVAD_2000_
{
	ULONG StartingVpn;
	ULONG EndingVpn;
	struct _MMVAD_2000_ *Parent;
	struct _MMVAD_2000_ *LeftChild;
	struct _MMVAD_2000_ *RightChild;
	ULONG NumberGenericTableElements;
	PCONTROL_AREA_2000 ControlArea;
}MMVAD_2000, *PMMVAD_2000;

typedef enum _SYSTEM_INFORMATION_CLASS {    
	SystemBasicInformation,    
	SystemProcessorInformation,    
	SystemPerformanceInformation,    
	SystemTimeOfDayInformation,    
	SystemNotImplemented1,    
	SystemProcessesInformation,    
	SystemCallCounts,    
	SystemConfigurationInformation,    
	SystemProcessorTimes,    
	SystemGlobalFlag,    
	SystemNotImplemented2,    
	SystemModuleInformation,    
	SystemLockInformation,    
	SystemNotImplemented3,    
	SystemNotImplemented4,    
	SystemNotImplemented5,    
	SystemHandleInformation, //枚举系统中的全部句柄 
	SystemObjectInformation,    
	SystemPagefileInformation,    
	SystemInstructionEmulationCounts,    
	SystemInvalidInfoClass1,    
	SystemCacheInformation,    
	SystemPoolTagInformation,    
	SystemProcessorStatistics,    
	SystemDpcInformation,    
	SystemNotImplemented6,    
	SystemLoadImage,    
	SystemUnloadImage,    
	SystemTimeAdjustment,    
	SystemNotImplemented7,    
	SystemNotImplemented8,    
	SystemNotImplemented9,    
	SystemCrashDumpInformation,    
	SystemExceptionInformation,    
	SystemCrashDumpStateInformation,    
	SystemKernelDebuggerInformation,    
	SystemContextSwitchInformation,    
	SystemRegistryQuotaInformation,    
	SystemLoadAndCallImage,    
	SystemPrioritySeparation,    
	SystemNotImplemented10,    
	SystemNotImplemented11,    
	SystemInvalidInfoClass2,    
	SystemInvalidInfoClass3,    
	SystemTimeZoneInformation,    
	SystemLookasideInformation,    
	SystemSetTimeSlipEvent,    
	SystemCreateSession,    
	SystemDeleteSession,    
	SystemInvalidInfoClass4,    
	SystemRangeStartInformation,    
	SystemVerifierInformation,    
	SystemAddVerifier,    
	SystemSessionProcessesInformation    
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;  

#if defined(_WIN64)
typedef ULONG SYSINF_PAGE_COUNT;
#else
typedef SIZE_T SYSINF_PAGE_COUNT;
#endif

typedef struct _SYSTEM_BASIC_INFORMATION {
	ULONG Reserved;
	ULONG TimerResolution;
	ULONG PageSize;
	SYSINF_PAGE_COUNT NumberOfPhysicalPages;
	SYSINF_PAGE_COUNT LowestPhysicalPageNumber;
	SYSINF_PAGE_COUNT HighestPhysicalPageNumber;
	ULONG AllocationGranularity;
	ULONG_PTR MinimumUserModeAddress;
	ULONG_PTR MaximumUserModeAddress;
	ULONG_PTR ActiveProcessorsAffinityMask;
	CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_THREAD {    
	LARGE_INTEGER           KernelTime;    
	LARGE_INTEGER           UserTime;    
	LARGE_INTEGER           CreateTime;    
	ULONG                   WaitTime;    
	PVOID                   StartAddress;    
	CLIENT_ID               ClientId;    
	KPRIORITY               Priority;    
	LONG                    BasePriority;    
	ULONG                   ContextSwitchCount;    
	ULONG                   State;    
	KWAIT_REASON            WaitReason;    
} SYSTEM_THREAD, *PSYSTEM_THREAD;    

typedef struct _SYSTEM_PROCESS_INFORMATION {    
	ULONG                   NextEntryOffset; //NextEntryDelta 构成结构序列的偏移量 
	ULONG                   NumberOfThreads; //线程数目 
	LARGE_INTEGER           Reserved[3];    
	LARGE_INTEGER           CreateTime;   //创建时间 
	LARGE_INTEGER           UserTime;     //用户模式(Ring 3)的CPU时间 
	LARGE_INTEGER           KernelTime;   //内核模式(Ring 0)的CPU时间 
	UNICODE_STRING          ImageName;    //进程名称 
	KPRIORITY               BasePriority; //进程优先权 
	HANDLE                  ProcessId;    //ULONG UniqueProcessId 进程标识符 
	HANDLE                  InheritedFromProcessId; //父进程的标识符 
	ULONG                   HandleCount; //句柄数目 
	ULONG                   Reserved2[2];    
	ULONG                   PrivatePageCount;    
	VM_COUNTERS             VirtualMemoryCounters; //虚拟存储器的结构 
	IO_COUNTERS             IoCounters; //IO计数结构 
	SYSTEM_THREAD           Threads[0]; //进程相关线程的结构数组 
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;   

typedef enum _MEMORY_INFORMATION_CLASS
{
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName
}MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_BASIC_INFORMATION {
	PVOID BaseAddress;
	PVOID AllocationBase;
	DWORD AllocationProtect;
	SIZE_T RegionSize;
	DWORD State;
	DWORD Protect;
	DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

typedef struct _WIN2K_HANDLE_TABLE
{
	ULONG Flags;
	LONG HandleCount;
	PHANDLE_TABLE_ENTRY **Table;
	PEPROCESS QuotaProcess;
	HANDLE UniqueProcessId;
	LONG FirstFreeTableEntry;
	LONG NextIndexNeedingPool;
	ERESOURCE HandleTableLock;
	LIST_ENTRY HandleTableList;
	KEVENT HandleContentionEvent;
}WIN2K_HANDLE_TABLE , *PWIN2K_HANDLE_TABLE ;

typedef struct _MY_PEB {
	BOOLEAN InheritedAddressSpace; 
	BOOLEAN ReadImageFileExecOptions;   
	BOOLEAN BeingDebugged;             
	union {
		BOOLEAN BitField;            
		struct {
			BOOLEAN ImageUsesLargePages : 1;
			BOOLEAN SpareBits : 7;
		};
	};
	LONG Mutant;
	PVOID ImageBaseAddress;
	PVOID Ldr;
	PVOID ProcessParameters;
} MY_PEB, *PMY_PEB;

typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	HANDLE SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID EntryInProgress;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef enum _OBJECT_INFO_CLASS {
	ObjectBasicInfo,
	ObjectNameInfo,
	ObjectTypeInfo,
	ObjectAllInfo,
	ObjectDataInfo
} OBJECT_INFO_CLASS, *POBJECT_INFO_CLASS;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO{
	USHORT UniqueProcessId;
	USHORT CreatorBackTraceIndex;
	UCHAR ObjectTypeIndex;
	UCHAR HandleAttributes;
	USHORT HandleValue;
	PVOID Object;
	ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING          TypeName;
	ULONG                   TotalNumberOfHandles;
	ULONG                   TotalNumberOfObjects;
	WCHAR                   Unused1[8];
	ULONG                   HighWaterNumberOfHandles;
	ULONG                   HighWaterNumberOfObjects;
	WCHAR                   Unused2[8];
	ACCESS_MASK             InvalidAttributes;
	GENERIC_MAPPING         GenericMapping;
	ACCESS_MASK             ValidAttributes;
	BOOLEAN                 SecurityRequired;
	BOOLEAN                 MaintainHandleCount;
	USHORT                  MaintainTypeList;
	POOL_TYPE               PoolType;
	ULONG                   DefaultPagedPoolCharge;
	ULONG                   DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_DATA_INFORMATION {
	BOOLEAN                 InheritHandle;
	BOOLEAN                 ProtectFromClose;
} OBJECT_DATA_INFORMATION, *POBJECT_DATA_INFORMATION;

typedef struct _OBJECT_DIRECTORY_ENTRY
{
	struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
	PVOID Object;
	ULONG HashValue;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;

typedef struct _OBJECT_DIRECTORY
{
	struct _OBJECT_DIRECTORY_ENTRY *HashBuckets[NUMBER_HASH_BUCKETS];
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;

typedef struct _OBJECT_TYPE_XP_
{
	ERESOURCE Mutex;
	LIST_ENTRY TypeList;
	UNICODE_STRING Name;
	PVOID DefaultObject;
	ULONG Index;
} OBJECT_TYPE_XP, *POBJECT_TYPE_XP;

typedef struct _OBJECT_TYPE_2008ABOVE_
{
	LIST_ENTRY TypeList;
	UNICODE_STRING Name;
	PVOID DefaultObject;
	UCHAR Index;
} OBJECT_TYPE_2008ABOVE, *POBJECT_TYPE_2008ABOVE;

typedef enum _OBJECT_INFORMATION_CLASS_ {
	BasicInformation,
	NameInformation,
	TypeInformation,
	TypesInformation,
	HandleFlagInformation,
	SessionInformation,
	MaxObjectInfoClass  // MaxObjectInfoClass should always be the last enum
} SH_OBJECT_INFORMATION_CLASS;

typedef struct _KLDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	PVOID ExceptionTable;
	ULONG ExceptionTableSize;
	PVOID GpValue;
	DWORD UnKnow;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Unused5;
	PVOID SectionPointer;
	ULONG CheckSum;
	PVOID LoadedImports;
	PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;

typedef struct _OBJECT_HEADER_CREATOR_INFO {
	LIST_ENTRY TypeList;
	HANDLE CreatorUniqueProcess;
	USHORT CreatorBackTraceIndex;
	USHORT Reserved;
} OBJECT_HEADER_CREATOR_INFO, *POBJECT_HEADER_CREATOR_INFO;

typedef struct _KTIMER_TABLE_ENTRY {
	LIST_ENTRY Entry;
	ULARGE_INTEGER Time;
} KTIMER_TABLE_ENTRY, *PKTIMER_TABLE_ENTRY;

typedef struct _IO_TIMER {
	CSHORT Type;
	CSHORT TimerFlag;
	LIST_ENTRY TimerList;
	PIO_TIMER_ROUTINE TimerRoutine;
	PVOID Context;
	struct _DEVICE_OBJECT *DeviceObject;
} IO_TIMER, *PIO_TIMER;

typedef union {
	struct {

#define EX_WORKER_QUEUE_DISABLED    0x80000000

		ULONG QueueDisabled :  1;

		//
		// MakeThreadsAsNecessary indicates whether this work queue is eligible
		// for dynamic creation of threads not just for deadlock detection,
		// but to ensure that the CPUs are all kept busy clearing any work
		// item backlog.
		//

		ULONG MakeThreadsAsNecessary : 1;

		ULONG WaitMode : 1;

		ULONG WorkerCount   : 29;
	};
	LONG QueueWorkerInfo;
} EX_QUEUE_WORKER_INFO;

typedef struct _EX_WORK_QUEUE {

	//
	// Queue objects that that are used to hold work queue entries and
	// synchronize worker thread activity.
	//

	KQUEUE WorkerQueue;

	//
	// Number of dynamic worker threads that have been created "on the fly"
	// as part of worker thread deadlock prevention
	//

	ULONG DynamicThreadCount;

	//
	// Count of the number of work items processed.
	//

	ULONG WorkItemsProcessed;

	//
	// Used for deadlock detection, WorkItemsProcessedLastPass equals the value
	// of WorkItemsProcessed the last time ExpDetectWorkerThreadDeadlock()
	// ran.
	//

	ULONG WorkItemsProcessedLastPass;

	//
	// QueueDepthLastPass is also part of the worker queue state snapshot
	// taken by ExpDetectWorkerThreadDeadlock().
	//

	ULONG QueueDepthLastPass;

	EX_QUEUE_WORKER_INFO Info;

} EX_WORK_QUEUE, *PEX_WORK_QUEUE;

#pragma pack(push,1)

typedef struct tagGDTR{
	WORD    wLimit;
	DWORD   *dwBase;
}GDTR, *PGDTR;

typedef struct tagGDTENTRY{
	DWORD dwLimit : 16;
	DWORD dwBaselo : 16;
	DWORD dwBasemid : 8;
	DWORD dwType : 4;
	DWORD dwSystem : 1;
	DWORD dwDpl : 2;
	DWORD dwPresent : 1;
	DWORD dwLimithi : 4;
	DWORD dwAvailable : 1;
	DWORD dwZero : 1;
	DWORD dwSize : 1;
	DWORD dwGranularity : 1;
	DWORD dwBasehi : 8;
} GDTENTRY, *PGDTENTRY;

#pragma pack(pop)

#define NOT_BUSY            9
#define DEFAULT_TSS_LIMIT   0x20ab						// 缺省的 TSS 段界限
#define DEFAULT_IOPL_OFFSET  DEFAULT_TSS_LIMIT + 1		// 缺省的IopmOffset	

#define IOPM_COUNT      1           // Number of i/o access maps that exist (in addition to IO_ACCESS_MAP_NONE)
#define IO_ACCESS_MAP_NONE 0


////////////////////////////////////////////////////////////////////////

NTSTATUS        
ZwQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,    
						 OUT PVOID SystemInformation,    
						 IN ULONG SystemInformationLength,    
						 OUT PULONG ReturnLength OPTIONAL);

NTSTATUS 
ObReferenceObjectByName ( 
						 IN PUNICODE_STRING ObjectName, 
						 IN ULONG Attributes, 
						 IN PACCESS_STATE AccessState OPTIONAL, 
						 IN ACCESS_MASK DesiredAccess OPTIONAL, 
						 IN POBJECT_TYPE ObjectType, 
						 IN KPROCESSOR_MODE AccessMode, 
						 IN OUT PVOID ParseContext OPTIONAL, 
						 OUT PVOID *Object 
						 ) ;

NTKERNELAPI
NTSTATUS
SeCreateAccessState(
					PACCESS_STATE AccessState,
					PAUX_ACCESS_DATA AuxData,
					ACCESS_MASK DesiredAccess,
					PGENERIC_MAPPING GenericMapping
					);

NTKERNELAPI
VOID
SeDeleteAccessState(
					PACCESS_STATE AccessState
					);

NTKERNELAPI				
NTSTATUS
ObCreateObject(
			   IN KPROCESSOR_MODE ProbeMode,
			   IN POBJECT_TYPE ObjectType,
			   IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
			   IN KPROCESSOR_MODE OwnershipMode,
			   IN OUT PVOID ParseContext OPTIONAL,
			   IN ULONG ObjectBodySize,
			   IN ULONG PagedPoolCharge,
			   IN ULONG NonPagedPoolCharge,
			   OUT PVOID *Object
			   );

////////////////////////////////////////////////////////////////////////////////////

typedef 
NTSTATUS 
(*pfnNtQueryVirtualMemory)(
						   __in HANDLE ProcessHandle,
						   __in PVOID BaseAddress,
						   __in MEMORY_INFORMATION_CLASS MemoryInformationClass,
						   __out_bcount(MemoryInformationLength) PVOID MemoryInformation,
						   __in SIZE_T MemoryInformationLength,
						   __out_opt PSIZE_T ReturnLength
						   );
typedef
HANDLE
(*pfnPsGetThreadId)(
					__in PETHREAD Thread
					);
typedef 
NTSTATUS 
(* pfnRtlGetVersion)(
					OUT PRTL_OSVERSIONINFOW lpVersionInformation
					);
typedef
NTSTATUS
(*pfnIoQueryFileDosDeviceName) (
								__in  PFILE_OBJECT FileObject,
								__out POBJECT_NAME_INFORMATION *ObjectNameInformation
								);
typedef
NTSTATUS
(*pfnIoVolumeDeviceToDosName) (
							   __in  PVOID           VolumeDeviceObject,
							   __out PUNICODE_STRING DosName
							   );
typedef 
POBJECT_TYPE 
(*pfnObGetObjectType)(
					PVOID pObject
					);
typedef
PVOID
(*pfnExAllocatePoolWithTag)(
					  __in __drv_strictTypeMatch(__drv_typeExpr) POOL_TYPE PoolType,
					  __in SIZE_T NumberOfBytes,
					  __in ULONG Tag
					  );
typedef
BOOLEAN
(*pfnMmIsAddressValid) (
				  __in PVOID VirtualAddress
				  );
typedef
LONG 
(*pfnKeSetEvent)(
		   IN PRKEVENT  Event,
		   IN KPRIORITY  Increment,
		   IN BOOLEAN  Wait
		   );
typedef
KAFFINITY
(*pfnKeQueryActiveProcessors)(
						VOID
						);
typedef
VOID 
(*pfnKeInitializeEvent)(
				  IN PRKEVENT  Event,
				  IN EVENT_TYPE  Type,
				  IN BOOLEAN  State
				  );
typedef
VOID 
(*pfnKeInitializeDpc)(
				IN PRKDPC  Dpc,
				IN PKDEFERRED_ROUTINE  DeferredRoutine,
				IN PVOID  DeferredContext
				);
typedef
VOID
(*pfnKeSetTargetProcessorDpc)(
						IN PRKDPC  Dpc,
						IN CCHAR  Number
						);
typedef
VOID
(*pfnKeSetImportanceDpc)(
				   IN PRKDPC  Dpc,
				   IN KDPC_IMPORTANCE  Importance
				   );
typedef
BOOLEAN 
(*pfnKeInsertQueueDpc)(
				 IN PRKDPC  Dpc,
				 IN PVOID  SystemArgument1,
				 IN PVOID  SystemArgument2
				 );
typedef
NTSTATUS 
(*pfnKeWaitForSingleObject)(
					  IN PVOID  Object,
					  IN KWAIT_REASON  WaitReason,
					  IN KPROCESSOR_MODE  WaitMode,
					  IN BOOLEAN  Alertable,
					  IN PLARGE_INTEGER  Timeout OPTIONAL
					  );
typedef
PEPROCESS 
(*pfnIoGetCurrentProcess)(
					VOID
					);
typedef
VOID
(*pfnKeStackAttachProcess) (
					  IN PKPROCESS  Process,
					  OUT PRKAPC_STATE  ApcState
					  );
typedef
VOID
(*pfnKeUnstackDetachProcess)(
					   IN PRKAPC_STATE  ApcState
					   );
typedef
VOID 
(*pfnProbeForRead)(
			 IN CONST VOID  *Address,
			 IN SIZE_T  Length,
			 IN ULONG  Alignment
			 );
typedef
VOID 
(*pfnProbeForWrite)(
			  IN CONST VOID  *Address,
			  IN SIZE_T  Length,
			  IN ULONG  Alignment
			  );

typedef
NTSTATUS 
(*pfnObReferenceObjectByHandle)(
						  IN HANDLE  Handle,
						  IN ACCESS_MASK  DesiredAccess,
						  IN POBJECT_TYPE  ObjectType  OPTIONAL,
						  IN KPROCESSOR_MODE  AccessMode,
						  OUT PVOID  *Object,
						  OUT POBJECT_HANDLE_INFORMATION  HandleInformation  OPTIONAL
						  );
typedef
LONG_PTR
(FASTCALL *pfnObfDereferenceObject)(
					 __in PVOID Object
					 );

typedef
PEPROCESS
(*pfnIoThreadToProcess)(
				  IN PETHREAD  Thread
				  ); 
typedef
NTSTATUS        
(*pfnNtQuerySystemInformation)(
							   IN SYSTEM_INFORMATION_CLASS SystemInformationClass,    
							   OUT PVOID SystemInformation,    
							   IN ULONG SystemInformationLength,    
							   OUT PULONG ReturnLength OPTIONAL
							);
typedef
VOID
(*pfnExFreePoolWithTag)(
				  IN PVOID  P,
				  IN ULONG  Tag 
				  ); 
typedef
NTSTATUS
(*pfnPsLookupProcessByProcessId)(
						   IN HANDLE ProcessId,
						   OUT PEPROCESS *Process
						   );
typedef
PETHREAD 
(*pfnPsGetCurrentThread)(
				   VOID
				   );
typedef
HANDLE
(*pfnPsGetProcessId)(
			   IN PEPROCESS  Process
			   );
typedef
NTSTATUS
(*pfnNtOpenProcess)(
			  __out PHANDLE  ProcessHandle,
			  __in ACCESS_MASK  DesiredAccess,
			  __in POBJECT_ATTRIBUTES  ObjectAttributes,
			  __in_opt PCLIENT_ID  ClientId
			  );
typedef
NTSTATUS 
(*pfnNtQueryVirtualMemory)(
	__in HANDLE ProcessHandle,
	__in PVOID BaseAddress,
	__in MEMORY_INFORMATION_CLASS MemoryInformationClass,
	__out_bcount(MemoryInformationLength) PVOID MemoryInformation,
	__in SIZE_T MemoryInformationLength,
	__out_opt PSIZE_T ReturnLength
	);
typedef
NTSTATUS 
(*pfnNtClose)(
		IN HANDLE  Handle
		);
typedef
NTSTATUS 
(*pfnObReferenceObjectByPointer)(
						   IN PVOID  Object,
						   IN ACCESS_MASK  DesiredAccess,
						   IN POBJECT_TYPE  ObjectType,
						   IN KPROCESSOR_MODE  AccessMode
						   );
typedef
NTSTATUS
(*pfnNtOpenDirectoryObject)(
					  __out PHANDLE  DirectoryHandle,
					  __in ACCESS_MASK  DesiredAccess,
					  __in POBJECT_ATTRIBUTES  ObjectAttributes
					  );
typedef
NTSTATUS
(*pfnObQueryNameString)(
				  IN PVOID  Object,
				  OUT POBJECT_NAME_INFORMATION  ObjectNameInfo,
				  IN ULONG  Length,
				  OUT PULONG  ReturnLength
				  );

typedef
BOOLEAN
(*pfnMmIsNonPagedSystemAddressValid) (
								__in PVOID VirtualAddress
								);
typedef
PHYSICAL_ADDRESS
(*pfnMmGetPhysicalAddress)(
					 IN PVOID  BaseAddress
					 );
typedef
PVOID
(*pfnMmGetVirtualForPhysical) (
						 __in PHYSICAL_ADDRESS PhysicalAddress
						 );
typedef
PMDL 
(*pfnIoAllocateMdl)(
			  __in_opt PVOID  VirtualAddress,
			  __in ULONG  Length,
			  __in BOOLEAN  SecondaryBuffer,
			  __in BOOLEAN  ChargeQuota,
			  __inout_opt PIRP  Irp  OPTIONAL
			  );
typedef
VOID 
(*pfnMmBuildMdlForNonPagedPool)(
						  __inout PMDLX  MemoryDescriptorList
						  );
typedef
PVOID
(*pfnMmMapLockedPagesSpecifyCache)(
							 __in PMDLX  MemoryDescriptorList,
							 __in KPROCESSOR_MODE  AccessMode,
							 __in MEMORY_CACHING_TYPE  CacheType,
							 __in_opt PVOID  BaseAddress,
							 __in ULONG  BugCheckOnFailure,
							 __in MM_PAGE_PRIORITY  Priority
							 );
typedef
VOID 
(*pfnMmUnmapLockedPages)(
				   __in PVOID  BaseAddress,
				   __in PMDL  MemoryDescriptorList
				   );
typedef
VOID 
(*pfnIoFreeMdl)(
		  IN PMDL  Mdl
		  );
typedef
NTSTATUS
(*pfnNtUserBuildHwndList)(
					HDESK hDesktop,
					HWND hwndParent,
					BOOLEAN bChildren,
					ULONG dwThreadId,
					ULONG lParam,
					HWND* pWnd,
					ULONG* pBufSize);
typedef
NTSTATUS
(*pfnNtUserBuildHwndListWin8)(
						  HDESK hDesktop,
						  HWND hwndParent,
						  BOOLEAN bChildren,
						  ULONG dwThreadId,
						  ULONG nUnKnow,
						  ULONG lParam,
						  HWND* pWnd,
						  ULONG* pBufSize);

typedef
DWORD
(*pfnNtUserQueryWindow)(
						HWND hWnd, 
						DWORD Index
						);
typedef
NTSTATUS
(*pfnObOpenObjectByPointer)(
					  IN PVOID  Object,
					  IN ULONG  HandleAttributes,
					  IN PACCESS_STATE  PassedAccessState OPTIONAL,
					  IN ACCESS_MASK  DesiredAccess,
					  IN POBJECT_TYPE  ObjectType,
					  IN KPROCESSOR_MODE  AccessMode,
					  OUT PHANDLE  Handle
					  );
typedef
NTSTATUS
(*pfnNtOpenProcessToken)(IN HANDLE ProcessHandle,
				   IN ACCESS_MASK DesiredAccess,
				   OUT PHANDLE TokenHandle);
typedef
NTSTATUS
(*pfnNtQueryInformationToken)(IN HANDLE TokenHandle,
						IN TOKEN_INFORMATION_CLASS TokenInformationClass,
						OUT PVOID TokenInformation,
						IN ULONG TokenInformationLength,
						OUT PULONG ReturnLength);
typedef
NTSTATUS
(*pfnNtUnmapViewOfSection) (
					  __in PEPROCESS Process,
					  __in PVOID BaseAddress
					  );
typedef
NTSTATUS
(*pfnNtTerminateThread)(
				  __in_opt HANDLE ThreadHandle,
				  __in NTSTATUS ExitStatus
				  );
typedef
NTSTATUS
(*pfnNtSuspendThread)(
				__in HANDLE ThreadHandle,
				__out_opt PULONG PreviousSuspendCount
				);
typedef
NTSTATUS
(*pfnNtResumeThread)(
			   __in HANDLE ThreadHandle,
			   __out_opt PULONG PreviousSuspendCount
			   );
typedef
NTSTATUS
(*pfnNtQueryObject)(
					IN HANDLE               ObjectHandle,
					IN OBJECT_INFO_CLASS ObjectInformationClass,
					OUT PVOID               ObjectInformation,
					IN ULONG                Length,
					OUT PULONG              ResultLength );
typedef
NTSTATUS
(*pfnNtSetInformationObject) (
						__in HANDLE Handle,
						__in SH_OBJECT_INFORMATION_CLASS ObjectInformationClass,
						__in_bcount(ObjectInformationLength) PVOID ObjectInformation,
						__in ULONG ObjectInformationLength
						);
typedef
NTSTATUS
(*pfnNtFreeVirtualMemory) (
					 __in HANDLE ProcessHandle,
					 __inout PVOID *BaseAddress,
					 __inout PSIZE_T RegionSize,
					 __in ULONG FreeType
					 );
typedef
NTSTATUS
(*pfnNtProtectVirtualMemory) (IN HANDLE ProcessHandle,
					   IN OUT PVOID *UnsafeBaseAddress,
					   IN OUT SIZE_T *UnsafeNumberOfBytesToProtect,
					   IN ULONG NewAccessProtection,
					   OUT PULONG UnsafeOldAccessProtection);
typedef
NTSTATUS
(*pfnNtAdjustPrivilegesToken) (IN HANDLE TokenHandle,
						IN BOOLEAN DisableAllPrivileges,
						IN PTOKEN_PRIVILEGES NewState,
						IN ULONG BufferLength,
						OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
						OUT PULONG ReturnLength OPTIONAL);
typedef
NTSTATUS
(*pfnNtTerminateProcess) (IN HANDLE ProcessHandle OPTIONAL,
				   IN NTSTATUS ExitStatus);
typedef
NTSTATUS
(*pfnNtSuspendProcess)(IN HANDLE ProcessHandle);

typedef
NTSTATUS
(*pfnNtResumeProcess)(IN HANDLE ProcessHandle);

typedef
NTSTATUS
(*pfnWin2kPspTerminateThreadByPointer)(
							IN PETHREAD Thread,
							IN NTSTATUS ExitStatus
							);
typedef
NTSTATUS
(*pfnWin2k3PspTerminateThreadByPointer)(
									   IN PETHREAD Thread,
									   IN NTSTATUS ExitStatus,
									   IN ULONG aa
									   );
typedef
NTSTATUS
(*pfnNtOpenSymbolicLinkObject)(
						 OUT PHANDLE  LinkHandle,
						 IN ACCESS_MASK  DesiredAccess,
						 IN POBJECT_ATTRIBUTES  ObjectAttributes
						 );
typedef
NTSTATUS
(*pfnNtQuerySymbolicLinkObject)(
						  IN HANDLE LinkHandle,
						  IN OUT PUNICODE_STRING LinkTarget,
						  OUT PULONG ReturnedLength OPTIONAL
						  );
typedef
NTSTATUS
(*pfnIoCreateFile)(
			 OUT PHANDLE  FileHandle,
			 IN ACCESS_MASK  DesiredAccess,
			 IN POBJECT_ATTRIBUTES  ObjectAttributes,
			 OUT PIO_STATUS_BLOCK  IoStatusBlock,
			 IN PLARGE_INTEGER  AllocationSize OPTIONAL,
			 IN ULONG  FileAttributes,
			 IN ULONG  ShareAccess,
			 IN ULONG  Disposition,
			 IN ULONG  CreateOptions,
			 IN PVOID  EaBuffer OPTIONAL,
			 IN ULONG  EaLength,
			 IN CREATE_FILE_TYPE  CreateFileType,
			 IN PVOID  InternalParameters OPTIONAL,
			 IN ULONG  Options
			 );
typedef
VOID
(*pfnObMakeTemporaryObject) (
					   __in PVOID Object
					   );
typedef
NTSTATUS 
(*pfnPsTerminateSystemThread)(
						IN NTSTATUS  ExitStatus
						);
typedef
NTSTATUS 
(*pfnPsCreateSystemThread)(
					 OUT PHANDLE  ThreadHandle,
					 IN ULONG  DesiredAccess,
					 IN POBJECT_ATTRIBUTES  ObjectAttributes  OPTIONAL,
					 IN HANDLE  ProcessHandle  OPTIONAL,
					 OUT PCLIENT_ID  ClientId  OPTIONAL,
					 IN PKSTART_ROUTINE  StartRoutine,
					 IN PVOID  StartContext
					 );
typedef
VOID 
(*pfnIoDeleteDevice)(
			   IN PDEVICE_OBJECT  DeviceObject
			   );
typedef
NTSTATUS 
(*pfnKeWaitForSingleObject)(
					  IN PVOID  Object,
					  IN KWAIT_REASON  WaitReason,
					  IN KPROCESSOR_MODE  WaitMode,
					  IN BOOLEAN  Alertable,
					  IN PLARGE_INTEGER  Timeout  OPTIONAL
					  );
typedef
NTSTATUS
(*pfnObOpenObjectByName) (
					__in POBJECT_ATTRIBUTES ObjectAttributes,
					__in_opt POBJECT_TYPE ObjectType,
					__in KPROCESSOR_MODE AccessMode,
					__inout_opt PACCESS_STATE AccessState,
					__in_opt ACCESS_MASK DesiredAccess,
					__inout_opt PVOID ParseContext,
					__out PHANDLE Handle
					);
typedef
NTSTATUS 
(*pfnNtOpenKey)(
		  OUT PHANDLE  KeyHandle,
		  IN ACCESS_MASK  DesiredAccess,
		  IN POBJECT_ATTRIBUTES  ObjectAttributes
		  );
typedef
NTSTATUS
(*pfnNtEnumerateKey)(
			   __in HANDLE KeyHandle,
			   __in ULONG Index,
			   __in KEY_INFORMATION_CLASS KeyInformationClass,
			   __out_bcount_opt(Length) PVOID KeyInformation,
			   __in ULONG Length,
			   __out PULONG ResultLength
			   );
typedef
NTSTATUS
(*pfnNtEnumerateValueKey)(
					__in HANDLE KeyHandle,
					__in ULONG Index,
					__in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
					__out_bcount_opt(Length) PVOID KeyValueInformation,
					__in ULONG Length,
					__out PULONG ResultLength
					);
typedef
NTSTATUS
(*pfnPsSetCreateProcessNotifyRoutine)(
								__in PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
								__in BOOLEAN Remove
								);
typedef
NTSTATUS
(*pfnPsSetCreateThreadNotifyRoutine)(
							   IN PCREATE_THREAD_NOTIFY_ROUTINE  NotifyRoutine
							   );
typedef
NTSTATUS
(*pfnPsSetLoadImageNotifyRoutine)(
							IN PLOAD_IMAGE_NOTIFY_ROUTINE  NotifyRoutine
							);
typedef
NTSTATUS
(*pfnPsRemoveCreateThreadNotifyRoutine) (
								   IN PCREATE_THREAD_NOTIFY_ROUTINE  NotifyRoutine 
								   );
typedef
NTSTATUS
(*pfnPsRemoveLoadImageNotifyRoutine)(
							   IN PLOAD_IMAGE_NOTIFY_ROUTINE  NotifyRoutine 
							   );
typedef
NTSTATUS 
(*pfnIoRegisterShutdownNotification)(
							   IN PDEVICE_OBJECT  DeviceObject
							   );
typedef
NTSTATUS
(*pfnCmUnRegisterCallback)(
					 IN LARGE_INTEGER  Cookie
					 );
typedef
VOID 
(*pfnIoUnregisterShutdownNotification)(
								 IN PDEVICE_OBJECT  DeviceObject
								 );
typedef
BOOLEAN 
(*pfnKeCancelTimer)(
			  IN PKTIMER  Timer
			  );
typedef
VOID 
(*pfnIoStopTimer)(
			IN PDEVICE_OBJECT  DeviceObject
			);
typedef
VOID 
(*pfnIoStartTimer)(
			 IN PDEVICE_OBJECT  DeviceObject
			 );
typedef
NTSTATUS 
(*pfnIoInitializeTimer)(
				  IN PDEVICE_OBJECT  DeviceObject,
				  IN PIO_TIMER_ROUTINE  TimerRoutine,
				  IN PVOID  Context
				  );
typedef
BOOLEAN
(*pfnKe386IoSetAccessProcess) (
						 PKPROCESS Process,
						 ULONG MapNumber
						 );
typedef
NTSTATUS 
(*pfnIoGetDeviceObjectPointer)(
						 IN PUNICODE_STRING  ObjectName,
						 IN ACCESS_MASK  DesiredAccess,
						 OUT PFILE_OBJECT  *FileObject,
						 OUT PDEVICE_OBJECT  *DeviceObject
						 );
typedef
NTSTATUS
(*pfnObReferenceObjectByName)(
						__in PUNICODE_STRING ObjectName,
						__in ULONG Attributes,
						__in_opt PACCESS_STATE AccessState,
						__in_opt ACCESS_MASK DesiredAccess,
						__in POBJECT_TYPE ObjectType,
						__in KPROCESSOR_MODE AccessMode,
						__inout_opt PVOID ParseContext,
						__out PVOID *Object
						);
typedef
NTSTATUS
(*pfnNtOpenFile)(
		   OUT PHANDLE  FileHandle,
		   IN ACCESS_MASK  DesiredAccess,
		   IN POBJECT_ATTRIBUTES  ObjectAttributes,
		   OUT PIO_STATUS_BLOCK  IoStatusBlock,
		   IN ULONG  ShareAccess,
		   IN ULONG  OpenOptions
		   );
typedef
NTSTATUS 
(*pfnNtCreateSection)(
				OUT PHANDLE  SectionHandle,
				IN ACCESS_MASK  DesiredAccess,
				IN POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
				IN PLARGE_INTEGER  MaximumSize OPTIONAL,
				IN ULONG  SectionPageProtection,
				IN ULONG  AllocationAttributes,
				IN HANDLE  FileHandle OPTIONAL
				);
typedef
NTSTATUS 
(*pfnNtMapViewOfSection)(
				   IN HANDLE  SectionHandle,
				   IN HANDLE  ProcessHandle,
				   IN OUT PVOID  *BaseAddress,
				   IN ULONG_PTR  ZeroBits,
				   IN SIZE_T  CommitSize,
				   IN OUT PLARGE_INTEGER  SectionOffset  OPTIONAL,
				   IN OUT PSIZE_T  ViewSize,
				   IN SECTION_INHERIT  InheritDisposition,
				   IN ULONG  AllocationType,
				   IN ULONG  Win32Protect
				   );
typedef
VOID 
(*pfnRtlInitAnsiString)(
				  IN OUT PANSI_STRING  DestinationString,
				  IN PCSZ  SourceString
				  );
typedef
NTSTATUS 
(*pfnRtlAnsiStringToUnicodeString)(
							 IN OUT PUNICODE_STRING  DestinationString,
							 IN PANSI_STRING  SourceString,
							 IN BOOLEAN  AllocateDestinationString
							 );
typedef
VOID 
(*pfnRtlFreeUnicodeString)(
					 IN PUNICODE_STRING  UnicodeString
					 );
typedef
HANDLE
(*pfnPsGetCurrentProcessId)(
					  VOID
					  );
typedef
VOID
(*pfnIoFreeIrp)(
		  __in PIRP Irp
		  );
typedef
BOOLEAN 
(*pfnMmFlushImageSection)(
					IN PSECTION_OBJECT_POINTERS  SectionPointer,
					IN MMFLUSH_TYPE  FlushType
					);

typedef
NTSTATUS 
(*pfnNtDeleteKey)(
			IN HANDLE  KeyHandle
			);

typedef
NTSTATUS 
(*pfnNtCreateKey)(
			OUT PHANDLE  KeyHandle,
			IN ACCESS_MASK  DesiredAccess,
			IN POBJECT_ATTRIBUTES  ObjectAttributes,
			IN ULONG  TitleIndex,
			IN PUNICODE_STRING  Class  OPTIONAL,
			IN ULONG  CreateOptions,
			OUT PULONG  Disposition  OPTIONAL
			);
typedef
NTSTATUS 
(*pfnNtSetValueKey)(
			  IN HANDLE  KeyHandle,
			  IN PUNICODE_STRING  ValueName,
			  IN ULONG  TitleIndex  OPTIONAL,
			  IN ULONG  Type,
			  IN PVOID  Data,
			  IN ULONG  DataSize
			  );
typedef
NTSTATUS 
(*pfnNtDeleteValueKey)(
				 IN HANDLE  KeyHandle,
				 IN PUNICODE_STRING  ValueName
				 );

typedef enum _KAPC_ENVIRONMENT {
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
} KAPC_ENVIRONMENT;

typedef
VOID
(*pfnKeInitializeApc) (
				 __out PRKAPC Apc,
				 __in PRKTHREAD Thread,
				 __in KAPC_ENVIRONMENT Environment,
				 __in PKKERNEL_ROUTINE KernelRoutine,
				 __in_opt PKRUNDOWN_ROUTINE RundownRoutine,
				 __in_opt PKNORMAL_ROUTINE NormalRoutine,
				 __in_opt KPROCESSOR_MODE ApcMode,
				 __in_opt PVOID NormalContext
				 );

typedef
BOOLEAN
(*pfnKeInsertQueueApc) (
				  __inout PRKAPC Apc,
				  __in_opt PVOID SystemArgument1,
				  __in_opt PVOID SystemArgument2,
				  __in KPRIORITY Increment
				  );
typedef
NTSTATUS
(*pfnPsTerminateSystemThread)(
						__in NTSTATUS ExitStatus
						);

typedef
NTSTATUS
(*pfnNtReadVirtualMemory)(IN HANDLE ProcessHandle,
					IN PVOID BaseAddress,
					OUT PVOID Buffer,
					IN SIZE_T NumberOfBytesToRead,
					OUT PSIZE_T NumberOfBytesRead OPTIONAL);
typedef
NTSTATUS
(*pfnNtWriteVirtualMemory)(IN HANDLE ProcessHandle,
					 IN PVOID BaseAddress,
					 IN PVOID Buffer,
					 IN SIZE_T NumberOfBytesToWrite,
					 OUT PSIZE_T NumberOfBytesWritten OPTIONAL);
typedef
NTSTATUS
(*pfnNtProtectVirtualMemory)(IN HANDLE ProcessHandle,
					   IN OUT PVOID *UnsafeBaseAddress,
					   IN OUT SIZE_T *UnsafeNumberOfBytesToProtect,
					   IN ULONG NewAccessProtection,
					   OUT PULONG UnsafeOldAccessProtection);
typedef
NTSTATUS
(*pfnNtRenameKey)(
			IN HANDLE KeyHandle,
			IN PUNICODE_STRING NewName);

typedef
PIRP
(*pfnIoBuildDeviceIoControlRequest)(
							  IN ULONG IoControlCode,
							  IN PDEVICE_OBJECT DeviceObject,
							  IN PVOID InputBuffer OPTIONAL,
							  IN ULONG InputBufferLength,
							  OUT PVOID OutputBuffer OPTIONAL,
							  IN ULONG OutputBufferLength,
							  IN BOOLEAN InternalDeviceIoControl,
							  IN PKEVENT Event,
							  OUT PIO_STATUS_BLOCK IoStatusBlock
							  );

typedef
HWND
(*pfnNtUserGetForegroundWindow)(VOID);

typedef
LONG 
(*pfnInterlockedIncrement)(
					 IN PLONG  Addend
					 );

typedef
LONG 
(*pfnInterlockedDecrement)(
					 IN PLONG  Addend
					 );

typedef
BOOLEAN (*pfnNtUserDestroyWindow)(HWND Wnd);

typedef
HWND
(*pfnNtUserFindWindowEx)(
				   HWND  hwndParent,
				   HWND  hwndChildAfter,
				   PUNICODE_STRING  ucClassName,
				   PUNICODE_STRING  ucWindowName,
				   DWORD dwUnknown
				   );

typedef
BOOL
(*pfnNtUserMessageCall)(
				  HWND hWnd,
				  UINT Msg,
				  WPARAM wParam,
				  LPARAM lParam,
				  ULONG_PTR ResultInfo,
				  DWORD dwType, // FNID_XX types
				  BOOL Ansi);

typedef
BOOL
(*pfnNtUserPostThreadMessage)(
						DWORD idThread,
						UINT Msg,
						WPARAM wParam,
						LPARAM lParam);

typedef
HWND
(*pfnNtUserSetParent)(
				HWND hWndChild,
				HWND hWndNewParent);
typedef
LONG
(*pfnNtUserSetWindowLong)(
					HWND hWnd,
					DWORD Index,
					LONG NewValue,
					BOOL Ansi);
typedef
BOOL
(*pfnNtUserShowWindow)(
				 HWND hWnd,
				 LONG nCmdShow);

typedef
HWND
(*pfnNtUserWindowFromPoint)(
					  LONG X,
					  LONG Y);
typedef
BOOL
(*pfnNtUserPostMessage)(
				  HWND hWnd,
				  UINT Msg,
				  WPARAM wParam,
				  LPARAM lParam);
typedef
NTSTATUS
(*pfnPsLookupThreadByThreadId)(
						 IN HANDLE ThreadId,
						 OUT PETHREAD *Thread
						 );
typedef
NTSTATUS
(*pfnNtOpenThread)(
			 OUT PHANDLE ThreadHandle,
			 IN ACCESS_MASK DesiredAccess,
			 IN POBJECT_ATTRIBUTES ObjectAttributes,
			 IN PCLIENT_ID ClientId
			 );

typedef
NTSTATUS
(*pfnNtDuplicateObject)(
				  __in HANDLE SourceProcessHandle,
				  __in HANDLE SourceHandle,
				  __in_opt HANDLE TargetProcessHandle,
				  __out_opt PHANDLE TargetHandle,
				  __in ACCESS_MASK DesiredAccess,
				  __in ULONG HandleAttributes,
				  __in ULONG Options
				  ); 

typedef
NTSTATUS
(*pfnNtCreateProcess)(
				OUT PHANDLE ProcessHandle,
				IN ACCESS_MASK DesiredAccess,
				IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
				IN HANDLE ParentProcess,
				IN BOOLEAN InheritObjectTable,
				IN HANDLE SectionHandle OPTIONAL,
				IN HANDLE DebugPort OPTIONAL,
				IN HANDLE ExceptionPort OPTIONAL
				);

typedef
NTSTATUS
(*pfnNtCreateProcessEx)(OUT PHANDLE ProcessHandle,
				  IN ACCESS_MASK DesiredAccess,
				  IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
				  IN HANDLE ParentProcess,
				  IN ULONG Flags,
				  IN HANDLE SectionHandle OPTIONAL,
				  IN HANDLE DebugPort OPTIONAL,
				  IN HANDLE ExceptionPort OPTIONAL,
				  IN BOOLEAN InJob);
typedef
NTSTATUS
(*pfnNtCreateFile) (
			  __out PHANDLE FileHandle,
			  __in ACCESS_MASK DesiredAccess,
			  __in POBJECT_ATTRIBUTES ObjectAttributes,
			  __out PIO_STATUS_BLOCK IoStatusBlock,
			  __in_opt PLARGE_INTEGER AllocationSize,
			  __in ULONG FileAttributes,
			  __in ULONG ShareAccess,
			  __in ULONG CreateDisposition,
			  __in ULONG CreateOptions,
			  __in_bcount_opt(EaLength) PVOID EaBuffer,
			  __in ULONG EaLength
			  );

typedef
NTSTATUS
(*pfnNtCreateThread)(
			   __out PHANDLE ThreadHandle,
			   __in ACCESS_MASK DesiredAccess,
			   __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
			   __in HANDLE ProcessHandle,
			   __out PCLIENT_ID ClientId,
			   __in PCONTEXT ThreadContext,
			   __in PVOID InitialTeb,
			   __in BOOLEAN CreateSuspended
			   );

typedef
BOOL
(*pfnNtUserSwitchDesktop)(HDESK hDesktop);

typedef
NTSTATUS
(*pfnNtLoadKey2)(
		   __in POBJECT_ATTRIBUTES   TargetKey,
		   __in POBJECT_ATTRIBUTES   SourceFile,
		   __in ULONG                Flags
		   );
typedef
NTSTATUS
(*pfnNtReplaceKey) (
			 __in POBJECT_ATTRIBUTES NewFile,
			 __in HANDLE             TargetHandle,
			 __in POBJECT_ATTRIBUTES OldFile
			 );
typedef
NTSTATUS
(*pfnNtRestoreKey)(
			 __in HANDLE KeyHandle,
			 __in HANDLE FileHandle,
			 __in ULONG Flags
			 );
typedef
NTSTATUS
(*pfnNtInitiatePowerAction)(
					  IN POWER_ACTION SystemAction,
					  IN SYSTEM_POWER_STATE MinSystemState,
					  IN ULONG Flags,
					  IN BOOLEAN Asynchronous);
typedef
NTSTATUS
(*pfnNtSetSystemPowerState)(
					  IN POWER_ACTION SystemAction,
					  IN SYSTEM_POWER_STATE MinSystemState,
					  IN ULONG Flags);
typedef
DWORD_PTR
(*pfnNtUserCallOneParam)(
				   DWORD_PTR Param,
				   DWORD Routine);

typedef enum _SHUTDOWN_ACTION
{
	ShutdownNoReboot,
	ShutdownReboot,
	ShutdownPowerOff
} SHUTDOWN_ACTION;

typedef
NTSTATUS
(*pfnNtShutdownSystem)(
				 IN SHUTDOWN_ACTION Action
				 );
typedef
NTSTATUS
(*pfnNtSetSystemTime)(
				IN PLARGE_INTEGER SystemTime,
				IN PLARGE_INTEGER NewSystemTime OPTIONAL
				);

typedef
BOOL
(*pfnNtUserLockWorkStation) (VOID);

typedef
NTSTATUS 
(*pfnNtLoadDriver)(
			 IN PUNICODE_STRING  DriverServiceName
			 );

typedef
PVOID
(*pfnRtlImageDirectoryEntryToData)(PVOID BaseAddress,
								   BOOLEAN MappedAsImage,
								   USHORT Directory,
								   PULONG Size);
typedef
PIMAGE_NT_HEADERS
(NTAPI *pfnRtlImageNtHeader)(
				 PVOID Base
				 );
typedef
NTSTATUS 
(*pfnKeUserModeCallback)(
						 IN ULONG ApiNumber,
						 IN PVOID InputBuffer,
						 IN ULONG InputLength,
						 OUT ULONG OutputBuffer,
						 IN PULONG OutputLength
						 );
typedef
DWORD_PTR
(*pfnNtUserCallNoParam) (
				  DWORD Routine);
typedef
NTSTATUS 
(*pfnNtCreateUserProcess) (
								PHANDLE ProcessHandle,
								PHANDLE ThreadHandle,
								PVOID Parameter2,
								PVOID Parameter3,
								PVOID ProcessSecurityDescriptor,
								PVOID ThreadSecurityDescriptor,
								PVOID Parameter6,
								PVOID Parameter7,
								PVOID ProcessParameters,
								PVOID Parameter9,
								PVOID pProcessUnKnow);
typedef
NTSTATUS 
(*pfnNtCreateThreadEx) (
						  OUT PHANDLE ThreadHandle,
						  IN ACCESS_MASK DesiredAccess,
						  IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
						  IN HANDLE ProcessHandle,
						  IN PVOID StartRoutine,
						  IN PVOID StartContext,
						  IN ULONG CreateThreadFlags,
						  IN ULONG ZeroBits OPTIONAL,
						  IN ULONG StackSize OPTIONAL,
						  IN ULONG MaximumStackSize OPTIONAL,
						  IN PVOID AttributeList
						  );
typedef
NTSTATUS
(*pfnNtReadFile)(
		   IN HANDLE FileHandle,
		   IN HANDLE Event OPTIONAL,
		   IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		   IN PVOID ApcContext OPTIONAL,
		   OUT PIO_STATUS_BLOCK IoStatusBlock,
		   OUT PVOID Buffer,
		   IN ULONG Length,
		   IN PLARGE_INTEGER ByteOffset OPTIONAL,
		   IN PULONG Key OPTIONAL
		   );

typedef
NTSTATUS
(*pfnNtWriteFile)(
			IN HANDLE FileHandle,
			IN HANDLE Event OPTIONAL,
			IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
			IN PVOID ApcContext OPTIONAL,
			OUT PIO_STATUS_BLOCK IoStatusBlock,
			IN PVOID Buffer,
			IN ULONG Length,
			IN PLARGE_INTEGER ByteOffset OPTIONAL,
			IN PULONG Key OPTIONAL
			);

typedef
LONGLONG
(*pfnPsGetProcessCreateTimeQuadPart)(
							   __in PEPROCESS Process
							   );

////////////////////////////////////////////////////////////////////////

#endif