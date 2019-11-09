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


// CFileRenameDlg 对话框

class CFileRenameDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileRenameDlg)

public:
	CFileRenameDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileRenameDlg();

// 对话框数据
	enum { IDD = IDD_FILE_RENAME_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_szFileName;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CString m_szStaticInputPath;
};
