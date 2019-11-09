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
#include <vector>
#include <Tlhelp32.h> 

using namespace std;

class CProcessInfo 
{ 
public: 
	PROCESSENTRY32 m_pe32; 
	std::vector <MODULEENTRY32> m_me32; 

	static std::vector <CProcessInfo> GetProcessList() 
	{ 
		std::vector <CProcessInfo> vecProcessInfo; 
		HANDLE hHandle = NULL; 
		PROCESSENTRY32 pe32; 
		pe32.dwSize = sizeof(PROCESSENTRY32); 
		hHandle = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 ); 
		if (hHandle != INVALID_HANDLE_VALUE) 
		{ 
			if ( Process32First(hHandle, &pe32) ) 
			{ 
				pe32.dwSize = sizeof(PROCESSENTRY32); 
				do 
				{ 
					CProcessInfo pi; 
					pi.m_pe32=pe32; 
					GetProcessModule(pe32.th32ProcessID,pi.m_me32); 
					vecProcessInfo.push_back(pi); 
				} 
				while (Process32Next(hHandle, &pe32)); 
			} 
			CloseHandle(hHandle); 
		} 
		return vecProcessInfo; 
	} 

	//获得模块信息 
	static VOID GetProcessModule (DWORD dwPID,std::vector <MODULEENTRY32> &vecME32) 
	{ 
		//系统空闲进程 
		if (dwPID==0) 
			return; 

		HANDLE hModuleSnap = NULL; 
		MODULEENTRY32 me32 = {0}; 
		me32.dwSize = sizeof(MODULEENTRY32); 
		hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID); 
		if (hModuleSnap != INVALID_HANDLE_VALUE) 
		{ 
			if (Module32First(hModuleSnap, &me32)) 
			{ 
				do 
				{ 
					vecME32.push_back(me32); 
				} 
				while (Module32Next(hModuleSnap, &me32)); 
			} 

			CloseHandle (hModuleSnap); 
		}
	} 
};

class CJumpToRegistry 
{ 
public: 
	static void StringToVector(CString strSource, 
		std::vector <CString> &vec, 
		std::vector <CString> &strSpaces, 
		BOOL bIgnoreNULLString=TRUE ) 
	{ 
		vec.clear(); 
		CString strTmp = strSource; 

		for (;;) 
		{ 
			int idx=-1; 
			CString strFind; 

			std::vector <CString> ::iterator itSpaces; 
			for (itSpaces=strSpaces.begin(); itSpaces!=strSpaces.end(); ++itSpaces) 
			{ 
				int idxOneSpace = strTmp.Find(*itSpaces,0); 
				if (idxOneSpace!=-1) 
				{ 
					if (idx==-1 || idxOneSpace <=idx) 
					{ 
						idx=idxOneSpace; 
						strFind=*itSpaces; 
					} 
				} 
			} 

			if (idx == -1) 
				break; 

			CString strTmp2 = strTmp; 
			strTmp2.Delete(idx,strTmp2.GetLength()-idx); 
			if (bIgnoreNULLString) 
			{ 
				if (strTmp2.GetLength()> 0) 
					vec.push_back(strTmp2); 
			} 
			else 
			{ 
				vec.push_back(strTmp2); 
			} 

			strTmp.Delete(0,idx+strFind.GetLength()); 
		} 

		if (bIgnoreNULLString) 
		{ 
			if (strTmp.GetLength()> 0) 
				vec.push_back(strTmp); 
		} 
		else 
		{ 
			vec.push_back(strTmp); 
		} 
	} 

	static BOOL CALLBACK FindRegeditTreeView(HWND hwnd,LPARAM lParam) 
	{
		TCHAR szClass[255]={0}; 
		GetClassName(hwnd,szClass,255); 
		if (lstrcmpi(szClass,_T( "SysTreeView32"))==0) 
		{ 
			HWND *pwnd=(HWND*)lParam; 
			*pwnd=hwnd; 
			return FALSE; 
		} 

		return TRUE; 
	} 

	static BOOL CALLBACK FindRegeditListView(HWND hwnd,LPARAM lParam) 
	{
		TCHAR szClass[255]={0}; 
		GetClassName(hwnd,szClass,255); 
		if (lstrcmpi(szClass,_T( "SysListView32"))==0) 
		{ 
			HWND *pwnd=(HWND*)lParam; 
			*pwnd=hwnd; 
			return FALSE; 
		} 

		return TRUE; 
	} 

	static BOOL CALLBACK FindRegeditMainWnd(HWND hwnd,LPARAM lParam) 
	{ 
		std::vector <DWORD> *pv=(std::vector <DWORD> *)lParam; 
		DWORD dwProcessID=0; 
		GetWindowThreadProcessId(hwnd,&dwProcessID); 
		TCHAR szClass[255]={0}; 
		GetClassName(hwnd,szClass,255); 
		if (dwProcessID==(*pv)[0] && 
			lstrcmpi(szClass,_T( "regedit_regedit"))==0 ) 
		{
			HWND *pwnd=(HWND*)((*pv)[1]); 
			*pwnd=hwnd; 
			return FALSE; 
		}

		return TRUE; 
	} 

	static BOOL JumpToRegistry(CString lpszPath, CString lpszValue) 
	{ 
		CWaitCursor wc; 
		CString strKeyPath(lpszPath); 
		CString strValName(lpszValue); 
		CString strCmd = _T( "regedit.exe"); 
		DWORD dwProcessID = 0; 
		CProcessInfo cpi; 
		std::vector <CProcessInfo> vProcessList = cpi.GetProcessList(); 

		for(ULONG ip = 0; ip < vProcessList.size(); ++ip) 
		{ 
			PROCESSENTRY32 pe32 = vProcessList[ip].m_pe32; 
			if (lstrcmpi(pe32.szExeFile, _T( "regedit.exe")) == 0) 
			{ 
				dwProcessID = pe32.th32ProcessID; 
				break; 
			} 
		} 

		if (dwProcessID == 0) 
		{ 
			STARTUPINFO si; 
			PROCESS_INFORMATION pi; 
			ZeroMemory( &si, sizeof(si) ); 
			si.cb = sizeof(si); 
			ZeroMemory( &pi, sizeof(pi) ); 
			if(CreateProcess( NULL, 
				strCmd.GetBuffer(strCmd.GetLength()+1),
				NULL, 
				NULL,
				FALSE, 
				0, 
				NULL, 
				NULL, 
				&si,
				&pi )
				) 
			{ 
				dwProcessID = pi.dwProcessId; 
				CloseHandle( pi.hProcess ); 
				CloseHandle( pi.hThread ); 
			} 
		} 

		if (dwProcessID==0) 
			return FALSE; 

		//得到regedit的主窗口 
		HWND hTop=NULL; 
		std::vector <DWORD> vParams; 
		vParams.push_back(dwProcessID); 
		vParams.push_back((DWORD)&hTop); 

		for(int i=0; i <10; ++i) 
		{ 
			EnumWindows(FindRegeditMainWnd,(LPARAM)&vParams); 
			Sleep(300); 
			if (hTop) 
				break; 
		} 

		if (hTop==NULL) 
			return FALSE; 

		//得到TREEVIEW 
		HWND hTree=NULL; 
		{ 
			EnumChildWindows(hTop,FindRegeditTreeView,(LPARAM)&hTree); 
		} 

		if (hTree==NULL) 
		{
			MessageBox(NULL, L"find tree ERROR", L"F", 0);
			return FALSE; 
		}

		//得到listview 
		HWND hList=NULL; 
		{ 
			EnumChildWindows(hTop,FindRegeditListView,(LPARAM)&hList); 
		} 

		if (hList==NULL) 
			return FALSE; 

		//选中ROOT结点 
		::SendMessage(hTree,WM_SETFOCUS,0,0); 
		::SendMessage(hTree,WM_KEYDOWN,VK_HOME,0); 

		//展开TREE 
		std::vector <CString> vSpace; 
		vSpace.push_back(_T( "\\")); 
		std::vector <CString> vKeys; 
		StringToVector(strKeyPath,vKeys,vSpace,TRUE); 

		for (ULONG i = 0; i < vKeys.size(); ++i) 
		{ 
			::SendMessage(hTree,WM_KEYDOWN,VK_RIGHT,0); 
			CString sSelectKey=vKeys[i]; 
			sSelectKey.MakeUpper(); 
			for (int iLetter=0; iLetter <sSelectKey.GetLength(); ++iLetter) 
			{ 
				::SendMessage(hTree,WM_CHAR,sSelectKey.GetAt(iLetter),0); 
			} 
		}

		//LISTVIEW定位 
		::SendMessage(hList,WM_SETFOCUS,0,0); 

		//选中value 
		Sleep(300); 
		::SendMessage(hList,WM_KEYDOWN,VK_HOME,0); 
		CString sSelectVal=strValName; 
		sSelectVal.MakeUpper(); 
		for (int iLetter=0; iLetter <sSelectVal.GetLength(); ++iLetter) 
		{ 
			::SendMessage(hList,WM_CHAR,sSelectVal.GetAt(iLetter),0); 
		} 

		//前端显示 
		::SetWindowPos(hTop,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); 

		return TRUE; 
	} 
};