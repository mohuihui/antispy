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
#include "ParseNTFS.h"
#include "..\\..\\Common\\Common.h"
#include "Atapi.h"
#include "Acpi.h"
#include "InitWindows.h"
#include "CommonFunction.h"
#include "AtapiRW.h"



NTSTATUS ReadSector(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PREAD_SECTORS_CONTEXT pReadSector = (PREAD_SECTORS_CONTEXT)pInBuffer;
	ULONG nSectorCnt = 0, nSectorNumber = 0;
	PDEVICE_OBJECT pDiskDevice = NULL;
	pfnProbeForWrite MzfProbeForWrite = (pfnProbeForWrite)GetGlobalVeriable(enumProbeForWrite);
	BOOL bOK = TRUE;

	if (uInSize != sizeof(READ_SECTORS_CONTEXT) || !MzfProbeForWrite)
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

	pDiskDevice = (PDEVICE_OBJECT)GetGlobalVeriable(enumAtapiParseDiskDriverObject);
	if (!pDiskDevice)
	{
		WCHAR szPhysicDriver[] = {'\\','D','e','v','i','c','e','\\','H','a','r','d','d','i','s','k','0','\\','D','R','0','\0'};
		pDiskDevice = GetAtapiDeviceObject(szPhysicDriver);
		SetGlobalVeriable(enumAtapiParseDiskDriverObject, (ULONG)pDiskDevice);
	}
	
	if (pDiskDevice == NULL)
	{
		return status;
	}

	KdPrint(("atapi object: 0x%08X\n", pDiskDevice));
	
	nSectorCnt =  pReadSector->nSectorCounts;
	nSectorNumber = pReadSector->nSectorNum;

	__try
	{
		MzfProbeForWrite(pOutBuffer, nSectorCnt * 512, 1);
	}
	__except(1)
	{
		bOK = FALSE;
	}

	if (bOK)
	{
		status = AtapiReadWriteDisk(pDiskDevice, IRP_MJ_READ, pOutBuffer, nSectorNumber, nSectorCnt);
	}
	
	return status;
}