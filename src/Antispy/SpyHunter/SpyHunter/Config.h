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
#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <list>

class CConfig
{
public:
	CConfig();
	virtual ~CConfig();
	void WriteConfig();
	void ReadConfig();
	
	// 读取函数
	void GetAllRegPath(std::list <CString> &RegPathList);
	CString GetTitle()	{ return m_szTitle; }
	BOOL GetStayOnTop() { return m_bStayOnTop; }
	BOOL GetRandomTitle() { return m_bRandomTitle; }
	BOOL GetAutoDetectNewVersion() { return m_bAutoDetectNewVersion; }
	CHAR GetHotKeyForShowMainWiondow() { return m_chHotKeyForMainWindow; }
	BOOL GetShowProcessDetailInfo() { return m_bShowProcessDetailInfo; }
	BOOL GetShowProcessTreeAsDefault() { return m_bShowProcessTree; }
	BOOL GetShowLowerPaneAsDefault() { return m_bShowLowerPane; }
	BOOL GetSelfProtection() { return m_bSlefProtection; }
	BOOL GetEnableHotkeys() { return m_bUseHotKeys; }

	COLORREF GetHideProcColor();
	COLORREF GetMicrosoftProcColor();
	COLORREF GetNormalProcColor();
	COLORREF GetMicroHaveOtherModuleProcColor();

	COLORREF GetHiddenFileColor();
	COLORREF GetNormalFileColor();
	COLORREF GetSystemFileColor();
	COLORREF GetSystemAndHiddenFileColor();
	
	COLORREF GetNormalModuleColor();
	COLORREF GetHiddenOrHooksModuleColor();
	COLORREF GetNoSignatureModuleColor();
	COLORREF GetMicrosoftModuleColor();

	// 设置函数
	void AddRegPath(CString szRegPath);
	void SetTitle(CString szTitle) { m_szTitle = szTitle; }
	void SetStayOnTop(BOOL bStay) { m_bStayOnTop = bStay; }
	void SetRandomTitle(BOOL bRandom) { m_bRandomTitle = bRandom; }
	void SetAutoDetectNewVersion(BOOL bAtuto) { m_bAutoDetectNewVersion = bAtuto; }
	void SetHotKeyForShowMainWiondow(CHAR cChar) { m_chHotKeyForMainWindow = cChar; }
	void SetShowProcessDetailInfo(CHAR cChar) { m_bShowProcessDetailInfo = cChar; }
	void SetShowProcessTreeAsDefault(BOOL cChar) { m_bShowProcessTree = cChar; }
// 	void SetShowLowerPaneAsDefault(BOOL cChar) { m_bShowLowerPane = cChar; }
	void SetHideProcColor(COLORREF clr) {m_clrHideProc = clr;}
	void SetMicrosoftProcColor(COLORREF clr) {m_clrMicrosoftProc = clr;}
	void SetNormalProcColor(COLORREF clr) {m_clrNormalProc = clr;}
	void SetMicroHaveOtherModuleProcColor(COLORREF clr) {m_clrMicroHaveOtherModuleProc = clr;}
	void SetHiddenFileColor(COLORREF clr) {m_clrHideFile = clr;}
	void SetNormalFileColor(COLORREF clr) {m_clrNormalFile = clr;}
	void SetSystemFileColor(COLORREF clr) {m_clrSystemFile = clr;}
	void SetSystemAndHiddenFileColor(COLORREF clr) {m_clrSystemAndHideFile = clr;}
	void SetNormalModuleColor(COLORREF clr) {m_NormalItemClr = clr;}
	void SetHiddenOrHooksModuleColor(COLORREF clr) {m_HiddenOrHookItemClr = clr;}
	void SetNoSignatureModuleColor(COLORREF clr) {m_NotSignedItemClr = clr;}
	void SetMicrosoftModuleColor(COLORREF clr) {m_MicrosoftItemClr = clr;}
	void SetSelfProtection(BOOL bSet) { m_bSlefProtection = bSet; }
	void SetEnableHotkeys(BOOL bSet) { m_bUseHotKeys = bSet; }

private:
	CString ms2ws(LPCSTR szSrc, int cbMultiChar=-1);
	CStringA ws2ms(LPWSTR szSrc, int cbMultiChar=-1);
	void InitRegPathList();
	void EraseSpace(CString &szString);
	CString GetConfigFilePath();
	CHAR GetFirstChar(CString szString);
	int CString2_int(const CString & str,int radix = 10,int defaultValue = INT_MIN);
	COLORREF CString2_RGB(const CString & str);

private:
	CString m_szTitle;						// 软件运行标题
	std::list <CString> m_RegPathList;		// 注册表的快速定位项
	BOOL m_bStayOnTop;						// 总在最前
	BOOL m_bRandomTitle;					// 随机命名窗口标题
	BOOL m_bAutoDetectNewVersion;			// 软件启动时, 是否自动检测新版本
	BOOL m_bSlefProtection;					// 是否启动自我保护
	BOOL m_bShowProcessDetailInfo;			// 鼠标移到进程名上是否显示详细信息
	CHAR m_chHotKeyForMainWindow;			// 显示主窗口的热键
	BOOL m_bShowLowerPane;					// 默认下方显示进程模块列表
	BOOL m_bShowProcessTree;				// 默认显示进程树
	COLORREF m_clrHideProc;					// 隐藏进程的颜色
	COLORREF m_clrMicrosoftProc;			// 微软的进程颜色
	COLORREF m_clrNormalProc;				// 正常的进程，不是微软进程的颜色
	COLORREF m_clrMicroHaveOtherModuleProc;	// 微软带有非微软模块进程颜色

	COLORREF m_clrHideFile;					// 隐藏文件的颜色
	COLORREF m_clrSystemFile;				// 系统文件颜色
	COLORREF m_clrSystemAndHideFile;		// 系统并且隐藏的文件颜色
	COLORREF m_clrNormalFile;				// 正常文件颜色

	COLORREF m_NormalItemClr;				// 正常项目，非微软模块颜色		
	COLORREF m_HiddenOrHookItemClr;			// 隐藏或者钩子颜色
	COLORREF m_MicrosoftItemClr;			// 微软的项目颜色
	COLORREF m_NotSignedItemClr;			// 没有签名的模块颜色
	BOOL m_bUseHotKeys;						// 是否使用热键
};

#endif