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
#include "StdAfx.h"
#include "ProcessFunc.h"
#include "..\\..\\Common\Common.h"

#define OBJ_INHERIT             0x00000002L


CProcessFunc::CProcessFunc(void)
{

}

CProcessFunc::~CProcessFunc(void)
{

}

HANDLE CProcessFunc::OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
	HANDLE ProcessHandle = NULL;
	MZF_OBJECT_ATTRIBUTES ObjectAttributes;
	MZF_CLIENT_ID ClientId;

	ClientId.UniqueProcess = UlongToHandle(dwProcessId);
	ClientId.UniqueThread = 0;

	MzfInitializeObjectAttributes(&ObjectAttributes,
		NULL,
		(bInheritHandle ? OBJ_INHERIT : 0),
		NULL,
		NULL);

	COMMUNICATE_PROCES cp;
	cp.OpType = enumOpenProcess;
	cp.op.Open.ClientId = &ClientId;
	cp.op.Open.DesiredAccess = dwDesiredAccess;
	cp.op.Open.ObjectAttributes = (PMZF_OBJECT_ATTRIBUTES)&ObjectAttributes;

	BOOL bRet = m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), &ProcessHandle , sizeof(HANDLE), NULL);
	if (!bRet)
	{
		return NULL;
	}

	return ProcessHandle;
}

BOOL CProcessFunc::ReadProcessMemory(
					   HANDLE hProcess,
					   LPCVOID lpBaseAddress,
					   LPVOID lpBuffer,
					   SIZE_T nSize,
					   SIZE_T* lpNumberOfBytesRead
					   )
{
	COMMUNICATE_PROCES cp;
	cp.OpType = enumReadProcessMemory;
	cp.op.ReadMemory.hProcess = hProcess;
	cp.op.ReadMemory.lpBaseAddress = (PVOID)lpBaseAddress;
	cp.op.ReadMemory.lpBuffer = (PVOID)lpBuffer;
	cp.op.ReadMemory.nSize = nSize;
	
	return m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), lpNumberOfBytesRead , sizeof(SIZE_T), NULL);
}

BOOL CProcessFunc::WriteProcessMemory(
									 HANDLE hProcess,
									 LPCVOID lpBaseAddress,
									 LPVOID lpBuffer,
									 SIZE_T nSize,
									 SIZE_T* lpNumberOfBytesRead
									 )
{
	COMMUNICATE_PROCES cp;
	cp.OpType = enumWriteProcessMemory;
	cp.op.WriteMemory.hProcess = hProcess;
	cp.op.WriteMemory.lpBaseAddress = (PVOID)lpBaseAddress;
	cp.op.WriteMemory.lpBuffer = (PVOID)lpBuffer;
	cp.op.WriteMemory.nSize = nSize;

	return m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), lpNumberOfBytesRead , sizeof(SIZE_T), NULL);
}

BOOL CProcessFunc::ResumeProcess( HANDLE hProcess )
{
	COMMUNICATE_PROCES cp;
	cp.OpType = enumResumeProcess;
	cp.op.SuspendOrResume.hProcess = hProcess;

	return m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), NULL , 0, NULL);
}

BOOL CProcessFunc::SuspendProcess( HANDLE hProcess )
{
	COMMUNICATE_PROCES cp;
	cp.OpType = enumSuspendProcess;
	cp.op.SuspendOrResume.hProcess = hProcess;

	return m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), NULL , 0, NULL);
}

BOOL CProcessFunc::VirtualProtectEx( HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect )
{
	COMMUNICATE_PROCES cp;
	cp.OpType = enumVirtualProtectEx;
	cp.op.VirtualProtect.hProcess = hProcess;
	cp.op.VirtualProtect.lpAddress = lpAddress;
	cp.op.VirtualProtect.dwSize = dwSize;
	cp.op.VirtualProtect.flNewProtect = flNewProtect;

	return m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), lpflOldProtect , sizeof(DWORD), NULL);
}

BOOL CProcessFunc::QueryVirtualMemory(
						IN HANDLE ProcessHandle, 
						IN PVOID BaseAddress, 
						IN ULONG MemoryInformationClass, 
						OUT PVOID MemoryInformation, 
						IN ULONG MemoryInformationLength, 
						OUT PULONG ReturnLength OPTIONAL 
						)
{
	COMMUNICATE_PROCES cp;
	cp.OpType = enumQueryVirtualMemory;
	cp.op.QueryVirtualMemory.ProcessHandle = ProcessHandle;
	cp.op.QueryVirtualMemory.BaseAddress = BaseAddress;
	cp.op.QueryVirtualMemory.MemoryInformationClass = MemoryInformationClass;
	cp.op.QueryVirtualMemory.MemoryInformation = MemoryInformation;
	cp.op.QueryVirtualMemory.MemoryInformationLength = MemoryInformationLength;
	
	DWORD dwRet = 0;
	return m_Driver.CommunicateDriver(&cp, sizeof(COMMUNICATE_PROCES), &dwRet , sizeof(ULONG), NULL);
}