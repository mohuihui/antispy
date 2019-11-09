AntiSpy is a free but powerful anti virus and rootkits toolkit.It offers you the ability with the highest privileges that can detect,analyze and restore various kernel modifications and hooks.With its assistance,you can easily spot and neutralize malwares hidden from normal detectors.

Warning: 
I hope the AntiSpy can be useful to you,but I can not be held responsible for any error or negligence derived therefrom,you use it at your own risk.

*************************************************************************************

Our progress comes from our users. Help us be better.

Thanks 
	quark, xanax, EP_X0FF, STRELiTZIA, Petra. (Ps. This list is in no particular order)

*************************************************************************************
	
Email: zhenfei.mzf@gmail.com

If you have any need,please feel free to contact with me.
eg:
1.If there is any problem with your system,I will help you for free.
2.If you have some good ideas or advices,please tell me.I can improve AntiSpy in future versions.
3.If AntiSpy makes your system BSoD,or AntiSpy application crash,send the dump file to me.

*************************************************************************************

Default Colors

	*Process Manager:
	Hidden process - Red
	The corporation of process is Microsoft - Black
	Normal process,the corporation is not Microsoft - Blue
	Microsoft's process have other corporation modules - Orange 

    *File Manager:
	System files(folders) - Red
	Hidden files(folders) - Gray
	Normal files(folders) - Black

    *General:
	No signature module - Purple
	Hidden objects or hooks - Red
	The corporation of module is Microsoft - Black
	Normal module,the corporation is not Microsoft - Blue

*************************************************************************************

Default Hotkeys
	Show AntiSpy main window - Ctrl + Alt + Shift + A
	Show process modules - Ctrl + Alt + Shift + M
	Show process threas - Ctrl + Alt + Shift + T
	Show process handles - Ctrl + Alt + Shift + H
	Show process windows - Ctrl + Alt + Shift + W
	Show process memory - Ctrl + Alt + Shift + E
	Show process hotkeys - Ctrl + Alt + Shift + O
	Show process privileges - Ctrl + Alt + Shift + P
	Show process timers - Ctrl + Alt + Shift + I

*************************************************************************************

Currently,the following features are available(including but not limited to):

*Process Manager
	Display system process and thread basic informations.
	Detect hidden processes,threads,process modules.
	Terminate, suspend and resume processes and threads.
	View and manipulate process handles,windows and memory regions.
	View and manipulate process hotkeys,privileges,and timers.
	Detect and restore process hooks incluing inline hooks,patches,iat and eat hooks.
	Inject dll,dump process memory.
	Create debug dump,inclue mini dump and full dump.

*Kernel Module Viewer
	Display kernel module basic information,include image base,size,driver object,and so on.
	Detect hidden kernel modules.
	Unload kernel modules.
	Dump kernel image memory.
	Display and delete system driver service informations.

*Hook Detector
	Detect and restore SSDT,Shadow SSDT,sysenter and int2e hooks.
	Detect and restore FSD and keyboard disptach hooks.
	Detect and restore kernel code hooks including kernel inline hooks,patches,iat and eat hooks.
	Detect and restore message hooks,both global and local.
	Detect and restore kernel ObjectType hooks.
	Display Interrupt Descriptor Table(IDT).

*Other Kernel Information Viewer
	View and remove kernel notifications.
	View filters for common devices include disk,volume,keyboard and network devices. 
	View IO timers,DPC timers,system threads,and so on.

*Registry Manager
	View and edit system registry.
	Detect hidden registry entries using live registry hive analysis.

*File Manager
	Display file basic information,include file name,size,attributes,and so on.
	Detect hidden files.
	View and delete locked files and folders.

*Service Manager
	Display system services basic informations.
	Control services status.
	Modify services startup type.

*Autorun Manager
	Display almost all kinds of system autorun types.
	Enable,disable or permanently delete autoruns.

*Network Viewer
	Display current network connections,include TCP and UDP informations.
	View and delete IE plugins and context menu.
	Display winsock providers(LSP).
	View and edit hosts file.

*Other Tools
	Hex Editor - View and edit memory,include ring3 process memory and ring0 system memory.
	Disassembler - Like OllyDBG,support ring3 process memory and ring0 system memory.

*Settings
	Custom color settings.

*************************************************************************************

Change Log
V1.9 (2013-07-13)
	Added:
	1.Enumerate and fix file association
	2.Enumerate image hijacks
	3.Enumerate IME
	4.Added anti screen logger feature
	5.Added reset hosts file to default feature
	
	Modified:
	1.Fixed several bugs

V1.8 (2013-06-28)
	Added:
	1.Search process handles
	2.Import registry
	3.Registry unlocker
	4.Fix LSP
	5.Self-Defense
	6.MBR check and fix
	7.More advanced features,such as force reboot, fix safeboot.
	
	Modified:
	1.Fixed a BSoD bug
	2.Modify the default hotkeys
	3.GUI improvements

V1.7 (2013-06-02)
	Added:
	1.File manager added search online feature
	2.File manager added calc and compare MD5,SHA1 feature
	4.Registry manager supported key abbreviation,eg.HKEY_CURRENT_USER->HKCU
	5.Added view and delete IE plugins and context menu
	6.Added a tools tab
	7.Added detect more autorun items,and you can enable or disable it

	Modified:
	1.Fixed a BSoD bug
	2.Fixed the bug which antispy can not be normal displayed in some win8 system
	3.GUI improvements

V1.6 (2013-05-16)
	Add:
	1.Added a network tab.
	2.View and edit hosts file.
	3.Display current network connections
	4.View LSP informations

	Modify:
	1.Fixed several other bugs.

V1.5 (2013-04-28)
	Added:
	1.Added a file manager
	2.Added custom color settings
	3.Process manager added display process started time

	Modified:
	1.Fixed the bug which antispy can not be normal displayed in some computers.(Thanks EP_X0FF for his feedback and test).
	2.Optimized display Message hooks
	3.GUI improvements
	4.Fixed several bugs

V1.4 (2013-03-30)
	Add:
	1.The border of the window can be stretched.
	2.Registry manager added hive analysis.
	3.Added a config tab.
	4.Added a lower pane to display process modules.
	5.Display process detail informations as a tooltips window. 
	6.Added hot keys to view process modules,threads,handles and so on.
	7.Added a "quick positioning" feature in registry tab.
	8.Added a "Find" feature in registry tab.
	9.Added a hex editer in process memory dialog.

	Modify:
	1.Fixed several bugs.
	2.Optimized the UI.

V1.3 (2013-03-18)
	Added:
	1.Added edit REG_BINARG type values feature
	2.Added rename keys and values feature
	3.Process modules dialog added unload and delete file feature
	4.Process manager added inject dll feature

	Modified:
	1.Optimized detect SSDT HOOK
	2.Optimized registry manager
	3.Optimized process tree
	4.Fixed some translation problems of English version
	5.GUI improvements

V1.2 (2013-03-06)
	Added:
	1.Added show process tree feature
	2.Added kill process tree feature

	Modified:
	1.Fixed several bugs

V1.1 (2013-02-25) 
	Added:
	1.Added scan process hooks feature
	2.Added view process detail informations feature
	3.Added create debug dump feature

	Modified:
	1.Optimized search online feature
	2.Optimized open file in kernel
	3.Fixed two BSoD bugs
	4.GUI improvements
		
V1.0 (2013-01-16)
	1.Finished the first version