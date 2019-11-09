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

#include "ntreg.h"
#include "InitHive.h"


typedef struct _HIVE_INFOS {
	struct hive *pHive_HKLM_SAM;
	struct hive *pHive_HKLM_SECURITY;
	struct hive *pHive_HKLM_SOFTWARE;
	struct hive *pHive_HKLM_SYSTEM;
	struct hive *pHive_HKCU;

	// 其他用户,eg. .DEFAULT
	struct hive *pHive_OTHER_USER[20];

	// 其他的Hive文件, eg. BCD00000000
	struct hive *pHive_OTHER[10];

} HIVE_INFOS, *PHIVE_INFOS;

typedef enum _HIVE_OPEN_TYPE
{
	ReturnType_IsNoting,
	ReturnType_IsRootKey,
	ReturnType_ERRO,
	ReturnType_OK
} HIVE_OPEN_TYPE;

class COperateHive
{
public:
	COperateHive();
	virtual ~COperateHive();

	HIVE_OPEN_TYPE IsHiveOpen(IN char *szKeyPath, IN char *subKeyPath, IN int checkType, hive **pHive);
	BOOL ListSubKeys_hive (IN char *s );
	void SetTree(CTreeCtrl *pTree);
	void SetItem(HTREEITEM Item) { m_Item = Item; }
	void SetList(CListCtrl *pList) { m_pList = pList; }
	BOOL ListValues_hive (IN char *s);

private:
	int GetUserName (IN char* szOutUserName);
	int get_Key_s_subkey_Counts ( IN struct hive *hdesc, IN char *path, IN int vofs, IN int type );
	BOOL nk_ls_ListSubKeys ( IN struct hive *hdesc, IN char* szSubKeyPath);
	CString ms2ws(LPCSTR szSrc, int cbMultiChar = -1);
	BOOL ListValues_hive_intenal ( IN struct hive *hdesc, IN char* szSubKeyPath );
	void GetType(int nType, WCHAR *szType);
	CString GetValueData ( IN struct hive *hdesc, IN int nkofs, IN char *path, IN int type, IN int len ) ;
	CString GetKeyData(ULONG Type, WCHAR *Data, ULONG DataLength);
private:
	CTreeCtrl *m_pTree;
	HTREEITEM m_Item;
	CListCtrl *m_pList;
	HIVE_INFOS m_HiveInfos;
};