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


// CRegModifyDlg 对话框

class CRegModifyDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegModifyDlg)

public:
	CRegModifyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegModifyDlg();

// 对话框数据
	enum { IDD = IDD_REG_MODIFY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_szValueNameStatic;
	CString m_szValueName;
	CString m_szValueDataStatic;
	CString m_szValueData;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
