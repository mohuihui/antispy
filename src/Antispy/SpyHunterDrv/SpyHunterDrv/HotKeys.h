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
#ifndef _HOT_KEYS_H_
#define _HOT_KEYS_H_

#include "Struct.h"

//NTSTATUS ListHotKeys(PVOID pOutBuffer, ULONG nLen, ULONG *nRet);
NTSTATUS RemoveHotKey(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
NTSTATUS ListHotKeys(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet);
#endif
