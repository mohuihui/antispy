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
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 头中排除极少使用的资料
#endif

// 如果您必须使用下列所指定的平台之前的平台，则修改下面的定义。
// 有关不同平台的相应值的最新信息，请参考 MSDN。
#ifndef WINVER				// 允许使用特定于 Windows XP 或更高版本的功能。
#define WINVER 0x0501		// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用特定于 Windows 98 或更高版本的功能。
#define _WIN32_WINDOWS 0x0410 // 将它更改为适合 Windows Me 或更高版本的相应值。
#endif

#ifndef _WIN32_IE			// 允许使用特定于 IE 6.0 或更高版本的功能。
#define _WIN32_IE 0x0600	// 将此值更改为相应的值，以适用于 IE 的其他版本。值。
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxdlgs.h>

///////////////////////////////////////////////////////////////////////

#include "Config.h"
#include <shlwapi.h>
#include "common.h"
#include "ConnectDriver.h"

typedef enum WIN_VERSION {
	enumWINDOWS_UNKNOW,
	enumWINDOWS_2K,
	enumWINDOWS_XP,
	enumWINDOWS_2K3,
	enumWINDOWS_2K3_SP1_SP2,
	enumWINDOWS_VISTA,
	enumWINDOWS_VISTA_SP1_SP2,
	enumWINDOWS_7,
	enumWINDOWS_8
} WIN_VERSION;

extern WIN_VERSION g_WinVersion;
extern CConnectDriver g_ConnectDriver;
extern CTabCtrl *g_pTab;
extern CWnd *g_pRegistryDlg;
extern CWnd *g_pFileDlg;
extern CWnd *g_pServiceDlg;
extern WCHAR szWin32DriverName[32];
extern CString g_szVersion;
extern BOOL g_bLoadDriverOK;
extern CConfig g_Config;
extern LIST_ENTRY *g_pLdrpHashTable;
extern CWnd *g_pSetConfigDlg;			// 设置配置文件的对话框
extern CWnd *g_pMainDlg;
extern CWnd *g_pProcessDlg;

extern COLORREF g_NormalItemClr;
extern COLORREF g_HiddenOrHookItemClr;
extern COLORREF g_MicrosoftItemClr;	
extern COLORREF g_NotSignedItemClr;

extern CBitmap m_bmExplorer;						
extern CBitmap m_bmRefresh;
extern CBitmap m_bmDelete;
extern CBitmap m_bmSearch;
extern CBitmap m_bmExport;
extern CBitmap m_bmShuxing;
extern CBitmap m_bmCopy;
extern CBitmap m_bmAntiSpy;
extern CBitmap m_bmDetalInof;
extern CBitmap m_bmLookFor;
extern CBitmap m_bmWindbg;
extern CBitmap m_bmSign;
extern CBitmap m_bmReg;
extern CBitmap m_bmRecover;
extern CBitmap m_bmOD;

extern pfnNtQueryInformationProcess NtQueryInformationProcess;

#ifdef _DEBUG
#define _TRACK
#endif

#define _OUTPUT_STRHEADER	_T("AntiSpy: ")
#define _0X_LEN				wcslen(L"0x")
#define	_0X08X				L"0x%08X"

#define _CSTDIO_
#define _CSTRING_
#define _CWCHAR_

#pragma   warning(disable:4311)
#pragma   warning(disable:4312)
#pragma   warning(disable:4267)

#include "EasySize.h"
#include "String.h"
#include "CommonMacro.h"
#include "sortlistctrl.h"

// #define _XTP_EXCLUDE_CALENDAR
// #define _XTP_EXCLUDE_SYNTAXEDIT
// #define _XTP_EXCLUDE_REPORTCONTROL
// 

///////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <XTToolkitPro.h>   // Codejock Software Components