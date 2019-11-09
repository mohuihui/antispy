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
#ifndef _SELF_PROTECT_SSDT_H_
#define _SELF_PROTECT_SSDT_H_

#include "Struct.h"

NTSTATUS SetProcessPid(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
NTSTATUS StartOrStopSelfProtect(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);

HWND Fake_NtUserGetForegroundWindow();
NTSTATUS Fake_NtUserBuildHwndList( HDESK hDesktop, HWND hwndParent, BOOLEAN bChildren, ULONG dwThreadId, ULONG lParam, HWND* pWnd, ULONG* pBufSize);
NTSTATUS Fake_NtUserBuildHwndList_Win8( HDESK hDesktop, HWND hwndParent, BOOLEAN bChildren, ULONG dwThreadId, ULONG nUnKnow, ULONG lParam, HWND* pWnd, ULONG* pBufSize);
BOOLEAN Fake_NtUserDestroyWindow(HWND Wnd);
HWND Fake_NtUserFindWindowEx( HWND hwndParent, HWND hwndChildAfter, PUNICODE_STRING ucClassName, PUNICODE_STRING ucWindowName, DWORD dwUnknown );
BOOL Fake_NtUserMessageCall( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, ULONG_PTR ResultInfo, DWORD dwType, BOOL Ansi);
HWND Fake_NtUserWindowFromPoint( LONG X, LONG Y);
BOOL Fake_NtUserShowWindow( HWND hWnd, LONG nCmdShow);
LONG Fake_NtUserSetWindowLong( HWND hWnd, DWORD Index, LONG NewValue, BOOL Ansi);
HWND Fake_NtUserSetParent( HWND hWndChild, HWND hWndNewParent);
DWORD Fake_NtUserQueryWindow( HWND hWnd, DWORD Index);
BOOL Fake_NtUserPostThreadMessage( DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL Fake_NtUserPostMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

NTSTATUS Fake_NtOpenProcess( __out PHANDLE ProcessHandle, __in ACCESS_MASK DesiredAccess, __in POBJECT_ATTRIBUTES ObjectAttributes, __in_opt PCLIENT_ID ClientId );
NTSTATUS Fake_NtOpenThread( OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId );
NTSTATUS Fake_NtTerminateThread( __in_opt HANDLE ThreadHandle, __in NTSTATUS ExitStatus );
NTSTATUS Fake_NtTerminateProcess (IN HANDLE ProcessHandle OPTIONAL, IN NTSTATUS ExitStatus);
NTSTATUS Fake_NtDuplicateObject( __in HANDLE SourceProcessHandle, __in HANDLE SourceHandle, __in_opt HANDLE TargetProcessHandle, __out_opt PHANDLE TargetHandle, __in ACCESS_MASK DesiredAccess, __in ULONG HandleAttributes, __in ULONG Options );

BOOL IsCurrentProcessAntiSpy();

#endif