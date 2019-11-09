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
// stdafx.cpp : 只包括标准包含文件的源文件
// SpyHunter.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

WIN_VERSION g_WinVersion = enumWINDOWS_UNKNOW;
CTabCtrl *g_pTab = NULL;

WCHAR szWin32DriverName[32] = {'\\','\\','.','\\','\0'}; //	L"\\\\.\\SpyHunter"
CString g_szVersion = L"2.0";
BOOL g_bLoadDriverOK = FALSE;
CConnectDriver g_ConnectDriver;

CConfig g_Config;
CWnd *g_pServiceDlg = NULL;
CWnd *g_pSetConfigDlg = NULL;
CWnd *g_pRegistryDlg = NULL;
CWnd *g_pFileDlg = NULL;
CWnd *g_pMainDlg = NULL;
CWnd *g_pProcessDlg = NULL;

COLORREF g_NormalItemClr = RGB( 0, 0, 255 );			// 非微软的文件默认显示蓝色
COLORREF g_HiddenOrHookItemClr = RGB( 255, 0, 0 );		// 隐藏或者钩子默认显示红色
COLORREF g_MicrosoftItemClr = RGB(0, 0, 0);				// 微软模块默认显示黑色
COLORREF g_NotSignedItemClr = RGB( 180, 0, 255 );		// 未签名项默认显示紫色

//LdrpHashTable
LIST_ENTRY *g_pLdrpHashTable = NULL; // 用来枚举进程模块

// 菜单图标
CBitmap m_bmExplorer;						
CBitmap m_bmRefresh;
CBitmap m_bmDelete;
CBitmap m_bmSearch;
CBitmap m_bmExport;
CBitmap m_bmShuxing;
CBitmap m_bmCopy;
CBitmap m_bmAntiSpy;
CBitmap m_bmDetalInof;
CBitmap m_bmLookFor;
CBitmap m_bmWindbg;
CBitmap m_bmSign;
CBitmap m_bmReg;
CBitmap m_bmRecover;
CBitmap m_bmOD;

pfnNtQueryInformationProcess NtQueryInformationProcess = NULL;