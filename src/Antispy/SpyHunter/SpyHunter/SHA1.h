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
	100% free public domain implementation of the SHA-1 algorithm
	by Dominik Reichl <dominik.reichl@t-online.de>

	Version 1.5 - 2005-01-01
	- 64-bit compiler compatibility added
	- Made variable wiping optional (define SHA1_WIPE_VARIABLES)
	- Removed unnecessary variable initializations
	- ROL32 improvement for the Microsoft compiler (using _rotl)

	======== Test Vectors (from FIPS PUB 180-1) ========

	SHA1("abc") =
		A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D

	SHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") =
		84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1

	SHA1(A million repetitions of "a") =
		34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#ifndef ___SHA1_HDR___
#define ___SHA1_HDR___

#include <stdio.h>  // Needed for file access
#include <memory.h> // Needed for memset and memcpy
#include <string.h> // Needed for strcat and strcpy

#ifdef _MSC_VER
#include <stdlib.h>
#endif

// If you're compiling big endian, just comment out the following line
#define SHA1_LITTLE_ENDIAN

// #define or #undef this, if you want the CSHA1 class to wipe all
// temporary variables after processing
#define SHA1_WIPE_VARIABLES

/////////////////////////////////////////////////////////////////////////////
// Define 8- and 32-bit variables

#ifndef UINT_32

#ifdef _MSC_VER

#define UINT_8  unsigned __int8
#define UINT_32 unsigned __int32

#else

#define UINT_8 unsigned char

#if (ULONG_MAX == 0xFFFFFFFF)
#define UINT_32 unsigned long
#else
#define UINT_32 unsigned int
#endif

#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Declare SHA1 workspace

typedef union
{
	UINT_8  c[64];
	UINT_32 l[16];
} SHA1_WORKSPACE_BLOCK;

class CSHA1
{
public:
	// Two different formats for ReportHash(...)
	enum
	{
		REPORT_HEX = 0,
		REPORT_DIGIT = 1
	};

	// Constructor and Destructor
	CSHA1();
	~CSHA1();

	UINT_32 m_state[5];
	UINT_32 m_count[2];
	UINT_8  m_buffer[64];
	UINT_8  m_digest[20];

	void Reset();

	// Update the hash value
	void Update(UINT_8 *data, UINT_32 len);
	bool HashFile(const TCHAR *szFileName, TCHAR *szOut);
	// Finalize hash and report
	void Final();
	void ReportHash(TCHAR *szReport, unsigned char uReportType = REPORT_HEX);
	void GetHash(UINT_8 *puDest);

private:
	// Private SHA-1 transformation
	void Transform(UINT_32 *state, UINT_8 *buffer);

	// Member variables
	UINT_8 m_workspace[64];
	SHA1_WORKSPACE_BLOCK *m_block; // SHA1 pointer to the byte array above
};

#endif
