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
#ifndef _REG_STRING_TOOL_H_
#define _REG_STRING_TOOL_H_

#include <set>
#include <vector>
using namespace std;

#define _EXPLORER_PROCESS_NAME_			L"explorer.exe"
#define  SHORT_FILE_NAME				L"X:\\X"
#define  RES_PREFIX_NAME				L"res://"
#define  SYSTEM_DRIVERS_PATH			L"system32\\DRIVERS\\"
#define  SYSTEM_ROOT_PATH				L"\\SystemRoot\\"
#define  SYSTEM_ROOT_PATH2				L"SystemRoot\\"  // added by minzhenfei
#define	 SYSTEM32DIR					L"System32\\"    // added by minzhenfei
#define  CONTROL_RUNDLL					L"Control_RunDLL"
#define  EXPEND_RUNDLL32_FULL_PATH		L"%SystemRoot%\\system32\\rundll32.exe"
#define	 RUNDLL32_SHORT_PATH			L"rundll32.exe"
#define	 REGSVR32_SHORT_PATH			L"regsvr32.exe"
#define	 EXPEND_REGSVR32_PATH			L"%SystemRoot%\\system32\\regsvr32.exe"
#define  EXPLORER_PATH_NO_DRIVE			L"\\windows\\explorer.exe"

#define  PIPE_FILE_NAME1				L"\\\\.\\pipe\\" 
#define  PIPE_FILE_NAME2				L"\"\\\\.\\pipe\\"
#define  DEVICE_PREFIX_NAME1			L"\\Device\\"
#define  DEVICE_PREFIX_NAME2			L"\\\\.\\"
#define  PHYSICAL_DRIVE_NAME			L"\\\\.\\PHYSICALDRIVE"

#define MAX_SCAN_NUMBER		64
#define SYSTEM_PROCESS_ID	8
#define HKLM_PREFIX_NAME    L"HKEY_LOCAL_MACHINE"
#define HKUSERS_PREFIX_NAME	L"HKEY_USERS"
#define SCAN_TIME_DELAY		(500UL)
#define SHORT_TIME_DELAY	(50UL)
#define LONG_TIME_DELAY		(500UL)
#define FomatClsidInProcServier32String L"CLSID\\%s\\InProcServer32"
#define SYSTEM32_DRIVER_PATH	L"system32\\drivers\\"
#define SYS_EXT					L".sys"

#define  MAX_BUFFER_SIZE			1024
#define  MAX_EXPLORER_COUNT			16

VOID ParseRegistryPathFromType( __in LPWSTR RegisterFileParameter, __in LPWSTR MulitFileParameter, __in DWORD Type, __inout set<wstring> &FileList);

#endif