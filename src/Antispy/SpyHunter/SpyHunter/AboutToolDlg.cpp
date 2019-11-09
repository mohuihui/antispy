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
// AboutToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SpyHunter.h"
#include "AboutToolDlg.h"
#include "UpdateDlg.h"
#include "ProcessHookDlg.h"
#include <afxinet.h>

// CAboutToolDlg 对话框

IMPLEMENT_DYNAMIC(CAboutToolDlg, CDialog)

CAboutToolDlg::CAboutToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutToolDlg::IDD, pParent)
	, m_szCheckUpdate(_T(""))
	, m_szContactAuthor(_T(""))
	, m_szAuthor(_T(""))
	, m_szEmail(_T(""))
	, m_szSina(_T(""))
	, m_szTencent(_T(""))
	, m_szInformationAndThanks(_T(""))
	, m_szInformation(_T(""))
	, m_szSystem(_T(""))
	, m_szThanks(_T(""))
	, m_szCopyright(_T(""))
	, m_szMianze(_T(""))
	, m_szUpdate(_T(""))
	, m_szWebsite(_T(""))
	, m_szAntiSpyDotCn(_T(""))
	, m_szQQ(_T(""))
{
	m_szVersion = L"";
}

CAboutToolDlg::~CAboutToolDlg()
{
}

void CAboutToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_UPDATE, m_szCheckUpdate);
	DDX_Text(pDX, IDC_STATIC_CONTACT_AUTHOR, m_szContactAuthor);
	DDX_Text(pDX, IDC_STATIC_AUTHOR, m_szAuthor);
	DDX_Text(pDX, IDC_STATIC_EMAIL, m_szEmail);
	DDX_Text(pDX, IDC_STATIC_SINA, m_szSina);
	DDX_Text(pDX, IDC_STATIC_TENCENT, m_szTencent);
	DDX_Text(pDX, IDC_STATIC_INFORMATION_AND_THANKS, m_szInformationAndThanks);
	DDX_Text(pDX, IDC_STATIC_INFORMATION, m_szInformation);
	DDX_Text(pDX, IDC_STATIC_SYSTEM, m_szSystem);
	DDX_Text(pDX, IDC_STATIC_THANKS, m_szThanks);
	DDX_Text(pDX, IDC_STATIC_COPYRIGHT, m_szCopyright);
	DDX_Text(pDX, IDC_STATIC_MIANZE, m_szMianze);
	DDX_Text(pDX, IDC_UPDAGE, m_szUpdate);
	DDX_Text(pDX, IDC_STATIC_WEBSITE, m_szWebsite);
	DDX_Control(pDX, IDC_STATIC_FENGQU, m_FengquHypeLike);
	DDX_Control(pDX, IDC_STATIC_51POJIE, m_52PojieHypeLink);
	DDX_Control(pDX, IDC_STATIC_WEBSITE, m_BolgHypeLink);
	DDX_Control(pDX, IDC_STATIC_SINA, m_SinaHypeLink);
	DDX_Control(pDX, IDC_STATIC_TENCENT, m_TencetHypeLink);
	DDX_Control(pDX, IDC_STATIC_EMAIL, m_EmailHypeLink);
	//	DDX_Control(pDX, IDC_STATIC_UPDATE, m_CheckUpdateHypeLink);
	DDX_Text(pDX, IDC_STATIC_ANTISPY_CN, m_szAntiSpyDotCn);
	DDX_Control(pDX, IDC_STATIC_ANTISPY_CN, m_AntiSpyDotCnHypeLink);
	DDX_Text(pDX, IDC_STATIC_QQ, m_szQQ);
}


BEGIN_MESSAGE_MAP(CAboutToolDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CAboutToolDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CAboutToolDlg 消息处理程序

void CAboutToolDlg::OnBnClickedOk()
{
}

BOOL g_bCover = FALSE;

HBRUSH CAboutToolDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (
// 		pWnd->GetDlgCtrlID() == IDC_STATIC_TENCENT ||
// 		pWnd->GetDlgCtrlID() == IDC_STATIC_SINA ||
// 		pWnd->GetDlgCtrlID() == IDC_STATIC_EMAIL ||
		pWnd->GetDlgCtrlID() == IDC_STATIC_UPDATE// ||
/*		pWnd->GetDlgCtrlID() == IDC_STATIC_WEBSITE*/
		)
	{
		pDC->SetTextColor(RGB(0, 0, 255));
	}

// 	if (g_bCover)
// 	{
// 		pDC->SetTextColor(RGB(255, 0, 0));
// 	} 

// 	if (pWnd->GetDlgCtrlID() == IDC_STATIC_51POJIE)
// 	{
// 		pDC->SetTextColor(RGB(0, 0, 255));
// 	}

	return hbr;
}

void CAboutToolDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//先得到当前鼠标坐标
// 	CPoint PointEmail; 
// 	GetCursorPos(&PointEmail); 
// 	CPoint PointSina; 
// 	GetCursorPos(&PointSina); 
// 	CPoint PointTencent; 
// 	GetCursorPos(&PointTencent); 
 	CPoint PointUpdate; 
 	GetCursorPos(&PointUpdate); 
// 	CPoint PointWebsite; 
// 	GetCursorPos(&PointWebsite); 
// 
// 	CPoint Point52Pojie; 
// 	GetCursorPos(&Point52Pojie); 
// 
// 	//然后得到static控件rect。
// 	CRect rectEmail; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_EMAIL)->GetSafeHwnd(), &rectEmail); 
// 	CRect rectSina; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_SINA)->GetSafeHwnd(), &rectSina); 
// 	CRect rectTencent; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_TENCENT)->GetSafeHwnd(), &rectTencent); 
	CRect rectUpdate; 
	::GetClientRect(GetDlgItem(IDC_STATIC_UPDATE)->GetSafeHwnd(), &rectUpdate); 
// 	CRect rectWebsite; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_WEBSITE)->GetSafeHwnd(), &rectWebsite); 
// 
// 	CRect rect52Pojie; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_51POJIE)->GetSafeHwnd(), &rect52Pojie); 
// 
// 	//然后把当前鼠标坐标转为相对于rect的坐标。 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_EMAIL)->GetSafeHwnd(), &PointEmail); 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_SINA)->GetSafeHwnd(), &PointSina); 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_TENCENT)->GetSafeHwnd(), &PointTencent); 
	::ScreenToClient(GetDlgItem(IDC_STATIC_UPDATE)->GetSafeHwnd(), &PointUpdate); 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_WEBSITE)->GetSafeHwnd(), &PointWebsite); 
// 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_51POJIE)->GetSafeHwnd(), &Point52Pojie); 
// 
// 	if(rectEmail.PtInRect(PointEmail) ||
// 	   rectSina.PtInRect(PointSina) ||
// 	   rectTencent.PtInRect(PointTencent) ||
// 	   rectTencent.PtInRect(PointUpdate) ||
// 	   rectWebsite.PtInRect(PointWebsite)) 
// 	{
// 		SetCursor(LoadCursor(NULL, IDC_HAND));
// 	} 

	if(rectUpdate.PtInRect(PointUpdate)) 
	{
//		g_bCover = TRUE;
		SetCursor(LoadCursor(NULL, IDC_HAND));
//		OnCtlColor(GetDlgItem(IDC_STATIC_UPDATE)->GetDC(), GetDlgItem(IDC_STATIC_UPDATE)->GetWindow(0), 0);
	} 
// 	else
// 	{
// 		g_bCover = FALSE;
// 	}

	CDialog::OnMouseMove(nFlags, point);
}

void CAboutToolDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	//先得到当前鼠标坐标
// 	CPoint PointEmail; 
// 	GetCursorPos(&PointEmail); 
// 	CPoint PointSina; 
// 	GetCursorPos(&PointSina); 
// 	CPoint PointTencent; 
// 	GetCursorPos(&PointTencent); 
	CPoint PointUpdate; 
	GetCursorPos(&PointUpdate); 
// 	CPoint PointWebsite; 
// 	GetCursorPos(&PointWebsite); 
// 
// 	CPoint Point52Pojie; 
// 	GetCursorPos(&Point52Pojie); 
// 
// 	//然后得到static控件rect。
// 	CRect rectEmail; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_EMAIL)->GetSafeHwnd(), &rectEmail); 
// 	CRect rectSina; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_SINA)->GetSafeHwnd(), &rectSina); 
// 	CRect rectTencent; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_TENCENT)->GetSafeHwnd(), &rectTencent); 
	CRect rectUpdate; 
	::GetClientRect(GetDlgItem(IDC_STATIC_UPDATE)->GetSafeHwnd(), &rectUpdate); 
// 	CRect rectWebsite; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_WEBSITE)->GetSafeHwnd(), &rectWebsite); 
// 
// 	CRect rect52Pojie; 
// 	::GetClientRect(GetDlgItem(IDC_STATIC_51POJIE)->GetSafeHwnd(), &rect52Pojie); 
// 
// 	//然后把当前鼠标坐标转为相对于rect的坐标。 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_EMAIL)->GetSafeHwnd(), &PointEmail); 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_SINA)->GetSafeHwnd(), &PointSina); 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_TENCENT)->GetSafeHwnd(), &PointTencent); 
 	::ScreenToClient(GetDlgItem(IDC_STATIC_UPDATE)->GetSafeHwnd(), &PointUpdate); 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_WEBSITE)->GetSafeHwnd(), &PointWebsite); 
// 
// 	::ScreenToClient(GetDlgItem(IDC_STATIC_51POJIE)->GetSafeHwnd(), &Point52Pojie); 
// 
// 	if (rectEmail.PtInRect(PointEmail))
// 	{
// 		WCHAR szMailTo[] = {'m','a','i','l','t','o',':','m','i','n','z','h','e','n','f','e','i','@','1','6','3','.','c','o','m','\0'};
// 		ShellExecute(NULL,NULL, szMailTo,NULL,NULL,SW_SHOW);
// 	}
// 	else if (rectSina.PtInRect(PointSina))
// 	{
// 		WCHAR szUrl[] = {'h','t','t','p',':','/','/','w','e','i','b','o','.','c','o','m','/','m','i','n','z','h','e','n','f','e','i','\0'};	
// 		ShellExecuteW(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
// 	}
// 	else if (rectTencent.PtInRect(PointTencent))
// 	{
// 		WCHAR szUrl[] = {'h','t','t','p',':','/','/','t','.','q','q','.','c','o','m','/','m','i','n','z','h','e','n','f','e','i','\0'};	
// 		ShellExecuteW(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
// 	}
// 	else 
	if (rectUpdate.PtInRect(PointUpdate))
	{
		CheckForUpdate();
	}
// 	else if (rectWebsite.PtInRect(PointWebsite))
// 	{
// 		WCHAR szUrl[] = {'h','t','t','p',':','/','/','w','w','w','.','k','i','n','g','p','r','o','t','e','c','t','.','c','o','m','\0'};	
// 		ShellExecuteW(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
// 	}
// 
// 	else if(rect52Pojie.PtInRect(Point52Pojie)) 
// 	{
// 		WCHAR szUrl[] = {'h','t','t','p',':','/','/','w','w','w','.','5','2','p','o','j','i','e','.','c','n','\0'};	
// 		ShellExecuteW(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
// 	} 

	CDialog::OnLButtonDown(nFlags, point);
}

inline CString ms2ws(LPCSTR szSrc, int cbMultiChar=-1)
{
	CString strDst;
	if ( szSrc==NULL || cbMultiChar==0 ){
		return strDst;
	}

	WCHAR*pBuff=NULL;
	int nLen=MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,NULL,0);
	if ( nLen>0 ){
		pBuff=new WCHAR[nLen+1];
		if ( pBuff ){
			MultiByteToWideChar(CP_ACP,0,szSrc,cbMultiChar,pBuff,nLen);
			pBuff[nLen]=0;
			strDst = pBuff;
			delete[] pBuff;
		}
	}

	return strDst;
}

BOOL CAboutToolDlg::CheckForUpdates()
{
	BOOL bRet = FALSE;
	WCHAR szUrlChinese[] = {'h','t','t','p',':','/','/','m','z','f','2','0','0','8','.','b','l','o','g','.','1','6','3','.','c','o','m','/','b','l','o','g','/','s','t','a','t','i','c','/','3','5','5','9','9','7','8','6','2','0','1','1','4','1','8','7','2','6','8','5','0','2','/','\0'};
	CStringA strline;
	CString szSource;
	CHttpFile* myHttpFile = NULL;
	CInternetSession mySession(NULL, 0);

	m_szVersion = L"";

	try
	{
		myHttpFile = (CHttpFile*)mySession.OpenURL(szUrlChinese);
		if (myHttpFile)
		{
			BOOL bStart = FALSE;
			CHAR szEnd[] = {'n','b','w','-','b','l','o','g','-','e','n','d','\0'};
			CHAR szStart[] = {'n','b','w','-','b','l','o','g','-','s','t','a','r','t','\0'};

			while(myHttpFile->ReadString((CString&)strline))
			{
				if (strline.Find(szEnd) != -1)
				{
					bStart = FALSE;
					break;
				}

				if (bStart)
				{
					szSource += ms2ws(strline.GetBuffer());
				}

				if (strline.Find(szStart) != -1)
				{
					bStart = TRUE;
				}
			}

			myHttpFile->Close();
			mySession.Close();
		}
	}
	catch(CInternetException *IE)
	{
		if (myHttpFile)
		{
			myHttpFile->Close();
		}
		IE->Delete();
	}

	if (!szSource.IsEmpty())
	{
		WCHAR Version[] = {'v','e','r','s','i','o','n',':','\0'};	// version:
		WCHAR UNDIV[] = {'<','/','\0'};								// </

		// 获取版本信息
		int nDiv = -1;
		int nPos = szSource.Find(Version);
		if (nPos != -1)
		{
			nPos += wcslen(Version);
			nDiv = szSource.Find(UNDIV, nPos);
			if (nDiv != -1 && nDiv > nPos)
			{
				m_szVersion = szSource.Mid(nPos, nDiv-nPos);
			}
		}

		if (!m_szVersion.IsEmpty()) 
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

void CAboutToolDlg::GotoUpdateUrl()
{
	WCHAR szOpen[] = {'o','p','e','n','\0'};

	if (g_enumLang == enumEnglish)
	{
		WCHAR szGoogleCode[] = {'h','t','t','p',':','/','/','c','o','d','e','.','g','o','o','g','l','e','.','c','o','m','/','p','/','a','n','t','i','s','p','y','/','d','o','w','n','l','o','a','d','s','/','l','i','s','t','\0'};
		ShellExecuteW(NULL, szOpen, szGoogleCode, NULL, NULL, SW_SHOW);
	}
	else
	{
		WCHAR szAntiSpyUrl[] = {'h','t','t','p',':','/','/','w','w','w','.','A','n','t','i','S','p','y','.','c','n','\0'};
		ShellExecuteW(NULL, szOpen, szAntiSpyUrl, NULL, NULL, SW_SHOW);
	}
}

void CAboutToolDlg::CheckForUpdate()
{
	if (CheckForUpdates())
	{
		if (m_szVersion.CompareNoCase(g_szVersion))
		{
			GotoUpdateUrl();
		}
		else
		{
			::MessageBox(NULL, szTheLatestVersion[g_enumLang], L"AntiSpy", MB_OK | MB_ICONINFORMATION);
		}
	}
	else
	{
		GotoUpdateUrl();
	}
}

BOOL CAboutToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 如果是英文版本，则下面的友情链接就不显示
	if (g_enumLang == enumEnglish)
	{
		GetDlgItem(IDC_STATIC_51POJIE)->ShowWindow(FALSE);
		GetDlgItem(IDC_LINKS)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_FENGQU)->ShowWindow(FALSE);
	}

	GetDlgItem(IDC_STATIC_DONATE)->SetWindowText(szDonate[g_enumLang]);
	
	m_szContactAuthor = szContactAuthor[g_enumLang];
	m_szAuthor = szAuthor[g_enumLang];
	m_szEmail = szEmail[g_enumLang];
	m_szSina = szSina[g_enumLang];
	m_szTencent = szTencent[g_enumLang];
	m_szInformationAndThanks = szInformationAndCopyright[g_enumLang];
	m_szInformation = szInformation[g_enumLang];
	m_szSystem = szSystem[g_enumLang];
	m_szThanks = szThanks[g_enumLang];
	m_szCopyright = szCopyright[g_enumLang];
	m_szMianze = szMianze[g_enumLang];
	m_szUpdate = szUpdates[g_enumLang];
	m_szWebsite = szWebsite[g_enumLang];
	m_szAntiSpyDotCn = szAntiSpy[g_enumLang];
	m_szQQ = szQQ[g_enumLang];
	m_szCheckUpdate.Format(szCheckForUpdates[g_enumLang], g_szVersion);

	WCHAR szUrlFengqu[] = {'h','t','t','p',':','/','/','w','w','w','.','f','e','n','q','u','.','c','o','m','\0'};	
	m_FengquHypeLike.SetUnderline(false);
	m_FengquHypeLike.SetURL(szUrlFengqu);
	m_FengquHypeLike.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	WCHAR szUrl52Pojie[] = {'h','t','t','p',':','/','/','w','w','w','.','5','2','p','o','j','i','e','.','c','n','\0'};	
	m_52PojieHypeLink.SetUnderline(false);
	m_52PojieHypeLink.SetURL(szUrl52Pojie);
	m_52PojieHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));
	
	WCHAR szUrlBlog[] = {'h','t','t','p',':','/','/','w','w','w','.','K','i','n','g','P','r','o','t','e','c','t','.','c','o','m','\0'};	
	m_BolgHypeLink.SetUnderline(false);
	m_BolgHypeLink.SetURL(szUrlBlog);
	m_BolgHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	WCHAR szUrlSina[] = {'h','t','t','p',':','/','/','w','e','i','b','o','.','c','o','m','/','m','i','n','z','h','e','n','f','e','i','\0'};	
	m_SinaHypeLink.SetUnderline(false);
	m_SinaHypeLink.SetURL(szUrlSina);
	m_SinaHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	WCHAR szUrlTecent[] = {'h','t','t','p',':','/','/','t','.','q','q','.','c','o','m','/','m','i','n','z','h','e','n','f','e','i','\0'};	
	m_TencetHypeLink.SetUnderline(false);
	m_TencetHypeLink.SetURL(szUrlTecent);
	m_TencetHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	
	WCHAR szAntiSpyMail[] = {'A','n','t','i','S','p','y','@','1','6','3','.','c','o','m','\0'};
	WCHAR szMailTo[] = {'m','a','i','l','t','o',':','A','n','t','i','S','p','y','@','1','6','3','.','c','o','m','\0'};
	m_EmailHypeLink.SetUnderline(false);
	m_EmailHypeLink.SetURL(szMailTo);
	m_EmailHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));
	m_EmailHypeLink.SetTipText(szAntiSpyMail);

	WCHAR szAntiSpy[] = {'h','t','t','p',':','/','/','w','w','w','.','A','n','t','i','S','p','y','.','c','n','\0'};	
	m_AntiSpyDotCnHypeLink.SetUnderline(false);
	m_AntiSpyDotCnHypeLink.SetURL(szAntiSpy);
	m_AntiSpyDotCnHypeLink.SetColors(RGB(0, 0, 255), 0x00800080, RGB(255, 0, 0));

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CAboutToolDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE; 

	return CDialog::PreTranslateMessage(pMsg);
}
