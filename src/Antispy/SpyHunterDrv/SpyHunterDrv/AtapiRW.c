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
#include "AtapiRW.h"
#include <srb.h>
#include "InitWindows.h"
#include "CommonFunction.h"
#include "DeleteFile.h"

typedef struct _SENSE_DATA {
	unsigned char Valid;
	unsigned char SegmentNumber;
	unsigned char FileMark;
	unsigned char Information[4];
	unsigned char AdditionalSenseLength;
	unsigned char CommandSpecificInformation[4];
	unsigned char AdditionalSenseCode;
	unsigned char AdditionalSenseCodeQualifier;
	unsigned char FieldReplaceableUnitCode;
	unsigned char SenseKeySpecific[3];
} SENSE_DATA, *PSENSE_DATA;

NTSTATUS MyIoCallDriver(PDEVICE_OBJECT DeviceObject,PIRP Irp)//自己的IoCallDriver
{
	PIO_STACK_LOCATION stack;
	--Irp->CurrentLocation;
	stack = IoGetNextIrpStackLocation( Irp );
	Irp->Tail.Overlay.CurrentStackLocation = stack;//移动堆栈
	stack->DeviceObject = DeviceObject;
	return (DeviceObject->DriverObject->MajorFunction[(ULONG)stack->MajorFunction])(DeviceObject, Irp);
}

NTSTATUS IrpCompletionRoutine_0(
								IN PDEVICE_OBJECT  DeviceObject,
								IN PIRP  Irp,
								IN PVOID  Context
								)
{
	PMDL mdl = NULL;
	Irp->UserIosb->Status = Irp->IoStatus.Status;
	Irp->UserIosb->Information = Irp->IoStatus.Information;
	mdl = Irp->MdlAddress;

	if ( mdl )
	{
		KdPrint(("read size: %d..", Irp->IoStatus.Information));
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
	}

	KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);
	IoFreeIrp(Irp);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

ULONG RestoreAtapiDispatch(ULONG nIndex)
{
	ULONG OriginDispatch = 0;
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAtapiDriverObject);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (nIndex < 28)
	{
		PULONG pReload = (PULONG)GetGlobalVeriable(enumAtapiReloadDispatchs);
		
		if (MzfMmIsAddressValid &&
			(ULONG)pDriverObject > SYSTEM_ADDRESS_START &&
			MzfMmIsAddressValid(pDriverObject) &&
			(ULONG)pReload > SYSTEM_ADDRESS_START && 
			MzfMmIsAddressValid((PVOID)pReload[nIndex]))
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			OriginDispatch = InterlockedExchange((PLONG)&(pDriverObject->MajorFunction[nIndex]), pReload[nIndex]);
			WPON();
			KeLowerIrql(OldIrql);
		}
	}
	else if (nIndex == 28)
	{
		ULONG nOrigin = GetGlobalVeriable(enumOriginAtapiStartIoDispatch);
		ULONG nReload = GetGlobalVeriable(enumReloadAtapiStartIoDispatch);

		if (MzfMmIsAddressValid &&
			(ULONG)pDriverObject > SYSTEM_ADDRESS_START &&
			MzfMmIsAddressValid(pDriverObject) &&
			nOrigin > SYSTEM_ADDRESS_START &&
			nReload > SYSTEM_ADDRESS_START
			)
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			OriginDispatch = InterlockedExchange((PLONG)&(pDriverObject->DriverStartIo), nReload);
			WPON();
			KeLowerIrql(OldIrql);
		}
	}

	return OriginDispatch;
}

void RestoreAtapiHook(ULONG nIndex, ULONG pHookDispatch)
{
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAtapiDriverObject);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid &&
		pHookDispatch > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pHookDispatch) &&
		(ULONG)pDriverObject > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pDriverObject))
	{
		if (nIndex < 28)
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			InterlockedExchange((PLONG)&(pDriverObject->MajorFunction[nIndex]), pHookDispatch);
			WPON();
			KeLowerIrql(OldIrql);
		}
		else if (nIndex == 28)
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			InterlockedExchange((PLONG)&(pDriverObject->DriverStartIo), pHookDispatch);
			WPON();
			KeLowerIrql(OldIrql);
		}
	}
}

ULONG RestoreAcpiDispatch(ULONG nIndex)
{
	ULONG OriginDispatch = 0;
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAcpiDriverObject);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid &&
		(ULONG)pDriverObject > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid(pDriverObject))
	{
		if (nIndex < 28)
		{
			PULONG pReload = (PULONG)GetGlobalVeriable(enumAcpiReloadDispatchs);

			if ((ULONG)pReload > SYSTEM_ADDRESS_START && MzfMmIsAddressValid((PVOID)pReload[nIndex]))
			{
				KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
				WPOFF();
				OriginDispatch = InterlockedExchange((PLONG)&(pDriverObject->MajorFunction[nIndex]), pReload[nIndex]);
				WPON();
				KeLowerIrql(OldIrql);
			}
		}
	}

	return OriginDispatch;
}

void RestoreAcpiHook(ULONG nIndex, ULONG pHookDispatch)
{
	PDRIVER_OBJECT pDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAcpiReloadDispatchs);
	pfnMmIsAddressValid MzfMmIsAddressValid = (pfnMmIsAddressValid)GetGlobalVeriable(enumMmIsAddressValid);

	if (MzfMmIsAddressValid &&
		pHookDispatch > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pHookDispatch) &&
		(ULONG)pDriverObject > SYSTEM_ADDRESS_START &&
		MzfMmIsAddressValid((PVOID)pDriverObject))
	{
		if (nIndex < 28)
		{
			KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
			WPOFF();
			InterlockedExchange((PLONG)&(pDriverObject->MajorFunction[nIndex]), pHookDispatch);
			WPON();
			KeLowerIrql(OldIrql);
		}
	}
}

NTSTATUS AtapiReadWriteDisk (
						  PDEVICE_OBJECT dev_object,
						  ULONG MajorFunction, 
						  PVOID buffer,
						  ULONG DiskPos, //Number Of Sector
						  int BlockCount //Count Of Sectors
						  )
{
	NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
	PSCSI_REQUEST_BLOCK srb = NULL;
	PSENSE_DATA sense = NULL;
	KEVENT Event;
	PIRP irp = NULL;
	PMDL mdl = NULL;
	IO_STATUS_BLOCK isb;
	PIO_STACK_LOCATION isl = NULL;
	PVOID psense = NULL;
	PDRIVER_OBJECT pAtapiDriverObject = NULL, pAcpiDriverObject = NULL;
	ULONG nAtapiSCSIDispatch = 0, nAtapiStartIODispatch = 0;
	ULONG nAcpiSCSIDispatch = 0;

	srb = ExAllocatePoolWithTag(NonPagedPool, sizeof(SCSI_REQUEST_BLOCK), MZFTAG);
	if(!srb) {	return status; }

	sense = ExAllocatePoolWithTag(NonPagedPool, sizeof(SENSE_DATA), MZFTAG);
	psense = sense;
	if(!sense)
	{	
		ExFreePool(srb);
		return status;
	}

	memset(srb, 0, sizeof(SCSI_REQUEST_BLOCK));
	memset(sense, 0, sizeof(SENSE_DATA));
	srb->Length = sizeof(SCSI_REQUEST_BLOCK); // 更多关于srb,请看《SCSI 总线和IDE接口：协议、应用和编程》和《SCSI程序员指南》
	srb->Function = 0;
	srb->DataBuffer = buffer;
	srb->DataTransferLength = BlockCount<<9; // sector size*number of sector
	srb->QueueAction = SRB_FLAGS_DISABLE_AUTOSENSE;
	srb->SrbStatus = 0;
	srb->ScsiStatus = 0;
	srb->NextSrb = 0;
	srb->SenseInfoBuffer = sense;
	srb->SenseInfoBufferLength = sizeof(SENSE_DATA);
	if(MajorFunction == IRP_MJ_READ)
		srb->SrbFlags = SRB_FLAGS_DATA_IN;
	else
		srb->SrbFlags = SRB_FLAGS_DATA_OUT;

	if(MajorFunction == IRP_MJ_READ)
		srb->SrbFlags |= SRB_FLAGS_ADAPTER_CACHE_ENABLE;

	srb->SrbFlags |= SRB_FLAGS_DISABLE_AUTOSENSE;
	srb->TimeOutValue = (srb->DataTransferLength >> 10) + 1;
	srb->QueueSortKey = DiskPos;
	srb->CdbLength = 10;
	srb->Cdb[0] = 2 * ((UCHAR)MajorFunction + 17);
	srb->Cdb[1] = srb->Cdb[1] & 0x1F | 0x80;
	srb->Cdb[2] = (unsigned char)(DiskPos>>0x18)&0xFF;   //
	srb->Cdb[3] = (unsigned char)(DiskPos>>0x10)&0xFF;   //
	srb->Cdb[4] = (unsigned char)(DiskPos>>0x08)&0xFF;   //
	srb->Cdb[5] = (UCHAR)DiskPos;						 // 填写sector位置
	srb->Cdb[7] = (UCHAR)BlockCount>>0x08;
	srb->Cdb[8] = (UCHAR)BlockCount;

	KeInitializeEvent(&Event, 0, 0);

	irp = IoAllocateIrp(dev_object->StackSize,0);
	if (!irp)
	{
		ExFreePool(srb);
		ExFreePool(psense);
		KdPrint(("IoAllocateIrp error\n"));
		return status;
	}

	mdl = IoAllocateMdl(buffer, BlockCount<<9, 0, 0, irp);
	irp->MdlAddress = mdl;
	if(!mdl)
	{
		ExFreePool(srb);
		ExFreePool(psense);
		IoFreeIrp(irp);
		KdPrint(("IoAllocateMdl error\n"));
		return status;
	}

	MmProbeAndLockPages(mdl,0,(MajorFunction == IRP_MJ_READ?0:1));
	srb->OriginalRequest = irp;
	irp->UserIosb = &isb;
	irp->UserEvent = &Event;
	irp->IoStatus.Status = 0;
	irp->IoStatus.Information = 0;
	irp->Flags = IRP_SYNCHRONOUS_API|IRP_NOCACHE;
	irp->AssociatedIrp.SystemBuffer = 0;
	irp->Cancel = 0;
	irp->RequestorMode = 0;
	irp->CancelRoutine = 0;
	irp->Tail.Overlay.Thread = PsGetCurrentThread();
	isl = IoGetNextIrpStackLocation(irp);
	isl->DeviceObject = dev_object;
	isl->MajorFunction = IRP_MJ_SCSI;
	isl->Parameters.Scsi.Srb = srb;
	isl->CompletionRoutine = IrpCompletionRoutine_0;
	isl->Context = srb;
	isl->Control = SL_INVOKE_ON_CANCEL|SL_INVOKE_ON_SUCCESS|SL_INVOKE_ON_ERROR;

	pAtapiDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAtapiDriverObject);
	if (pAtapiDriverObject)
	{
		nAtapiSCSIDispatch = RestoreAtapiDispatch(IRP_MJ_SCSI);
		nAtapiStartIODispatch = RestoreAtapiDispatch(28);
	}

	pAcpiDriverObject = (PDRIVER_OBJECT)GetGlobalVeriable(enumAcpiDriverObject);
	if (pAcpiDriverObject)
	{
		nAcpiSCSIDispatch = RestoreAcpiDispatch(IRP_MJ_SCSI);
	}

	RecoverIopfCompleteRequest();

	status = MyIoCallDriver(dev_object,irp);
	KdPrint(("IoCallDriverBack\n"));

	if ( status == STATUS_PENDING )
	{
		KeWaitForSingleObject(&Event, 0, 0, 0, 0);
	}
	
	RestoreIopfCompleteRequest();

	if (pAtapiDriverObject)
	{
		RestoreAtapiHook(IRP_MJ_SCSI, nAtapiSCSIDispatch);
		RestoreAtapiHook(28, nAtapiStartIODispatch);
	}

	if (pAcpiDriverObject)
	{
		RestoreAcpiHook(IRP_MJ_SCSI, nAtapiSCSIDispatch);
	}

	if(srb->SenseInfoBuffer != psense && srb->SenseInfoBuffer)
	{
		ExFreePool(srb->SenseInfoBuffer);
	}

	ExFreePool(srb);
	ExFreePool(psense);

	if ( NT_SUCCESS(status) )
	{
		return status;
	}
	
	return STATUS_INSUFFICIENT_RESOURCES;
}