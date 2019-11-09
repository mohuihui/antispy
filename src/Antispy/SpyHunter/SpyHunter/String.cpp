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
	L"反间谍记录器",
};

WCHAR *szAntiScreenLogger[] = {
	L"Anti Screen Logger",
	L"反屏幕截屏记录器",
};

WCHAR *szRegistryIsNoExsit[] = {
	L"Cannot find",
	L"无法找到",
};

WCHAR *szEnableHotkeys[] = {
	L"Enable HotKeys",
	L"开启热键",
};

WCHAR *szGoto[] = {
	L"Goto",
	L"转到",
};

WCHAR *szProcess[] = {
	L"Process",
	L"进程"
};

WCHAR *szUnknowModule[] = {
	L"Unknow Module",
	L"未知模块"
};

WCHAR *szKernelModule[] = {
	L"Driver",
	L"驱动模块"
};

WCHAR *szImageName[] = {
	L"Image Name",
	L"映像名称"
};

WCHAR *szPid[] = {
	L"Pid",
	L"进程ID"
};

WCHAR *szParentPid[] = {
	L"Parent Pid",
	L"父进程ID"
};

WCHAR *szImagePath[] = {
	L"Image Path",
	L"映像路径"
};

WCHAR *szR3Access[] = {
	L"Ring3 Access",
	L"应用层访问"
};

WCHAR *szRenameKey[] = {
	L"Rename Key",
	L"重命名项"
};

WCHAR *szCreateKey[] = {
	L"Create Key",
	L"新建项"
};

WCHAR *szThisKeyAlreadyExists[] = {
	L"Can not create the key. This key already exists.",
	L"创建注册表项失败，此注册表项已经存在。"
};

WCHAR *szKeyName[] = {
	L"Key Name:",
	L"项名称："
};

WCHAR *szValueName[] = {
	L"Value Name:",
	L"值名称："
};

WCHAR *szFileCorporation[] = {
	L"File Corporation",
	L"文件厂商"
};

WCHAR *szProcessState[] = {
	L"Processes: %d, Hidden Processes: %d, Ring3 Inaccessible Processes: %d",
	L"进程：%d，隐藏进程：%d，应用层不可访问进程：%d"
};

WCHAR *szProcessModuleState[] = {
	L", [%s] Modules: %d/%d",
	L"，[%s] 模块：%d/%d"
};

WCHAR *szProcessHandleState[] = {
	L", [%s] Handles: %d",
	L"，[%s] 句柄：%d"
};

WCHAR *szProcessThreadState[] = {
	L", [%s] Threads: %d",
	L"，[%s] 线程：%d"
};

WCHAR *szProcessMemoryState[] = {
	L", [%s] Memorys: %d",
	L"，[%s] 内存：%d"
};

WCHAR *szProcessWindowsState[] = {
	L", [%s] Windows: %d",
	L"，[%s] 窗口：%d"
};

WCHAR *szProcessHotKeysState[] = {
	L", [%s] HotKeys: %d",
	L"，[%s] 热键：%d"
};

WCHAR *szProcessPrivilegesState[] = {
	L", [%s] Enable: %d, Disable: %d",
	L"，[%s] Enable：%d，Disable：%d"
};

WCHAR *szProcessTimersState[] = {
	L", [%s] Timers: %d",
	L"，[%s] 定时器：%d"
};

WCHAR *szDeny[] = {
	L"Deny",
	L"拒绝"
};

WCHAR *szProcessObject[] = {
	L"Process Object",
	L"进程对象"
};

WCHAR *szViewModules[] = {
	L"View Modules",
	L"查看进程模块"
};

WCHAR *szFind[] = {
	L"Find",
	L"查找"
};

WCHAR *szProcessRefresh[] = {
	L"Refresh (F5)",
	L"刷新 (F5)"
};

WCHAR *szRefresh[] = {
	L"Refresh",
	L"刷新"
};

WCHAR *szDelete[] = {
	L"Delete",
	L"删除"
};

WCHAR *szViewThreads[] = {
	L"View Threads",
	L"查看进程线程"
};

WCHAR *szViewHandles[] = {
	L"View Handles",
	L"查看进程句柄"
};

WCHAR *szViewWindows[] = {
	L"View Windows",
	L"查看进程窗口"
};

WCHAR *szViewMemory[] = {
	L"View Memory",
	L"查看进程内存"
};

WCHAR *szViewTimers[] = {
	L"View Timers",
	L"查看进程定时器"
};

WCHAR *szViewHotKeys[] = {
	L"View HotKeys",
	L"查看进程热键"
};

WCHAR *szViewStrings[] = {
	L"View Strings",
	L"查看进程字符串"
};

WCHAR *szViewPrivileges[] = {
	L"View Privileges",
	L"查看进程权限"
};

WCHAR *szKillProcess[] = {
	L"Kill (Del)",
	L"结束进程 (Del)"
};

WCHAR *szView[] = {
	L"View...",
	L"查看..."
};

WCHAR *szShowLowerPane[] = {
	L"Show Lower Pane",
	L"在下方显示上述信息"
};

WCHAR *szForceKillProcess[] = {
	L"Force Kill",
	L"强制结束进程"
};

WCHAR *szShowProcessTree[] = {
	L"Show Process Tree",
	L"显示进程树"
};

WCHAR *szKillAndDeleteFile[] = {
	L"Kill And Delete File",
	L"结束进程并删除文件"
};

WCHAR *szForceKillAndDelete[] = {
	L"Force Kill And Delete File",
	L"强制结束进程并删除文件"
};

WCHAR *szVerifyProcessSignature[] = {
	L"Verify Process Signature",
	L"验证当前进程数字签名"
};

WCHAR *szVerifyAllProcessSignature[] = {
	L"Verify All Process Signature",
	L"验证所有进程数字签名"
};

WCHAR *szSuspend[] = {
	L"Suspend",
	L"暂停进程运行"
};

WCHAR *szResume[] = {
	L"Resume",
	L"恢复进程运行"
};

WCHAR *szCopyImageName[] = {
	L"Copy Image Name",
	L"复制进程名"
};

WCHAR *szCopyImagePath[] = {
	L"Copy Image Path",
	L"复制进程路径"
};

WCHAR *szAnalyseOnline[] = {
	L"Analyse Online",
	L"在线文件鉴定"
};

WCHAR *szSearchOnlineBaidu[] = {
	L"Search Online(Baidu)",
	L"在线搜索进程名(Baidu)"
};

// WCHAR *szSearchOnlineGoogle[] = {
// 	L"Search Online(Google)",
// 	L"在线搜索进程名(Google)"
// };

WCHAR *szSearchOnlineGoogle[] = {
	L"Search Online",
	L"在线搜索模块名"
};

WCHAR *szSearchProcessOnline[] = {
	L"Search Online",
	L"在线搜索进程名"
};

WCHAR *szFindInExplorer[] = {
	L"Find Target In Explorer",
	L"在资源管理器中定位文件"
};

WCHAR *szFindInAntiSpy[] = {
	L"Find Target In AntiSpy",
	L"在AntiSpy文件管理器中定位文件"
};

WCHAR *szProcessProperties[] = {
	L"Properties",
	L"查看进程文件属性"
};

WCHAR *szFindModule[] = {
	L"Find DLLs Or Handles",
	L"在所有进程中查找模块或句柄"
};

WCHAR *szFindUnsignedModules[] = {
	L"Find Unsigned DLLs",
	L"在所有进程中查找没有签名模块"
};

WCHAR *szExportTo[] = {
	L"Export To...",
	L"导出到..."
};

WCHAR *szStartType[] = {
	L"Set Start Type",
	L"设置启动方式"
};

WCHAR *szText[] = {
	L"Text",
	L"文本文件"
};

WCHAR *szExportToText[] = {
	L"Export To Text",
	L"导出到文本"
};

WCHAR *szExportToExcel[] = {
	L"Export To Excel",
	L"导出到Excel表格"
};

WCHAR *szStringValue[] = {
	L"String Value",
	L"字符串值"
};

WCHAR *szBinaryValue[] = {
	L"Binary Value",
	L"二进制值"
};

WCHAR *szDwordValue[] = {
	L"DWORD Value",
	L"DWORD值"
};

WCHAR *szMultiStringValue[] = {
	L"Multi-String Value",
	L"多字符串值"
};

WCHAR *szExpandableStringValue[] = {
	L"Expandable String Value",
	L"可扩充字符串值"
};

WCHAR *szExcel[] = {
	L"Excel",
	L"Excel表格"
};

WCHAR *szModuleName[] = {
	L"Module Name",
	L"模块名"
};

WCHAR *szBase[] = {
	L"Base",
	L"基地址"
};

WCHAR *szSize[] = {
	L"Size",
	L"大小"
};

WCHAR *szModulePath[] = {
	L"Module Path",
	L"模块路径"
};

WCHAR *szModuleState[] = {
	L"Image Name: %s, Modules: %d, Suspicious Modules: %d",
	L"映像：%s， 模块：%d， 可疑模块：%d"
};

WCHAR *szNotShowMicrosoftModules[] = {
	L"Hide Microsoft's Modules",
	L"不显示微软模块"
};

WCHAR *szDumpMemory[] = {
	L"Dump Memory",
	L"拷贝模块内存"
};

WCHAR *szExportRegistry[] = {
	L"Export Registry",
	L"导出注册表"
};

WCHAR *szAreYouSureDeleteKey[] = {
	L"Are you sure you want to delete the key and its subkeys?",
	L"确认要删除该项及其所对应的子项吗？"
};

WCHAR *szAreYouSureDeleteValue[] = {
	L"Are you sure you want to delete?",
	L"确认要删除吗？"
};

WCHAR *szAreYouSureUnloadAndDeleteModule[] = {
	L"Are you sure you want to unload and delete the module?",
	L"确认要卸载并删除这个模块吗？"
};

WCHAR *szDeleteKey[] = {
	L"Delete Key",
	L"删除项"
};

WCHAR *szDeleteValue[] = {
	L"Delete Value",
	L"删除值"
};

WCHAR *szUnLoadModule[] = {
	L"Unload",
	L"卸载此模块"
};

WCHAR *szUnLoadModuleInAllProcess[] = {
	L"Unload In All Processes",
	L"在所有进程中卸载此模块"
};

WCHAR *szCopyModuleName[] = {
	L"Copy Module Name",
	L"复制模块名"
};

WCHAR *szCopyModulePath[] = {
	L"Copy Module Path",
	L"复制模块路径"
};

WCHAR *szVerifyModuleSignature[] = {
	L"Verify Signature",
	L"验证数字签名"
};

WCHAR *szVerifyAllModuleSignature[] = {
	L"Verify All Signature",
	L"验证所有数字签名"
};

WCHAR *szModuleProperties[] = {
	L"Properties",
	L"查看模块属性"
};

WCHAR *szThreadId[] = {
	L"Tid",
	L"线程ID"
};

WCHAR *szThreadObject[] = {
	L"Thread Object",
	L"线程对象"
};

WCHAR *szTeb[] = {
	L"Teb",
	L"线程环境块"
};

WCHAR *szPriority[] = {
	L"Priority",
	L"优先级"
};

WCHAR *szStartAddress[] = {
	L"Start Address",
	L"线程入口"
};

WCHAR *szSwitchTimes[] = {
	L"Switch Times",
	L"切换次数"
};

WCHAR *szThreadStatus[] = {
	L"Status",
	L"线程状态"
};

WCHAR *szStartModule[] = {
	L"Start Module",
	L"线程所属模块"
};

WCHAR *szThreadState[] = {
	L"Image Name: %s, Threads: %d, Hideen Threads: %d, No Module Threads: %d",
	L"映像：%s， 线程：%d， 隐藏线程：%d， 无模块线程：%d"
};

WCHAR *szSystemThreadState[] = {
// 	L"Threads: %d, Hideen Threads: %d, No Module Threads: %d",
// 	L"线程：%d， 隐藏线程：%d， 无模块线程：%d"
	L"Threads: %d",
	L"线程：%d"
};

WCHAR *szThreadInfoState[] = {
	L"Threads: %d, Hideen Threads: %d, No Module Threads: %d",
	L"线程：%d， 隐藏线程：%d， 无模块线程：%d"
};

WCHAR *szShowTerminatedThreads[] = {
	L"Show Terminated Threads",
	L"显示已经终止的线程"
};

WCHAR *szKillThread[] = {
	L"Kill",
	L"结束线程"
};

WCHAR *szForceKillThread[] = {
	L"Force Kill",
	L"强制结束线程"
};

WCHAR *szSuspendThread[] = {
	L"Suspend",
	L"挂起线程"
};

WCHAR *szResumeThread[] = {
	L"Resume",
	L"恢复线程"
};

WCHAR *szForceSuspendThread[] = {
	L"Force Suspend",
	L"强制挂起线程"
};

WCHAR *szDisassemblyStartAddress[] = {
	L"Disassemble StartAddress",
	L"反汇编线程入口"
};

WCHAR *szForceResumeThread[] = {
	L"Force Resume",
	L"强制恢复线程"
};

WCHAR *szVerifyStartModuleSignature[] = {
	L"Verify Module Signature",
	L"验证模块签名"
};

WCHAR *szFindStartModuleInExplorer[] = {
	L"Find Module In Explorer",
	L"资源管理器中定位模块"
};

WCHAR *szStartModuleProperties[] = {
	L"Module Properties",
	L"查看模块属性"
};

WCHAR *szProcessModules[] = {
	L"Process Module",
	L"进程模块"
};

WCHAR *szProcessThreads[] = {
	L"Process Thread",
	L"进程线程"
};

WCHAR *szDumpOK[] = {
	L"Dump OK!",
	L"拷贝内存成功！"
};

WCHAR *szDumpFailed[] = {
	L"Dump Failed!",
	L"拷贝内存失败！"
};

WCHAR *szFileExist[] = {
	L"The file has exist, do you truncate it?",
	L"文件已经存在，是否覆盖？"
};

WCHAR *szSureToUnloadModule[] = {
	L"Unload modules maybe cause the target process crash,\r\are you sure you want to continue?",
	L"卸载模块可能会导致目标进程奔溃，您确定要卸载吗？"
};

WCHAR *szGlobalUnloadModule[] = {
	L"Unload modules in some system processes maybe cause your system crash,\r\nare you sure you want to continue?",
	L"卸载系统进程中的某些模块可能会导致您的系统不稳定或者奔溃，\r\n确定要卸载吗？"
};

WCHAR *szUnloadModule[] = {
	L"Unload Module",
	L"卸载模块"
};

WCHAR *szHandleType[] = {
	L"Type",
	L"句柄类型"
};

WCHAR *szHandleName[] = {
	L"Name",
	L"句柄名"
};

WCHAR *szHandleValue[] = {
	L"Handle",
	L"句柄"
};

WCHAR *szHandleObject[] = {
	L"Object",
	L"句柄对象"
};

WCHAR *szHandleRefCount[] = {
	L"RefCount",
	L"引用计数"
};

WCHAR *szHandleState[] = {
	L"Image Name: %s, Handles: %d",
	L"映像：%s，  句柄：%d",
};

WCHAR *szHandleInfoState[] = {
	L"Handles: %d",
	L"句柄数：%d",
};

WCHAR *szHandleTypeNumber[] = {
	L"Type Number",
	L"句柄类型代号"
};

WCHAR *szProcessHandles[] = {
	L"Process Handle",
	L"进程句柄"
};

WCHAR *szHandleClose[] = {
	L"Close",
	L"关闭句柄"
};

WCHAR *szHandleForceClose[] = {
	L"Force Close",
	L"强制关闭句柄"
};

WCHAR *szAddress[] = {
	L"Address",
	L"地址"
};

WCHAR *szHex[] = {
	L"Hex",
	L"十六进制"
};

WCHAR *szDisassembly[] = {
	L"Disassembly",
	L"反汇编"
};

WCHAR *szMemoryState[] = {
	L"Image Name: %s, Memorys: %d",
	L"映像： %s， 内存块： %d"
};

WCHAR *szMemoryInfoState[] = {
	L"Memorys: %d",
	L"内存块： %d"
};

WCHAR *szProcessMemory[] = {
	L"Process Memory",
	L"进程内存"
};

WCHAR *szMemoryDump[] = {
	L"Dump Memory",
	L"拷贝内存"
};

WCHAR *szMemoryDumpSelect[] = {
	L"Dump Selected",
	L"拷贝所选内存"
};

WCHAR *szMemoryZero[] = {
	L"Zero Memory",
	L"清零内存"
};

WCHAR *szMemoryModifyProtection[] = {
	L"Modify Protection",
	L"修改保护属性"
};

WCHAR *szMemoryFree[] = {
	L"Free",
	L"释放内存"
};

WCHAR *szDumpAddress[] = {
	L"Address:",
	L"地址："
};

WCHAR *szDumpSize[] = {
	L"Size:",
	L"大小："
};

WCHAR *szOk[] = {
	L"OK",
	L"确定"
};

WCHAR *szCancel[] = {
	L"Cancel",
	L"取消"
};

WCHAR *szDumpDlgMemory[] = {
	L"Dump Memory",
	L"拷贝内存"
};

WCHAR *szMemoryFreeMemory[] = {
	L"Free Memory",
	L"释放内存"
};

WCHAR *szMemoryZeroMemory[] = {
	L"Zero Memory",
	L"清零内存"
};

WCHAR *szMemoryFreeFailed[] = {
	L"Free Memory Failed!",
	L"释放内存失败！"
};

WCHAR *szMemoryDangerous[] = {
	L"It maybe cause the target process crash, are you sure you want to contionue?",
	L"此操作可能会造成目标程序奔溃，您确定要继续吗？"
};

WCHAR *szModifyProtection[] = {
	L"Modify Protection",
	L"修改保护属性"
};

WCHAR *szModifyProtectionFailed[] = {
	L"Modify Protection Failed!",
	L"修改保护属性失败！"
};

WCHAR *szModifyMemoryProtection[] = {
	L"Modify Protection",
	L"修改内存保护属性"
};

WCHAR *szZeroMemory[] = {
	L"Zero Memory",
	L"清零内存"
};

WCHAR *szZeroMemoryFailed[] = {
	L"Zero Memory Failed!",
	L"清零内存失败！"
};

WCHAR *szZeroMemoryOk[] = {
	L"Zero Memory Ok!",
	L"清零内存成功！"
};

WCHAR *szShowAllProcessesTimer[] = {
	L"Show All Processes Timer",
	L"显示所有进程定时器"
};

WCHAR *szRemove[] = {
	L"Remove",
	L"移除"
};

WCHAR *szModifyTimeOutValue[] = {
	L"Modify Time Out Value",
	L"修改响应时间"
};

WCHAR *szProcessTimer[] = {
	L"Process Timer",
	L"进程定时器"
};

WCHAR *szTimerState[] = {
	L"Image Name: %s, Timers: %d",
	L"映像： %s， 定时器： %d"
};

WCHAR *szTimerStateNew[] = {
	L"Timers: %d",
	L"定时器：%d"
};

WCHAR *szTimerObject[] = {
	L"Object",
	L"定时器对象"
};

WCHAR *szTimeOutValue[] = {
	L"Time Out(s)",
	L"响应时间(s)"
};

WCHAR *szCallback[] = {
	L"Callback",
	L"回调函数"
};

WCHAR *szTimerModulePath[] = {
	L"Module Path",
	L"定时器所属模块"
};

WCHAR *szTid[] = {
	L"Tid",
	L"线程ID"
};

WCHAR *szAllProcesses[] = {
	L"All Processes",
	L"所有进程"
};

WCHAR *szProcesseName[] = {
	L"Process Name",
	L"进程名"
};

WCHAR *szHotKeyObject[] = {
	L"Object",
	L"热键对象"
};

WCHAR *szHotKeyId[] = {
	L"Id",
	L"热键Id"
};

WCHAR *szHotKeyKey[] = {
	L"Key",
	L"热键"
};

WCHAR *szHotKeyTid[] = {
	L"Tid",
	L"线程Id"
};

WCHAR *szHotKeyPid[] = {
	L"Pid",
	L"进程Id"
};

WCHAR *szHotKeyProcessName[] = {
	L"Process Name",
	L"进程名"
};

WCHAR *szHotKeyState[] = {
	L"Image Name: %s, HotKeys: %d",
	L"映像：%s， 热键：%d"
};

WCHAR *szHotKeyStateNew[] = {
	L"HotKeys: %d",
	L"热键数：%d"
};

WCHAR *szProcessHotKey[] = {
	L"Process HotKey",
	L"进程热键"
};

WCHAR *szShowAllProcessesHotKey[] = {
	L"Show All Processes HotKey",
	L"显示所有进程热键"
};

WCHAR *szProcessPrivilege[] = {
	L"Process Privilege",
	L"进程权限"
};

WCHAR *szPrivilegeStatus[] = {
	L"Status",
	L"状态"
};

WCHAR *szPrivilege[] = {
	L"Privilege",
	L"权限"
};

WCHAR *szPrivilegeDescription[] = {
	L"Description",
	L"描述"
};

WCHAR *szPrivilegeState[] = {
	L"Image Name: %s, Enabled: %d, Disabled: %d",
	L"映像： %s， Enabled：%d， Disabled：%d"
};

WCHAR *szPrivilegeInfoState[] = {
	L"Enabled: %d, Disabled: %d",
	L"Enabled：%d， Disabled：%d"
};

WCHAR *szPrivilegeEnable[] = {
	L"Enable",
	L"开启权限"
};

WCHAR *szPrivilegeDisable[] = {
	L"Disable",
	L"取消权限"
};

WCHAR *szHwnd[] = {
	L"Hwnd",
	L"窗口句柄"
};

WCHAR *szWindowTitle[] = {
	L"Window Title",
	L"窗口标题"
};

WCHAR *szClassName[] = {
	L"Class Name",
	L"窗口类名"
};

WCHAR *szWndVisible[] = {
	L"Visible",
	L"窗口可见性"
};

WCHAR *szWndState[] = {
	L"Image Name: %s, Windows: %d",
	L"映像： %s， 窗口： %d"
};

WCHAR *szWndStateNew[] = {
	L"Windows: %d",
	L"窗口数： %d"
};

WCHAR *szProcessWindow[] = {
	L"Process Window",
	L"进程窗口"
};

WCHAR *szShowAllProcessWindow[] = {
	L"Show All Process Window",
	L"显示所有进程窗口"
};

WCHAR *szShowWindow[] = {
	L"Show Window",
	L"显示窗口"
};

WCHAR *szHideWindow[] = {
	L"Hide Window",
	L"隐藏窗口"
};

WCHAR *szShowMaximized[] = {
	L"Show Maximized",
	L"最大化窗口"
};

WCHAR *szShowMinimized[] = {
	L"Show Minimized",
	L"最小化窗口"
};

WCHAR *szEnableWindow[] = {
	L"Enable Window",
	L"激活窗口"
};

WCHAR *szDisableWindow[] = {
	L"Disable Window",
	L"冻结窗口"
};

WCHAR *szDestroyWindow[] = {
	L"Destroy Window",
	L"销毁窗口"
};

WCHAR *szTopMost[] = {
	L"Top Most",
	L"窗口置顶"
};

WCHAR *szCancelTopMost[] = {
	L"Cancel Top Most",
	L"取消窗口置顶"
};

WCHAR *szVisible[] = {
	L"Yes",
	L"可见"
};

WCHAR *szProcessDump[] = {
	L"Dump Memory",
	L"拷贝进程内存映像"
};

WCHAR *szFixDump[] = {
	L"Would you like to fix the dump?",
	L"需要修复映像文件吗？"
};

WCHAR *szFindModuleDlg[] = {
	L"Find Handles or DLLs",
	L"查找模块或句柄"
};

WCHAR *szFindDllModuleName[] = {
	L"Filter:",
	L"名称："
};

WCHAR *szFindHnadles[] = {
	L"Hnadles",
	L"句柄"
};

WCHAR *szFindDllModule[] = {
	L"DLLs",
	L"模块"
};

WCHAR *szRegFind[] = {
	L"Find...",
	L"查找..."
};

WCHAR *szRegExport[] = {
	L"Export",
	L"导出注册表"
};

WCHAR *szRegImport[] = {
	L"Import",
	L"导入注册表"
};

WCHAR *szRegImportOk[] = {
	L"Import OK!",
	L"导入成功！"
};

WCHAR *szRegExportOk[] = {
	L"Export OK!",
	L"导出成功！"
};

WCHAR *szRegDelete[] = {
	L"Delete",
	L"删除"
};

WCHAR *szRegModify[] = {
	L"Modify",
	L"修改"
};

WCHAR *szRegCopyValueName[] = {
	L"Copy Value Name",
	L"复制值名称"
};

WCHAR *szUseHiveAnalyze[] = {
	L"Use Hive Analysis",
	L"使用Hive解析"
};

WCHAR *szAddToQuickPositioning[] = {
	L"Add to Quick Position",
	L"添加到快速定位"
};

WCHAR *szRegCopyValueData[] = {
	L"Copy Value Data",
	L"复制值数据"
};

WCHAR *szRegRename[] = {
	L"Rename",
	L"重命名"
};

WCHAR *szRegEditString[] = {
	L"Edit String",
	L"编辑字符串"
};

WCHAR *szRegEditDwordValue[] = {
	L"Edit DWORD Value",
	L"编辑DWORD值"
};

WCHAR *szRegValueName[] = {
	L"Value Name:",
	L"数值名称："
};

WCHAR *szRegBase[] = {
	L"Base",
	L"基数"
};

WCHAR *szRegValueData[] = {
	L"Value Data:",
	L"数值数据："
};

WCHAR *szNewKey[] = {
	L"New Key",
	L"新建项"
};

WCHAR *szNewValue[] = {
	L"New Value",
	L"新建值"
};

WCHAR *szRenameValue[] = {
	L"Rename Value",
	L"重命名值"
};

WCHAR *szCopyKeyName[] = {
	L"Copy Key Name",
	L"复制项名称"
};

WCHAR *szCopyFullKeyName[] = {
	L"Copy Full Key Name",
	L"复制完整项名称"
};

WCHAR *szProcessName[] = {
	L"Process Name",
	L"进程名"
};

WCHAR *szFindModuleStatus[] = {
	L"Module Name: %s, Count: %d",
	L"被查找的模块名：%s，模块数：%d"
};

WCHAR *szDeleteModuleFile[] = {
	L"Delete Module File",
	L"删除模块文件"
};

WCHAR *szUnloadAndDeleteModuleFile[] = {
	L"Unload And Delete File",
	L"卸载并删除模块文件"
};

WCHAR *szUnload[] = {
	L"Unload",
	L"卸载模块"
};

WCHAR *szUnloadAll[] = {
	L"Unload All",
	L"卸载所有模块"
};

WCHAR *szFindModuleInExplorer[] = {
	L"Find Target",
	L"定位模块文件"
};

WCHAR *szFindNotSignatureModule[] = {
	L"Find No Signature Module",
	L"查找没有数字签名模块"
};

WCHAR *szNoSignatureModuleCount[] = {
	L"No Signature Modules: %d",
	L"无数字签名模块：%d"
};

WCHAR *szDeleteFile[] = {
	L"Delete File",
	L"删除文件"
};

WCHAR *szForceResumeProcess[] = {
	L"Force Resume",
	L"强制恢复进程运行"
};

WCHAR *szForceSuspendProcess[] = {
	L"Force Suspend",
	L"强制暂停进程运行"
};

WCHAR *szInjectDll[] = {
	L"Inject DLL",
	L"注入模块"
};

WCHAR *szScaning[] = {
	L"[Scanning...]",
	L"[正在扫描，请稍后...]"
};

WCHAR *szDriverName[] = {
	L"Driver Name",
	L"驱动名"
};

WCHAR *szUserName[] = {
	L"User Name",
	L"用户名"
};

WCHAR *szDiskName[] = {
	L"DISK",
	L"磁盘"
};

WCHAR *szReadMbrFalied[] = {
	L"Read MBR Failed!",
	L"读取MBR失败！"
};

WCHAR *szUserPrivileges[] = {
	L"Privileges",
	L"权限"
};

WCHAR *szUserStatus[] = {
	L"Users: %d, Hidden Users: %d",
	L"用户：%d，隐藏用户：%d"
};

WCHAR *szDriverObject[] = {
	L"Driver Object",
	L"驱动对象"
};

WCHAR *szDriverPath[] = {
	L"Driver Path",
	L"驱动路径"
};

WCHAR *szServiceName[] = {
	L"Service Name",
	L"服务名"
};

WCHAR *szLoadOrder[] = {
	L"Load Order",
	L"加载顺序"
};

WCHAR *szDriverStatus[] = {
	L"Drivers: %d, Hidden Drivers: %d, Suspicious PE Image: %d",
	L"驱动：%d， 隐藏驱动：%d， 可疑PE映像：%d"
};

WCHAR *szSuspiciousPEImage[] = {
	L"Suspicious PE Image",
	L"可疑PE映像"
};

WCHAR *szFileNotFound[] = {
	L"File not found",
	L"文件不存在"
};

WCHAR *szDriverDump[] = {
	L"Dump Memory",
	L"拷贝模块内存"
};

WCHAR *szDriverUnload[] = {
	L"Unload Driver(Dangerous)",
	L"卸载驱动(危险)"
};

WCHAR *szDeleteDriverFile[] = {
	L"Delete Driver File",
	L"删除驱动文件"
};

WCHAR *szDeleteDriverFileAndReg[] = {
	L"Delete Driver File And Reg",
	L"删除驱动文件和注册表"
};

WCHAR *szCopyDriverName[] = {
	L"Copy Driver Name",
	L"拷贝驱动名"
};

WCHAR *szCopyDriverPath[] = {
	L"Copy Driver Path",
	L"拷贝驱动路径"
};

WCHAR *szLocateInRegedit[] = {
	L"Jump to Registry",
	L"定位到注册表"
};

WCHAR *szVerifyDriverSignature[] = {
	L"Verify Driver Signature",
	L"验证驱动签名"
};

WCHAR *szVerifyAllDriverSignature[] = {
	L"Verify All Driver Signature",
	L"验证所有驱动签名"
};

WCHAR *szDriverProperties[] = {
	L"Properties",
	L"查看驱动文件属性"
};

WCHAR *szSearchOnlineDriver[] = {
	L"Search Online",
	L"在线搜索驱动名"
};

WCHAR *szVerifyDlgImagePath[] = {
	L"File Path: ",
	L"文件路径： "
};

WCHAR *szVerifyStatusVerifyNow[] = {
	L"Status: Verify Signature Now, Please Wait...",
	L"签名状态： 正在验证文件签名，请稍后..."
};

WCHAR *szVerifyStatusVerifySigned[] = {
	L"Status: Signed.",
	L"签名状态： 文件已经签名。"
};

WCHAR *szVerifyStatusVerifyNotSigned[] = {
	L"Status: Not Signed.",
	L"签名状态： 文件未通过签名。"
};

WCHAR *szOK[] = {
	L"OK",
	L"确定"
};

WCHAR *szDisasm[] = {
	L"Disasm",
	L"反汇编"
};

WCHAR *szProcessList[] = {
	L"Process List",
	L"进程列表"
};

WCHAR *szDisasmCode[] = {
	L"Disassembly Code",
	L"反汇编代码"
};

WCHAR *szSave[] = {
	L"Save",
	L"保存修改"
};

WCHAR *szReadMemory[] = {
	L"Read Memory",
	L"读取内存"
};

WCHAR *szConfirmToWrite[] = {
	L"Confirm To Write Memory",
	L"确认写入内存"
};

//Show amaranth color on no signed item. \r\n
WCHAR *szVerifysignatureNowPleaseWait[] = {
	L"Verify signature now, please wait...",
	L"正在校验文件签名，请稍后..."
};

WCHAR *szVerifySignature[] = {
	L"Verify Signature",
	L"校验文件签名"
};

WCHAR *szKernelMode[] = {
	L"Kernel Mode",
	L"内核层"
};

WCHAR *szUserMode[] = {
	L"User Mode",
	L"用户层"
};

WCHAR *szRegistry[] = {
	L"Registry",
	L"注册表"
};

WCHAR *szService[] = {
	L"Service",
	L"服务"
};

WCHAR *szAutostart[] = {
	L"Autostart",
	L"启动项"
};

WCHAR *szCommand[] = {
	L"Command",
	L"命令行"
};

WCHAR *szAbout[] = {
	L"About",
	L"关于"
};

WCHAR *szSetConfig[] = {
	L"Config",
	L"设置"
};

WCHAR *szMyComputer[] = {
	L"My Computer",
	L"我的电脑"
};

WCHAR *szAll[] = {
	L"All",
	L"全部启动项"
};

WCHAR *szStartup[] = {
	L"Startup",
	L"启动"
};

WCHAR *szWinlogon[] = {
	L"Winlogon",
	L"系统登录相关"
};

WCHAR *szExplorer[] = {
	L"Explorer",
	L"资源管理器相关"
};

WCHAR *szInternetExplorer[] = {
	L"Internet Explorer",
	L"IE浏览器相关"
};

WCHAR *szAutoRunServices[] = {
	L"Services",
	L"系统服务"
};

WCHAR *szAutoRunDrivers[] = {
	L"Drivers",
	L"驱动程序"
};

WCHAR *szRing0Hook[] = {
	L"Ring0 Hook",
	L"内核钩子"
};

WCHAR *szRoutineEntry[] = {
	L"Routine Entry",
	L"回调入口"
};

WCHAR *szNotifyType[] = {
	L"Notify Type",
	L"通知类型"
};

WCHAR *szNote[] = {
	L"Note",
	L"备注"
};

WCHAR *szCallbackStatus[] = {
	L"CreateProcess: %d, CreateThread: %d, LoadImage: %d, Registry: %d, Shutdown: %d",
	L"CreateProcess: %d, CreateThread: %d, LoadImage: %d, Registry: %d, Shutdown: %d"
};

WCHAR *szCallbackFindInExplorer[] = {
	L"Find Target In Explorer",
	L"在资源管理器中定文件"
//	L"定位模块文件"
};

WCHAR *szCallbackModuleProperties[] = {
	L"Properties",
	L"查看模块属性"
};

WCHAR *szFixLsp[] = {
	L"Repair LSP",
	L"修复LSP"
};

WCHAR *szDisassemblerEntry[] = {
	L"Disassembler Entry",
	L"反汇编入口"
};

WCHAR *szCallbackVerifyModuleSignature[] = {
	L"Verify Signature",
	L"校验模块签名"
};

WCHAR *szRemoveAll[] = {
	L"Remove All",
	L"全部移除"
};

WCHAR *szNotifyRoutine[] = {
	L"Notify Routine",
	L"系统回调"
};

WCHAR *szSSDT[] = {
	L"SSDT",
	L"SSDT"
};

WCHAR *szIdt[] = {
	L"IDT",
	L"系统中断表"
};

WCHAR *szObjectHook[] = {
	L"Object Type",
	L"Object钩子"
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
	L"键盘"
};

WCHAR *szMouClass[] = {
	L"Mouse",
	L"鼠标"
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
	L"DPC定时器"
};

WCHAR *szIoTimer[] = {
	L"IO Timer",
	L"IO定时器"
};

WCHAR *szSystemThread[] = {
	L"System Thread",
	L"系统线程"
};

WCHAR *szHandleInfo[] = {
	L"Handle",
	L"句柄"
};

WCHAR *szWindowInfo[] = {
	L"Window",
	L"窗口"
};

WCHAR *szHotKeyInfo[] = {
	L"HotKey",
	L"热键"
};

WCHAR *szTimerInfo[] = {
	L"Timer",
	L"定时器"
};

WCHAR *szPrivilegeInfo[] = {
	L"Privilege",
	L"权限"
};

WCHAR *szWorkerThread[] = {
	L"Worker Thread",
	L"工作队列线程"
};

WCHAR *szDebugRegisters[] = {
	L"Debug Registers",
	L"调试寄存器"
};

WCHAR *szFilter[] = {
	L"Filter",
	L"过滤驱动"
};

WCHAR *szObjectHijack[] = {
	L"Object Hijack",
	L"对象劫持"
};

WCHAR *szModuleInfo[] = {
	L"Module",
	L"模块"
};

WCHAR *szThreadInfo[] = {
	L"Thread",
	L"线程"
};

WCHAR *szProcessInformation[] = {
	L"Process Information",
	L"进程信息"
};

WCHAR *szModuleInfoState[] = {
	L"Modules: %d",
	L"模块数：%d"
};

WCHAR *szThreadsInfoState[] = {
	L"Threads: %d",
	L"线程数：%d"
};

WCHAR *szMemoryInfo[] = {
	L"Memory",
	L"内存"
};

WCHAR *szDpcTimerObject[] = {
	L"Timer Object",
	L"定时器对象"
};

WCHAR *szDpc[] = {
	L"DPC",
	L"DPC"
};

WCHAR *szPeriod[] = {
	L"Period(s)",
	L"触发周期(s)"
};

WCHAR *szDpcTimerRoutineEntry[] = {
	L"Routine Entry",
	L"函数入口"
};

WCHAR *szDpcTimerStatus[] = {
	L"DPC Timers: %d",
	L"DPC定时器: %d"
};

WCHAR *szDeviceObject[] = {
	L"Device Object",
	L"设备对象"
};

WCHAR *szIoTimerStatus[] = {
	L"Status",
	L"状态"
};

WCHAR *szStop[] = {
	L"Stop",
	L"停止"
};

WCHAR *szStart[] = {
	L"Start",
	L"开启"
};

WCHAR *szIoTimerCnt[] = {
	L"IO Timers: %d",
	L"IO定时器: %d"
};

WCHAR *szIoTimerStatusWorking[] = {
	L"Working",
	L"已开启"
};

WCHAR *szIoTimerStatusStop[] = {
	L"Stop",
	L"已停止"
};

WCHAR *szType[] = {
	L"Type",
	L"类型"
};

WCHAR *szStatus[] = {
	L"Status",
	L"状态"
};

WCHAR *szStartupType[] = {
	L"Startup Type",
	L"启动类型"
};

// WCHAR *szFile[] = {
// 	L"Startup Type",
// 	L"启动类型"
// };

WCHAR *szDisableDirectIo[] = {
	L"Disable Direct IO",
	L"禁止直接IO"
};

WCHAR *szWorkerThreadStatus[] = {
	L"Worker Threads: %d, CriticalWorkQueue: %d, DelayedWorkQueue: %d, HyperCriticalWorkQueue: %d",
	L"工作线程： %d, CriticalWorkQueue： %d, DelayedWorkQueue： %d, HyperCriticalWorkQueue： %d",
};

WCHAR *szRegister[] = {
	L"Register",
	L"寄存器"
};

WCHAR *szRegisterValue[] = {
	L"Value",
	L"寄存器值"
};

WCHAR *szRestore[] = {
	L"Restore",
	L"恢复"
};

WCHAR *szBreakpointLength[] = {
	L"Length",
	L"断点长度"
};

WCHAR *szBreakpointAddress[] = {
	L"Break Address",
	L"断点地址"
};

WCHAR *szBreakOn[] = {
	L"Break Reason",
	L"断点条件"
};

WCHAR *szBreakPointStatus[] = {
	L"Status",
	L"状态"
};

WCHAR *szObject[] = {
	L"Object",
	L"对象"
};

WCHAR *szObjectType[] = {
	L"Object Type",
	L"对象类型"
};

WCHAR *szObjectName[] = {
	L"Object Name",
	L"对象名"
};

WCHAR *szDescription[] = {
	L"Description",
	L"描述"
};

WCHAR *szDevice[] = {
	L"Device",
	L"设备"
};

WCHAR *szAttachDeviceName[] = {
	L"Attach Device Name",
	L"宿主驱动对象名"
};

WCHAR *szDirectIO[] = {
	L"Direct IO",
	L"直接IO"
};

WCHAR *szDirectIOReason[] = {
	L"Reason",
	L"原因"
};

WCHAR *szIOPLIsTrue[] = {
	L"IOPL is true",
	L"进程IOPL被设置成TRUE",
};

WCHAR *szTssIsLargerThanNormal[] = {
	L"Task state segment is larger than normal",
	L"任务状态段的大小异常",
};

WCHAR *szIoMapIsChanged[] = {
	L"IoMap is changed",
	L"进程IO位图被更改",
};

WCHAR *szDirectIOProcessCnt[] = {
	L"Direct IO Processes: %d",
	L"直接IO进程：%d",
};

WCHAR *szDirectIOKillProcess[] = {
	L"Kill",
	L"结束进程"
};

WCHAR *szFilterDriverCnt[] = {
	L"Filter Drivers: %d",
	L"过滤设备：%d",
};

WCHAR *szObjectHijackStatus[] = {
	L"Object Hijack: %d",
	L"对象劫持：%d"
};

WCHAR *szIndex[] = {
	L"Index",
	L"索引"
};

WCHAR *szName[] = {
	L"Name",
	L"名称"
};

WCHAR *szDispalyName[] = {
	L"Dispaly Name",
	L"显示名"
};

WCHAR *szKey[] = {
	L"Key",
	L"项"
};

WCHAR *szFile[] = {
	L"File",
	L"文件"
};

WCHAR *szData[] = {
	L"Data",
	L"数据"
};

WCHAR *szValue[] = {
	L"Value",
	L"值"
};

WCHAR *szRegAddress[] = {
	L"Quick Position",
	L"快速定位"
};

WCHAR *szFunction[] = {
	L"Function",
	L"函数名"
};

WCHAR *szFunctionAddress[] = {
	L"Function",
	L"函数"
};

WCHAR *szDefault[] = {
	L"(Default)",
	L"(默认)"
};

WCHAR *szValueNoSet[] = {
	L"(Value No Set)",
	L"(数值未设置)"
};

WCHAR *szCurrentEntry[] = {
	L"Current Entry",
	L"当前函数地址"
};

WCHAR *szHook[] = {
	L"Hook",
	L"Hook"
};

WCHAR *szHookType[] = {
	L"Hook Type",
	L"钩子类型"
};

WCHAR *szOriginalEntry[] = {
	L"Original Entry",
	L"原始函数地址"
};

WCHAR *szModule[] = {
	L"Module",
	L"当前函数地址所在模块"
};

WCHAR *szServiceModule[] = {
	L"Service Module",
	L"服务模块"
};

WCHAR *szStarted[] = {
	L"Started",
	L"已启动"
};

WCHAR *szSSdtStatus[] = {
	L"SSDT Entry: %d, Hooks: %d",
	L"SSDT函数：%d，被挂钩函数：%d"
};

WCHAR *szFindProcessByWindwo[] = {
	L"Find Process By Window",
	L"查找窗口定位进程"
};

WCHAR *szShadowSSdtStatus[] = {
	L"Shadow SSDT Entry: %d, Hooks: %d",
	L"Shadow SSDT函数：%d，被挂钩函数：%d"
};

WCHAR *szOnlyShowHooks[] = {
	L"Only Show Hooks",
	L"仅显示挂钩函数"
};

WCHAR *szDisassemblerCurrentEntry[] = {
	L"Disassemble CurrentEntry",
	L"反汇编当前函数地址"
};

WCHAR *szDisassemblerHookAddress[] = {
	L"Disassemble HookAddress",
	L"反汇编挂钩地址"
};

WCHAR *szDisassemblerOrginalEntry[] = {
	L"Disassemble OrginalEntry",
	L"反汇编原始函数地址"
};

WCHAR *szRestoreAll[] = {
	L"Restore All",
	L"恢复全部"
};

WCHAR *szDisassembleBreakpoint[] = {
	L"Disassemble Breakpoint",
	L"反汇编断点地址"
};

WCHAR *szFsdStatus[] = {
	L"FSD Entry: %d, Hooks: %d",
	L"FSD派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szMouStatus[] = {
	L"MOU Entry: %d, Hooks: %d",
	L"MOU派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szIatEatStatus[] = {
	L"IAT/EAT Hooks: %d",
	L"IAT/EAT钩子：%d"
};

WCHAR *szDiskStatus[] = {
	L"Disk Entry: %d, Hooks: %d",
	L"Disk派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szAtapiStatus[] = {
	L"Atapi Entry: %d, Hooks: %d",
	L"Atapi派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szAcpiStatus[] = {
	L"Acpi Entry: %d, Hooks: %d",
	L"Acpi派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szKdbStatus[] = {
	L"KBD Entry: %d, Hooks: %d",
	L"KBD派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szTcpipStatus[] = {
	L"Tcpip Entry: %d, Hooks: %d",
	L"Tcpip派遣函数：%d，被挂钩函数：%d"
};

WCHAR *szIdtStatus[] = {
	L"IDT Entry: %d, Hooks: %d",
	L"中断表函数：%d，被挂钩函数：%d"
};

WCHAR *szKernelEntryStatus[] = {
	L"Kernel Entry Hook: %d",
	L"内核入口钩子：%d"
};

WCHAR *szObjectHookStatus[] = {
	L"Object Type Functions: %d, Hooks: %d",
	L"对象类型函数：%d，被挂钩函数：%d"
};

WCHAR *szMessageHook[] = {
	L"Message Hook",
	L"消息钩子"
};

WCHAR *szProcessHook[] = {
	L"Process Hook",
	L"进程钩子"
};

WCHAR *szKernelCallbackTable[] = {
	L"KernelCallbackTable",
	L"内核回调表"
};

WCHAR *szProcessModuleHijack[] = {
	L"Module Hijack",
	L"模块劫持"
};

WCHAR *szObjectAddress[] = {
	L"Object Address",
	L"对象地址"
};

WCHAR *szHookAddress[] = {
	L"Hook Address",
	L"挂钩位置"
};

WCHAR *szCurrentValue[] = {
	L"Current Value",
	L"挂钩处当前值"
};

WCHAR *szPatchLenInBytes[] = {
	L"Length(Bytes)",
	L"长度(字节)"
};

WCHAR *szOriginalValue[] = {
	L"Original Value",
	L"挂钩处原始值"
};

WCHAR *szKernelEntry[] = {
	L"Kernel Entry",
	L"内核入口"
};

WCHAR *szIatEat[] = {
	L"IAT/EAT",
	L"IAT/EAT",
};

WCHAR *szScanning[] = {
	L"Scanning...",
	L"正在扫描...",
};

WCHAR *szPatchHooks[] = {
	L"Hooks/Patches: %d",
	L"钩子/代码修改： %d",
};

WCHAR *szModifyedCode[] = {
	L"Modified Code",
	L"代码修改",
};

WCHAR *szProcessPath[] = {
	L"Process Path",
	L"进程路径"
};

WCHAR *szHandle[] = {
	L"Handle",
	L"句柄"
};

WCHAR *szProcesse[] = {
	L"Process",
	L"进程名"
};

WCHAR *szRunProcessAsSuspend[] = {
	L"Run Process As Suspend",
	L"以挂起方式启动进程"
};

WCHAR *szAutoStart[] = {
	L"Automatic",
	L"自动"
};

WCHAR *szBoot[] = {
	L"Boot Start",
	L"引导启动"
};

WCHAR *szSystemType[] = {
	L"System Start",
	L"系统启动"
};

WCHAR *szDisabled[] = {
	L"Disabled",
	L"已禁用"
};

WCHAR *szManual[] = {
	L"Manual",
	L"手动"
};

WCHAR *szSystemStart[] = {
	L"System Start",
	L"系统启动"
};

WCHAR *szBootStart[] = {
	L"Boot Start",
	L"引导启动"
};

WCHAR *szServiceCnt[] = {
	L"Service: ",
	L"服务： "
};

WCHAR *szServiceStart[] = {
	L"Start",
	L"启动"
};

WCHAR *szServiceStop[] = {
	L"Stop",
	L"停止"
};

WCHAR *szServiceRestart[] = {
	L"Restart",
	L"重新启动"
};

WCHAR *szServiceAutomatic[] = {
	L"Auto Start",
	L"自动启动"
};

WCHAR *szServiceManual[] = {
	L"Manual Start",
	L"手动启动"
};

WCHAR *szServiceDisabled[] = {
	L"Disabled",
	L"禁止启动"
};

WCHAR *szServiceDeleteRegistry[] = {
	L"Delete Registry",
	L"删除注册表"
};

WCHAR *szServiceDeleteRegistryAndFile[] = {
	L"Delete Registry And File",
	L"删除注册表和服务模块"
};

WCHAR *szKnowDlls[] = {
	L"Know DLLs",
	L"公关动态链接库"
};

WCHAR *szPrintMonitors[] = {
	L"Print Monitors",
	L"打印监视程序"
};

WCHAR *szNetworkProviders[] = {
	L"Network Providers",
	L"网络供应者"
};

WCHAR *szWinsockProviders[] = {
	L"Winsock Providers",
	L"Winsock供应者"
};

WCHAR *szSecurityProviders[] = {
	L"Security Providers",
	L"安全供应者"
};

WCHAR *szScheduledTasks[] = {
	L"Scheduled Tasks",
	L"计划任务"
};

WCHAR *szJmpToReg[] = {
	L"Jump to Registry",
	L"跳转到注册表"
};

WCHAR *szDeleteStartup[] = {
	L"Delete Startup",
	L"删除启动信息"
};

WCHAR *szEnableStartup[] = {
	L"Enable",
	L"开启自启动"
};

WCHAR *szDisableStartup[] = {
	L"Disable",
	L"禁止自启动"
};

WCHAR *szExportStartup[] = {
	L"Export Startup",
	L"导出启动信息"
};

WCHAR *szDeleteStartupAndFile[] = {
	L"Delete Startup And File",
	L"删除启动信息和文件"
};

WCHAR *szCopyStartupName[] = {
	L"Copy Startup Name",
	L"拷贝启动项名"
};

WCHAR *szCopyStartupPath[] = {
	L"Copy Startup Path",
	L"拷贝启动项路径"
};

WCHAR *szStartupSearchOnline[] = {
	L"Search Online",
	L"在线搜索启动项"
};

WCHAR *szAreYouSureUnloadDriver[] = {
	L"Are you sure you want to unload the driver?",
	L"卸载驱动有蓝屏风险，确认要卸载？"
};

WCHAR *szUnloadDriver[] = {
	L"Unload Driver",
	L"卸载驱动"
};

WCHAR *szDisassmSize[] = {
	L"Size(Bytes):",
	L"大小(字节)："
};

WCHAR *szDisassmPID[] = {
	L"PID:",
	L"PID："
};

WCHAR *szDisassembler[] = {
	L"Disassembler",
	L"反汇编器"
};

WCHAR *szFollowImediate[] = {
	L"Follow Imediate",
	L"跟随立即数"
};

WCHAR *szCopy[] = {
	L"Copy...",
	L"复制..."
};

WCHAR *szCopyFullLine[] = {
	L"Full row",
	L"整行"
};

WCHAR *szCopyAddress[] = {
	L"Address",
	L"地址"
};

WCHAR *szCopyHex[] = {
	L"Hex",
	L"16进制"
};

WCHAR *szCopyCode[] = {
	L"Assembly code",
	L"汇编代码"
};

WCHAR *szLowerCase[] = {
	L"LowerCase",
	L"小写字体"
};

WCHAR *szDisassemble[] = {
	L"Disassembler",
	L"反汇编当前地址"
};

WCHAR *szDisassembleCallback[] = {
	L"Disassemble Callback",
	L"反汇编回调函数"
};

WCHAR *szDisassembleEntry[] = {
	L"Disassemble Entry",
	L"反汇编入口"
};

WCHAR *szCannotFindFile[] = {
	L"Cannot find the file!",
	L"文件不存在!"
};

WCHAR *szHardwareBreakpoint[] = {
	L"Hardware breakpoint: %d",
	L"硬件断点：%d"
};

WCHAR *szMessageHookStatus[] = {
	L"Message Hook: %d, Global Message Hook: %d",
	L"消息钩子：%d，全局消息钩子：%d"
};

WCHAR *szOnlyShowGlobalHook[] = {
	L"Only Show Global Hook",
	L"仅显示全局钩子"
};

WCHAR *szUnhook[] = {
	L"Unhook",
	L"卸载钩子"
};

WCHAR *szDisassembleMessageHook[] = {
	L"Disassemble Entry",
	L"反汇编钩子入口"
};

// WCHAR *szCheckForUpdate[] = {
// 	L"Checking for updates now, please wait for a moment...\r\nConntecting to the server.",
// 	L"正在检查更新，请稍后...\r\n正在连接服务器。"
// };

WCHAR *szCheckForUpdate[] = {
	L"Please wait while your request is being processed. This could take a few seconds.\r\nConntecting to the server.",
	L"正在处理更新请求，这可能需要几秒钟时间，请稍后。\r\n正在连接服务器..."
};

WCHAR *szUpdate[] = {
	L"Check for updates",
	L"检查更新"
};

WCHAR *szConnectedOK[] = {
	L"Connected OK.\r\nWaiting for server response...",
	L"连接服务器成功。\r\n正在等待服务器响应..."
};

// WCHAR *szUpdateFailed[] = {
// 	L"Update failed, please try again later.",
// 	L"更新失败，请稍后再试。"
// };

WCHAR *szCurrentVersion[] = {
	L"Current Version: ",
	L"当前版本："
};

WCHAR *szLatestVersion[] = {
	L"Latest Version: ",
	L"最新版本："
};

WCHAR *szUpdateLog[] = {
	L"Update Log:",
	L"更新日志："
};

WCHAR *szAlreadyHaveRecentVersion[] = {
	L"You already have the most recent version of AntiSpy.",
	L"AntiSpy已经是最新版本，无需更新。"
};

WCHAR *szUpdateError[] = {
	L"Check for updates error, please try again later.",
	L"检查更新出错，请稍后再试。"
};

WCHAR *szDownloadError[] = {
	L"Download the file error, please try again later.",
	L"下载文件出错，请稍后再试。"
};

WCHAR *szUpdateFailed[] = {
	L"Update failed, please try again later.",
	L"更新文件失败，请稍后再试。"
};

WCHAR *szUpdateSuccess[] = {
	L"The update was successful, enjoy it.",
	L"更新成功。"
};

WCHAR *szUpdateNow[] = {
	L"Update Now",
	L"立即更新"
};

WCHAR *szGotoNewVersion[] = {
	L"Goto New Version",
	L"定位到新版本"
};

WCHAR *szCheckForUpdates[] = {
	L"Check for updates (the current version is %s)",
	L"检测新版本 (当前是%s版本)"
};

WCHAR *szContactAuthor[] = {
	L"Contact Author",
	L"联系作者"
};

WCHAR *szAuthor[] = {
	L"Author: mzf",
	L"作者：mzf (莫灰灰)"
};

WCHAR *szEmail[] = {
	L"Email: AntiSpy@163.com",
	L"Email：AntiSpy@163.com"
};

WCHAR *szWebsite[] = {
	L"Blog: www.KingProtect.com",
	L"博客：www.KingProtect.com"
};

WCHAR *szQQ[] = {
	L"QQ: 454858525",
	L"QQ  ：454858525， QQ群：17385388"
};

WCHAR *szAntiSpy[] = {
	L"Website: www.AntiSpy.cn",
	L"官网：www.AntiSpy.cn"
};


WCHAR *szSina[] = {
	L"MicroBlog: http://weibo.com/minzhenfei (Sina)",
	L"微博：http://weibo.com/minzhenfei (新浪)"
};

WCHAR *szTencent[] = {
	L"     http://weibo.com/minzhenfei (Tencent)",
	L"http://t.qq.com/minzhenfei (腾讯)"
};

WCHAR *szInformationAndCopyright[] = {
	L"Information && Copyright",
	L"说明与致谢"
};

WCHAR *szInformation[] = {
	L"Description: AntiSpy is a handy anti-virus and rootkit tools. It can detect,analyze and restore various kernel modifications, just like ssdt hooks. With its assistance, you can easily spot and remove malwares in your system.",
	L"说        明：这是一款手工杀毒辅助工具，在她的帮助下您可以轻而易举的揪出电脑中隐藏至深的病毒和木马，还您一个干净舒心的上网环境。"
};

WCHAR *szSystem[] = {
	L"",
	L"适用系统：Win2000，xp，Win2003，Vista，Win2008，Win7，Win8 (32位)"
};

WCHAR *szThanks[] = {
	L"Platform: Win2000, xp, Win2003, Vista, Win2008, Win7, Win8 (32bit)",
	L"致        谢：在这款工具的写作过程中，参考了多位技术大牛的文章，在此一并表示感谢。另外非常感谢hxj同学给本工具设计的软件图标。"
};

WCHAR *szCopyright[] = {
	L"",
	L"版        权：(C) 2011 - 2013 mzf. 版权所有."
};

WCHAR *szMianze[] = {
	L"Copyright: (C) 2011 - 2013 mzf. All Rights Reserved.",
	L"免责申明：由于本工具的大部分功能都在内核中完成，因此存在一定的不稳定性。如果您在使用本工具的过程中，给您造成直接或者间接的损失，本人概不负责。从您使用本工具的一刻起，将视为您已经接受了本免职声明。"
};

WCHAR *szUpdates[] = {
	L"Update",
	L"更新"
};

WCHAR *szUnsupportedOS[] = {
	L"Unfortunately this OS is totally unsupported, please email me your OS version, I will support it in improved version, thanks.",
	L"暂不支持的操作系统，请发邮件告知您的操作系统版本号，我将在后续版本中改进，谢谢您的支持。"
};

WCHAR *szLoadDirverError[] = {
	L"Failed to load the diver.",
	L"加载驱动失败！"
};

WCHAR *szCannotSupportX64[] = {
	L"The current version does not support 64 bit Windows.",
	L"当前版本暂不支持64位操作系统！"
};

WCHAR *szShowAllProcessInfo[] = {
	L"View Detail Info (DClick)",
	L"查看进程详细信息 (DClick)"
};

WCHAR *szShowModulesBelow[] = {
	L"Show Lower Modules Pane",
	L"在下方显示模块窗口"
};

WCHAR *szModulesBelow[] = {
	L", Modules(%s): %d/%d",
	L"， 模块(%s)： %d/%d"
};

WCHAR *szAreYOUSureKillProcess[] = {
	L"Are you sure you want to kill %s?",
	L"您确定要结束进程 %s 吗？"
};

WCHAR *szAreYOUSureKillTheseProcess[] = {
	L"Are you sure you want to kill these processes?",
	L"您确定要结束这些进程吗？"
};

WCHAR *szAreYOUSureKillAndDeleteProcess[] = {
	L"Are you sure you want to kill and delete %s?",
	L"您确定要结束进程 %s 并删除相关文件吗？"
};

WCHAR *szAreYOUSureKillAndDeleteProcesses[] = {
	L"Are you sure you want to continue?",
	L"您确定要结束这些进程并删除相关文件吗？"
};

WCHAR *szInitializing[] = {
	L"Initializing...",
	L"正在初始化..."
};

WCHAR *szScanDonePatchHooks[] = {
	L"Scan done. Hooks/Patches: %d",
	L"扫描完毕，钩子/代码修改： %d",
};

WCHAR *szCreateDump[] = {
	L"Create Debug Dump...",
	L"创建调试Dump...",
};

WCHAR *szCreateDumpOK[] = {
	L"Create Dump OK!",
	L"创建Dump成功！",
};

WCHAR *szCreateDumpFailed[] = {
	L"Failed to create dump!",
	L"创建Dump失败！",
};

WCHAR *szProcessImage[] = {
	L"Image",
	L"映像",
};

WCHAR *szNonExistentProcess[] = {
	L"Non-existent process",
	L"该进程已退出",
};

WCHAR *szImageFile[] = {
	L"Image File",
	L"映像文件",
};

WCHAR *szFileSize[] = {
	L"Size:",
	L"文件大小：",
};

WCHAR *szFileDescription[] = {
	L"Description:",
	L"文件描述：",
};

WCHAR *szFileCompany[] = {
	L"Corporation:",
	L"文件厂商：",
};

WCHAR *szFilePath[] = {
	L"Path:",
	L"文件路径：",
};

WCHAR *szFileTime[] = {
	L"Time:",
	L"时        间：",
};

WCHAR *szFileVersion[] = {
	L"Version:",
	L"文件版本：",
};

WCHAR *szCommandLine[] = {
	L"Command Line:",
	L"命令行：",
};

WCHAR *szCurrentDirectory[] = {
	L"Current Directory:",
	L"当前目录：",
};

WCHAR *szStartTime[] = {
	L"Started:",
	L"运行时间：",
};

WCHAR *szPEB[] = {
	L"PEB:",
	L"PEB：",
};

WCHAR *szParent[] = {
	L"Parent:",
	L"父进程：",
};

WCHAR *szFileFind[] = {
	L"Find",
	L"定位",
};

WCHAR *szFileDelete[] = {
	L"Delete",
	L"删除",
};

WCHAR *szScanHooks[] = {
	L"Scan Hooks",
	L"扫描进程钩子"
};

WCHAR *szAlreadyRunning[] = {
	L"AntiSpy is already running.",
	L"AntiSpy进程已经开启！"
};

WCHAR *szKillProcessTree[] = {
	L"Kill Process Tree",
	L"结束进程树"
};

WCHAR *szAreYouSureKillDescendants[] = {
	L"Are you sure you want to kill %s and its descendants?",
	L"确定要结束进程 %s 以及它的所有子进程吗？"
};

WCHAR *szTheLatestVersion[] = {
	L"You already have the latest version of AntiSpy.",
	L"您的 AntiSpy 已经是最新版本。"
};

WCHAR *szEditBinaryValue[] = {
	L"Edit Binary Value",
	L"编辑二进制数值"
};

WCHAR *szHideMicrosofeModules[] = {
	L"Hide Microsoft's Modules",
	L"隐藏微软模块"
};

WCHAR *szInjectDllModuleOk[] = {
	L"Inject dll module ok!",
	L"注入模块成功！"
};

WCHAR *szInjectDllModuleFailed[] = {
	L"Inject dll module failed!",
	L"注入模块失败！"
};

WCHAR *szGeneralConfiguration[] = {
// 	L"General configuration,the changes will not take effect until the next time it is started.",
// 	L"General configurations, it will take effect when the application restarted.",
// 	L"常规配置选项，软件重启后生效"
	L"General",
	L"常规"
};

WCHAR *szProcessListConfiguration[] = {
// 	L"Process tab configurations, it will take effect when the application restarted.",
// 	L"进程标签配置选项，软件重启后生效"
	L"Process",
	L"进程"
};

WCHAR *szTitleName[] = {
	L"Custom title name:",
	L"自定义窗口标题："
};

WCHAR *szRandomTitle[] = {
	L"Random title name",
	L"随机标题名"
};

WCHAR *szStayOnTop[] = {
	L"Stay on top",
	L"窗口置顶"
};

WCHAR *szAutoDetectNewVersion[] = {
	L"Auto-detect new version",
	L"软件启动时自动检测新版本"
};

WCHAR *szFindANewVersion[] = {
	L"Find a new version of AntiSpy, are you want to update?",
	L"发现新版本，是否立刻升级？"
};

WCHAR *szHotKeyForMainWindow[] = {
	L"Show main window: \tCtrl + Alt + Shift + ",
	L"显示软件主窗口：\tCtrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessModulesDlg[] = {
	L"Show process modules:\tCtrl + Alt + Shift + ",
	L"显示进程模块对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessThreadsDlg[] = {
	L"Show process threas:\tCtrl + Alt + Shift + ",
	L"显示进程线程对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessHandlesDlg[] = {
	L"Show process handles:\tCtrl + Alt + Shift + ",
	L"显示进程句柄对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessWndDlg[] = {
	L"Show process windows:\tCtrl + Alt + Shift + ",
	L"显示进程窗口对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessMemoryDlg[] = {
	L"Show process memory:\tCtrl + Alt + Shift + ",
	L"显示进程内存对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessHotkeysDlg[] = {
	L"Show process hotkeys:\tCtrl + Alt + Shift + ",
	L"显示进程热键对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessPrivilegeDlg[] = {
	L"Show process privileges:\tCtrl + Alt + Shift + ",
	L"显示进程权限对话框：Ctrl + Alt + Shift + "
};

WCHAR *szHotKeyForProcessTimersDlg[] = {
	L"Show process timers:\tCtrl + Alt + Shift + ",
	L"显示定时器对话框：\tCtrl + Alt + Shift + "
};

WCHAR *szShowProcessDetailInformations[] = {
	L"Automatic display process detail information tips",
	L"当鼠标移动到进程列表上时自动显示进程详细信息"
};

WCHAR *szHideMicrosoftService[] = {
	L"Hide Microsoft's Modules",
	L"隐藏微软服务"
};

WCHAR *szSearchData[] = {
	L"Data",
	L"数据"
};

WCHAR *szSearchValues[] = {
	L"Values",
	L"值"
};

WCHAR *szSearchKeys[] = {
	L"Keys",
	L"项"
};

WCHAR *szSearchMachCase[] = {
	L"Mach case",
	L"区分大小写"
};

WCHAR *szSearchMachWholeString[] = {
	L"Mach whole string",
	L"匹配整个字符串"
};

WCHAR *szFindWhat[] = {
	L"Find what:",
	L"查找内容："
};

WCHAR *szWhereToSearch[] = {
	L"Where to search",
	L"搜索范围"
};

WCHAR *szSearchInKey[] = {
	L"Search in key:",
	L"在此项中搜索："
};

WCHAR *szSearchTheEntireRegistry[] = {
	L"Search the entire registry",
	L"搜索整个注册表"
};

WCHAR *szRegistryFind[] = {
	L"Find",
	L"查找"
};

WCHAR *szRegistryStop[] = {
	L"Stop",
	L"停止"
};

WCHAR *szRegistryPleaseInputFindWhat[] = {
	L"Please enter the content that you're searching for.",
	L"请输入您要查找的内容。"
};

WCHAR *szRegistryPleaseInputWhatTheKeyToFind[] = {
	L"Please enter the key path that you're searching for.",
	L"请输入您要在其中查找的项路径。"
};

WCHAR *szCanNotOpenKey[] = {
	L"Cannot open the key.",
	L"无法打开该键。"
};

WCHAR *szRegistrySearchNow[] = {
	L"Searching now, please wait for a moment...",
	L"正在搜索，请稍后..."
};

WCHAR *szFindResults[] = {
	L"Find results - found %d matches",
	L"查找结果 - 找到 %d 个匹配的项目"
};

WCHAR *szFindResult[] = {
	L"Find results",
	L"查找结果"
};

WCHAR *szMemoryHexEdit[] = {
	L"Hex Editer",
	L"内存编辑器"
};

WCHAR *szReadOnly[] = {
	L"Read Only",
	L"只读"
};

WCHAR *szKernelAddress[] = {
	L"Kernel Mode Address (Address > 0x80000000)",
	L"内核模式地址 (地址 > 0x80000000)"
};

WCHAR *szDeafultShowLowPaner[] = {
	L"Show lower pane as default",
	L"默认显示下方模块列表"
};

WCHAR *szDeafultShowProcessTree[] = {
	L"Show process tree as default",
	L"默认显示进程树"
};

WCHAR *szDeleteFileSucess[] = {
	L"Delete file success.",
	L"删除文件成功。"
};

WCHAR *szDeleteFileFailed[] = {
	L"Delete file failed.",
	L"删除文件失败。"
};

WCHAR *szJumpToService[] = {
	L"Jump to Service",
	L"跳转到相应服务"
};

WCHAR *szLocalDisk[] = {
	L"Local Disk",
	L"本地磁盘"
};

WCHAR *szCDDrive[] = {
	L"CD Drive",
	L"光驱"
};

WCHAR *szRemovableDisk[] = {
	L"Removable Disk",
	L"可移动磁盘"
};

WCHAR *szDiskUnknow[] = {
	L"Unknow",
	L"未知盘"
};

WCHAR *szRemoteDisk[] = {
	L"Remote Disk",
	L"远程磁盘"
};

WCHAR *szRamDisk[] = {
	L"Ram Disk",
	L"内存盘"
};

WCHAR *szFileName[] = {
	L"File Name",
	L"名称"
};

WCHAR *szCreationTime[] = {
	L"Creation Time",
	L"创建日期"
};

WCHAR *szModificationTime[] = {
	L"Modification Time",
	L"修改日期"
};

WCHAR *szFileSizeEx[] = {
	L"File Size",
	L"大小"
};

WCHAR *szFileAttributes[] = {
	L"Attributes",
	L"属性"
};

WCHAR *szFileType[] = {
	L"Type",
	L"类型"
};

WCHAR *szFileDirectory[] = {
	L"Directory",
	L"文件夹"
};

WCHAR *szFileReadOnly[] = {
	L"ReadOnly",
	L"只读"
};

WCHAR *szFileHIDDEN[] = {
	L"Hidden",
	L"隐藏"
};

WCHAR *szFileSYSTEM[] = {
	L"System",
	L"系统"
};

WCHAR *szFileDIRECTORY[] = {
	L"Directory",
	L"文件夹"
};

WCHAR *szFileARCHIVE[] = {
	L"Archive",
	L"档案"
};

WCHAR *szFileNORMAL[] = {
	L"Normal",
	L"普通"
};

WCHAR *szFileTEMPORARY[] = {
	L"Temporary",
	L"临时"
};

WCHAR *szFileCOMPRESSED[] = {
	L"Compressed",
	L"压缩"
};

WCHAR *szFileOpen[] = {
	L"Open",
	L"打开"
};

WCHAR *szFileRefresh[] = {
	L"Refresh",
	L"刷新"
};

WCHAR *szFileLookForLockInfo[] = {
	L"View File Lock Info",
	L"查看锁定情况"
};

WCHAR *szKeyLookForLockInfo[] = {
	L"View Key Lock Info",
	L"查看锁定情况"
};

WCHAR *szFileDeleteAndDenyRebuild[] = {
	L"Delete Aand Deny Rebuild",
	L"删除并阻止文件再生"
};

WCHAR *szFileAddToRestartDelete[] = {
	L"Add To Delay Delete List",
	L"添加到重启删除列表"
};

WCHAR *szFileRestartAndReplace[] = {
	L"Add To Delay Replace List",
	L"重启替换为..."
};

WCHAR *szFileCopyTo[] = {
	L"Copy to...",
	L"拷贝到..."
};

WCHAR *szFileRename[] = {
	L"Rename",
	L"重命名"
};

WCHAR *szFileCopyFileName[] = {
	L"Copy File Name",
	L"拷贝文件名"
};

WCHAR *szFileCopyFilePath[] = {
	L"Copy File Path",
	L"拷贝文件路径"
};

WCHAR *szFileProperties[] = {
	L"Properties",
	L"查看文件属性"
};

WCHAR *szFileFindInExplorer[] = {
	L"Find Target In Explorer",
	L"在资源管理器中定位文件"
};

WCHAR *szFileVerifySignature[] = {
	L"Verify Signature",
	L"校验文件签名"
};

WCHAR *szFileVerifyAllSignature[] = {
	L"Verify Aall Signature",
	L"校验所有文件签名"
};

WCHAR *szFileDeleteHideSystemProperties[] = {
	L"Set Normal Attributes",
	L"去掉只读、隐藏、系统属性"
};

WCHAR *szFileUnlocker[] = {
	L"File Unlocker",
	L"文件解锁器"
};

WCHAR *szRegUnlocker[] = {
	L"Registry Unlocker",
	L"注册表解锁器"
};

WCHAR *szFileLockedProcess[] = {
	L"Process Name",
	L"进程映像"
};

WCHAR *szFileLockedProcessPid[] = {
	L"Pid",
	L"进程Id"
};

WCHAR *szFileLockedHandle[] = {
	L"Handle",
	L"句柄"
};

WCHAR *szFileLockedFilePath[] = {
	L"Locked File Path",
	L"被锁定文件路径"
};

WCHAR *szLockedRegistryPath[] = {
	L"Locked Registry Path",
	L"被锁定的注册表路径"
};

WCHAR *szFileUnlocked[] = {
	L"File(Directory) not be locked.",
	L"文件(文件夹)未被锁定。"
};

WCHAR *szRegUnlocked[] = {
	L"The key is not be locked.",
	L"当前注册表键值未被锁定。"
};

WCHAR *szFileUnlock[] = {
	L"Unlock",
	L"解锁"
};

WCHAR *szFileUnlockAll[] = {
	L"Unlock All",
	L"解锁全部"
};

WCHAR *szFileKillLockProcess[] = {
	L"Kill Lock Process",
	L"结束锁定进程"
};

WCHAR *szFileFindProcess[] = {
	L"Find Process In Explorer",
	L"定位到锁定进程"
};

WCHAR *szFileFindFile[] = {
	L"Find File In Explorer",
	L"定位到被锁定文件"
};

WCHAR *szFileRestartDeleteType[] = {
	L"Operation Type",
	L"操作类型"
};

WCHAR *szFileRestartFilePath[] = {
	L"File Path",
	L"文件路径"
};

WCHAR *szFileRestartNewFilePath[] = {
	L"New File Path",
	L"新文件路径"
};

WCHAR *szFileRestartDeleteInfo[] = {
	L"File Delay Operation Information",
	L"文件延迟操作信息"
};

WCHAR *szFileViewRestartDeleteInfo[] = {
	L"View Delay Operation Info",
	L"查看文件延迟操作信息"
};

WCHAR *szFilePendingDelayDelete[] = {
	L"Delay Delete",
	L"重启删除"
};

WCHAR *szFilePendingDelayReplace[] = {
	L"Delay Replace",
	L"重启替换"
};

WCHAR *szFilePendingStatus[] = {
	L"Delay Delete: %d, Delay Replace: %d",
	L"重启删除：%d， 重启替换：%d"
};

WCHAR *szFilePendingDeleteCurrent[] = {
	L"Delete Current Operation",
	L"删除当前延迟操作信息"
};

WCHAR *szFilePendingDeleteAll[] = {
	L"Delete All Operation",
	L"删除所有延迟操作信息"
};

WCHAR *szFilePendingFindInExplorer[] = {
	L"Find File In Explorer",
	L"定位到文件"
};

WCHAR *szFilePendingProperties[] = {
	L"File Properties",
	L"查看文件属性"
};

WCHAR *szFilePendingFindNewInExplorer[] = {
	L"Find New File In Explorer",
	L"定位到新文件"
};

WCHAR *szFilePendingNewFileProperties[] = {
	L"New File Properties",
	L"查看新文件属性"
};

WCHAR *szFileSetFileAttribute[] = {
	L"Set Hidden Attributes",
	L"设置隐藏、系统属性"
};

WCHAR *szInputFilePath[] = {
	L"Input File Path",
	L"输入文件路径"
};

WCHAR *szStaticInputFilePath[] = {
	L"File Path:",
	L"文件路径："
};

WCHAR *szFileBrowse[] = {
	L"Browse",
	L"浏览"
};

WCHAR *szInputFileName[] = {
	L"Input New File Name",
	L"输入新文件名"
};

WCHAR *szStaticInputFileName[] = {
	L"New File Name:",
	L"新文件名："
};

WCHAR *szFileDescriptionNew[] = {
	L"File Description",
	L"文件描述"
};

WCHAR *szProcessStartedTime[] = {
	L"Started Time",
	L"启动时间"
};

WCHAR *szFileCreationTime[] = {
	L"Creation Time",
	L"文件创建时间"
};

WCHAR *szFileHasExist[] = {
	L"The file has exist, please input other name.",
	L"此文件名已经存在，请输入其他文件名。"
};

WCHAR *szFileCopy[] = {
	L"File Copy",
	L"文件拷贝"
};

WCHAR *szNormalProcess[] = {
	L"Normal process, the corporation is not Micorsoft",
	L"文件厂商不是微软的进程"
};

WCHAR *szHideProcess[] = {
	L"Hidden process",
	L"隐藏进程"
};

WCHAR *szMirosoftProcess[] = {
	L"The corporation of process is Micorsoft",
	L"文件厂商是微软的进程"
};

WCHAR *szMirosoftProcessHaveOtherModule[] = {
	L"Micorsoft's process have other corporation modules",
	L"微软进程含有非微软的模块"
};

WCHAR *szSystemAndHidden[] = {
	L"System && Hidden",
	L"系统和隐藏属性"
};

WCHAR *szNormalFile[] = {
	L"Normal File",
	L"普通属性"
};

WCHAR *szSystemOnly[] = {
	L"System Only",
	L"系统属性"
};

WCHAR *szHiddenOnly[] = {
	L"Hidden Only",
	L"隐藏属性"
};

WCHAR *szColorConfiguration[] = {
// 	L"Color configurations, it will take effect when the application restarted.",
// 	L"颜色配置选项，软件重启后生效"
	L"Colors",
	L"颜色"
};

WCHAR *szHotKeysConfiguration[] = {
	L"HotKeys",
	L"热键"
};

WCHAR *szColorProcess[] = {
	L"Process",
	L"进程"
};

WCHAR *szColorFile[] = {
	L"File",
	L"文件"
};

WCHAR *szColorGeneral[] = {
	L"General",
	L"通用"
};

WCHAR *szColorNormalItem[] = {
	L"Normal module, the corporation is not Micorsoft",
	L"文件厂商不是微软的模块"
};

WCHAR *szColorMicrosoftItem[] = {
	L"The corporation of module is Micorsoft",
	L"文件厂商是微软的模块"
};

WCHAR *szColorHiddenOrHooksItem[] = {
	L"Hidden objects or hooks",
	L"隐藏对象、可疑对象、钩子"
};

WCHAR *szColorNotSignedItem[] = {
	L"No signature module",
	L"没有签名的模块"
};

WCHAR *szDecimal[] = {
	L"Decimal",
	L"十进制"
};

WCHAR *szHexadecimal[] = {
	L"Hexadecimal",
	L"十六进制"
};

WCHAR *szPort[] = {
	L"Port",
	L"网络连接"
};

WCHAR *szHostsFile[] = {
	L"Hosts File",
	L"Hosts文件"
};

WCHAR *szSPI[] = {
	L"LSP",
	L"LSP"
};

WCHAR *szNetwork[] = {
	L"Network",
	L"网络"
};

WCHAR *szPortProcess[] = {
	L"Process",
	L"进程"
};

WCHAR *szPortPID[] = {
	L"PID",
	L"PID"
};

WCHAR *szPortProtocol[] = {
	L"Protocol",
	L"协议"
};

WCHAR *szPortLocalAddress[] = {
	L"Local Address",
	L"本地地址"
};

WCHAR *szPortLocalPort[] = {
	L"Local Port",
	L"本地端口"
};

WCHAR *szPortRemoteAddress[] = {
	L"Remote Address",
	L"远程地址"
};

WCHAR *szPortRemotePort[] = {
	L"Remote Port",
	L"远程端口"
};

WCHAR *szPortState[] = {
	L"State",
	L"状态"
};

WCHAR *szVerified[] = {
	L"(Verified) ",
	L"(已签名) "
};

WCHAR *szNotVerified[] = {
	L"(Not Verified) ",
	L"(未签名) "
};

WCHAR *szTcpUdpStatus[] = {
	L"Tcp: %d, Udp: %d",
	L"Tcp: %d, Udp: %d",
};

WCHAR *szPortCopyFullRow[] = {
	L"Copy Full Row",
	L"复制整行",
};

WCHAR *szPortCopyRemoteAddress[] = {
	L"Copy Remote Address",
	L"复制远程地址",
};

WCHAR *szPortVerifyProcessSignature[] = {
	L"Verify Process Signature",
	L"验证进程数字签名"
};

WCHAR *szPortProcessProperties[] = {
	L"Process Properties",
	L"查看进程文件属性"
};

WCHAR *szPortFindInExplorer[] = {
	L"Find Process File In Explorer",
	L"在资源管理器中定位进程文件"
};

WCHAR *szPortFindInAntiSpy[] = {
	L"Find Process File In AntiSpy",
	L"在AntiSpy文件管理器中定位进程文件"
};

WCHAR *szPortKillProcess[] = {
	L"Kill Process",
	L"结束联网进程"
};

WCHAR *szOpenWithNotepad[] = {
	L"Open With Notepad",
	L"用记事本打开"
};

WCHAR *szHooks[] = {
	L"Hooks",
	L"钩子"
};

WCHAR *szFileSearchOnline[] = {
	L"Search File Name Online",
	L"在线搜索文件名"
};

WCHAR *szFileCalcMd5[] = {
	L"Calc File Hash",
	L"计算文件哈希值"
};

WCHAR *szFileHashFile[] = {
	L"File",
	L"文件"
};

WCHAR *szFileHashCompare[] = {
	L"Compare",
	L"比较"
};

WCHAR *szFileHashDlgTitle[] = {
	L"File Hash Calculator",
	L"文件哈希计算器"
};

WCHAR *szFileHashBtnCompare[] = {
	L"Compare",
	L"比较"
};

WCHAR *szFileHashBtnCalc[] = {
	L"Calc",
	L"计算"
};

WCHAR *szFileHashCalcing[] = {
	L"Calcing...",
	L"计算中..."
};

WCHAR *szIeShell[] = {
	L"IE Shell",
	L"IE右键菜单"
};

WCHAR *szIeShellStatus[] = {
	L"IE Shell: %d",
	L"IE右键菜单项：%d"
};

WCHAR *szRclickDeleteReg[] = {
	L"Delete Reg",
	L"删除注册表"
};

WCHAR *szIePlugin[] = {
	L"IE Plugin",
	L"IE插件"
};

WCHAR *szIePluginStatus[] = {
	L"Total: %d, BHO: %d, IE Shell: %d, URL Search Hooks: %d, Distribution Units: %d, Toolbar: %d, ActiveX: %d",
	L"总共：%d，BHO：%d，浏览器扩展：%d，URL查询钩子：%d，Distribution Units：%d，工具条：%d，ActiveX：%d"
};

WCHAR *szIEPluginDeleteReg[] = {
	L"Delete Reg",
	L"删除注册表"
};

WCHAR *szIEPluginDeleteFileAndReg[] = {
	L"Delete Reg And File",
	L"删除注册表和文件"
};

WCHAR *szEnable[] = {
	L"Enable",
	L"启用"
};

WCHAR *szDisable[] = {
	L"Disable",
	L"禁用"
};

WCHAR *szIEPluginCopyName[] = {
	L"Copy Name",
	L"复制插件名"
};

WCHAR *szIEPluginCopyPath[] = {
	L"Copy Path",
	L"复制插件路径"
};

WCHAR *szIEPluginSearchOnline[] = {
	L"Search Online",
	L"在线搜索插件名"
};

WCHAR *szIEPluginShuxing[] = {
	L"Properties",
	L"查看插件属性"
};

WCHAR *szIEPluginSign[] = {
	L"Verify File Signature",
	L"验证插件签名"
};

WCHAR *szToolsHexEditor[] = {
	L"Hex Editor",
	L"16进制编辑器"
};

WCHAR *szTools[] = {
	L"Tools",
	L"工具"
};

WCHAR *szToolsDisassembler[] = {
	L"Disassembler",
	L"反汇编器"
};

WCHAR *szAdvancedTools[] = {
	L"Advanced Tools",
	L"高级工具集"
};

WCHAR *szUsers[] = {
	L"Users",
	L"系统用户"
};

WCHAR *szMBR[] = {
	L"MBR",
	L"MBR"
};

WCHAR *szHexEditerPid[] = {
	L"Pid:",
	L"Pid："
};

WCHAR *szHexEditerInputHaveWrong[] = {
	L"Input values maybe have something wrong. ",
	L"输入的值可能有误，请检查您输入的参数"
};

WCHAR *szHexEditerKernelModeAddressWrong[] = {
	L"Kernel mode address must be greater than 0x80000000. ",
	L"内核模式地址必须大于0x80000000"
};

WCHAR *szHexEditerUserModeAddressWrong[] = {
	L"User mode address must be less than 0x80000000. ",
	L"用户模式地址必须小于0x80000000"
};

WCHAR *szHexEditerUserModePidWrong[] = {
	L"User mode address must specify the pid. ",
	L"用户模式地址必须指定相应进程的pid"
};

WCHAR *szHexEditerModifyKernelMode[] = {
	L"Modify kernel mode memory maybe cause your system crash.\r\n\r\nAre you sure you want to continue?",
	L"修改内核模式的内存，可能会引起系统奔溃。\r\n\r\n您确定要继续吗？"
};

WCHAR *szAutorunRefesh[] = {
	L"Refresh All",
	L"全部刷新"
};

WCHAR *szFindDllsOrModulesInputHaveWrong[] = {
	L"Input values can't be empty. ",
	L"要查找的名称不能为空。"
};

WCHAR *szAreYouSureModifyMemory[] = {
	L"Are you sure you want to modify the memory?",
	L"您确认要修改该内存吗？"
};

WCHAR *szModifyMemoryFailed[] = {
	L"Modify Memory Failed!",
	L"修改内存失败！"
};

WCHAR *szModifyMemorySuccess[] = {
	L"Modify Memory OK!",
	L"修改内存成功！"
};

WCHAR *szSearchDoneDlls[] = {
	L"Search done. DLLs: %d",
	L"查找完毕，发现模块： %d",
};

WCHAR *szSearchDoneHandles[] = {
	L"Search done. Handles: %d",
	L"查找完毕，发现句柄： %d",
};

WCHAR *szSearching[] = {
	L"[Scanning...]",
	L"[正在搜索，请稍后...]"
};

WCHAR *szSelfProtection[] = {
	L"Self-Defense",
	L"自我保护"
};

WCHAR *szConfigPS[] = {
	L"Ps: All the settings will take effect when the application restarted.",
	L"Ps：以下任何一项设置需要重启软件才能生效。"
};

WCHAR *szForbidCreatingProcess[] = {
	L"Forbid Creating Process",
	L"禁止创建进程"
};

WCHAR *szForbidCreatingThread[] = {
	L"Forbid Creating Thread",
	L"禁止创建线程"
};

WCHAR *szForbidCreatingFile[] = {
	L"Forbid Creating File",
	L"禁止创建文件"
};

WCHAR *szForbidSwitchingDesktop[] = {
	L"Forbid Switching Desktop",
	L"禁止切换桌面"
};

WCHAR *szForbidCreatingKeyAndValue[] = {
	L"Forbid Creating Key And Value",
	L"禁止创建注册表键和值"
};

WCHAR *szForbidSettingKeyValue[] = {
	L"Forbid Setting Key Value",
	L"禁止设置注册表值"
};

WCHAR *szForbidLoadingImage[] = {
	L"Forbid Loading Image",
	L"禁止加载模块"
};

WCHAR *szForbidGlobalMessageHook[] = {
	L"Forbid Global Message Hook",
	L"禁止全局消息钩子注入"
};

WCHAR *szForbidSystemShutdown[] = {
	L"Forbid System Power Operation",
	L"禁止关机、重启、注销"
};

WCHAR *szForbidModifyingSystemTime[] = {
	L"Forbid Modifying System Time",
	L"禁止修改系统时间"
};

WCHAR *szForbidLockingWorkStation[] = {
	L"Forbid Locking Work Station",
	L"禁止锁定计算机"
};

WCHAR *szForbidLoadingDriver[] = {
	L"Forbid Loading Driver",
	L"禁止加载驱动"
};

WCHAR *szForbid[] = {
	L"Forbid",
	L"禁止"
};

WCHAR *szAreYouSureReboot[] = {
	L"Are you sure you want to reboot the system?",
	L"您确定要重启计算机吗？"
};

WCHAR *szAreYouSureForceReboot[] = {
	L"Are you sure you want to reboot the system?",
	L"您确定要重启计算机吗？"
};

WCHAR *szAreYouSureShutdown[] = {
	L"Are you sure you want to power off the system?",
	L"您确定要关闭计算机吗？"
};

WCHAR *szReboot[] = {
	L"System Reboot",
	L"重启计算机"
};

WCHAR *szForceReboot[] = {
	L"Force System Reboot",
	L"强制重启计算机"
};

WCHAR *szPowerOff[] = {
	L"System Power Off",
	L"关闭计算机"
};

WCHAR *szPower[] = {
	L"Power",
	L"电源"
};

WCHAR *szToolsEnable[] = {
	L"Enable",
	L"解锁"
};

WCHAR *szEnableRegistryTools[] = {
	L"Enable RegEdit",
	L"解锁注册表编辑器"
};

WCHAR *szEnableTaskTools[] = {
	L"Enable Task Manager",
	L"解锁任务管理器"
};

WCHAR *szEnableCmd[] = {
	L"Enable CMD",
	L"解锁命令解释器(CMD.exe)"
};

WCHAR *szEnableControlPane[] = {
	L"Enable Control Pane",
	L"解锁控制面板"
};

WCHAR *szEnableStartMenu[] = {
	L"Enable Taskbar Right Click Menu",
	L"解锁任务栏右键菜单"
};

WCHAR *szClearAllGroupPolicies[] = {
	L"Clear All Group Policies",
	L"解锁组策略"
};

WCHAR *szEnableRun[] = {
	L"Enable Run Feature in Start Menu",
	L"解锁开始菜单中的运行功能"
};

WCHAR *szRepairLSPOK[] = {
	L"Repair LSP OK!",
	L"修复LSP成功！"
};

WCHAR *szRepairSafebootOK[] = {
	L"Repair Safeboot OK!",
	L"修复安全模式成功！"
};

WCHAR *szRepairSafebootFailed[] = {
	L"Repair Safeboot Failed!",
	L"修复安全模式失败！"
};

WCHAR *szRepairLSPFailed[] = {
	L"Repair LSP Failed!",
	L"修复LSP失败！"
};

WCHAR *szAllEnable[] = {
	L"Enabled OK!",
	L"解锁成功！",
};

WCHAR *szPleaseChoseEnable[] = {
	L"Please select the items you want to enable.",
	L"请选择您要解锁的项！",
};

WCHAR *szSafeboot[] = {
	L"Safeboot",
	L"安全模式",
};

WCHAR *szRepair[] = {
	L"Repair",
	L"修复",
};

WCHAR *szRepairAll[] = {
	L"Repair All",
	L"全部修复",
};

WCHAR *szWhatIsMBR[] = {
	L"The MBR, short for the Master Boot Record, it contains the primary boot loader.",
	L"MBR就是Master Boot Record的简称，它是硬盘的主引导记录。",
};

WCHAR *szHowToReapirMBR[] = {
	L"Some Trojans can modify the default MBR, and it can not be killed even you reinstall the system.",
	L"有些木马会修改系统默认的MBR，中了这种MBR木马，即使重装系统也不能完全消灭它。要想彻底解决此类问题，就需要恢复系统原始的MBR。",
};

WCHAR *szMBRnote[] = {
	L"Note: If your system installed the disk encryption or system restore softwares, do not use this tools to restore the default MBR.",
	L"注意：如果您安装了磁盘加密软件或者还原软件，请不要使用本工具，否则可能会造成电脑无法启动或者数据丢失的问题。",
};

WCHAR *szMBRSelectDisk[] = {
	L"Select a physical disk:",
	L"请选择物理磁盘：",
}; 

WCHAR *szMBRIsOk[] = {
	L"The MBR of %s is ok.",
	L"%s的MBR正常。",
}; 

WCHAR *szMBRIsError[] = {
	L"The MBR of %s may be modified.",
	L"%s的MBR可能已经被修改了。",
}; 

WCHAR *szReadAndCheckMBR[] = {
	L"Read And Check MBR",
	L"读取并检测MBR",
}; 

WCHAR *szBackupMBR[] = {
	L"Backup MBR To File",
	L"备份MBR(建议保存到U盘或网盘)",
}; 

WCHAR *szRestoreMBR[] = {
	L"Restore MBR From Backup File",
	L"从备份文件中恢复MBR",
}; 

WCHAR *szRestoreDefaultMBR[] = {
	L"Restore Default MBR",
	L"恢复系统默认MBR",
}; 

WCHAR *szBackupMBROK[] = {
	L"Backup MBR OK!",
	L"备份MBR成功！",
}; 

WCHAR *szBackupMBRERROR[] = {
	L"Backup MBR Failed!",
	L"备份MBR失败！",
}; 

WCHAR *szRestoreMBRNotify[] = {
	L"Restore the MBR has certain risk, are you sure you want to continue?",
	L"恢复MBR有一定的风险，您确认是否要继续？\r\n\r\nPs: 恢复过程中杀毒软件可能会有提示，请选择允许。",
}; 

WCHAR *szMbrFileInvalid[] = {
	L"The MBR backup file is invalid. ",
	L"MBR备份文件无效！",
};

WCHAR *szRestoreMBROK[] = {
	L"Restore MBR OK! ",
	L"恢复MBR成功！",
}; 

WCHAR *szRestoreMBRERROR[] = {
	L"Restore MBR Failed! ",
	L"恢复MBR失败！",
}; 

WCHAR *szDiskPartitionInfoError[] = {
	L"Disk partition information is error,so cannot restore MBR to default!",
	L"磁盘分区信息有误，所以不能恢复到默认MBR。",
}; 

WCHAR *szEanbleRun[] = {
	L"Enable Run Feature in Start Menu",
	L"解锁开始菜单中的运行功能",
}; 

WCHAR *szDonate[] = {
	L"Donate",
	L"捐赠",
}; 

WCHAR *szDonateString[] = {
	L"Donate",
	L"AntiSpy是从我在大学里就开始写的一款工具，\r\n期间断断续续历时2年有余。\r\n",
}; 

WCHAR *szIME[] = {
	L"IME",
	L"IME输入法",
}; 

WCHAR *szFileAssociation[] = {
	L"File Association",
	L"文件关联",
}; 

WCHAR *szIFEO[] = {
	L"Image Hijacks(IFEO)",
	L"映像劫持",
}; 

WCHAR *szRegPath[] = {
	L"Reg Path",
	L"注册表路径",
}; 

WCHAR *szPath[] = {
	L"Path",
	L"路径",
}; 

WCHAR *szIMEPath[] = {
	L"IME File Path",
	L"IME程序路径",
}; 

WCHAR *szIMEFileCorporation[] = {
	L"IME File Corporation",
	L"IME文件厂商",
}; 

WCHAR *szLayoutPath[] = {
	L"Layout File Path",
	L"Layout文件路径",
}; 

WCHAR *szLayoutFileCompany[] = {
	L"Layout File Corporation",
	L"Layout文件厂商",
}; 

WCHAR *szIMEFileProperties[] = {
	L"IME File Properties",
	L"查看IME文件属性",
}; 

WCHAR *szLayoutFileProperties[] = {
	L"Layout File Properties",
	L"查看Layout文件属性",
}; 

WCHAR *szVerifyIMEFileSignature[] = {
	L"Verify IME File Signature",
	L"验证IME文件签名",
}; 

WCHAR *szVerifyLayoutFileSignature[] = {
	L"Verify Layout File Signature",
	L"验证Layout文件签名",
};

WCHAR *szFileIMEFileInExplorer[] = {
	L"Find IME File In Explorer",
	L"在资源管理器中定位IME文件"
};

WCHAR *szFileLayoutFileInExplorer[] = {
	L"Find Layout File In Explorer",
	L"在资源管理器中定位Layout文件"
};

WCHAR *szFileIMEFileInAntiSpy[] = {
	L"Find IME File In AntiSpy",
	L"在AntiSpy文件管理器中定位IME文件"
};

WCHAR *szFileLayoutFileInAntiSpy[] = {
	L"Find Layout File In AntiSpy",
	L"在AntiSpy文件管理器中定位Layout文件"
};

WCHAR *szImeStatus[] = {
	L"IMEs: %d",
	L"IME输入法：%d"
};

WCHAR *szImageHijackName[] = {
	L"Be Hijacked Process Name",
	L"被劫持的进程名"
};

WCHAR *szHijackProcessPath[] = {
	L"Hijack Process Path",
	L"劫持的进程路径"
};

WCHAR *szImageHijacks[] = {
	L"Image Hijacks: %d",
	L"映像劫持数量：%d"
};

WCHAR *szHijackFileProperties[] = {
	L"Hijack File Properties",
	L"查看劫持文件属性"
};

WCHAR *szHijackFileSignature[] = {
	L"Verify Hijack File Signature",
	L"检验劫持文件签名"
};

WCHAR *szHijackFileInExplorer[] = {
	L"Find Hijack File In Explorer",
	L"在资源管理器中定位劫持文件"
};

WCHAR *szHijackFileInAntiSpy[] = {
	L"Find Hijack File In AntiSpy",
	L"在AntiSpy文件管理器中定位劫持文件"
};

WCHAR *szResetToDefult[] = {
	L"Reset To Default",
	L"重置为默认"
};

WCHAR *szAreSureResetToDefult[] = {
	L"Are you sure you want to reset the hosts to defult?",
	L"您确认要将Hosts文件重置为默认吗？"
};

WCHAR *szFindWindow[] = {
	L"Find Window",
	L"查找窗口"
};

WCHAR *szFindWindowShuoming[] = {
	L"Drag the Find Tool over a window to select it,\r\nthen release the mouse button.",
	L"请拖动左侧的指针到需要查看的窗口上释放"
};

WCHAR *szFindWindowPid[] = {
	L"PID",
	L"进程ID"
};

WCHAR *szFindWindowTid[] = {
	L"TID",
	L"线程ID"
};

WCHAR *szFindProcess[] = {
	L"Find Process",
	L"定位进程"
};

WCHAR *szFindThread[] = {
	L"Find Thread",
	L"定位线程"
};

WCHAR *szFindWindowsKillProcess[] = {
	L"Kill Process",
	L"结束进程"
};

WCHAR *szFindWindowsKillThread[] = {
	L"Kill Thread",
	L"结束线程"
};

WCHAR *szAreYOUSureKillTheProcesses[] = {
	L"Are you sure you want to kill the process?",
	L"您确定要结束这个进程吗？"
};

WCHAR *szAreYOUSureKillTheThread[] = {
	L"Are you sure you want to kill the thread?",
	L"您确定要结束这个线程吗？"
};

WCHAR *szMinimumLength[] = {
	L"Minimum Length (Chars)",
	L"最短长度(字符数)"
};

WCHAR *szStringType[] = {
	L"String Type",
	L"字符串类型"
};

WCHAR *szMemoryRegionsType[] = {
	L"Search in the following types of memory regions",
	L"搜索的内存块类型"
};

WCHAR *szYourSystemUpdate[] = {
	L"Your system has just installed the windows updates, \r\nplease reboot your system at frist.",
	L"您的电脑可能刚刚打完系统补丁，请重启电脑后再使用本工具，谢谢！"
};