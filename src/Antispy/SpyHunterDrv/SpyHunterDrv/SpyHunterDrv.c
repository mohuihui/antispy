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
#include <ntifs.h>
#include <windef.h>
#include "SpyHunterDrv.h"
#include "Control.h"
#include "InitWindows.h"
#include "ProcessWnd.h"
#include "CommonFunction.h"

WCHAR szDeviceName[256] = {'\\','D','e','v','i','c','e','\0'};
WCHAR szDosName[256] = {'\\','D','o','s','D','e','v','i','c','e','s','\0'};

extern PSHORT NtBuildNumber;

NTSTATUS 
DispatchClose(
    IN PDEVICE_OBJECT		DeviceObject,
    IN PIRP					Irp
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS 
DispatchCreate(
				IN PDEVICE_OBJECT		DeviceObject,
				IN PIRP					Irp
				)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS pCurrentEprocess = PsGetCurrentProcess();
	ULONG nDebugportOffset = GetGlobalVeriable(enumDebugportOffset_EPROCESS);
	if (pCurrentEprocess && nDebugportOffset)
	{
		ULONG DebugObject = *(PULONG)((ULONG)pCurrentEprocess + nDebugportOffset);
		if (DebugObject)
		{
			status = STATUS_INVALID_PARAMETER;
		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS 
DispatchDeviceControl(
    IN PDEVICE_OBJECT		DeviceObject,
    IN PIRP					Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
	ULONG uIoControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
	DWORD uInSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	DWORD uOutSize = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	DWORD dwRet = 0;

    switch(uIoControlCode)
    {
	case IOCTL_NEITHER_CONTROL:
		{
			PVOID pOutBuffer = Irp->UserBuffer;
			PVOID pInBuffer = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
			status = CommunicatNeitherControl(pInBuffer, uInSize, pOutBuffer, uOutSize, &dwRet);
		}
		break;

    default:
		KdPrint(("IOCTL Code Error fuck\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = dwRet;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID 
DriverUnload(
    IN PDRIVER_OBJECT	DriverObject
    )
{
    PDEVICE_OBJECT pdoNextDeviceObj = DriverObject->DeviceObject;
	UNICODE_STRING usSymlinkName;
	
	UnInitProcessVariable();

	RtlInitUnicodeString(&usSymlinkName, szDosName);
    IoDeleteSymbolicLink(&usSymlinkName);
    while(pdoNextDeviceObj)
    {
        PDEVICE_OBJECT pdoThisDeviceObj = pdoNextDeviceObj;
        pdoNextDeviceObj = pdoThisDeviceObj->NextDevice;
        IoDeleteDevice(pdoThisDeviceObj);
    }
}

BOOL IsSupportedSystem()
{
	return (
		*NtBuildNumber == 2195									||
		*NtBuildNumber == 2600									|| 
		*NtBuildNumber == 3790									||
		*NtBuildNumber >= 0x1770 && *NtBuildNumber <= 0x1772	|| 
		*NtBuildNumber == 7600									|| 
		*NtBuildNumber == 7601									||
		*NtBuildNumber == 9200
		);
}

NTSTATUS 
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING      RegistryPath
    )
{
    PDEVICE_OBJECT pdoDeviceObj = 0;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING usSymlinkName, usDeviceName;
	WCHAR* szRegName = NULL;
	WCHAR *szRegPath = NULL;
	ULONG nLen = 0;

	KdPrint(("Enter DriverEntry\n"));
	
//	return STATUS_INVALID_DISPOSITION;

	if (!DriverObject || !RegistryPath)
	{
		return status;
	}

	if (!IsSupportedSystem())
	{
		KdPrint(("not SupportedSystem\n"));
		return status;
	}

	// 如果检测ntoskrnl的checksum失败，那么就返回一个特定的错误码
	if (!NT_SUCCESS(CheckUpdate()))
	{
		KdPrint(("CheckUpdate error\n"));
		return STATUS_INVALID_DISPOSITION;
	}

	nLen = RegistryPath->MaximumLength + sizeof(WCHAR);
	szRegPath = (WCHAR *)ExAllocatePoolWithTag(PagedPool, nLen, MZFTAG);
	if (szRegPath == NULL)
	{
		return status;
	}
	
	memset(szRegPath, 0, nLen);
	wcsncpy(szRegPath, RegistryPath->Buffer, RegistryPath->Length / sizeof(WCHAR));
	KdPrint(("szRegPath: %S\n", szRegPath));

	szRegName = wcsrchr(szRegPath, '\\');
	if (szRegName && szRegName[1] != 0 && wcslen(szRegName) < 200)
	{
		InitWindows(DriverObject);

		wcscat(szDeviceName, szRegName);
		wcscat(szDosName, szRegName);
 		KdPrint(("szRegName: %S, szDeviceName: %S, szDosName: %S\n", szRegName, szDeviceName, szDosName)); 

		MzfInitUnicodeString(&usDeviceName, szDeviceName);
		if(!NT_SUCCESS(status = IoCreateDevice(
			DriverObject,
			0,
			&usDeviceName,
			FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN,
			FALSE,
			&pdoDeviceObj
			)))
		{
			KdPrint(("IoCreateDevice Error\n"));
			ExFreePool(szRegPath);
			return status;
		}

		MzfInitUnicodeString(&usSymlinkName, szDosName);
		if(!NT_SUCCESS(status = IoCreateSymbolicLink(
			&usSymlinkName,
			&usDeviceName
			)))
		{
			KdPrint(("IoCreateSymbolicLink Error\n"));
			ExFreePool(szRegPath);
			IoDeleteDevice(pdoDeviceObj);
			return status;
		}

		DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
		DriverObject->DriverUnload = DriverUnload;

		status = STATUS_SUCCESS;
 	}

	ExFreePool(szRegPath);
    return status;
}