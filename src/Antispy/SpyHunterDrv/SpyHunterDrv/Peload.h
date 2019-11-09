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
#ifndef _PE_LOAD_H_
#define _PE_LOAD_H_

#include "Struct.h"

BOOL ReloadNtoskrnl();
UINT AlignSize(UINT nSize, UINT nAlign);
BOOL PeLoad( WCHAR* szFilePath, PULONG NewBase, DWORD OriginBase );
BOOL IoGetFileSystemVpbInfo(IN PFILE_OBJECT FileObject, PDEVICE_OBJECT *DeviceObject, PDEVICE_OBJECT *RealDevice);
NTSTATUS KernelOpenFile( WCHAR *szFilePath, PHANDLE FileHandle, ACCESS_MASK DesiredAccess, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions );
NTSTATUS KernelGetFileSize(HANDLE hFile, PLARGE_INTEGER FileSize);
NTSTATUS KernelReadFile(HANDLE hFile, PLARGE_INTEGER ByteOffset, ULONG Length, PVOID FileBuffer, PIO_STATUS_BLOCK IoStatusBlock);
BOOL ImageFile(PVOID FileBuffer, PVOID* ImageModuleBase);
BOOL MzfMapFile( WCHAR* szFilePath, ULONG* NewBase, ULONG* NewEnd );
PVOID FindExportedRoutineInReloadModule ( IN ULONG DllBase, IN char *RoutineName );
PVOID MiFindExportedRoutine ( IN PVOID DllBase, BOOL ByName, IN char *RoutineName, DWORD Ordinal );
BOOL PeLoadWithoutFix(WCHAR* szFilePath, PULONG NewBase);
PVOID GetKernelModuleBase(CHAR* KernelModuleName);
BOOL GetWindowsRootName(WCHAR *szRootName);
BOOL GetDeviceObjectFromFileFullName(WCHAR *FileFullName, PDEVICE_OBJECT *RealDevice, PDEVICE_OBJECT *DeviceObject);
BOOL GetNtoskrnlInfo(WCHAR *szNtoskrnlPath, PDWORD SystemKernelModuleBase, PDWORD SystemKernelModuleSize);
BOOL GetWin32kInfo(WCHAR *szPath, PDWORD pModuleBase, PDWORD pModuleSize);
#endif