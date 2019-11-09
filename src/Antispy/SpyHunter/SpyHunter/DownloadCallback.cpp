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
#include "DownloadCallback.h"


CDownloadCallback::CDownloadCallback() : m_pDlg(NULL)
{
	m_bStop = FALSE;
// 	InitializeCriticalSection(&m_cs);
}

CDownloadCallback::~CDownloadCallback()
{
// 	DeleteCriticalSection(&m_cs);
}

HRESULT CDownloadCallback::OnProgress ( ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText )
{
// 	EnterCriticalSection(&m_cs);
	if (m_bStop)
	{
		return E_ABORT;
	}
// 	LeaveCriticalSection(&m_cs);
	
	if (m_pDlg)
	{
		m_pDlg->ProgressUpdate ( ulProgressMax, ulProgress );
		return S_OK;
	}
	
	return E_FAIL;
}