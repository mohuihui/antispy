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
#include "Peload.h"
#include "IrpOperate.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "Driver.h"

//****************************************************************************
// Return:	  如果获取的数值都成功，那么返回TRUE，否则FALSE.
// Qualifier: 通过ZwQuerySystemInformation函数的第11号功能，即SystemModuleInformation，查询得到系统模块的信息。
// Parameter: WCHAR * szNtoskrnlPath - 返回系统模块的路径，一般为：\SystemRoot\system32\ntoskrnl.exe
// Parameter: PDWORD SystemKernelModuleBase - 返回系统模块的基址
// Parameter: PDWORD SystemKernelModuleSize - 返回系统模块的大小
//****************************************************************************
BOOL GetNtoskrnlInfo(WCHAR *szNtoskrnlPath, PDWORD SystemKernelModuleBase, PDWORD SystemKernelModuleSize)
{
	ULONG ulSize = 0;
	PMODULES pModuleList = NULL;
	CHAR* szKernelName = NULL;
	ANSI_STRING AnsiKernelName;
	UNICODE_STRING UnKernelName;
	BOOL bRet = TRUE;
	WCHAR szSystem32[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','s','y','s','t','e','m','3','2','\\','\0'};

	if (!szNtoskrnlPath || !SystemKernelModuleBase || !SystemKernelModuleSize) 
	{
		return FALSE;
	}

	if (ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &ulSize) != STATUS_INFO_LENGTH_MISMATCH)
	{
		return FALSE;
	}

	pModuleList = (PMODULES)ExAllocatePoolWithTag(PagedPool, ulSize, MZFTAG);
	if (pModuleList)
	{
		memset(pModuleList, 0, ulSize);
		if (!NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, pModuleList, ulSize, &ulSize)))
		{
			ExFreePool(pModuleList);
			KdPrint(("ZwQuerySystemInformation error\n"));
			return FALSE;
		}
	}

	szKernelName = pModuleList->smi[0].ModuleNameOffset + pModuleList->smi[0].ImageName;
	RtlInitAnsiString(&AnsiKernelName, szKernelName);
	RtlAnsiStringToUnicodeString(&UnKernelName, &AnsiKernelName, TRUE);

	wcscat(szNtoskrnlPath, szSystem32);
	memcpy(szNtoskrnlPath + wcslen(szSystem32), UnKernelName.Buffer, UnKernelName.Length);

	*SystemKernelModuleBase = (DWORD)pModuleList->smi[0].Base;
	*SystemKernelModuleSize = (DWORD)pModuleList->smi[0].Size;
	
	ExFreePool(pModuleList);
	RtlFreeUnicodeString(&UnKernelName);
	
	return TRUE;
}

BOOL GetWin32kInfo(WCHAR *szPath, PDWORD pModuleBase, PDWORD pModuleSize)
{
	ULONG ulSize = 0, i = 0, nCnt = 0;
	PMODULES pModuleList = NULL;
	ANSI_STRING AnsiKernelName;
	UNICODE_STRING UnKernelName;
	WCHAR szSystem32[] = {'\\','S','y','s','t','e','m','R','o','o','t','\\','s','y','s','t','e','m','3','2','\\','\0'};

	if (!szPath || !pModuleBase || !pModuleSize) 
	{
		return FALSE;
	}

	if (ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &ulSize) != STATUS_INFO_LENGTH_MISMATCH)
	{
		return FALSE;
	}

	pModuleList = (PMODULES)ExAllocatePoolWithTag(PagedPool, ulSize, MZFTAG);
	if (pModuleList)
	{
		memset(pModuleList, 0, ulSize);

		if (!NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, pModuleList, ulSize, &ulSize)))
		{
			ExFreePool(pModuleList);
			KdPrint(("ZwQuerySystemInformation error\n"));
			return FALSE;
		}
	}
	
	nCnt = pModuleList->ulCount;
	for (i = 0; i < nCnt; i++)
	{
		CHAR szWin32k[] = {'w','i','n','3','2','k','.','s','y','s','\0'};
		CHAR* szKernelName = pModuleList->smi[i].ModuleNameOffset + pModuleList->smi[i].ImageName;
		if (strlen(szWin32k) == strlen(szKernelName) && !_strnicmp(szWin32k, szKernelName, strlen(szWin32k)))
		{
			RtlInitAnsiString(&AnsiKernelName, szKernelName);
			RtlAnsiStringToUnicodeString(&UnKernelName, &AnsiKernelName, TRUE);

			wcscat(szPath, szSystem32);
			memcpy(szPath + wcslen(szSystem32), UnKernelName.Buffer, UnKernelName.Length);

			*pModuleBase = (DWORD)pModuleList->smi[i].Base;
			*pModuleSize = (DWORD)pModuleList->smi[i].Size;
			
			RtlFreeUnicodeString(&UnKernelName);

			break;
		}
	}

	ExFreePool(pModuleList);

	return TRUE;
}

BOOL IoGetFileSystemVpbInfo(IN PFILE_OBJECT FileObject, PDEVICE_OBJECT *DeviceObject, PDEVICE_OBJECT *RealDevice)
{
	/*
		FileObject->DeviceObject + FileObject->FileName 即可得到文件全路径
		FileObject->Vpb->DeviceObject->DriverObject->DriverName 可以得到文件系统类型

		typedef struct _FILE_OBJECT {
		CSHORT Type;
		CSHORT Size;
		PDEVICE_OBJECT DeviceObject;        目标文件所在的磁盘设备
		PVPB Vpb;   指向文件卷参数块，Vpb->DeviceObject指向文件卷设备对象，即相应文件系统FSD创建出来的设备对象
									  Vpb->RealDevice指向代表着具体逻辑磁盘的设备对象
				    VPB一方面表明目标对象存在于哪一个逻辑磁盘上，另一方面表明这个磁盘承载着什么样的文件卷，属于哪一种文件系统
		PVOID FsContext;
		PVOID FsContext2;
		PSECTION_OBJECT_POINTERS SectionObjectPointer;
		PVOID PrivateCacheMap;
		NTSTATUS FinalStatus;
		struct _FILE_OBJECT *RelatedFileObject;
		BOOLEAN LockOperation;
		BOOLEAN DeletePending;
		BOOLEAN ReadAccess;
		BOOLEAN WriteAccess;
		BOOLEAN DeleteAccess;
		BOOLEAN SharedRead;
		BOOLEAN SharedWrite;
		BOOLEAN SharedDelete;
		ULONG Flags;
		UNICODE_STRING FileName;
		LARGE_INTEGER CurrentByteOffset;
		__volatile ULONG Waiters;
		__volatile ULONG Busy;
		PVOID LastLock;
		KEVENT Lock;
		KEVENT Event;
		__volatile PIO_COMPLETION_CONTEXT CompletionContext;
		KSPIN_LOCK IrpListLock;
		LIST_ENTRY IrpList;
		__volatile PVOID FileObjectExtension;
		} FILE_OBJECT;
		typedef struct _FILE_OBJECT *PFILE_OBJECT; 
	*/

	BOOL bRet = FALSE;
	
	if (!FileObject)
	{
		return FALSE;
	}

	if (DeviceObject)
	{
		*DeviceObject = NULL;
	}

	if (RealDevice)
	{
		*RealDevice = NULL;
	}

	if(FileObject->Vpb != NULL)
	{
		if (DeviceObject && FileObject->Vpb->DeviceObject != NULL)
		{
			// 指向文件卷设备对象，即相应文件系统FSD创建出来的设备对象
			*DeviceObject = FileObject->Vpb->DeviceObject; 
		}
		
		if (RealDevice && FileObject->Vpb->RealDevice != NULL)
		{
			// 指向代表着具体逻辑磁盘的设备对象
			*RealDevice = FileObject->Vpb->RealDevice;
		}
		
		bRet = TRUE;
	}
	else if( !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN)	&&
			 FileObject->DeviceObject->Vpb != NULL			)
	{
		if (DeviceObject && FileObject->DeviceObject->Vpb->DeviceObject != NULL)
		{
			*DeviceObject = FileObject->DeviceObject->Vpb->DeviceObject;
		}

		if (RealDevice && FileObject->DeviceObject->Vpb->RealDevice != NULL)
		{
			*RealDevice = FileObject->DeviceObject->Vpb->RealDevice;
		}
		
		bRet = TRUE;
	}
	else
	{
		if (DeviceObject)
		{
			*DeviceObject = FileObject->DeviceObject;
		}

		if (RealDevice)
		{
			*RealDevice = NULL;
		}

		bRet = TRUE;
	}
	
	return bRet;
}

BOOL GetDeviceObjectFromFileFullName(WCHAR *FileFullName, PDEVICE_OBJECT *RealDevice, PDEVICE_OBJECT *DeviceObject)
{
	WCHAR wRootName[32] = {0};
	UNICODE_STRING RootName;
	OBJECT_ATTRIBUTES ObjectAttributes;
	NTSTATUS status;
	HANDLE hFile;
	IO_STATUS_BLOCK IoStatus;
	PFILE_OBJECT FileObject = NULL;

	if (FileFullName[0] == L'\\')
	{
		WCHAR szSystemRoot[] = {'\\','S','y','s','t','e','m','R','o','o','t','\0'};
		wcscpy(wRootName, szSystemRoot);
	}
	else
	{
		WCHAR szDosDevices[] = {'\\','D','o','s','D','e','v','i','c','e','s','\\','*',':','\\','\0'};
		wcscpy(wRootName, szDosDevices);
		wRootName[12] = FileFullName[0];
	}

	RtlInitUnicodeString(&RootName, wRootName);
	InitializeObjectAttributes(&ObjectAttributes, &RootName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = IoCreateFile(
		&hFile,
		SYNCHRONIZE,
		&ObjectAttributes,
		&IoStatus,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0,
		0,
		NULL,
		IO_NO_PARAMETER_CHECKING);

	if (!NT_SUCCESS(status))
	{
		return FALSE;
	}

	status = ObReferenceObjectByHandle(hFile, 1, *IoFileObjectType, KernelMode, &FileObject, NULL);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		return FALSE;
	}

	if(!IoGetFileSystemVpbInfo(FileObject, DeviceObject, RealDevice))
	{
		ObDereferenceObject(FileObject);
		ZwClose(hFile);
		return FALSE;
	}

	ObDereferenceObject(FileObject);
	ZwClose(hFile);
	return TRUE;
}

//*************************************************

// Returns:   BOOL
// Qualifier: 获得系统根目录名字，一般为Windows
// Parameter: WCHAR * szRootName - 返回系统根目录名字，一般为Windows

//*************************************************
BOOL GetWindowsRootName(WCHAR *szRootName)
{
	UNICODE_STRING RootName, ObjectName;
	OBJECT_ATTRIBUTES oa;
	HANDLE hLink;
	NTSTATUS status;
	WCHAR *ObjectNameBuffer = ExAllocatePoolWithTag(NonPagedPool, MAX_PATH_WCHAR, MZFTAG);
	BOOL bRet = FALSE;

	if (!szRootName)
	{
		return FALSE;
	}

	if (ObjectNameBuffer)
	{
		WCHAR szSysRoot[] = {'\\','S','y','s','t','e','m','R','o','o','t','\0'};
		RtlZeroMemory(ObjectNameBuffer, MAX_PATH_WCHAR);
		RtlInitUnicodeString(&RootName, szSysRoot);
		InitializeObjectAttributes(&oa, &RootName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

		// 用winobj查看 "\SystemRoot", 其实这个是一个符号连接
		// 一般情况下，它链接到：\Device\Harddisk0\Partition1\Windows
		status = ZwOpenSymbolicLinkObject(&hLink, GENERIC_READ, &oa);

		if (NT_SUCCESS(status))
		{
			ObjectName.Buffer = ObjectNameBuffer;
			ObjectName.Length = 0;
			ObjectName.MaximumLength = MAX_PATH_WCHAR;

			status = ZwQuerySymbolicLinkObject(hLink, &ObjectName, NULL);

			if (NT_SUCCESS(status))
			{
				ULONG nLen = ObjectName.Length / sizeof(WCHAR);
				ULONG i = 0;
				
				// 从SymbolicLink的ObjectName后面开始到分隔符，取得Windows部分
				for ( i = nLen - 1; i > 0; i-- )
				{
					if (ObjectNameBuffer[i] == L'\\')
					{
						RtlCopyMemory(szRootName, &ObjectNameBuffer[i], (nLen - i) * sizeof(WCHAR));
						bRet = TRUE;
						break;
					}
				}
			}

			ZwClose(hLink);
		}

		ExFreePool(ObjectNameBuffer);
	}

	// 如果上面那种方法取得失败，那么就读取0x7FFE0030这个地方的字符串，一般为Windows路径，例如：c:\windows
	if (!bRet)
	{
		__try
		{
			WCHAR *SystemRootName = (WCHAR*)0x7FFE0030;
			WCHAR szWindows[] = {'X',':','\\','W','I','N','D','O','W','S','\0'};

			ProbeForRead(SystemRootName, wcslen(szWindows) * sizeof(WCHAR), sizeof(WCHAR));
			if (SystemRootName[1] == L':' && SystemRootName[2] == L'\\') 
			{
				wcscpy(szRootName, &SystemRootName[2]);
				bRet = TRUE;
			}
		}
		__except(1)
		{
		}
	}

	return bRet;
}

NTSTATUS 
KernelOpenFile(
			 WCHAR *szFilePath, 
			 PHANDLE FileHandle, 
			 ACCESS_MASK DesiredAccess, 
			 ULONG FileAttributes, 
			 ULONG ShareAccess, 
			 ULONG CreateDisposition, 
			 ULONG CreateOptions
			 )
{
	WCHAR SystemRootName[] = {'\\','S','y','s','t','e','m','R','o','o','t','\0'};
	WCHAR FileNodeName[MAX_PATH] = {0};
	UNICODE_STRING FilePath;
	PDEVICE_OBJECT RealDevice = NULL, DeviceObject = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT FileObject = NULL;

	if (!szFilePath || !FileHandle)
	{
		return status;
	}

	// 把\SystemRoot\xxx的文件路径形式，改成Windows\xxx的形式
	if (!_wcsnicmp(szFilePath, SystemRootName, wcslen(SystemRootName)))
	{
		ULONG Len = 0;

		if(!GetWindowsRootName(FileNodeName))
		{
			return status;
		}

		Len = wcslen(SystemRootName);
		wcscat(FileNodeName, &szFilePath[Len]);
	}
	else
	{
		// 或者如果文件带完成路径，例如：c:\1\2.exe, 那么转换成\1\2.exe形式
		if (szFilePath[1] != L':' || szFilePath[2] != L'\\')
		{
			return status;
		}

		wcscpy(FileNodeName, &szFilePath[2]);
	}

	if(!GetDeviceObjectFromFileFullName(szFilePath, &RealDevice, &DeviceObject))
	{
		KdPrint(("get device object and real device object faild\n"));
		return status;
	}

	RtlInitUnicodeString(&FilePath, FileNodeName);
// 	KdPrint(("szFilePath: %S\n", szFilePath));
//  KdPrint(("KernelOpenFile -> FilePath: %wZ\n", &FilePath));

	status = IrpCreateFile(
			&FilePath, 
			DesiredAccess, 
			FileAttributes, 
			ShareAccess, 
			CreateDisposition, 
			CreateOptions, 
			DeviceObject, 
			RealDevice, 
			&FileObject);
	
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Irp create file failed\n"));
		return status;
	}

	status = ObOpenObjectByPointer(
		FileObject,
		OBJ_KERNEL_HANDLE, 
		NULL,
		DesiredAccess | SYNCHRONIZE,
		*IoFileObjectType,
		KernelMode,
		FileHandle);

	ObDereferenceObject(FileObject);
	return status;
}

NTSTATUS KernelGetFileSize(HANDLE hFile, PLARGE_INTEGER FileSize)
{
	NTSTATUS status;
	PFILE_OBJECT FileObject = NULL;
	PDEVICE_OBJECT DeviceObject = NULL, RealDevice = NULL;
	FILE_STANDARD_INFORMATION FileInformation;

	status = ObReferenceObjectByHandle(hFile, 0, *IoFileObjectType, KernelMode, &FileObject, NULL);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if(!IoGetFileSystemVpbInfo(FileObject, &DeviceObject, &RealDevice))
	{
		ObDereferenceObject(FileObject);
		return STATUS_UNSUCCESSFUL;
	}

	status = IrpQueryInformationFile(FileObject, DeviceObject, &FileInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(status))
	{
		ObDereferenceObject(FileObject);
		return status;
	}

	FileSize->HighPart = FileInformation.EndOfFile.HighPart;
	FileSize->LowPart = FileInformation.EndOfFile.LowPart;

	ObDereferenceObject(FileObject);
	return status;
}

NTSTATUS KernelReadFile(HANDLE hFile, PLARGE_INTEGER ByteOffset, ULONG Length, PVOID FileBuffer, PIO_STATUS_BLOCK IoStatusBlock)
{
	NTSTATUS status;
	PFILE_OBJECT FileObject = NULL;
	PDEVICE_OBJECT DeviceObject = NULL, RealDevice = NULL;
	FILE_STANDARD_INFORMATION FileInformation;

	status = ObReferenceObjectByHandle(hFile, 0, *IoFileObjectType, KernelMode, &FileObject, 0);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	if(!IoGetFileSystemVpbInfo(FileObject, &DeviceObject, &RealDevice))
	{
		ObDereferenceObject(FileObject);
		return STATUS_UNSUCCESSFUL;
	}

	status = IrpReadFile(FileObject, DeviceObject, IoStatusBlock, FileBuffer, Length, ByteOffset);
	ObDereferenceObject(FileObject);
	return status;
}

UINT AlignSize(UINT nSize, UINT nAlign)
{
	return ((nSize + nAlign - 1) / nAlign * nAlign);
}

//*************************************************

// Returns:   BOOL
// Qualifier: 将磁盘布局的映像修改成内存布局
// Parameter: PVOID FileBuffer - 磁盘布局的文件映像
// Parameter: PVOID * ImageModuleBase - 返回修改后的，以内存布局的映像地址

//*************************************************
BOOL ImageFile(PVOID FileBuffer, PVOID* ImageModuleBase)
{
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	PIMAGE_SECTION_HEADER ImageSectionHeader = NULL;
	DWORD FileAlignment = 0, SectionAlignment = 0, NumberOfSections = 0, SizeOfImage = 0, SizeOfHeaders = 0;
	DWORD Index = 0;
	PVOID ImageBase = NULL;
	DWORD SizeOfNtHeaders = 0;
	
	if (!FileBuffer || !ImageModuleBase)
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

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)FileBuffer + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		FileAlignment = ImageNtHeaders->OptionalHeader.FileAlignment;
		SectionAlignment = ImageNtHeaders->OptionalHeader.SectionAlignment;
		NumberOfSections = ImageNtHeaders->FileHeader.NumberOfSections;
		SizeOfImage = ImageNtHeaders->OptionalHeader.SizeOfImage;
		SizeOfHeaders = ImageNtHeaders->OptionalHeader.SizeOfHeaders;
		SizeOfImage = AlignSize(SizeOfImage, SectionAlignment);

		ImageBase = ExAllocatePoolWithTag(NonPagedPool, SizeOfImage, MZFTAG);
		if (ImageBase == NULL)
		{
			return FALSE;
		}

		RtlZeroMemory(ImageBase, SizeOfImage);

		SizeOfNtHeaders = 
			sizeof(ImageNtHeaders->FileHeader) + 
			sizeof(ImageNtHeaders->Signature) + 
			ImageNtHeaders->FileHeader.SizeOfOptionalHeader;

		ImageSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)ImageNtHeaders + SizeOfNtHeaders);

		for (Index = 0; Index < NumberOfSections; Index++)
		{
			ImageSectionHeader[Index].SizeOfRawData = AlignSize(ImageSectionHeader[Index].SizeOfRawData, FileAlignment);
			ImageSectionHeader[Index].Misc.VirtualSize = AlignSize(ImageSectionHeader[Index].Misc.VirtualSize, SectionAlignment);
		}

		if (ImageSectionHeader[NumberOfSections - 1].VirtualAddress + ImageSectionHeader[NumberOfSections - 1].SizeOfRawData > SizeOfImage)
		{
			ImageSectionHeader[NumberOfSections - 1].SizeOfRawData = SizeOfImage - ImageSectionHeader[NumberOfSections - 1].VirtualAddress;
		}

		// 首先拷贝PE头
		RtlCopyMemory(ImageBase, FileBuffer, SizeOfHeaders);

		// 然后拷贝各个区段
		for (Index = 0; Index < NumberOfSections; Index++)
		{
			DWORD FileOffset = ImageSectionHeader[Index].PointerToRawData;
			DWORD Length = ImageSectionHeader[Index].SizeOfRawData;
			DWORD ImageOffset = ImageSectionHeader[Index].VirtualAddress;
			RtlCopyMemory(&((PBYTE)ImageBase)[ImageOffset], &((PBYTE)FileBuffer)[FileOffset], Length);
		}

		*ImageModuleBase = ImageBase;
	}
	__except(1)
	{
		return FALSE;
	}

	return TRUE;
}

//****************************************************************************

// Returns:   成功返回TRUE, 失败返回FALSE
// Qualifier: 根据文件路径，重新映射一份到映像到内存中
// Parameter: WCHAR * szFilePath - 需要映射的文件路径
// Parameter: PULONG NewBase - 映射完成返回的地址

//****************************************************************************
BOOL PeLoadWithoutFix(WCHAR* szFilePath, PULONG NewBase)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE hFile = NULL;
	LARGE_INTEGER FileSize;
	DWORD Length = 0;
	BYTE *FileBuffer = NULL;
	BYTE *ImageBase = NULL;
 	IO_STATUS_BLOCK IoStatus;

	Status = KernelOpenFile(
		szFilePath, 
		&hFile, 
		SYNCHRONIZE | FILE_EXECUTE, 
		FILE_ATTRIBUTE_NORMAL, 
		FILE_SHARE_READ, 
		FILE_OPEN, 
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (!NT_SUCCESS(Status) || hFile == NULL)
	{
		return FALSE;
	}

	Status = KernelGetFileSize(hFile, &FileSize);
	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);
		return FALSE;
	}

	Length = FileSize.LowPart;
	FileBuffer = ExAllocatePoolWithTag(PagedPool, Length, MZFTAG);
	if (FileBuffer == NULL)
	{
		ZwClose(hFile);
		return FALSE;
	}

	Status = KernelReadFile(hFile, NULL, Length, FileBuffer, &IoStatus);
	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);
		ExFreePool(FileBuffer);
		return FALSE;
	}

	ZwClose(hFile);

	if(!ImageFile(FileBuffer, &ImageBase))
	{
		KdPrint(("ImageFile failed\n"));
		ExFreePool(FileBuffer);
		return FALSE;
	}

	ExFreePool(FileBuffer);

	*NewBase = (ULONG)ImageBase;
	return TRUE;
}

PVOID
MiFindExportedRoutine (
					   IN PVOID DllBase,
					   BOOL ByName,
					   IN char *RoutineName,
					   DWORD Ordinal
						)

{
	USHORT OrdinalNumber;
	PULONG NameTableBase;
	PUSHORT NameOrdinalTableBase;
	PULONG AddressTableBase;
	PULONG Addr;
	LONG High;
	LONG Low;
	LONG Middle;
	LONG Result;
	ULONG ExportSize;
	PVOID FunctionAddress;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory;

	__try
	{
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

		AddressTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

		if (!ByName)
		{
			KdPrint(("Import by orgin\n"));
			return (PVOID)AddressTableBase[Ordinal];
		}

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

	return FunctionAddress;
}

PVOID
FindExportedRoutineInReloadModule ( IN ULONG DllBase, IN char *RoutineName )
{
	USHORT OrdinalNumber;
	PULONG NameTableBase;
	PUSHORT NameOrdinalTableBase;
	PULONG AddressTableBase;
	PULONG Addr;
	LONG High;
	LONG Low;
	LONG Middle;
	LONG Result;
	ULONG ExportSize;
	PVOID FunctionAddress;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory;
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)GetGlobalVeriable(enumRtlImageDirectoryEntryToData);
	
	if (!DllBase || !RoutineName || !MzfRtlImageDirectoryEntryToData)
	{
		return NULL;
	}

	__try
	{
		ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) MzfRtlImageDirectoryEntryToData (
			(PVOID)DllBase,
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

		AddressTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

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

		//
		// 如果是转发函数，就直接返回NULL 
		// 例如：Tdi.sys中导出的函数TdiMatchPdoWithChainedReceiveContext，它其实是导出NDIS的NdisMatchPdoWithPacket
		// .edata:00013BBB ; BOOLEAN __stdcall TdiMatchPdoWithChainedReceiveContext(PVOID TsduDescriptor, PVOID PDO)
		// .edata:00013BBB TdiMatchPdoWithChainedReceiveContext db 'NDIS.NdisMatchPdoWithPacket',0
		//

		if ((FunctionAddress >= (PVOID)ExportDirectory) &&
			(FunctionAddress <= (PVOID)((PCHAR)ExportDirectory + ExportSize)))
		{
			return NULL;
		}

		// 	ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
		// 		(FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));
	}
	__except(1)
	{
		FunctionAddress = NULL;
	}

	return FunctionAddress;
}

//*************************************************
// Returns:   返回模块基地址
// Qualifier: 通过模块名，获得该模块的基地址
// Parameter: KernelModuleName - 模块名
//*************************************************
PVOID GetKernelModuleBase(CHAR* szModuleName)
{
	PLDR_DATA_TABLE_ENTRY FirstEntry = NULL, LdrEntry = NULL;
	ANSI_STRING asModuleName;
	UNICODE_STRING unModuleName;
	ULONG Lentgh = 0, Index = 0;
	PVOID pRet = NULL;

	FirstEntry = LdrEntry = (PLDR_DATA_TABLE_ENTRY)GetGlobalVeriable(enumNtoskrnl_KLDR_DATA_TABLE_ENTRY);

	if (!FirstEntry || !szModuleName)
	{
		return NULL;
	}

	RtlInitAnsiString(&asModuleName, szModuleName);
	RtlAnsiStringToUnicodeString(&unModuleName, &asModuleName, TRUE);

	do
	{
		if ((ULONG)LdrEntry->DllBase > SYSTEM_ADDRESS_START		&&
			LdrEntry->BaseDllName.Length > 0					&&
			LdrEntry->BaseDllName.Buffer != NULL				&&
			MmIsAddressValid(LdrEntry->BaseDllName.Buffer)		&&
			MmIsAddressValid(&(LdrEntry->BaseDllName.Buffer[LdrEntry->BaseDllName.Length / sizeof(WCHAR) - 1])))
		{
		//	KdPrint(("RtlEqualUnicodeString: %wZ, %wZ\n", &LdrEntry->BaseDllName, &unModuleName));
			if (RtlEqualUnicodeString(&LdrEntry->BaseDllName, &unModuleName, TRUE))
			{
				pRet = LdrEntry->DllBase;
				break;
			}
		}

		LdrEntry = (PLDR_DATA_TABLE_ENTRY)LdrEntry->InLoadOrderLinks.Flink;

	}while (LdrEntry && FirstEntry != LdrEntry);

	RtlFreeUnicodeString(&unModuleName);

	return pRet;
}

BOOL InsertOriginalFirstThunk(DWORD ImageBase, DWORD ExistImageBase, PIMAGE_THUNK_DATA FirstThunk)
{
	DWORD Offset = 0;
	PIMAGE_THUNK_DATA OriginalFirstThunk = NULL;
	
	Offset = (DWORD)FirstThunk - ImageBase;
	OriginalFirstThunk = (PIMAGE_THUNK_DATA)(ExistImageBase + Offset);

	__try
	{
		while (OriginalFirstThunk->u1.Function)
		{
			KdPrint(("Fuction Address:%X\n",OriginalFirstThunk->u1.Function));
			FirstThunk->u1.Function = OriginalFirstThunk->u1.Function;
			OriginalFirstThunk++;
			FirstThunk++;
		}
	}
	__except(1)
	{

	}

	return TRUE;
}

//*************************************************
// Returns:   BOOL
// Qualifier: 修复可执行文件导入表
// Parameter: BYTE * ImageBase - 新的Image地址
// Parameter: DWORD ExistImageBase - 老的Image地址
//*************************************************
BOOL FixImportTable(BYTE *ImageBase, DWORD ExistImageBase)
{
	PIMAGE_IMPORT_DESCRIPTOR ImageImportDescriptor = NULL;
	PIMAGE_THUNK_DATA ImageThunkData = NULL, FirstThunk = NULL;
	PIMAGE_IMPORT_BY_NAME ImortByName = NULL;
	DWORD ImportSize = 0;
	PVOID ModuleBase = NULL;
	DWORD FunctionAddress = 0;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	UNICODE_STRING unFunc;
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = NULL;
	
	WCHAR szFunc[] = {'R','t','l','I','m','a','g','e','D','i','r','e','c','t','o','r','y','E','n','t','r','y','T','o','D','a','t','a','\0'};
	RtlInitUnicodeString(&unFunc, szFunc);

	MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)MmGetSystemRoutineAddress(&unFunc);

	if (!ImageBase || 
		!ExistImageBase || 
		!MzfRtlImageDirectoryEntryToData)
	{
		return FALSE;
	}

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)((ULONG)ImageBase + ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		ImageImportDescriptor = 
			(PIMAGE_IMPORT_DESCRIPTOR)MzfRtlImageDirectoryEntryToData(
			ImageBase, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_IMPORT, 
			&ImportSize);

		// 如果没有导入表，那么直接返回TRUE.
		if (ImageImportDescriptor == NULL)
		{
			return TRUE;
		}

		
		while (ImageImportDescriptor->OriginalFirstThunk && 
			ImageImportDescriptor->Name &&
			MmIsAddressValid((CHAR*)(ImageBase + ImageImportDescriptor->Name)))
		{
			CHAR ModuleName[MAX_PATH] = {0};
			CHAR szNtkrnlpa[] = {'n','t','k','r','n','l','p','a','.','e','x','e','\0'};
			CHAR szNtoskrnl[] = {'n','t','o','s','k','r','n','l','.','e','x','e','\0'};
			CHAR szNtkrnlmp[] = {'n','t','k','r','n','l','m','p','.','e','x','e','\0'};
			CHAR szNtkrpamp[] = {'n','t','k','r','p','a','m','p','.','e','x','e','\0'};
			CHAR szHalmacpi[] = {'h','a','l','m','a','c','p','i','.','d','l','l','\0'};
			CHAR szHal[] = {'h','a','l','.','d','l','l','\0'};

			strcpy(ModuleName, (CHAR*)(ImageBase + ImageImportDescriptor->Name));

			// ntoskrnl.exe(NTKRNLPA.exe、ntkrnlmp.exe、ntkrpamp.exe)：
			if (_stricmp(ModuleName, szNtkrnlpa) == 0 ||
				_stricmp(ModuleName, szNtoskrnl) == 0 ||
				_stricmp(ModuleName, szNtkrnlmp) == 0 ||
				_stricmp(ModuleName, szNtkrpamp) == 0 )
			{
				ModuleBase = GetKernelModuleBase(szNtoskrnl);
				if (ModuleBase == NULL)
				{
					ModuleBase = GetKernelModuleBase(szNtkrnlpa);
					if (ModuleBase == NULL)
					{
						ModuleBase = GetKernelModuleBase(szNtkrnlmp);
						if (ModuleBase == NULL)
						{
							ModuleBase = GetKernelModuleBase(szNtkrpamp);
						}
					}
				}
			}
			else if (!_stricmp(ModuleName, szHal) ||
				!_stricmp(ModuleName, szHalmacpi))
			{
				KdPrint(("GetKernelModuleBase: %s\n", szHal));
				ModuleBase = GetKernelModuleBase(szHal);
				if (!ModuleBase)
				{
					ModuleBase = GetKernelModuleBase(szHalmacpi);
				}
			}
			else
			{
				ModuleBase = GetKernelModuleBase(ModuleName);
			}

			if (ModuleBase == NULL)
			{
				KdPrint(("FixImportTable-> can't find module:%s\n", ModuleName));
				
// 				FirstThunk = (PIMAGE_THUNK_DATA)(ImageBase + ImageImportDescriptor->FirstThunk);
// 				InsertOriginalFirstThunk((DWORD)ImageBase, ExistImageBase, FirstThunk);
// 				ImageImportDescriptor++;
// 				continue;

				// 如果有一个模块未修复，那么直接返回FALSE
				return FALSE;
			}

			ImageThunkData = (PIMAGE_THUNK_DATA)(ImageBase + ImageImportDescriptor->OriginalFirstThunk);
			FirstThunk = (PIMAGE_THUNK_DATA)(ImageBase + ImageImportDescriptor->FirstThunk);
			while(ImageThunkData->u1.Ordinal)
			{
				// 序号导入
				if(IMAGE_SNAP_BY_ORDINAL32(ImageThunkData->u1.Ordinal))
				{
					FunctionAddress = (DWORD)MiFindExportedRoutine(ModuleBase, FALSE, NULL, ImageThunkData->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32);
					if (FunctionAddress == 0)
					{
						KdPrint(("can't find funcion Index %d \n", ImageThunkData->u1.Ordinal & ~IMAGE_ORDINAL_FLAG32));
						return FALSE;
					}

					FirstThunk->u1.Function = FunctionAddress;
				}
				else // 函数名导入
				{
					ImortByName = (PIMAGE_IMPORT_BY_NAME)(ImageBase+ImageThunkData->u1.AddressOfData);
					FunctionAddress = (DWORD)MiFindExportedRoutine(ModuleBase, TRUE, ImortByName->Name, 0);
					if (FunctionAddress == 0)
					{
						KdPrint(("can't Funcion Name:%s\n", ImortByName->Name));
						return FALSE;
					}

					FirstThunk->u1.Function = FunctionAddress;
				}

				FirstThunk++;
				ImageThunkData++;
			}

			ImageImportDescriptor++;
		}
	}
	__except(1)
	{
		return FALSE;
	}

	return TRUE;
}

PIMAGE_BASE_RELOCATION
LdrProcessRelocationBlockLongLong(
								  IN ULONG_PTR VA,
								  IN ULONG SizeOfBlock,
								  IN PUSHORT NextOffset,
								  IN LONGLONG Diff
								  )
{
	PUCHAR FixupVA;
	USHORT Offset;
	LONG Temp;
	ULONG Temp32;
	ULONGLONG Value64;
	LONGLONG Temp64;

	while (SizeOfBlock-- && MmIsAddressValid(NextOffset)) {

		Offset = *NextOffset & (USHORT)0xfff;
		FixupVA = (PUCHAR)(VA + Offset);
		
		if (!MmIsAddressValid(FixupVA))
		{
			break;
		}

		//
		// Apply the fixups.
		//

		switch ((*NextOffset) >> 12) {

			case IMAGE_REL_BASED_HIGHLOW :
				//
				// HighLow - (32-bits) relocate the high and low half
				//      of an address.
				//
				*(LONG UNALIGNED *)FixupVA += (ULONG) Diff;
				break;

			case IMAGE_REL_BASED_HIGH :
				//
				// High - (16-bits) relocate the high half of an address.
				//
				Temp = *(PUSHORT)FixupVA << 16;
				Temp += (ULONG) Diff;
				*(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
				break;

			case IMAGE_REL_BASED_HIGHADJ :
				//
				// Adjust high - (16-bits) relocate the high half of an
				//      address and adjust for sign extension of low half.
				//

				//
				// If the address has already been relocated then don't
				// process it again now or information will be lost.
				//
				if (Offset & LDRP_RELOCATION_FINAL) {
					++NextOffset;
					--SizeOfBlock;
					break;
				}

				Temp = *(PUSHORT)FixupVA << 16;
				++NextOffset;
				--SizeOfBlock;
				Temp += (LONG)(*(PSHORT)NextOffset);
				Temp += (ULONG) Diff;
				Temp += 0x8000;
				*(PUSHORT)FixupVA = (USHORT)(Temp >> 16);

				break;

			case IMAGE_REL_BASED_LOW :
				//
				// Low - (16-bit) relocate the low half of an address.
				//
				Temp = *(PSHORT)FixupVA;
				Temp += (ULONG) Diff;
				*(PUSHORT)FixupVA = (USHORT)Temp;
				break;

			case IMAGE_REL_BASED_IA64_IMM64:

				//
				// Align it to bundle address before fixing up the
				// 64-bit immediate value of the movl instruction.
				//

				FixupVA = (PUCHAR)((ULONG_PTR)FixupVA & ~(15));
				Value64 = (ULONGLONG)0;

				//
				// Extract the lower 32 bits of IMM64 from bundle
				//


				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM7B_INST_WORD_X,
					EMARCH_ENC_I17_IMM7B_SIZE_X,
					EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM7B_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM9D_INST_WORD_X,
					EMARCH_ENC_I17_IMM9D_SIZE_X,
					EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM9D_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM5C_INST_WORD_X,
					EMARCH_ENC_I17_IMM5C_SIZE_X,
					EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM5C_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IC_INST_WORD_X,
					EMARCH_ENC_I17_IC_SIZE_X,
					EMARCH_ENC_I17_IC_INST_WORD_POS_X,
					EMARCH_ENC_I17_IC_VAL_POS_X);
				EXT_IMM64(Value64,
					(PULONG)FixupVA + EMARCH_ENC_I17_IMM41a_INST_WORD_X,
					EMARCH_ENC_I17_IMM41a_SIZE_X,
					EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41a_VAL_POS_X);

				EXT_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41b_INST_WORD_X),
					EMARCH_ENC_I17_IMM41b_SIZE_X,
					EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41b_VAL_POS_X);
				EXT_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41c_INST_WORD_X),
					EMARCH_ENC_I17_IMM41c_SIZE_X,
					EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41c_VAL_POS_X);
				EXT_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_SIGN_INST_WORD_X),
					EMARCH_ENC_I17_SIGN_SIZE_X,
					EMARCH_ENC_I17_SIGN_INST_WORD_POS_X,
					EMARCH_ENC_I17_SIGN_VAL_POS_X);
				//
				// Update 64-bit address
				//

				Value64+=Diff;

				//
				// Insert IMM64 into bundle
				//

				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM7B_INST_WORD_X),
					EMARCH_ENC_I17_IMM7B_SIZE_X,
					EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM7B_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM9D_INST_WORD_X),
					EMARCH_ENC_I17_IMM9D_SIZE_X,
					EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM9D_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM5C_INST_WORD_X),
					EMARCH_ENC_I17_IMM5C_SIZE_X,
					EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM5C_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IC_INST_WORD_X),
					EMARCH_ENC_I17_IC_SIZE_X,
					EMARCH_ENC_I17_IC_INST_WORD_POS_X,
					EMARCH_ENC_I17_IC_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41a_INST_WORD_X),
					EMARCH_ENC_I17_IMM41a_SIZE_X,
					EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41a_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41b_INST_WORD_X),
					EMARCH_ENC_I17_IMM41b_SIZE_X,
					EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41b_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_IMM41c_INST_WORD_X),
					EMARCH_ENC_I17_IMM41c_SIZE_X,
					EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X,
					EMARCH_ENC_I17_IMM41c_VAL_POS_X);
				INS_IMM64(Value64,
					((PULONG)FixupVA + EMARCH_ENC_I17_SIGN_INST_WORD_X),
					EMARCH_ENC_I17_SIGN_SIZE_X,
					EMARCH_ENC_I17_SIGN_INST_WORD_POS_X,
					EMARCH_ENC_I17_SIGN_VAL_POS_X);
				break;

			case IMAGE_REL_BASED_DIR64:

				*(ULONGLONG UNALIGNED *)FixupVA += Diff;

				break;

			case IMAGE_REL_BASED_MIPS_JMPADDR :
				//
				// JumpAddress - (32-bits) relocate a MIPS jump address.
				//
				Temp = (*(PULONG)FixupVA & 0x3ffffff) << 2;
				Temp += (ULONG) Diff;
				*(PULONG)FixupVA = (*(PULONG)FixupVA & ~0x3ffffff) |
					((Temp >> 2) & 0x3ffffff);

				break;

			case IMAGE_REL_BASED_ABSOLUTE :
				//
				// Absolute - no fixup required.
				//
				break;

			case IMAGE_REL_BASED_SECTION :
				//
				// Section Relative reloc.  Ignore for now.
				//
				break;

			case IMAGE_REL_BASED_REL32 :
				//
				// Relative intrasection. Ignore for now.
				//
				break;

			default :
				//
				// Illegal - illegal relocation type.
				//

				return (PIMAGE_BASE_RELOCATION)NULL;
		}
		++NextOffset;
	}
	return (PIMAGE_BASE_RELOCATION)NextOffset;
}

BOOL
FixBaseRelocTable (
    PVOID NewImageBase,
	DWORD ExistImageBase
    )
/*++

Routine Description:

    This routine relocates an image file that was not loaded into memory
    at the preferred address.

Arguments:

    NewBase - Supplies a pointer to the image base.

    AdditionalBias - An additional quantity to add to all fixups.  The
                     32-bit X86 loader uses this when loading 64-bit images
                     to specify a NewBase that is actually a 64-bit value.

    LoaderName - Indicates which loader routine is being called from.

    Success - Value to return if relocation successful.

    Conflict - Value to return if can't relocate.

    Invalid - Value to return if relocations are invalid.

Return Value:

    Success if image is relocated.
    Conflict if image can't be relocated.
    Invalid if image contains invalid fixups.

--*/

{
    LONGLONG Diff;
    ULONG TotalCountBytes = 0;
    ULONG_PTR VA;
    ULONGLONG OriginalImageBase;
    ULONG SizeOfBlock;
    PUCHAR FixupVA;
    USHORT Offset;
    PUSHORT NextOffset = NULL;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	UNICODE_STRING unRtlImageNtHeader;
	UNICODE_STRING unFunc;
	pfnRtlImageDirectoryEntryToData MzfRtlImageDirectoryEntryToData = NULL;
	pfnRtlImageNtHeader MzfRtlImageNtHeader = NULL;
	WCHAR szFunc[] = {'R','t','l','I','m','a','g','e','D','i','r','e','c','t','o','r','y','E','n','t','r','y','T','o','D','a','t','a','\0'};
	WCHAR szRtlImageNtHeader[] = {'R','t','l','I','m','a','g','e','N','t','H','e','a','d','e','r','\0'};

	RtlInitUnicodeString(&unFunc, szFunc);
	RtlInitUnicodeString(&unRtlImageNtHeader, szRtlImageNtHeader);

	MzfRtlImageDirectoryEntryToData = (pfnRtlImageDirectoryEntryToData)MmGetSystemRoutineAddress(&unFunc);
	MzfRtlImageNtHeader = (pfnRtlImageNtHeader)MmGetSystemRoutineAddress(&unRtlImageNtHeader);
	
	if (!MzfRtlImageNtHeader || !MzfRtlImageDirectoryEntryToData)
	{
		return FALSE;
	}

	__try
	{
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

		NtHeaders = MzfRtlImageNtHeader( NewImageBase );
		if (NtHeaders == NULL) 
		{
			return FALSE;
		}

		switch (NtHeaders->OptionalHeader.Magic) {

		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:

			OriginalImageBase =
				((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;
			break;

		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:

			OriginalImageBase =
				((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;
			break;

		default:
			return FALSE;
		}

		//
		// Locate the relocation section.
		//

		NextBlock = (PIMAGE_BASE_RELOCATION)MzfRtlImageDirectoryEntryToData(
			NewImageBase, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_BASERELOC, 
			&TotalCountBytes
			);

		//
		// It is possible for a file to have no relocations, but the relocations
		// must not have been stripped.
		//

		if (!NextBlock || !TotalCountBytes) 
		{
			/*return TRUE;*/
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

		//
		// If the image has a relocation table, then apply the specified fixup
		// information to the image.
		//
		Diff = (ULONG_PTR)ExistImageBase - OriginalImageBase;
		while (TotalCountBytes)
		{
			SizeOfBlock = NextBlock->SizeOfBlock;
			TotalCountBytes -= SizeOfBlock;
			SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
			SizeOfBlock /= sizeof(USHORT);
			NextOffset = (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

			VA = (ULONG_PTR)NewImageBase + NextBlock->VirtualAddress;

			if ( !(NextBlock = LdrProcessRelocationBlockLongLong( VA,
				SizeOfBlock,
				NextOffset,
				Diff)) ) 
			{
				KdPrint(("%s: Unknown base relocation type\n"));
				return FALSE;
			}
		}
	}
	__except(1)
	{
		return FALSE;
	}

    return TRUE;
}

BOOL PeLoad(WCHAR* szFilePath, PULONG NewBase, DWORD OriginBase)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE hFile;
	LARGE_INTEGER FileSize;
	DWORD Length = 0;
	BYTE *FileBuffer = NULL;
	BYTE *ImageBase = NULL;
	IO_STATUS_BLOCK IoStatus;

	if (!szFilePath || !NewBase || !OriginBase)
	{
		return FALSE;
	}
	
	*NewBase = 0;

	KdPrint(("PeLoad: %S\n", szFilePath));

	Status = KernelOpenFile(szFilePath, 
		&hFile, 
		SYNCHRONIZE | FILE_EXECUTE, 
		FILE_ATTRIBUTE_NORMAL, 
		FILE_SHARE_READ, 
		FILE_OPEN, 
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (!NT_SUCCESS(Status))
	{
		return FALSE;
	}

	Status = KernelGetFileSize(hFile, &FileSize);
	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);
		return FALSE;
	}

	Length = FileSize.LowPart;
	FileBuffer = ExAllocatePoolWithTag(PagedPool, Length, MZFTAG);
	if (FileBuffer == NULL)
	{
		ZwClose(hFile);
		return FALSE;
	}

	Status = KernelReadFile(hFile, NULL, Length, FileBuffer, &IoStatus);
	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);
		ExFreePool(FileBuffer);
		return FALSE;
	}

	ZwClose(hFile);

	if(!ImageFile(FileBuffer, &ImageBase))
	{
		KdPrint(("ImageFile failed\n"));
		ExFreePool(FileBuffer);
		return FALSE;
	}
	
	KdPrint(("ImageBase: 0x%08X\n", ImageBase));
	ExFreePool(FileBuffer);

	if(!FixImportTable(ImageBase, OriginBase))
	{
		KdPrint(("FixImportTable failed\n"));
		ExFreePool(ImageBase);
		return FALSE;
	}

	if(!FixBaseRelocTable(ImageBase, OriginBase))
	{
		KdPrint(("FixBaseRelocTable failed\n"));
		ExFreePool(ImageBase);
		return FALSE;
	}

	*NewBase = (ULONG)ImageBase;
	return TRUE;
}

BOOL 
MzfMapFile(
	   WCHAR* szFilePath,
	   ULONG* NewBase,
	   ULONG* NewEnd
	   )
{
	NTSTATUS Status;
	HANDLE hFile;
	LARGE_INTEGER FileSize;
	DWORD Length;
	BYTE *FileBuffer;
	BYTE *ImageBase;
	IO_STATUS_BLOCK IoStatus;

	if (!szFilePath || !NewBase || !NewEnd)
	{
		return FALSE;
	}

	Status = KernelOpenFile(szFilePath, &hFile, SYNCHRONIZE | FILE_EXECUTE, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(Status))
	{
		return FALSE;
	}

	Status = KernelGetFileSize(hFile, &FileSize);
	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);
		return FALSE;
	}

	Length = FileSize.LowPart;
	FileBuffer = ExAllocatePoolWithTag(PagedPool, Length, MZFTAG);
	if (!FileBuffer)
	{
		ULONG i = 1;
		do 
		{
			ULONG nLen = Length + 2048 * i;
			FileBuffer = ExAllocatePoolWithTag(PagedPool, nLen, 0); 
		} while (FileBuffer == NULL && ++i < 10);
	}

	if (FileBuffer == NULL)
	{
		ZwClose(hFile);
		return FALSE;
	}

	Status = KernelReadFile(hFile, NULL, Length, FileBuffer, &IoStatus);
	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);
		ExFreePool(FileBuffer);
		return FALSE;
	}

	ZwClose(hFile);

	if(!ImageFile(FileBuffer, &ImageBase))
	{
		KdPrint(("ImageFile failed\n"));
		ExFreePool(FileBuffer);
		return FALSE;
	}

	ExFreePool(FileBuffer);
	
	*NewBase = (ULONG)ImageBase;
	*NewEnd = (ULONG)ImageBase + Length;

	return TRUE;
}

//*************************************************
// Qualifier: 获取系统原始的KiServiceTable表
// Parameter: NewImageBase - Reload的内核基址
// Parameter: ExistImageBase - 系统中的内核基址
// Parameter: *NewKiServiceTable - 返回的KiServiceTable的地址
//*************************************************
BOOL GetOriginalKiServiceTable(DWORD NewImageBase, DWORD ExistImageBase, DWORD* NewKiServiceTable)
{
	PIMAGE_DOS_HEADER ImageDosHeader = NULL;
	PIMAGE_NT_HEADERS ImageNtHeaders = NULL;
	DWORD KeServiceDescriptorTableRva = 0;
	PIMAGE_BASE_RELOCATION ImageBaseReloc = NULL;
	DWORD RelocSize = 0;
	int ItemCount = 0, Index = 0;
	int Type = 0;
	PDWORD RelocAddress = NULL;
	DWORD RvaData = 0;
	DWORD count = 0;
	WORD *TypeOffset = NULL;
	CHAR szKeServiceDescriptorTable[] = 
		{'K','e','S','e','r','v','i','c','e','D','e','s','c','r','i','p','t','o','r','T','a','b','l','e','\0'};

	if (!NewImageBase || !ExistImageBase || !NewKiServiceTable)
	{
		return FALSE;
	}

	*NewKiServiceTable = 0;

	__try
	{
		ImageDosHeader = (PIMAGE_DOS_HEADER)NewImageBase;
		if (ImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return FALSE;
		}

		ImageNtHeaders = (PIMAGE_NT_HEADERS)(NewImageBase +ImageDosHeader->e_lfanew);
		if (ImageNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}

		KeServiceDescriptorTableRva = (DWORD)MiFindExportedRoutine((PVOID)NewImageBase, TRUE, szKeServiceDescriptorTable, 0);
		if (KeServiceDescriptorTableRva == 0)
		{
			return FALSE;
		}

		KeServiceDescriptorTableRva = KeServiceDescriptorTableRva - (DWORD)NewImageBase;
		ImageBaseReloc = MzfImageDirectoryEntryToData((PVOID)NewImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &RelocSize);
		if (ImageBaseReloc == NULL)
		{
			KdPrint(("ImageBaseReloc error\n"));
			return FALSE;
		}

		while (ImageBaseReloc->SizeOfBlock)
		{  
			count++;
			ItemCount = (ImageBaseReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			TypeOffset = (WORD*)((DWORD)ImageBaseReloc + sizeof(IMAGE_BASE_RELOCATION));
			for (Index = 0; Index < ItemCount; Index++)
			{
				Type = TypeOffset[Index] >> 12;
				if (Type == IMAGE_REL_BASED_HIGHLOW)
				{
					RelocAddress = (PDWORD)((DWORD)(TypeOffset[Index] & 0x0fff) + ImageBaseReloc->VirtualAddress + (DWORD)NewImageBase);
					RvaData = *RelocAddress - ExistImageBase;

					if (RvaData == KeServiceDescriptorTableRva)
					{
						if(*(USHORT*)((DWORD)RelocAddress - 2) == 0x05c7)
						{
							*NewKiServiceTable = *(DWORD*)((DWORD)RelocAddress + 4) - ExistImageBase + (DWORD)NewImageBase;
							KdPrint(("Find OriginalImage ssdt :%X\n", *NewKiServiceTable));
							return TRUE;
						}
					}
				}
			}

			ImageBaseReloc = (PIMAGE_BASE_RELOCATION)((DWORD)ImageBaseReloc + ImageBaseReloc->SizeOfBlock);
		}
	}
	__except(1)
	{
	}

	return FALSE;
}

//*************************************************
// Qualifier: 获得Reload内核之后的SSDT函数地址
// Parameter: OriginKiServiceTable - 系统中原始的SSDT函数数组
// Parameter: ReloadKiServiceTable - Reload之后需要保存的SSDT函数数组
// Parameter: NewBase - reload内核基址
// Parameter: OldBase - old内核基址
//*************************************************
BOOL FixReloadKiServiceTable(PDWORD OriginKiServiceTable, PDWORD ReloadKiServiceTable, DWORD NewBase, DWORD OldBase)
{
	DWORD i = 0;
	DWORD nCnt = 0;
	PServiceDescriptorTableEntry_t Ssdt = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
	if (!Ssdt || !OriginKiServiceTable || !ReloadKiServiceTable || !NewBase || !OldBase)
	{
		return FALSE;
	}

	for (i = 0, nCnt = Ssdt->NumberOfServices; i < nCnt; i++)
	{
		ReloadKiServiceTable[i] = OriginKiServiceTable[i] - OldBase + NewBase;
	//	KdPrint(("%d  origin: 0x%08X   new: 0x%08X\n", i, OriginKiServiceTable[i], ReloadKiServiceTable[i]));
	}

	return TRUE;
}

/*************************************************
Qualifier: 重载一份新的内核到内存中，然后把系统原始的SSDT表和reload后的ssdt表也保存起来
*************************************************/
BOOL ReloadNtoskrnl()
{
	ULONG KernelModuleBase = 0, KernelModuleSize = 0, NewImageBase = 0;
	WCHAR *szNtoskrnlPath = ExAllocatePoolWithTag(PagedPool, MAX_PATH * sizeof(WCHAR), MZFTAG);
	
	if (!szNtoskrnlPath)
	{
		return FALSE;
	}

 	memset(szNtoskrnlPath, 0, MAX_PATH * sizeof(WCHAR));

	if (!GetNtoskrnlInfo(szNtoskrnlPath, &KernelModuleBase, &KernelModuleSize))
	{
		KdPrint(("GetNtoskrnlInfo failed\n"));
		return FALSE;
	}

	SetGlobalVeriable(enumOriginKernelBase, KernelModuleBase);
	SetGlobalVeriable(enumKernelModuleSize, KernelModuleSize);

	KdPrint(("Path: %S, Base: 0x%08X, Size: 0x%X\n", szNtoskrnlPath, KernelModuleBase, KernelModuleSize));
	
	if (/*PeLoadWithoutFix(szNtoskrnlPath, &NewImageBaseWithoutFix) &&*/
		PeLoad(szNtoskrnlPath, &NewImageBase, KernelModuleBase)
		)
	{
		PServiceDescriptorTableEntry_t Ssdt = (PServiceDescriptorTableEntry_t)GetGlobalVeriable(enumNowSSDT);
		PVOID OriginKiServiceTable = NULL, ReloadKiServiceTable = NULL;
		PServiceDescriptorTableEntry_t OriginSSDT = NULL, ReloadSSDT = NULL;

		SetGlobalVeriable(enumNewKernelBase, NewImageBase);
		KdPrint(("Init Ntos module success, NewImageBase: 0x%08X\n", NewImageBase));

		//SetGlobalVeriable(enumNewKernelBaseWithoutFix, NewImageBaseWithoutFix);
		//KdPrint(("PeLoadWithoutFix SUCCESS: 0x%08X\n", NewImageBaseWithoutFix));
		
		OriginKiServiceTable = ExAllocatePoolWithTag(NonPagedPool, Ssdt->NumberOfServices * sizeof(DWORD), MZFTAG);
		OriginSSDT = ExAllocatePoolWithTag(NonPagedPool, sizeof(ServiceDescriptorTableEntry_t), MZFTAG);
		ReloadKiServiceTable = ExAllocatePoolWithTag(NonPagedPool, Ssdt->NumberOfServices * sizeof(DWORD), MZFTAG);
		ReloadSSDT = ExAllocatePoolWithTag(NonPagedPool, sizeof(ServiceDescriptorTableEntry_t), MZFTAG);

		if (OriginKiServiceTable && OriginSSDT && ReloadKiServiceTable && ReloadSSDT)
		{
			memset(OriginKiServiceTable, 0, Ssdt->NumberOfServices * sizeof(DWORD));
			memset(OriginSSDT, 0, sizeof(ServiceDescriptorTableEntry_t));
			memset(ReloadKiServiceTable, 0, Ssdt->NumberOfServices * sizeof(DWORD));
			memset(ReloadSSDT, 0, sizeof(ServiceDescriptorTableEntry_t));

			if(GetOriginalKiServiceTable(NewImageBase, KernelModuleBase, (PDWORD)&OriginKiServiceTable))
			{
				OriginSSDT->NumberOfServices = Ssdt->NumberOfServices;
				OriginSSDT->ParamTableBase = Ssdt->ParamTableBase;
				OriginSSDT->ServiceCounterTableBase = Ssdt->ServiceCounterTableBase;
				OriginSSDT->ServiceTableBase = OriginKiServiceTable;

				SetGlobalVeriable(enumOriginKiServiceTable, (ULONG)OriginKiServiceTable);
				SetGlobalVeriable(enumOriginSSDT, (ULONG)OriginSSDT);

				if (FixReloadKiServiceTable((PDWORD)OriginKiServiceTable, (PDWORD)ReloadKiServiceTable, (DWORD)NewImageBase, KernelModuleBase))
				{
					ReloadSSDT->NumberOfServices = Ssdt->NumberOfServices;
					ReloadSSDT->ParamTableBase = Ssdt->ParamTableBase;
					ReloadSSDT->ServiceCounterTableBase = Ssdt->ServiceCounterTableBase;
					ReloadSSDT->ServiceTableBase = ReloadKiServiceTable;

					SetGlobalVeriable(enumReloadKiServiceTable, (ULONG)ReloadKiServiceTable);
					SetGlobalVeriable(enumReloadSSDT, (ULONG)ReloadSSDT);

					if (szNtoskrnlPath)
					{
						ExFreePool(szNtoskrnlPath);
						szNtoskrnlPath = NULL;
					}

					return TRUE;
				}
			}
		}

		if (OriginKiServiceTable != NULL)
			ExFreePool(OriginKiServiceTable);

		if (ReloadKiServiceTable != NULL)
			ExFreePool(ReloadKiServiceTable);

		if (OriginSSDT != NULL)
			ExFreePool(OriginSSDT);

		if (ReloadSSDT != NULL)
			ExFreePool(ReloadSSDT);
	}
	
	if (NewImageBase)
		ExFreePool((PVOID)NewImageBase);

	if (szNtoskrnlPath)
	{
		ExFreePool(szNtoskrnlPath);
		szNtoskrnlPath = NULL;
	}
		
	return FALSE;
}