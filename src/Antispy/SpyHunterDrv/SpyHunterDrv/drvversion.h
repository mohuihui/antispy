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
///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2012 - <company name here>
///
/// Defines for the version information in the resource file
///
/// (File was in the PUBLIC DOMAIN  - Created by: ddkwizard\.assarbad\.net)
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __DRVVERSION_H_VERSION__
#define __DRVVERSION_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "buildnumber.h"

// ---------------------------------------------------------------------------
// Several defines have to be given before including this file. These are:
// ---------------------------------------------------------------------------
#define TEXT_AUTHOR            mzf // author (optional value)
#define PRD_MAJVER             2 // major product version
#define PRD_MINVER             0 // minor product version
#define PRD_BUILD              1 // build number for product
#define FILE_MAJVER            2 // major file version
#define FILE_MINVER            0 // minor file version
#define FILE_BUILD             _FILE_VERSION_BUILD // file build number
#define DRV_YEAR               2011-2013 // current year or timespan (e.g. 2003-2009)
#define TEXT_WEBSITE           www.AntiSpy.cn // website
#define TEXT_PRODUCTNAME       AntiSpy // product's name
#define TEXT_FILEDESC          Anti Virus & Rootkit Tools// component description
#define TEXT_COMPANY           AntiSpy@163.com // company
#define TEXT_MODULE            AntiSpy.sys // module name
#define TEXT_COPYRIGHT         (C) 2011-2013 mzf. All Rights Reserved.//Copyright DRV_YEAR TEXT_COMPANY. // copyright information
// #define TEXT_SPECIALBUILD      // optional comment for special builds
#define TEXT_INTERNALNAME      AntiSpy.sys // copyright information
// #define TEXT_COMMENTS          // optional comments
// ---------------------------------------------------------------------------
// ... well, that's it. Pretty self-explanatory ;)
// ---------------------------------------------------------------------------

#endif // __DRVVERSION_H_VERSION__
