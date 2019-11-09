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
#include "SelfProtectSSDT.h"
#include "..\\..\\Common\\Common.h"
#include "InitWindows.h"
#include "HookEngine.h"

NTSTATUS SetProcessPid(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_SELF_PROTECT pSelfProtect = (PCOMMUNICATE_SELF_PROTECT)pInBuffer;
	ULONG nPid = pSelfProtect->op.SetPid.nPid;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnPsLookupProcessByProcessId MzfPsLookupProcessByProcessId = (pfnPsLookupProcessByProcessId)GetGlobalVeriable(enumPsLookupProcessByProcessId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	PEPROCESS pEprocess = NULL;

	if (nPid <= 0 || MzfPsLookupProcessByProcessId == NULL || !MzfObfDereferenceObject)
	{
		return status;
	}

	status = MzfPsLookupProcessByProcessId((HANDLE)nPid, &pEprocess);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	SetGlobalVeriable(enumAntiSpyPid, nPid);
	SetGlobalVeriable(enumAntiSpyEprocess, (ULONG)pEprocess);
	
	MzfObfDereferenceObject(pEprocess);

	return STATUS_SUCCESS;
}


NTSTATUS StartOrStopSelfProtect(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet)
{
	PCOMMUNICATE_SELF_PROTECT pSelfProtect = (PCOMMUNICATE_SELF_PROTECT)pInBuffer;
	ULONG nMask = pSelfProtect->op.StartOrStop.nMask;
	ULONG bStart = pSelfProtect->op.StartOrStop.bStart;

	if (bStart == TRUE)
	{
		if (nMask & SELF_PROTECT_SSDT)
		{
			SetSSDTHook(enumNtOpenProcessIndex);
			SetSSDTHook(enumNtOpenThreadIndex);
			SetSSDTHook(enumNtTerminateThreadIndex);
			SetSSDTHook(enumNtTerminateProcessIndex);
			SetSSDTHook(enumNtDuplicateObjectIndex);
		}

		if (nMask & SELF_PROTECT_SHADOW)
		{
			SetShadowHook(enumNtUserGetForegroundWindowIndex);
			SetShadowHook(enumNtUserBuildHwndListIndex);
			SetShadowHook(enumNtUserDestroyWindowIndex);
			SetShadowHook(enumNtUserFindWindowExIndex);
			SetShadowHook(enumNtUserMessageCallIndex);
			SetShadowHook(enumNtUserWindowFromPointIndex);
			SetShadowHook(enumNtUserShowWindowIndex);
 			SetShadowHook(enumNtUserSetWindowLongIndex);
 			SetShadowHook(enumNtUserSetParentIndex);
 			SetShadowHook(enumNtUserQueryWindowIndex);
			SetShadowHook(enumNtUserPostThreadMessageIndex);
			SetShadowHook(enumNtUserPostMessageIndex);
		}
	}
	else
	{
		if (nMask & SELF_PROTECT_SSDT)
		{
			RestoreSSDTHook(enumNtOpenProcessIndex);
			RestoreSSDTHook(enumNtOpenThreadIndex);
			RestoreSSDTHook(enumNtTerminateThreadIndex);
			RestoreSSDTHook(enumNtTerminateProcessIndex);
			RestoreSSDTHook(enumNtDuplicateObjectIndex);
		}

		if (nMask & SELF_PROTECT_SHADOW)
		{
			RestoreShadowHook(enumNtUserGetForegroundWindowIndex);
			RestoreShadowHook(enumNtUserBuildHwndListIndex);
			RestoreShadowHook(enumNtUserDestroyWindowIndex);
			RestoreShadowHook(enumNtUserFindWindowExIndex);
			RestoreShadowHook(enumNtUserMessageCallIndex);
			RestoreShadowHook(enumNtUserWindowFromPointIndex);
			RestoreShadowHook(enumNtUserShowWindowIndex);
			RestoreShadowHook(enumNtUserSetWindowLongIndex);
			RestoreShadowHook(enumNtUserSetParentIndex);
			RestoreShadowHook(enumNtUserQueryWindowIndex);
			RestoreShadowHook(enumNtUserPostThreadMessageIndex);
			RestoreShadowHook(enumNtUserPostMessageIndex);
		}
	}

	return STATUS_SUCCESS;
}

BOOL IsAntiSpyWnd(HWND hWnd)
{
	BOOL bRet = FALSE;
	pfnNtUserQueryWindow MzfNtUserQueryWindow = (pfnNtUserQueryWindow)GetGlobalVeriable(enumNtUserQueryWindow);
	ULONG nPid = GetGlobalVeriable(enumAntiSpyPid);
	DWORD dwRetPid = 0;

	if (hWnd == NULL || 
		nPid == 0 ||
		MzfNtUserQueryWindow == NULL
		)
	{
		return bRet;
	}

	dwRetPid = MzfNtUserQueryWindow(hWnd, 0);
	if (dwRetPid == nPid)
	{
		bRet = TRUE;
	}

	return bRet;
}

// 判断当前进程是否是AS
BOOL IsCurrentProcessAntiSpy()
{
	BOOL bRet = FALSE;
	PEPROCESS pAntiSpyEprocess = (PEPROCESS)GetGlobalVeriable(enumAntiSpyEprocess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);

	if (pAntiSpyEprocess && 
		MzfIoGetCurrentProcess && 
		MzfIoGetCurrentProcess() == pAntiSpyEprocess)
	{
		bRet = TRUE;
	}

	return bRet;
}

HWND Fake_NtUserGetForegroundWindow()
{
	pfnNtUserGetForegroundWindow NtUserGetForegroundWindow = NULL;
	HWND hWnd = NULL;
	
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	NtUserGetForegroundWindow = (pfnNtUserGetForegroundWindow)GetGlobalVeriable(enumOriginNtUserGetForegroundWindow);
	hWnd = NtUserGetForegroundWindow();

	// 如果当前进程不是AS
	if (!IsCurrentProcessAntiSpy())
	{
		if (IsAntiSpyWnd(hWnd))
		{
			hWnd = (HWND)GetGlobalVeriable(enumFakeWnd);
		}
		else
		{
			SetGlobalVeriable(enumFakeWnd, (ULONG)hWnd);
		}
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	
	return hWnd;
}

// 主要过滤我们的窗口
NTSTATUS
Fake_NtUserBuildHwndList(
					HDESK hDesktop,
					HWND hwndParent,
					BOOLEAN bChildren,
					ULONG dwThreadId,
					ULONG lParam,
					HWND* pWnd,
					ULONG* pBufSize)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnNtUserBuildHwndList pOriginNtUserBuildHwndList = NULL;

	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	pOriginNtUserBuildHwndList = (pfnNtUserBuildHwndList)GetGlobalVeriable(enumOriginNtUserBuildHwndList);
	
	status = pOriginNtUserBuildHwndList(hDesktop, hwndParent, bChildren, dwThreadId, lParam, pWnd, pBufSize);

	if (NT_SUCCESS(status) &&			// 函数调用成功
		!IsCurrentProcessAntiSpy() &&	// 当前进程不是AntiSpy
		*pBufSize > 0)					// 枚举到的窗口要 》 0
	{
		// 如果枚举的是枚举AntiSpy的窗口，那么把枚举到的句柄全部置0
		if (hwndParent && IsAntiSpyWnd(hwndParent))
		{
			ULONG i = 0, nCnt = *pBufSize;
			for (i = 0; i < nCnt; i++)
			{
				pWnd[i] = 0;
			}

			*pBufSize = 0;
			status = STATUS_UNSUCCESSFUL;
		}
		else if ( hwndParent == NULL )
		{
			ULONG i = 0, nCnt = *pBufSize, n = 0;

			do
			{
				if ( !IsAntiSpyWnd(pWnd[n]) )
				{
					if ( i != n )
					{
						pWnd[i] = pWnd[n];
					}

					++i;
				}

				++n;
			}
			while ( n < nCnt );

			*pBufSize = i;
		}
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	return status;
}

NTSTATUS
Fake_NtUserBuildHwndList_Win8(
							  HDESK hDesktop,
							  HWND hwndParent,
							  BOOLEAN bChildren,
							  ULONG dwThreadId,
							  ULONG nUnKnow,
							  ULONG lParam,
							  HWND* pWnd,
							  ULONG* pBufSize)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnNtUserBuildHwndListWin8 pOriginNtUserBuildHwndList = NULL;

	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	pOriginNtUserBuildHwndList = (pfnNtUserBuildHwndListWin8)GetGlobalVeriable(enumOriginNtUserBuildHwndList);

	status = pOriginNtUserBuildHwndList(hDesktop, hwndParent, bChildren, dwThreadId, nUnKnow, lParam, pWnd, pBufSize);

	if (NT_SUCCESS(status) &&			// 函数调用成功
		!IsCurrentProcessAntiSpy() &&	// 当前进程不是AntiSpy
		*pBufSize > 0)					// 枚举到的窗口要 》 0
	{
		// 如果枚举的是枚举AntiSpy的窗口，那么把枚举到的句柄全部置0
		if (hwndParent && IsAntiSpyWnd(hwndParent))
		{
			ULONG i = 0, nCnt = *pBufSize;
			for (i = 0; i < nCnt; i++)
			{
				pWnd[i] = 0;
			}

			*pBufSize = 0;
			status = STATUS_UNSUCCESSFUL;
		}
		else if ( hwndParent == NULL )
		{
			ULONG i = 0, nCnt = *pBufSize, n = 0;

			do
			{
				if ( !IsAntiSpyWnd(pWnd[n]) )
				{
					if ( i != n )
					{
						pWnd[i] = pWnd[n];
					}

					++i;
				}

				++n;
			}
			while ( n < nCnt );

			*pBufSize = i;
		}
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	return status;
}

BOOLEAN Fake_NtUserDestroyWindow(HWND Wnd)
{
	BOOLEAN bRet = FALSE;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	
	/*
		1.如果当前进程是AS
		2.如果被销毁的窗体不是AS的，那么就调用原始函数
	*/
	if ( IsCurrentProcessAntiSpy() || !IsAntiSpyWnd(Wnd) )
	{
		pfnNtUserDestroyWindow NtUserDestroyWindow = (pfnNtUserDestroyWindow)GetGlobalVeriable(enumOriginNtUserDestroyWindow);
		bRet = NtUserDestroyWindow(Wnd);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return bRet;
}

HWND
Fake_NtUserFindWindowEx(
						 HWND  hwndParent,
						 HWND  hwndChildAfter,
						 PUNICODE_STRING  ucClassName,
						 PUNICODE_STRING  ucWindowName,
						 DWORD dwUnknown
						 )
{
	HWND hRet = NULL;
	pfnNtUserFindWindowEx NtUserFindWindowEx = NULL;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	NtUserFindWindowEx = (pfnNtUserFindWindowEx)GetGlobalVeriable(enumOriginNtUserFindWindowEx);
	hRet = NtUserFindWindowEx(hwndParent, hwndChildAfter, ucClassName, ucWindowName, dwUnknown);
	if (hRet && !IsCurrentProcessAntiSpy() && IsAntiSpyWnd(hRet))
	{
		hRet = NULL;
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return hRet;
}

BOOL IsNotFilterMessage(UINT Msg)
{
	BOOL bRet = FALSE;

	if ( Msg - 0x3A <= 1 )
	{
		bRet = 1;
	}

	return bRet;
}

BOOL
Fake_NtUserMessageCall(
						HWND hWnd,
						UINT Msg,
						WPARAM wParam,
						LPARAM lParam,
						ULONG_PTR ResultInfo,
						DWORD dwType, // FNID_XX types
						BOOL Ansi)
{
	BOOL bRet = FALSE;
	pfnNtUserMessageCall NtUserMessageCall = NULL;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	
	NtUserMessageCall = (pfnNtUserMessageCall)GetGlobalVeriable(enumOriginNtUserMessageCall);
	
	if (Msg == 0x7F || 
		Msg == 0x9150 || 
		Msg == 0x3e4 || 
		IsNotFilterMessage(Msg) ||		// 放行不过滤的消息
		IsCurrentProcessAntiSpy() ||	// 如果当前进程是AS，则放行
		!IsAntiSpyWnd(hWnd))			// 如果目标窗口不是AS，则放行
	{
		bRet = NtUserMessageCall(hWnd, Msg, wParam, lParam, ResultInfo, dwType, Ansi);
	}
	else
	{
		KdPrint(("Msg: 0x%X\n", Msg));
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return bRet;
}

#define WM_SYSCOMMAND                   0x0112

BOOL
Fake_NtUserPostMessage(
						HWND hWnd,
						UINT Msg,
						WPARAM wParam,
						LPARAM lParam)
{
	BOOL bRet = FALSE;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	if (Msg == WM_SYSCOMMAND || IsCurrentProcessAntiSpy() || !IsAntiSpyWnd(hWnd))
	{
		pfnNtUserPostMessage NtUserPostMessage = (pfnNtUserPostMessage)GetGlobalVeriable(enumOriginNtUserPostMessage);
		bRet = NtUserPostMessage(hWnd, Msg, wParam, lParam);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return bRet;
}

BOOL IsAntiSpyProcessByTid(DWORD dwTid)
{
	BOOL bRet = FALSE;
	PETHREAD pEthread = NULL;
	PEPROCESS pAntiSpyProcess = (PEPROCESS)GetGlobalVeriable(enumAntiSpyEprocess);
	pfnIoThreadToProcess MzfIoThreadToProcess = (pfnIoThreadToProcess)GetGlobalVeriable(enumIoThreadToProcess);
	pfnPsLookupThreadByThreadId MzfPsLookupThreadByThreadId = (pfnPsLookupThreadByThreadId)GetGlobalVeriable(enumPsLookupThreadByThreadId);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);

	if (dwTid == 0 || 
		!MzfIoThreadToProcess ||
		!MzfPsLookupThreadByThreadId ||
		!MzfObfDereferenceObject ||
		!pAntiSpyProcess) 
	{
		return bRet;
	}

	if (NT_SUCCESS(MzfPsLookupThreadByThreadId((HANDLE)dwTid, &pEthread)))
	{
		PEPROCESS pEprocess = MzfIoThreadToProcess(pEthread);
		if (pEprocess == pAntiSpyProcess)
		{
			bRet = TRUE;
		}

		MzfObfDereferenceObject(pEthread);
	}

	return bRet;
}

BOOL
Fake_NtUserPostThreadMessage(
							  DWORD idThread,
							  UINT Msg,
							  WPARAM wParam,
							  LPARAM lParam)
{
	BOOL bRet = FALSE;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	
	if (IsCurrentProcessAntiSpy() || !IsAntiSpyProcessByTid(idThread))
	{
		pfnNtUserPostThreadMessage NtUserPostThreadMessage = (pfnNtUserPostThreadMessage)GetGlobalVeriable(enumOriginNtUserPostThreadMessage);
		bRet = NtUserPostThreadMessage(idThread, Msg, wParam, lParam);
	}
	
	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return bRet;
}

DWORD
Fake_NtUserQueryWindow(
						HWND hWnd, 
						DWORD Index)
{
	DWORD dwRet = 0;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	
	if (IsCurrentProcessAntiSpy() || !IsAntiSpyWnd(hWnd))
	{
		pfnNtUserQueryWindow NtUserQueryWindow = (pfnNtUserQueryWindow)GetGlobalVeriable(enumNtUserQueryWindow)	;
		dwRet = NtUserQueryWindow(hWnd, Index);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return dwRet;
}

HWND
Fake_NtUserSetParent(
					  HWND hWndChild,
					  HWND hWndNewParent)
{
	HWND hWnd = NULL;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	if (IsCurrentProcessAntiSpy() || !IsAntiSpyWnd(hWndChild))
	{
		pfnNtUserSetParent NtUserSetParent = (pfnNtUserSetParent)GetGlobalVeriable(enumOriginNtUserSetParent);
		hWnd = NtUserSetParent(hWndChild, hWndNewParent);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return hWnd;
}

LONG
Fake_NtUserSetWindowLong(
						  HWND hWnd,
						  DWORD Index,
						  LONG NewValue,
						  BOOL Ansi)
{
	LONG nRet = 0;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	if (IsCurrentProcessAntiSpy() || !IsAntiSpyWnd(hWnd))
	{
		pfnNtUserSetWindowLong NtUserSetWindowLong = (pfnNtUserSetWindowLong)GetGlobalVeriable(enumOriginNtUserSetWindowLong);
		nRet = NtUserSetWindowLong(hWnd, Index, NewValue, Ansi);
 	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return nRet;
}

BOOL
Fake_NtUserShowWindow(
					   HWND hWnd,
					   LONG nCmdShow)
{
	BOOL bRet = FALSE;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	if (IsCurrentProcessAntiSpy() || !IsAntiSpyWnd(hWnd))
	{
		pfnNtUserShowWindow NtUserShowWindow = (pfnNtUserShowWindow)GetGlobalVeriable(enumOriginNtUserShowWindow);
		bRet = NtUserShowWindow(hWnd, nCmdShow);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return bRet;
}

HWND
Fake_NtUserWindowFromPoint(
							LONG X,
							LONG Y)
{
	HWND hWnd = NULL;
	pfnNtUserWindowFromPoint NtUserWindowFromPoint = NULL;
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	NtUserWindowFromPoint = (pfnNtUserWindowFromPoint)GetGlobalVeriable(enumOriginNtUserWindowFromPoint);
	hWnd = NtUserWindowFromPoint(X, Y);
	if (hWnd && !IsCurrentProcessAntiSpy() && IsAntiSpyWnd(hWnd))
	{
		hWnd = NULL;
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return hWnd;
}

//////////////////////////////////////////////////////////////////

BOOL IsAntiSpyProcessHandle(HANDLE hProcess)
{
	BOOL bRet = FALSE;
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	POBJECT_TYPE ProcessType = (POBJECT_TYPE)GetGlobalVeriable(enumPsProcessType);
	PEPROCESS pEprocess = NULL;
	PEPROCESS pAntiSpyEprocess = (PEPROCESS)GetGlobalVeriable(enumAntiSpyEprocess);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);

	if (hProcess == NULL ||
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject ||
		!ProcessType ||
		!MzfIoGetCurrentProcess)
	{
		return bRet;
	}

	if (NT_SUCCESS(MzfObReferenceObjectByHandle(hProcess, 0, ProcessType, KernelMode, &pEprocess, NULL)))
	{
		if (MzfIoGetCurrentProcess() == pEprocess)
		{
			bRet = FALSE;
		}
		else if (pAntiSpyEprocess == pEprocess)
		{
			bRet = TRUE;
		}

		MzfObfDereferenceObject(pEprocess);
	}

	return bRet;
}
  
#define PROCESS_TERMINATE         (0x0001)  
#define PROCESS_CREATE_THREAD     (0x0002)  
#define PROCESS_SET_SESSIONID     (0x0004)  
#define PROCESS_VM_OPERATION      (0x0008)  
#define PROCESS_VM_READ           (0x0010)  
#define PROCESS_VM_WRITE          (0x0020)  
#define PROCESS_DUP_HANDLE        (0x0040)  
#define PROCESS_CREATE_PROCESS    (0x0080)  
#define PROCESS_SET_QUOTA         (0x0100)  
#define PROCESS_SET_INFORMATION   (0x0200)  
#define PROCESS_QUERY_INFORMATION (0x0400)  
#define PROCESS_SUSPEND_RESUME    (0x0800)  
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
	0xFFF)


NTSTATUS
Fake_NtOpenProcess(
			  __out PHANDLE  ProcessHandle,
			  __in ACCESS_MASK  DesiredAccess,
			  __in POBJECT_ATTRIBUTES  ObjectAttributes,
			  __in_opt PCLIENT_ID  ClientId
			  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnNtOpenProcess NtOpenProcess = NULL;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	NtOpenProcess = (pfnNtOpenProcess)GetGlobalVeriable(enumOriginNtOpenProcess);
	status = NtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
	
	if (NT_SUCCESS(status) && mode != KernelMode && !IsCurrentProcessAntiSpy())
	{
		HANDLE hProcess = *ProcessHandle;
		ACCESS_MASK dwAccessNotAllowed = PROCESS_TERMINATE | PROCESS_VM_WRITE | PROCESS_SUSPEND_RESUME | PROCESS_SET_INFORMATION | PROCESS_VM_OPERATION | WRITE_DAC | WRITE_OWNER | DELETE;

		if (IsAntiSpyProcessHandle(hProcess) && (DesiredAccess & dwAccessNotAllowed))
		{
			HANDLE HandleTarget = NULL;
			
			// 过滤掉这些权限
			DesiredAccess &= (~dwAccessNotAllowed); 

			status = ZwDuplicateObject(NtCurrentProcess(), 
				hProcess, 
				NtCurrentProcess(), 
				&HandleTarget, 
				DesiredAccess, 
				0, 
				DUPLICATE_CLOSE_SOURCE
				);

			if (!NT_SUCCESS(status))
			{
				ZwClose(hProcess);
				*ProcessHandle = NULL;
				status = STATUS_ACCESS_DENIED;
			}
			else
			{		
				*ProcessHandle = HandleTarget;
			}		
		}
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	return status;
}

BOOL IsAntiSpyThreadHandle(HANDLE hThread)
{
	BOOL bRet = FALSE;
	pfnObReferenceObjectByHandle MzfObReferenceObjectByHandle = (pfnObReferenceObjectByHandle)GetGlobalVeriable(enumObReferenceObjectByHandle);
	pfnIoThreadToProcess MzfIoThreadToProcess = (pfnIoThreadToProcess)GetGlobalVeriable(enumIoThreadToProcess);
	pfnObfDereferenceObject MzfObfDereferenceObject = (pfnObfDereferenceObject)GetGlobalVeriable(enumObfDereferenceObject);
	pfnIoGetCurrentProcess MzfIoGetCurrentProcess = (pfnIoGetCurrentProcess)GetGlobalVeriable(enumIoGetCurrentProcess);
	POBJECT_TYPE ThreadType = (POBJECT_TYPE)GetGlobalVeriable(enumPsThreadType);
	PETHREAD pThread = NULL;
	PEPROCESS pAntiSpyEprocess = (PEPROCESS)GetGlobalVeriable(enumAntiSpyEprocess);

	if (hThread == NULL ||
		!MzfObReferenceObjectByHandle ||
		!MzfObfDereferenceObject ||
		!MzfIoThreadToProcess ||
		!ThreadType ||
		!MzfIoGetCurrentProcess)
	{
		return bRet;
	}

	if (NT_SUCCESS(MzfObReferenceObjectByHandle(hThread, 0, ThreadType, KernelMode, &pThread, NULL)))
	{
		PEPROCESS pEprocess = MzfIoThreadToProcess(pThread);

		if (MzfIoGetCurrentProcess() == pEprocess)
		{
			bRet = FALSE;
		}
		else if (pEprocess == pAntiSpyEprocess)
		{
			bRet = TRUE;
		}

		MzfObfDereferenceObject(pThread);
	}

	return bRet;
}

#define THREAD_QUERY_INFORMATION       (0x0040)  

NTSTATUS
Fake_NtOpenThread(
				   OUT PHANDLE ThreadHandle,
				   IN ACCESS_MASK DesiredAccess,
				   IN POBJECT_ATTRIBUTES ObjectAttributes,
				   IN PCLIENT_ID ClientId
				   )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	pfnNtOpenThread NtOpenThread = NULL;
	ACCESS_MASK dwAccessNotAllowed;

	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	
	NtOpenThread = (pfnNtOpenThread)GetGlobalVeriable(enumOriginNtOpenThread);
	status = NtOpenThread(ThreadHandle, DesiredAccess, ObjectAttributes, ClientId);

	if (mode != KernelMode && NT_SUCCESS(status) && !IsCurrentProcessAntiSpy())
	{
		HANDLE hThread = *ThreadHandle;
		dwAccessNotAllowed = THREAD_TERMINATE | THREAD_SUSPEND_RESUME | THREAD_SET_CONTEXT | THREAD_SET_INFORMATION | WRITE_DAC | WRITE_OWNER | DELETE;

		if (IsAntiSpyThreadHandle(hThread) && (DesiredAccess & dwAccessNotAllowed))
		{
			HANDLE HandleTarget = NULL;

			// 过滤掉这些权限
			DesiredAccess &= (~dwAccessNotAllowed); 

			status = ZwDuplicateObject(NtCurrentProcess(), 
				hThread, 
				NtCurrentProcess(), 
				&HandleTarget, 
				DesiredAccess, 
				0, 
				DUPLICATE_CLOSE_SOURCE
				);

			if (!NT_SUCCESS(status))
			{
				ZwClose(hThread);
				*ThreadHandle = NULL;
				status = STATUS_ACCESS_DENIED;
			}
			else
			{		
				*ThreadHandle = HandleTarget;
			}		
		}
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	return status;
}

NTSTATUS
Fake_NtTerminateProcess (IN HANDLE ProcessHandle OPTIONAL,
						  IN NTSTATUS ExitStatus)
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	// 当前进程是AS，或者目标句柄不是AS，调用原始函数
	if (mode == KernelMode || IsCurrentProcessAntiSpy() || !IsAntiSpyProcessHandle(ProcessHandle))
	{
		pfnNtTerminateProcess NtTerminateProcess = (pfnNtTerminateProcess)GetGlobalVeriable(enumOriginNtTerminateProcess);
		status = NtTerminateProcess(ProcessHandle, ExitStatus);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return status;
}

NTSTATUS
Fake_NtTerminateThread(
						__in_opt HANDLE ThreadHandle,
						__in NTSTATUS ExitStatus
				  )
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	KPROCESSOR_MODE mode = ExGetPreviousMode();
	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	// 当前进程是AS，或者目标句柄不是AS，调用原始函数
	if (mode == KernelMode || IsCurrentProcessAntiSpy() || !IsAntiSpyThreadHandle(ThreadHandle))
	{
		pfnNtTerminateThread NtTerminateThread = (pfnNtTerminateThread)GetGlobalVeriable(enumOriginNtTerminateThread);
		status = NtTerminateThread(ThreadHandle, ExitStatus);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return status;
}

NTSTATUS
Fake_NtDuplicateObject(
						__in HANDLE SourceProcessHandle,
						__in HANDLE SourceHandle,
						__in_opt HANDLE TargetProcessHandle,
						__out_opt PHANDLE TargetHandle,
						__in ACCESS_MASK DesiredAccess,
						__in ULONG HandleAttributes,
						__in ULONG Options
						)
{
	NTSTATUS status = STATUS_ACCESS_DENIED;
	pfnNtDuplicateObject NtDuplicateObject = NULL;
	KPROCESSOR_MODE mode = ExGetPreviousMode();

	InterlockedIncrement((LONG*)GetGlobalVeriable_Address(enumAddend));

	NtDuplicateObject = (pfnNtDuplicateObject)GetGlobalVeriable(enumOriginNtDuplicateObject);
	
	// 当前进程是AS，或者目标句柄不是AS，调用原始函数
	if (mode == KernelMode || IsCurrentProcessAntiSpy())
	{
		status = NtDuplicateObject(SourceProcessHandle, SourceHandle, TargetProcessHandle, TargetHandle, DesiredAccess, HandleAttributes, Options);
	}
	else
	{
		if (IsAntiSpyProcessHandle(SourceProcessHandle))
		{
			Options = Options & ~DUPLICATE_CLOSE_SOURCE;
		}

		status = NtDuplicateObject(SourceProcessHandle, SourceHandle, TargetProcessHandle, TargetHandle, DesiredAccess, HandleAttributes, Options);
	}

	InterlockedDecrement((LONG*)GetGlobalVeriable_Address(enumAddend));
	return status;
}