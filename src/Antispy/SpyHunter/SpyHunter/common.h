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
#ifndef _COMMON_H_MY_
#define _COMMON_H_MY_

////////////////////////////////////////////////////////////////////////

//
// 自定义消息
//

#define WM_CUSTOMER_MY					WM_USER + 1000

// 进程相关的消息
#define WM_RESIZE_ALL_PROC_WND			WM_CUSTOMER_MY + 1		// 重新分布进程列表下面的各个窗口
#define WM_CLICK_LIST_HEADER_START		WM_CUSTOMER_MY + 2		// 开始点击列表头
#define WM_CLICK_LIST_HEADER_END		WM_CUSTOMER_MY + 3		// 结束点击列表头
#define WM_UPDATE_PROCESS_DATA			WM_CUSTOMER_MY + 4		// 更新进程相关信息

// 验签名相关
#define WM_VERIFY_SIGN_OVER				WM_CUSTOMER_MY + 5		// 验签名结束了
#define WM_MODIFY_PROTECTION			WM_CUSTOMER_MY + 6		// 更改内存属性成功了

// 进程枚举相关信息
#define WM_ENUM_TIMER					WM_CUSTOMER_MY + 7		// 枚举进程定时器
#define WM_ENUM_MODULES					WM_CUSTOMER_MY + 8		// 枚举进程模块
#define WM_ENUM_HANDLES					WM_CUSTOMER_MY + 9		// 枚举进程句柄
#define WM_ENUM_THREADS					WM_CUSTOMER_MY + 10		// 枚举进程线程
#define WM_ENUM_MEMORY					WM_CUSTOMER_MY + 11		// 枚举进程内存
#define WM_ENUM_WINDOWS					WM_CUSTOMER_MY + 12		// 枚举进程窗口
#define WM_ENUM_HOTKEYS					WM_CUSTOMER_MY + 13		// 枚举进程热键
#define WM_ENUM_PRIVILEGES				WM_CUSTOMER_MY + 14		// 枚举进程权限

////////////////////////////////////////////////////////////////////////

//
// 每一项的颜色相关
//

typedef enum _ITEM_COLOR_
{
	enumBlack = 0,		// 微软的模块
	enumRed,			// 危险的项
	enumBlue,			// 其他的非微软模块
	enumPurple,			// 未带签名的项
	enumTuhuang,		// 微软的进程中带有非微软的模块
}ITEM_COLOR;

typedef struct _ITEM_COLOR_INFO_
{
	ITEM_COLOR textClr;
	ITEM_COLOR bgdClr;
}ITEM_COLOR_INFO, *PITEM_COLOR_INFO;

////////////////////////////////////////////////////////////////////////

#define	CLASSES_ROOT		L"\\Registry\\Machine\\SOFTWARE\\Classes"
#define	LOCAL_MACHINE		L"\\Registry\\Machine"
#define USERS				L"\\Registry\\User"
#define CURRENT_CONFIGL     L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current"

#define	REG_SERVICE			L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\"
#define SERVICE_KEY_NAME	L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services"

////////////////////////////////////////////////////////////////////////

//
// Windows内核中的一些结构体
//
typedef LONG NTSTATUS;

typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct
{
	ULONG          AllocationSize;
	ULONG          ActualSize;
	ULONG          Flags;
	ULONG          Unknown1;
	UNICODE_STRING Unknown2;
	HANDLE         InputHandle;
	HANDLE         OutputHandle;
	HANDLE         ErrorHandle;
	UNICODE_STRING CurrentDirectory;
	HANDLE         CurrentDirectoryHandle;
	UNICODE_STRING SearchPaths;
	UNICODE_STRING ApplicationName;
	UNICODE_STRING CommandLine;
	PVOID          EnvironmentBlock;
	ULONG          Unknown[9];
	UNICODE_STRING Unknown3;
	UNICODE_STRING Unknown4;
	UNICODE_STRING Unknown5;
	UNICODE_STRING Unknown6;
} PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;

typedef struct
{
	ULONG               AllocationSize;
	ULONG               Unknown1;
	HINSTANCE           ProcessHinstance;
	PVOID               ListDlls;
	PPROCESS_PARAMETERS ProcessParameters;
	ULONG               Unknown2;
	HANDLE              Heap;
} PEB, *PPEB;

typedef struct
{
	DWORD ExitStatus;
	PPEB  PebBaseAddress;
	DWORD AffinityMask;
	DWORD BasePriority;
	ULONG UniqueProcessId;
	ULONG InheritedFromUniqueProcessId;
}   PROCESS_BASIC_INFORMATION;

typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName,
	MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;

typedef struct _LDR_DATA_TABLE_ENTRY
{     
	LIST_ENTRY     InLoadOrderLinks;
	LIST_ENTRY     InMemoryOrderLinks;
	LIST_ENTRY     InInitializationOrderLinks;
	PVOID          DllBase;
	PVOID          EntryPoint;
	ULONG          SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG          Flags;
	USHORT         LoadCount;
	USHORT         TlsIndex;
	union
	{
		LIST_ENTRY HashLinks;
		struct
		{
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union
	{
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	PVOID          EntryPointActivationContext;
	PVOID          PatchInformation;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L

#define InitializeObjectAttributes( p, n, a, r, s ) {		\
	(p)->Length = sizeof( MZF_OBJECT_ATTRIBUTES );      \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
}

typedef enum _KEY_VALUE_INFORMATION_CLASS {
	KeyValueBasicInformation,
	KeyValueFullInformation,
	KeyValuePartialInformation,
	KeyValueFullInformationAlign64,
	KeyValuePartialInformationAlign64,
	MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;

typedef struct _KEY_VALUE_FULL_INFORMATION {
	ULONG   TitleIndex;
	ULONG   Type;
	ULONG   DataOffset;
	ULONG   DataLength;
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable size
	//          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_BASIC_INFORMATION {
	LARGE_INTEGER LastWriteTime;
	ULONG   TitleIndex;
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef 
NTSTATUS
(WINAPI *pfnNtQueryVirtualMemory) (
	IN HANDLE ProcessHandle, 
	IN PVOID BaseAddress, 
	IN MEMORY_INFORMATION_CLASS MemoryInformationClass, 
	OUT PVOID MemoryInformation, 
	IN ULONG MemoryInformationLength, 
	OUT PULONG ReturnLength OPTIONAL 
	);

typedef
NTSTATUS 
(WINAPI *pfnZwLoadDriver)(
							 PUNICODE_STRING  DriverServiceName
							 );

#define ProcessBasicInformation 0

typedef 
NTSTATUS 
(WINAPI *pfnNtQueryInformationProcess)(HANDLE,
									   UINT,
									   PVOID,
									   ULONG,
									   PULONG
									   );

/////////////////////////////////////////////////////////////////////////

//
// 各个列表头
//

typedef enum _SSDT_HOOK_HEADER_INDEX
{
	eHookIndex,
	eHookFunction,
	eHookOriginalEntry,
	eHookType,
	eHookCurrentEntry,
	eHookModule,
}SSDT_HOOK_HEADER_INDEX;

#endif