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
#include "FileFunc.h"
#include <list>
#include <vector>
using namespace std;
#include "Function.h"
#include "ConnectDriver.h"
#include "SignVerifyDlg.h"
#include "afxwin.h"
#include "xySplitterWnd.h"

typedef struct _FILE_VOLUME_INFO_
{
	HTREEITEM hItem;
	CString szVolume;
}FILE_VOLUME_INFO, *PFILE_VOLUME_INFO;

typedef struct _FILE_DIRECTORY_INFORMATION {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;


typedef struct _FILE_INFO {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER ModificationTime;
	LARGE_INTEGER FileSize;
	ULONG FileAttributes;
	CString FileName;
} FILE_INFO, *PFILE_INFO;

typedef struct _FILE_INFO_DISK {
	WCHAR* szPath;
	LARGE_INTEGER FileSize;
	BOOL bIsDriectory;
} FILE_INFO_DISK, *PFILE_INFO_DISK;

typedef struct _NT_AND_DOS_VOLUME_NAME
{
	CString szDosName;		/*   \Device\HarddiskVolume1  */
	CString szNtName;		/*   C:\   */
}NT_AND_DOS_VOLUME_NAME;

// CFileDlg dialog

class CFileDlg : public CDialog
{
	DECLARE_EASYSIZE
	DECLARE_DYNAMIC(CFileDlg)

public:
	CFileDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileDlg();

// Dialog Data
	enum { IDD = IDD_FILE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void EmnuVolume();
	BOOL IsPathHaveDirectory(CString szPath);
	CTreeCtrl m_tree;
	CSortListCtrl m_list;
	CImageList m_TreeImageList;
	CFileFunc m_FileFunc;
	list<FILE_VOLUME_INFO> m_VolumeList;	// 保存计算机磁盘信息
	BOOL IsVolumeItem(HTREEITEM hItem, CString &szVolume);
	CString m_szCombo;
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	void GetFiles(CString szPath, vector<FILE_INFO> &FileInfoList);
	VOID SortByFileAttributes();
	void EnumFiles(CString szPath);
	void InsertFileItems(CString szPath);
	CString GetFileAttributes(ULONG nAttributes);
	vector<FILE_INFO> m_FileInfoList;		// 保存当前列表框的文件信息
	CImageList m_FileImageList;
	HICON m_hDirectoryIcon;					// list列表中文件夹的图标
	afx_msg void OnNMCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpandedTree(NMHDR *pNMHDR, LRESULT *pResult);
	void AddDirectoryItemToTree(CString szPath, HTREEITEM hItem);
	void DeleteSubTree(HTREEITEM TreeItem);
	CString m_szCurrentPath;	// 记录当前选中的文件夹
	CString GetPathByTreeItem(HTREEITEM hItem);
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	void TrimPath();
	HTREEITEM m_hCurrentTreeItem;
	BOOL m_bEnumTree;
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileOpen();
	afx_msg void OnFileRefresh();
	afx_msg void OnFileLockInfo();
	afx_msg void OnFileDelete();
	afx_msg void OnFileDeleteAndDenyRebuild();
	afx_msg void OnFileAddToRestartDelete();
	afx_msg void OnFileRestartReplace();
	afx_msg void OnFileCopyTo();
	afx_msg void OnFileRename();
	afx_msg void OnFileCopyFileName();
	afx_msg void OnFileCopyFilePath();
	afx_msg void OnFileShuxing();
	afx_msg void OnFileJmpToExplorer();
	afx_msg void OnFileDeleteHideSystemShuxing();
	afx_msg void OnFileCheckVerify();
	afx_msg void OnFileCheckAllVerify();
	CString GetFilePathInListCtrl();
	BOOL InitDevice2Path();
	void ConvertVolumePaths( IN PWCHAR DeviceName, IN PWCHAR VolumeName );
	BOOL DeleteDirectory(CString szDir);
	void OpenDirectory(CString szFileName);
	void OpenParentDirectory();
	void FixDotPath(CString &szPath);
	CommonFunctions m_Functions;
	vector <NT_AND_DOS_VOLUME_NAME> m_nt_and_dos_valume_list;
	CConnectDriver m_Driver;
	CSignVerifyDlg m_SignVerifyDlg;
	afx_msg void OnFileTreeRefresh();
	afx_msg void OnFileTreeLockInfo();
	afx_msg void OnFileTreeDelete();
	afx_msg void OnFileTreeCopyFileName();
	afx_msg void OnFileTreeCopyFilePath();
	afx_msg void OnFileTreeShuxing();
	afx_msg void OnFileTreeGotoExplorer();
	afx_msg void OnFileTreeDeleteAllShuxing();
	void InitTree();
	void LookForFileLockInfo(CString szFilePath, BOOL bDirectory);
//	BOOL SetFileNormalAttribute(CString szPath);
	BOOL SetFileNormalAttribute(CString szPath, ULONG Attribute);
	void InitComboBox();
	CString m_szAddress;
	CButton m_BtnGoto;
	CComboBox m_ComboBox;
	afx_msg void OnBnClickedGoto();
	HTREEITEM GetVolumeItem(CString szVolume);
	HTREEITEM GetTreeSubItemByName(HTREEITEM PatentItem, CString szName);
	afx_msg void OnFileViewFileLockedInfo();
	afx_msg void OnFileTreeViewFileLockedInfo();
	afx_msg void OnFileTreeSetHideSystemAttribute();
	afx_msg void OnFileSetHideSystemAttribute();
	BOOL FileCopy(CString szSrc, CString szDes);
	afx_msg void OnFileTreeCopyTo();
	afx_msg void OnFileTreeRename();
	CString GetComboPathByTreeItem(HTREEITEM hItem);
	afx_msg void OnCbnDropdownCombo();
	void GotoPath(CString szPath, CString szFile);
	COLORREF m_clrHideFile;					// 隐藏文件的颜色
	COLORREF m_clrSystemFile;				// 系统文件颜色
	COLORREF m_clrSystemAndHideFile;		// 系统并且隐藏的文件颜色
	COLORREF m_clrNormalFile;				// 正常文件颜色
	afx_msg void OnFileCheckOnline();
	afx_msg void OnFileCalcMd5();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMSetfocusList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMKillfocusList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSetfocusCombo();
	afx_msg void OnCbnKillfocusCombo();
	void EnumFilesByPareseDisk(CString szPath);
	BOOL m_bInComboBox;
	BOOL m_bInListCtrl;
	vector<FILE_INFO_DISK> m_vectorFileParaseDisk;
	CxSplitterWnd   m_xSplitter;
};
