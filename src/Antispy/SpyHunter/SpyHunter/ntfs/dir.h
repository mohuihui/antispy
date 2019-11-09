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
/*
 * dir.h - Exports for directory handling. Originated from the Linux-NTFS project.
 */

#ifndef _NTFS_DIR_H
#define _NTFS_DIR_H

#include <wchar.h>
#include "types.h"

#define PATH_SEP '\\'

/*
 * We do not have these under DJGPP, so define our version that do not conflict
 * with other S_IFs defined under DJGPP.
 */

#ifndef S_IFLNK
#define S_IFLNK  0120000
#endif
#ifndef S_ISLNK
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif


#define S_ISREG(m) (m == S_IFREG)

/*
 * The little endian Unicode strings $I30, $SII, $SDH, $O, $Q, $R
 * as a global constant.
 */
extern ntfschar NTFS_INDEX_I30[5];
extern ntfschar NTFS_INDEX_SII[5];
extern ntfschar NTFS_INDEX_SDH[5];
extern ntfschar NTFS_INDEX_O[3];
extern ntfschar NTFS_INDEX_Q[3];
extern ntfschar NTFS_INDEX_R[3];

//////////////////////////////////////////////////////////////////////////

extern BOOL
IsDirectory (
    IN ntfs_volume *vol, 
	IN const u64 mref
	) ;

extern VOID
ntfs_inode_Enum_name(
	IN ntfs_inode *dir_ni
	) ;

extern u64 ntfs_inode_lookup_by_name(ntfs_inode *dir_ni,
		const ntfschar *uname, const int uname_len);

extern ntfs_inode *ntfs_pathname_to_inode(ntfs_volume *vol, ntfs_inode *parent,
		const char *pathname);

extern ntfs_inode *ntfs_create(ntfs_inode *dir_ni, ntfschar *name, u8 name_len,
		dev_t type);
extern ntfs_inode *ntfs_create_device(ntfs_inode *dir_ni,
		ntfschar *name, u8 name_len, dev_t type, dev_t dev);
extern ntfs_inode *ntfs_create_symlink(ntfs_inode *dir_ni,
		ntfschar *name, u8 name_len, ntfschar *target, u8 target_len);
extern int ntfs_check_empty_dir(ntfs_inode *ni);
extern int ntfs_delete(ntfs_inode *ni, ntfs_inode *dir_ni, ntfschar *name,
		u8 name_len);

extern int ntfs_link(ntfs_inode *ni, ntfs_inode *dir_ni, ntfschar *name,
		u8 name_len);

/*
 * File types (adapted from include <linux/fs.h>)
 */
#define NTFS_DT_UNKNOWN		0
#define NTFS_DT_FIFO		1
#define NTFS_DT_CHR		2
#define NTFS_DT_DIR		4
#define NTFS_DT_BLK		6
#define NTFS_DT_REG		8
#define NTFS_DT_LNK		10
#define NTFS_DT_SOCK		12
#define NTFS_DT_WHT		14

/*
 * This is the "ntfs_filldir" function type, used by ntfs_readdir() to let
 * the caller specify what kind of dirent layout it wants to have.
 * This allows the caller to read directories into their application or
 * to have different dirent layouts depending on the binary type.
 */
typedef int (*ntfs_filldir_t)(void *dirent, const ntfschar *name,
		const int name_len, const int name_type, const s64 pos,
		const MFT_REF mref, const unsigned dt_type);

extern int ntfs_readdir(ntfs_inode *dir_ni, s64 *pos,
		void *dirent, ntfs_filldir_t filldir);

#endif /* defined _NTFS_DIR_H */

