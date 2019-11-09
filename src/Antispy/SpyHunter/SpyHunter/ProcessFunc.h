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
#pragma once

#include "ConnectDriver.h"

class CProcessFunc
{
public:
	CProcessFunc(void);
	~CProcessFunc(void);

public:
	HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
	BOOL ReadProcessMemory( HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead );
	BOOL WriteProcessMemory( HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead );
	BOOL SuspendProcess( HANDLE hProcess );
	BOOL ResumeProcess( HANDLE hProcess );
	BOOL VirtualProtectEx( HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect );
	BOOL QueryVirtualMemory(
		IN HANDLE ProcessHandle, 
		IN PVOID BaseAddress, 
		IN ULONG MemoryInformationClass, 
		OUT PVOID MemoryInformation, 
		IN ULONG MemoryInformationLength, 
		OUT PULONG ReturnLength OPTIONAL 
		);

private:
	CConnectDriver m_Driver;
};
