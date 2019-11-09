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
// from: http://blog.csdn.net/cooblily/archive/2008/02/04/2080822.aspx

#ifndef _IRP_OPERATE_H_
#define _IRP_OPERATE_H_

#include <ntifs.h>
#include <windef.h>

//
// IoCompletionRoutine
//
// This routine is used to handle I/O (read OR write) completion
//
// Inputs:
// DeviceObject - not used
// Irp - the I/O operation being completed
// Context - not used
//
// Outputs:
// None.
//
// Returns:
// STATUS_MORE_PROCESSING_REQUIRED
//
// Notes:
// The purpose of this routine is to do "cleanup" on I/O operations
// so we don''t constantly throw away perfectly good MDLs as part of
// completion processing.
//
NTSTATUS
IoCompletionRoutine(
					IN PDEVICE_OBJECT DeviceObject,
					IN PIRP Irp,
					IN PVOID Context
					);

//
// IrpCreateFile
//
// This routine is used as NtCreateFile but first and third parameter.
//
// Inputs:
// DesiredAccess - Specifies an ACCESS_MASK value that determines
//                     the requested access to the object. 
// FilePath - Path of file to create,as L"C:\\Windows"(Unicode).
// AllocationSize - Pointer to a LARGE_INTEGER that contains the initial allocation
//                     size, in bytes, for a file that is created or overwritten.
// FileAttributes - Specifies one or more FILE_ATTRIBUTE_XXX flags, which represent
//                     the file attributes to set if you are creating or overwriting a file.
// ShareAccess - Type of share access.
// CreateDisposition - Specifies the action to perform if the file does or does not exist.
// CreateOptions - Specifies the options to apply when creating or opening the file. 
// EaBuffer - For device and intermediate drivers, this parameter must be a NULL pointer. 
// EaLength - For device and intermediate drivers, this parameter must be zero.
//
// Ouputs:
// FileObject - Pointer to a PFILE_OBJECT variable that receives a PFILE_OBJECT to the file.
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtCreateFile,but return FILE_OBJECT not HANDLE.
//
NTSTATUS
IrpCreateFile(
			  IN PUNICODE_STRING FilePath,
			  IN ACCESS_MASK DesiredAccess,
			  IN ULONG FileAttributes,
			  IN ULONG ShareAccess,
			  IN ULONG CreateDisposition,
			  IN ULONG CreateOptions,
			  IN PDEVICE_OBJECT DeviceObject,
			  IN PDEVICE_OBJECT RealDevice,
			  OUT PFILE_OBJECT *FileObject
			  );

//
// IrpClose
//
// This routine is used as ObDereferenceObject.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT variable that will close
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to ObDereferenceObject
//
NTSTATUS
IrpClose(
		 IN PFILE_OBJECT FileObject
		 );

//
// IrpQueryDirectoryFile
//
// This routine is used as NtQueryDirectoryFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Length - Size, in bytes, of the buffer pointed to by FileInformation. The caller
//            should set this parameter according to the given FileInformationClass. 
// FileInformationClass - Type of information to be returned about files in the directory. 
// FileName - Pointer to a caller-allocated Unicode string containing the name of a file
//            (or multiple files, if wildcards are used) within the directory specified by FileHandle.
//            This parameter is optional and can be NULL. 
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation. 
// FileInformation - Pointer to a buffer that receives the desired
//                          information about the file. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtQueryDirectoryFile, but no ApcRoutine.
//
NTSTATUS
IrpQueryDirectoryFile(
					  IN PFILE_OBJECT FileObject,
					  OUT PIO_STATUS_BLOCK IoStatusBlock,
					  OUT PVOID FileInformation,
					  IN ULONG Length,
					  IN FILE_INFORMATION_CLASS FileInformationClass,
					  IN PUNICODE_STRING FileName OPTIONAL
					  );

//
// IrpQueryInformationFile
//
// This routine is used as NtQueryInformationFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Length - Size, in bytes, of the buffer pointed to by FileInformation. The caller
//            should set this parameter according to the given FileInformationClass. 
// FileInformationClass - Type of information to be returned about files in the directory. 
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation. 
// FileInformation - Pointer to a buffer that receives the desired
//                          information about the file. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtQueryInformationFile.
//
NTSTATUS
IrpQueryInformationFile(
						IN PFILE_OBJECT FileObject,
						IN PDEVICE_OBJECT DeviceObject,
						OUT PVOID FileInformation,
						IN ULONG Length,
						IN FILE_INFORMATION_CLASS FileInformationClass);

//
// IrpSetInformationFile
//
// This routine is used as NtSetInformationFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// FileInformation - Pointer to a buffer that contains the information to set for the file. 
// Length - Size, in bytes, of the buffer pointed to by FileInformation. The caller
//            should set this parameter according to the given FileInformationClass. 
// FileInformationClass - Type of information to be returned about files in the directory. 
// ReplaceIfExists - Set to TRUE to specify that if a file with the same name already exists,
//                     it should be replaced with the given file. Set to FALSE if the rename
//                     operation should fail if a file with the given name already exists. 
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtSetInformationFile.
//
NTSTATUS
IrpSetInformationFile(
					  IN PFILE_OBJECT FileObject,
					  OUT PIO_STATUS_BLOCK IoStatusBlock,
					  IN PVOID FileInformation,
					  IN ULONG Length,
					  IN FILE_INFORMATION_CLASS FileInformationClass,
					  IN BOOLEAN ReplaceIfExists
					  );

//
// IrpReadFile
//
// This routine is used as NtReadFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Buffer - Pointer to a caller-allocated buffer that receives the data read from the file.
// Length - The size, in bytes, of the buffer pointed to by Buffer. 
// ByteOffset - Pointer to a variable that specifies the starting byte offset
//                 in the file where the read operation will begin. 
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                completion status and information about the requested read operation. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtReadFile, but no ApcRoutine.
//
NTSTATUS
IrpReadFile(
			IN PFILE_OBJECT FileObject,
			IN PDEVICE_OBJECT DeviceObject,
			OUT PIO_STATUS_BLOCK IoStatusBlock,
			OUT PVOID Buffer,
			IN ULONG Length,
			IN PLARGE_INTEGER ByteOffset OPTIONAL);

//
// IrpReadFile
//
// This routine is used as NtReadFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Buffer - Pointer to a caller-allocated buffer that contains the data to write to the file. 
// Length - The size, in bytes, of the buffer pointed to by Buffer. 
// ByteOffset - Pointer to a variable that specifies the starting byte offset
//                 in the file for beginning the write operation.
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation. 
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtReadFile, but no ApcRoutine.
//
NTSTATUS 
IrpWriteFile(
			 IN PFILE_OBJECT FileObject,
			 OUT PIO_STATUS_BLOCK IoStatusBlock,
			 IN PVOID Buffer,
			 IN ULONG Length,
			 IN PLARGE_INTEGER ByteOffset OPTIONAL
			 );




/////////////////功能同Irpxxx，但是ex函数会恢复FSD的钩子//////////////////////////

NTSTATUS IrpCreateFileEx( 
						 IN PUNICODE_STRING FilePath, 
						 IN ACCESS_MASK DesiredAccess, 
						 IN ULONG FileAttributes, 
						 IN ULONG ShareAccess, 
						 IN ULONG CreateDisposition, 
						 IN ULONG CreateOptions, 
						 IN PDEVICE_OBJECT DeviceObject, 
						 IN PDEVICE_OBJECT RealDevice, 
						 OUT PFILE_OBJECT *FileObject 
						 );


NTSTATUS IrpQueryDirectoryFileEx( 
								 IN HANDLE hFile, 
								 OUT PVOID FileInformation, 
								 IN ULONG Length, 
								 IN FILE_INFORMATION_CLASS FileInformationClass, 
								 IN BOOLEAN ReturnSingleEntry, 
								 IN BOOLEAN RestartScan, 
								 OUT PULONG pWrittenLen
								 );
#endif