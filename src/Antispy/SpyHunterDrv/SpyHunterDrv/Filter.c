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
#include "Filter.h"
#include "Peload.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"
#include "Driver.h"

void AddFilterInfo(PDEVICE_OBJECT pDeviceObject, PDRIVER_OBJECT pAttachDriver, PFILTER_DRIVER pFilterDriver, FILTER_TYPE Type)
{
	if (pDeviceObject && pAttachDriver && pFilterDriver)
	{
		if (pFilterDriver->nCnt > pFilterDriver->nRetCnt)
		{
			ULONG nRetCnt = pFilterDriver->nRetCnt;
			PDRIVER_OBJECT pDriverObject = pDeviceObject->DriverObject;
			PKLDR_DATA_TABLE_ENTRY entry = NULL;

			pFilterDriver->Filter[nRetCnt].pDeviceObject = (ULONG)pDeviceObject;
			pFilterDriver->Filter[nRetCnt].Type = Type;

			if (IsUnicodeStringValid(&(pDriverObject->DriverName)))
			{
				ULONG nLen = CmpAndGetStringLength(&(pDriverObject->DriverName), 100);
				wcsncpy(pFilterDriver->Filter[nRetCnt].szDriverName, pDriverObject->DriverName.Buffer, nLen);
			}

			if (IsUnicodeStringValid(&(pAttachDriver->DriverName)))
			{
				ULONG nLen = CmpAndGetStringLength(&(pAttachDriver->DriverName), 100);
				wcsncpy(pFilterDriver->Filter[nRetCnt].szAttachedDriverName, pAttachDriver->DriverName.Buffer, nLen);
			}

			entry = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;

			if ((ULONG)entry > SYSTEM_ADDRESS_START)
			{
				if (IsUnicodeStringValid(&(entry->FullDllName)))
				{
					ULONG nLen = CmpAndGetStringLength(&(entry->FullDllName), MAX_PATH);
					wcsncpy(pFilterDriver->Filter[nRetCnt].szPath, entry->FullDllName.Buffer, nLen);
				}
				else if (IsUnicodeStringValid(&(entry->BaseDllName)))
				{
					ULONG nLen = CmpAndGetStringLength(&(entry->BaseDllName), MAX_PATH);
					wcsncpy(pFilterDriver->Filter[nRetCnt].szPath, entry->BaseDllName.Buffer, nLen);
				}
			}
		}

		pFilterDriver->nRetCnt++;
	}
}

// 获得文件系统过滤驱动
NTSTATUS GetFileFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnIoCreateFile MzfIoCreateFile = (pfnIoCreateFile)GetGlobalVeriable(enumIoCreateFile);
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnNtClose MzfNtClose = (pfnNtClose)GetGlobalVeriable(enumNtClose);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);
	POBJECT_TYPE FileObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoFileObjectType);
	WCHAR szC[] = {'\\', '?', '?', '\\', 'c', ':', '\\', '\0'};
	UNICODE_STRING unC;
	OBJECT_ATTRIBUTES oa;
	HANDLE hFile;
	IO_STATUS_BLOCK isb;
	PFILE_OBJECT pFileObject = NULL;

	if (!MzfPsGetCurrentThread ||
		!MzfNtClose ||
		!MzfIoCreateFile ||
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject ||
		!FileObjectType)
	{
		return status;
	}

	MzfInitUnicodeString(&unC, szC);
	InitializeObjectAttributes(&oa, &unC, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	status = MzfIoCreateFile(
		&hFile,
		SYNCHRONIZE,
		&oa,
		&isb,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_DIRECTORY_FILE,
		0,
		0,
		0,
		0,
		IO_NO_PARAMETER_CHECKING);

	if (NT_SUCCESS(status))
	{
		PDEVICE_OBJECT pDeviceObject = NULL;
		PETHREAD pThread = NULL;
		CHAR PreMode = 0;

		status = MzfObReferenceObjectByHandle(
			hFile,
			1,
			FileObjectType,
			KernelMode,
			&pFileObject,
			NULL);

		if (NT_SUCCESS(status))
		{
			if ( IoGetFileSystemVpbInfo(pFileObject, &pDeviceObject, NULL) )
			{
				while ( 1 )
				{
					PDRIVER_OBJECT pDriverObject = pDeviceObject->DriverObject;
					PDEVICE_OBJECT pAttachedDevice = pDeviceObject->AttachedDevice;

					pDeviceObject = pAttachedDevice;

					if ( !pAttachedDevice )
					{
						break;
					}

					AddFilterInfo( pAttachedDevice, pDriverObject, pFilterDriver, enumFile );
					KdPrint(("File-> Driver: 0x%08X, AttachedDevice: 0x%08X, AttachedDriver: 0x%08X", pDriverObject, pAttachedDevice, pAttachedDevice->DriverObject));
				}
			}

			MzfObfDereferenceObject(pFileObject);
		}

		pThread = MzfPsGetCurrentThread();
		PreMode = ChangePreMode(pThread);
		MzfNtClose(hFile);
		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

NTSTATUS GetFilterDriverByDeviceName(WCHAR *szDeviceName, PFILTER_DRIVER pFilterDriver, FILTER_TYPE Type)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING unDevice;
	PFILE_OBJECT pFileObject = NULL;
	PDEVICE_OBJECT pAttachDeviceObject = NULL;
	pfnIoGetDeviceObjectPointer MzfIoGetDeviceObjectPointer = (pfnIoGetDeviceObjectPointer)GetGlobalVeriable(enumIoGetDeviceObjectPointer);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!MzfIoGetDeviceObjectPointer ||
		!MzfObfDereferenceObject)
	{
		return status;
	}

	MzfInitUnicodeString(&unDevice, szDeviceName);

	status = MzfIoGetDeviceObjectPointer(
		&unDevice,
		FILE_READ_ATTRIBUTES,
		&pFileObject,
		&pAttachDeviceObject);

	if (NT_SUCCESS(status))
	{
		PDEVICE_OBJECT pDeviceObject = NULL;

		if ( IoGetFileSystemVpbInfo(pFileObject, &pDeviceObject, NULL) )
		{
			while ( 1 )
			{
				PDRIVER_OBJECT pDriverObject = pDeviceObject->DriverObject;
				PDEVICE_OBJECT pAttachedDevice = pDeviceObject->AttachedDevice;

				pDeviceObject = pAttachedDevice;
				if ( !pAttachedDevice )
				{
					break;
				}

				AddFilterInfo( pAttachedDevice, pDriverObject, pFilterDriver, Type );
				KdPrint(("Driver: 0x%08X, AttachedDevice: 0x%08X, AttachedDriver: 0x%08X", pDriverObject, pAttachedDevice, pAttachedDevice->DriverObject));
			}
		}

		MzfObfDereferenceObject(pFileObject);
	}

	return status;
}

NTSTATUS GetDiskFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDeviceHarddiskDr0[] = {
		'\\','D','e','v','i','c','e','\\','H','a','r','d','d','i','s','k','0','\\','D','R','0','\0'
	};

	return GetFilterDriverByDeviceName(szDeviceHarddiskDr0, pFilterDriver, enumDisk);
}

NTSTATUS GetVolumeFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDeviceVolume[] = {
		'\\','D','e','v','i','c','e','\\','H','a','r','d','d','i','s','k','V','o','l','u','m','e','1','\0'
	};

	return GetFilterDriverByDeviceName(szDeviceVolume, pFilterDriver, enumVolume);
}

NTSTATUS GetFilterDriverByDriverName(WCHAR *szDriverName, PFILTER_DRIVER pFilterDriver, FILTER_TYPE Type)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING unDriverName;
	PDRIVER_OBJECT pDriverObject = NULL;
	POBJECT_TYPE DriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);
	pfnObReferenceObjectByName MzfObReferenceObjectByName = (pfnObReferenceObjectByName)GetGlobalVeriable(enumObReferenceObjectByName);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (!DriverObjectType ||
		!MzfObReferenceObjectByName ||
		!MzfObfDereferenceObject)
	{
		return status;
	}
	
	MzfInitUnicodeString(&unDriverName, szDriverName);
	
	status = MzfObReferenceObjectByName(
		&unDriverName,
		0x40,
		NULL,
		0,
		DriverObjectType,
		KernelMode,
		NULL,
		&pDriverObject);

	if (NT_SUCCESS(status) && pDriverObject)
	{
		PDEVICE_OBJECT pDeviceObject = NULL;

		for ( pDeviceObject = pDriverObject->DeviceObject; pDeviceObject; pDeviceObject = pDeviceObject->NextDevice )
		{
			PDRIVER_OBJECT pDriver = pDeviceObject->DriverObject;
			PDEVICE_OBJECT pAttachDevice = NULL;

			for ( pAttachDevice = pDeviceObject->AttachedDevice; pAttachDevice; pAttachDevice = pAttachDevice->AttachedDevice )
			{
				AddFilterInfo( pAttachDevice, pDriver, pFilterDriver, Type );
				KdPrint(("Driver: 0x%08X, AttachedDevice: 0x%08X, AttachedDriver: 0x%08X", pDriver, pAttachDevice, pAttachDevice->DriverObject));
				pDriver = pAttachDevice->DriverObject;
			}
		}

		MzfObfDereferenceObject(pDriverObject);
	}

	return status;
}

NTSTATUS GetKbdClassFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDriverKbdClass[] = {
		'\\','D','r','i','v','e','r','\\','K','b','d','C','l','a','s','s','\0'	
	};

	return GetFilterDriverByDriverName(szDriverKbdClass, pFilterDriver, enumKeyboard);
}

NTSTATUS GetMouClassFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDriverMouClass[] = {
		'\\','D','r','i','v','e','r','\\','M','o','u','c','l','a','s','s','\0'	
	};

	return GetFilterDriverByDriverName(szDriverMouClass, pFilterDriver, enumMouse);
}

NTSTATUS GetI8242prtFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDriverI8042ptr[] = {
		'\\','D','r','i','v','e','r','\\','i','8','0','4','2','p','r','t','\0'	
	};

	return GetFilterDriverByDriverName(szDriverI8042ptr, pFilterDriver, enumI8042prt);
}

NTSTATUS GetTcpipFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDriverTcpip[] = {
		'\\','D','r','i','v','e','r','\\','T','c','p','i','p','\0'	
	};

	return GetFilterDriverByDriverName(szDriverTcpip, pFilterDriver, enumTcpip);
}

NTSTATUS GetNdisFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDriverNdis[] = {
		'\\','D','r','i','v','e','r','\\','N','D','I','S','\0'	
	};

	return GetFilterDriverByDriverName(szDriverNdis, pFilterDriver, enumNDIS);
}

NTSTATUS GetPnpManagerFilterDriver(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szDriverPnpManager[] = {
		'\\','D','r','i','v','e','r','\\','P','n','p','M','a','n','a','g','e','r','\0'	
	};

	return GetFilterDriverByDriverName(szDriverPnpManager, pFilterDriver, enumPnpManager);
}

VOID FixFilterDriverPath(PFILTER_DRIVER pFilterDriver)
{
	WCHAR szWindowsDirectory[MAX_PATH] = {0};
	WCHAR szDriverDirectory[MAX_PATH] = {0};

	if (!pFilterDriver)
	{
		return;
	}

	if (GetWindowsDirectory(szWindowsDirectory) && GetDriverDirectory(szDriverDirectory))
	{
		ULONG i = 0, nCnt = pFilterDriver->nRetCnt;

		__try
		{
			for (i = 0; i < nCnt; i++)
			{
				if (wcslen(pFilterDriver->Filter[i].szPath))
				{
					WCHAR *szTemp = NULL, *szOriginPath = pFilterDriver->Filter[i].szPath;
					WCHAR szPath[MAX_PATH] = {0};

					szTemp = wcschr(szOriginPath, L'\\');
					if (!szTemp) // 没有目录信息，只有一个驱动名字的，直接拼接Driver目录。
					{
						wcscpy(szPath, szDriverDirectory);
						wcscat(szPath, szOriginPath);
						wcscpy(szOriginPath, szPath);
						szOriginPath[wcslen(szPath)] = L'\0';
					}
					else
					{
						WCHAR szFuck[] = {'\\', '?', '?', '\\', '\0'};
						WCHAR szSystemRoot[] = {'\\', 'S', 'y', 's', 't', 'e', 'm', 'R', 'o', 'o', 't', '\0'};
						WCHAR szWindows[] = {'\\', 'W', 'i', 'n', 'd', 'o', 'w', 's', '\0'};
						WCHAR szWinnt[] = {'\\', 'W', 'i', 'n', 'n', 't', '\0'};

						if (wcslen(szOriginPath) >= wcslen(szFuck) && !_wcsnicmp(szOriginPath, szFuck, wcslen(szFuck)))
						{
							wcscpy(szPath, szOriginPath + wcslen(szFuck));
							wcscpy(szOriginPath, szPath);
							szOriginPath[wcslen(szPath)] = L'\0';
						}
						else if (wcslen(szOriginPath) >= wcslen(szSystemRoot) && !_wcsnicmp(szOriginPath, szSystemRoot, wcslen(szSystemRoot)))
						{
							wcscpy(szPath, szWindowsDirectory);
							wcscat(szPath, szOriginPath + wcslen(szSystemRoot));
							wcscpy(szOriginPath, szPath);
							szOriginPath[wcslen(szPath)] = L'\0';
						}
						else if (wcslen(szOriginPath) >= wcslen(szWindows) && !_wcsnicmp(szOriginPath, szWindows, wcslen(szWindows)))
						{
							wcscpy(szPath, szWindowsDirectory);
							wcscat(szPath, szOriginPath + wcslen(szWindows));
							wcscpy(szOriginPath, szPath);
							szOriginPath[wcslen(szPath)] = L'\0';
						}
						else if (wcslen(szOriginPath) >= wcslen(szWinnt) && !_wcsnicmp(szOriginPath, szWinnt, wcslen(szWinnt)))
						{
							wcscpy(szPath, szWindowsDirectory);
							wcscat(szPath, szOriginPath + wcslen(szWinnt));
							wcscpy(szOriginPath, szPath);
							szOriginPath[wcslen(szPath)] = L'\0';
						}
					}
				}
			}
		}
		__except(1)
		{
		}
	}
}

NTSTATUS EnumFilterDriver(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PFILTER_DRIVER pFilterDriver = (PFILTER_DRIVER)pOutBuffer;

	GetFileFilterDriver(pFilterDriver);
	GetDiskFilterDriver(pFilterDriver);
	GetVolumeFilterDriver(pFilterDriver);
	GetKbdClassFilterDriver(pFilterDriver);
	GetMouClassFilterDriver(pFilterDriver);
	GetI8242prtFilterDriver(pFilterDriver);
	GetTcpipFilterDriver(pFilterDriver);
	GetNdisFilterDriver(pFilterDriver);
	GetPnpManagerFilterDriver(pFilterDriver);
	
	if (pFilterDriver->nCnt >= pFilterDriver->nRetCnt)
	{
		FixFilterDriverPath(pFilterDriver);
		status = STATUS_SUCCESS;
	}

	return status;
}