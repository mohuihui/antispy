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
#include "ProcessWnd.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"

//
// 枚举进程窗口
//
NTSTATUS ListProcessWnds(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WIN_VERSION WinVersion = GetGlobalVeriable(enumWindowsVersion);
	pfnNtUserQueryWindow MzfNtUserQueryWindow = (pfnNtUserQueryWindow)GetGlobalVeriable(enumNtUserQueryWindow);
	pfnExAllocatePoolWithTag MzfExAllocatePoolWithTag = (pfnExAllocatePoolWithTag)GetGlobalVeriable(enumExAllocatePoolWithTag);
	pfnExFreePoolWithTag MzfExFreePoolWithTag = (pfnExFreePoolWithTag)GetGlobalVeriable(enumExFreePoolWithTag);

	KdPrint(("ListProcessWnds-> pOutBuffer: 0x%08X\n", pOutBuffer));

	if (!MzfNtUserQueryWindow || !MzfExAllocatePoolWithTag || !MzfExFreePoolWithTag)
	{
		return status;
	}

	// 这个函数pOutBuffer和nOutLen都不能为空
	if (!pInBuffer || 
		uInSize != sizeof(OPERATE_TYPE) || 
		!pOutBuffer ||
		nOutLen < sizeof(ALL_WNDS)
		)
	{
		return STATUS_INVALID_PARAMETER;
	}

	if (WinVersion == enumWINDOWS_8)
	{
		pfnNtUserBuildHwndListWin8 MzfNtUserBuildHwndListWin8 = (pfnNtUserBuildHwndListWin8)GetGlobalVeriable(enumNtUserBuildHwndList);
		if (!MzfNtUserBuildHwndListWin8)
		{
			return status;
		}

		status = MzfNtUserBuildHwndListWin8(
			NULL,
			NULL,
			FALSE,
			1,
			0,
			(nOutLen - sizeof(ALL_WNDS)) / sizeof(WND_INFO),
			(PVOID)((ULONG)pOutBuffer + sizeof(ULONG)),
			(ULONG*)pOutBuffer);
	}
	else
	{
		pfnNtUserBuildHwndList MzfNtUserBuildHwndList = (pfnNtUserBuildHwndList)GetGlobalVeriable(enumNtUserBuildHwndList);
		if (!MzfNtUserBuildHwndList)
		{
			return status;
		}

		status = MzfNtUserBuildHwndList(
			NULL,
			NULL,
			FALSE,
			0,
			(nOutLen - sizeof(ALL_WNDS)) / sizeof(WND_INFO),
			(PVOID)((ULONG)pOutBuffer + sizeof(ULONG)),
			(ULONG*)pOutBuffer);
	}
	
	KdPrint(("MzfNtUserBuildHwndList failed, Status: 0x%08X, error code: %d\n", status, RtlNtStatusToDosError(status)));

	if (NT_SUCCESS(status))
	{
		DWORD nCnt = *((DWORD*)pOutBuffer);
		ULONG i = 0;
		HWND*pHwndBuffer = (HWND*)MzfExAllocatePoolWithTag(NonPagedPool, sizeof(HWND) * nCnt, MZFTAG);

		KdPrint(("MzfNtUserBuildHwndList success-> wnd cnt: %d\n", nCnt));

		if (pHwndBuffer)
		{
			PALL_WNDS pWnds = (PALL_WNDS)pOutBuffer;
			memcpy(pHwndBuffer, (PVOID)((ULONG)pOutBuffer + sizeof(ULONG)), sizeof(HWND) * nCnt);
			
			for (i = 0; i < nCnt; i++)
			{
				ULONG Tid = 0, Pid = 0;
				HWND hWnd = pHwndBuffer[i];
				
				Pid = MzfNtUserQueryWindow(hWnd, 0);

				if (WinVersion == enumWINDOWS_7 ||
					WinVersion == enumWINDOWS_8)
				{
					Tid = MzfNtUserQueryWindow(hWnd, 2);
				}
				else
				{
					Tid = MzfNtUserQueryWindow(hWnd, 1);
				}
				
				pWnds->WndInfo[i].hWnd = hWnd;
				pWnds->WndInfo[i].uPid = Pid;
				pWnds->WndInfo[i].uTid = Tid;
			}
			
			pWnds->nCnt = nCnt;

			MzfExFreePoolWithTag(pHwndBuffer, 0);
		}
	}

	return status;
}