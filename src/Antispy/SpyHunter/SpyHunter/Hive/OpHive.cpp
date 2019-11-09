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
#include "OpHive.h"

#define SZ_ROOT				"HKEY_CLASSES_ROOT"
#define SZ_SOFTWARE			"HKEY_LOCAL_MACHINE\\SOFTWARE"
#define SZ_CUTTENT_USER		"HKEY_CURRENT_USER"
#define SZ_SAM				"HKEY_LOCAL_MACHINE\\SAM"
#define	SZ_SECURITY			"HKEY_LOCAL_MACHINE\\SECURITY"
#define SZ_SYSTEM			"HKEY_LOCAL_MACHINE\\SYSTEM"
#define SZ_USERS			"HKEY_USERS"
#define	SZ_LOCAL_MACHINE	"HKEY_LOCAL_MACHINE"
#define SZ_CURRENT_CONFIG	"HKEY_CURRENT_CONFIG"
#define	SZ_HARDWARE			"HKEY_LOCAL_MACHINE\\HARDWARE"

// hive 键值
#define HIVE_SAM		"\\REGISTRY\\MACHINE\\SAM"
#define HIVE_SECURITY	"\\REGISTRY\\MACHINE\\SECURITY"
#define HIVE_SOFTWARE	"\\REGISTRY\\MACHINE\\SOFTWARE"
#define HIVE_SYSTEM		"\\REGISTRY\\MACHINE\\SYSTEM"
#define HIVE_DEFAULT	"\\REGISTRY\\USER\\.DEFAULT"

typedef BOOL (WINAPI* PtrConvertSidToStringSid)(
	PSID Sid,
	LPTSTR* StringSid
	);

#pragma warning(disable:4996)  
#pragma warning(disable:4244)
#pragma warning(disable:4267)

COperateHive::COperateHive()
{
	m_pTree = NULL;
	m_Item = NULL;
	m_pList = NULL;

	memset(&m_HiveInfos, 0, sizeof(HIVE_INFOS));
}

COperateHive::~COperateHive()
{
	My_closeHive(m_HiveInfos.pHive_HKLM_SAM);
	My_closeHive(m_HiveInfos.pHive_HKLM_SECURITY);
	My_closeHive(m_HiveInfos.pHive_HKLM_SOFTWARE);
	My_closeHive(m_HiveInfos.pHive_HKLM_SYSTEM);
	My_closeHive(m_HiveInfos.pHive_HKCU);

	for (int i = 0; i < 20; i++)
	{
		My_closeHive(m_HiveInfos.pHive_OTHER_USER[i]);
	}

	for (int i = 0; i < 10; i++)
	{
		My_closeHive(m_HiveInfos.pHive_OTHER[i]);
	}
}

void COperateHive::SetTree(CTreeCtrl *pTree)
{
	m_pTree = pTree;
}

int COperateHive::GetUserName (IN char* szOutUserName)
{
	if ( szOutUserName == NULL ) { return 0 ; }
	*szOutUserName = 0;

	HANDLE hProcess = GetCurrentProcess();
	if(!hProcess) {
		return 0;
	}

	HANDLE hToken;
	if( !OpenProcessToken(hProcess, TOKEN_QUERY, &hToken) || !hToken ){
		CloseHandle(hProcess);
		return 0;
	}

	DWORD dwTemp = 0;
	char tagTokenInfoBuf[256] = {0};
	PTOKEN_USER tagTokenInfo = (PTOKEN_USER)tagTokenInfoBuf;
	if( !GetTokenInformation( hToken, TokenUser, tagTokenInfoBuf, sizeof(tagTokenInfoBuf),\
		&dwTemp ) ) {
			CloseHandle(hToken);
			CloseHandle(hProcess);
			return 0;
	}

	PtrConvertSidToStringSid dwPtr = (PtrConvertSidToStringSid)GetProcAddress( 
		LoadLibraryA("Advapi32.dll"), "ConvertSidToStringSidA" );

	LPTSTR MySid = NULL;
	dwPtr( tagTokenInfo->User.Sid, (LPTSTR*)&MySid );

	strcpy( szOutUserName, (char*)MySid );
	//	printf("sudami's PC Name:\n%s\n", MySid);
	//	getchar ();
	LocalFree( (HLOCAL)MySid );

	CloseHandle(hToken);
	CloseHandle(hProcess);

	return 0;
}

// checkType - 0 是ListValue; 1 是ListSubKey
HIVE_OPEN_TYPE COperateHive::IsHiveOpen(IN char *szKeyPath,
										IN char *subKeyPath,
										IN int checkType,
										hive **pHive)
{
	HIVE_OPEN_TYPE nRet = ReturnType_ERRO;

	if ( szKeyPath == NULL || 
		subKeyPath  == NULL ||
		pHive == NULL) 
	{ 
		return nRet ; 
	}
	
	*pHive = NULL;

	// 如果是没有HIVE与之对应的键,那么就返回,直接用API枚举
	if ((!stricmp(szKeyPath, SZ_LOCAL_MACHINE)) ||
		!stricmp( szKeyPath, SZ_USERS) ||
		!strnicmp(szKeyPath, SZ_CURRENT_CONFIG, strlen(SZ_CURRENT_CONFIG)) ||
		!strnicmp(szKeyPath, SZ_HARDWARE, strlen(SZ_HARDWARE)))
	{
		return ReturnType_IsRootKey ;
	}

	// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
	if ( (!strnicmp( szKeyPath, SZ_ROOT, strlen(SZ_ROOT) )) || 
		(!strnicmp( szKeyPath, SZ_SOFTWARE, strlen(SZ_SOFTWARE) )) )
	{
		// HKEY_LOCAL_MACHINE\SOFTWARE\Classes 是 HKEY_CLASSES_ROOT 的真身
		if ( !strnicmp( szKeyPath, SZ_ROOT, strlen(SZ_ROOT) ) ) 
		{
			char* ptr = strchr( szKeyPath, '\\' );
			if ( ptr++ ) 
			{
				sprintf( subKeyPath, "\\Classes\\%s", ptr );
			} 
			else 
			{
				strcpy( subKeyPath, "\\Classes" );
			}

		} 
		else if( !strnicmp( szKeyPath, SZ_SOFTWARE, strlen(SZ_SOFTWARE) ) )
		{
			char* ptr = strchr( szKeyPath, '\\' );
			if ( ptr++ )
			{
				ptr = strchr( ptr, '\\' );
				if ( ptr ) 
				{
					sprintf( subKeyPath, "%s", ptr );
				}
			}
		}

		if ( m_HiveInfos.pHive_HKLM_SOFTWARE ) 
		{
			// 表明已打开此hive
			*pHive = m_HiveInfos.pHive_HKLM_SOFTWARE ;
			return ReturnType_OK ;
		} 
		
		*pHive = m_HiveInfos.pHive_HKLM_SOFTWARE = My_openHive( HIVE_SOFTWARE, HMODE_RW );
		if ( NULL == *pHive ) 
		{ 
			return ReturnType_ERRO; 
		}

		return ReturnType_OK;
	}
	
	// 表明要列举 HKEY_CURRENT_USER\\xx 的键值.故要先打开
	// eg. HKEY_USERS\S-1-5-21-1214440339-1078145449-1343024091-500
	// 即直接打开当前用户的CLSID对应的Hive即可
	else if ( !strnicmp( szKeyPath, SZ_CUTTENT_USER, strlen(SZ_CUTTENT_USER) ) )
	{
		// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
		char* ptr = strchr( szKeyPath, '\\' );
		if ( ptr ) {
			sprintf( subKeyPath, "%s", ptr );
		}

		char szCurrentUserName[256] = {0};
		char szTmp[256] = {0};
		GetUserName( szCurrentUserName );
		
		if ( m_HiveInfos.pHive_HKCU && !strnicmp( m_HiveInfos.pHive_HKCU->filename, szCurrentUserName, strlen(szCurrentUserName) ) ) 
		{
			// 表明已打开此hive
			*pHive = m_HiveInfos.pHive_HKCU ;
			return ReturnType_OK ;
		}

		sprintf( szTmp, "\\REGISTRY\\USER\\%s", szCurrentUserName ); 
		*pHive = m_HiveInfos.pHive_HKCU = My_openHive( szTmp, HMODE_RW );
		if ( NULL == *pHive ) 
		{ 
			return ReturnType_ERRO ; 
		}

		return ReturnType_OK ;
	}


	else if (!strnicmp( szKeyPath, SZ_SAM, strlen(SZ_SAM) ) )
	{
		// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
		char* ptr = strchr( szKeyPath, '\\' );
		if ( ptr++ )
		{
			ptr = strchr( ptr, '\\' );
			if ( ptr ) {
				sprintf( subKeyPath, "%s", ptr );
			}
		}

		if ( m_HiveInfos.pHive_HKLM_SAM && !strnicmp( m_HiveInfos.pHive_HKLM_SAM->filename, HIVE_SAM, strlen(HIVE_SAM) ) ) 
		{
			// 表明已打开此hive
			*pHive = m_HiveInfos.pHive_HKLM_SAM ;
			return ReturnType_OK ;
		}
		
		*pHive = m_HiveInfos.pHive_HKLM_SAM = My_openHive( HIVE_SAM, HMODE_RW );
		if ( NULL == *pHive ) 
		{ 
			return ReturnType_ERRO; 
		}

		return ReturnType_OK ;
	}


	else if ( !strnicmp( szKeyPath, SZ_SECURITY, strlen(SZ_SECURITY) ) )
	{
		// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
		char* ptr = strchr( szKeyPath, '\\' );
		if ( ptr++ )
		{
			ptr = strchr( ptr, '\\' );
			if ( ptr ) {
				sprintf( subKeyPath, "%s", ptr );
			}
		}
		
		if ( m_HiveInfos.pHive_HKLM_SECURITY && !strnicmp( m_HiveInfos.pHive_HKLM_SECURITY->filename, HIVE_SECURITY, strlen(HIVE_SECURITY) ) ) 
		{
			// 表明已打开此hive
			*pHive = m_HiveInfos.pHive_HKLM_SECURITY ;
			return ReturnType_OK ;
		}
		
		*pHive = m_HiveInfos.pHive_HKLM_SECURITY = My_openHive( HIVE_SECURITY, HMODE_RW );

		if ( NULL == *pHive ) 
		{ 
			return ReturnType_ERRO ; 
		}

		return ReturnType_OK ;
	}

	
	else if ( !strnicmp( szKeyPath, SZ_SYSTEM, strlen(SZ_SYSTEM) ))
	{
/*++
  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Hardware Profiles\Current 是 HKEY_CURRENT_CONFIG 的真身
需要打开 HIVE_SYSTEM,还需填充相应的子键全路径
--*/
		// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
		char* ptr = strchr( szKeyPath, '\\' );

//		if( 0 != strstr( szKeyPath, "HKEY_CURRENT_CONFIG" ) ) {
//			// 这个项在hive文件中没有,也是一个Link. 只能用常规API来做了
//// 			if ( ptr ) {
//// 				sprintf( subKeyPath, "\\CurrentControlSet\\Hardware Profiles\\Current%s", ptr );
//// 			} else {
//// 				strcpy( subKeyPath, "\\CurrentControlSet\\Hardware Profiles\\Current" );
//// 			}
//			
//		} else {

		if ( ptr++ )
		{
			ptr = strchr( ptr, '\\' );
			if ( ptr ) {
				sprintf( subKeyPath, "%s", ptr );
			}
		}

//		}

		if ( m_HiveInfos.pHive_HKLM_SYSTEM && !strnicmp( m_HiveInfos.pHive_HKLM_SYSTEM->filename, HIVE_SYSTEM, strlen(HIVE_SYSTEM) ) ) 
		{
			// 表明已打开此hive
			*pHive = m_HiveInfos.pHive_HKLM_SYSTEM ;
			return ReturnType_OK ;
		}
		
		*pHive = m_HiveInfos.pHive_HKLM_SYSTEM = My_openHive( HIVE_SYSTEM, HMODE_RW );

		if ( NULL == *pHive ) 
		{ 
			return ReturnType_ERRO ; 
		}

		return ReturnType_OK ;
	}
	
	else if ( !strnicmp( szKeyPath, SZ_USERS, strlen(SZ_USERS) ) )
	{
		// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
		char* ptrXX = strchr( szKeyPath, '\\' );
		if ( ptrXX++ )
		{
			ptrXX = strchr( ptrXX, '\\' );
			if ( ptrXX ) {
				sprintf( subKeyPath, "%s", ptrXX );
			}
		}

		// 表明要列举 HKEY_USERS\\xx 的键值.
		char* ptr = strchr( szKeyPath, '\\' ) ;
		if ( NULL == ptr )
		{
			// 表明是打开的HKEY_USERS这个根键
			return ReturnType_IsRootKey ;
		}

		ptr++ ;
		char* ptrTmp = strchr( ptr, '\\' );
		if ( ptrTmp )
		{
			int nlength = (int)( ptrTmp - ptr );
			*( ptr + nlength ) = '\0';
		}

		char szTmp[256] = "";
		sprintf( szTmp, "\\REGISTRY\\USER\\%s", ptr ); 

		int nxx = 0;
		for( nxx = 0; m_HiveInfos.pHive_OTHER_USER[nxx]; nxx++ ) 
		{
			if ( !stricmp(  m_HiveInfos.pHive_OTHER_USER[nxx]->filename, szTmp ) )
			{
				// 要打开的键值的hive已经被map过了,直接取出即可
				*pHive = m_HiveInfos.pHive_OTHER_USER[nxx] ;
				return ReturnType_OK ;
			}
		}
		
		// 没有找到,则要map一份hive到内存
		for( nxx = 0; ; nxx++ ) 
		{
			// 找到一个空的结构
			if ( NULL ==  m_HiveInfos.pHive_OTHER_USER[nxx] ) { break; }
			if ( 20 == nxx ) 
			{ return ReturnType_ERRO ; }
		}

		*pHive = m_HiveInfos.pHive_OTHER_USER[nxx] = My_openHive( szTmp, HMODE_RW );
		if ( NULL == *pHive ) 
		{ 
			return ReturnType_ERRO ; 
		}

		return ReturnType_OK ;
	}
	else
	{
		if (!strnicmp(szKeyPath, SZ_LOCAL_MACHINE, strlen(SZ_LOCAL_MACHINE)))
		{
			// 先剥离出供解析hive的函数使用的子键全路径. eg. \\360Safe
			char* ptr = strchr( szKeyPath, '\\' );
			char* ptrTemp = ptr;
			if ( ptr++ )
			{
				ptr = strchr( ptr, '\\' );
				if ( ptr ) {
					sprintf( subKeyPath, "%s", ptr );
				}
			}

			char szTemp[1024] = {0};
			strcpy(szTemp, "\\REGISTRY\\MACHINE");
			if (!ptr)
			{
				strcat(szTemp, szKeyPath + strlen(SZ_LOCAL_MACHINE));
			}
			else
			{
				strncat(szTemp, szKeyPath + strlen(SZ_LOCAL_MACHINE), ptr - szKeyPath - strlen(SZ_LOCAL_MACHINE));
			}

			int nxx = 0;
			for( nxx = 0;  m_HiveInfos.pHive_OTHER[nxx]; nxx++ ) 
			{
				if ( !stricmp(  m_HiveInfos.pHive_OTHER[nxx]->filename, szTemp ) )
				{
					// 要打开的键值的hive已经被map过了,直接取出即可
					*pHive = m_HiveInfos.pHive_OTHER[nxx] ;
					return ReturnType_OK ;
				}
			}

			// 没有找到,则要map一份hive到内存
			for( nxx = 0; ; nxx++ ) 
			{
				// 找到一个空的结构
				if ( NULL ==  m_HiveInfos.pHive_OTHER[nxx] ) { break; }
				if ( 10 == nxx ) 
				{ return ReturnType_ERRO ; }
			}

			*pHive = m_HiveInfos.pHive_OTHER[nxx] = My_openHive( szTemp, HMODE_RW );
			if ( NULL == *pHive ) 
			{ 
				return ReturnType_ERRO ; 
			}

			return ReturnType_OK ;
		}
	}

	if ( 0 == strstr( szKeyPath, "\\" ) ) 
	{ 
		return ReturnType_IsRootKey ; 
	}

	return ReturnType_ERRO ;
}

BOOL COperateHive::ListSubKeys_hive (IN char *s )
{
	BOOL bRet = FALSE;
	char szSubKeyPath[2048] = {0} ;
	hive *pHive = NULL;

	int nRtnType = IsHiveOpen( s, szSubKeyPath, 1, &pHive );
	if ( ReturnType_OK != nRtnType || !pHive ) 
	{ 
		return bRet ; 
	}

	// 通过解析hive来枚举该键的所有子键
	FlushFileBuffers( pHive->hFileMemory );
	FlushFileBuffers( (HANDLE)pHive->buffer );
	bRet = nk_ls_ListSubKeys( pHive, szSubKeyPath );

	return bRet ;
}

CString COperateHive::ms2ws(LPCSTR szSrc, int cbMultiChar)
{
	CString strDst;

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
			delete[] pBuff;
			pBuff = NULL;
		}
	}

	return strDst;
}

BOOL COperateHive::nk_ls_ListSubKeys (
				   IN struct hive *hdesc, 
				   IN char* szSubKeyPath)
{
	BOOL bRet = FALSE;
	char szTmpxx[512] = {0} ;
	struct nk_key *key ;
	struct ex_data ex ;
	int nkofs = 0, count = 0, countri = 0;
	
	if (!m_Item || !m_pTree)
	{
		return FALSE;
	}

	nkofs = trav_path( hdesc, hdesc->rootofs + 4, szSubKeyPath, TPF_NK );
	if ( !nkofs ) 
	{
		return bRet;
	}
	nkofs += 4;

	key = (struct nk_key *)(hdesc->buffer + nkofs);
	if (key->id != 0x6b6e)
	{
		return bRet;
	}

	if (key->no_subkeys)
	{
		count = 0;	
// 		ex.name = (char *)malloc( 0x1000 ); // 不管怎样,先分配块大内存,供每次遍历存放名字用
// 		if (!ex.name)
// 		{
// 			return bRet;
// 		}

		int n = 0;
		while( (ex_next_n( hdesc, nkofs, &count, &countri, &ex ) > 0) ) 
		{
			HTREEITEM hItem = m_pTree->InsertItem(ms2ws(ex.name), 1, 2, m_Item, TVI_LAST);

			// 查看该子键是否存在子键,若有获取子键个数
			n = get_Key_s_subkey_Counts( 
				hdesc, "", ex.nkoffs + 4, TPF_NK );
			
			if (n)
			{
				m_pTree->InsertItem(L"1", hItem, TVI_LAST);
			}

			FREE(ex.name);
		}

// 		free( ex.name );
// 		ex.name = NULL;
	}

	return TRUE;
}

int COperateHive::get_Key_s_subkey_Counts (
						 IN struct hive *hdesc, 
						 IN char *path, 
						 IN int vofs,
						 IN int type
						 )
{
	struct nk_key *key;
	int nkofs;
	int count = 0, countri = 0, plen = 0 ;

	nkofs = trav_path( hdesc, vofs, path, type );

	if(!nkofs) 
	{
	//	printf("nk_ls: Key <%s> not found\n",path);
		return 0;
	}
	nkofs += 4;

	key = (struct nk_key *)(hdesc->buffer + nkofs);

	if (key->id != 0x6b6e)
	{
// 		printf("Error: Not a 'nk' node!\n");
// 		debugit( hdesc->buffer, hdesc->size );
		return 0;
	}

//	printf("Node has %ld subkeys and %ld values",key->no_subkeys,key->no_values);

	return (int)key->no_subkeys ;
}

CString COperateHive::GetKeyData(ULONG Type, WCHAR *Data, ULONG DataLength)
{
	CString szRet;

	if (!DataLength || !Data)
	{
		return szRet;
	}

	switch (Type)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		{
			WCHAR *szFuck = (WCHAR*)malloc(DataLength + sizeof(WCHAR));
			if (szFuck)
			{
				memset(szFuck, 0, DataLength + sizeof(WCHAR));
				memcpy(szFuck, Data, DataLength);
				szRet = szFuck;

				free(szFuck);
			}
		}
		break;

	case REG_LINK:
	case REG_NONE:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_LIST:
	case REG_BINARY:
		{
			for (ULONG i = 0; i < DataLength; i++)
			{
				CString szTemp;
				szTemp.Format(L"%02x ", *((PBYTE)Data + i));
				szRet += szTemp;
			}
		}
		break;

	case REG_DWORD:
		szRet.Format(L"0x%08X (%d)", *(PULONG)Data, *(PULONG)Data);
		break;

	case REG_DWORD_BIG_ENDIAN:
		{
			BYTE Value[4] = {0};
			Value[0] = *((PBYTE)Data + 3);
			Value[1] = *((PBYTE)Data + 2);
			Value[2] = *((PBYTE)Data + 1);
			Value[3] = *((PBYTE)Data + 0);
			szRet.Format(L"0x%08X (%d)", *(PULONG)Value, *(PULONG)Value);
		}
		break;

// 	case REG_LINK:
// 		szRet = L"";
// 		break;

	case REG_MULTI_SZ:
		{
			DWORD len = 0;
			while (wcslen(Data + len))
			{
				szRet += (Data + len);
				szRet += L" ";
				len += wcslen(Data + len) + 1;
			}
		}
		break;

	case REG_QWORD:
		for (ULONG i = 0; i < 8; i++)
		{
			CString szTemp;
			szTemp.Format(L"%02x ", *((PBYTE)Data + i));
			szRet += szTemp;
		}
		break;

	default:
		szRet = L"Unknow";
	}

	return szRet;
}

CString COperateHive::GetValueData (
	IN struct hive *hdesc,
	IN int nkofs, 
	IN char *path,
	IN int type,
	IN int len
	)
/*++

Routine Description:
 得到键值的内容

Arguments:
 path - ValueName. 键的名字

--*/
{    
	void *data ;
	CString string;
	
	if (type == -1 || len <= 0) 
	{
//		printf("No such value <%s>\n",path);
		return NULL;
	}
	
	data = (void *)get_val_data(hdesc, nkofs, path, 0, TPF_VK);
	if (!data) { return NULL; }

	string = GetKeyData(type, (WCHAR*)data, len);
	return string;
}

void COperateHive::GetType(int nType, WCHAR *szType)
{
	if (!szType)
	{
		return;
	}

	switch (nType)
	{
	case REG_NONE:
		wcscpy(szType, L"REG_NONE");
		break;

	case REG_SZ:
		wcscpy(szType, L"REG_SZ");
		break;

	case REG_EXPAND_SZ:
		wcscpy(szType, L"REG_EXPAND_SZ");
		break;

	case REG_BINARY:
		wcscpy(szType, L"REG_BINARY");
		break;

	case REG_DWORD:
		wcscpy(szType, L"REG_DWORD");
		break;

	case REG_DWORD_BIG_ENDIAN:
		wcscpy(szType, L"REG_DWORD_BIG_ENDIAN");
		break;

	case REG_LINK:
		wcscpy(szType, L"REG_LINK");
		break;

	case REG_MULTI_SZ:
		wcscpy(szType, L"REG_MULTI_SZ");
		break;

	case REG_RESOURCE_LIST:
		wcscpy(szType, L"REG_RESOURCE_LIST");
		break;

	case REG_FULL_RESOURCE_DESCRIPTOR:
		wcscpy(szType, L"REG_FULL_RESOURCE_DESCRIPTOR");
		break;

	case REG_RESOURCE_REQUIREMENTS_LIST:
		wcscpy(szType, L"REG_RESOURCE_REQUIREMENTS_LIST");
		break;

	case REG_QWORD:
		wcscpy(szType, L"REG_QWORD");
		break;

	default:
		wcscpy(szType, L"unknow");
	}
}

BOOL 
COperateHive::ListValues_hive_intenal (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath
	)
/*++

Routine Description:
  遍历当前键的键值

Arguments:
  NormalRegPath - 为""默认,为"\\"表示根键,为其他则是指定的键(eg:360\\XGB\\sudami)

--*/
{
	BOOL bRet = FALSE;
	struct nk_key *key;
	int nkofs;
	struct vex_data vex;
	int count = 0 ;
	char valb[2046] = {0};
	
	nkofs = trav_path( hdesc, hdesc->rootofs + 4, szSubKeyPath, TPF_NK );
	if ( !nkofs ) 
	{
		return bRet;
	}
	nkofs += 4;
	
	key = (struct nk_key *)(hdesc->buffer + nkofs);
	if (key->id != 0x6b6e)
	{
// 		printf("Error: Not a 'nk' node!\n");
// 		debugit( hdesc->buffer, hdesc->size );
		return bRet;
	}
	
// 	printf("Node has %ld subkeys and %ld values",key->no_subkeys, key->no_values);
// 	if (key->len_classnam) { printf(", and class-data of %d bytes", key->len_classnam); }
// 	printf("\n");
	
	// 先随便插入一个“默认的键值”
	m_pList->InsertItem(0, szDefault[g_enumLang], 0);
	m_pList->SetItemText(0, 1, L"REG_SZ");
	m_pList->SetItemText(0, 2, szValueNoSet[g_enumLang]);

	if (key->no_values) 
	{
		count = 0;
	//	printf("-------- SelfValue Lists --------\n");
		while ( (ex_next_v( hdesc, nkofs, &count, &vex ) > 0) ) 
		{
			char *name = NULL;
			int nItem = 0;

			if (!*vex.name) 
				name = "@";
			else 
			{
				name = vex.name;
				nItem = m_pList->GetItemCount();
			}
 			
			int nImage = 1;
			if (vex.type == REG_SZ ||
				vex.type == REG_EXPAND_SZ ||
				vex.type == REG_MULTI_SZ)
			{
				nImage = 0;
			}

			// 如果Item不为0，那么就插入，否则只是设置。
			if (nItem)
			{
				CString szName = *vex.name ? ms2ws(vex.name) : szDefault[g_enumLang];
				m_pList->InsertItem(nItem, szName, nImage);
			}
			
			WCHAR szType[MAX_PATH] = {0};
			GetType(vex.type, szType);
			m_pList->SetItemText(nItem, 1, szType);
			
			CString szData = GetValueData( hdesc, nkofs, name, vex.type, vex.size );
			m_pList->SetItemText(nItem, 2, szData);
			
			if (!*vex.name) 
			{
				m_pList->SetItemData(nItem, 1);
			}

			FREE(vex.name);
		}
	}

	return TRUE;
}


BOOL COperateHive::ListValues_hive (IN char *s)
{

	char szSubKeyPath[2048] = {0} ;
	hive *pHive = NULL;
	
	m_pList->DeleteAllItems();

	int nRtnType = IsHiveOpen( s, szSubKeyPath, 0, &pHive );
 	if ( ReturnType_OK != nRtnType || pHive == NULL) 
	{ 
		return FALSE; 
	}

	// 通过解析hive来枚举该键的所有值
	FlushFileBuffers( pHive->hFileMemory );
	FlushFileBuffers( (HANDLE)pHive->buffer );
	return ListValues_hive_intenal( pHive, szSubKeyPath );
}

#pragma warning(default:4996)  
#pragma warning(default:4244)
#pragma warning(default:4267)