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
#include "afxwin.h"
#include"resource.h"

// CUpdateDlg 对话框

class CUpdateDlg : public CDialog
{
	DECLARE_DYNAMIC(CUpdateDlg)

public:
	CUpdateDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUpdateDlg();

// 对话框数据
	enum { IDD = IDD_UPDATE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_Progress;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	BOOL CheckForUpdates();
	afx_msg LRESULT OnUpdateData(WPARAM wParam, LPARAM lParam);  
	void ProgressUpdate( ULONG nMaxBytes, ULONG nDoneBytes );
	BOOL OnDownLoad();
	BOOL StartUnpack(CString szPath);
	CString m_szUpdateLogEdit;  // 总的输出给用户的信息
	CButton m_btnUpdate;    
	BOOL m_bUpdate;			// 是否需要更新
	CString m_szUpdateURL;	// 最新包下载地址
	CString m_szVersion;	// 最新版本信息
	CString m_szUpdateLog;	// 更新日志
	CEdit m_OutputEdit;	
	CString m_szRarPath;	// 最新版本文件下载下来保存的路径，一般是下载到临时文件夹
	CString m_szNewVersionPath;
	BOOL m_bStopDownload;
	afx_msg void OnClose();
	HANDLE m_hDownloadThread;
	HANDLE m_hConnectThread;
};
