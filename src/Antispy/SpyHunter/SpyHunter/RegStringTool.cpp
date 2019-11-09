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
#include "stdafx.h"
#include "RegStringTool.h"
#include "string_tool.h"
#include <shlwapi.h>
#include <iostream>
#include <fstream>
#include <Sddl.h>
#include <set>
#include <io.h>
#include <strsafe.h>
#include <UserEnv.h>
#include <vector>
#include <string>

#pragma  comment(lib,"Userenv.lib")
#pragma comment(lib, "shlwapi.lib")

//#define _EXPLORER_PROCESS_NAME_			L"explorer.exe"
//#define  RES_PREFIX_NAME				L"res://"
//#define  SYSTEM_DRIVERS_PATH			L"system32\\DRIVERS\\"
//#define  SYSTEM_ROOT_PATH				L"\\SystemRoot\\"
//#define  SYSTEM_ROOT_PATH2				L"SystemRoot\\"
//#define  CONTROL_RUNDLL					L"Control_RunDLL"
//#define  EXPEND_RUNDLL32_FULL_PATH		L"%SystemRoot%\\system32\\rundll32.exe"
//#define	 RUNDLL32_SHORT_PATH			L"rundll32.exe"
//#define	 REGSVR32_SHORT_PATH			L"regsvr32.exe"
//#define	 EXPEND_REGSVR32_PATH			L"%SystemRoot%\\system32\\regsvr32.exe"
//#define  EXPLORER_PATH_NO_DRIVE			L"\\windows\\explorer.exe"
//
//#define  PIPE_FILE_NAME1				L"\\\\.\\pipe\\" 
//#define  PIPE_FILE_NAME2				L"\"\\\\.\\pipe\\"
//#define  DEVICE_PREFIX_NAME1			L"\\Device\\"
//#define  DEVICE_PREFIX_NAME2			L"\\\\.\\"
//#define  PHYSICAL_DRIVE_NAME			L"\\\\.\\PHYSICALDRIVE"
//
//#define MAX_SCAN_NUMBER		64
//#define SYSTEM_PROCESS_ID	8
//#define HKLM_PREFIX_NAME    L"HKEY_LOCAL_MACHINE"
//#define HKUSERS_PREFIX_NAME	L"HKEY_USERS"
//#define SCAN_TIME_DELAY		(500UL)
//#define SHORT_TIME_DELAY	(50UL)
//#define LONG_TIME_DELAY		(500UL)
//#define FomatClsidInProcServier32String L"CLSID\\%s\\InProcServer32"
//#define SYSTEM32_DRIVER_PATH	L"system32\\drivers\\"
//#define SYS_EXT					L".sys"
//
//#define  MAX_BUFFER_SIZE			1024
//#define  MAX_EXPLORER_COUNT			16
//
//HANDLE	gExploerToken[MAX_EXPLORER_COUNT];
//WCHAR	gNtExplorerName[MAX_PATH/2];
//
//VOID
//ParseRegistryPath(__in LPWSTR RegisterParameter, 
//				  __in DWORD Type, 
//				  __inout std::set<std::wstring> &FileList);
//
//
//VOID LinkToWindowSystemPath(
//							__inout std::set<std::wstring> &FileList,
//							__in LPWSTR FileName)
//{
//
//	WCHAR FullPath[MAX_PATH];
//
//
//	if (*FileName == UNICODE_NULL){
//		return;
//	}
//	if( wcschr(FileName, L'\\') ) {
//		FileList.insert(_wcslwr(FileName)) ;
//		return;
//	}
//
//	RtlZeroMemory(FullPath, sizeof(FullPath));
//	GetWindowsDirectory(FullPath, MAX_PATH);
//
//	if (FullPath[wcslen(FullPath) -1] != L'\\'&& FileName[0] != L'\\' ){
//		FullPath[wcslen(FullPath)] = L'\\';
//	}
//
//	wcsncat_s(FullPath, MAX_PATH, FileName, _TRUNCATE);
//	FileList.insert(_wcslwr(FullPath));
//
//
//	RtlZeroMemory(FullPath, sizeof(FullPath));
//	GetSystemDirectory(FullPath, MAX_PATH);
//
//	if (FullPath[wcslen(FullPath) -1] != L'\\' && FileName[0] != L'\\'){
//		FullPath[wcslen(FullPath)] = L'\\';
//	}	
//
//	wcsncat_s(FullPath, MAX_PATH, FileName, _TRUNCATE);
//	FileList.insert(_wcslwr(FullPath));
//
//}
//
//BOOL IsSystemPath(__in LPWSTR Path, __inout std::set<std::wstring> &FileList)
//{
//	WCHAR FileName[MAX_PATH];
//
//	if( 0 == _wcsnicmp(Path, SYSTEM_DRIVERS_PATH, sizeof(SYSTEM_DRIVERS_PATH)/sizeof(WCHAR) - 1) )
//	{
//		RtlZeroMemory(FileName, sizeof(FileName));
//
//		GetWindowsDirectory(FileName, MAX_PATH);
//
//		// fixed by mzf
//		if (FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\' ){
//			FileName[wcslen(FileName)] = L'\\';
//		}
//
//		wcsncat_s(FileName, MAX_PATH, Path, _TRUNCATE);
//
//		FileList.insert(_wcslwr(FileName));
//
//		return TRUE;
//
//
//	}else if ( 0 == _wcsnicmp(Path, SYSTEM_ROOT_PATH, sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1) )
//	{
//
//		RtlZeroMemory(FileName, sizeof(FileName));
//
//		GetWindowsDirectory(FileName, MAX_PATH);
//
//		if ( FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\' ){
//			FileName[wcslen(FileName)] = L'\\';
//		}
//
//		//	Path += (sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1);
//
//		// fixed by mzf
//		if (UNICODE_NULL == Path[(sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1)]){
//			return FALSE;
//		}
//
//		wcsncat_s(FileName, MAX_PATH, Path + (sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1) , _TRUNCATE);
//
//		FileList.insert(_wcslwr(FileName));
//
//		return TRUE;
//	}
//	else if ( 0 == _wcsnicmp(Path, SYSTEM_ROOT_PATH2, sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1) )
//	{
//
//		RtlZeroMemory(FileName, sizeof(FileName));
//
//		GetWindowsDirectory(FileName, MAX_PATH);
//
//		if (FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\'){
//			FileName[wcslen(FileName)] = L'\\';
//		}
//		// 
//		// 		Path += (sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1);
//		// 
//		if (UNICODE_NULL == Path[(sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1)]){
//			return FALSE;
//		}
//
//		wcsncat_s(FileName, MAX_PATH, Path + (sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1) , _TRUNCATE);
//
//		FileList.insert(_wcslwr(FileName));
//
//		return TRUE;
//
//	}
//
//	return FALSE;
//}
//BOOL IsResPath(__in LPWSTR Path, __inout std::set<std::wstring> &FileList )
//{
//	LPWSTR FindFirt;
//	LPWSTR FindEnd;
//	WCHAR FileName[MAX_PATH];
//
//	if( 0 != _wcsnicmp(Path, RES_PREFIX_NAME, sizeof(RES_PREFIX_NAME)/sizeof(WCHAR) - 1) )
//	{
//		return FALSE;
//	}
//
//	FindFirt = (Path + sizeof(RES_PREFIX_NAME)/sizeof(WCHAR) - 1);
//
//	FindEnd = FindFirt;
//	while(UNICODE_NULL != *FindEnd)
//	{
//		if (*FindEnd == L'/'){
//			break;
//		}
//		FindEnd ++;
//	}
//
//	if (UNICODE_NULL == *FindEnd){
//		return FALSE;
//	}
//
//	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//
//	LinkToWindowSystemPath(FileList, FileName);
//
//	return TRUE;
//}
//
//BOOL IsRegsvr32Path(__in LPWSTR Path,  __inout std::set<std::wstring> &FileList)
//{
//	static WCHAR Regsvr32Path[MAX_PATH] = {0};
//	LPWSTR FindEnd;
//	LPWSTR FindFirt;
//	BOOL IsHaveQuotation = FALSE;
//	BOOL IsShortName;
//	WCHAR FileName[MAX_PATH];
//	BOOL FindControlDLL = FALSE;
//	ULONG Counter = 0;
//
//	if (UNICODE_NULL == *Regsvr32Path){
//		ExpandEnvironmentStrings(EXPEND_REGSVR32_PATH, Regsvr32Path, MAX_PATH);
//	}
//
//	FindFirt = Path;
//	if(L'"'== *Path)
//	{
//		IsHaveQuotation = TRUE;
//
//		FindFirt++;
//	}
//
//	if(0 == _wcsnicmp(FindFirt, REGSVR32_SHORT_PATH, wcslen(REGSVR32_SHORT_PATH)))
//	{
//		IsShortName = TRUE;
//	}
//	else if(0 == _wcsnicmp(FindFirt,  Regsvr32Path, wcslen(Regsvr32Path)) )
//	{
//		IsShortName = FALSE;
//
//	}else{
//
//		return FALSE;
//	}
//
//
//	if(FALSE == IsHaveQuotation)
//	{
//
//		FindFirt = Path;
//
//		FindEnd = wcschr(Path,L' ');
//		if (NULL == FindEnd)
//		{
//
//			LinkToWindowSystemPath(FileList, Path);
//
//			Counter ++;
//
//			return TRUE;
//		}
//
//		RtlZeroMemory(FileName,sizeof(FileName));
//		wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//		LinkToWindowSystemPath(FileList, FileName);
//
//		Counter ++;
//
//		FindFirt = FindEnd;
//
//		FindEnd = wcsstr(FindFirt, L"/i:");
//		if (NULL == FindEnd){
//			return  Counter != 0 ? TRUE : FALSE;
//		}
//
//		while(*FindEnd != L' '){
//			FindEnd++;
//		}
//		if(UNICODE_NULL == *FindEnd){
//			return Counter != 0 ? TRUE : FALSE;
//		}
//
//		while(*FindEnd == L' '){
//			FindEnd++;
//		}
//		if(UNICODE_NULL == *FindEnd){
//			return Counter != 0 ? TRUE : FALSE;
//		}
//
//		FindFirt = FindEnd;
//		if (L'"' == *FindFirt)
//		{
//			FindFirt++;
//			FindEnd = wcschr(FindFirt, L'"');
//			if (NULL == FindEnd){
//				return  Counter != 0 ? TRUE : FALSE;
//			}
//
//		}else{
//
//			FindEnd = FindFirt;
//			while( TRUE )
//			{
//				if(*FindEnd == L' '||
//					*FindEnd == L','||
//					*FindEnd == UNICODE_NULL){
//
//						FindControlDLL = TRUE;
//
//						break;
//				}
//
//				FindEnd++;
//			}		
//		}
//
//
//		RtlZeroMemory(FileName,sizeof(FileName));
//		wcsncpy_s(FileName,MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//		LinkToWindowSystemPath(FileList, FileName);
//		return  Counter != 0 ? TRUE : FALSE;
//	}
//
//	FindFirt = Path + 1;
//
//	FindEnd = wcschr(FindFirt, L'"');
//	if (NULL == FindEnd)
//	{
//		LinkToWindowSystemPath(FileList, Path + 1);
//
//		return TRUE;
//	}
//
//	RtlZeroMemory(FileName,sizeof(FileName));
//	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//	LinkToWindowSystemPath(FileList, FileName);
//
//	FindFirt = FindEnd;
//
//	FindEnd = wcsstr(FindFirt, L"/i:");
//	if (NULL == FindEnd){
//		return  Counter != 0 ? TRUE : FALSE;
//	}
//
//	while(*FindEnd != L' '){
//		FindEnd++;
//	}
//	if(UNICODE_NULL == *FindEnd){
//		return Counter != 0 ? TRUE : FALSE;
//	}
//
//	while(*FindEnd == L' '){
//		FindEnd++;
//	}
//	if(UNICODE_NULL == *FindEnd){
//		return Counter != 0 ? TRUE : FALSE;
//	}
//
//	FindFirt = FindEnd;
//
//	if (L'"' == *FindFirt)
//	{
//		FindFirt++;
//		FindEnd = wcschr(FindFirt, L'"');
//		if (NULL == FindEnd)
//		{
//			return TRUE;
//		}
//
//	}else{
//
//		FindEnd = FindFirt;
//		while( TRUE )
//		{
//			if(*FindEnd == L' '||
//				*FindEnd == L','||
//				*FindEnd == L'/'||
//				*FindEnd == UNICODE_NULL){
//
//					FindControlDLL = TRUE;
//
//					break;
//			}
//
//			FindEnd++;
//		}		
//	}
//
//	RtlZeroMemory(FileName,sizeof(FileName));
//	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//	LinkToWindowSystemPath(FileList, FileName);
//
//	return TRUE;
//}
//
//BOOL IsRundll32Path(__in LPWSTR Path,  __inout std::set<std::wstring> &FileList)
//{
//
//	LPWSTR FindEnd;
//	LPWSTR FindFirt;
//	static WCHAR Rundll32Path[MAX_PATH] = {0};
//	BOOL IsHaveQuotation = FALSE;
//	BOOL IsShortName;
//	WCHAR FileName[MAX_PATH];
//	BOOL FindControlDLL = FALSE;
//
//
//	if (UNICODE_NULL == *Rundll32Path)
//	{
//		ExpandEnvironmentStrings(EXPEND_RUNDLL32_FULL_PATH, Rundll32Path, MAX_PATH);
//	}
//
//
//	FindFirt = Path;
//	if(L'"'== *Path)
//	{
//		IsHaveQuotation = TRUE;
//
//		FindFirt++;
//	}
//
//	if(0 == _wcsnicmp(FindFirt, Rundll32Path, wcslen(Rundll32Path)))
//	{
//		IsShortName = FALSE;
//	}
//	else if(0 == _wcsnicmp(FindFirt,  RUNDLL32_SHORT_PATH, wcslen(RUNDLL32_SHORT_PATH)) )
//	{
//		IsShortName = TRUE;
//
//	}else{
//
//		return FALSE;
//	}
//
//	FindFirt = wcsstr(Path, CONTROL_RUNDLL );
//	if(NULL != FindFirt && (FindFirt = wcschr(FindFirt, L' ') )	)
//	{
//
//		while(*FindFirt == L' '){
//			FindFirt++;
//		}
//
//		FindEnd = FindFirt;
//		if (L'"' == *FindFirt)
//		{
//			FindFirt++;
//			FindEnd = wcschr(FindFirt, L'"');
//			if (NULL != FindEnd)
//			{
//				FindControlDLL = TRUE;
//			}
//
//		}else{
//
//			FindEnd = FindFirt;
//			while( TRUE )
//			{
//				if(*FindEnd == L' '||
//					*FindEnd == L','||
//					*FindEnd == UNICODE_NULL){
//
//						FindControlDLL = TRUE;
//
//						break;
//				}
//
//				FindEnd++;
//			}		
//		}	
//	}
//
//	if(TRUE == FindControlDLL) {
//
//		RtlZeroMemory(FileName,sizeof(FileName));
//		wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//		LinkToWindowSystemPath(FileList, FileName);
//	}
//
//	if(FALSE == IsHaveQuotation)
//	{
//
//		FindFirt = Path;
//
//		FindEnd = wcschr(Path,L' ');
//		if (NULL == FindEnd)
//		{
//			LinkToWindowSystemPath(FileList, Path);
//
//			return TRUE;
//		}
//
//		RtlZeroMemory(FileName,sizeof(FileName));
//		wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//		LinkToWindowSystemPath(FileList, FileName);
//
//
//		while(*FindEnd == L' '){
//			FindEnd++;
//		}
//		if(UNICODE_NULL == *FindEnd){
//			return FALSE;
//		}
//
//		FindFirt = FindEnd ;
//
//		if (L'"' == *FindFirt)
//		{
//			FindFirt++;
//			FindEnd = wcschr(FindFirt, L'"');
//			if (NULL == FindEnd)
//			{
//				return TRUE;
//			}
//
//		}else{
//
//			FindEnd = FindFirt;
//			while( TRUE )
//			{
//				if(*FindEnd == L','||
//					*FindEnd == UNICODE_NULL){
//
//						FindControlDLL = TRUE;
//
//						break;;
//				}
//
//				FindEnd++;
//			}
//
//		}
//
//
//		RtlZeroMemory(FileName,sizeof(FileName));
//		wcsncpy_s(FileName,MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//		LinkToWindowSystemPath(FileList, FileName);
//		return TRUE;
//	}
//
//	FindFirt = Path + 1;
//
//	FindEnd = wcschr(FindFirt, L'"');
//	if (NULL == FindEnd)
//	{
//		LinkToWindowSystemPath(FileList, Path + 1);
//
//		return TRUE;
//	}
//
//	RtlZeroMemory(FileName,sizeof(FileName));
//	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//	LinkToWindowSystemPath(FileList, FileName);
//
//	FindEnd = wcschr(FindFirt, L' ');
//	if(NULL == FindEnd)
//	{
//		return TRUE;
//	}
//
//	while(*FindEnd == L' ')
//	{
//		FindEnd++;
//	}
//	if(UNICODE_NULL == *FindEnd){
//		return TRUE;
//	}
//
//	FindFirt = FindEnd;
//	if (L'"' == *FindFirt)
//	{
//		FindFirt++;
//		FindEnd = wcschr(FindFirt, L'"');
//		if (NULL == FindEnd)
//		{
//			return TRUE;
//		}
//
//	}else{
//
//		FindEnd = FindFirt;
//		while( TRUE )
//		{
//			if(*FindEnd == L','||
//				*FindEnd == UNICODE_NULL){
//
//					FindControlDLL = TRUE;
//
//					break;;
//			}
//
//			FindEnd++;
//		}
//	}
//
//	RtlZeroMemory(FileName,sizeof(FileName));
//	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//	LinkToWindowSystemPath(FileList, FileName);
//
//	return TRUE;
//}
//
//BOOL
//IsDoubleQuotationFile(__in LPWSTR Parameter,  __out std::set<std::wstring> &FileList )
//{
//
//
//	LPWSTR PathEntry;
//	LPWSTR FindFirt;
//	LPWSTR FindEnd;
//	WCHAR FileName[MAX_PATH];
//	BOOL Counter = 0;
//
//	PathEntry = Parameter;
//
//
//	while( TRUE )
//	{
//
//		FindFirt = wcschr(PathEntry, L'"');
//		if(NULL == FindFirt)
//		{
//			return Counter != 0 ? TRUE : FALSE;
//		}
//
//		FindFirt++;
//
//		FindEnd = wcschr(FindFirt, L'"');
//		if(NULL == FindEnd)
//		{
//			return Counter != 0 ? TRUE : FALSE;
//		}
//
//		if((FindEnd - FindFirt) >= (sizeof(SHORT_FILE_NAME)/sizeof(WCHAR) -1) ) {
//
//			RtlZeroMemory(FileName, sizeof(FileName));
//			wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//			FileList.insert(_wcslwr(FileName));
//
//			Counter++;
//		}
//
//		FindEnd++;
//
//		PathEntry = FindEnd;
//	}
//
//	return TRUE;
//}
//
//BOOL IsCommaFile(__in LPWSTR Parameter, __inout std::set<std::wstring> &FileList)
//{
//	LPWSTR FindFirt;
//	LPWSTR FindEnd;
//	WCHAR FileName[MAX_PATH];
//	LPWSTR FileEntry;
//
//	FindFirt = FileEntry = Parameter;
//
//	while( TRUE )
//	{
//
//		if (*FileEntry == L',' ||
//			*FileEntry == UNICODE_NULL) {
//
//				FindEnd = FileEntry;
//
//				RtlZeroMemory(FileName, sizeof(FileName));
//				wcsncpy_s(FileName, MAX_PATH, 
//					FindFirt, 
//					(FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//				LinkToWindowSystemPath(FileList, FileName);
//
//				if (*FileEntry == UNICODE_NULL) {
//					break;
//			 }
//
//				FindFirt = FileEntry + 1;
//
//		}
//
//		FileEntry++;
//	}
//
//	return TRUE;
//}
//
//BOOL IsAbsolutePathFile(__in LPWSTR Parameter, __inout std::set<std::wstring> &FileList)
//{
//	LPWSTR PathEntry = Parameter;
//	LPWSTR FindFirt;
//	LPWSTR FindEnd;
//	PWCHAR pDotPostionW;
//	WCHAR FileName[MAX_PATH];
//	DWORD ParameterLength = (DWORD)wcslen(Parameter);
//
//	PathEntry = Parameter;
//
//	if (ParameterLength < sizeof(SHORT_FILE_NAME) - sizeof(WCHAR) ||
//		PathEntry[1] != L':')
//	{
//		return FALSE;
//	}
//
//	FindFirt = PathEntry;
//
//	pDotPostionW = wcschr(PathEntry, L'.');
//	if (NULL == pDotPostionW){
//
//		//
//		// 逗号
//		//
//		FileList.insert(_wcslwr(PathEntry));
//		return FALSE;
//	}
//
//	PathEntry = FindEnd = pDotPostionW;
//
//	while( TRUE )
//	{
//		if (*PathEntry == L','   ||
//			*PathEntry == L' ' ||
//			*PathEntry == L'-'||
//			*PathEntry == L'/' ||
//			*PathEntry == UNICODE_NULL 
//			){
//
//				FindEnd = PathEntry;
//
//				break;
//		}
//
//		PathEntry++;
//	}
//
//	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
//
//	FileList.insert(_wcslwr(FileName));
//
//	return TRUE;
//}
//
//BOOL IsIgnoreName(__in LPWSTR Name)
//{
//	if (UNICODE_NULL == *Name){
//		return TRUE;
//	}
//
//	if (0 == _wcsnicmp(Name, DEVICE_PREFIX_NAME1, sizeof(DEVICE_PREFIX_NAME1)/sizeof(WCHAR) -1) ){
//		return TRUE;
//	}
//
//	if (L'"' == *Name){
//		Name++;
//	}
//
//	if (0 == _wcsnicmp(Name, DEVICE_PREFIX_NAME2 , sizeof(DEVICE_PREFIX_NAME2)/sizeof(WCHAR) - 1)) {
//
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//VOID
//ParseRegistryPathFromType( __in LPWSTR RegisterFileParameter,
//						  __in LPWSTR MulitFileParameter,
//						  __in DWORD Type, 
//						  __inout set<wstring> &FileList)
//{
//	DWORD BufferLength;
//	BOOL  Expand = FALSE;
//	WCHAR ExpandParameter[MAX_PATH*2];
//	LPWSTR FileParameter;
//	BOOL MallocExpandBuffer = FALSE;
//
//	if (REG_SZ == Type)
//	{
//		ParseRegistryPath(RegisterFileParameter, REG_SZ, FileList);
//		return;
//	}
//
//	if (REG_EXPAND_SZ == Type)
//	{
//		//
//		//   当前服务为Session 0
//		//
//		FileParameter =  ExpandParameter;
//		BufferLength = ExpandEnvironmentStrings(RegisterFileParameter, FileParameter, sizeof(ExpandParameter)/sizeof(WCHAR));
//		if (BufferLength > sizeof(ExpandParameter)/sizeof(WCHAR))
//		{
//			FileParameter = (LPWSTR)malloc(BufferLength * sizeof(WCHAR) + UNICODE_NULL);
//			if(NULL == FileParameter)
//			{
//				return;
//			}
//
//			ExpandEnvironmentStrings(RegisterFileParameter, FileParameter, BufferLength + UNICODE_NULL);
//			MallocExpandBuffer = TRUE;
//		}
//
//		ParseRegistryPath(FileParameter, REG_SZ, FileList);
//
//		if (MallocExpandBuffer){
//			free(FileParameter);
//			MallocExpandBuffer = FALSE;
//		}
//
//		return;
//	}
//
//	if (REG_MULTI_SZ == Type)
//	{
//		FileParameter = MulitFileParameter;
//
//		if (NULL == FileParameter)
//		{
//			return;
//		}
//
//		while( UNICODE_NULL != *FileParameter )
//		{
//			ParseRegistryPath(FileParameter, REG_SZ, FileList);
//			FileParameter += (ULONG)wcslen(FileParameter) + 1;
//		}
//
//		return;
//	}
//}
//
//VOID
//ParseRegistryPath(__in LPWSTR RegisterParameter, 
//				  __in DWORD Type, 
//				  __inout std::set<std::wstring> &FileList)
//{
//
//	BOOL  Expand = FALSE;
//	WCHAR ExpandParameter[MAX_PATH];
//	LPWSTR PathEntry = NULL;
//	DWORD BufferLength;
//	LPWSTR Parameter;
//	DWORD ParameterLength;
//	std::wstring FileEntry;
//	BOOL MallocExpandBuffer = FALSE;
//	std::vector<MY_WSTRING> vecScanFilePath;
//	std::vector<MY_WSTRING>::const_iterator itScanFilePath;
//
//	if (NULL == RegisterParameter)
//	{
//		return;
//	}
//
//	if (REG_EXPAND_SZ != Type &&
//		REG_SZ != Type &&
//		REG_MULTI_SZ != Type)
//	{
//			return;
//	}
//
//	ParameterLength = (DWORD)wcslen(RegisterParameter);
//
//	if (0 == ParameterLength){
//		return;
//	}
//
//	while( UNICODE_NULL != *RegisterParameter )
//	{
//		if (*RegisterParameter != L' ' &&
//			*RegisterParameter != L'@' &&
//			*RegisterParameter != L'!' &&
//			*RegisterParameter != L'\\' &&
//			*RegisterParameter != L'?' ){
//
//				break;
//		}
//
//		RegisterParameter++;
//		ParameterLength --;
//	}
//
//	if (0 == ParameterLength){
//		return;
//	}
//
//	Parameter = RegisterParameter;
//	if(REG_EXPAND_SZ == Type)
//	{
//		Expand = TRUE;
//
//		BufferLength = ExpandEnvironmentStrings(RegisterParameter, ExpandParameter, sizeof(ExpandParameter)/sizeof(WCHAR));
//		if (BufferLength > sizeof(ExpandParameter)/sizeof(WCHAR)){
//
//			Parameter = (LPWSTR)malloc(BufferLength * sizeof(WCHAR) + UNICODE_NULL);
//			if(NULL == Parameter){
//				return;
//			}
//
//			ExpandEnvironmentStrings(RegisterParameter, Parameter, BufferLength + UNICODE_NULL);
//			MallocExpandBuffer = TRUE;
//
//		}else{
//
//			Parameter = ExpandParameter;
//		}
//	}
//
//
//	if ( IsIgnoreName(RegisterParameter) ){
//
//		goto __exit;
//	}
//
//	if ( IsSystemPath(Parameter, FileList) ){
//		goto __exit;
//	}
//
//	if( IsResPath(Parameter, FileList) ) {
//		goto __exit;
//	}
//
//	if( IsRundll32Path(Parameter, FileList) ){
//		goto __exit;
//	}
//
//	if ( IsRegsvr32Path(Parameter, FileList) ){
//		goto __exit;
//	}
//
//	if( IsDoubleQuotationFile(Parameter, FileList) ){
//		goto __exit;
//	}
//
//	if ( IsAbsolutePathFile(Parameter, FileList) ){
//		goto __exit;
//	}
//
//	if ( IsCommaFile(Parameter, FileList) ){
//		goto __exit;
//	}
//
//__exit:
//
//	GetFileFullPath(Parameter, NULL, vecScanFilePath);
//	itScanFilePath = vecScanFilePath.begin();
//	for (; itScanFilePath != vecScanFilePath.end(); itScanFilePath++)
//	{
//		if (wcslen(itScanFilePath->string) > sizeof(SHORT_FILE_NAME)/sizeof(WCHAR) - 1){
//
//			//	FileList.insert(itScanFilePath->string);
//
//			// fixed by minzhenfei
//			wchar_t strTemp[MAX_PATH*2] = {0};
//
//			wcscpy(strTemp, itScanFilePath->string);
//			FileList.insert(_wcslwr(strTemp));
//		}
//
//	}
//
//	if (MallocExpandBuffer){
//		free(Parameter);
//	}
//}

VOID
ParseRegistryPath(__in LPWSTR RegisterParameter, 
				  __in DWORD Type, 
				  __inout std::set<std::wstring> &FileList);

BOOL ParseShortNameItem(WCHAR* strFilePath, WCHAR* strLongPath)
{

	if (strFilePath == NULL || 
		strLongPath == NULL ||
		wcslen(strFilePath)<=wcslen(SHORT_FILE_NAME) ||
		PathIsDirectory(strFilePath))
	{
		return FALSE;
	}

	if ( wcschr(strFilePath, L'~') )
	{
		if ( MAX_PATH >= GetLongPathName(strFilePath, strLongPath, MAX_PATH) && 
			wcslen(strLongPath) > wcslen(strFilePath))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void JointExt(int spritlen, LPWSTR FileName, __inout std::set<std::wstring> &FileList)
{
	WCHAR filePath[MAX_PATH] = {0};
	WCHAR *extName[] = {L".exe", L".dll", L".sys"};

	for (int i=0; i<sizeof(extName)/sizeof(extName[0]); i++)
	{
		memset(filePath, 0, MAX_PATH);
		wcscpy_s(filePath, MAX_PATH, FileName);
		wcscat_s(filePath, MAX_PATH, extName[i]);

		if (!PathFileExists(filePath))
		{
			continue;
		}
		else
		{
			WCHAR strLongPath[MAX_PATH] = {0};
			if (ParseShortNameItem(filePath, strLongPath))
			{
				_wcslwr_s(strLongPath, MAX_PATH);
				FileList.insert(strLongPath);
			}
			else
			{
				_wcslwr_s(filePath, MAX_PATH);
				FileList.insert(filePath);
			}	
		}
	}

	int spacelen = (int)(wcsrchr(FileName, L' ') - FileName);
	if (spacelen > spritlen)
	{
		memset(filePath, 0, MAX_PATH);
		wcsncpy_s(filePath, MAX_PATH, FileName, spacelen);

		JointExt(spritlen, filePath, FileList);
	}
}

void ParseDataToFullPath(WCHAR* strFilePath, __inout std::set<std::wstring> &FileList)
{
	int spritlen = 0, dotlen = 0;

	if (strFilePath == NULL || 
		wcslen(strFilePath)<=wcslen(SHORT_FILE_NAME) ||
		PathIsDirectory(strFilePath))
	{
		return;
	}

	spritlen = (int)(wcsrchr(strFilePath, L'\\') - strFilePath);
	dotlen = (int) (wcsrchr(strFilePath, L'.') - strFilePath);

	if (spritlen > dotlen)
	{
		if (!PathIsDirectory(strFilePath))
		{
			JointExt(spritlen, strFilePath, FileList);
		}
	}
	else
	{
		WCHAR strLongPath[MAX_PATH] = {0};
		if (ParseShortNameItem(strFilePath, strLongPath))
		{
			_wcslwr_s(strLongPath, MAX_PATH);
			FileList.insert(strLongPath);
		}
		else
		{
			_wcslwr_s(strFilePath, MAX_PATH);
			FileList.insert(strFilePath);
		}	
	}
}

VOID LinkToWindowSystemPath(
							__inout std::set<std::wstring> &FileList,
							__in LPWSTR FileName)
{
	WCHAR FullPath[MAX_PATH];
	int spritlen = 0, dotlen = 0;
	WCHAR *temp = NULL;

	if (*FileName == UNICODE_NULL){
		return;
	}

	if( wcschr(FileName, L'\\') ) {

		WCHAR strTemp[MAX_PATH] = {0};
		wcscpy_s(strTemp, MAX_PATH, FileName);
		_wcslwr_s(strTemp, MAX_PATH);

		ParseDataToFullPath(strTemp, FileList);

		return;
	}

	RtlZeroMemory(FullPath, sizeof(FullPath));
	GetWindowsDirectory(FullPath, MAX_PATH);

	if (FullPath[wcslen(FullPath) -1] != L'\\'&& FileName[0] != L'\\' ){
		FullPath[wcslen(FullPath)] = L'\\';
	}

	wcsncat_s(FullPath, MAX_PATH, FileName, _TRUNCATE);
	_wcslwr_s(FullPath, MAX_PATH);

	ParseDataToFullPath(FullPath, FileList);

	RtlZeroMemory(FullPath, sizeof(FullPath));
	GetSystemDirectory(FullPath, MAX_PATH);

	if (FullPath[wcslen(FullPath) -1] != L'\\' && FileName[0] != L'\\'){
		FullPath[wcslen(FullPath)] = L'\\';
	}	

	wcsncat_s(FullPath, MAX_PATH, FileName, _TRUNCATE);
	_wcslwr_s(FullPath, MAX_PATH);

	ParseDataToFullPath(FullPath, FileList);
}

BOOL IsSystemPath(__in LPWSTR Path, __inout std::set<std::wstring> &FileList)
{
	WCHAR FileName[MAX_PATH];

	if( 0 == _wcsnicmp(Path, SYSTEM_DRIVERS_PATH, sizeof(SYSTEM_DRIVERS_PATH)/sizeof(WCHAR) - 1) )
	{
		RtlZeroMemory(FileName, sizeof(FileName));

		GetWindowsDirectory(FileName, MAX_PATH);

		// fixed by mzf
		if (FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\' ){
			FileName[wcslen(FileName)] = L'\\';
		}

		wcsncat_s(FileName, MAX_PATH, Path, _TRUNCATE);
		_wcslwr_s(FileName, MAX_PATH);

		ParseDataToFullPath(FileName, FileList);

		return TRUE;
	}

	else if ( 0 == _wcsnicmp(Path, SYSTEM_ROOT_PATH, sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1) )
	{
		RtlZeroMemory(FileName, sizeof(FileName));

		GetWindowsDirectory(FileName, MAX_PATH);

		if ( FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\' ){
			FileName[wcslen(FileName)] = L'\\';
		}

		//	Path += (sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1);

		// fixed by mzf
		if (UNICODE_NULL == Path[(sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1)]){
			return FALSE;
		}

		wcsncat_s(FileName, MAX_PATH, Path + (sizeof(SYSTEM_ROOT_PATH)/sizeof(WCHAR) - 1) , _TRUNCATE);
		_wcslwr_s(FileName, MAX_PATH);

		ParseDataToFullPath(FileName, FileList);

		return TRUE;
	}

	else if ( 0 == _wcsnicmp(Path, SYSTEM_ROOT_PATH2, sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1) )
	{

		RtlZeroMemory(FileName, sizeof(FileName));

		GetWindowsDirectory(FileName, MAX_PATH);

		if (FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\'){
			FileName[wcslen(FileName)] = L'\\';
		}

		// 
		// 		Path += (sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1);
		// 

		if (UNICODE_NULL == Path[(sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1)]){
			return FALSE;
		}

		wcsncat_s(FileName, MAX_PATH, Path + (sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1) , _TRUNCATE);
		_wcslwr_s(FileName, MAX_PATH);

		ParseDataToFullPath(FileName, FileList);

		return TRUE;
	}

	// added by minzhenfei  
	// 解析 system32\clfs.sys
	else if ( 0 == _wcsnicmp(Path, SYSTEM32DIR, sizeof(SYSTEM32DIR)/sizeof(WCHAR)-1))
	{
		RtlZeroMemory(FileName, sizeof(FileName));

		GetWindowsDirectory(FileName, MAX_PATH);

		if (FileName[wcslen(FileName) - 1] != L'\\' && Path[0] != L'\\'){
			FileName[wcslen(FileName)] = L'\\';
		}

		if (UNICODE_NULL == Path[(sizeof(SYSTEM_ROOT_PATH2)/sizeof(WCHAR) - 1)]){
			return FALSE;
		}

		wcsncat_s(FileName, MAX_PATH, Path, _TRUNCATE);
		_wcslwr_s(FileName, MAX_PATH);

		ParseDataToFullPath(FileName, FileList);

		return TRUE;
	}

	return FALSE;
}


BOOL IsResPath(__in LPWSTR Path, __inout std::set<std::wstring> &FileList )
{
	LPWSTR FindFirt;
	LPWSTR FindEnd;
	WCHAR FileName[MAX_PATH];

	if( 0 != _wcsnicmp(Path, RES_PREFIX_NAME, sizeof(RES_PREFIX_NAME)/sizeof(WCHAR) - 1) )
	{
		return FALSE;
	}

	FindFirt = (Path + sizeof(RES_PREFIX_NAME)/sizeof(WCHAR) - 1);

	FindEnd = FindFirt;
	while(UNICODE_NULL != *FindEnd)
	{
		if (*FindEnd == L'/'){
			break;
		}
		FindEnd ++;
	}

	if (UNICODE_NULL == *FindEnd){
		return FALSE;
	}

	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));

	LinkToWindowSystemPath(FileList, FileName);

	return TRUE;
}

BOOL IsRegsvr32Path(__in LPWSTR Path,  __inout std::set<std::wstring> &FileList)
{
	static WCHAR Regsvr32Path[MAX_PATH] = {0};
	LPWSTR FindEnd;
	LPWSTR FindFirt;
	BOOL IsHaveQuotation = FALSE;
	BOOL IsShortName;
	WCHAR FileName[MAX_PATH];
	BOOL FindControlDLL = FALSE;
	ULONG Counter = 0;

	if (UNICODE_NULL == *Regsvr32Path){
		ExpandEnvironmentStrings(EXPEND_REGSVR32_PATH, Regsvr32Path, MAX_PATH);
	}

	FindFirt = Path;
	if(L'"'== *Path)
	{
		IsHaveQuotation = TRUE;

		FindFirt++;
	}

	if(0 == _wcsnicmp(FindFirt, REGSVR32_SHORT_PATH, wcslen(REGSVR32_SHORT_PATH)))
	{
		IsShortName = TRUE;
	}
	else if(0 == _wcsnicmp(FindFirt,  Regsvr32Path, wcslen(Regsvr32Path)) )
	{
		IsShortName = FALSE;

	}else{

		return FALSE;
	}


	if(FALSE == IsHaveQuotation)
	{

		FindFirt = Path;

		FindEnd = wcschr(Path,L' ');
		if (NULL == FindEnd)
		{

			LinkToWindowSystemPath(FileList, Path);

			Counter ++;

			return TRUE;
		}

		RtlZeroMemory(FileName,sizeof(FileName));
		wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
		LinkToWindowSystemPath(FileList, FileName);

		Counter ++;

		FindFirt = FindEnd;

		FindEnd = wcsstr(FindFirt, L"/i:");
		if (NULL == FindEnd){
			return  Counter != 0 ? TRUE : FALSE;
		}

		while(*FindEnd != L' '){
			FindEnd++;
		}
		if(UNICODE_NULL == *FindEnd){
			return Counter != 0 ? TRUE : FALSE;
		}

		while(*FindEnd == L' '){
			FindEnd++;
		}
		if(UNICODE_NULL == *FindEnd){
			return Counter != 0 ? TRUE : FALSE;
		}

		FindFirt = FindEnd;
		if (L'"' == *FindFirt)
		{
			FindFirt++;
			FindEnd = wcschr(FindFirt, L'"');
			if (NULL == FindEnd){
				return  Counter != 0 ? TRUE : FALSE;
			}

		}else{

			FindEnd = FindFirt;
			while( TRUE )
			{
				if(*FindEnd == L' '||
					*FindEnd == L','||
					*FindEnd == UNICODE_NULL){

						FindControlDLL = TRUE;

						break;
				}

				FindEnd++;
			}		
		}


		RtlZeroMemory(FileName,sizeof(FileName));
		wcsncpy_s(FileName,MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
		LinkToWindowSystemPath(FileList, FileName);
		return  Counter != 0 ? TRUE : FALSE;
	}

	FindFirt = Path + 1;

	FindEnd = wcschr(FindFirt, L'"');
	if (NULL == FindEnd)
	{
		LinkToWindowSystemPath(FileList, Path + 1);

		return TRUE;
	}

	RtlZeroMemory(FileName,sizeof(FileName));
	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
	LinkToWindowSystemPath(FileList, FileName);

	FindFirt = FindEnd;

	FindEnd = wcsstr(FindFirt, L"/i:");
	if (NULL == FindEnd){
		return  Counter != 0 ? TRUE : FALSE;
	}

	while(*FindEnd != L' '){
		FindEnd++;
	}
	if(UNICODE_NULL == *FindEnd){
		return Counter != 0 ? TRUE : FALSE;
	}

	while(*FindEnd == L' '){
		FindEnd++;
	}
	if(UNICODE_NULL == *FindEnd){
		return Counter != 0 ? TRUE : FALSE;
	}

	FindFirt = FindEnd;

	if (L'"' == *FindFirt)
	{
		FindFirt++;
		FindEnd = wcschr(FindFirt, L'"');
		if (NULL == FindEnd)
		{
			return TRUE;
		}

	}else{

		FindEnd = FindFirt;
		while( TRUE )
		{
			if(*FindEnd == L' '||
				*FindEnd == L','||
				*FindEnd == L'/'||
				*FindEnd == UNICODE_NULL){

					FindControlDLL = TRUE;

					break;
			}

			FindEnd++;
		}		
	}

	RtlZeroMemory(FileName,sizeof(FileName));
	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
	LinkToWindowSystemPath(FileList, FileName);

	return TRUE;
}

BOOL IsRundll32Path(__in LPWSTR Path,  __inout std::set<std::wstring> &FileList)
{

	LPWSTR FindEnd;
	LPWSTR FindFirt;
	static WCHAR Rundll32Path[MAX_PATH] = {0};
	BOOL IsHaveQuotation = FALSE;
	BOOL IsShortName;
	WCHAR FileName[MAX_PATH];
	BOOL FindControlDLL = FALSE;


	if (UNICODE_NULL == *Rundll32Path)
	{
		ExpandEnvironmentStrings(EXPEND_RUNDLL32_FULL_PATH, Rundll32Path, MAX_PATH);
	}


	FindFirt = Path;
	if(L'"'== *Path)
	{
		IsHaveQuotation = TRUE;

		FindFirt++;
	}

	if(0 == _wcsnicmp(FindFirt, Rundll32Path, wcslen(Rundll32Path)))
	{
		IsShortName = FALSE;
	}
	else if(0 == _wcsnicmp(FindFirt,  RUNDLL32_SHORT_PATH, wcslen(RUNDLL32_SHORT_PATH)) )
	{
		IsShortName = TRUE;

	}else{

		return FALSE;
	}

	FindFirt = wcsstr(Path, CONTROL_RUNDLL );
	if(NULL != FindFirt && (FindFirt = wcschr(FindFirt, L' ') )	)
	{

		while(*FindFirt == L' '){
			FindFirt++;
		}

		FindEnd = FindFirt;
		if (L'"' == *FindFirt)
		{
			FindFirt++;
			FindEnd = wcschr(FindFirt, L'"');
			if (NULL != FindEnd)
			{
				FindControlDLL = TRUE;
			}

		}else{

			FindEnd = FindFirt;
			while( TRUE )
			{
				if(*FindEnd == L' '||
					*FindEnd == L','||
					*FindEnd == UNICODE_NULL){

						FindControlDLL = TRUE;

						break;
				}

				FindEnd++;
			}		
		}	
	}

	if(TRUE == FindControlDLL) {

		RtlZeroMemory(FileName,sizeof(FileName));
		wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
		LinkToWindowSystemPath(FileList, FileName);
	}

	if(FALSE == IsHaveQuotation)
	{

		FindFirt = Path;

		FindEnd = wcschr(Path,L' ');
		if (NULL == FindEnd)
		{
			LinkToWindowSystemPath(FileList, Path);

			return TRUE;
		}

		RtlZeroMemory(FileName,sizeof(FileName));
		wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
		LinkToWindowSystemPath(FileList, FileName);


		while(*FindEnd == L' '){
			FindEnd++;
		}
		if(UNICODE_NULL == *FindEnd){
			return FALSE;
		}

		FindFirt = FindEnd ;

		if (L'"' == *FindFirt)
		{
			FindFirt++;
			FindEnd = wcschr(FindFirt, L'"');
			if (NULL == FindEnd)
			{
				return TRUE;
			}

		}else{

			FindEnd = FindFirt;
			while( TRUE )
			{
				if(*FindEnd == L','||
					*FindEnd == UNICODE_NULL){

						FindControlDLL = TRUE;

						break;;
				}

				FindEnd++;
			}

		}


		RtlZeroMemory(FileName,sizeof(FileName));
		wcsncpy_s(FileName,MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
		LinkToWindowSystemPath(FileList, FileName);
		return TRUE;
	}

	FindFirt = Path + 1;

	FindEnd = wcschr(FindFirt, L'"');
	if (NULL == FindEnd)
	{
		LinkToWindowSystemPath(FileList, Path + 1);

		return TRUE;
	}

	RtlZeroMemory(FileName,sizeof(FileName));
	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
	LinkToWindowSystemPath(FileList, FileName);

	FindEnd = wcschr(FindFirt, L' ');
	if(NULL == FindEnd)
	{
		return TRUE;
	}

	while(*FindEnd == L' ')
	{
		FindEnd++;
	}
	if(UNICODE_NULL == *FindEnd){
		return TRUE;
	}

	FindFirt = FindEnd;
	if (L'"' == *FindFirt)
	{
		FindFirt++;
		FindEnd = wcschr(FindFirt, L'"');
		if (NULL == FindEnd)
		{
			return TRUE;
		}

	}else{

		FindEnd = FindFirt;
		while( TRUE )
		{
			if(*FindEnd == L','||
				*FindEnd == UNICODE_NULL){

					FindControlDLL = TRUE;

					break;;
			}

			FindEnd++;
		}
	}

	RtlZeroMemory(FileName,sizeof(FileName));
	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
	LinkToWindowSystemPath(FileList, FileName);

	return TRUE;
}

BOOL
IsDoubleQuotationFile(__in LPWSTR Parameter,  __out std::set<std::wstring> &FileList )
{


	LPWSTR PathEntry;
	LPWSTR FindFirt;
	LPWSTR FindEnd;
	WCHAR FileName[MAX_PATH];
	BOOL Counter = 0;

	PathEntry = Parameter;


	while( TRUE )
	{

		FindFirt = wcschr(PathEntry, L'"');
		if(NULL == FindFirt)
		{
			return Counter != 0 ? TRUE : FALSE;
		}

		FindFirt++;

		FindEnd = wcschr(FindFirt, L'"');
		if(NULL == FindEnd)
		{
			return Counter != 0 ? TRUE : FALSE;
		}

		if((FindEnd - FindFirt) >= (sizeof(SHORT_FILE_NAME)/sizeof(WCHAR) -1) ) {

			RtlZeroMemory(FileName, sizeof(FileName));
			wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
			_wcslwr_s(FileName, MAX_PATH);

			ParseDataToFullPath(FileName, FileList);

			Counter++;
		}

		FindEnd++;

		PathEntry = FindEnd;
	}

	return TRUE;
}

BOOL IsCommaFile(__in LPWSTR Parameter, __inout std::set<std::wstring> &FileList)
{
	LPWSTR FindFirt;
	LPWSTR FindEnd;
	WCHAR FileName[MAX_PATH];
	LPWSTR FileEntry;

	FindFirt = FileEntry = Parameter;

	while( TRUE )
	{

		if (*FileEntry == L',' ||
			*FileEntry == UNICODE_NULL) {

				FindEnd = FileEntry;

				RtlZeroMemory(FileName, sizeof(FileName));
				wcsncpy_s(FileName, MAX_PATH, 
					FindFirt, 
					(FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
				LinkToWindowSystemPath(FileList, FileName);

				if (*FileEntry == UNICODE_NULL) {
					break;
			 }

				FindFirt = FileEntry + 1;

		}

		FileEntry++;
	}

	return TRUE;
}

BOOL IsAbsolutePathFile(__in LPWSTR Parameter, __inout std::set<std::wstring> &FileList)
{
	LPWSTR PathEntry = Parameter;
	LPWSTR FindFirt;
	LPWSTR FindEnd;
	PWCHAR pDotPostionW;
	WCHAR FileName[MAX_PATH];
	DWORD ParameterLength = (DWORD)wcslen(Parameter);

	PathEntry = Parameter;

	if (ParameterLength < sizeof(SHORT_FILE_NAME) - sizeof(WCHAR) ||
		PathEntry[1] != L':')
	{
		return FALSE;
	}

	FindFirt = PathEntry;

	pDotPostionW = wcschr(PathEntry, L'.');
	if (NULL == pDotPostionW){

		//
		// 逗号
		//
		WCHAR strTemp[MAX_PATH] = {0};
		wcscpy_s(strTemp, MAX_PATH, PathEntry);
		_wcslwr_s(strTemp, MAX_PATH);

		ParseDataToFullPath(strTemp, FileList);

		return FALSE;
	}

	PathEntry = FindEnd = pDotPostionW;

	while( TRUE )
	{
		if (*PathEntry == L','   ||
			*PathEntry == L' ' ||
			*PathEntry == L'-'||
			*PathEntry == L'/' ||
			*PathEntry == UNICODE_NULL 
			){

				FindEnd = PathEntry;

				break;
		}

		PathEntry++;
	}

	wcsncpy_s(FileName, MAX_PATH, FindFirt, (FindEnd - FindFirt) > MAX_PATH ? (MAX_PATH - 1) :(FindEnd - FindFirt));
	_wcslwr_s(FileName, MAX_PATH);

	ParseDataToFullPath(FileName, FileList);

	return TRUE;
}

BOOL IsIgnoreName(__in LPWSTR Name)
{
	if (UNICODE_NULL == *Name){
		return TRUE;
	}

	if (0 == _wcsnicmp(Name, DEVICE_PREFIX_NAME1, sizeof(DEVICE_PREFIX_NAME1)/sizeof(WCHAR) -1) ){
		return TRUE;
	}

	if (L'"' == *Name){
		Name++;
	}

	if (0 == _wcsnicmp(Name, DEVICE_PREFIX_NAME2 , sizeof(DEVICE_PREFIX_NAME2)/sizeof(WCHAR) - 1)) {

		return TRUE;
	}

	return FALSE;
}

VOID
ParseRegistryPathFromType( __in LPWSTR RegisterFileParameter,
						  __in LPWSTR MulitFileParameter,
						  __in DWORD Type, 
						  __inout std::set<std::wstring> &FileList)
{
	DWORD BufferLength;
	BOOL  Expand = FALSE;
	WCHAR ExpandParameter[MAX_PATH];
	LPWSTR FileParameter;
	BOOL MallocExpandBuffer = FALSE;

	if (REG_SZ == Type){

		ParseRegistryPath(RegisterFileParameter, REG_SZ, FileList);

		return;
	}

	if (REG_EXPAND_SZ == Type){

		//
		//   当前服务为Session 0
		//
		FileParameter =  ExpandParameter;
		BufferLength = ExpandEnvironmentStrings(RegisterFileParameter, FileParameter, sizeof(ExpandParameter)/sizeof(WCHAR));
		if (BufferLength > sizeof(ExpandParameter)/sizeof(WCHAR)){

			FileParameter = (LPWSTR)malloc(BufferLength * sizeof(WCHAR) + UNICODE_NULL);
			if(NULL == FileParameter){
				return;
			}

			ExpandEnvironmentStrings(RegisterFileParameter, FileParameter, BufferLength + UNICODE_NULL);
			MallocExpandBuffer = TRUE;
		}

		ParseRegistryPath(FileParameter, REG_SZ, FileList);

		if (MallocExpandBuffer){
			free(FileParameter);
			MallocExpandBuffer = FALSE;
		}

		return;
	}

	if (REG_MULTI_SZ == Type){

		FileParameter = MulitFileParameter;

		if (NULL == FileParameter){
			return;
		}

		while( UNICODE_NULL != *FileParameter )
		{
			ParseRegistryPath(FileParameter, REG_SZ, FileList);

			FileParameter += (ULONG)wcslen(FileParameter) + 1;
		}

		return;
	}
}

VOID
ParseRegistryPath(__in LPWSTR RegisterParameter, 
				  __in DWORD Type, 
				  __inout std::set<std::wstring> &FileList)
{

	BOOL  Expand = FALSE;
	WCHAR ExpandParameter[MAX_PATH];
	LPWSTR PathEntry = NULL;
	DWORD BufferLength;
	LPWSTR Parameter;
	DWORD ParameterLength;
	std::wstring FileEntry;
	BOOL MallocExpandBuffer = FALSE;
	std::vector<MY_WSTRING> vecScanFilePath;
	std::vector<MY_WSTRING>::const_iterator itScanFilePath;

	if (NULL == RegisterParameter){
		return;
	}

	if (REG_EXPAND_SZ != Type &&
		REG_SZ != Type &&
		REG_MULTI_SZ != Type){

			return;
	}

	ParameterLength = (DWORD)wcslen(RegisterParameter);

	if (0 == ParameterLength){
		return;
	}

	while( UNICODE_NULL != *RegisterParameter )
	{
		if (*RegisterParameter != L' ' &&
			*RegisterParameter != L'@' &&
			*RegisterParameter != L'!' &&
			*RegisterParameter != L'\\' &&
			*RegisterParameter != L'?' ){

				break;
		}

		RegisterParameter++;
		ParameterLength --;
	}

	if (0 == ParameterLength){
		return;
	}

	Parameter = RegisterParameter;
	if(REG_EXPAND_SZ == Type)
	{
		Expand = TRUE;

		BufferLength = ExpandEnvironmentStrings(RegisterParameter, ExpandParameter, sizeof(ExpandParameter)/sizeof(WCHAR));
		if (BufferLength > sizeof(ExpandParameter)/sizeof(WCHAR)){

			Parameter = (LPWSTR)malloc(BufferLength * sizeof(WCHAR) + UNICODE_NULL);
			if(NULL == Parameter){
				return;
			}

			ExpandEnvironmentStrings(RegisterParameter, Parameter, BufferLength + UNICODE_NULL);
			MallocExpandBuffer = TRUE;

		}else{

			Parameter = ExpandParameter;
		}
	}

	if ( ::PathFileExists(Parameter))
	{
		goto __exit;
	}

	if ( IsIgnoreName(RegisterParameter) ){

		goto __exit;
	}

	if ( IsSystemPath(Parameter, FileList) ){
		goto __exit;
	}

	if( IsResPath(Parameter, FileList) ) {
		goto __exit;
	}

	if( IsRundll32Path(Parameter, FileList) ){
		goto __exit;
	}

	if ( IsRegsvr32Path(Parameter, FileList) ){
		goto __exit;
	}

	if( IsDoubleQuotationFile(Parameter, FileList) ){
		goto __exit;
	}

	if ( IsAbsolutePathFile(Parameter, FileList) ){
		goto __exit;
	}

	if ( IsCommaFile(Parameter, FileList) ){
		goto __exit;
	}

__exit:

	GetFileFullPath(Parameter, NULL, vecScanFilePath);
	itScanFilePath = vecScanFilePath.begin();
	for (; itScanFilePath != vecScanFilePath.end(); itScanFilePath++)
	{
		if (wcslen(itScanFilePath->string) > sizeof(SHORT_FILE_NAME)/sizeof(WCHAR) - 1){

			wchar_t strTemp[MAX_PATH] = {0};

			wcscpy_s(strTemp, MAX_PATH, itScanFilePath->string);
			_wcslwr_s(strTemp, MAX_PATH);

			ParseDataToFullPath(strTemp, FileList);
		}

	}

	if (MallocExpandBuffer){
		free(Parameter);
	}
}