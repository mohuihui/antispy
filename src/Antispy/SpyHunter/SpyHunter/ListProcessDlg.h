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
#include "afxcmn.h"
#include "ConnectDriver.h"
#include "Function.h"
#include "SignVerifyDlg.h"
#include "..\\..\\Common\Common.h"
#include <list>
#include <vector>
#include "ProcessFunc.h"
#include "afxwin.h"
#include "ColumnTreeCtrl.h"
#include "xySplitterWnd.h"
#include "ListProcess.h"
#include "DllModuleDlg.h"
#include "HandleDlg.h"
#include "ThreadDlg.h"
#include "ProcessMemoryDlg.h"
#include "ProcessWndDlg.h"
#include "HotKeyDlg.h"
#include "ProcessTimerDlg.h"
#include "ProcessPrivilegesDlg.h"

using namespace std;

typedef enum _BELOW_DLG_INDEX_
{
	eDllModuleDlg,
	eHandleDlg,
	eThreadDlg,
	eMemoryDlg,
	eWndDlg,
	eHotKeyDlg,
	eTimerDlg,
	ePrivilegeDlg,
}BELOW_DLG_INDEX;

typedef struct _PROCESS_INFO_EX_
{
	ULONG ulPid;
	ULONG ulParentPid;
	ULONG ulEprocess;
	ULONG nIconOrder; // 进程图标的索引
	CString szPath;
	BOOL bInsert;
	ULONGLONG nCreateTime;
	HTREEITEM hItem;
}PROCESS_INFO_EX, *PPROCESS_INFO_EX;

// CListProcessDlg 对话框

class CListProcessDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CListProcessDlg)

public:
	CListProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CListProcessDlg();

// 对话框数据
	enum { IDD = IDD_PROCESS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL GetRing3ProcessList();
	VOID ListProcess();
	afx_msg void OnNMCustomdrawProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcessRefresh();
	BOOL IsProcessHide(ULONG pid);
	BOOL CanOpenProcess(DWORD dwPid);
	afx_msg void OnCheckSign();
	afx_msg void OnCheckAllSign();
	BOOL IsNotSignItem(ULONG nItem);
	afx_msg void OnKillProcess();
	afx_msg void OnKillDelete();
	afx_msg void OnSuspendProcess();
	afx_msg void OnResumeProcess();
	afx_msg void OnCopyProcessName();
	afx_msg void OnCopyProcessPath();
	afx_msg void OnCheckAttribute();
	afx_msg void OnLocationExplorer();
	afx_msg void OnSearchOnline();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowAllProcessInfo();
	afx_msg LRESULT VerifySignOver(WPARAM wParam, LPARAM lParam);  
	afx_msg void OnExportExcel();
	afx_msg void OnExportText();
	afx_msg void OnDumpProcessMemory();
	afx_msg void OnProcessFindModule();
	afx_msg void OnProcessFindUnsignedModues();
	SUSPEND_OR_RESUME_TYPE ProcessMenuResumeOrSuspend();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMDblclkProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcessHook();
	afx_msg void OnCreateMinidump();
	afx_msg void OnCreateFulldump();
	BOOL CreateDump(DWORD dwFlag, CString szPath);
	CString CreateOpenFileDlg();
	CString GetParentProcessInfo(DWORD dwParentPid);
	void ShowProcessAsList();
	void ShowProcessAsTreeList();
	VOID SortByParentPid(vector<PROCESS_INFO> &ProcessListEx);
	BOOL IsHaveParent(PROCESS_INFO_EX info);
	void InsertTreeProcessChildItem(PROCESS_INFO_EX* pItem, HTREEITEM hRoot);
	afx_msg void OnRclickedColumntree(LPNMHDR pNMHDR, LRESULT* pResult);
 	HTREEITEM GetTreeListSelectItem();
	void DblclkColumntree(HTREEITEM hItem);
	afx_msg void OnShowProcessTree();
	afx_msg void OnUpdateShowProcessTree(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnKillProcessTree();
	void KillSubProcess(DWORD dwPid);
	afx_msg void OnInjectDll();
	void AdjustShowWindow();
	afx_msg void OnLvnItemchangedProcessList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedTreeList(NMHDR *pNMHDR, LRESULT *pResult);
	void MouseOverListCtrl(CPoint point);
	int ListHitTestEx(CPoint& point, int* nCol);
	HTREEITEM ListTreeHitTestEx(CPoint& point, int* nCol);
	void MouseOverCtrl(CPoint point, HWND hWnd);
	afx_msg LRESULT OnClickListHeaderStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumTimer(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumModules(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumHandles(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumThreads(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumMemory(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumWindows(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumHotKeys(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MsgEnumPrivileges(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClickListHeaderEnd(WPARAM wParam, LPARAM lParam);
	BOOL IsHaveNotMicrosoftModule(DWORD dwPID);
	CString FileTime2SystemTime(ULONGLONG ulCreateTile);
	afx_msg void OnProcessLocationAtFileManager();
	afx_msg LRESULT OnUpdateProcessInfoStatus(WPARAM wParam, LPARAM lParam);  
	SUSPEND_OR_RESUME_TYPE IsSuspendOrResumeProcessTree();
	void AddProcessItem(PROCESS_INFO item);
	afx_msg void OnLookWindowForProcess();
	void GotoProcess(DWORD dwPid);
	void GotoThread(DWORD dwPid, DWORD dwTid);
	void KillProcessByFindWindow(DWORD dwPid);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	void InitProcInfoWndTab(CDialog *pDlg);
	void UpdateBelowDlg();
	LRESULT ResizeAllProcWnd(WPARAM wParam, LPARAM lParam);
	PPROCESS_INFO GetProcessInfoByItem(int nItem);
	PPROCESS_INFO_EX GetProcessInfoExByItem(HTREEITEM nItem);
	void TransferProcessInfo2ProcessInfoEx();
	void AddProcessTreeItem(PROCESS_INFO_EX* pInfo, HTREEITEM hItem);
	SUSPEND_OR_RESUME_TYPE GetProcessSuspendOrResumeStatus(ULONG nPid, ULONG pEprocess);
	void ShowProcessListDetailInfo(int nItem);
	void ShowProcessTreeDetailInfo(HTREEITEM hItem);
	void EnumProcessModules(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessHandles(ULONG nPid, ULONG pEprocess, CString szImage);
	afx_msg void OnNMClickListProcess(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickTreeList(NMHDR *pNMHDR, LRESULT *pResult);
	void EnumProcessThreads(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessMemorys(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessWnds(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessHotKeys(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessTimers(ULONG nPid, ULONG pEprocess, CString szImage);
	void EnumProcessPrivileges(ULONG nPid, ULONG pEprocess, CString szImage);
	CString GetCurrentProcessInfo(ULONG &nPid, ULONG &pEprocess);
public:	
	CString m_szProcessCnt;						// 进程下方状态,显示进程数量等信息
	CImageList m_ProImageList;					// 进程Image列表
	list<ULONG> m_Ring3ProcessList;				// 进程Ring3枚举得到的list,只保存pid,用来比较哪些是隐藏的进程
	CSignVerifyDlg m_SignVerifyDlg;				// 验证签名的对话框类
	vector<PROCESS_INFO> m_Ring0ProcessList;	// R0枚举上来的进程信息列表
	CConnectDriver m_Driver;					// 与驱动通信的类
	CommonFunctions m_Functions;				// 常用的API类
	CTabCtrl m_tab;								// 下面的Tab控件
	HACCEL m_hAccel;							// 快捷键句柄
	HICON m_hProcessIcon;						// 没有与之对应的进程Image,就设置一个默认的Ico句柄
	CProcessFunc m_ProcessFunc;					// 常用的进程操作类函数, eg.内核打开进程
	CImageList m_imgList;						// 进程树形控件与之对应的Image列表
	vector<PROCESS_INFO_EX> m_ProcessListEx;	// 与树形控件相关的进程列表信息
	BOOL m_bShowAsTreeList;						// 是否以树形控件的形式显示
	CSortListCtrl m_processList;				// 待底部模块列表的进程列表
	CColumnTreeCtrl m_processTree;				// 带下方模块列表的进程树形控件
	ULONG m_nProcessCnt;						// 总共的进程数量						
	ULONG m_nHideCnt;							// 隐藏进程的数量
	ULONG m_nDenyAccessCnt;						// R3无法打开的进程数量
	CXTPTipWindow m_tipWindow;					// 用来显示进程详细信息的气泡
	BOOL m_bShowTipWindow;						// 是否显示进程详细信息的气泡
	COLORREF m_clrHideProc;						// 隐藏进程的颜色
	COLORREF m_clrMicrosoftProc;				// 微软的进程颜色
	COLORREF m_clrNormalProc;					// 正常的进程，不是微软进程的颜色
	COLORREF m_clrMicroHaveOtherModuleProc;		// 微软带有非微软模块进程颜色
	COLORREF m_clrNoSigniture;					// 没有数字签名的进程颜色
	vector<ITEM_COLOR_INFO> m_vectorColor;		// 颜色列表
	vector<PROCESS_INFO> m_vectorProcesses;		// 在列表框中显示的进程列表
	CySplitterWnd m_ySplitter;					// 分割条
	CListProcess m_clsProcess;					// 进程类
	BOOL m_bShowBelow;							// 是否需要显示下方数据，当点击列表头的时候是不需要显示的
	vector<PROCESS_INFO_EX> m_vectorProcessEx;	// 与树形控件相关的进程列表信息
	CDialog *m_pBelowDialog[20];				// 下面的窗口
	ULONG m_nBelowDialog;						// 总共有几个窗口
	BELOW_DLG_INDEX m_nCurSel;					// 下面的窗口现在选的是哪
	CDllModuleDlg m_ModuleDlg;					// 下面的模块对话框
	CHandleDlg m_HandleDlg;						// 下面的句柄对话框
	CThreadDlg m_ThreadDlg;						// 下面的线程对话框
	CProcessMemoryDlg m_MemoryDlg;				// 下面的内存对话框
	CProcessWndDlg	m_WndDlg;					// 下面的窗口对话框
	CHotKeyDlg	m_HotKeyDlg;					// 下面的热键对话框
	CProcessTimerDlg m_TimerDlg;				// 下面的定时器对话框
	CProcessPrivilegesDlg m_PrivilegeDlg;		// 下面的权限对话框
};