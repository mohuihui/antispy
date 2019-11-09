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
#include "MBR.h"
#include "AtapiRW.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "..\\..\\Common\\Common.h"
#include "Atapi.h"
#include "Acpi.h"

PDEVICE_OBJECT GetAtapiDeviceObject(WCHAR *szDiskDR0) 
{
	PDEVICE_OBJECT pRetDevice = NULL, pTempDevice = NULL;
	WCHAR szDriverDisk[] = {'\\','D','r','i','v','e','r','\\','D','i','s','k','\0'};
	UNICODE_STRING unDriverDisk;
	POBJECT_TYPE pDriverObjectType = (POBJECT_TYPE)GetGlobalVeriable(enumIoDriverObjectType);
	PDRIVER_OBJECT pDriverObject = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nLen = 0;
	pfnObReferenceObjectByName MzfObReferenceObjectByName = (pfnObReferenceObjectByName)GetGlobalVeriable(enumObReferenceObjectByName);
	pfnObQueryNameString MzfObQueryNameString = (pfnObQueryNameString)GetGlobalVeriable(enumObQueryNameString);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);
	
	if (!MzfObReferenceObjectByName ||
		!MzfObQueryNameString ||
		!MzfObfDereferenceObject ||
		!pDriverObjectType ||
		!MzfExAllocatePoolWithTag ||
		!MzfExFreePoolWithTag ||
		!szDiskDR0 ||
		(nLen = wcslen(szDiskDR0)) == 0)
	{	
		return pRetDevice;
	}

	MzfInitUnicodeString(&unDriverDisk, szDriverDisk);
	status = MzfObReferenceObjectByName(&unDriverDisk, 64, NULL, 0, pDriverObjectType, KernelMode, NULL, &pDriverObject);
	if (!NT_SUCCESS(status))
	{
		return pRetDevice;
	}

	pTempDevice = pDriverObject->DeviceObject;
	if ( !pTempDevice )	
	{
		return pRetDevice;
	}

	while ( 1 )
	{	
		if ( pTempDevice->DeviceType == FILE_DEVICE_DISK && pTempDevice->Flags & DO_DEVICE_HAS_NAME)
		{
			ULONG nNameLen = 0;
			status = MzfObQueryNameString(
				pTempDevice,
				NULL,
				0,
				&nNameLen);
			
			if (status == STATUS_INFO_LENGTH_MISMATCH && nNameLen > 0)
			{
				POBJECT_NAME_INFORMATION pNameBuffer = NULL;

				nNameLen += 256;
				pNameBuffer = (POBJECT_NAME_INFORMATION)MzfExAllocatePoolWithTag(NonPagedPool, nNameLen, MZFTAG);
				if (pNameBuffer)
				{
					memset(pNameBuffer, 0, nNameLen);

					status = MzfObQueryNameString(
						pTempDevice,
						pNameBuffer,
						nNameLen,
						&nNameLen);

					if (NT_SUCCESS(status) && 
						pNameBuffer->Name.Length / sizeof(WCHAR) == nLen &&
						!_wcsnicmp(pNameBuffer->Name.Buffer, szDiskDR0, nLen))
					{
						 pRetDevice = pTempDevice;
					}

					MzfExFreePoolWithTag(pNameBuffer, 0);
				}
			}
		}

		pTempDevice = pTempDevice->NextDevice;
		if ( pRetDevice != NULL || !pTempDevice )
		{
			break;
		}
	}

	MzfObfDereferenceObject(pDriverObject);

	KdPrint(("DRO Device: 0x%08X\n", pRetDevice));

	return pRetDevice;
}

NTSTATUS ReadMBR(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MBR pMbr = NULL;
	WCHAR *szDiskName = NULL;
	PVOID pBuffer = NULL;
	ULONG nReadBytes = 0;
	PDEVICE_OBJECT pDiskDevice = NULL;
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	
	if (uInSize != sizeof(COMMUNICATE_MBR) || !MzfProbeForWrite)
	{
		return status;
	}

	if (!GetGlobalVeriable(enumAtapiDriverObject))
	{
		IninAtapi();
	}
	
	if (!GetGlobalVeriable(enumAcpiDriverObject))
	{
		IninAcpi();
	}

	pMbr = (PCOMMUNICATE_MBR)pInBuffer;
	szDiskName = pMbr->op.Read.szDiskName;
	pDiskDevice = GetAtapiDeviceObject(szDiskName);
	if (pDiskDevice == NULL)
	{
		return status;
	}

	pBuffer =  pMbr->op.Read.pBuffer;
	nReadBytes = pMbr->op.Read.nReadBytes;
	
	KdPrint(("szDiskName: %S, pBuffer: 0x%08X, nReadBytes: %d\n", szDiskName, pBuffer, nReadBytes));

	__try
	{
		MzfProbeForWrite(pBuffer, nReadBytes, 1);
		status = AtapiReadWriteDisk(pDiskDevice, IRP_MJ_READ, pBuffer, 0, 1);
	}
	__except(1)
	{
		
	}

	return status;
}

NTSTATUS WriteMBR(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_MBR pMbr = NULL;
	WCHAR *szDiskName = NULL;
	PVOID pBuffer = NULL;
	ULONG nWriteBytes = 0;
	PDEVICE_OBJECT pDiskDevice = NULL;
	pfnProbeForRead MzfProbeForRead = (pfnProbeForRead)GetGlobalVeriable(enumProbeForRead);

	if (uInSize != sizeof(COMMUNICATE_MBR) || !MzfProbeForRead)
	{
		return status;
	}

	if (!GetGlobalVeriable(enumAtapiDriverObject))
	{
		IninAtapi();
	}

	if (!GetGlobalVeriable(enumAcpiDriverObject))
	{
		IninAcpi();
	}

	pMbr = (PCOMMUNICATE_MBR)pInBuffer;
	szDiskName = pMbr->op.Write.szDiskName;
	pDiskDevice = GetAtapiDeviceObject(szDiskName);
	if (pDiskDevice == NULL)
	{
		return status;
	}

	pBuffer =  pMbr->op.Write.pBuffer;
	nWriteBytes = pMbr->op.Write.nWriteBytes;

	KdPrint(("szDiskName: %S, pBuffer: 0x%08X, nReadBytes: %d\n", szDiskName, pBuffer, nWriteBytes));

	__try
	{
		MzfProbeForRead(pBuffer, nWriteBytes, 1);
		status = AtapiReadWriteDisk(pDiskDevice, IRP_MJ_WRITE, pBuffer, 0, 1);
	}
	__except(1)
	{
		
	}

	return status;
}