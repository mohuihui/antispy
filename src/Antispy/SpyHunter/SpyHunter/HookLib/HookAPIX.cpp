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
// XHookAPI.cpp: implementation of the CXHookAPI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HookAPIX.h"
#pragma warning (disable : 4731) // “CXHookAPI::ThisCallFunction”: 框架指针寄存器“ebp”被内联程序集代码修改
namespace XLIB
{


#define MakePtr(cast, ptr, AddValue) (cast)((DWORD)(ptr)+(DWORD)(AddValue))


DWORD CStack_ESP::GetReturnAddress()
{
	return *LPDWORD(ESP);

}

DWORD CStack_ESP::GetArgument(DWORD index)
{
	return *LPDWORD(ESP+4+index*4);
}

LPVOID* CStack_ESP::GetArguList()
{
	return (LPVOID*)(ESP+4);
}

DWORD CStack_ESP::GetEflags()
{
	return eflags;
}

CXHookAPI::CXHookAPI()
{
	m_pOriginFunction = 0;
	m_dwOrginCodeLen = 0;
	m_HookedAddr = 0;
}

CXHookAPI::~CXHookAPI()
{

}

PIMAGE_IMPORT_DESCRIPTOR GetNamedImportDescriptor(HMODULE hModule, LPCSTR lplpszImportModuleule)   
{   
  
	if ((lplpszImportModuleule == NULL) || (hModule == NULL)) return NULL;  
 
   
  // Get the Dos header.    
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER) hModule;   
   
  // Is this the MZ header?    
	if (IsBadReadPtr(pDOSHeader, sizeof(IMAGE_DOS_HEADER)) || (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)) return NULL;     

   
	// Get the PE header.    
	PIMAGE_NT_HEADERS pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);   
   
  // Is this a real PE image?    
	if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) || (pNTHeader->Signature != IMAGE_NT_SIGNATURE))  return NULL;   

   
	// If there is no imports section, leave now.    
	if (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)return NULL;    
 
   
	// Get the pointer to the imports section.    
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader,   
                                                 pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].   
                                                 VirtualAddress);   
   
	// Loop through the import module descriptors looking for the module whose name matches lplpszImportModuleule.    
	while (pImportDesc->Name)   
	{   
		PSTR szCurrMod = MakePtr(PSTR, pDOSHeader, pImportDesc->Name);   
   
		if (lstrcmpiA(szCurrMod, lplpszImportModuleule) == 0)break; // Found it.      
       
   
		// Look at the next one.    
		pImportDesc++;   
	}   
	// If the name is NULL, then the module is not imported.    
	if (pImportDesc->Name == NULL)return NULL;   
       
	// All OK, Jumpmaster!    
	return pImportDesc;   
}   

LPVOID CXHookAPI::GetAddressByNameA( LPCSTR lpszImportModule,LPCSTR lpszFunctionName )
{
	HMODULE hModule = LoadLibraryA(lpszImportModule);
	if( 0 == hModule )return 0;

	return GetProcAddress( hModule,lpszFunctionName );
}

BOOL CXHookAPI::HookVirtualFunction( LPVOID lpThisPointer,int vIndex,LPVOID lpNewFun,BOOL bCanUnHook )
{
	LPVOID* lpVirtaulTable;

	lpVirtaulTable = *( (LPVOID**)lpThisPointer );
	if( 0 == lpVirtaulTable )return FALSE;

	m_dwOrginCodeLen = 0;
	m_HookedAddr = 0;

	m_pOriginFunction = (DWORD)lpVirtaulTable[vIndex];
	if( m_pOriginFunction == (DWORD)lpNewFun )return FALSE;

	if( bCanUnHook )
	{
		m_HookedAddr = &lpVirtaulTable[vIndex];
		m_dwOrginCodeLen = sizeof(LPVOID);
		CopyMemory( m_pOriginCode,m_HookedAddr,m_dwOrginCodeLen );
	} 

	return WriteMemory( &lpVirtaulTable[vIndex],&lpNewFun,sizeof(LPVOID) );
}

BOOL CXHookAPI::HookImportTable( HMODULE hModule, LPCSTR lpszImportModule,LPCSTR lpszFunctionName,LPVOID lpNewFunction )
{
	DWORD dwFunctionAddress = 0;

	if( 0 == hModule )hModule = GetModuleHandle(0);
	if( 0 == hModule )return FALSE;
 	if( 0 == lpszImportModule || 0 == lpszFunctionName)return FALSE;

	// Get the specific import descriptor.    
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = GetNamedImportDescriptor(hModule, lpszImportModule);   
	if (pImportDesc == NULL)return FALSE; // The requested module was not imported.    
   
	// Get the original thunk information for this DLL.  I cannot use    
	//  the thunk information stored in the pImportDesc->FirstThunk    
	//  because the that is the array that the loader has already    
	//  bashed to fix up all the imports.  This pointer gives us acess    
	//  to the function names.    

	// Get the array pointed to by the pImportDesc->FirstThunk.  This is    
	//  where I will do the actual bash.    
	PIMAGE_THUNK_DATA pRealThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->FirstThunk);   
	PIMAGE_THUNK_DATA pOrigThunk = 0;
	
	if( pImportDesc->OriginalFirstThunk )//OriginalFirstThunk有可能为0
	{
		pOrigThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->OriginalFirstThunk);   
	}else
	{
		dwFunctionAddress = (DWORD)GetAddressByNameA( lpszImportModule,lpszFunctionName );
	}
	
	
	BOOL bDoHook = FALSE;  
	// Loop through and look for the one that matches the name.    
	while (1)   
	{
		if( pOrigThunk && 0 == pOrigThunk->u1.AddressOfData )break;
		else if( 0 == pRealThunk->u1.Function )break;

		if( pOrigThunk )
		{
			// Only look at those that are imported by name, not ordinal.    
			if (IMAGE_ORDINAL_FLAG != (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))   
			{   
				// Look get the name of this imported function.    
				PIMAGE_IMPORT_BY_NAME pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, hModule, pOrigThunk->u1.AddressOfData);   
   
				// If the name starts with NULL, then just skip out now.    
				if( 0 == pByName->Name[0]) continue;
				// Determines if we do the hook.    
				if( lstrcmpiA( lpszFunctionName,(char*)pByName->Name ) == 0 )bDoHook = TRUE;
			}
		}else
		{
			if( dwFunctionAddress == (DWORD)pRealThunk->u1.Function )
			{
				bDoHook = TRUE;
			}

		}
			// TODO TODO    
			//  Might want to consider bsearch here.    
   
			// See if the particular function name is in the import    
			//  list.  It might be good to consider requiring the    
			//  paHookArray to be in sorted order so bsearch could be    
			//  used so the lookup will be faster.  However, the size of    
			//  uiCount coming into this function should be rather    
			//  small but it is called for each function imported by    
			//  lpszImportModule.  
			if( bDoHook )
			{
 
				// I found it.  Now I need to change the protection to    
				//  writable before I do the blast.  Note that I am now    
				//  blasting into the real thunk area!    
				MEMORY_BASIC_INFORMATION mbi_thunk;   
				VirtualQuery(pRealThunk, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));   
				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect);   
				
				//保存hook前的代码
				m_HookedAddr = &pRealThunk->u1.Function;
				m_dwOrginCodeLen = sizeof(LPVOID);
				CopyMemory( m_pOriginCode,m_HookedAddr,m_dwOrginCodeLen);

				// Save the original address if requested.    
				m_pOriginFunction = (DWORD)pRealThunk->u1.Function;
				
				// Do the actual hook.    
				*PDWORD(&pRealThunk->u1.Function) = (DWORD)lpNewFunction;   
				

				// Change the protection back to what it was before I blasted.    
				DWORD dwOldProtect;   
				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);   
				break;

			}   
 
   
		// Increment both tables.    
		if(pOrigThunk)pOrigThunk++;   
		pRealThunk++;   
	}   
 
	return bDoHook;   
}

BOOL CXHookAPI::HookExportTable( HMODULE hModule,LPCSTR lpszFunctionName,LPVOID lpNewFunction )
{	
	
	DWORD BaseAddress = NULL;
	IMAGE_DOS_HEADER * dosheader;
	IMAGE_OPTIONAL_HEADER * opthdr;
	PIMAGE_EXPORT_DIRECTORY exports;
 
	USHORT index=0 ; 
	ULONG  i;
	PUCHAR pFuncName = NULL;
	PULONG pAddressOfFunctions,pAddressOfNames;
	PUSHORT pAddressOfNameOrdinals;

 	if( 0 == lpszFunctionName || 0 == lpNewFunction)return FALSE;
	if( 0 == hModule )hModule = GetModuleHandle(0);

	BaseAddress= (DWORD)hModule;
 
	dosheader = (IMAGE_DOS_HEADER *)hModule;
	opthdr =(IMAGE_OPTIONAL_HEADER *) ((BYTE*)hModule+dosheader->e_lfanew+24);
	exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)dosheader+ opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
 
	pAddressOfFunctions=(ULONG*)((BYTE*)hModule+exports->AddressOfFunctions); //函数列表
	pAddressOfNames=(ULONG*)((BYTE*)hModule+exports->AddressOfNames);		//名字列表
	pAddressOfNameOrdinals=(USHORT*)((BYTE*)hModule+exports->AddressOfNameOrdinals); //名字相对应的函数列表中的地址项index
 
	//这个是查一下是按照什么方式（函数名称or函数序号）来查函数地址的
	DWORD dwNameOrdinals = (DWORD)lpszFunctionName;
	if ((dwNameOrdinals & 0xFFFF0000) == 0)
	{
		if( dwNameOrdinals > exports->NumberOfFunctions )return FALSE;
		dwNameOrdinals -= exports->Base;
		index=pAddressOfNameOrdinals[dwNameOrdinals];

		MEMORY_BASIC_INFORMATION mbi_thunk;   
		VirtualQuery( &pAddressOfFunctions[index], &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));   
		VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect);   
   
		m_HookedAddr = &pAddressOfFunctions[index];
		m_dwOrginCodeLen = sizeof(LPVOID);
		CopyMemory( m_pOriginCode,m_HookedAddr,m_dwOrginCodeLen);

		m_pOriginFunction = (DWORD)(PUCHAR)hModule + pAddressOfFunctions[index];
		pAddressOfFunctions[index] = ( DWORD )lpNewFunction - BaseAddress;
				
		DWORD dwOldProtect;   
		VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);   
   
		return TRUE;
	}

	for (i = 0; i < exports->NumberOfNames; i++) 
	{
		index=pAddressOfNameOrdinals[i];
		pFuncName = (PUCHAR)( (BYTE*)hModule + pAddressOfNames[i]);
		if (lstrcmpiA( (char*)pFuncName,lpszFunctionName) == 0)
		{
			MEMORY_BASIC_INFORMATION mbi_thunk;   
			VirtualQuery( &pAddressOfFunctions[index], &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));   
			VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect);   
   
			m_HookedAddr = &pAddressOfFunctions[index];
			m_dwOrginCodeLen = sizeof(LPVOID);
			CopyMemory( m_pOriginCode,m_HookedAddr,m_dwOrginCodeLen);

			m_pOriginFunction = (DWORD)(PUCHAR)hModule + pAddressOfFunctions[index];
			pAddressOfFunctions[index] = ( DWORD )lpNewFunction - BaseAddress;
				
			DWORD dwOldProtect;   
			VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);   
   

			return TRUE;
		}
 
	}
 
	return FALSE;
}




DWORD __stdcall GetOpCodeSize(BYTE* iptr0)
{
	BYTE* iptr = iptr0;

	DWORD f = 0;

prefix:
	BYTE b = *iptr++;

	f |= table_1[b];

	if (f&C_FUCKINGTEST)
		if (((*iptr)&0x38)==0x00)   // ttt
			f=C_MODRM+C_DATAW0;       // TEST
		else
			f=C_MODRM;                // NOT,NEG,MUL,IMUL,DIV,IDIV

	if (f&C_TABLE_0F)
	{
		b = *iptr++;
		f = table_0F[b];
	}

	if (f==C_ERROR)
	{
		//printf("error in %02X\n",b);
		return C_ERROR;
	}

	if (f&C_PREFIX)
	{
		f&=~C_PREFIX;
		goto prefix;
	}

	if (f&C_DATAW0) if (b&0x01) f|=C_DATA66; else f|=C_DATA1;

	if (f&C_MODRM)
	{
		b = *iptr++;
		BYTE mod = b & 0xC0;
		BYTE rm  = b & 0x07;
		if (mod!=0xC0)
		{
			if (f&C_67)         // modrm16
			{
				if ((mod==0x00)&&(rm==0x06)) f|=C_MEM2;
				if (mod==0x40) f|=C_MEM1;
				if (mod==0x80) f|=C_MEM2;
			}
			else                // modrm32
			{
				if (mod==0x40) f|=C_MEM1;
				if (mod==0x80) f|=C_MEM4;
				if (rm==0x04) rm = (*iptr++) & 0x07;    // rm<-sib.base
				if ((rm==0x05)&&(mod==0x00)) f|=C_MEM4;
			}
		}
	} // C_MODRM

	if (f&C_MEM67)  if (f&C_67) f|=C_MEM2;  else f|=C_MEM4;
	if (f&C_DATA66) if (f&C_66) f|=C_DATA2; else f|=C_DATA4;

	if (f&C_MEM1)  iptr++;
	if (f&C_MEM2)  iptr+=2;
	if (f&C_MEM4)  iptr+=4;

	if (f&C_DATA1) iptr++;
	if (f&C_DATA2) iptr+=2;
	if (f&C_DATA4) iptr+=4;

	return (DWORD)(iptr - iptr0);
}

int CXHookAPI::CopyCodeToBuffer( LPVOID lpCode,LPVOID lpBuffer,int*	nCodeLen )//返回复制代码的长度
{
	DWORD	dwCodeLen = 0;
	LPBYTE	pCode = (LPBYTE)lpCode;
	LPBYTE	pBuffer = (LPBYTE)lpBuffer;
	DWORD	dwCodeOffset = 0;
	DWORD	dwBuffOffset = 0;

	while( dwCodeOffset < 5 )
	{
		dwCodeLen = GetOpCodeSize(&pCode[dwCodeOffset]);
		CopyMemory( &pBuffer[dwBuffOffset],&pCode[dwCodeOffset],dwCodeLen );

		if( 0xEB == pBuffer[dwBuffOffset] )//jmp short
		{
			pBuffer[dwBuffOffset] = 0xE9;
			FILL_JMP( &pBuffer[dwBuffOffset],&pBuffer[dwBuffOffset],MAKE_JMPSHORT_ADDRESS(&pCode[dwCodeOffset]) );
			
			dwCodeOffset += dwCodeLen;	
			dwCodeLen = 5;
			dwBuffOffset += dwCodeLen;
			continue;
		}else if( 0xe9 == pBuffer[dwBuffOffset] )//jmp far
		{
			FILL_JMP( &pBuffer[dwBuffOffset],&pBuffer[dwBuffOffset],MAKE_JMP_ADDRESS(&pCode[dwCodeOffset]) );
		}else if( 0xe8 == pBuffer[dwBuffOffset] )//call far
		{
			FILL_JMP( &pBuffer[dwBuffOffset],&pBuffer[dwBuffOffset],MAKE_JMP_ADDRESS(&pCode[dwCodeOffset]) );
		}

		dwBuffOffset += dwCodeLen;
		dwCodeOffset += dwCodeLen;
	}
	if(nCodeLen)*nCodeLen = dwCodeOffset;
	return dwBuffOffset;
}
/*
HookAddress( LPVOID lpAddress,FILTERROUTINE pRoutine )
lpAddress:

	jmp m_pTempCode

m_pTempCode:


00402411      60            PUSHAD
00402410 >    9C            PUSHFD
00402412      54            PUSH ESP
00402413      E8 E8DBFFFF   CALL pRoutine
00402418      83C4 04       ADD ESP,4
0040241C      9D            POPFD
0040241B      61            POPAD
; m_pOriginFunction is here
	lpAddress前几个字节内容
0040241D    - E9 DEDBFFFF   JMP lpAddress+size




*/

BOOL CXHookAPI::InlineHookAddress( LPVOID lpAddress,FILTERROUTINE pRoutine )
{
	int		nPos,nLen;
	int		nCodelen = 0;
	LPBYTE	lpCode = (LPBYTE)lpAddress;
	DWORD	dwOldProtect = 0;  
	BOOL	bRet;

	if( 0 == lpAddress || 0 == pRoutine)return FALSE;

	nPos = 0;

	m_pTempCode[nPos++] = 0x60;//PUSHAD
	m_pTempCode[nPos++] = 0x9c;//PUSHFD
	m_pTempCode[nPos++] = 0x54;//PUSH ESP
	m_pTempCode[nPos] = 0xe8;//CALL
	FILL_JMP( &m_pTempCode[nPos],&m_pTempCode[nPos],pRoutine );
	nPos +=5;
	m_pTempCode[nPos++] = 0x83;//ADD ESP,4
	m_pTempCode[nPos++] = 0xC4;
	m_pTempCode[nPos++] = 0x04;
	m_pTempCode[nPos++] = 0x9D;//POPFD
	m_pTempCode[nPos++] = 0x61;//POPAD

	m_pOriginFunction = (DWORD)&m_pTempCode[nPos];
	//将lpFunction前面大于?个字节复制到m_pTempCode
	nLen = CopyCodeToBuffer( lpCode,&m_pTempCode[nPos],&nCodelen );
 	nPos += nLen;

	//存保原始代码,以后恢复
	m_HookedAddr = lpCode;
	m_dwOrginCodeLen = nLen;
	CopyMemory( m_pOriginCode,m_HookedAddr,m_dwOrginCodeLen);

	m_pTempCode[nPos] = 0xe9;//jmp
	FILL_JMP( &m_pTempCode[nPos],&m_pTempCode[nPos],(DWORD)lpCode+5);
	bRet = VirtualProtect( m_pTempCode,sizeof(m_pTempCode), PAGE_EXECUTE_READWRITE, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;

	bRet = VirtualProtect( lpCode,nLen,PAGE_EXECUTE_READWRITE, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;
	
	//修改lpFunction前5字节为jmp xxxx
	memset( lpCode,0x90,nCodelen );
	lpCode[0] = 0xe9;//jmp
	FILL_JMP( lpCode,lpCode,m_pTempCode );
	bRet = VirtualProtect( lpCode,nLen,dwOldProtect, &dwOldProtect);
	if( FALSE == bRet )return FALSE;

	return TRUE;
}

/*
HookFunction( LPVOID lpFunction,LPVOID lpNewFunction )

lpFunction:
	jmp lpNewFunction

lpNewFunction:
CallFunction()
{
  m_pOriginFunction = m_pTempCode;
}

m_pTempCode:
	lpFunction前几个字节内容
	jmp lpFunction+5
*/
BOOL CXHookAPI::InlineHookFunction( LPVOID lpFunction,LPVOID lpNewFunction )
{
	int		nLen;
	LPBYTE	lpCode = (LPBYTE)lpFunction;
	DWORD	dwOldProtect = 0;  
	BOOL	bRet;
	int		nCodelen = 0;

	if( 0 == lpFunction || 0 == lpNewFunction)return FALSE;

	//将lpFunction前面大于?个字节复制到m_pTempCode
	nLen = CopyCodeToBuffer( lpFunction,m_pTempCode,&nCodelen );

	//存保原始代码,以后恢复
	m_HookedAddr = lpCode;
	m_dwOrginCodeLen = nLen;
	CopyMemory( m_pOriginCode,m_HookedAddr,m_dwOrginCodeLen);

 	//修改m_pTempCode为jmp xxxx
	m_pTempCode[nLen]=0xe9;//jmp
	FILL_JMP( &m_pTempCode[nLen],&m_pTempCode[nLen],(DWORD)lpFunction+5 );
	bRet = VirtualProtect( m_pTempCode,sizeof(m_pTempCode), PAGE_EXECUTE_READWRITE, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;

	m_pOriginFunction = (DWORD)m_pTempCode;


	bRet = VirtualProtect( lpCode,nLen,PAGE_EXECUTE_READWRITE, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;

	//修改lpFunction前?字节为jmp xxxx
	memset( lpCode,0x90,nCodelen );
	lpCode[0] = 0xe9;//jmp
	FILL_JMP( lpCode,lpCode,lpNewFunction );
	bRet = VirtualProtect( lpCode,nLen,dwOldProtect, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;

	return TRUE;
}

DWORD CXHookAPI::CallFunction(DWORD dwArguCount,...)
{
	DWORD	 dwOriginFunction = m_pOriginFunction;
	DWORD	dwRetValue;
	__asm
	{
		//保存寄存器
		pushad
		pushfd
		
		lea ebx,dwArguCount	//edx=第一个参数的地址
		mov eax,dwOriginFunction//eax=函数地址

		//保存栈
		push ebp
		mov ebp,esp
		
		mov ecx,[ebx]	//ecx=dwArguCount
LOOPPUSH:
		cmp ecx,0
		jz EX

		push [ebx+4*ecx];
		dec ecx
		jmp LOOPPUSH
EX:
		call eax

		//恢复栈
		mov esp,ebp
		pop ebp
		mov dwRetValue,eax
		//恢复寄存器
		popfd
		popad
	}
	return dwRetValue;
}

DWORD CXHookAPI::ThisCallFunction(LPVOID lpThisPointer,DWORD dwArguCount,...)
{
	DWORD	 dwOriginFunction = m_pOriginFunction;
	DWORD		dwRetValue;
	__asm
	{
		//保存寄存器
		pushad
		pushfd
		
		lea ebx,dwArguCount	//edx=第一个参数的地址
		mov eax,dwOriginFunction//eax=函数地址
		mov edx,lpThisPointer

		//保存栈
		push ebp
		mov ebp,esp
		
		mov ecx,[ebx]	//ecx=dwArguCount
LOOPPUSH:
		cmp ecx,0
		jz EX

		push [ebx+4*ecx];
		dec ecx
		jmp LOOPPUSH
EX:
		mov ecx,edx
		call eax

		//恢复栈
		mov esp,ebp
		pop ebp
		mov dwRetValue,eax
		//恢复寄存器
		popfd
		popad
	}
	return 0;
}

BOOL CXHookAPI::WriteMemory( LPVOID lpAddress,LPVOID lpData,int nSize )
{
	BOOL	bRet;
	DWORD	dwOldProtect;

	bRet = VirtualProtect( lpAddress,nSize,PAGE_EXECUTE_READWRITE, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;

	CopyMemory( lpAddress,lpData,nSize );

	bRet = VirtualProtect( lpAddress,nSize,dwOldProtect, &dwOldProtect);   
	if( FALSE == bRet )return FALSE;

	return TRUE;
}

BOOL CXHookAPI::UnHook()
{
	if( 0 == m_HookedAddr || 0 == m_dwOrginCodeLen )return FALSE;
	if( IsBadReadPtr( m_HookedAddr,m_dwOrginCodeLen )) return FALSE;


	return WriteMemory( m_HookedAddr,m_pOriginCode,m_dwOrginCodeLen);
}

__declspec(naked) VOID _CallFunction()
{
	
	__asm
	{
		push ebp
		mov ebp,esp
		//[ebp+4]=返回地址
		//[ebp+8]函数地址
		//[ebp+8+4]第一个参数

		//保存栈及寄存器
		push 0			//开个空间来保存esp值[ebp-4]
		
		pushfd
		mov [ebp-4],esp//[ebp-4]保存着当前esp地址

		mov eax,[ebp+0x0c]
LOOPPUSH:
		
		push [ebp+0x0c+4*eax]
		dec eax
		jnz LOOPPUSH

		call [ebp+8]
		
		//恢复栈及寄存器
		mov esp,[ebp-4]//[ebp-4]保存着当前esp地址
		popfd
		
		mov esp,ebp
		pop ebp
		retn
	}
}

__declspec(naked) VOID _ThisCallFunction()
{
	__asm
	{
		push ebp
		mov ebp,esp
		//[ebp+4]=this指针
		//[ebp+4+4]=函数地址
		//[ebp+8+4]函数地址,
		//[ebp+8+4+4]第一个参数,

		//保存栈及寄存器
		push 0			//开个空间来保存esp值[ebp-4]
		push ecx
		pushfd
		mov [ebp-4],esp//[ebp-4]保存着当前esp地址

		mov eax,[ebp+0x10]
LOOPPUSH:
		
		push [ebp+0x10+4*eax]
		dec eax
		jnz LOOPPUSH
	
		mov ecx,[ebp+8]
		call [ebp+0x0c]

		//恢复栈及寄存器
		mov esp,[ebp-4]//[ebp-4]保存着当前esp地址
		popfd
		pop ecx
		
		mov esp,ebp
		pop ebp
		retn
	}
}

}