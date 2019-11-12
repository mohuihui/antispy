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
#include "stdafx.h"
#include "String.h"

LANGUAGE_TYPE g_enumLang = /*enumChinese; */ enumEnglish;

WCHAR szToolName[] = {'A','n','t','i','S','p','y','\0'};
WCHAR szAntiSpyDisableAutoruns[] = {'A','n','t','i','S','p','y','D','i','s','a','b','l','e','d','A','u','t','o','r','u','n','s','\0'};

WCHAR *szDispatchName[] = {
	L"IRP_MJ_CREATE",
	L"IRP_MJ_CREATE_NAMED_PIPE",
	L"IRP_MJ_CLOSE",
	L"IRP_MJ_READ",
	L"IRP_MJ_WRITE",
	L"IRP_MJ_QUERY_INFORMATION",
	L"IRP_MJ_SET_INFORMATION",
	L"IRP_MJ_QUERY_EA",
	L"IRP_MJ_SET_EA",
	L"IRP_MJ_FLUSH_BUFFERS",
	L"IRP_MJ_QUERY_VOLUME_INFORMATION",
	L"IRP_MJ_SET_VOLUME_INFORMATION",
	L"IRP_MJ_DIRECTORY_CONTROL",
	L"IRP_MJ_FILE_SYSTEM_CONTROL",
	L"IRP_MJ_DEVICE_CONTROL",
	L"IRP_MJ_INTERNAL_DEVICE_CONTROL",
	L"IRP_MJ_SHUTDOWN",
	L"IRP_MJ_LOCK_CONTROL",
	L"IRP_MJ_CLEANUP",
	L"IRP_MJ_CREATE_MAILSLOT",
	L"IRP_MJ_QUERY_SECURITY",
	L"IRP_MJ_SET_SECURITY",
	L"IRP_MJ_POWER",
	L"IRP_MJ_SYSTEM_CONTROL",
	L"IRP_MJ_DEVICE_CHANGE",
	L"IRP_MJ_QUERY_QUOTA",
	L"IRP_MJ_SET_QUOTA",
	L"IRP_MJ_PNP_POWER",
};

WCHAR *szAntiLogger[] = {
	L"Anti Logger",
	L"�������¼��",
};

WCHAR *szRegistryIsNoExsit[] = {
	L"Cannot find",
	L"�޷��ҵ�",
};

WCHAR *szEnableHotkeys[] = {
	L"Enable HotKeys",
	L"�����ȼ�",
};

WCHAR *szGoto[] = {
	L"Goto",
	L"ת��",
};

WCHAR *szProcess[] = {
	L"Process",
	L"����"
};

WCHAR *szUnknowModule[] = {
	L"Unknow Module",
	L"δ֪ģ��"
};

WCHAR *szKernelModule[] = {
	L"Driver",
	L"����ģ��"
};

WCHAR *szImageName[] = {
	L"Image Name",
	L"ӳ������"
};

WCHAR *szPid[] = {
	L"Pid",
	L"����ID"
};

WCHAR *szParentPid[] = {
	L"Parent Pid",
	L"������ID"
};

WCHAR *szImagePath[] = {
	L"Image Path",
	L"ӳ��·��"
};

WCHAR *szR3Access[] = {
	L"Ring3 Access",
	L"Ӧ�ò����"
};

WCHAR *szRenameKey[] = {
	L"Rename Key",
	L"��������"
};

WCHAR *szCreateKey[] = {
	L"Create Key",
	L"�½���"
};

WCHAR *szThisKeyAlreadyExists[] = {
	L"Can not create the key. This key already exists.",
	L"����ע�����ʧ�ܣ���ע������Ѿ����ڡ�"
};

WCHAR *szKeyName[] = {
	L"Key Name:",
	L"�����ƣ�"
};

WCHAR *szValueName[] = {
	L"Value Name:",
	L"ֵ���ƣ�"
};

WCHAR *szFileCorporation[] = {
	L"File Corporation",
	L"�ļ�����"
};

WCHAR *szProcessState[] = {
	L"Processes: %d, Hidden Processes: %d, Ring3 Inaccessible Processes: %d",
	L"���̣�%d�����ؽ��̣�%d��Ӧ�ò㲻�ɷ��ʽ��̣�%d"
};

WCHAR *szProcessModuleState[] = {
	L", [%s] Modules: %d/%d",
	L"��[%s] ģ�飺%d/%d"
};

WCHAR *szProcessHandleState[] = {
	L", [%s] Handles: %d",
	L"��[%s] �����%d"
};

WCHAR *szProcessThreadState[] = {
	L", [%s] Threads: %d",
	L"��[%s] �̣߳�%d"
};

WCHAR *szProcessMemoryState[] = {
	L", [%s] Memorys: %d",
	L"��[%s] �ڴ棺%d"
};

WCHAR *szProcessWindowsState[] = {
	L", [%s] Windows: %d",
	L"��[%s] ���ڣ�%d"
};

WCHAR *szProcessHotKeysState[] = {
	L", [%s] HotKeys: %d",
	L"��[%s] �ȼ���%d"
};

WCHAR *szProcessPrivilegesState[] = {
	L", [%s] Enable: %d, Disable: %d",
	L"��[%s] Enable��%d��Disable��%d"
};

WCHAR *szProcessTimersState[] = {
	L", [%s] Timers: %d",
	L"��[%s] ��ʱ����%d"
};

WCHAR *szDeny[] = {
	L"Deny",
	L"�ܾ�"
};

WCHAR *szProcessObject[] = {
	L"Process Object",
	L"���̶���"
};

WCHAR *szViewModules[] = {
	L"View Modules",
	L"�鿴����ģ��"
};

WCHAR *szFind[] = {
	L"Find",
	L"����"
};

WCHAR *szProcessRefresh[] = {
	L"Refresh (F5)",
	L"ˢ�� (F5)"
};

WCHAR *szRefresh[] = {
	L"Refresh",
	L"ˢ��"
};

WCHAR *szDelete[] = {
	L"Delete",
	L"ɾ��"
};

WCHAR *szViewThreads[] = {
	L"View Threads",
	L"�鿴�����߳�"
};

WCHAR *szViewHandles[] = {
	L"View Handles",
	L"�鿴���̾��"
};

WCHAR *szViewWindows[] = {
	L"View Windows",
	L"�鿴���̴���"
};

WCHAR *szViewMemory[] = {
	L"View Memory",
	L"�鿴�����ڴ�"
};

WCHAR *szViewTimers[] = {
	L"View Timers",
	L"�鿴���̶�ʱ��"
};

WCHAR *szViewHotKeys[] = {
	L"View HotKeys",
	L"�鿴�����ȼ�"
};

WCHAR *szViewStrings[] = {
	L"View Strings",
	L"�鿴�����ַ���"
};

WCHAR *szViewPrivileges[] = {
	L"View Privileges",
	L"�鿴����Ȩ��"
};

WCHAR *szKillProcess[] = {
	L"Kill (Del)",
	L"�������� (Del)"
};

WCHAR *szView[] = {
	L"View...",
	L"�鿴..."
};

WCHAR *szShowLowerPane[] = {
	L"Show Lower Pane",
	L"���·���ʾ������Ϣ"
};

WCHAR *szForceKillProcess[] = {
	L"Force Kill",
	L"ǿ�ƽ�������"
};

WCHAR *szShowProcessTree[] = {
	L"Show Process Tree",
	L"��ʾ������"
};

WCHAR *szKillAndDeleteFile[] = {
	L"Kill And Delete File",
	L"�������̲�ɾ���ļ�"
};

WCHAR *szForceKillAndDelete[] = {
	L"Force Kill And Delete File",
	L"ǿ�ƽ������̲�ɾ���ļ�"
};

WCHAR *szVerifyProcessSignature[] = {
	L"Verify Process Signature",
	L"��֤��ǰ��������ǩ��"
};

WCHAR *szVerifyAllProcessSignature[] = {
	L"Verify All Process Signature",
	L"��֤���н�������ǩ��"
};

WCHAR *szSuspend[] = {
	L"Suspend",
	L"��ͣ��������"
};

WCHAR *szResume[] = {
	L"Resume",
	L"�ָ���������"
};

WCHAR *szCopyImageName[] = {
	L"Copy Image Name",
	L"���ƽ�����"
};

WCHAR *szCopyImagePath[] = {
	L"Copy Image Path",
	L"���ƽ���·��"
};

WCHAR *szAnalyseOnline[] = {
	L"Analyse Online",
	L"�����ļ�����"
};

WCHAR *szSearchOnlineBaidu[] = {
	L"Search Online(Baidu)",
	L"��������������(Baidu)"
};

// WCHAR *szSearchOnlineGoogle[] = {
// 	L"Search Online(Google)",
// 	L"��������������(Google)"
// };

WCHAR *szSearchOnlineGoogle[] = {
	L"Search Online",
	L"��������ģ����"
};

WCHAR *szSearchProcessOnline[] = {
	L"Search Online",
	L"��������������"
};

WCHAR *szFindInExplorer[] = {
	L"Find Target In Explorer",
	L"����Դ�������ж�λ�ļ�"
};

WCHAR *szFindInAntiSpy[] = {
	L"Find Target In AntiSpy",
	L"��AntiSpy�ļ��������ж�λ�ļ�"
};

WCHAR *szProcessProperties[] = {
	L"Properties",
	L"�鿴�����ļ�����"
};

WCHAR *szFindModule[] = {
	L"Find DLLs Or Handles",
	L"�����н����в���ģ�����"
};

WCHAR *szFindUnsignedModules[] = {
	L"Find Unsigned DLLs",
	L"�����н����в���û��ǩ��ģ��"
};

WCHAR *szExportTo[] = {
	L"Export To...",
	L"������..."
};

WCHAR *szStartType[] = {
	L"Set Start Type",
	L"����������ʽ"
};

WCHAR *szText[] = {
	L"Text",
	L"�ı��ļ�"
};

WCHAR *szExportToText[] = {
	L"Export To Text",
	L"�������ı�"
};

WCHAR *szExportToExcel[] = {
	L"Export To Excel",
	L"������Excel����"
};

WCHAR *szStringValue[] = {
	L"String Value",
	L"�ַ���ֵ"
};

WCHAR *szBinaryValue[] = {
	L"Binary Value",
	L"������ֵ"
};

WCHAR *szDwordValue[] = {
	L"DWORD Value",
	L"DWORDֵ"
};

WCHAR *szMultiStringValue[] = {
	L"Multi-String Value",
	L"���ַ���ֵ"
};

WCHAR *szExpandableStringValue[] = {
	L"Expandable String Value",
	L"�������ַ���ֵ"
};

WCHAR *szExcel[] = {
	L"Excel",
	L"Excel����"
};

WCHAR *szModuleName[] = {
	L"Module Name",
	L"ģ����"
};

WCHAR *szBase[] = {
	L"Base",
	L"����ַ"
};

WCHAR *szSize[] = {
	L"Size",
	L"��С"
};

WCHAR *szModulePath[] = {
	L"Module Path",
	L"ģ��·��"
};

WCHAR *szModuleState[] = {
	L"Image Name: %s, Modules: %d, Suspicious Modules: %d",
	L"ӳ��%s�� ģ�飺%d�� ����ģ�飺%d"
};

WCHAR *szNotShowMicrosoftModules[] = {
	L"Hide Microsoft's Modules",
	L"����ʾ΢��ģ��"
};

WCHAR *szDumpMemory[] = {
	L"Dump Memory",
	L"����ģ���ڴ�"
};

WCHAR *szExportRegistry[] = {
	L"Export Registry",
	L"����ע���"
};

WCHAR *szAreYouSureDeleteKey[] = {
	L"Are you sure you want to delete the key and its subkeys?",
	L"ȷ��Ҫɾ�����������Ӧ��������"
};

WCHAR *szAreYouSureDeleteValue[] = {
	L"Are you sure you want to delete?",
	L"ȷ��Ҫɾ����"
};

WCHAR *szAreYouSureUnloadAndDeleteModule[] = {
	L"Are you sure you want to unload and delete the module?",
	L"ȷ��Ҫж�ز�ɾ�����ģ����"
};

WCHAR *szDeleteKey[] = {
	L"Delete Key",
	L"ɾ����"
};

WCHAR *szDeleteValue[] = {
	L"Delete Value",
	L"ɾ��ֵ"
};

WCHAR *szUnLoadModule[] = {
	L"Unload",
	L"ж�ش�ģ��"
};

WCHAR *szUnLoadModuleInAllProcess[] = {
	L"Unload In All Processes",
	L"�����н�����ж�ش�ģ��"
};

WCHAR *szCopyModuleName[] = {
	L"Copy Module Name",
	L"����ģ����"
};

WCHAR *szCopyModulePath[] = {
	L"Copy Module Path",
	L"����ģ��·��"
};

WCHAR *szVerifyModuleSignature[] = {
	L"Verify Signature",
	L"��֤����ǩ��"
};

WCHAR *szVerifyAllModuleSignature[] = {
	L"Verify All Signature",
	L"��֤��������ǩ��"
};

WCHAR *szModuleProperties[] = {
	L"Properties",
	L"�鿴ģ������"
};

WCHAR *szThreadId[] = {
	L"Tid",
	L"�߳�ID"
};

WCHAR *szThreadObject[] = {
	L"Thread Object",
	L"�̶߳���"
};

WCHAR *szTeb[] = {
	L"Teb",
	L"�̻߳�����"
};

WCHAR *szPriority[] = {
	L"Priority",
	L"���ȼ�"
};

WCHAR *szStartAddress[] = {
	L"Start Address",
	L"�߳����"
};

WCHAR *szSwitchTimes[] = {
	L"Switch Times",
	L"�л�����"
};

WCHAR *szThreadStatus[] = {
	L"Status",
	L"�߳�״̬"
};

WCHAR *szStartModule[] = {
	L"Start Module",
	L"�߳�����ģ��"
};

WCHAR *szThreadState[] = {
	L"Image Name: %s, Threads: %d, Hideen Threads: %d, No Module Threads: %d",
	L"ӳ��%s�� �̣߳�%d�� �����̣߳�%d�� ��ģ���̣߳�%d"
};

WCHAR *szSystemThreadState[] = {
// 	L"Threads: %d, Hideen Threads: %d, No Module Threads: %d",
// 	L"�̣߳�%d�� �����̣߳�%d�� ��ģ���̣߳�%d"
	L"Threads: %d",
	L"�̣߳�%d"
};

WCHAR *szThreadInfoState[] = {
	L"Threads: %d, Hideen Threads: %d, No Module Threads: %d",
	L"�̣߳�%d�� �����̣߳�%d�� ��ģ���̣߳�%d"
};

WCHAR *szShowTerminatedThreads[] = {
	L"Show Terminated Threads",
	L"��ʾ�Ѿ���ֹ���߳�"
};

WCHAR *szKillThread[] = {
	L"Kill",
	L"�����߳�"
};

WCHAR *szForceKillThread[] = {
	L"Force Kill",
	L"ǿ�ƽ����߳�"
};

WCHAR *szSuspendThread[] = {
	L"Suspend",
	L"�����߳�"
};

WCHAR *szResumeThread[] = {
	L"Resume",
	L"�ָ��߳�"
};

WCHAR *szForceSuspendThread[] = {
	L"Force Suspend",
	L"ǿ�ƹ����߳�"
};

WCHAR *szDisassemblyStartAddress[] = {
	L"Disassemble StartAddress",
	L"������߳����"
};

WCHAR *szForceResumeThread[] = {
	L"Force Resume",
	L"ǿ�ƻָ��߳�"
};

WCHAR *szVerifyStartModuleSignature[] = {
	L"Verify Module Signature",
	L"��֤ģ��ǩ��"
};

WCHAR *szFindStartModuleInExplorer[] = {
	L"Find Module In Explorer",
	L"��Դ�������ж�λģ��"
};

WCHAR *szStartModuleProperties[] = {
	L"Module Properties",
	L"�鿴ģ������"
};

WCHAR *szProcessModules[] = {
	L"Process Module",
	L"����ģ��"
};

WCHAR *szProcessThreads[] = {
	L"Process Thread",
	L"�����߳�"
};

WCHAR *szDumpOK[] = {
	L"Dump OK!",
	L"�����ڴ�ɹ���"
};

WCHAR *szDumpFailed[] = {
	L"Dump Failed!",
	L"�����ڴ�ʧ�ܣ�"
};

WCHAR *szFileExist[] = {
	L"The file has exist, do you truncate it?",
	L"�ļ��Ѿ����ڣ��Ƿ񸲸ǣ�"
};

WCHAR *szSureToUnloadModule[] = {
	L"Unload modules maybe cause the target process crash,\r\are you sure you want to continue?",
	L"ж��ģ����ܻᵼ��Ŀ����̱�������ȷ��Ҫж����"
};

WCHAR *szGlobalUnloadModule[] = {
	L"Unload modules in some system processes maybe cause your system crash,\r\nare you sure you want to continue?",
	L"ж��ϵͳ�����е�ĳЩģ����ܻᵼ������ϵͳ���ȶ����߱�����\r\nȷ��Ҫж����"
};

WCHAR *szUnloadModule[] = {
	L"Unload Module",
	L"ж��ģ��"
};

WCHAR *szHandleType[] = {
	L"Type",
	L"�������"
};

WCHAR *szHandleName[] = {
	L"Name",
	L"�����"
};

WCHAR *szHandleValue[] = {
	L"Handle",
	L"���"
};

WCHAR *szHandleObject[] = {
	L"Object",
	L"�������"
};

WCHAR *szHandleRefCount[] = {
	L"RefCount",
	L"���ü���"
};

WCHAR *szHandleState[] = {
	L"Image Name: %s, Handles: %d",
	L"ӳ��%s��  �����%d",
};

WCHAR *szHandleInfoState[] = {
	L"Handles: %d",
	L"�������%d",
};

WCHAR *szHandleTypeNumber[] = {
	L"Type Number",
	L"������ʹ���"
};

WCHAR *szProcessHandles[] = {
	L"Process Handle",
	L"���̾��"
};

WCHAR *szHandleClose[] = {
	L"Close",
	L"�رվ��"
};

WCHAR *szHandleForceClose[] = {
	L"Force Close",
	L"ǿ�ƹرվ��"
};

WCHAR *szAddress[] = {
	L"Address",
	L"��ַ"
};

WCHAR *szHex[] = {
	L"Hex",
	L"ʮ������"
};

WCHAR *szDisassembly[] = {
	L"Disassembly",
	L"�����"
};

WCHAR *szMemoryState[] = {
	L"Image Name: %s, Memorys: %d",
	L"ӳ�� %s�� �ڴ�飺 %d"
};

WCHAR *szMemoryInfoState[] = {
	L"Memorys: %d",
	L"�ڴ�飺 %d"
};

WCHAR *szProcessMemory[] = {
	L"Process Memory",
	L"�����ڴ�"
};

WCHAR *szMemoryDump[] = {
	L"Dump Memory",
	L"�����ڴ�"
};

WCHAR *szMemoryDumpSelect[] = {
	L"Dump Selected",
	L"������ѡ�ڴ�"
};

WCHAR *szMemoryZero[] = {
	L"Zero Memory",
	L"�����ڴ�"
};

WCHAR *szMemoryModifyProtection[] = {
	L"Modify Protection",
	L"�޸ı�������"
};

WCHAR *szMemoryFree[] = {
	L"Free",
	L"�ͷ��ڴ�"
};

WCHAR *szDumpAddress[] = {
	L"Address:",
	L"��ַ��"
};

WCHAR *szDumpSize[] = {
	L"Size:",
	L"��С��"
};

WCHAR *szOk[] = {
	L"OK",
	L"ȷ��"
};

WCHAR *szCancel[] = {
	L"Cancel",
	L"ȡ��"
};

WCHAR *szDumpDlgMemory[] = {
	L"Dump Memory",
	L"�����ڴ�"
};

WCHAR *szMemoryFreeMemory[] = {
	L"Free Memory",
	L"�ͷ��ڴ�"
};

WCHAR *szMemoryZeroMemory[] = {
	L"Zero Memory",
	L"�����ڴ�"
};

WCHAR *szMemoryFreeFailed[] = {
	L"Free Memory Failed!",
	L"�ͷ��ڴ�ʧ�ܣ�"
};

WCHAR *szMemoryDangerous[] = {
	L"It maybe cause the target process crash, are you sure you want to contionue?",
	L"�˲������ܻ����Ŀ�����������ȷ��Ҫ������"
};

WCHAR *szModifyProtection[] = {
	L"Modify Protection",
	L"�޸ı�������"
};

WCHAR *szModifyProtectionFailed[] = {
	L"Modify Protection Failed!",
	L"�޸ı�������ʧ�ܣ�"
};

WCHAR *szModifyMemoryProtection[] = {
	L"Modify Protection",
	L"�޸��ڴ汣������"
};

WCHAR *szZeroMemory[] = {
	L"Zero Memory",
	L"�����ڴ�"
};

WCHAR *szZeroMemoryFailed[] = {
	L"Zero Memory Failed!",
	L"�����ڴ�ʧ�ܣ�"
};

WCHAR *szZeroMemoryOk[] = {
	L"Zero Memory Ok!",
	L"�����ڴ�ɹ���"
};

WCHAR *szShowAllProcessesTimer[] = {
	L"Show All Processes Timer",
	L"��ʾ���н��̶�ʱ��"
};

WCHAR *szRemove[] = {
	L"Remove",
	L"�Ƴ�"
};

WCHAR *szModifyTimeOutValue[] = {
	L"Modify Time Out Value",
	L"�޸���Ӧʱ��"
};

WCHAR *szProcessTimer[] = {
	L"Process Timer",
	L"���̶�ʱ��"
};

WCHAR *szTimerState[] = {
	L"Image Name: %s, Timers: %d",
	L"ӳ�� %s�� ��ʱ���� %d"
};

WCHAR *szTimerStateNew[] = {
	L"Timers: %d",
	L"��ʱ����%d"
};

WCHAR *szTimerObject[] = {
	L"Object",
	L"��ʱ������"
};

WCHAR *szTimeOutValue[] = {
	L"Time Out(s)",
	L"��Ӧʱ��(s)"
};

WCHAR *szCallback[] = {
	L"Callback",
	L"�ص�����"
};

WCHAR *szTimerModulePath[] = {
	L"Module Path",
	L"��ʱ������ģ��"
};

WCHAR *szTid[] = {
	L"Tid",
	L"�߳�ID"
};

WCHAR *szAllProcesses[] = {
	L"All Processes",
	L"���н���"
};

WCHAR *szProcesseName[] = {
	L"Process Name",
	L"������"
};

WCHAR *szHotKeyObject[] = {
	L"Object",
	L"�ȼ�����"
};

WCHAR *szHotKeyId[] = {
	L"Id",
	L"�ȼ�Id"
};

WCHAR *szHotKeyKey[] = {
	L"Key",
	L"�ȼ�"
};

WCHAR *szHotKeyTid[] = {
	L"Tid",
	L"�߳�Id"
};

WCHAR *szHotKeyPid[] = {
	L"Pid",
	L"����Id"
};

WCHAR *szHotKeyProcessName[] = {
	L"Process Name",
	L"������"
};

WCHAR *szHotKeyState[] = {
	L"Image Name: %s, HotKeys: %d",
	L"ӳ��%s�� �ȼ���%d"
};

WCHAR *szHotKeyStateNew[] = {
	L"HotKeys: %d",
	L"�ȼ�����%d"
};

WCHAR *szProcessHotKey[] = {
	L"Process HotKey",
	L"�����ȼ�"
};

WCHAR *szShowAllProcessesHotKey[] = {
	L"Show All Processes HotKey",
	L"��ʾ���н����ȼ�"
};

WCHAR *szProcessPrivilege[] = {
	L"Process Privilege",
	L"����Ȩ��"
};

WCHAR *szPrivilegeStatus[] = {
	L"Status",
	L"״̬"
};

WCHAR *szPrivilege[] = {
	L"Privilege",
	L"Ȩ��"
};

WCHAR *szPrivilegeDescription[] = {
	L"Description",
	L"����"
};

WCHAR *szPrivilegeState[] = {
	L"Image Name: %s, Enabled: %d, Disabled: %d",
	L"ӳ�� %s�� Enabled��%d�� Disabled��%d"
};

WCHAR *szPrivilegeInfoState[] = {
	L"Enabled: %d, Disabled: %d",
	L"Enabled��%d�� Disabled��%d"
};

WCHAR *szPrivilegeEnable[] = {
	L"Enable",
	L"����Ȩ��"
};

WCHAR *szPrivilegeDisable[] = {
	L"Disable",
	L"ȡ��Ȩ��"
};

WCHAR *szHwnd[] = {
	L"Hwnd",
	L"���ھ��"
};

WCHAR *szWindowTitle[] = {
	L"Window Title",
	L"���ڱ���"
};

WCHAR *szClassName[] = {
	L"Class Name",
	L"��������"
};

WCHAR *szWndVisible[] = {
	L"Visible",
	L"���ڿɼ���"
};

WCHAR *szWndState[] = {
	L"Image Name: %s, Windows: %d",
	L"ӳ�� %s�� ���ڣ� %d"
};

WCHAR *szWndStateNew[] = {
	L"Windows: %d",
	L"�������� %d"
};

WCHAR *szProcessWindow[] = {
	L"Process Window",
	L"���̴���"
};

WCHAR *szShowAllProcessWindow[] = {
	L"Show All Process Window",
	L"��ʾ���н��̴���"
};

WCHAR *szShowWindow[] = {
	L"Show Window",
	L"��ʾ����"
};

WCHAR *szHideWindow[] = {
	L"Hide Window",
	L"���ش���"
};

WCHAR *szShowMaximized[] = {
	L"Show Maximized",
	L"��󻯴���"
};

WCHAR *szShowMinimized[] = {
	L"Show Minimized",
	L"��С������"
};

WCHAR *szEnableWindow[] = {
	L"Enable Window",
	L"�����"
};

WCHAR *szDisableWindow[] = {
	L"Disable Window",
	L"���ᴰ��"
};

WCHAR *szDestroyWindow[] = {
	L"Destroy Window",
	L"���ٴ���"
};

WCHAR *szTopMost[] = {
	L"Top Most",
	L"�����ö�"
};

WCHAR *szCancelTopMost[] = {
	L"Cancel Top Most",
	L"ȡ�������ö�"
};

WCHAR *szVisible[] = {
	L"Yes",
	L"�ɼ�"
};

WCHAR *szProcessDump[] = {
	L"Dump Memory",
	L"���������ڴ�ӳ��"
};

WCHAR *szFixDump[] = {
	L"Would you like to fix the dump?",
	L"��Ҫ�޸�ӳ���ļ���"
};

WCHAR *szFindModuleDlg[] = {
	L"Find Handles or DLLs",
	L"����ģ�����"
};

WCHAR *szFindDllModuleName[] = {
	L"Filter:",
	L"���ƣ�"
};

WCHAR *szFindHnadles[] = {
	L"Hnadles",
	L"���"
};

WCHAR *szFindDllModule[] = {
	L"DLLs",
	L"ģ��"
};

WCHAR *szRegFind[] = {
	L"Find...",
	L"����..."
};

WCHAR *szRegExport[] = {
	L"Export",
	L"����ע���"
};

WCHAR *szRegImport[] = {
	L"Import",
	L"����ע���"
};

WCHAR *szRegImportOk[] = {
	L"Import OK!",
	L"����ɹ���"
};

WCHAR *szRegExportOk[] = {
	L"Export OK!",
	L"�����ɹ���"
};

WCHAR *szRegDelete[] = {
	L"Delete",
	L"ɾ��"
};

WCHAR *szRegModify[] = {
	L"Modify",
	L"�޸�"
};

WCHAR *szRegCopyValueName[] = {
	L"Copy Value Name",
	L"����ֵ����"
};

WCHAR *szUseHiveAnalyze[] = {
	L"Use Hive Analysis",
	L"ʹ��Hive����"
};

WCHAR *szAddToQuickPositioning[] = {
	L"Add to Quick Position",
	L"���ӵ����ٶ�λ"
};

WCHAR *szRegCopyValueData[] = {
	L"Copy Value Data",
	L"����ֵ����"
};

WCHAR *szRegRename[] = {
	L"Rename",
	L"������"
};

WCHAR *szRegEditString[] = {
	L"Edit String",
	L"�༭�ַ���"
};

WCHAR *szRegEditDwordValue[] = {
	L"Edit DWORD Value",
	L"�༭DWORDֵ"
};

WCHAR *szRegValueName[] = {
	L"Value Name:",
	L"��ֵ���ƣ�"
};

WCHAR *szRegBase[] = {
	L"Base",
	L"����"
};

WCHAR *szRegValueData[] = {
	L"Value Data:",
	L"��ֵ���ݣ�"
};

WCHAR *szNewKey[] = {
	L"New Key",
	L"�½���"
};

WCHAR *szNewValue[] = {
	L"New Value",
	L"�½�ֵ"
};

WCHAR *szRenameValue[] = {
	L"Rename Value",
	L"������ֵ"
};

WCHAR *szCopyKeyName[] = {
	L"Copy Key Name",
	L"����������"
};

WCHAR *szCopyFullKeyName[] = {
	L"Copy Full Key Name",
	L"��������������"
};

WCHAR *szProcessName[] = {
	L"Process Name",
	L"������"
};

WCHAR *szFindModuleStatus[] = {
	L"Module Name: %s, Count: %d",
	L"�����ҵ�ģ������%s��ģ������%d"
};

WCHAR *szDeleteModuleFile[] = {
	L"Delete Module File",
	L"ɾ��ģ���ļ�"
};

WCHAR *szUnloadAndDeleteModuleFile[] = {
	L"Unload And Delete File",
	L"ж�ز�ɾ��ģ���ļ�"
};

WCHAR *szUnload[] = {
	L"Unload",
	L"ж��ģ��"
};

WCHAR *szUnloadAll[] = {
	L"Unload All",
	L"ж������ģ��"
};

WCHAR *szFindModuleInExplorer[] = {
	L"Find Target",
	L"��λģ���ļ�"
};

WCHAR *szFindNotSignatureModule[] = {
	L"Find No Signature Module",
	L"����û������ǩ��ģ��"
};

WCHAR *szNoSignatureModuleCount[] = {
	L"No Signature Modules: %d",
	L"������ǩ��ģ�飺%d"
};

WCHAR *szDeleteFile[] = {
	L"Delete File",
	L"ɾ���ļ�"
};

WCHAR *szForceResumeProcess[] = {
	L"Force Resume",
	L"ǿ�ƻָ���������"
};

WCHAR *szForceSuspendProcess[] = {
	L"Force Suspend",
	L"ǿ����ͣ��������"
};

WCHAR *szInjectDll[] = {
	L"Inject DLL",
	L"ע��ģ��"
};

WCHAR *szScaning[] = {
	L"[Scanning...]",
	L"[����ɨ�裬���Ժ�...]"
};

WCHAR *szDriverName[] = {
	L"Driver Name",
	L"������"
};

WCHAR *szUserName[] = {
	L"User Name",
	L"�û���"
};

WCHAR *szDiskName[] = {
	L"DISK",
	L"����"
};

WCHAR *szReadMbrFalied[] = {
	L"Read MBR Failed!",
	L"��ȡMBRʧ�ܣ�"
};

WCHAR *szUserPrivileges[] = {
	L"Privileges",
	L"Ȩ��"
};

WCHAR *szUserStatus[] = {
	L"Users: %d, Hidden Users: %d",
	L"�û���%d�������û���%d"
};

WCHAR *szDriverObject[] = {
	L"Driver Object",
	L"��������"
};

WCHAR *szDriverPath[] = {
	L"Driver Path",
	L"����·��"
};

WCHAR *szServiceName[] = {
	L"Service Name",
	L"������"
};

WCHAR *szLoadOrder[] = {
	L"Load Order",
	L"����˳��"
};

WCHAR *szDriverStatus[] = {
	L"Drivers: %d, Hidden Drivers: %d, Suspicious PE Image: %d",
	L"������%d�� ����������%d�� ����PEӳ��%d"
};

WCHAR *szSuspiciousPEImage[] = {
	L"Suspicious PE Image",
	L"����PEӳ��"
};

WCHAR *szFileNotFound[] = {
	L"File not found",
	L"�ļ�������"
};

WCHAR *szDriverDump[] = {
	L"Dump Memory",
	L"����ģ���ڴ�"
};

WCHAR *szDriverUnload[] = {
	L"Unload Driver(Dangerous)",
	L"ж������(Σ��)"
};

WCHAR *szDeleteDriverFile[] = {
	L"Delete Driver File",
	L"ɾ�������ļ�"
};

WCHAR *szDeleteDriverFileAndReg[] = {
	L"Delete Driver File And Reg",
	L"ɾ�������ļ���ע���"
};

WCHAR *szCopyDriverName[] = {
	L"Copy Driver Name",
	L"����������"
};

WCHAR *szCopyDriverPath[] = {
	L"Copy Driver Path",
	L"��������·��"
};

WCHAR *szLocateInRegedit[] = {
	L"Jump to Registry",
	L"��λ��ע���"
};

WCHAR *szVerifyDriverSignature[] = {
	L"Verify Driver Signature",
	L"��֤����ǩ��"
};

WCHAR *szVerifyAllDriverSignature[] = {
	L"Verify All Driver Signature",
	L"��֤��������ǩ��"
};

WCHAR *szDriverProperties[] = {
	L"Properties",
	L"�鿴�����ļ�����"
};

WCHAR *szSearchOnlineDriver[] = {
	L"Search Online",
	L"��������������"
};

WCHAR *szVerifyDlgImagePath[] = {
	L"File Path: ",
	L"�ļ�·���� "
};

WCHAR *szVerifyStatusVerifyNow[] = {
	L"Status: Verify Signature Now, Please Wait...",
	L"ǩ��״̬�� ������֤�ļ�ǩ�������Ժ�..."
};

WCHAR *szVerifyStatusVerifySigned[] = {
	L"Status: Signed.",
	L"ǩ��״̬�� �ļ��Ѿ�ǩ����"
};

WCHAR *szVerifyStatusVerifyNotSigned[] = {
	L"Status: Not Signed.",
	L"ǩ��״̬�� �ļ�δͨ��ǩ����"
};

WCHAR *szOK[] = {
	L"OK",
	L"ȷ��"
};

WCHAR *szDisasm[] = {
	L"Disasm",
	L"�����"
};

WCHAR *szProcessList[] = {
	L"Process List",
	L"�����б�"
};

WCHAR *szDisasmCode[] = {
	L"Disassembly Code",
	L"��������"
};

WCHAR *szSave[] = {
	L"Save",
	L"�����޸�"
};

WCHAR *szReadMemory[] = {
	L"Read Memory",
	L"��ȡ�ڴ�"
};

WCHAR *szConfirmToWrite[] = {
	L"Confirm To Write Memory",
	L"ȷ��д���ڴ�"
};

//Show amaranth color on no signed item. \r\n
WCHAR *szVerifysignatureNowPleaseWait[] = {
	L"Verify signature now, please wait...",
	L"����У���ļ�ǩ�������Ժ�..."
};

WCHAR *szVerifySignature[] = {
	L"Verify Signature",
	L"У���ļ�ǩ��"
};

WCHAR *szKernelMode[] = {
	L"Kernel Mode",
	L"�ں˲�"
};

WCHAR *szUserMode[] = {
	L"User Mode",
	L"�û���"
};

WCHAR *szRegistry[] = {
	L"Registry",
	L"ע���"
};

WCHAR *szService[] = {
	L"Service",
	L"����"
};

WCHAR *szAutostart[] = {
	L"Autostart",
	L"������"
};

WCHAR *szCommand[] = {
	L"Command",
	L"������"
};

WCHAR *szAbout[] = {
	L"About",
	L"����"
};

WCHAR *szSetConfig[] = {
	L"Config",
	L"����"
};

WCHAR *szMyComputer[] = {
	L"My Computer",
	L"�ҵĵ���"
};

WCHAR *szAll[] = {
	L"All",
	L"ȫ��������"
};

WCHAR *szStartup[] = {
	L"Startup",
	L"����"
};

WCHAR *szWinlogon[] = {
	L"Winlogon",
	L"ϵͳ��¼���"
};

WCHAR *szExplorer[] = {
	L"Explorer",
	L"��Դ���������"
};

WCHAR *szInternetExplorer[] = {
	L"Internet Explorer",
	L"IE��������"
};

WCHAR *szAutoRunServices[] = {
	L"Services",
	L"ϵͳ����"
};

WCHAR *szAutoRunDrivers[] = {
	L"Drivers",
	L"��������"
};

WCHAR *szRing0Hook[] = {
	L"Ring0 Hook",
	L"�ں˹���"
};

WCHAR *szRoutineEntry[] = {
	L"Routine Entry",
	L"�ص����"
};

WCHAR *szNotifyType[] = {
	L"Notify Type",
	L"֪ͨ����"
};

WCHAR *szNote[] = {
	L"Note",
	L"��ע"
};

WCHAR *szCallbackStatus[] = {
	L"CreateProcess: %d, CreateThread: %d, LoadImage: %d, Registry: %d, Shutdown: %d",
	L"CreateProcess: %d, CreateThread: %d, LoadImage: %d, Registry: %d, Shutdown: %d"
};

WCHAR *szCallbackFindInExplorer[] = {
	L"Find Target In Explorer",
	L"����Դ�������ж��ļ�"
//	L"��λģ���ļ�"
};

WCHAR *szCallbackModuleProperties[] = {
	L"Properties",
	L"�鿴ģ������"
};

WCHAR *szFixLsp[] = {
	L"Repair LSP",
	L"�޸�LSP"
};

WCHAR *szDisassemblerEntry[] = {
	L"Disassembler Entry",
	L"��������"
};

WCHAR *szCallbackVerifyModuleSignature[] = {
	L"Verify Signature",
	L"У��ģ��ǩ��"
};

WCHAR *szRemoveAll[] = {
	L"Remove All",
	L"ȫ���Ƴ�"
};

WCHAR *szNotifyRoutine[] = {
	L"Notify Routine",
	L"ϵͳ�ص�"
};

WCHAR *szSSDT[] = {
	L"SSDT",
	L"SSDT"
};

WCHAR *szIdt[] = {
	L"IDT",
	L"ϵͳ�жϱ�"
};

WCHAR *szObjectHook[] = {
	L"Object Type",
	L"Object����"
};

WCHAR *szShadowSSDT[] = {
	L"Shadow SSDT",
	L"Shadow SSDT"
};

WCHAR *szFSD[] = {
	L"FSD",
	L"FSD"
};

WCHAR *szKbdClass[] = {
	L"Keyboard",
	L"����"
};

WCHAR *szMouClass[] = {
	L"Mouse",
	L"���"
};

WCHAR *szDisk[] = {
	L"Disk",
	L"Disk"
};

WCHAR *szAtapi[] = {
	L"Atapi",
	L"Atapi"
};

WCHAR *szAcpi[] = {
	L"Acpi",
	L"Acpi"
};

WCHAR *szTcpip[] = {
	L"Tcpip",
	L"Tcpip"
};

WCHAR *szDpcTimer[] = {
	L"DPC Timer",
	L"DPC��ʱ��"
};

WCHAR *szIoTimer[] = {
	L"IO Timer",
	L"IO��ʱ��"
};

WCHAR *szSystemThread[] = {
	L"System Thread",
	L"ϵͳ�߳�"
};

WCHAR *szHandleInfo[] = {
	L"Handle",
	L"���"
};

WCHAR *szWindowInfo[] = {
	L"Window",
	L"����"
};

WCHAR *szHotKeyInfo[] = {
	L"HotKey",
	L"�ȼ�"
};

WCHAR *szTimerInfo[] = {
	L"Timer",
	L"��ʱ��"
};

WCHAR *szPrivilegeInfo[] = {
	L"Privilege",
	L"Ȩ��"
};

WCHAR *szWorkerThread[] = {
	L"Worker Thread",
	L"���������߳�"
};

WCHAR *szDebugRegisters[] = {
	L"Debug Registers",
	L"���ԼĴ���"
};

WCHAR *szFilter[] = {
	L"Filter",
	L"��������"
};

WCHAR *szObjectHijack[] = {
	L"Object Hijack",
	L"����ٳ�"
};

WCHAR *szModuleInfo[] = {
	L"Module",
	L"ģ��"
};

WCHAR *szThreadInfo[] = {
	L"Thread",
	L"�߳�"
};

WCHAR *szProcessInformation[] = {
	L"Process Information",
	L"������Ϣ"
};

WCHAR *szModuleInfoState[] = {
	L"Modules: %d",
	L"ģ������%d"
};

WCHAR *szThreadsInfoState[] = {
	L"Threads: %d",
	L"�߳�����%d"
};

WCHAR *szMemoryInfo[] = {
	L"Memory",
	L"�ڴ�"
};

WCHAR *szDpcTimerObject[] = {
	L"Timer Object",
	L"��ʱ������"
};

WCHAR *szDpc[] = {
	L"DPC",
	L"DPC"
};

WCHAR *szPeriod[] = {
	L"Period(s)",
	L"��������(s)"
};

WCHAR *szDpcTimerRoutineEntry[] = {
	L"Routine Entry",
	L"�������"
};

WCHAR *szDpcTimerStatus[] = {
	L"DPC Timers: %d",
	L"DPC��ʱ��: %d"
};

WCHAR *szDeviceObject[] = {
	L"Device Object",
	L"�豸����"
};

WCHAR *szIoTimerStatus[] = {
	L"Status",
	L"״̬"
};

WCHAR *szStop[] = {
	L"Stop",
	L"ֹͣ"
};

WCHAR *szStart[] = {
	L"Start",
	L"����"
};

WCHAR *szIoTimerCnt[] = {
	L"IO Timers: %d",
	L"IO��ʱ��: %d"
};

WCHAR *szIoTimerStatusWorking[] = {
	L"Working",
	L"�ѿ���"
};

WCHAR *szIoTimerStatusStop[] = {
	L"Stop",
	L"��ֹͣ"
};

WCHAR *szType[] = {
	L"Type",
	L"����"
};

WCHAR *szStatus[] = {
	L"Status",
	L"״̬"
};

WCHAR *szStartupType[] = {
	L"Startup Type",
	L"��������"
};

// WCHAR *szFile[] = {
// 	L"Startup Type",
// 	L"��������"
// };

WCHAR *szDisableDirectIo[] = {
	L"Disable Direct IO",
	L"��ֱֹ��IO"
};

WCHAR *szWorkerThreadStatus[] = {
	L"Worker Threads: %d, CriticalWorkQueue: %d, DelayedWorkQueue: %d, HyperCriticalWorkQueue: %d",
	L"�����̣߳� %d, CriticalWorkQueue�� %d, DelayedWorkQueue�� %d, HyperCriticalWorkQueue�� %d",
};

WCHAR *szRegister[] = {
	L"Register",
	L"�Ĵ���"
};

WCHAR *szRegisterValue[] = {
	L"Value",
	L"�Ĵ���ֵ"
};

WCHAR *szRestore[] = {
	L"Restore",
	L"�ָ�"
};

WCHAR *szBreakpointLength[] = {
	L"Length",
	L"�ϵ㳤��"
};

WCHAR *szBreakpointAddress[] = {
	L"Break Address",
	L"�ϵ��ַ"
};

WCHAR *szBreakOn[] = {
	L"Break Reason",
	L"�ϵ�����"
};

WCHAR *szBreakPointStatus[] = {
	L"Status",
	L"״̬"
};

WCHAR *szObject[] = {
	L"Object",
	L"����"
};

WCHAR *szObjectType[] = {
	L"Object Type",
	L"��������"
};

WCHAR *szObjectName[] = {
	L"Object Name",
	L"������"
};

WCHAR *szDescription[] = {
	L"Description",
	L"����"
};

WCHAR *szDevice[] = {
	L"Device",
	L"�豸"
};

WCHAR *szAttachDeviceName[] = {
	L"Attach Device Name",
	L"��������������"
};

WCHAR *szDirectIO[] = {
	L"Direct IO",
	L"ֱ��IO"
};

WCHAR *szDirectIOReason[] = {
	L"Reason",
	L"ԭ��"
};

WCHAR *szIOPLIsTrue[] = {
	L"IOPL is true",
	L"����IOPL�����ó�TRUE",
};

WCHAR *szTssIsLargerThanNormal[] = {
	L"Task state segment is larger than normal",
	L"����״̬�εĴ�С�쳣",
};

WCHAR *szIoMapIsChanged[] = {
	L"IoMap is changed",
	L"����IOλͼ������",
};

WCHAR *szDirectIOProcessCnt[] = {
	L"Direct IO Processes: %d",
	L"ֱ��IO���̣�%d",
};

WCHAR *szDirectIOKillProcess[] = {
	L"Kill",
	L"��������"
};

WCHAR *szFilterDriverCnt[] = {
	L"Filter Drivers: %d",
	L"�����豸��%d",
};

WCHAR *szObjectHijackStatus[] = {
	L"Object Hijack: %d",
	L"����ٳ֣�%d"
};

WCHAR *szIndex[] = {
	L"Index",
	L"����"
};

WCHAR *szName[] = {
	L"Name",
	L"����"
};

WCHAR *szDispalyName[] = {
	L"Dispaly Name",
	L"��ʾ��"
};

WCHAR *szKey[] = {
	L"Key",
	L"��"
};

WCHAR *szFile[] = {
	L"File",
	L"�ļ�"
};

WCHAR *szData[] = {
	L"Data",
	L"����"
};

WCHAR *szValue[] = {
	L"Value",
	L"ֵ"
};

WCHAR *szRegAddress[] = {
	L"Quick Position",
	L"���ٶ�λ"
};

WCHAR *szFunction[] = {
	L"Function",
	L"������"
};

WCHAR *szFunctionAddress[] = {
	L"Function",
	L"����"
};

WCHAR *szDefault[] = {
	L"(Default)",
	L"(Ĭ��)"
};

WCHAR *szValueNoSet[] = {
	L"(Value No Set)",
	L"(��ֵδ����)"
};

WCHAR *szCurrentEntry[] = {
	L"Current Entry",
	L"��ǰ������ַ"
};

WCHAR *szHook[] = {
	L"Hook",
	L"Hook"
};

WCHAR *szHookType[] = {
	L"Hook Type",
	L"��������"
};

WCHAR *szOriginalEntry[] = {
	L"Original Entry",
	L"ԭʼ������ַ"
};

WCHAR *szModule[] = {
	L"Module",
	L"��ǰ������ַ����ģ��"
};

WCHAR *szServiceModule[] = {
	L"Service Module",
	L"����ģ��"
};

WCHAR *szStarted[] = {
	L"Started",
	L"������"
};

WCHAR *szSSdtStatus[] = {
	L"SSDT Entry: %d, Hooks: %d",
	L"SSDT������%d�����ҹ�������%d"
};

WCHAR *szFindProcessByWindwo[] = {
	L"Find Process By Window",
	L"���Ҵ��ڶ�λ����"
};

WCHAR *szShadowSSdtStatus[] = {
	L"Shadow SSDT Entry: %d, Hooks: %d",
	L"Shadow SSDT������%d�����ҹ�������%d"
};

WCHAR *szOnlyShowHooks[] = {
	L"Only Show Hooks",
	L"����ʾ�ҹ�����"
};

WCHAR *szDisassemblerCurrentEntry[] = {
	L"Disassemble CurrentEntry",
	L"����൱ǰ������ַ"
};

WCHAR *szDisassemblerHookAddress[] = {
	L"Disassemble HookAddress",
	L"�����ҹ���ַ"
};

WCHAR *szDisassemblerOrginalEntry[] = {
	L"Disassemble OrginalEntry",
	L"�����ԭʼ������ַ"
};

WCHAR *szRestoreAll[] = {
	L"Restore All",
	L"�ָ�ȫ��"
};

WCHAR *szDisassembleBreakpoint[] = {
	L"Disassemble Breakpoint",
	L"�����ϵ��ַ"
};

WCHAR *szFsdStatus[] = {
	L"FSD Entry: %d, Hooks: %d",
	L"FSD��ǲ������%d�����ҹ�������%d"
};

WCHAR *szMouStatus[] = {
	L"MOU Entry: %d, Hooks: %d",
	L"MOU��ǲ������%d�����ҹ�������%d"
};

WCHAR *szIatEatStatus[] = {
	L"IAT/EAT Hooks: %d",
	L"IAT/EAT���ӣ�%d"
};

WCHAR *szDiskStatus[] = {
	L"Disk Entry: %d, Hooks: %d",
	L"Disk��ǲ������%d�����ҹ�������%d"
};

WCHAR *szAtapiStatus[] = {
	L"Atapi Entry: %d, Hooks: %d",
	L"Atapi��ǲ������%d�����ҹ�������%d"
};

WCHAR *szAcpiStatus[] = {
	L"Acpi Entry: %d, Hooks: %d",
	L"Acpi��ǲ������%d�����ҹ�������%d"
};

WCHAR *szKdbStatus[] = {
	L"KBD Entry: %d, Hooks: %d",
	L"KBD��ǲ������%d�����ҹ�������%d"
};

WCHAR *szTcpipStatus[] = {
	L"Tcpip Entry: %d, Hooks: %d",
	L"Tcpip��ǲ������%d�����ҹ�������%d"
};

WCHAR *szIdtStatus[] = {
	L"IDT Entry: %d, Hooks: %d",
	L"�жϱ�������%d�����ҹ�������%d"
};

WCHAR *szKernelEntryStatus[] = {
	L"Kernel Entry Hook: %d",
	L"�ں���ڹ��ӣ�%d"
};

WCHAR *szObjectHookStatus[] = {
	L"Object Type Functions: %d, Hooks: %d",
	L"�������ͺ�����%d�����ҹ�������%d"
};

WCHAR *szMessageHook[] = {
	L"Message Hook",
	L"��Ϣ����"
};

WCHAR *szProcessHook[] = {
	L"Process Hook",
	L"���̹���"
};

WCHAR *szKernelCallbackTable[] = {
	L"KernelCallbackTable",
	L"�ں˻ص���"
};

WCHAR *szProcessModuleHijack[] = {
	L"Module Hijack",
	L"ģ��ٳ�"
};

WCHAR *szObjectAddress[] = {
	L"Object Address",
	L"�����ַ"
};

WCHAR *szHookAddress[] = {
	L"Hook Address",
	L"�ҹ�λ��"
};

WCHAR *szCurrentValue[] = {
	L"Current Value",
	L"�ҹ�����ǰֵ"
};

WCHAR *szPatchLenInBytes[] = {
	L"Length(Bytes)",
	L"����(�ֽ�)"
};

WCHAR *szOriginalValue[] = {
	L"Original Value",
	L"�ҹ���ԭʼֵ"
};

WCHAR *szKernelEntry[] = {
	L"Kernel Entry",
	L"�ں����"
};

WCHAR *szIatEat[] = {
	L"IAT/EAT",
	L"IAT/EAT",
};

WCHAR *szScanning[] = {
	L"Scanning...",
	L"����ɨ��...",
};

WCHAR *szPatchHooks[] = {
	L"Hooks/Patches: %d",
	L"����/�����޸ģ� %d",
};

WCHAR *szModifyedCode[] = {
	L"Modified Code",
	L"�����޸�",
};

WCHAR *szProcessPath[] = {
	L"Process Path",
	L"����·��"
};

WCHAR *szHandle[] = {
	L"Handle",
	L"���"
};

WCHAR *szProcesse[] = {
	L"Process",
	L"������"
};

WCHAR *szRunProcessAsSuspend[] = {
	L"Run Process As Suspend",
	L"�Թ���ʽ��������"
};

WCHAR *szAutoStart[] = {
	L"Automatic",
	L"�Զ�"
};

WCHAR *szBoot[] = {
	L"Boot Start",
	L"��������"
};

WCHAR *szSystemType[] = {
	L"System Start",
	L"ϵͳ����"
};

WCHAR *szDisabled[] = {
	L"Disabled",
	L"�ѽ���"
};

WCHAR *szManual[] = {
	L"Manual",
	L"�ֶ�"
};

WCHAR *szSystemStart[] = {
	L"System Start",
	L"ϵͳ����"
};

WCHAR *szBootStart[] = {
	L"Boot Start",
	L"��������"
};

WCHAR *szServiceCnt[] = {
	L"Service: ",
	L"���� "
};

WCHAR *szServiceStart[] = {
	L"Start",
	L"����"
};

WCHAR *szServiceStop[] = {
	L"Stop",
	L"ֹͣ"
};

WCHAR *szServiceRestart[] = {
	L"Restart",
	L"��������"
};

WCHAR *szServiceAutomatic[] = {
	L"Auto Start",
	L"�Զ�����"
};

WCHAR *szServiceManual[] = {
	L"Manual Start",
	L"�ֶ�����"
};

WCHAR *szServiceDisabled[] = {
	L"Disabled",
	L"��ֹ����"
};

WCHAR *szServiceDeleteRegistry[] = {
	L"Delete Registry",
	L"ɾ��ע���"
};

WCHAR *szServiceDeleteRegistryAndFile[] = {
	L"Delete Registry And File",
	L"ɾ��ע����ͷ���ģ��"
};

WCHAR *szKnowDlls[] = {
	L"Know DLLs",
	L"���ض�̬���ӿ�"
};

WCHAR *szPrintMonitors[] = {
	L"Print Monitors",
	L"��ӡ���ӳ���"
};

WCHAR *szNetworkProviders[] = {
	L"Network Providers",
	L"���繩Ӧ��"
};

WCHAR *szWinsockProviders[] = {
	L"Winsock Providers",
	L"Winsock��Ӧ��"
};

WCHAR *szSecurityProviders[] = {
	L"Security Providers",
	L"��ȫ��Ӧ��"
};

WCHAR *szScheduledTasks[] = {
	L"Scheduled Tasks",
	L"�ƻ�����"
};

WCHAR *szJmpToReg[] = {
	L"Jump to Registry",
	L"��ת��ע���"
};

WCHAR *szDeleteStartup[] = {
	L"Delete Startup",
	L"ɾ��������Ϣ"
};

WCHAR *szEnableStartup[] = {
	L"Enable",
	L"����������"
};

WCHAR *szDisableStartup[] = {
	L"Disable",
	L"��ֹ������"
};

WCHAR *szExportStartup[] = {
	L"Export Startup",
	L"����������Ϣ"
};

WCHAR *szDeleteStartupAndFile[] = {
	L"Delete Startup And File",
	L"ɾ��������Ϣ���ļ�"
};

WCHAR *szCopyStartupName[] = {
	L"Copy Startup Name",
	L"������������"
};

WCHAR *szCopyStartupPath[] = {
	L"Copy Startup Path",
	L"����������·��"
};

WCHAR *szStartupSearchOnline[] = {
	L"Search Online",
	L"��������������"
};

WCHAR *szAreYouSureUnloadDriver[] = {
	L"Are you sure you want to unload the driver?",
	L"ж���������������գ�ȷ��Ҫж�أ�"
};

WCHAR *szUnloadDriver[] = {
	L"Unload Driver",
	L"ж������"
};

WCHAR *szDisassmSize[] = {
	L"Size(Bytes):",
	L"��С(�ֽ�)��"
};

WCHAR *szDisassmPID[] = {
	L"PID:",
	L"PID��"
};

WCHAR *szDisassembler[] = {
	L"Disassembler",
	L"�������"
};

WCHAR *szFollowImediate[] = {
	L"Follow Imediate",
	L"����������"
};

WCHAR *szCopy[] = {
	L"Copy...",
	L"����..."
};

WCHAR *szCopyFullLine[] = {
	L"Full row",
	L"����"
};

WCHAR *szCopyAddress[] = {
	L"Address",
	L"��ַ"
};

WCHAR *szCopyHex[] = {
	L"Hex",
	L"16����"
};

WCHAR *szCopyCode[] = {
	L"Assembly code",
	L"������"
};

WCHAR *szLowerCase[] = {
	L"LowerCase",
	L"Сд����"
};

WCHAR *szDisassemble[] = {
	L"Disassembler",
	L"����൱ǰ��ַ"
};

WCHAR *szDisassembleCallback[] = {
	L"Disassemble Callback",
	L"�����ص�����"
};

WCHAR *szDisassembleEntry[] = {
	L"Disassemble Entry",
	L"��������"
};

WCHAR *szCannotFindFile[] = {
	L"Cannot find the file!",
	L"�ļ�������!"
};

WCHAR *szHardwareBreakpoint[] = {
	L"Hardware breakpoint: %d",
	L"Ӳ���ϵ㣺%d"
};

WCHAR *szMessageHookStatus[] = {
	L"Message Hook: %d, Global Message Hook: %d",
	L"��Ϣ���ӣ�%d��ȫ����Ϣ���ӣ�%d"
};

WCHAR *szOnlyShowGlobalHook[] = {
	L"Only Show Global Hook",
	L"����ʾȫ�ֹ���"
};

WCHAR *szUnhook[] = {
	L"Unhook",
	L"ж�ع���"
};

WCHAR *szDisassembleMessageHook[] = {
	L"Disassemble Entry",
	L"����๳�����"
};

// WCHAR *szCheckForUpdate[] = {
// 	L"Checking for updates now, please wait for a moment...\r\nConntecting to the server.",
// 	L"���ڼ����£����Ժ�...\r\n�������ӷ�������"
// };

WCHAR *szCheckForUpdate[] = {
	L"Please wait while your request is being processed. This could take a few seconds.\r\nConntecting to the server.",
	L"���ڴ������������������Ҫ������ʱ�䣬���Ժ�\r\n�������ӷ�����..."
};

WCHAR *szUpdate[] = {
	L"Check for updates",
	L"������"
};

WCHAR *szConnectedOK[] = {
	L"Connected OK.\r\nWaiting for server response...",
	L"���ӷ������ɹ���\r\n���ڵȴ���������Ӧ..."
};

// WCHAR *szUpdateFailed[] = {
// 	L"Update failed, please try again later.",
// 	L"����ʧ�ܣ����Ժ����ԡ�"
// };

WCHAR *szCurrentVersion[] = {
	L"Current Version: ",
	L"��ǰ�汾��"
};

WCHAR *szLatestVersion[] = {
	L"Latest Version: ",
	L"���°汾��"
};

WCHAR *szUpdateLog[] = {
	L"Update Log:",
	L"������־��"
};

WCHAR *szAlreadyHaveRecentVersion[] = {
	L"You already have the most recent version of AntiSpy.",
	L"AntiSpy�Ѿ������°汾��������¡�"
};

WCHAR *szUpdateError[] = {
	L"Check for updates error, please try again later.",
	L"�����³��������Ժ����ԡ�"
};

WCHAR *szDownloadError[] = {
	L"Download the file error, please try again later.",
	L"�����ļ����������Ժ����ԡ�"
};

WCHAR *szUpdateFailed[] = {
	L"Update failed, please try again later.",
	L"�����ļ�ʧ�ܣ����Ժ����ԡ�"
};

WCHAR *szUpdateSuccess[] = {
	L"The update was successful, enjoy it.",
	L"���³ɹ���"
};

WCHAR *szUpdateNow[] = {
	L"Update Now",
	L"��������"
};

WCHAR *szGotoNewVersion[] = {
	L"Goto New Version",
	L"��λ���°汾"
};

WCHAR *szCheckForUpdates[] = {
	L"Check for updates (the current version is %s)",
	L"����°汾 (��ǰ��%s�汾)"
};

WCHAR *szContactAuthor[] = {
	L"Contact Author",
	L"��ϵ����"
};

WCHAR *szAuthor[] = {
	L"Author: mzf",
	L"���ߣ�mzf (Ī�һ�)"
};

WCHAR *szEmail[] = {
	L"Email: AntiSpy@163.com",
	L"Email��AntiSpy@163.com"
};

WCHAR *szWebsite[] = {
	L"Blog: www.KingProtect.com",
	L"���ͣ�www.KingProtect.com"
};

WCHAR *szQQ[] = {
	L"QQ: 454858525",
	L"QQ  ��454858525�� QQȺ��17385388"
};

WCHAR *szAntiSpy[] = {
	L"Website: www.AntiSpy.cn",
	L"������www.AntiSpy.cn"
};


WCHAR *szSina[] = {
	L"MicroBlog: http://weibo.com/minzhenfei (Sina)",
	L"΢����http://weibo.com/minzhenfei (����)"
};

WCHAR *szTencent[] = {
	L"     http://weibo.com/minzhenfei (Tencent)",
	L"http://t.qq.com/minzhenfei (��Ѷ)"
};

WCHAR *szInformationAndCopyright[] = {
	L"Information && Copyright",
	L"˵������л"
};

WCHAR *szInformation[] = {
	L"Description: AntiSpy is a handy anti-virus and rootkit tools. It can detect,analyze and restore various kernel modifications, just like ssdt hooks. With its assistance, you can easily spot and remove malwares in your system.",
	L"˵        ��������һ���ֹ�ɱ���������ߣ������İ���������������׾ٵľ�����������������Ĳ�����ľ��������һ���ɾ����ĵ�����������"
};

WCHAR *szSystem[] = {
	L"",
	L"����ϵͳ��Win2000��xp��Win2003��Vista��Win2008��Win7��Win8 (32λ)"
};

WCHAR *szThanks[] = {
	L"Platform: Win2000, xp, Win2003, Vista, Win2008, Win7, Win8 (32bit)",
	L"��        л�������ߵ�д�������У��ο��˶�λ������ţ�����£��ڴ�һ����ʾ��л������ǳ���лhxjͬѧ����������Ƶ�����ͼ�ꡣ"
};

WCHAR *szCopyright[] = {
	L"",
	L"��        Ȩ��(C) 2011 - 2013 mzf. ��Ȩ����."
};

WCHAR *szMianze[] = {
	L"Copyright: (C) 2011 - 2013 mzf. All Rights Reserved.",
	L"�������������ڱ����ߵĴ󲿷ֹ��ܶ����ں�����ɣ���˴���һ���Ĳ��ȶ��ԡ��������ʹ�ñ����ߵĹ����У��������ֱ�ӻ��߼�ӵ���ʧ�����˸Ų����𡣴���ʹ�ñ����ߵ�һ���𣬽���Ϊ���Ѿ������˱���ְ������"
};

WCHAR *szUpdates[] = {
	L"Update",
	L"����"
};

WCHAR *szUnsupportedOS[] = {
	L"Unfortunately this OS is totally unsupported, please email me your OS version, I will support it in improved version, thanks.",
	L"�ݲ�֧�ֵĲ���ϵͳ���뷢�ʼ���֪���Ĳ���ϵͳ�汾�ţ��ҽ��ں����汾�иĽ���лл����֧�֡�"
};

WCHAR *szLoadDirverError[] = {
	L"Failed to load the diver.",
	L"��������ʧ�ܣ�"
};

WCHAR *szCannotSupportX64[] = {
	L"The current version does not support 64 bit Windows.",
	L"��ǰ�汾�ݲ�֧��64λ����ϵͳ��"
};

WCHAR *szShowAllProcessInfo[] = {
	L"View Detail Info (DClick)",
	L"�鿴������ϸ��Ϣ (DClick)"
};

WCHAR *szShowModulesBelow[] = {
	L"Show Lower Modules Pane",
	L"���·���ʾģ�鴰��"
};

WCHAR *szModulesBelow[] = {
	L", Modules(%s): %d/%d",
	L"�� ģ��(%s)�� %d/%d"
};

WCHAR *szAreYOUSureKillProcess[] = {
	L"Are you sure you want to kill %s?",
	L"��ȷ��Ҫ�������� %s ��"
};

WCHAR *szAreYOUSureKillTheseProcess[] = {
	L"Are you sure you want to kill these processes?",
	L"��ȷ��Ҫ������Щ������"
};

WCHAR *szAreYOUSureKillAndDeleteProcess[] = {
	L"Are you sure you want to kill and delete %s?",
	L"��ȷ��Ҫ�������� %s ��ɾ������ļ���"
};

WCHAR *szAreYOUSureKillAndDeleteProcesses[] = {
	L"Are you sure you want to continue?",
	L"��ȷ��Ҫ������Щ���̲�ɾ������ļ���"
};

WCHAR *szInitializing[] = {
	L"Initializing...",
	L"���ڳ�ʼ��..."
};

WCHAR *szScanDonePatchHooks[] = {
	L"Scan done. Hooks/Patches: %d",
	L"ɨ����ϣ�����/�����޸ģ� %d",
};

WCHAR *szCreateDump[] = {
	L"Create Debug Dump...",
	L"��������Dump...",
};

WCHAR *szCreateDumpOK[] = {
	L"Create Dump OK!",
	L"����Dump�ɹ���",
};

WCHAR *szCreateDumpFailed[] = {
	L"Failed to create dump!",
	L"����Dumpʧ�ܣ�",
};

WCHAR *szProcessImage[] = {
	L"Image",
	L"ӳ��",
};

WCHAR *szNonExistentProcess[] = {
	L"Non-existent process",
	L"�ý������˳�",
};

WCHAR *szImageFile[] = {
	L"Image File",
	L"ӳ���ļ�",
};

WCHAR *szFileSize[] = {
	L"Size:",
	L"�ļ���С��",
};

WCHAR *szFileDescription[] = {
	L"Description:",
	L"�ļ�������",
};

WCHAR *szFileCompany[] = {
	L"Corporation:",
	L"�ļ����̣�",
};

WCHAR *szFilePath[] = {
	L"Path:",
	L"�ļ�·����",
};

WCHAR *szFileTime[] = {
	L"Time:",
	L"ʱ        �䣺",
};

WCHAR *szFileVersion[] = {
	L"Version:",
	L"�ļ��汾��",
};

WCHAR *szCommandLine[] = {
	L"Command Line:",
	L"�����У�",
};

WCHAR *szCurrentDirectory[] = {
	L"Current Directory:",
	L"��ǰĿ¼��",
};

WCHAR *szStartTime[] = {
	L"Started:",
	L"����ʱ�䣺",
};

WCHAR *szPEB[] = {
	L"PEB:",
	L"PEB��",
};

WCHAR *szParent[] = {
	L"Parent:",
	L"�����̣�",
};

WCHAR *szFileFind[] = {
	L"Find",
	L"��λ",
};

WCHAR *szFileDelete[] = {
	L"Delete",
	L"ɾ��",
};

WCHAR *szScanHooks[] = {
	L"Scan Hooks",
	L"ɨ����̹���"
};

WCHAR *szAlreadyRunning[] = {
	L"AntiSpy is already running.",
	L"AntiSpy�����Ѿ�������"
};

WCHAR *szKillProcessTree[] = {
	L"Kill Process Tree",
	L"����������"
};

WCHAR *szAreYouSureKillDescendants[] = {
	L"Are you sure you want to kill %s and its descendants?",
	L"ȷ��Ҫ�������� %s �Լ����������ӽ�����"
};

WCHAR *szTheLatestVersion[] = {
	L"You already have the latest version of AntiSpy.",
	L"���� AntiSpy �Ѿ������°汾��"
};

WCHAR *szEditBinaryValue[] = {
	L"Edit Binary Value",
	L"�༭��������ֵ"
};

WCHAR *szHideMicrosofeModules[] = {
	L"Hide Microsoft's Modules",
	L"����΢��ģ��"
};

WCHAR *szInjectDllModuleOk[] = {
	L"Inject dll module ok!",
	L"ע��ģ��ɹ���"
};

WCHAR *szInjectDllModuleFailed[] = {
	L"Inject dll module failed!",
	L"ע��ģ��ʧ�ܣ�"
};

WCHAR *szGeneralConfiguration[] = {
// 	L"General configuration,the changes will not take effect until the next time it is started.",
// 	L"General configurations, it will take effect when the application restarted.",
// 	L"��������ѡ�������������Ч"
	L"General",
	L"����"
};

WCHAR *szProcessListConfiguration[] = {
// 	L"Process tab configurations, it will take effect when the application restarted.",
// 	L"���̱�ǩ����ѡ�������������Ч"
	L"Process",
	L"����"
};

WCHAR *szTitleName[] = {
	L"Custom title name:",
	L"�Զ��崰�ڱ��⣺"
};

WCHAR *szRandomTitle[] = {
	L"Random title name",
	L"���������"
};

WCHAR *szStayOnTop[] = {
	L"Stay on top",
	L"�����ö�"
};

WCHAR *szAutoDetectNewVersion[] = {
	L"Auto-detect new version",
	L"��������ʱ�Զ�����°汾"
};

WCHAR *szFindANewVersion[] = {
	L"Find a new version of AntiSpy, are you want to update?",
	L"�����°汾���Ƿ�����������"
};

WCHAR *szHotKeyForMainWindow[] = {
	L"Show main window: \tCtrl + Alt + Shift + ",
	L"��ʾ���������ڣ�\tCtrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessModulesDlg[] = {
	L"Show process modules:\tCtrl + Alt + Shift + ",
	L"��ʾ����ģ��Ի���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessThreadsDlg[] = {
	L"Show process threas:\tCtrl + Alt + Shift + ",
	L"��ʾ�����̶߳Ի���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessHandlesDlg[] = {
	L"Show process handles:\tCtrl + Alt + Shift + ",
	L"��ʾ���̾���Ի���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessWndDlg[] = {
	L"Show process windows:\tCtrl + Alt + Shift + ",
	L"��ʾ���̴��ڶԻ���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessMemoryDlg[] = {
	L"Show process memory:\tCtrl + Alt + Shift + ",
	L"��ʾ�����ڴ�Ի���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessHotkeysDlg[] = {
	L"Show process hotkeys:\tCtrl + Alt + Shift + ",
	L"��ʾ�����ȼ��Ի���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessPrivilegeDlg[] = {
	L"Show process privileges:\tCtrl + Alt + Shift + ",
	L"��ʾ����Ȩ�޶Ի���Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessTimersDlg[] = {
	L"Show process timers:\tCtrl + Alt + Shift + ",
	L"��ʾ��ʱ���Ի���\tCtrl + Alt + Shift + "
};

WCHAR *szShowProcessDetailInformations[] = {
	L"Automatic display process detail information tips",
	L"������ƶ��������б���ʱ�Զ���ʾ������ϸ��Ϣ"
};

WCHAR *szHideMicrosoftService[] = {
	L"Hide Microsoft's Modules",
	L"����΢������"
};

WCHAR *szSearchData[] = {
	L"Data",
	L"����"
};

WCHAR *szSearchValues[] = {
	L"Values",
	L"ֵ"
};

WCHAR *szSearchKeys[] = {
	L"Keys",
	L"��"
};

WCHAR *szSearchMachCase[] = {
	L"Mach case",
	L"���ִ�Сд"
};

WCHAR *szSearchMachWholeString[] = {
	L"Mach whole string",
	L"ƥ�������ַ���"
};

WCHAR *szFindWhat[] = {
	L"Find what:",
	L"�������ݣ�"
};

WCHAR *szWhereToSearch[] = {
	L"Where to search",
	L"������Χ"
};

WCHAR *szSearchInKey[] = {
	L"Search in key:",
	L"�ڴ�����������"
};

WCHAR *szSearchTheEntireRegistry[] = {
	L"Search the entire registry",
	L"��������ע���"
};

WCHAR *szRegistryFind[] = {
	L"Find",
	L"����"
};

WCHAR *szRegistryStop[] = {
	L"Stop",
	L"ֹͣ"
};

WCHAR *szRegistryPleaseInputFindWhat[] = {
	L"Please enter the content that you're searching for.",
	L"��������Ҫ���ҵ����ݡ�"
};

WCHAR *szRegistryPleaseInputWhatTheKeyToFind[] = {
	L"Please enter the key path that you're searching for.",
	L"��������Ҫ�����в��ҵ���·����"
};

WCHAR *szCanNotOpenKey[] = {
	L"Cannot open the key.",
	L"�޷��򿪸ü���"
};

WCHAR *szRegistrySearchNow[] = {
	L"Searching now, please wait for a moment...",
	L"�������������Ժ�..."
};

WCHAR *szFindResults[] = {
	L"Find results - found %d matches",
	L"���ҽ�� - �ҵ� %d ��ƥ�����Ŀ"
};

WCHAR *szFindResult[] = {
	L"Find results",
	L"���ҽ��"
};

WCHAR *szMemoryHexEdit[] = {
	L"Hex Editer",
	L"�ڴ�༭��"
};

WCHAR *szReadOnly[] = {
	L"Read Only",
	L"ֻ��"
};

WCHAR *szKernelAddress[] = {
	L"Kernel Mode Address (Address > 0x80000000)",
	L"�ں�ģʽ��ַ (��ַ > 0x80000000)"
};

WCHAR *szDeafultShowLowPaner[] = {
	L"Show lower pane as default",
	L"Ĭ����ʾ�·�ģ���б�"
};

WCHAR *szDeafultShowProcessTree[] = {
	L"Show process tree as default",
	L"Ĭ����ʾ������"
};

WCHAR *szDeleteFileSucess[] = {
	L"Delete file success.",
	L"ɾ���ļ��ɹ���"
};

WCHAR *szDeleteFileFailed[] = {
	L"Delete file failed.",
	L"ɾ���ļ�ʧ�ܡ�"
};

WCHAR *szJumpToService[] = {
	L"Jump to Service",
	L"��ת����Ӧ����"
};

WCHAR *szLocalDisk[] = {
	L"Local Disk",
	L"���ش���"
};

WCHAR *szCDDrive[] = {
	L"CD Drive",
	L"����"
};

WCHAR *szRemovableDisk[] = {
	L"Removable Disk",
	L"���ƶ�����"
};

WCHAR *szDiskUnknow[] = {
	L"Unknow",
	L"δ֪��"
};

WCHAR *szRemoteDisk[] = {
	L"Remote Disk",
	L"Զ�̴���"
};

WCHAR *szRamDisk[] = {
	L"Ram Disk",
	L"�ڴ���"
};

WCHAR *szFileName[] = {
	L"File Name",
	L"����"
};

WCHAR *szCreationTime[] = {
	L"Creation Time",
	L"��������"
};

WCHAR *szModificationTime[] = {
	L"Modification Time",
	L"�޸�����"
};

WCHAR *szFileSizeEx[] = {
	L"File Size",
	L"��С"
};

WCHAR *szFileAttributes[] = {
	L"Attributes",
	L"����"
};

WCHAR *szFileType[] = {
	L"Type",
	L"����"
};

WCHAR *szFileDirectory[] = {
	L"Directory",
	L"�ļ���"
};

WCHAR *szFileReadOnly[] = {
	L"ReadOnly",
	L"ֻ��"
};

WCHAR *szFileHIDDEN[] = {
	L"Hidden",
	L"����"
};

WCHAR *szFileSYSTEM[] = {
	L"System",
	L"ϵͳ"
};

WCHAR *szFileDIRECTORY[] = {
	L"Directory",
	L"�ļ���"
};

WCHAR *szFileARCHIVE[] = {
	L"Archive",
	L"����"
};

WCHAR *szFileNORMAL[] = {
	L"Normal",
	L"��ͨ"
};

WCHAR *szFileTEMPORARY[] = {
	L"Temporary",
	L"��ʱ"
};

WCHAR *szFileCOMPRESSED[] = {
	L"Compressed",
	L"ѹ��"
};

WCHAR *szFileOpen[] = {
	L"Open",
	L"��"
};

WCHAR *szFileRefresh[] = {
	L"Refresh",
	L"ˢ��"
};

WCHAR *szFileLookForLockInfo[] = {
	L"View File Lock Info",
	L"�鿴�������"
};

WCHAR *szKeyLookForLockInfo[] = {
	L"View Key Lock Info",
	L"�鿴�������"
};

WCHAR *szFileDeleteAndDenyRebuild[] = {
	L"Delete Aand Deny Rebuild",
	L"ɾ������ֹ�ļ�����"
};

WCHAR *szFileAddToRestartDelete[] = {
	L"Add To Delay Delete List",
	L"���ӵ�����ɾ���б�"
};

WCHAR *szFileRestartAndReplace[] = {
	L"Add To Delay Replace List",
	L"�����滻Ϊ..."
};

WCHAR *szFileCopyTo[] = {
	L"Copy to...",
	L"������..."
};

WCHAR *szFileRename[] = {
	L"Rename",
	L"������"
};

WCHAR *szFileCopyFileName[] = {
	L"Copy File Name",
	L"�����ļ���"
};

WCHAR *szFileCopyFilePath[] = {
	L"Copy File Path",
	L"�����ļ�·��"
};

WCHAR *szFileProperties[] = {
	L"Properties",
	L"�鿴�ļ�����"
};

WCHAR *szFileFindInExplorer[] = {
	L"Find Target In Explorer",
	L"����Դ�������ж�λ�ļ�"
};

WCHAR *szFileVerifySignature[] = {
	L"Verify Signature",
	L"У���ļ�ǩ��"
};

WCHAR *szFileVerifyAllSignature[] = {
	L"Verify Aall Signature",
	L"У�������ļ�ǩ��"
};

WCHAR *szFileDeleteHideSystemProperties[] = {
	L"Set Normal Attributes",
	L"ȥ��ֻ�������ء�ϵͳ����"
};

WCHAR *szFileUnlocker[] = {
	L"File Unlocker",
	L"�ļ�������"
};

WCHAR *szRegUnlocker[] = {
	L"Registry Unlocker",
	L"ע���������"
};

WCHAR *szFileLockedProcess[] = {
	L"Process Name",
	L"����ӳ��"
};

WCHAR *szFileLockedProcessPid[] = {
	L"Pid",
	L"����Id"
};

WCHAR *szFileLockedHandle[] = {
	L"Handle",
	L"���"
};

WCHAR *szFileLockedFilePath[] = {
	L"Locked File Path",
	L"�������ļ�·��"
};

WCHAR *szLockedRegistryPath[] = {
	L"Locked Registry Path",
	L"��������ע���·��"
};

WCHAR *szFileUnlocked[] = {
	L"File(Directory) not be locked.",
	L"�ļ�(�ļ���)δ��������"
};

WCHAR *szRegUnlocked[] = {
	L"The key is not be locked.",
	L"��ǰע�����ֵδ��������"
};

WCHAR *szFileUnlock[] = {
	L"Unlock",
	L"����"
};

WCHAR *szFileUnlockAll[] = {
	L"Unlock All",
	L"����ȫ��"
};

WCHAR *szFileKillLockProcess[] = {
	L"Kill Lock Process",
	L"������������"
};

WCHAR *szFileFindProcess[] = {
	L"Find Process In Explorer",
	L"��λ����������"
};

WCHAR *szFileFindFile[] = {
	L"Find File In Explorer",
	L"��λ���������ļ�"
};

WCHAR *szFileRestartDeleteType[] = {
	L"Operation Type",
	L"��������"
};

WCHAR *szFileRestartFilePath[] = {
	L"File Path",
	L"�ļ�·��"
};

WCHAR *szFileRestartNewFilePath[] = {
	L"New File Path",
	L"���ļ�·��"
};

WCHAR *szFileRestartDeleteInfo[] = {
	L"File Delay Operation Information",
	L"�ļ��ӳٲ�����Ϣ"
};

WCHAR *szFileViewRestartDeleteInfo[] = {
	L"View Delay Operation Info",
	L"�鿴�ļ��ӳٲ�����Ϣ"
};

WCHAR *szFilePendingDelayDelete[] = {
	L"Delay Delete",
	L"����ɾ��"
};

WCHAR *szFilePendingDelayReplace[] = {
	L"Delay Replace",
	L"�����滻"
};

WCHAR *szFilePendingStatus[] = {
	L"Delay Delete: %d, Delay Replace: %d",
	L"����ɾ����%d�� �����滻��%d"
};

WCHAR *szFilePendingDeleteCurrent[] = {
	L"Delete Current Operation",
	L"ɾ����ǰ�ӳٲ�����Ϣ"
};

WCHAR *szFilePendingDeleteAll[] = {
	L"Delete All Operation",
	L"ɾ�������ӳٲ�����Ϣ"
};

WCHAR *szFilePendingFindInExplorer[] = {
	L"Find File In Explorer",
	L"��λ���ļ�"
};

WCHAR *szFilePendingProperties[] = {
	L"File Properties",
	L"�鿴�ļ�����"
};

WCHAR *szFilePendingFindNewInExplorer[] = {
	L"Find New File In Explorer",
	L"��λ�����ļ�"
};

WCHAR *szFilePendingNewFileProperties[] = {
	L"New File Properties",
	L"�鿴���ļ�����"
};

WCHAR *szFileSetFileAttribute[] = {
	L"Set Hidden Attributes",
	L"�������ء�ϵͳ����"
};

WCHAR *szInputFilePath[] = {
	L"Input File Path",
	L"�����ļ�·��"
};

WCHAR *szStaticInputFilePath[] = {
	L"File Path:",
	L"�ļ�·����"
};

WCHAR *szFileBrowse[] = {
	L"Browse",
	L"���"
};

WCHAR *szInputFileName[] = {
	L"Input New File Name",
	L"�������ļ���"
};

WCHAR *szStaticInputFileName[] = {
	L"New File Name:",
	L"���ļ�����"
};

WCHAR *szFileDescriptionNew[] = {
	L"File Description",
	L"�ļ�����"
};

WCHAR *szProcessStartedTime[] = {
	L"Started Time",
	L"����ʱ��"
};

WCHAR *szFileCreationTime[] = {
	L"Creation Time",
	L"�ļ�����ʱ��"
};

WCHAR *szFileHasExist[] = {
	L"The file has exist, please input other name.",
	L"���ļ����Ѿ����ڣ������������ļ�����"
};

WCHAR *szFileCopy[] = {
	L"File Copy",
	L"�ļ�����"
};

WCHAR *szNormalProcess[] = {
	L"Normal process, the corporation is not Micorsoft",
	L"�ļ����̲���΢���Ľ���"
};

WCHAR *szHideProcess[] = {
	L"Hidden process",
	L"���ؽ���"
};

WCHAR *szMirosoftProcess[] = {
	L"The corporation of process is Micorsoft",
	L"�ļ�������΢���Ľ���"
};

WCHAR *szMirosoftProcessHaveOtherModule[] = {
	L"Micorsoft's process have other corporation modules",
	L"΢�����̺��з�΢����ģ��"
};

WCHAR *szSystemAndHidden[] = {
	L"System && Hidden",
	L"ϵͳ����������"
};

WCHAR *szNormalFile[] = {
	L"Normal File",
	L"��ͨ����"
};

WCHAR *szSystemOnly[] = {
	L"System Only",
	L"ϵͳ����"
};

WCHAR *szHiddenOnly[] = {
	L"Hidden Only",
	L"��������"
};

WCHAR *szColorConfiguration[] = {
// 	L"Color configurations, it will take effect when the application restarted.",
// 	L"��ɫ����ѡ�������������Ч"
	L"Colors",
	L"��ɫ"
};

WCHAR *szHotKeysConfiguration[] = {
	L"HotKeys",
	L"�ȼ�"
};

WCHAR *szColorProcess[] = {
	L"Process",
	L"����"
};

WCHAR *szColorFile[] = {
	L"File",
	L"�ļ�"
};

WCHAR *szColorGeneral[] = {
	L"General",
	L"ͨ��"
};

WCHAR *szColorNormalItem[] = {
	L"Normal module, the corporation is not Micorsoft",
	L"�ļ����̲���΢����ģ��"
};

WCHAR *szColorMicrosoftItem[] = {
	L"The corporation of module is Micorsoft",
	L"�ļ�������΢����ģ��"
};

WCHAR *szColorHiddenOrHooksItem[] = {
	L"Hidden objects or hooks",
	L"���ض��󡢿��ɶ��󡢹���"
};

WCHAR *szColorNotSignedItem[] = {
	L"No signature module",
	L"û��ǩ����ģ��"
};

WCHAR *szDecimal[] = {
	L"Decimal",
	L"ʮ����"
};

WCHAR *szHexadecimal[] = {
	L"Hexadecimal",
	L"ʮ������"
};

WCHAR *szPort[] = {
	L"Port",
	L"��������"
};

WCHAR *szHostsFile[] = {
	L"Hosts File",
	L"Hosts�ļ�"
};

WCHAR *szSPI[] = {
	L"LSP",
	L"LSP"
};

WCHAR *szNetwork[] = {
	L"Network",
	L"����"
};

WCHAR *szPortProcess[] = {
	L"Process",
	L"����"
};

WCHAR *szPortPID[] = {
	L"PID",
	L"PID"
};

WCHAR *szPortProtocol[] = {
	L"Protocol",
	L"Э��"
};

WCHAR *szPortLocalAddress[] = {
	L"Local Address",
	L"���ص�ַ"
};

WCHAR *szPortLocalPort[] = {
	L"Local Port",
	L"���ض˿�"
};

WCHAR *szPortRemoteAddress[] = {
	L"Remote Address",
	L"Զ�̵�ַ"
};

WCHAR *szPortRemotePort[] = {
	L"Remote Port",
	L"Զ�̶˿�"
};

WCHAR *szPortState[] = {
	L"State",
	L"״̬"
};

WCHAR *szVerified[] = {
	L"(Verified) ",
	L"(��ǩ��) "
};

WCHAR *szNotVerified[] = {
	L"(Not Verified) ",
	L"(δǩ��) "
};

WCHAR *szTcpUdpStatus[] = {
	L"Tcp: %d, Udp: %d",
	L"Tcp: %d, Udp: %d",
};

WCHAR *szPortCopyFullRow[] = {
	L"Copy Full Row",
	L"��������",
};

WCHAR *szPortCopyRemoteAddress[] = {
	L"Copy Remote Address",
	L"����Զ�̵�ַ",
};

WCHAR *szPortVerifyProcessSignature[] = {
	L"Verify Process Signature",
	L"��֤��������ǩ��"
};

WCHAR *szPortProcessProperties[] = {
	L"Process Properties",
	L"�鿴�����ļ�����"
};

WCHAR *szPortFindInExplorer[] = {
	L"Find Process File In Explorer",
	L"����Դ�������ж�λ�����ļ�"
};

WCHAR *szPortFindInAntiSpy[] = {
	L"Find Process File In AntiSpy",
	L"��AntiSpy�ļ��������ж�λ�����ļ�"
};

WCHAR *szPortKillProcess[] = {
	L"Kill Process",
	L"������������"
};

WCHAR *szOpenWithNotepad[] = {
	L"Open With Notepad",
	L"�ü��±���"
};

WCHAR *szHooks[] = {
	L"Hooks",
	L"����"
};

WCHAR *szFileSearchOnline[] = {
	L"Search File Name Online",
	L"���������ļ���"
};

WCHAR *szFileCalcMd5[] = {
	L"Calc File Hash",
	L"�����ļ���ϣֵ"
};

WCHAR *szFileHashFile[] = {
	L"File",
	L"�ļ�"
};

WCHAR *szFileHashCompare[] = {
	L"Compare",
	L"�Ƚ�"
};

WCHAR *szFileHashDlgTitle[] = {
	L"File Hash Calculator",
	L"�ļ���ϣ������"
};

WCHAR *szFileHashBtnCompare[] = {
	L"Compare",
	L"�Ƚ�"
};

WCHAR *szFileHashBtnCalc[] = {
	L"Calc",
	L"����"
};

WCHAR *szFileHashCalcing[] = {
	L"Calcing...",
	L"������..."
};

WCHAR *szIeShell[] = {
	L"IE Shell",
	L"IE�Ҽ��˵�"
};

WCHAR *szIeShellStatus[] = {
	L"IE Shell: %d",
	L"IE�Ҽ��˵��%d"
};

WCHAR *szRclickDeleteReg[] = {
	L"Delete Reg",
	L"ɾ��ע���"
};

WCHAR *szIePlugin[] = {
	L"IE Plugin",
	L"IE���"
};

WCHAR *szIePluginStatus[] = {
	L"Total: %d, BHO: %d, IE Shell: %d, URL Search Hooks: %d, Distribution Units: %d, Toolbar: %d, ActiveX: %d",
	L"�ܹ���%d��BHO��%d���������չ��%d��URL��ѯ���ӣ�%d��Distribution Units��%d����������%d��ActiveX��%d"
};

WCHAR *szIEPluginDeleteReg[] = {
	L"Delete Reg",
	L"ɾ��ע���"
};

WCHAR *szIEPluginDeleteFileAndReg[] = {
	L"Delete Reg And File",
	L"ɾ��ע������ļ�"
};

WCHAR *szEnable[] = {
	L"Enable",
	L"����"
};

WCHAR *szDisable[] = {
	L"Disable",
	L"����"
};

WCHAR *szIEPluginCopyName[] = {
	L"Copy Name",
	L"���Ʋ����"
};

WCHAR *szIEPluginCopyPath[] = {
	L"Copy Path",
	L"���Ʋ��·��"
};

WCHAR *szIEPluginSearchOnline[] = {
	L"Search Online",
	L"�������������"
};

WCHAR *szIEPluginShuxing[] = {
	L"Properties",
	L"�鿴�������"
};

WCHAR *szIEPluginSign[] = {
	L"Verify File Signature",
	L"��֤���ǩ��"
};

WCHAR *szToolsHexEditor[] = {
	L"Hex Editor",
	L"16���Ʊ༭��"
};

WCHAR *szTools[] = {
	L"Tools",
	L"����"
};

WCHAR *szToolsDisassembler[] = {
	L"Disassembler",
	L"�������"
};

WCHAR *szAdvancedTools[] = {
	L"Advanced Tools",
	L"�߼����߼�"
};

WCHAR *szUsers[] = {
	L"Users",
	L"ϵͳ�û�"
};

WCHAR *szMBR[] = {
	L"MBR",
	L"MBR"
};

WCHAR *szHexEditerPid[] = {
	L"Pid:",
	L"Pid��"
};

WCHAR *szHexEditerInputHaveWrong[] = {
	L"Input values maybe have something wrong. ",
	L"�����ֵ������������������Ĳ���"
};

WCHAR *szHexEditerKernelModeAddressWrong[] = {
	L"Kernel mode address must be greater than 0x80000000. ",
	L"�ں�ģʽ��ַ�������0x80000000"
};

WCHAR *szHexEditerUserModeAddressWrong[] = {
	L"User mode address must be less than 0x80000000. ",
	L"�û�ģʽ��ַ����С��0x80000000"
};

WCHAR *szHexEditerUserModePidWrong[] = {
	L"User mode address must specify the pid. ",
	L"�û�ģʽ��ַ����ָ����Ӧ���̵�pid"
};

WCHAR *szHexEditerModifyKernelMode[] = {
	L"Modify kernel mode memory maybe cause your system crash.\r\n\r\nAre you sure you want to continue?",
	L"�޸��ں�ģʽ���ڴ棬���ܻ�����ϵͳ������\r\n\r\n��ȷ��Ҫ������"
};

WCHAR *szAutorunRefesh[] = {
	L"Refresh All",
	L"ȫ��ˢ��"
};

WCHAR *szFindDllsOrModulesInputHaveWrong[] = {
	L"Input values can't be empty. ",
	L"Ҫ���ҵ����Ʋ���Ϊ�ա�"
};

WCHAR *szAreYouSureModifyMemory[] = {
	L"Are you sure you want to modify the memory?",
	L"��ȷ��Ҫ�޸ĸ��ڴ���"
};

WCHAR *szModifyMemoryFailed[] = {
	L"Modify Memory Failed!",
	L"�޸��ڴ�ʧ�ܣ�"
};

WCHAR *szModifyMemorySuccess[] = {
	L"Modify Memory OK!",
	L"�޸��ڴ�ɹ���"
};

WCHAR *szSearchDoneDlls[] = {
	L"Search done. DLLs: %d",
	L"������ϣ�����ģ�飺 %d",
};

WCHAR *szSearchDoneHandles[] = {
	L"Search done. Handles: %d",
	L"������ϣ����־���� %d",
};

WCHAR *szSearching[] = {
	L"[Scanning...]",
	L"[�������������Ժ�...]"
};

WCHAR *szSelfProtection[] = {
	L"Self-Defense",
	L"���ұ���"
};

WCHAR *szConfigPS[] = {
	L"Ps: All the settings will take effect when the application restarted.",
	L"Ps�������κ�һ��������Ҫ��������������Ч��"
};

WCHAR *szForbidCreatingProcess[] = {
	L"Forbid Creating Process",
	L"��ֹ��������"
};

WCHAR *szForbidCreatingThread[] = {
	L"Forbid Creating Thread",
	L"��ֹ�����߳�"
};

WCHAR *szForbidCreatingFile[] = {
	L"Forbid Creating File",
	L"��ֹ�����ļ�"
};

WCHAR *szForbidSwitchingDesktop[] = {
	L"Forbid Switching Desktop",
	L"��ֹ�л�����"
};

WCHAR *szForbidCreatingKeyAndValue[] = {
	L"Forbid Creating Key And Value",
	L"��ֹ����ע�������ֵ"
};

WCHAR *szForbidSettingKeyValue[] = {
	L"Forbid Setting Key Value",
	L"��ֹ����ע���ֵ"
};

WCHAR *szForbidLoadingImage[] = {
	L"Forbid Loading Image",
	L"��ֹ����ģ��"
};

WCHAR *szForbidGlobalMessageHook[] = {
	L"Forbid Global Message Hook",
	L"��ֹȫ����Ϣ����ע��"
};

WCHAR *szForbidSystemShutdown[] = {
	L"Forbid System Power Operation",
	L"��ֹ�ػ���������ע��"
};

WCHAR *szForbidModifyingSystemTime[] = {
	L"Forbid Modifying System Time",
	L"��ֹ�޸�ϵͳʱ��"
};

WCHAR *szForbidLockingWorkStation[] = {
	L"Forbid Locking Work Station",
	L"��ֹ���������"
};

WCHAR *szForbidLoadingDriver[] = {
	L"Forbid Loading Driver",
	L"��ֹ��������"
};

WCHAR *szForbid[] = {
	L"Forbid",
	L"��ֹ"
};

WCHAR *szAreYouSureReboot[] = {
	L"Are you sure you want to reboot the system?",
	L"��ȷ��Ҫ�����������"
};

WCHAR *szAreYouSureForceReboot[] = {
	L"Are you sure you want to reboot the system?",
	L"��ȷ��Ҫ�����������"
};

WCHAR *szAreYouSureShutdown[] = {
	L"Are you sure you want to power off the system?",
	L"��ȷ��Ҫ�رռ������"
};

WCHAR *szReboot[] = {
	L"System Reboot",
	L"���������"
};

WCHAR *szForceReboot[] = {
	L"Force System Reboot",
	L"ǿ�����������"
};

WCHAR *szPowerOff[] = {
	L"System Power Off",
	L"�رռ����"
};

WCHAR *szPower[] = {
	L"Power",
	L"��Դ"
};

WCHAR *szToolsEnable[] = {
	L"Enable",
	L"����"
};

WCHAR *szEnableRegistryTools[] = {
	L"Enable RegEdit",
	L"����ע����༭��"
};

WCHAR *szEnableTaskTools[] = {
	L"Enable Task Manager",
	L"�������������"
};

WCHAR *szEnableCmd[] = {
	L"Enable CMD",
	L"�������������(CMD.exe)"
};

WCHAR *szEnableControlPane[] = {
	L"Enable Control Pane",
	L"�����������"
};

WCHAR *szEnableStartMenu[] = {
	L"Enable Taskbar Right Click Menu",
	L"�����������Ҽ��˵�"
};

WCHAR *szClearAllGroupPolicies[] = {
	L"Clear All Group Policies",
	L"���������"
};

WCHAR *szEnableRun[] = {
	L"Enable Run Feature in Start Menu",
	L"������ʼ�˵��е����й���"
};

WCHAR *szRepairLSPOK[] = {
	L"Repair LSP OK!",
	L"�޸�LSP�ɹ���"
};

WCHAR *szRepairSafebootOK[] = {
	L"Repair Safeboot OK!",
	L"�޸���ȫģʽ�ɹ���"
};

WCHAR *szRepairSafebootFailed[] = {
	L"Repair Safeboot Failed!",
	L"�޸���ȫģʽʧ�ܣ�"
};

WCHAR *szRepairLSPFailed[] = {
	L"Repair LSP Failed!",
	L"�޸�LSPʧ�ܣ�"
};

WCHAR *szAllEnable[] = {
	L"Enabled OK!",
	L"�����ɹ���",
};

WCHAR *szPleaseChoseEnable[] = {
	L"Please select the items you want to enable.",
	L"��ѡ����Ҫ�������",
};

WCHAR *szSafeboot[] = {
	L"Safeboot",
	L"��ȫģʽ",
};

WCHAR *szRepair[] = {
	L"Repair",
	L"�޸�",
};

WCHAR *szRepairAll[] = {
	L"Repair All",
	L"ȫ���޸�",
};

WCHAR *szWhatIsMBR[] = {
	L"The MBR, short for the Master Boot Record, it contains the primary boot loader.",
	L"MBR����Master Boot Record�ļ�ƣ�����Ӳ�̵���������¼��",
};

WCHAR *szHowToReapirMBR[] = {
	L"Some Trojans can modify the default MBR, and it can not be killed even you reinstall the system.",
	L"��Щľ�����޸�ϵͳĬ�ϵ�MBR����������MBRľ������ʹ��װϵͳҲ������ȫ��������Ҫ�볹�׽���������⣬����Ҫ�ָ�ϵͳԭʼ��MBR��",
};

WCHAR *szMBRnote[] = {
	L"Note: If your system installed the disk encryption or system restore softwares, do not use this tools to restore the default MBR.",
	L"ע�⣺�������װ�˴��̼����������߻�ԭ�������벻Ҫʹ�ñ����ߣ�������ܻ���ɵ����޷������������ݶ�ʧ�����⡣",
};

WCHAR *szMBRSelectDisk[] = {
	L"Select a physical disk:",
	L"��ѡ���������̣�",
}; 

WCHAR *szMBRIsOk[] = {
	L"The MBR of %s is ok.",
	L"%s��MBR������",
}; 

WCHAR *szMBRIsError[] = {
	L"The MBR of %s may be modified.",
	L"%s��MBR�����Ѿ����޸��ˡ�",
}; 

WCHAR *szReadAndCheckMBR[] = {
	L"Read And Check MBR",
	L"��ȡ�����MBR",
}; 

WCHAR *szBackupMBR[] = {
	L"Backup MBR To File",
	L"����MBR(���鱣�浽U�̻�����)",
}; 

WCHAR *szRestoreMBR[] = {
	L"Restore MBR From Backup File",
	L"�ӱ����ļ��лָ�MBR",
}; 

WCHAR *szRestoreDefaultMBR[] = {
	L"Restore Default MBR",
	L"�ָ�ϵͳĬ��MBR",
}; 

WCHAR *szBackupMBROK[] = {
	L"Backup MBR OK!",
	L"����MBR�ɹ���",
}; 

WCHAR *szBackupMBRERROR[] = {
	L"Backup MBR Failed!",
	L"����MBRʧ�ܣ�",
}; 

WCHAR *szRestoreMBRNotify[] = {
	L"Restore the MBR has certain risk, are you sure you want to continue?",
	L"�ָ�MBR��һ���ķ��գ���ȷ���Ƿ�Ҫ������\r\n\r\nPs: �ָ�������ɱ���������ܻ�����ʾ����ѡ��������",
}; 

WCHAR *szMbrFileInvalid[] = {
	L"The MBR backup file is invalid. ",
	L"MBR�����ļ���Ч��",
};

WCHAR *szRestoreMBROK[] = {
	L"Restore MBR OK! ",
	L"�ָ�MBR�ɹ���",
}; 

WCHAR *szRestoreMBRERROR[] = {
	L"Restore MBR Failed! ",
	L"�ָ�MBRʧ�ܣ�",
}; 

WCHAR *szDiskPartitionInfoError[] = {
	L"Disk partition information is error,so cannot restore MBR to default!",
	L"���̷�����Ϣ�������Բ��ָܻ���Ĭ��MBR��",
}; 

WCHAR *szEanbleRun[] = {
	L"Enable Run Feature in Start Menu",
	L"������ʼ�˵��е����й���",
}; 

WCHAR *szDonate[] = {
	L"Donate",
	L"����",
}; 

WCHAR *szDonateString[] = {
	L"Donate",
	L"AntiSpy�Ǵ����ڴ�ѧ��Ϳ�ʼд��һ��ߣ�\r\n�ڼ�϶�������ʱ2�����ࡣ\r\n",
}; 

WCHAR *szIME[] = {
	L"IME",
	L"IME���뷨",
}; 

WCHAR *szFileAssociation[] = {
	L"File Association",
	L"�ļ�����",
}; 

WCHAR *szIFEO[] = {
	L"Image Hijacks(IFEO)",
	L"ӳ��ٳ�",
}; 

WCHAR *szRegPath[] = {
	L"Reg Path",
	L"ע���·��",
}; 

WCHAR *szPath[] = {
	L"Path",
	L"·��",
}; 

WCHAR *szIMEPath[] = {
	L"IME File Path",
	L"IME����·��",
}; 

WCHAR *szIMEFileCorporation[] = {
	L"IME File Corporation",
	L"IME�ļ�����",
}; 

WCHAR *szLayoutPath[] = {
	L"Layout File Path",
	L"Layout�ļ�·��",
}; 

WCHAR *szLayoutFileCompany[] = {
	L"Layout File Corporation",
	L"Layout�ļ�����",
}; 

WCHAR *szIMEFileProperties[] = {
	L"IME File Properties",
	L"�鿴IME�ļ�����",
}; 

WCHAR *szLayoutFileProperties[] = {
	L"Layout File Properties",
	L"�鿴Layout�ļ�����",
}; 

WCHAR *szVerifyIMEFileSignature[] = {
	L"Verify IME File Signature",
	L"��֤IME�ļ�ǩ��",
}; 

WCHAR *szVerifyLayoutFileSignature[] = {
	L"Verify Layout File Signature",
	L"��֤Layout�ļ�ǩ��",
};

WCHAR *szFileIMEFileInExplorer[] = {
	L"Find IME File In Explorer",
	L"����Դ�������ж�λIME�ļ�"
};

WCHAR *szFileLayoutFileInExplorer[] = {
	L"Find Layout File In Explorer",
	L"����Դ�������ж�λLayout�ļ�"
};

WCHAR *szFileIMEFileInAntiSpy[] = {
	L"Find IME File In AntiSpy",
	L"��AntiSpy�ļ��������ж�λIME�ļ�"
};

WCHAR *szFileLayoutFileInAntiSpy[] = {
	L"Find Layout File In AntiSpy",
	L"��AntiSpy�ļ��������ж�λLayout�ļ�"
};

WCHAR *szImeStatus[] = {
	L"IMEs: %d",
	L"IME���뷨��%d"
};

WCHAR *szImageHijackName[] = {
	L"Be Hijacked Process Name",
	L"���ٳֵĽ�����"
};

WCHAR *szHijackProcessPath[] = {
	L"Hijack Process Path",
	L"�ٳֵĽ���·��"
};

WCHAR *szImageHijacks[] = {
	L"Image Hijacks: %d",
	L"ӳ��ٳ�������%d"
};

WCHAR *szHijackFileProperties[] = {
	L"Hijack File Properties",
	L"�鿴�ٳ��ļ�����"
};

WCHAR *szHijackFileSignature[] = {
	L"Verify Hijack File Signature",
	L"����ٳ��ļ�ǩ��"
};

WCHAR *szHijackFileInExplorer[] = {
	L"Find Hijack File In Explorer",
	L"����Դ�������ж�λ�ٳ��ļ�"
};

WCHAR *szHijackFileInAntiSpy[] = {
	L"Find Hijack File In AntiSpy",
	L"��AntiSpy�ļ��������ж�λ�ٳ��ļ�"
};

WCHAR *szResetToDefult[] = {
	L"Reset To Default",
	L"����ΪĬ��"
};

WCHAR *szAreSureResetToDefult[] = {
	L"Are you sure you want to reset the hosts to defult?",
	L"��ȷ��Ҫ��Hosts�ļ�����ΪĬ����"
};

WCHAR *szFindWindow[] = {
	L"Find Window",
	L"���Ҵ���"
};

WCHAR *szFindWindowShuoming[] = {
	L"Drag the Find Tool over a window to select it,\r\nthen release the mouse button.",
	L"���϶�����ָ�뵽��Ҫ�鿴�Ĵ������ͷ�"
};

WCHAR *szFindWindowPid[] = {
	L"PID",
	L"����ID"
};

WCHAR *szFindWindowTid[] = {
	L"TID",
	L"�߳�ID"
};

WCHAR *szFindProcess[] = {
	L"Find Process",
	L"��λ����"
};

WCHAR *szFindThread[] = {
	L"Find Thread",
	L"��λ�߳�"
};

WCHAR *szFindWindowsKillProcess[] = {
	L"Kill Process",
	L"��������"
};

WCHAR *szFindWindowsKillThread[] = {
	L"Kill Thread",
	L"�����߳�"
};

WCHAR *szAreYOUSureKillTheProcesses[] = {
	L"Are you sure you want to kill the process?",
	L"��ȷ��Ҫ�������������"
};

WCHAR *szAreYOUSureKillTheThread[] = {
	L"Are you sure you want to kill the thread?",
	L"��ȷ��Ҫ��������߳���"
};

WCHAR *szMinimumLength[] = {
	L"Minimum Length (Chars)",
	L"��̳���(�ַ���)"
};

WCHAR *szStringType[] = {
	L"String Type",
	L"�ַ�������"
};

WCHAR *szMemoryRegionsType[] = {
	L"Search in the following types of memory regions",
	L"�������ڴ������"
};

WCHAR *szYourSystemUpdate[] = {
	L"Your system has just installed the windows updates, \r\nplease reboot your system at frist.",
	L"���ĵ��Կ��ܸոմ���ϵͳ���������������Ժ���ʹ�ñ����ߣ�лл��"
};