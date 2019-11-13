# AntiSpy

![avatar](icon/icon.ico)

[English](./README.md) | 简体中文

## 一款功能强大的手工杀毒辅助工具

AntiSpy 是一款完全免费，并且功能强大的手工杀毒辅助工具。她可以枚举系统中隐藏至深的进程、文件、网络连接、内核对象等，并且也可以检测用户态、内核态各种钩子。在她的帮助下，您可以删除各种顽固病毒、木马、Rootkit，还您一片干净舒适的上网环境。

## 写在最前

1. 此工具是本人学习 Windows 内核的一个产物，界面层采用 MFC 编写，驱动层采用 WDK7600 编译。

2. 此工具 90% 以上的功能都是在内核驱动中实现，Ring3 层程序只做展示结果之用。虽然本人已经在诸多环境中做过严格的测试，但也可能存在一定的不稳定性。

3. 此工具目前支持 Win2000、XP、Win2003、Vista、Win2008、Win7、Win8 等绝大多数 Windows 32位 操作系统。程序界面支持简体中文和英文两种语言，会随着操作系统语言的不同而自适应。

4. 如果本程序或系统奔溃，请将相应的dump发送给我，以帮我改进本工具。另外，如果您有好的意见或者建议也可通过邮件发送给我（zhenfei.mzf@gmail.com）。

5. 如果您对 Window 操作系统不甚了解，请谨慎使用本工具，胡乱操作可能会对您的操作系统造成不可逆的严重后果。                

6. 在您使用本工具的过程中，对您造成的直接或者间接的损失，本人概不负责。

## 开发环境

* 开发工具: Visual Studio 2008
* 用户层: MFC
* 内核层: WDK7600
* 3方控件库: Codejock toolkit pro

## 代码结构

```
AntiSpy_Root_Dir
├── LICENSE                         (开源协议)
├── README.md                       (AntiSpy工程介绍文档)
├── doc                             (工具介绍，版本更新记录等)
│   ├── Readme.txt 
│   └── 说明.txt
├── icon                            (AntiSpy软件图标，感谢设计师HXJ童鞋)
│   └── icon.ico
├── src                               
│   ├── Antispy                     (AntiSpy主工程代码)
│   │   ├── Common                  (驱动和界面共用的头文件、数据结构等)
│   │   ├── SpyHunter               (AntiSpy用户态界面代码，采用MFC编写)
│   │   ├── SpyHunter.sln           (VS2008工程文件)
│   │   └── SpyHunterDrv            (AntiSpy内核驱动代码)
│   └── ResourceEncrypt             (对驱动等资源进行加密的工程)
│       ├── ResourceEncrypt
│       ├── ResourceEncrypt.sln
│       └── clear.bat
└── tools
    ├── ResourceEncrypt.exe         (已经编译好的加密工具)
    └── TestTools.exe               (测试AntiSpy安全能力是否可用的工具)
```

## 主要功能

本工具目前实现功能如下（包括但不限于）：

### 进程管理器
* 查看进程线程、模块、窗口、内存、热键、定时器、权限等信息；
* 查看进程运行时间、命令行、当前目录、PEB等信息；
* 关闭进程、关闭线程、卸载模块、拷贝进程内存，查找进程模块；
* 创建进程调试DUMP；
* 往进程中注入模块；
* 扫描进程Ring3钩子；

### 各种钩子查看及恢复
* 常见内核钩子的查看和恢复，包括SSDT、Shadow SSDT、FSD、键盘、鼠标、TCPIP、Classpnp、Atapi、Acpi、IDT、Object hook、内核入口等；
* 内核模块的iat、eat、inline hook、patches检测和恢复；
* 用户层消息钩子的查看和卸载；
* CreateProcess、CreateThread、LoadImage、Registry、Shutdown等Notify Routine信息查看和删除；

### 内核对象查看及管理
* 内核驱动模块的查看，内核驱动模块的内存拷贝，卸载驱动内核模块；
* DPC和IO定时器等内核定时器的查看和删除；
* 系统线程的查看和结束；
* WorkerThread信息查看；
* 内核调试寄存器的查看和恢复；
* 磁盘、卷、键盘、网络层等过滤驱动的枚举；
* 内核对象劫持检测；
* 直接IO进程的检测和恢复；

### 注册表编辑器
* 通过解析原始hive，能够查看和编辑隐藏的注册表键值；
* 快速定位到最常用的注册表键

### 文件管理器
* 展示文件基本信息，包括文件名、文件属性、文件大小等；
* 快速定位到最常用的文件夹；
* 通过IRP底层操作，查看和编辑隐藏的文件；
* 查看和删除被锁定的文件和文件夹；
* 计算文件hash及文件比较器；

### 系统服务管理器
* 系统服务的枚举和操作，可以枚举隐藏的服务；
* 对系统服务进行管理，比如更改启动顺序，启动状态等；

### 开机自启动项管理
* 能够枚举系统中几乎所有的开机启动项；
* 管理启动项，包括停止、运行、永久删除；

### 网络信息管理
* 查看应用程序的联网情况，包括端口、远程地址等信息；
* 对hosts文件的查看、编辑和重置为默认；
* 查看和修复系统LSP信息；

### 其他一些常用功能
* 系统用户、隐藏用户的枚举和删除
* 禁止创建进程、禁止创建线程、禁止加载驱动等反病毒选项
* 解锁注册表、任务管理器、命令解释器等
* 修复安全模式
* 以16进制形式查看和编辑系统内存和进程内存
* 反汇编系统内存和进程内存
* MBR病毒的检测和修复
* 常用文件关联项的枚举和修复
* 映像劫持的检测和修复
* IME输入法的枚举和管理
* 反间谍记录器，包括反截屏记录器等
* ...

## 用户界面

如下仅展示了软件部分功能截图，想了解 AntiSpy 完整功能，可网上搜索下载体验。

**进程管理**
![avatar](images/process_tree.png)

**进程菜单**
![avatar](images/process_menu.png)

**网络连接管理**
![avatar](images/network.png)

**文件管理**
![avatar](images/file_manager.png)

**自动运行软件管理**
![avatar](images/autorun.png)

## 联系我

QQ：454858525

邮箱：zhenfei.mzf@gmail.com

微博：https://weibo.com/minzhenfei

对 AntiSpy 有任何使用上或者技术上的疑问，欢迎随时联系作者。

## 开源协议

基于 [木兰宽松许可证](https://license.coscl.org.cn/MulanPSL/) 进行开源。