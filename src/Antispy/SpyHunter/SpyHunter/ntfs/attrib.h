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
 * attrib.h - Exports for attribute handling.
 */

#ifndef _NTFS_ATTRIB_H
#define _NTFS_ATTRIB_H

/* Forward declarations */
typedef struct _ntfs_attr ntfs_attr;
typedef struct _ntfs_attr_search_ctx ntfs_attr_search_ctx;

#include "types.h"
#include "inode.h"
#include "unistr.h"
#include "runlist.h"
#include "volume.h"
#include "debug.h"
#include "logging.h"

extern ntfschar AT_UNNAMED[];

//////////////////////////////////////////////////////////////////////////


/**
 * enum ntfs_lcn_special_values - special return values for ntfs_*_vcn_to_lcn()
 *
 * Special return values for ntfs_rl_vcn_to_lcn() and ntfs_attr_vcn_to_lcn().
 *
 * TODO: Describe them.
 */
#pragma pack(1)
typedef enum {
	LCN_HOLE		= -1,	/* Keep this as highest value or die! */
	LCN_RL_NOT_MAPPED	= -2,
	LCN_ENOENT		= -3,
	LCN_EINVAL		= -4,
	LCN_EIO			= -5,
} ntfs_lcn_special_values;
#pragma pack()


//
// 用于查找属性函数的结构体
//
#pragma pack(1)
struct _ntfs_attr_search_ctx {
	MFT_RECORD *mrec;			// 要查找的MFT
	ATTR_RECORD *attr;			// attribute record in @mrec where to begin/continue search
	BOOL is_first;				// if true lookup_attr() begins search with @attr, else after @attr
	ntfs_inode *ntfs_ino;		
	ATTR_LIST_ENTRY *al_entry;
	ntfs_inode *base_ntfs_ino;
	MFT_RECORD *base_mrec;
	ATTR_RECORD *base_attr;
};
#pragma pack()


extern void ntfs_attr_reinit_search_ctx(ntfs_attr_search_ctx *ctx);
extern ntfs_attr_search_ctx *ntfs_attr_get_search_ctx(ntfs_inode *ni,
		MFT_RECORD *mrec);
extern void ntfs_attr_put_search_ctx(ntfs_attr_search_ctx *ctx);

extern int ntfs_attr_lookup(const ATTR_TYPES type, const ntfschar *name,
		const u32 name_len, const IGNORE_CASE_BOOL ic,
		const VCN lowest_vcn, const u8 *val, const u32 val_len,
		ntfs_attr_search_ctx *ctx);

extern ATTR_DEF *ntfs_attr_find_in_attrdef(const ntfs_volume *vol,
		const ATTR_TYPES type);

/**
 * ntfs_attrs_walk - syntactic sugar for walking all attributes in an inode
 * @ctx:	initialised attribute search context
 *
 * Syntactic sugar for walking attributes in an inode.
 *
 * Return 0 on success and -1 on error with errno set to the error code from
 * ntfs_attr_lookup().
 *
 * Example: When you want to enumerate all attributes in an open ntfs inode
 *	    @ni, you can simply do:
 *
 *	int err;
 *	ntfs_attr_search_ctx *ctx = ntfs_attr_get_search_ctx(ni, NULL);
 *	if (!ctx)
 *		// Error code is in errno. Handle this case.
 *	while (!(err = ntfs_attrs_walk(ctx))) {
 *		ATTR_RECORD *attr = ctx->attr;
 *		// attr now contains the next attribute. Do whatever you want
 *		// with it and then just continue with the while loop.
 *	}
 *	if (err && errno != ENOENT)
 *		// Ooops. An error occurred! You should handle this case.
 *	// Now finished with all attributes in the inode.
 */
static int ntfs_attrs_walk(ntfs_attr_search_ctx *ctx)
{
	return ntfs_attr_lookup(AT_UNUSED, NULL, 0, CASE_SENSITIVE, 0, NULL, 0, ctx);
}



//
// 该结构体用来缓存 内存中的非常驻属性
//
#pragma pack(1)
struct _ntfs_attr {
	runlist_element *rl;	// 数据运行
	ntfs_inode *ni;			// base ntfs inode to which this attribute belongs
	ATTR_TYPES type;		// 属性类型 eg. 0x90H
	ntfschar *name;			// 属性名 unicode格式
	u32 name_len;			// 属性名长度
	unsigned long state;	// NTFS attribute specific flags describing this attribute

	// 下面这些都是从该属性中copy出来的
	s64 allocated_size;		// 分配的大小
	s64 data_size;			// 实际大小
	s64 initialized_size;	// 初始化大小
	s64 compressed_size;	// 压缩大小	
	u32 compression_block_size;
	u8 compression_block_size_bits;
	u8 compression_block_clusters;
};
#pragma pack()

/**
 * enum ntfs_attr_state_bits - bits for the state field in the ntfs_attr structure
 */
typedef enum {
	NA_Initialized,		/* 1: structure is initialized. */
	NA_NonResident,		/* 1: Attribute is not resident. */
} ntfs_attr_state_bits;

#define  test_nattr_flag(na, flag)	 test_bit(NA_##flag, (na)->state)
#define  set_nattr_flag(na, flag)	 set_bit(NA_##flag, (na)->state)
#define  clear_nattr_flag(na, flag)	 clear_bit(NA_##flag, (na)->state)

#define NAttrInitialized(na)		 test_nattr_flag(na, Initialized)
#define NAttrSetInitialized(na)		 set_nattr_flag(na, Initialized)
#define NAttrClearInitialized(na)	 clear_nattr_flag(na, Initialized)

#define NAttrNonResident(na)		 test_nattr_flag(na, NonResident)
#define NAttrSetNonResident(na)		 set_nattr_flag(na, NonResident)
#define NAttrClearNonResident(na)	 clear_nattr_flag(na, NonResident)

// 
// #define GenNAttrIno(func_name,flag)				\
// static inline int NAttr##func_name(ntfs_attr *na)		\
// {								\
// 	if (na->type == AT_DATA && na->name == AT_UNNAMED)	\
// 		return (na->ni->flags & FILE_ATTR_##flag);	\
// 	return 0;						\
// }								\
// static inline void NAttrSet##func_name(ntfs_attr *na)		\
// {								\
// 	if (na->type == AT_DATA && na->name == AT_UNNAMED)	\
// 		na->ni->flags |= FILE_ATTR_##flag;		\
// 	else							\
// 		ntfs_log_trace("BUG! Should be called only for "\
// 			"unnamed data attribute.\n");		\
// }								\
// static inline void NAttrClear##func_name(ntfs_attr *na)		\
// {								\
// 	if (na->type == AT_DATA && na->name == AT_UNNAMED)	\
// 		na->ni->flags &= ~FILE_ATTR_##flag;		\
// }
// 
// GenNAttrIno(Compressed, COMPRESSED)
// GenNAttrIno(Encrypted, ENCRYPTED)
// GenNAttrIno(Sparse, SPARSE_FILE)


// (Compressed, COMPRESSED)
int NAttrCompressed(ntfs_attr *na);	
void NAttrSetCompressed(ntfs_attr *na);								
void NAttrClearCompressed(ntfs_attr *na);

// (Encrypted, ENCRYPTED)
int NAttrEncrypted(ntfs_attr *na);
void NAttrSetEncrypted(ntfs_attr *na);
void NAttrClearEncrypted(ntfs_attr *na);

// (Sparse, SPARSE_FILE)
int NAttrSparse(ntfs_attr *na);	
void NAttrSetSparse(ntfs_attr *na);							
void NAttrClearSparse(ntfs_attr *na);

/**
 * union attr_val - Union of all known attribute values
 *
 * For convenience. Used in the attr structure.
 */
#pragma pack(1)
typedef union {
	u8 _default;	/* Unnamed u8 to serve as default when just using
			   a_val without specifying any of the below. */
	STANDARD_INFORMATION std_inf;
	ATTR_LIST_ENTRY al_entry;
	FILE_NAME_ATTR filename;
	OBJECT_ID_ATTR obj_id;
	SECURITY_DESCRIPTOR_ATTR sec_desc;
	VOLUME_NAME vol_name;
	VOLUME_INFORMATION vol_inf;
	DATA_ATTR data;
	INDEX_ROOT index_root;
	INDEX_BLOCK index_blk;
	BITMAP_ATTR bmp;
	REPARSE_POINT reparse;
	EA_INFORMATION ea_inf;
	EA_ATTR ea;
// 	PROPERTY_SET property_set;
// 	LOGGED_UTILITY_STREAM logged_util_stream;
	EFS_ATTR_HEADER efs;
} attr_val;
#pragma pack()

extern void ntfs_attr_init(ntfs_attr *na, const BOOL non_resident,
		const BOOL compressed, const BOOL encrypted, const BOOL sparse,
		const s64 allocated_size, const s64 data_size,
		const s64 initialized_size, const s64 compressed_size,
		const u8 compression_unit);

extern ntfs_attr *ntfs_attr_open(ntfs_inode *ni, const ATTR_TYPES type,
		ntfschar *name, u32 name_len);
extern void ntfs_attr_close(ntfs_attr *na);

extern s64 ntfs_attr_pread(ntfs_attr *na, const s64 pos, s64 count,
		void *b);
extern s64 ntfs_attr_pwrite(ntfs_attr *na, const s64 pos, s64 count,
		const void *b);

extern void *ntfs_attr_readall(ntfs_inode *ni, const ATTR_TYPES type,
			       ntfschar *name, u32 name_len, s64 *data_size);

extern s64 ntfs_attr_mst_pread(ntfs_attr *na, const s64 pos,
		const s64 bk_cnt, const u32 bk_size, void *dst);
extern s64 ntfs_attr_mst_pwrite(ntfs_attr *na, const s64 pos,
		s64 bk_cnt, const u32 bk_size, void *src);

extern int ntfs_attr_map_runlist(ntfs_attr *na, VCN vcn);
extern int ntfs_attr_map_whole_runlist(ntfs_attr *na);

extern LCN ntfs_attr_vcn_to_lcn(ntfs_attr *na, const VCN vcn);
extern runlist_element *ntfs_attr_find_vcn(ntfs_attr *na, const VCN vcn);

extern int ntfs_attr_size_bounds_check(const ntfs_volume *vol,
		const ATTR_TYPES type, const s64 size);
extern int ntfs_attr_can_be_non_resident(const ntfs_volume *vol,
		const ATTR_TYPES type);
extern int ntfs_attr_can_be_resident(const ntfs_volume *vol,
		const ATTR_TYPES type);

extern int ntfs_make_room_for_attr(MFT_RECORD *m, u8 *pos, u32 size);

extern int ntfs_resident_attr_record_add(ntfs_inode *ni, ATTR_TYPES type,
		ntfschar *name, u8 name_len, u8 *val, u32 size,
		u16 flags);
extern int ntfs_non_resident_attr_record_add(ntfs_inode *ni, ATTR_TYPES type,
		ntfschar *name, u8 name_len, VCN lowest_vcn, int dataruns_size,
		u16 flags);
extern int ntfs_attr_record_rm(ntfs_attr_search_ctx *ctx);

extern int ntfs_attr_add(ntfs_inode *ni, ATTR_TYPES type,
		ntfschar *name, u8 name_len, u8 *val, s64 size);
extern int ntfs_attr_rm(ntfs_attr *na);

extern int ntfs_attr_record_resize(MFT_RECORD *m, ATTR_RECORD *a, u32 new_size);

extern int ntfs_resident_attr_value_resize(MFT_RECORD *m, ATTR_RECORD *a,
		const u32 new_size);

extern int ntfs_attr_record_move_to(ntfs_attr_search_ctx *ctx, ntfs_inode *ni);
extern int ntfs_attr_record_move_away(ntfs_attr_search_ctx *ctx, int extra);

extern int ntfs_attr_update_mapping_pairs(ntfs_attr *na, VCN from_vcn);

extern int ntfs_attr_truncate(ntfs_attr *na, const s64 newsize);

// FIXME / TODO: Above here the file is cleaned up. (AIA)
/**
 * get_attribute_value_length - return the length of the value of an attribute
 * @a:	pointer to a buffer containing the attribute record
 *
 * Return the byte size of the attribute value of the attribute @a (as it
 * would be after eventual decompression and filling in of holes if sparse).
 * If we return 0, check errno. If errno is 0 the actual length was 0,
 * otherwise errno describes the error.
 *
 * FIXME: Describe possible errnos.
 */
extern s64 ntfs_get_attribute_value_length(const ATTR_RECORD *a);

/**
 * get_attribute_value - return the attribute value of an attribute
 * @vol:	volume on which the attribute is present
 * @a:		attribute to get the value of
 * @b:		destination buffer for the attribute value
 *
 * Make a copy of the attribute value of the attribute @a into the destination
 * buffer @b. Note, that the size of @b has to be at least equal to the value
 * returned by get_attribute_value_length(@a).
 *
 * Return number of bytes copied. If this is zero check errno. If errno is 0
 * then nothing was read due to a zero-length attribute value, otherwise
 * errno describes the error.
 */
extern s64 ntfs_get_attribute_value(const ntfs_volume *vol, 
				    const ATTR_RECORD *a, u8 *b);

extern void  ntfs_attr_name_free(char **name);
extern char *ntfs_attr_name_get(const ntfschar *uname, const int uname_len);
extern int   ntfs_attr_exist(ntfs_inode *ni, const ATTR_TYPES type,
			     ntfschar *name, u32 name_len);
extern int   ntfs_attr_remove(ntfs_inode *ni, const ATTR_TYPES type,
			      ntfschar *name, u32 name_len);

#endif /* defined _NTFS_ATTRIB_H */

