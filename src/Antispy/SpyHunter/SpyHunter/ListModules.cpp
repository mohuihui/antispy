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
#include "ListModules.h"
#include <TlHelp32.h>
#include "Function.h"

CListModules::CListModules()
{
	
}

CListModules::~CListModules()
{
	
}

// 
// 根据进程的快照取进程模块
//
BOOL CListModules::EnumModulesBySnapshot(ULONG nPid, vector<MODULE_INFO> &vectorModules)
{
	BOOL bRet = FALSE;
	
	vectorModules.clear();
	
	MODULEENTRY32 me32 = {0}; 
	me32.dwSize = sizeof(MODULEENTRY32); 
	
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nPid); 

	if (hModuleSnap != INVALID_HANDLE_VALUE) 
	{ 
		if (Module32First(hModuleSnap, &me32)) 
		{ 
			do 
			{ 
				MODULE_INFO info;

				info.Base = (ULONG)me32.modBaseAddr;
				info.Size = me32.modBaseSize;
				wcsncpy_s(info.Path, MAX_PATH - 1, me32.szModule, wcslen(me32.szModule));
				
				vectorModules.push_back(info);

			} while (Module32Next(hModuleSnap, &me32)); 
		} 

		CloseHandle (hModuleSnap); 
		bRet = TRUE;
	}

	return bRet;
}

//
//	读取进程的PEB获取进程模块
//
BOOL CListModules::EnumModulesByPeb(ULONG nPid, ULONG pEprocess, vector<MODULE_INFO> &vectorModules)
{
	vectorModules.clear();

	ULONG nCnt = 1000;
	PALL_MODULES pAllModules = NULL;
	BOOL bRet = FALSE;

	COMMUNICATE_PROCESS_MODULE cpm;
	cpm.OpType = enumListProcessModule;
	cpm.nPid = nPid;
	cpm.pEprocess = pEprocess;

	do 
	{
		ULONG nSize = 0;

		if (pAllModules)
		{
			free(pAllModules);
			pAllModules = NULL;
		}

		nSize = sizeof(ALL_MODULES) + nCnt * sizeof(MODULE_INFO);

		pAllModules = (PALL_MODULES)GetMemory(nSize);
		if (!pAllModules)
		{
			break;
		}

		bRet = g_ConnectDriver.CommunicateDriver(&cpm, sizeof(COMMUNICATE_PROCESS_MODULE), pAllModules, nSize, NULL);

		nCnt = pAllModules->nCnt + 100;

	} while (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && pAllModules->nCnt > 0)
	{
		for (ULONG i = 0; i < pAllModules->nCnt; i++)
		{
			MODULE_INFO inf0;

			inf0.Base = pAllModules->Modules[i].Base;
			inf0.Size = pAllModules->Modules[i].Size;
			CString szPath = TrimPath(pAllModules->Modules[i].Path);
			wcsncpy_s(inf0.Path, MAX_PATH, szPath.GetBuffer(), szPath.GetLength());
			szPath.ReleaseBuffer();

			vectorModules.push_back(inf0);

		//	vectorModules.push_back(pAllModules->Modules[i]);
		}
	}

	if (pAllModules)
	{
		free(pAllModules);
		pAllModules = NULL;
	}

	return bRet;
}

//
// 枚举LdrpHashTable链表得到进程模块
//
BOOL CListModules::EnumModulesByLdrpHashTable(ULONG nPid, vector<MODULE_INFO> &vectorModules)
{
	vectorModules.clear();

	if(!g_pLdrpHashTable)
	{
		DebugLog(L"找不到LdrpHashTable");
		return FALSE;
	}

	HANDLE hProcess = m_ProcessFunc.OpenProcess(PROCESS_VM_READ, FALSE, nPid);
	if(hProcess == NULL)
	{
		DebugLog(L"打开 %d 进程出错", nPid);
		return FALSE;
	}
	
	DWORD size = sizeof(LIST_ENTRY) * 32;
	PVOID pTemp = GetMemory(size);

	if (!pTemp)
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	// 读Hash表
	if(!m_ProcessFunc.ReadProcessMemory(hProcess, g_pLdrpHashTable, pTemp, size, &size))
	{
		CloseHandle(hProcess);
		free(pTemp);
		return FALSE;
	}

	LIST_ENTRY *LdrpHashTableTemp = (LIST_ENTRY *)pTemp;
	LIST_ENTRY *pListEntry = NULL, *pListHead = NULL;
	LDR_DATA_TABLE_ENTRY LdrDataEntry;
	
	for(int i = 0; i < 32; i++)
	{
		pListEntry = LdrpHashTableTemp + i;
		pListEntry = pListEntry->Flink;
		pListHead = g_pLdrpHashTable + i;	//指向某hash节点头

		while(pListEntry != pListHead)
		{
			if(!m_ProcessFunc.ReadProcessMemory(hProcess, (BYTE *)pListEntry - 0x3c, &LdrDataEntry, sizeof(LdrDataEntry), &size))
			{
				goto END;
			}
			
			UNICODE_STRING *pDllFullPath = &LdrDataEntry.FullDllName;
			WCHAR *pString = (WCHAR*)GetMemory(pDllFullPath->MaximumLength);
			if (!pString)
			{
				goto END;
			}
			
			if(!m_ProcessFunc.ReadProcessMemory(hProcess, pDllFullPath->Buffer, pString, pDllFullPath->Length, &size))
			{
				free(pString);
				pString = NULL;
				goto END;
			}

			MODULE_INFO info;
			info.Base = (ULONG)LdrDataEntry.DllBase;
			info.Size = LdrDataEntry.SizeOfImage;
			CString szPath = TrimPath(pString);
			wcsncpy_s(info.Path, MAX_PATH, pString, pDllFullPath->MaximumLength / sizeof(WCHAR));

			vectorModules.push_back(info);

			DebugLog(L"base: 0x%08X, size: 0x%X, %s", LdrDataEntry.DllBase, LdrDataEntry.SizeOfImage, pString);

			free(pString);
			pString = NULL;

			pListEntry = LdrDataEntry.HashLinks.Flink; // 通过节点的hashlinks来递进
		}
	}

END:
	if(!LdrpHashTableTemp)
	{
		free(LdrpHashTableTemp);
	}

	CloseHandle(hProcess);

	return TRUE;
}

//
// 卸载进程模块
//
BOOL CListModules::UnloadModule(ULONG nPid, ULONG pEprocess, ULONG nBase)
{
	if (nPid == 0 && pEprocess == 0)
	{
		return FALSE;
	}

	COMMUNICATE_PROCESS_MODULE cpmu;
	cpmu.op.Unload.Base = nBase;
	cpmu.op.Unload.LdrpHashTable = (ULONG)g_pLdrpHashTable;
	cpmu.nPid = nPid;
	cpmu.pEprocess = pEprocess;
	cpmu.OpType = enumUnloadDllModule;

	return g_ConnectDriver.CommunicateDriver(&cpmu, sizeof(COMMUNICATE_PROCESS_MODULE), NULL, 0, NULL);
}

PVOID CListModules::DumpModuleMemory(ULONG nPid, ULONG pEprocess, ULONG nBase, ULONG nSize)
{
	PVOID pBuffer = NULL;

	if ( (nPid == 0 && pEprocess == 0) || nSize == 0 )
	{
		return pBuffer;
	}

	pBuffer = GetMemory(nSize);
	if (!pBuffer)
	{
		return pBuffer;
	}

	COMMUNICATE_PROCESS_MODULE cpmd;

	cpmd.op.Dump.Base = nBase;
	cpmd.op.Dump.Size = nSize;
	cpmd.nPid = nPid;
	cpmd.pEprocess = pEprocess;
	cpmd.OpType = enumDumpDllModule;

	BOOL bRet = g_ConnectDriver.CommunicateDriver(&cpmd, sizeof(COMMUNICATE_PROCESS_MODULE), pBuffer, nSize, NULL);
	if (!bRet)
	{
		free(pBuffer);
		pBuffer = NULL;
	}	

	return pBuffer;
}

//
// 拷贝模块内存
//
BOOL CListModules::DumpModuleMemoryToFile(ULONG nPid, ULONG pEprocess, ULONG nBase, ULONG nSize, CString szModule)
{
	if ( (nPid == 0 && pEprocess == 0) || nSize == 0 )
	{
		return FALSE;
	}

	BOOL bCopy = FALSE;
	WCHAR szFile[MAX_PATH] = {0};
	wcsncpy_s(szFile, MAX_PATH, szModule.GetBuffer(), szModule.GetLength());
	szModule.ReleaseBuffer();

	CFileDialog fileDlg(
		FALSE, 
		0, 
		(LPWSTR)szFile, 
		0, 
		L"(*.exe; *.dll; *.sys; *.oct)|*.exe;*.dll;*.sys;*.oct|All Files (*.*)|*.*||",
		0
		);

	if (IDOK != fileDlg.DoModal())
	{
		return FALSE;
	}
	
	CString szFilePath = fileDlg.GetFileName();
	if ( !PathFileExists(szFilePath) ||
		(PathFileExists(szFilePath) && ::MessageBox(NULL, szFileExist[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES))
	{
		PVOID pBuffer = DumpModuleMemory(nPid, pEprocess, nBase, nSize);
		if (!pBuffer)
		{
			return FALSE;
		}
		
		if (::MessageBox(NULL, szFixDump[g_enumLang], szToolName, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			FixPeDump(pBuffer);
		}

		CFile file;
		TRY 
		{
			if (file.Open(szFilePath, CFile::modeCreate | CFile::modeWrite))
			{
				file.Write(pBuffer, nSize);
				file.Close();
				bCopy = TRUE;
			}
		}
		CATCH_ALL( e )
		{
			file.Abort();   // close file safely and quietly
			//THROW_LAST();
		}
		END_CATCH_ALL

		if (bCopy)
		{
			MessageBox(NULL, szDumpOK[g_enumLang], szToolName, MB_OK);
		}
		else
		{
			MessageBox(NULL, szDumpFailed[g_enumLang], szToolName, MB_OK);
		}

		free(pBuffer);
		pBuffer = NULL;
	}

	return bCopy;
}