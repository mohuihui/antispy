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
#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include "FindReg.h"
#include "ConnectDriver.h"
#include "resource.h"
#include "SignVerifyDlg.h"
#include "..\\..\\Common\Common.h"
#include <set>
using namespace std;

class CommonFunctions
{
public:
	CommonFunctions();
	virtual ~CommonFunctions();
	BOOL IsMicrosoftApp(CString szCompany);
	CString GetFileCompanyName(CString szPath);
	CString GetFileDescription(CString szPath);
	BOOL SetStringToClipboard(CString szImageName);
	void LocationExplorer(CString szPath);
	void OnCheckAttribute(CString szPath);
	void BaiduSearch(CString szImageName);
	void GoogleSearch(CString szImageName);
	void ExportListToExcel(CSortListCtrl* pList, CString strTitle, CString szStatus);
	void ExportListToTxt(CSortListCtrl* pList, CString szStatus);
	CString TrimPath(WCHAR * szPath);
	int GetSelectItem(CSortListCtrl *pList);
	CString GetLongPath(CString szPath);
	VOID FindReg(CString szKey, CString szValue);
	ULONG HexStringToLong(CString szHex);
	ULONG GetInlineAddress(ULONG pAddress);
	void SignVerify(CString szPath);
	BOOL IsMicrosoftAppByPath(CString szPath);
	BOOL InitUnicodeString(MZF_UNICODE_STRING *unString, WCHAR *szString);
	void FreeUnicodeString(MZF_UNICODE_STRING *unString);
	void JmpToMyRegistry(CString szKey, CString szData);
	BOOL KernelDeleteFile(CString szPath);
	void Disassemble(DWORD dwAddress, DWORD dwSize = 100, DWORD dwPid = 0, DWORD dwEprocess = 0);
	CString DisassembleString(DWORD dwAddress, DWORD dwSize = 100, DWORD dwPid = 0, DWORD dwEprocess = 0);
	CString ms2ws(LPCSTR szSrc, int cbMultiChar=-1);
	void ExportAutoRunValue(CString szKey, CString szPath, CString szValue);
	void HexEdit(DWORD dwAddress, DWORD dwSize, DWORD dwPid, DWORD dwEprocess);
	void JmpToService(CString szService);
	void JmpToFile(CString szFilePath);
	void SearchOnline(CString szImageName);
	CString RegParsePath(WCHAR *szData, ULONG dwType);
	CString RegParsePath(CString szPath);
	void DecryptResource(PVOID pBuffer, DWORD dwSize);
	BOOL EnablePrivilege(WCHAR *szPrivilege, BOOL bEnable);
	CString ReleaseResource(DWORD dwRid, WCHAR *szType);
	WCHAR *CString2WString(CString szString);

private:
	CString GetExcelDriver();
	BOOL GetDefaultXlsFileName(CString& sExcelFile);
	BOOL GetDefaultTxtFileName(CString& sExcelFile);
	DWORD GetChecksum(PVOID pBase,DWORD FileSize);
	CConnectDriver m_Driver;
};

PVOID GetMemory(ULONG nSize);
CString TrimPath(WCHAR * szPath);
WCHAR *CString2WString(CString szString);
void FixPeDump(PVOID pBuffer);
CString PathGetImageName(CString szPath);
#endif
