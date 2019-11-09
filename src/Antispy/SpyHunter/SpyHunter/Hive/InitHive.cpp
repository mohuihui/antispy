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
#include <map>
#include "ntreg.h"
#include "InitHive.h"
#include <Tlhelp32.h>
#include <winioctl.h>
#include "..\ProcessFunc.h"

//////////////////////////////////////////////////////////////////////////
typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation, /// Obsolete: Use KUSER_SHARED_DATA
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemMirrorMemoryInformation,
	SystemPerformanceTraceInformation,
	SystemObsolete0,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemPowerInformationNative,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemTimeSlipNotification,
	SystemSessionCreate,
	SystemSessionDetach,
	SystemSessionInformation,
	SystemRangeStartInformation,
	SystemVerifierInformation,
	SystemAddVerifier,
	SystemSessionProcessesInformation,
	SystemLoadGdiDriverInSystemSpaceInformation,
	SystemNumaProcessorMap,
	SystemPrefetcherInformation,
	SystemExtendedProcessInformation,
	SystemRecommendedSharedDataAlignment,
	SystemComPlusPackage,
	SystemNumaAvailableMemory,
	SystemProcessorPowerInformation,
	SystemEmulationBasicInformation,
	SystemEmulationProcessorInformation,
	SystemExtendedHanfleInformation,
	SystemLostDelayedWriteInformation,
	SystemBigPoolInformation,
	SystemSessionPoolTagInformation,
	SystemSessionMappedViewInformation,
	SystemHotpatchInformation,
	SystemObjectSecurityMode,
	SystemWatchDogTimerHandler,
	SystemWatchDogTimerInformation,
	SystemLogicalProcessorInformation,
	SystemWo64SharedInformationObosolete,
	SystemRegisterFirmwareTableInformationHandler,
	SystemFirmwareTableInformation,
	SystemModuleInformationEx,
	SystemVerifierTriageInformation,
	SystemSuperfetchInformation,
	SystemMemoryListInformation,
	SystemFileCacheInformationEx,
	SystemThreadPriorityClientIdInformation,
	SystemProcessorIdleCycleTimeInformation,
	SystemVerifierCancellationInformation,
	SystemProcessorPowerInformationEx,
	SystemRefTraceInformation,
	SystemSpecialPoolInformation,
	SystemProcessIdInformation,
	SystemErrorPortInformation,
	SystemBootEnvironmentInformation,
	SystemHypervisorInformation,
	SystemVerifierInformationEx,
	SystemTimeZoneInformation,
	SystemImageFileExecutionOptionsInformation,
	SystemCoverageInformation,
	SystemPrefetchPathInformation,
	SystemVerifierFaultsInformation,
	MaxSystemInfoClass,
} SYSTEM_INFORMATION_CLASS ;

// typedef struct _UNICODE_STRING {
// 	USHORT Length;        /* bytes */
// 	USHORT MaximumLength; /* bytes */
// 	PWSTR  Buffer;
// } UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_NAME_INFORMATION
{
	UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

/////////////////////////////////////////////////////////////////

DWORD g_Sytem_Pid				= 0 ;	// System进程Id
DWORD g_FileHandleObjectType	= 0 ;	// 文件句柄的对象类型标号

pNtQuerySystemInformation ZwQuerySystemInformation ;
pZwQueryInformationFile ZwQueryInformationFile;
pZwQueryObject ZwQueryObject;
pZwTerminateThread ZwTerminateThread;

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004)

CProcessFunc m_ProcessFunc;

//////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)  
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4018)

BOOL 
Init_hive_nt_fun_from_ntdll (
	)
/*++

Routine Description:
  分析Hive文件需要的3个Nt函数地址

Return Value:
  成功返回TRUE,失败返回FALSE   

--*/
{
	BOOL bResult = FALSE	;
	HMODULE hMod			;
	
	hMod = GetModuleHandle(_T("ntdll.dll"));
	if(NULL != hMod)
	{
		ZwQuerySystemInformation = (pNtQuerySystemInformation)GetProcAddress(hMod, "ZwQuerySystemInformation");
		ZwQueryInformationFile = (pZwQueryInformationFile)GetProcAddress(hMod, "ZwQueryInformationFile");
		ZwQueryObject = (pZwQueryObject)GetProcAddress(hMod, "ZwQueryObject");
		ZwTerminateThread = (pZwTerminateThread)GetProcAddress(hMod, "ZwTerminateThread");
		
		if(NULL != ZwQuerySystemInformation && NULL != ZwQueryInformationFile && NULL != ZwQueryObject && NULL != ZwTerminateThread)
		{
			bResult = TRUE;
		}
	}
	
	return bResult;
}


BOOL 
get_system_pid (
	)
/*++

Routine Description:
  获取System进程的Pid

Return Value:
  成功返回TRUE,失败返回FALSE  

--*/
{
	BOOL bResult = FALSE	;
	HANDLE hToolhelp		;
	PROCESSENTRY32 pi		;
	
	hToolhelp = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( INVALID_HANDLE_VALUE != hToolhelp )
	{
		pi.dwSize = sizeof( PROCESSENTRY32 );
		if( FALSE != Process32First( hToolhelp, &pi ) )
		{
			do 
			{
				if( 0 == _tcsicmp( pi.szExeFile, _T("System") ) )
				{
					g_Sytem_Pid = pi.th32ProcessID ;
					bResult = TRUE ;
					break;
				}

				memset( &pi, 0, sizeof( PROCESSENTRY32 ) );
				pi.dwSize = sizeof( PROCESSENTRY32 );
			} while( FALSE != Process32Next( hToolhelp, &pi ) );
		}
		
		CloseHandle( hToolhelp );
	}
	
	return bResult ;
}


BOOL 
get_file_ojbect_type_number (
	IN PALL_SYSTEM_HANDLE_INFORMATION pAHI, 
	IN HANDLE hNul
	)
/*++

Routine Description:
  获取文件句柄的ObjectTypeNumber

Arguments:
  pAHI - 系统句柄信息
  hNul - Nul句柄

Return Value:
  成功返回TRUE,失败返回FALSE  

--*/
{
	BOOL bResult = FALSE			;
	DWORD nCount, nIndex, nPid		;
	PSYSTEM_HANDLE_INFORMATION pSHI ;
	
	nPid	= GetCurrentProcessId( );
	nCount	= pAHI->Count ;
	pSHI	= (PSYSTEM_HANDLE_INFORMATION) pAHI->Handles ;

	for( nIndex = 0; nIndex < nCount; nIndex++, pSHI++ )
	{
		if( nPid == pSHI->ProcessId )
		{
			if( (USHORT)hNul == pSHI->Handle )
			{
				g_FileHandleObjectType = (DWORD) pSHI->ObjectTypeNumber ;
				bResult = TRUE ;
				break ;
			}
		}
	}

	return bResult;
}

PALL_SYSTEM_HANDLE_INFORMATION 
get_system_handle_table (
	)
/*++

Routine Description:
  获取系统句柄表

Return Value:
  成功返回系统句柄表存放内存指针,否则NULL 

--*/
{
	NTSTATUS status								= (NTSTATUS) -1 ;
	DWORD nNeedMemSize							= 0		;
	PALL_SYSTEM_HANDLE_INFORMATION pResult		= NULL	;



	status = ZwQuerySystemInformation (
		SystemHandleInformation,
		NULL,
		0,
		&nNeedMemSize );

	if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
	{
		printf( "FAILED_WITH_STATUS,NtQuerySystemInformation,Status:0x%08lx",status );
		return pResult;
	}
	
	do 
	{
		nNeedMemSize += 0x10000 ; // 加大点内存

		if (pResult)
		{
			free( pResult );
			pResult = NULL;
		}
		
		pResult = (PALL_SYSTEM_HANDLE_INFORMATION) malloc( nNeedMemSize );
		if(NULL == pResult)
		{
			break ;
		}

		status = ZwQuerySystemInformation (
			SystemHandleInformation, 
			(void *)pResult, 
			nNeedMemSize, 
			NULL) ;

		if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
		{
			printf( "FAILED_WITH_STATUS,NtQuerySystemInformation,Status:0x%08lx",status );
			free( pResult );
			pResult = NULL ;
			break ;
		}

	} while( status == STATUS_INFO_LENGTH_MISMATCH );
	
	return pResult;
}


BOOL 
get_reg_to_hive_file (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  枚举 HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\hivelist
  下的Hive文件名和注册表的对应情况

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  成功TRUE,否则FALSE

--*/
{
	BOOL bResult = FALSE;
	HKEY hKey ;
	DWORD nIndex, nType, nResult = 0 ;
	

	nResult =  (DWORD) RegOpenKeyEx (
		HKEY_LOCAL_MACHINE,
		_T( "SYSTEM\\CurrentControlSet\\Control\\hivelist" ),
		0, 
		KEY_READ,
		&hKey );

	if( ERROR_SUCCESS != nResult ) 
	{
		dprintf( "get_reg_to_hive_file() RegOpenKeyEx FAILED: %d\n", nResult );
		return bResult ;
	}
	
	//枚举其下的注册表值
	for( nIndex = 0; TRUE; nIndex++ )
	{
		DWORD nValueNameLen = MAX_PATH;
		DWORD nValueLen = MAX_PATH * sizeof(TCHAR);
		TCHAR strValueName[MAX_PATH] = {0};
		TCHAR strValue[MAX_PATH] = {0};
		RegHiveRootKey regHiveRootKey ;
		RegHiveFileItem regHiveFileItem ;

		nResult = (DWORD) RegEnumValue (
			hKey, nIndex, strValueName, &nValueNameLen, 
			0, &nType, (LPBYTE)strValue, &nValueLen ) ;

		if( ERROR_SUCCESS != nResult ) 
		{
			dprintf( "get_reg_to_hive_file() RegEnumValue FAILED: %d\n", nResult );
			return bResult ;
		}
		
		if(REG_SZ == nType)
		{
			if(0 != _tcslen(strValue))
			{
				bResult = TRUE;
				
				regHiveFileItem.hRegFile = NULL; // xx
				_tcsncpy( regHiveFileItem.strRegFilePath, strValue, MAX_PATH );
				regHiveFileItem.strRegFilePath[MAX_PATH - 1] = _T('\0') ;

				_tcsncpy( regHiveRootKey.strRegHiveRootKey, strValueName, MAX_PATH );
				regHiveRootKey.strRegHiveRootKey[MAX_PATH - 1] = _T('\0') ;
				
				mapRegHiveFile.insert (
					/*std::*/map<RegHiveRootKey, RegHiveFileItem>::value_type(regHiveRootKey, regHiveFileItem)
					);
			}
		}
		
		//nValueNameLen = MAX_PATH ;
		//nValueLen = MAX_PATH * sizeof(TCHAR) ;
	}
	
	RegCloseKey(hKey);

	return bResult ;
}

BOOL 
is_our_need (
	IN WCHAR *strFilePath,
	IN HANDLE hFile, 
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  文件句柄路径是否我们要的

Arguments:
  strFilePath - 文件句柄路径
  hFile - 文件句柄
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  如果是我们要的TRUE,否则一律FALSE

--*/
{
	BOOL bResult = FALSE;
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter;

	if( NULL == strFilePath ) { return bResult; }

	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); iter++ )
	{

		if( 0 == _tcsicmp( strFilePath, iter->second.strRegFilePath ) )
		{
			bResult = TRUE ;
			iter->second.hRegFile = hFile ;
			break;
		}
	}

	return bResult;
}


PVOID 
get_FileObjectName_from_handle (
	HANDLE hObject
	)
/*++

Routine Description:
  调用 ZwQueryInformationFile,ZwQueryObject获取文件句柄对应的文件路径

Arguments:
  hObject - 句柄(文件,设备,进程 ...)

Return Value:

--*/
{
	PVOID		pvBuffer = NULL;
	DWORD		dwLengthRet = 0;
	NTSTATUS	status = 0;
	UNICODE_STRING* pUnicodeString = NULL; 
	IO_STATUS_BLOCK ioStatus ;
	char sztmp[4096] = {0} ;

	status = ZwQueryInformationFile (
		hObject, 
		&ioStatus, 
		(PVOID)&sztmp, 
		sizeof(sztmp),
		FileNameInformation );
	
	if( STATUS_SUCCESS != status ) { return pvBuffer; }
	
	status = ZwQueryObject( hObject, ObjectNameInformation, NULL, 0, &dwLengthRet );
	if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
	{
		printf( "get_FileObjectName_from_handle() ZwQueryObject() FAILED,Status:0x%08lx",status );
		return pvBuffer;
	}

	do 
	{
		dwLengthRet += 0x10000 ; // 加大点内存
		
		if (pvBuffer)
		{
			free( pvBuffer );
			pvBuffer = NULL;
		}
		
		pvBuffer = malloc( dwLengthRet );
		if(NULL == pvBuffer)
		{
			break ;
		}
		
		status = ZwQueryObject( hObject, ObjectNameInformation, pvBuffer, dwLengthRet, &dwLengthRet );
		
		if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
		{
			printf( "FAILED_WITH_STATUS,NtQuerySystemInformation,Status:0x%08lx",status );
			free( pvBuffer );
			pvBuffer = NULL ;
			break ;
		}
		
	} while( status == STATUS_INFO_LENGTH_MISMATCH );
	
	pUnicodeString = (UNICODE_STRING *)pvBuffer ;
	if( 0 != pUnicodeString->Length && pUnicodeString->Buffer )
	{
	//	printf( "Name:%ws\n", pUnicodeString->Buffer );
		return pUnicodeString;
	}

	return NULL ;
}


void RaiseToDebugP()
{
    HANDLE hToken;
    HANDLE hProcess = GetCurrentProcess();
    if ( OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
    {
        TOKEN_PRIVILEGES tkp;
        if ( LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid) )
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            
            BOOL bREt = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0) ;
        }
        CloseHandle(hToken);
    }    
}


BOOL 
duplicate_hive_file_handle (
	IN PALL_SYSTEM_HANDLE_INFORMATION pAHI, 
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  复制注册表文件句柄到本进程

Arguments:
  pAHI - 系统句柄信息
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  成功TRUE,否则FALSE

--*/
{
	BOOL bResult = FALSE ;
	BOOL bIsOurNeed = FALSE;
	DWORD nCount, nIndex ;
	HANDLE hSystemProcess = NULL;
	HANDLE hCurrentHandle, hResultHandle;
	PSYSTEM_HANDLE_INFORMATION pSHI;

	RaiseToDebugP() ;
	hSystemProcess = m_ProcessFunc.OpenProcess( PROCESS_ALL_ACCESS, FALSE, g_Sytem_Pid );
	
	if( NULL == hSystemProcess ) 
	{ 
		MessageBox(NULL, L"ERROR", L"WW", 0);
		return bResult;
	}
// 	else
// 	{
// 		MessageBox(NULL, L"oK", L"WW", 0);
// 	}
	
	nCount	= pAHI->Count	;
	pSHI	= pAHI->Handles	;
	hCurrentHandle = GetCurrentProcess();
	
	for( nIndex = 0; nIndex < nCount; nIndex++, pSHI++ )
	{
		if( (g_Sytem_Pid == pSHI->ProcessId)
			&& (g_FileHandleObjectType == (DWORD) pSHI->ObjectTypeNumber)
		  )
		{
			// 是System进程中的File类型句柄就复制过来
			if( FALSE == 
				DuplicateHandle(
					hSystemProcess,
					(HANDLE)pSHI->Handle, 
					hCurrentHandle,
					&hResultHandle, 
					GENERIC_WRITE, 
					FALSE, 
					DUPLICATE_SAME_ACCESS )
			   )
			{
				continue ; 
			}
			
			WCHAR tmpName[4096] = {0} ;
			UNICODE_STRING* pObjectName = (UNICODE_STRING*)get_FileObjectName_from_handle( hResultHandle );
			
			if ( NULL != pObjectName )
			{
				memcpy(tmpName, pObjectName->Buffer, pObjectName->Length);

			//	sprintf( tmpName, "%ws", pObjectName->Buffer );
				bIsOurNeed = is_our_need( tmpName, hResultHandle, mapRegHiveFile );
				free( pObjectName );
			}
			
			if( FALSE == bIsOurNeed )	// 不是我们需要的就关闭复制过来的句柄
			{
				CloseHandle( hResultHandle );
			}
			else
			{
				bResult = TRUE;	// 设置成功标志
			}
		}
	}
	
	CloseHandle( hSystemProcess );
	return bResult;
}


VOID 
del_reg_hive_file_handle_null (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  规整下注册表和Hive文件的对应关系,把文件句柄为NULL的干掉

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:

--*/
{
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter;
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iterTemp;
	
	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); )
	{
		if( NULL != iter->second.hRegFile ) {
			iter++;
		} else {
			iterTemp = iter;
			iter++;
			mapRegHiveFile.erase( iterTemp );
		}
	}
}


VOID
Display_RegHive_in_map (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  把最终得到的注册表hive句柄和对应的路径显示出来

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:

--*/
{
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter ;

	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); iter++ )
	{
// 		printf ( 
// 			"注册表根键:%ws\n注册表HIVE路径:%ws\n关联句柄:0x%08lx\n\n",
// 			iter->first.strRegHiveRootKey,
// 			iter->second.strRegFilePath,
// 			iter->second.hRegFile
// 			);
	}

	return ;
}

WCHAR* ms2ws(LPCSTR szSrc, int cbMultiChar = -1)
{
	WCHAR *strDst = NULL;

	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	WCHAR*pBuff=NULL;
	int nLen=MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,NULL,0);
	if ( nLen>0 ){
		pBuff=new WCHAR[nLen+1];
		memset(pBuff, 0, (nLen+1)*sizeof(WCHAR));
		if ( pBuff ){
			MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,pBuff,nLen);
			pBuff[nLen]=0;
			strDst = pBuff;
		}
	}

	return strDst;
}

HANDLE
find_RegHive_handle_in_map (
	IN CHAR* strHiveRootPath,
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  在map容器中找到 比如根键 "\\REGISTRY\\MACHINE\\SAM" 对应的句柄

Arguments:
  strHiveRootPath - eg. "\\REGISTRY\\MACHINE\\SAM"
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  
--*/
{
	HANDLE hRet = NULL;
	map<RegHiveRootKey, RegHiveFileItem>::iterator iter ;
	WCHAR *szPath = NULL;

	if ( NULL == strHiveRootPath ) { return 0; }
	
	szPath = ms2ws(strHiveRootPath);
	if (szPath)
	{
		for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); iter++ )
		{
			if ( !wcsnicmp( iter->first.strRegHiveRootKey, szPath, wcslen(szPath) ) )
			{
				hRet = iter->second.hRegFile ;
				break;
			}
		}

		delete[] szPath;
		szPath = NULL;
	}
	
	return hRet;
}


//////////////////////////////////////////////////////////////////////////

BOOL g_bHive_Initied = FALSE ; 

BOOL 
Init_hive_analyse (
	/*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Routine Description:
  初始化R3解析hive,准备好一切东西

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:

--*/
{
	BOOL bResult = FALSE;
	HANDLE hFile;
	PALL_SYSTEM_HANDLE_INFORMATION pAHI;
	
	mapRegHiveFile.clear();
	
	// 获取3个Nt函数地址
	if( FALSE == Init_hive_nt_fun_from_ntdll() ) { return bResult; }	

	// 获取System进程Id
	if( FALSE == get_system_pid() ) { return bResult; }

	// 产生一个文件句柄
	hFile = CreateFile(_T("NUL"), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);	
	if( INVALID_HANDLE_VALUE == hFile ) { return bResult; }

	pAHI = get_system_handle_table( );	// 获取系统句柄表
	if( NULL == pAHI ) { return bResult; }
				
	// 获取文件句柄的对象类型标号
	if( FALSE == get_file_ojbect_type_number( pAHI, hFile ) ) { return bResult; }	
					
	// 获取注册表和Hive文件对应关系
	if( FALSE == get_reg_to_hive_file( mapRegHiveFile ) ) { return bResult; }	
	
	// 复制注册表文件句柄到本进程
	if( FALSE == duplicate_hive_file_handle( pAHI, mapRegHiveFile ) ) { return bResult; }
	
	// 规整下注册表和Hive文件的对应关系,把文件句柄为NULL的干掉
	del_reg_hive_file_handle_null( mapRegHiveFile );
	bResult = TRUE;

	// 打印收集的注册表hive和关联句柄等信息
	Display_RegHive_in_map( mapRegHiveFile );

	// 收尾工作
	free(pAHI);
	pAHI = NULL;	
	CloseHandle(hFile);

	if(FALSE == bResult) { mapRegHiveFile.clear(); }
	g_bHive_Initied = bResult ; 
	return bResult;
}



BOOL
map_or_read_file (
	IN char *filename,
	IN struct hive *hdesc
	)
{
	DWORD szread ;
	PCHAR pTMP = NULL ; 
	HANDLE hMap = NULL, htmpFile = NULL, hFileMemory = NULL ;

	if ( NULL == filename ) { return FALSE ; }

	hdesc->state		= 0		;
	hdesc->buffer		= NULL	;
	hdesc->bMapped		= FALSE ;
	hdesc->hFileMemory	= NULL	;
	hdesc->filename		= str_dup( filename );

	// 表明是要从内存中去解析hive, 路径如: "\\REGISTRY\\MACHINE\\SOFTWARE"
	if ( '\\' == *filename )
	{
		/*std::*/map<RegHiveRootKey, RegHiveFileItem> mapRegHiveFile ;
		
		Init_hive_analyse( mapRegHiveFile );
		hFileMemory = find_RegHive_handle_in_map( filename, mapRegHiveFile );
		if ( NULL == hFileMemory )
		{
			//
			// 清理工作
			//
			return FALSE ;
		}

		FlushFileBuffers( hFileMemory );

		hdesc->hFileMemory	= hFileMemory ;
		hdesc->size = GetFileSize( hFileMemory,  NULL );
		hMap = CreateFileMapping( hFileMemory, NULL, /*PAGE_READONLY |*/ PAGE_READWRITE, 0, 0, 0 );
		if( NULL == hMap ) { return FALSE ; }
		
		pTMP = (PCHAR)MapViewOfFileEx( hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, NULL );
		if ( NULL == pTMP ) { return FALSE ; }
		
		hdesc->buffer = pTMP;	
		hdesc->bMapped = TRUE ;
		return TRUE ;

	} 
	else // 是读其他hive文件
	{
		htmpFile = CreateFileA(hdesc->filename,
			GENERIC_READ,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		
		if (htmpFile == INVALID_HANDLE_VALUE) { return FALSE ; }
		
		// Read the whole file
		hdesc->size = GetFileSize( htmpFile,  NULL );
		/*ALLOC( hdesc->buffer, 1, hdesc->size );*/

		hdesc->buffer = (char *) calloc(1, hdesc->size);
		if (hdesc->buffer)
		{
			CloseHandle(htmpFile);
			return FALSE;
		}

		ReadFile( htmpFile, (void *)hdesc->buffer, hdesc->size, &szread, NULL );
		CloseHandle( htmpFile );
		return TRUE ;
	}

	return FALSE ;
}


struct hive *
My_openHive (
	IN char *filename,
	IN int mode
	)
{
	BOOL bResult = FALSE ;
	struct hive *hdesc;
	int vofs;
	unsigned long pofs;
	char *c;
	struct hbin_page *p;
	struct regf_header *hdr;
	
	int verbose = (mode & HMODE_VERBOSE);
	CREATE(hdesc,struct hive,1);
	
	bResult = map_or_read_file( filename, hdesc );
	if ( FALSE == bResult )
	{ 
		My_closeHive( hdesc );
		return NULL ;
	}
	
	// Now run through file, tallying all pages
	// NOTE/KLUDGE: Assume first page starts at offset 0x1000
	pofs = 0x1000;
	hdr = (struct regf_header *)hdesc->buffer;
	if (hdr->id != 0x66676572) 
	{
		printf("openHive(%s): File does not seem to be a registry hive!\n",filename);
		return NULL ;
	}
	for (c = hdr->name; *c && (c < hdr->name + 64); c += 2) 
		putchar(*c);
	
	printf( "\n" );
	hdesc->rootofs = hdr->ofs_rootkey + 0x1000;
	while (pofs < hdesc->size) 
	{
#ifdef LOAD_DEBUG
		if (verbose) 
			hexdump(hdesc->buffer,pofs,pofs+0x20,1);
#endif
		p = (struct hbin_page *)(hdesc->buffer + pofs);
		if (p->id != 0x6E696268) 
		{
			printf("Page at 0x%lx is not 'hbin', assuming file contains garbage at end",pofs);
			break;
		}
		
		hdesc->pages++;
#ifdef LOAD_DEBUG
		if (verbose) 
			printf("\n###### Page at 0x%0lx has size 0x%0lx, next at 0x%0lx ######\n",pofs,p->len_page,p->ofs_next);
#endif
		
		if (p->ofs_next == 0) 
		{
#ifdef LOAD_DEBUG
			if (verbose) 
				printf("openhive debug: bailing out.. pagesize zero!\n");
#endif
			return(hdesc);
		}
		
#if 0
		
		if (p->len_page != p->ofs_next)
		{
#ifdef LOAD_DEBUG
			if (verbose) 
				printf("openhive debug: len & ofs not same. HASTA!\n");
#endif
			exit(0);
			
		}
		
#endif
		vofs = pofs + 0x20; /* Skip page header */
		
#if 1
		while (vofs-pofs < p->ofs_next)
		{
			vofs += parse_block(hdesc,vofs,verbose);
		}
#endif
		pofs += p->ofs_next;
		
	}

	return(hdesc);
}



void My_closeHive(struct hive *hdesc)
{
	if (!hdesc)
	{
		return;
	}

	// FREE(hdesc->filename);
	FlushFileBuffers( hdesc->hFileMemory );

	if ( hdesc->bMapped ) {
		UnmapViewOfFile( hdesc->buffer );
	} else {
		FREE( hdesc->buffer );
	}
	
	if (hdesc->hFileMemory)
	{
		CloseHandle(hdesc->hFileMemory);
	}

	if (hdesc->filename)
	{
		free(hdesc->filename);
	}

	free( hdesc );
}


int My_writeHive(struct hive *hdesc)
{
	
	HANDLE hFile;
	DWORD dwBytesWritten;
	hFile = CreateFileA("C:\\tmp2.hiv",  
		GENERIC_WRITE,                // open for writing
		0,                            // do not share
		NULL,                         // no security
		CREATE_ALWAYS,                  // open or create
		FILE_ATTRIBUTE_NORMAL,        // normal file
		NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{      
		printf("Can't open dump file");
		return 0;
	}
	WriteFile(hFile, hdesc->buffer, hdesc->size,&dwBytesWritten, NULL);
	if(dwBytesWritten != hdesc->size)
	{
		printf("WriteHive error\n");
	}
	CloseHandle(hFile);
	return 0;
}

#pragma warning(default:4996)  
#pragma warning(default:4244)
#pragma warning(default:4267)
#pragma warning(default:4018)