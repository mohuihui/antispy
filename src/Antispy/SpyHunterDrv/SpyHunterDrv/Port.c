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
#include "Port.h"
#include <tdiinfo.h>
#include <tdistat.h>
#include <Wsk.h>
#include "InitWindows.h"
#include "CommonFunction.h"

typedef struct _MIB_UDPROW_OWNER_PID
{
	DWORD           dwLocalAddr;
	DWORD           dwLocalPort;
	DWORD           dwOwningPid;
} MIB_UDPROW_OWNER_PID, *PMIB_UDPROW_OWNER_PID;

typedef struct _MIB_TCPROW_OWNER_PID
{
	DWORD       dwState;
	DWORD       dwLocalAddr;
	DWORD       dwLocalPort;
	DWORD       dwRemoteAddr;
	DWORD       dwRemotePort;
	DWORD       dwOwningPid;
} MIB_TCPROW_OWNER_PID, *PMIB_TCPROW_OWNER_PID;

typedef struct _NSI_PARAM
{
	ULONG_PTR UnknownParam1;
	SIZE_T UnknownParam2;
	PVOID UnknownParam3;
	SIZE_T UnknownParam4;
	ULONG UnknownParam5;
	ULONG UnknownParam6;
	PVOID UnknownParam7;
	SIZE_T UnknownParam8;
	PVOID UnknownParam9;
	SIZE_T UnknownParam10;
	PVOID UnknownParam11;
	SIZE_T UnknownParam12;
	PVOID UnknownParam13;
	SIZE_T UnknownParam14;
	SIZE_T ConnCount;      

}NSI_PARAM,*PNSI_PARAM;

typedef struct _INTERNAL_TCP_TABLE_SUBENTRY
{
	char bytesfill0[2];
	USHORT Port;
	ULONG dwIP;
	char bytesfill[20];

}INTERNAL_TCP_TABLE_SUBENTRY,*PINTERNAL_TCP_TABLE_SUBENTRY;

typedef struct _INTERNAL_TCP_TABLE_ENTRY
{
	INTERNAL_TCP_TABLE_SUBENTRY localEntry;
	INTERNAL_TCP_TABLE_SUBENTRY remoteEntry;

}INTERNAL_TCP_TABLE_ENTRY,*PINTERNAL_TCP_TABLE_ENTRY;

typedef struct _NSI_STATUS_ENTRY
{   
	ULONG  dwState;
	char bytesfill[8]; 

}NSI_STATUS_ENTRY,*PNSI_STATUS_ENTRY;

typedef struct _NSI_PROCESSID_INFO
{

	ULONG dwUdpProId;
	ULONG UnknownParam2;
	ULONG UnknownParam3;
	ULONG dwProcessId;
	ULONG UnknownParam5;
	ULONG UnknownParam6;
	ULONG UnknownParam7;
	ULONG UnknownParam8;

}NSI_PROCESSID_INFO,*PNSI_PROCESSID_INFO;

typedef struct _INTERNAL_UDP_TABLE_ENTRY
{
	char bytesfill0[2];
	USHORT Port;
	ULONG dwIP;
	char bytesfill[20];

}INTERNAL_UDP_TABLE_ENTRY,*PINTERNAL_UDP_TABLE_ENTRY;

extern LONG *NtBuildNumber;

enum {TCPPORT, UDPPORT};

#define IOCTL_NSI_GETALLPARAM 0x12001B

#define ntohs(s) \
	(((s >> 8) & 0x00FF) | \
	((s << 8) & 0xFF00))

#define FSCTL_TCP_BASE                  FILE_DEVICE_NETWORK

#define _TCP_CTL_CODE(function, method, access) \
	CTL_CODE(FSCTL_TCP_BASE, function, method, access)

#define IOCTL_TCP_QUERY_INFORMATION_EX  \
	_TCP_CTL_CODE(0, METHOD_NEITHER, FILE_ANY_ACCESS)

NTSTATUS GetObjectByName(
						 HANDLE *pFileHandle,
						 OUT PFILE_OBJECT  *FileObject,
						 IN WCHAR  *DeviceName
						 )
{
	UNICODE_STRING    deviceTCPUnicodeString;
	OBJECT_ATTRIBUTES  TCP_object_attr;
	NTSTATUS      status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK    IoStatus;
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	HANDLE FileHandle = NULL;

	if (!MzfObReferenceObjectByHandle || 
		!FileObject ||
		!DeviceName)
	{
		return status;
	}

	MzfInitUnicodeString(&deviceTCPUnicodeString, DeviceName);

	InitializeObjectAttributes(&TCP_object_attr,
		&deviceTCPUnicodeString,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
		0,
		0
		);

	status = ZwCreateFile(
		&FileHandle,
		GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
		&TCP_object_attr,
		&IoStatus,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		0,
		0,
		0
		);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("Failed to open"));
		return STATUS_UNSUCCESSFUL;
	}

	status = MzfObReferenceObjectByHandle(
		FileHandle,
		FILE_ANY_ACCESS,
		*IoFileObjectType,
		KernelMode,
		(PVOID*)FileObject,
		NULL
		);

//	ZwClose(FileHandle);
	if (pFileHandle)
	{
		*pFileHandle = FileHandle;
	}

	return status;
}

///////////////////////////////////////////////////////////////////////////////////
//
//  功能实现：XP-2003下枚举网络连接端口信息
//  输入参数：OutLength为输出缓冲区的大小
//        PortType为要枚举的端口类型
//        TCPPORT-TCP端口
//        UDPPORT-UDP端口
//
///////////////////////////////////////////////////////////////////////////////////
PVOID EnumXpPortInformation(OUT PULONG OutLength, IN USHORT PortType)
{
	ULONG  BufLen = PAGE_SIZE * 4;
	PVOID  pInputBuff = NULL;
	PVOID  pOutputBuff = NULL;
	PVOID  pOutBuf = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING  DeviceName;
	PFILE_OBJECT pFileObject = NULL;
	PDEVICE_OBJECT pDeviceObject = NULL;
	KEVENT  Event ;
	IO_STATUS_BLOCK StatusBlock;
	PIRP    pIrp = NULL;
	PIO_STACK_LOCATION StackLocation = NULL;
	ULONG    NumOutputBuffers = 0;
	ULONG    i = 0;
	TCP_REQUEST_QUERY_INFORMATION_EX    TdiId;
	HANDLE hFile = NULL;
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	
	if (!MzfExAllocatePoolWithTag ||
		!MzfObfDereferenceObject ||
		!OutLength)
	{
		return NULL;
	}

	*OutLength = 0;
	RtlZeroMemory(&TdiId, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));

	if(TCPPORT == PortType)
	{
		TdiId.ID.toi_entity.tei_entity = CO_TL_ENTITY;
	}
	else if(UDPPORT == PortType)
	{
		TdiId.ID.toi_entity.tei_entity = CL_TL_ENTITY;
	}

	TdiId.ID.toi_entity.tei_instance = ENTITY_LIST_ID;
	TdiId.ID.toi_class = INFO_CLASS_PROTOCOL;
	TdiId.ID.toi_type = INFO_TYPE_PROVIDER;
	TdiId.ID.toi_id = 0x102;

	pInputBuff = (PVOID)&TdiId;

	__try
	{
// 		if(UDPPORT == PortType)
// 		{
// 			BufLen *= 3;
// 		}

		pOutputBuff = MzfExAllocatePoolWithTag(NonPagedPool, BufLen, MZFTAG);
		if(NULL == pOutputBuff)
		{
			KdPrint(("输出缓冲区内存分配失败！\n"));
			return NULL;
		}

		memset(pOutputBuff, 0, BufLen);

		if(TCPPORT == PortType)
		{
			WCHAR szTcp[] = {'\\','D','e','v','i','c','e','\\','T','c','p','\0'};
			status = GetObjectByName(
				&hFile,
				&pFileObject,
				szTcp
				);
		}
		else if(UDPPORT == PortType)
		{
			WCHAR szUdp[] = {'\\','D','e','v','i','c','e','\\','U','d','p','\0'};
			status = GetObjectByName(
				&hFile,
				&pFileObject,
				szUdp
				);
		}

		if (!NT_SUCCESS(status))
		{
			KdPrint(("通过设备名获取设备对象失败！\n"));
			ExFreePool(pOutputBuff);
			pOutputBuff = NULL;

			if (hFile)
			{
				ZwClose(hFile);
				hFile = NULL;
			}

			return NULL;
		}
		
		pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
		if (NULL == pDeviceObject)
		{
			KdPrint(("获取设备对象失败！\n"));
			__leave;
		}

		KdPrint(("Tcpip Driver Object: 0x%08X\n", pDeviceObject->DriverObject));
		KeInitializeEvent(&Event, 0, FALSE);

		pIrp = IoBuildDeviceIoControlRequest(
			IOCTL_TCP_QUERY_INFORMATION_EX, 
			pDeviceObject, 
			pInputBuff, 
			sizeof(TCP_REQUEST_QUERY_INFORMATION_EX), 
			pOutputBuff,
			BufLen, 
			FALSE,
			&Event,
			&StatusBlock
			);

		if (NULL == pIrp)
		{
			KdPrint(("IRP生成失败！\n"));
			__leave;
		}

		StackLocation = IoGetNextIrpStackLocation(pIrp);
		StackLocation->FileObject = pFileObject; //不设置这里会蓝屏
		StackLocation->DeviceObject = pDeviceObject;

		status  = IoCallDriver(pDeviceObject, pIrp);

		KdPrint(("IoCallDriver STATUS: %d\n", RtlNtStatusToDosError(status)));

		if (STATUS_BUFFER_OVERFLOW == status)
		{
			KdPrint(("缓冲区太小！%d\n", StatusBlock.Information));
		}

		if (STATUS_PENDING == status)
		{
			KdPrint(("STATUS_PENDING"));

			status = KeWaitForSingleObject(
				&Event, 
				0, 
				0, 
				0, 
				0
				);
		}

		if(STATUS_CANCELLED == status)
		{
			KdPrint(("STATUS_CANCELLED"));
		}

		if(status == STATUS_SUCCESS)
		{
			KdPrint(("Information:%d\r\n", StatusBlock.Information));
			*OutLength = StatusBlock.Information;
			pOutBuf = pOutputBuff;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{ 
		if (pOutputBuff)
		{
			ExFreePool(pOutputBuff);
			pOutputBuff = NULL;
		}
		
		if (hFile)
		{
			ZwClose(hFile);
			hFile = NULL;
		}

		MzfObfDereferenceObject(pFileObject);
		return NULL;
	}

	if (hFile)
	{
		ZwClose(hFile);
		hFile = NULL;
	}
	MzfObfDereferenceObject(pFileObject);
	return pOutBuf;
}

NTSTATUS EnumXpPort(PPORT_INFO pPortInfos, ULONG nCanStore, PULONG nRetCnt)
{
	PVOID pTcpBuf = NULL;
	PVOID pUdpBuf = NULL;
	ULONG ulTcpBuf = 0;
	ULONG ulUdpBuf = 0;
	ULONG i = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nTotal = 0;

	if (!pPortInfos || nCanStore == 0 || nRetCnt == NULL)
	{
		return status;
	}

	*nRetCnt = 0;

	if (KeGetCurrentIrql() > PASSIVE_LEVEL)
	{
		return status;
	}

	pTcpBuf = EnumXpPortInformation(&ulTcpBuf, TCPPORT);
	if (pTcpBuf != NULL || ulTcpBuf > 0)
	{
		PMIB_TCPROW_OWNER_PID pTcpInfo = (PMIB_TCPROW_OWNER_PID)pTcpBuf;
		ULONG nCnt = ulTcpBuf / sizeof(MIB_TCPROW_OWNER_PID);

		for( ; MmIsAddressValid(pTcpInfo + i) && i < nCnt && pTcpInfo[i].dwOwningPid; i++ )
		{
			ULONG ulLocalPort = ntohs(pTcpInfo[i].dwLocalPort);
			ULONG ulRemotePort = ntohs(pTcpInfo[i].dwRemotePort);
			ULONG ulPid = pTcpInfo[i].dwOwningPid;

			if (pTcpInfo[i].dwRemoteAddr == 0)
			{
				ulRemotePort = 0;
			}

			// 排除一些垃圾的东西吧
			if (pTcpInfo[i].dwState > 20 ||
				ulLocalPort > 65535 ||
				ulPid <= 0)
			{
				continue;
			}
			
			if (nCanStore > nTotal)
			{
				pPortInfos[nTotal].nPortType = enumTcp;
				pPortInfos[nTotal].nConnectState = pTcpInfo[i].dwState;
				pPortInfos[nTotal].nLocalAddress = pTcpInfo[i].dwLocalAddr;
				pPortInfos[nTotal].nLocalPort = ulLocalPort;
				pPortInfos[nTotal].nRemoteAddress = pTcpInfo[i].dwRemoteAddr;;
				pPortInfos[nTotal].nRemotePort = ulRemotePort;
				pPortInfos[nTotal].nPid = ulPid;
			}

			KdPrint(("Pid: %d, LocalPort: %d, RemotePort: %d\n", ulPid, ulLocalPort, ulRemotePort));

			nTotal++;
		}
	}

	if (pTcpBuf)
	{
		ExFreePool(pTcpBuf);
		pTcpBuf = NULL;
	}

	//-------------------------------------------------------
	i = 0;
	pUdpBuf = EnumXpPortInformation(&ulUdpBuf, UDPPORT);
	if (pUdpBuf != NULL && ulUdpBuf > 0)
	{
		PMIB_UDPROW_OWNER_PID pUdpInfo = (PMIB_UDPROW_OWNER_PID)pUdpBuf;
		ULONG nCnt = ulTcpBuf / sizeof(MIB_UDPROW_OWNER_PID);

		for( ; 
			MmIsAddressValid(pUdpInfo) && i < nCnt && pUdpInfo[i].dwOwningPid; 
			i++ )
		{
			ULONG ulLocalPort = ntohs(pUdpInfo[i].dwLocalPort);
			ULONG ulPid = pUdpInfo[i].dwOwningPid;

			if (ulPid <= 0			||
				ulLocalPort > 65535 )
			{
				continue;
			}

			KdPrint(("Pid:%d, Port:%d\n", ulPid, ulLocalPort));
			
			if (nCanStore > nTotal)
			{
				pPortInfos[nTotal].nPortType = enumUdp;
				pPortInfos[nTotal].nConnectState = 1;
				pPortInfos[nTotal].nLocalAddress = pUdpInfo[i].dwLocalAddr;
				pPortInfos[nTotal].nLocalPort = ulLocalPort;
				pPortInfos[nTotal].nRemoteAddress = 0;
				pPortInfos[nTotal].nRemotePort = 0;
				pPortInfos[nTotal].nPid = ulPid;
			}

			nTotal++;
		}
	}
	
	if (pUdpBuf)
	{
		ExFreePool(pUdpBuf);
		pUdpBuf = NULL;
	}

	*nRetCnt = nTotal;
	return status; 
} 

//////////////////////////////////////////////////////////////////////////////////////////////////////
NTSTATUS  
	EnumTcpPortInformationWin7(
	PMIB_TCPROW_OWNER_PID  *TcpRow,
	ULONG_PTR  *len
	)
{
	PINTERNAL_TCP_TABLE_ENTRY  pBuf1 = NULL;
	PNSI_STATUS_ENTRY  pBuf2 = NULL;
	PNSI_PROCESSID_INFO  pBuf3 = NULL;
 	PMIB_TCPROW_OWNER_PID pOutputBuff = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT pFileObject = NULL;
	PDEVICE_OBJECT pDeviceObject = NULL;
	KEVENT    Event;
	IO_STATUS_BLOCK  StatusBlock;
	PIRP        pIrp = NULL;
	PIO_STACK_LOCATION StackLocation = NULL;
	ULONG_PTR  retLen = 0;
	ULONG   i = 0;
	NSI_PARAM   nsiStruct = {0};
	WCHAR szNsi[] = {'\\','d','e','v','i','c','e','\\','n','s','i','\0'};
	HANDLE hFile = NULL;
	NPI_MODULEID NPI_MS_TCP_MODULEID = 
	{
		sizeof(NPI_MODULEID),
		MIT_GUID,
		{0xEB004A03, 0x9B1A, 0x11D4,
		{0x91, 0x23, 0x00, 0x50, 0x04, 0x77, 0x59, 0xBC}}
	};
	pfnIoBuildDeviceIoControlRequest MzfIoBuildDeviceIoControlRequest = (pfnIoBuildDeviceIoControlRequest)GetGlobalVeriable(enumIoBuildDeviceIoControlRequest);

	if (!TcpRow || !len || !MzfIoBuildDeviceIoControlRequest)
	{
		return status;
	}

	*len = 0;
	*TcpRow = NULL;

	nsiStruct.UnknownParam3 = &NPI_MS_TCP_MODULEID;
	nsiStruct.UnknownParam4 = 3;
	nsiStruct.UnknownParam5 = 1;
	nsiStruct.UnknownParam6 = 1;
// nsiStruct.ConnCount=retLen;

	status = GetObjectByName(&hFile, &pFileObject, szNsi);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("获取设备名失败！\n"));
		goto __end;
	}

	pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

	if (!pDeviceObject)
	{
		KdPrint(("获取设备对象或文件对象失败！\n"));
		status = STATUS_UNSUCCESSFUL;
		goto __end;
	}

	KdPrint(("Tcpip IRP_MJ_DEVICE_CONTROL: %08lX\n", pDeviceObject->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]));

	KeInitializeEvent(&Event, NotificationEvent, FALSE);

	pIrp = MzfIoBuildDeviceIoControlRequest(
		IOCTL_NSI_GETALLPARAM, 
		pDeviceObject,
		&nsiStruct,
		sizeof(NSI_PARAM), 
		&nsiStruct,
		sizeof(NSI_PARAM), 
		FALSE, 
		&Event, 
		&StatusBlock
		);

	if (NULL == pIrp)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("IRP生成失败！\n"));
		goto __end;
	}

	StackLocation = IoGetNextIrpStackLocation(pIrp);
	StackLocation->FileObject = pFileObject; 
	// pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	StackLocation->DeviceObject = pDeviceObject;
	pIrp->RequestorMode = KernelMode;
	//StackLocation->MinorFunction = IRP_MN_USER_FS_REQUEST;

	status  = IoCallDriver(pDeviceObject, pIrp);

	KdPrint(("1STATUS: %08lX\n", status));

	if (STATUS_PENDING == status)
	{
		status = KeWaitForSingleObject(&Event, Executive,KernelMode,TRUE, 0);
	}

	retLen = nsiStruct.ConnCount;
	retLen += 2;

	pBuf1 = (PINTERNAL_TCP_TABLE_ENTRY)ExAllocatePoolWithTag(NonPagedPool, 56 * retLen, MZFTAG);
	if (NULL == pBuf1)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pBuf1, 56 * retLen);
	KdPrint(("pBuf1: 0x%08X\n", pBuf1));

	pBuf2 = (PNSI_STATUS_ENTRY)ExAllocatePoolWithTag(NonPagedPool, 16 * retLen, MZFTAG);
	if (NULL == pBuf2)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pBuf2, 16 * retLen);
	KdPrint(("pBuf2: 0x%08X\n", pBuf2));

	pBuf3 = (PNSI_PROCESSID_INFO)ExAllocatePoolWithTag(NonPagedPool, 32 * retLen, MZFTAG);
	if (NULL == pBuf3)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pBuf3, 32 * retLen);
	KdPrint(("pBuf3: 0x%08X\n", pBuf3));

	pOutputBuff = (PMIB_TCPROW_OWNER_PID)ExAllocatePoolWithTag(NonPagedPool, retLen * sizeof(MIB_TCPROW_OWNER_PID), MZFTAG);
	if (NULL == pOutputBuff)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pOutputBuff, retLen * sizeof(MIB_TCPROW_OWNER_PID));
	KdPrint(("pOutputBuff: 0x%08X\n", pOutputBuff));

	nsiStruct.UnknownParam7 = pBuf1;
	nsiStruct.UnknownParam8 = 56;
	nsiStruct.UnknownParam11 = pBuf2;
	nsiStruct.UnknownParam12 = 16;
	nsiStruct.UnknownParam13 = pBuf3;
	nsiStruct.UnknownParam14 = 32;

	pIrp = MzfIoBuildDeviceIoControlRequest(
		IOCTL_NSI_GETALLPARAM, 
		pDeviceObject,
		&nsiStruct,
		sizeof(NSI_PARAM), 
		&nsiStruct,
		sizeof(NSI_PARAM), 
		FALSE, 
		&Event, 
		&StatusBlock
		);
	if (NULL == pIrp)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	StackLocation = IoGetNextIrpStackLocation(pIrp);
	StackLocation->FileObject = pFileObject; 
	// pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	StackLocation->DeviceObject = pDeviceObject;
	pIrp->RequestorMode = KernelMode;
	//StackLocation->MinorFunction = IRP_MN_USER_FS_REQUEST;

	status = IoCallDriver(pDeviceObject, pIrp);

	KdPrint(("2STATUS:%08lX-%d\n", status, RtlNtStatusToDosError(status)));

	if (STATUS_PENDING == status)
	{
		status = KeWaitForSingleObject(&Event, Executive,KernelMode,TRUE, 0);
	}

	for(i = 0; i < nsiStruct.ConnCount; i++)
	{
		KdPrint(("state: %d\n", pBuf2[i].dwState));

		pOutputBuff[i].dwState = pBuf2[i].dwState;
		//if ( pBuf1[i].localEntry.bytesfill0 == 2 )
		//{
		pOutputBuff[i].dwLocalAddr = pBuf1[i].localEntry.dwIP;
		pOutputBuff[i].dwLocalPort = pBuf1[i].localEntry.Port;
		pOutputBuff[i].dwRemoteAddr = pBuf1[i].remoteEntry.dwIP;
		// }
		//  else
		/* {
		pOutputBuff[i].dwLocalAddr = 0;
		pOutputBuff[i].dwLocalPort=pBuf1[i].localEntry.Port;
		pOutputBuff[i].dwRemoteAddr= 0;

		}*/
		pOutputBuff[i].dwRemotePort = pBuf1[i].remoteEntry.Port;
		pOutputBuff[i].dwOwningPid = pBuf3[i].dwProcessId;
	}

	*TcpRow = pOutputBuff;
	*len = nsiStruct.ConnCount;
	status = STATUS_SUCCESS;

__end:

	if(NULL != pBuf1)
	{
		ExFreePool(pBuf1);
		pBuf1 = NULL;
	}

	if(NULL != pBuf2)
	{
		ExFreePool(pBuf2);
		pBuf2 = NULL;
	}

	if(NULL != pBuf3)
	{
		ExFreePool(pBuf3);
		pBuf3 = NULL;
	}

	if (NULL != pFileObject)
		ObDereferenceObject(pFileObject);

	if (hFile != NULL)
	{
		ZwClose(hFile);
		hFile = NULL;
	}

	if (!NT_SUCCESS(status))
	{
		if (pOutputBuff)
		{
			ExFreePool(pOutputBuff);
			pOutputBuff = NULL;
		}

		*TcpRow = NULL;
		*len = 0;
	}

	return status;
}

/////
//UDP
NTSTATUS  
	EnumUdpPortInformationWin7(
	PMIB_UDPROW_OWNER_PID  *udpRow,
	ULONG_PTR  *len
	)
{
	PINTERNAL_UDP_TABLE_ENTRY  pBuf1 = NULL;
	PNSI_PROCESSID_INFO  pBuf2 = NULL;
	PMIB_UDPROW_OWNER_PID pOutputBuff = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PFILE_OBJECT pFileObject = NULL;
	PDEVICE_OBJECT pDeviceObject = NULL;
	KEVENT    Event;
	IO_STATUS_BLOCK  StatusBlock;
	PIRP        pIrp = NULL;
	PIO_STACK_LOCATION StackLocation = NULL;
	ULONG_PTR  retLen = 0;
	NSI_PARAM   nsiStruct={0};
	ULONG i = 0;
	HANDLE hFile = NULL;
	WCHAR szNsi[] = {'\\','d','e','v','i','c','e','\\','n','s','i','\0'};
	NPI_MODULEID NPI_MS_UDP_MODULEID = 
	{
		sizeof(NPI_MODULEID),
		MIT_GUID,
		{0xEB004A02, 0x9B1A, 0x11D4,
		{0x91, 0x23, 0x00, 0x50, 0x04, 0x77, 0x59, 0xBC}}
	};

	if (!udpRow || !len)
	{
		return status;
	}

	*len = 0;
	*udpRow = NULL;

	nsiStruct.UnknownParam3 = &NPI_MS_UDP_MODULEID;
	nsiStruct.UnknownParam4 = 1;
	nsiStruct.UnknownParam5 = 1;
	nsiStruct.UnknownParam6 = 1;
	// nsiStruct.ConnCount=retLen;

	status = GetObjectByName(&hFile, &pFileObject, szNsi);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("获取设备名失败！\n"));
		goto __end;
	}

	pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
	if (!pDeviceObject)
	{
		KdPrint(("获取设备对象或文件对象失败！\n"));
		status = STATUS_UNSUCCESSFUL;
		goto __end;
	}

	KdPrint(("nsi IRP_MJ_DEVICE_CONTROL:%08lX\n", pDeviceObject->DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]));

	KeInitializeEvent(&Event, NotificationEvent, FALSE);

	pIrp = IoBuildDeviceIoControlRequest(
		IOCTL_NSI_GETALLPARAM, 
		pDeviceObject,
		&nsiStruct,
		sizeof(NSI_PARAM), 
		&nsiStruct,
		sizeof(NSI_PARAM), 
		FALSE, 
		&Event, 
		&StatusBlock
		);
	if (NULL == pIrp)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("IRP生成失败！\n"));
		goto __end;
	}

	StackLocation = IoGetNextIrpStackLocation(pIrp);
	StackLocation->FileObject = pFileObject; 
	//pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	StackLocation->DeviceObject = pDeviceObject;
	pIrp->RequestorMode = KernelMode;

	//StackLocation->MinorFunction = IRP_MN_USER_FS_REQUEST;

	status  = IoCallDriver(pDeviceObject, pIrp);

	KdPrint(("STATUS:%08lX\n", status));

	if (STATUS_PENDING == status)
	{
		status = KeWaitForSingleObject(&Event, Executive,KernelMode,TRUE, 0);
	}

	retLen = nsiStruct.ConnCount;
	retLen += 2;

	pBuf1 = (PINTERNAL_UDP_TABLE_ENTRY)ExAllocatePoolWithTag(NonPagedPool, sizeof(INTERNAL_UDP_TABLE_ENTRY) * retLen, MZFTAG);
	if (NULL == pBuf1)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pBuf1, sizeof(INTERNAL_UDP_TABLE_ENTRY) * retLen);

	pBuf2=  (PNSI_PROCESSID_INFO)ExAllocatePoolWithTag(NonPagedPool, sizeof(NSI_PROCESSID_INFO) * retLen, MZFTAG);
	if (NULL == pBuf2)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pBuf2, sizeof(NSI_PROCESSID_INFO) * retLen);

	pOutputBuff = (PMIB_UDPROW_OWNER_PID)ExAllocatePoolWithTag(NonPagedPool, retLen * sizeof(MIB_UDPROW_OWNER_PID), MZFTAG);
	if (NULL == pOutputBuff)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto __end;
	}
	RtlZeroMemory(pOutputBuff, retLen * sizeof(MIB_UDPROW_OWNER_PID));

	nsiStruct.UnknownParam7 = pBuf1;
	nsiStruct.UnknownParam8 = sizeof(INTERNAL_UDP_TABLE_ENTRY);
	nsiStruct.UnknownParam13 = pBuf2;
	nsiStruct.UnknownParam14 = sizeof(NSI_PROCESSID_INFO);

	pIrp = IoBuildDeviceIoControlRequest(
		IOCTL_NSI_GETALLPARAM, 
		pDeviceObject,
		&nsiStruct,
		sizeof(NSI_PARAM), 
		&nsiStruct,
		sizeof(NSI_PARAM), 
		FALSE, 
		&Event, 
		&StatusBlock
		);
	if (NULL == pIrp)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("IRP生成失败！\n"));
		goto __end;
	}

	StackLocation = IoGetNextIrpStackLocation(pIrp);
	StackLocation->FileObject = pFileObject; 
	// pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	StackLocation->DeviceObject = pDeviceObject;
	pIrp->RequestorMode = KernelMode;
	//StackLocation->MinorFunction = IRP_MN_USER_FS_REQUEST;

	status = IoCallDriver(pDeviceObject, pIrp);

	KdPrint(("STATUS:%08lX\n", status));

	if (STATUS_PENDING == status)
	{
		status = KeWaitForSingleObject(&Event, Executive,KernelMode,TRUE, 0);
	}

	for(i = 0; i < nsiStruct.ConnCount; i++)
	{
		//if ( pBuf1[i].localEntry.bytesfill0 == 2 )
		//{
		pOutputBuff[i].dwLocalAddr = pBuf1[i].dwIP;
		pOutputBuff[i].dwLocalPort = pBuf1[i].Port;

		// }
		//  else
		/* {
		pOutputBuff[i].dwLocalAddr = 0;
		pOutputBuff[i].dwLocalPort=pBuf1[i].localEntry.Port;
		pOutputBuff[i].dwRemoteAddr= 0;

		}*/
		pOutputBuff[i].dwOwningPid = pBuf2[i].dwUdpProId;
	}

	*udpRow = pOutputBuff;
	*len = nsiStruct.ConnCount;
	status = STATUS_SUCCESS;

__end:

	if(NULL != pBuf1 && MmIsAddressValid(pBuf1))
	{
		ExFreePool(pBuf1);
		pBuf1 = NULL;
	}

	if(NULL != pBuf2 && MmIsAddressValid(pBuf2))
	{
		ExFreePool(pBuf2);
		pBuf2 = NULL;
	}

	if (NULL != pFileObject)
		ObDereferenceObject(pFileObject);

	if (hFile)
	{
		ZwClose(hFile);
		hFile = NULL;
	}

	if (!NT_SUCCESS(status))
	{
		if (pOutputBuff && MmIsAddressValid(pOutputBuff))
		{
			ExFreePool(pOutputBuff);
			pOutputBuff = NULL;
		}

		*udpRow = NULL;
		*len = 0;
	}

	return status;
}

NTSTATUS PrintTcpIpInWin7(PPORT_INFO pPortInfos, ULONG nCanStore, PULONG nRetCnt)
{
	PMIB_TCPROW_OWNER_PID tcpRow = NULL;
	PMIB_UDPROW_OWNER_PID  udpRow = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR  num = 0;
	ULONG i = 0;
	ULONG ulLocalPort = 0;
	ULONG ulRemotePort = 0;
	ULONG ulPid = 0;
	ULONG nTotal = 0;
	
	if (!pPortInfos || nCanStore == 0 || nRetCnt == NULL)
	{
		return Status;
	}
	
	*nRetCnt = 0;

	Status = EnumTcpPortInformationWin7(&tcpRow, &num);
	if( NT_SUCCESS(Status) && num > 0 && tcpRow != NULL)
	{
		PMIB_TCPROW_OWNER_PID pTcpInfo = (PMIB_TCPROW_OWNER_PID)tcpRow;

		for( ; MmIsAddressValid(pTcpInfo + i) && i < (ULONG)num; i++ )
		{
			ulLocalPort = ntohs(pTcpInfo[i].dwLocalPort);
			ulRemotePort = ntohs(pTcpInfo[i].dwRemotePort);

			if (pTcpInfo[i].dwRemoteAddr == 0)
			{
				ulRemotePort = 0;
			}

			// 排除一些垃圾的东西吧
			if (pTcpInfo[i].dwState > 20 ||
				ulLocalPort > 65535 )
			{
				continue;
			}

			if (nCanStore > nTotal)
			{
				pPortInfos[nTotal].nPortType = enumTcp;
				pPortInfos[nTotal].nConnectState = pTcpInfo[i].dwState;
				pPortInfos[nTotal].nLocalAddress = pTcpInfo[i].dwLocalAddr;
				pPortInfos[nTotal].nLocalPort = ulLocalPort;
				pPortInfos[nTotal].nRemoteAddress = pTcpInfo[i].dwRemoteAddr;;
				pPortInfos[nTotal].nRemotePort = ulRemotePort;
				pPortInfos[nTotal].nPid = pTcpInfo[i].dwOwningPid;
			}

			KdPrint(("tcp-> Pid:%d, LocalPort:%d, RemotePort:%d\n", pTcpInfo[i].dwOwningPid, ulLocalPort, ulRemotePort));
			nTotal++;
		}
	}
	
	if (tcpRow)
	{
		ExFreePool(tcpRow);
		tcpRow = NULL;
	}

	//-------------------------------------------------------
	
	i = 0;
	Status = EnumUdpPortInformationWin7(&udpRow, &num);
	if(NT_SUCCESS(Status) && udpRow != NULL && (ULONG)num > 0)
	{
		PMIB_UDPROW_OWNER_PID pUdpInfo = (PMIB_UDPROW_OWNER_PID)udpRow;

		for( ; MmIsAddressValid(pUdpInfo + i) && i < (ULONG)num; i++ )
		{
			ulLocalPort = ntohs(pUdpInfo[i].dwLocalPort);
			ulPid = pUdpInfo[i].dwOwningPid;
		
			// 排除一些垃圾的东西吧
			if (ulPid <= 0 ||
				ulLocalPort > 65535)
			{
				continue;
			}

			KdPrint(("UDP-> Pid:%d, Port:%d\n", ulPid, ulLocalPort));

			if (nCanStore > nTotal)
			{
				pPortInfos[nTotal].nPortType = enumUdp;
				pPortInfos[nTotal].nConnectState = 1;
				pPortInfos[nTotal].nLocalAddress = pUdpInfo[i].dwLocalAddr;
				pPortInfos[nTotal].nLocalPort = ulLocalPort;
				pPortInfos[nTotal].nRemoteAddress = 0;
				pPortInfos[nTotal].nRemotePort = 0;
				pPortInfos[nTotal].nPid = ulPid;
			}

			nTotal++;
		}
	}
	
	if (udpRow)
	{
		ExFreePool(udpRow);
		udpRow = NULL;
	}

	*nRetCnt = nTotal;
	return Status; 
}

NTSTATUS EnumPort(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG nNtBuildNumber = GetGlobalVeriable(enumBuildNumber);
	PCOMMUNICATE_PORT pCpt = (PCOMMUNICATE_PORT)pOutBuffer;
	PPORT_INFO pPortInfos = pCpt->Ports;
	ULONG nCanStore = (uOutSize - sizeof(DWORD)) / sizeof(PORT_INFO);
	
	KdPrint(("EnumPort-> nCanStore: %d\n", nCanStore));

	__try
	{
		if (nNtBuildNumber != 2195)
		{
			if (nNtBuildNumber >= 6000)
			{
				PrintTcpIpInWin7(pPortInfos, nCanStore, &(pCpt->nCnt));
			}
			else
			{
				EnumXpPort(pPortInfos, nCanStore, &(pCpt->nCnt));
			}

			if (pCpt->nCnt <= nCanStore)
			{
				status = STATUS_SUCCESS;
			}
		}
	}
	__except(1)
	{
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}