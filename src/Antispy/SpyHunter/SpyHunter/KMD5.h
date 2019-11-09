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
#if !defined(AFX_KMD5_H__FE736FB9_6F6A_47CC_9A60_CB429DEA268D__INCLUDED_)
#define AFX_KMD5_H__FE736FB9_6F6A_47CC_9A60_CB429DEA268D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tchar.h>

class MD5_CTX
{
public:
	MD5_CTX();
	~MD5_CTX();
	
	void MD5Update(const unsigned char* input, size_t inputLen);
	void MD5Final(unsigned char digest[16]);

private:
	unsigned long int state[4];					/* state (ABCD) */
	unsigned long int count[2];					/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];       /* input buffer */
	unsigned char PADDING[64];		/* What? */

private:
	void MD5Init ();
	void MD5Transform (unsigned long int state[4], const unsigned char block[64]);
	void MD5_memcpy (unsigned char* output, const unsigned char* input,unsigned int len);
	void Encode (unsigned char *output, const unsigned long int *input,unsigned int len);
	void Decode (unsigned long int *output, const unsigned char *input, unsigned int len);
	void MD5_memset (unsigned char* output,int value,unsigned int len);
};

#ifndef BYTE
typedef unsigned char BYTE;
#endif

static const int NMD5_SIZE = 16;

class KMD5
{
public:
// -------------------------------------------------------------------------
// 函数		:  KMD5::GetMD5Str
// 功能		: 获取一个文件的128位MD5串
// 返回值	: const TCHAR* MD5字符串的指针 
//					如：_TEXT("C5CE883D2DBDED17F46D24BC906D2A8F")
// 参数		: const TCHAR *pszFileName 文件名
// 附注		: 
// -------------------------------------------------------------------------
	const TCHAR* GetMD5Str(const TCHAR* pszFileName);
// -------------------------------------------------------------------------
// 函数		: const unsigned char* KMD5::GetMD5
// 功能		: 获取一个文件的128位MD5串
// 返回值	: BYTE MD5串的指针 128位
// 参数		: const TCHAR *pszFileName	文件名
// 附注		: 
// -------------------------------------------------------------------------
	const BYTE* GetMD5(const TCHAR* pszFileName);
	KMD5();
	virtual ~KMD5();

private:
	BYTE m_pbyMD5[NMD5_SIZE];			// MD5串
	TCHAR m_pszMD5[(NMD5_SIZE + 1) * 2];			// MD5字符串，以0结尾
	MD5_CTX* m_pAlgoMD5;	// MD5算法
	BYTE* m_pbyBuf;		// 文件缓冲
};

#endif 

