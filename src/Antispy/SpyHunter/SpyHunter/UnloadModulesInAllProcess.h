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


// CUnloadModulesInAllProcess dialog

class CUnloadModulesInAllProcess : public CDialog
{
	DECLARE_DYNAMIC(CUnloadModulesInAllProcess)

public:
	CUnloadModulesInAllProcess(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUnloadModulesInAllProcess();

// Dialog Data
	enum { IDD = IDD_UNLOAD_MODULES_IN_ALL_PROCESS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_szImage;
};
