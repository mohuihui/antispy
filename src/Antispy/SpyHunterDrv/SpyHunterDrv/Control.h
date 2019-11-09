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
#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <ntifs.h>
#include <windef.h>

// NTSTATUS CommunicatControl(PVOID pInOutBuffer, DWORD dwInLen, DWORD dwOutLen, DWORD *dwRet);
NTSTATUS CommunicatNeitherControl(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);

#endif