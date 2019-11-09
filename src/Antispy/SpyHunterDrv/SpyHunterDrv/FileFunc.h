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
#ifndef _FILE_FUNC_H
#define _FILE_FUNC_H

#include "Struct.h"

NTSTATUS MzfCreateFile( WCHAR *szFilePath, 
						PHANDLE FileHandle, 
						ACCESS_MASK DesiredAccess, 
						ULONG FileAttributes, 
						ULONG ShareAccess, 
						ULONG CreateDisposition, 
						ULONG CreateOptions 
						);

NTSTATUS MzfGetFileSize(HANDLE hFile, 
						   PLARGE_INTEGER FileSize
						   );

NTSTATUS MzfReadFile(HANDLE hFile, 
						PLARGE_INTEGER ByteOffset, 
						ULONG Length, 
						PVOID FileBuffer, 
						PIO_STATUS_BLOCK IoStatusBlock
						);

NTSTATUS MzfCreateFileEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
NTSTATUS MzfReadFileEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
NTSTATUS MzfWriteFileEx(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG uOutSize, ULONG* dwRet);
#endif