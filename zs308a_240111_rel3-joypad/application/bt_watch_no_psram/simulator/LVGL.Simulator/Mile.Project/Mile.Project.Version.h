﻿/*
 * PROJECT:   Mouri Internal Library Essentials
 * FILE:      Mile.Project.Version.h
 * PURPOSE:   Version Definition for Mile.Project
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#ifndef MILE_PROJECT_VERSION
#define MILE_PROJECT_VERSION

#ifndef MILE_PROJECT_MACRO_TO_STRING
#define _MILE_PROJECT_MACRO_TO_STRING(arg) L#arg
#define MILE_PROJECT_MACRO_TO_STRING(arg) _MILE_PROJECT_MACRO_TO_STRING(arg)
#endif

#ifndef MILE_PROJECT_MACRO_TO_UTF8_STRING
#define _MILE_PROJECT_MACRO_TO_UTF8_STRING(arg) #arg
#define MILE_PROJECT_MACRO_TO_UTF8_STRING(arg) \
    _MILE_PROJECT_MACRO_TO_UTF8_STRING(arg)
#endif

#ifndef MILE_PROJECT_DEFINE_COMMA_VERSION
#define MILE_PROJECT_DEFINE_COMMA_VERSION(MAJOR,MINOR,BUILD,REVISION) \
    MAJOR,MINOR,BUILD,REVISION
#endif

#ifndef MILE_PROJECT_DEFINE_DOT_VERSION
#define MILE_PROJECT_DEFINE_DOT_VERSION(MAJOR,MINOR,BUILD,REVISION) \
    MAJOR.MINOR.BUILD.REVISION
#endif

#ifndef MILE_PROJECT_COMMA_VERSION
#define MILE_PROJECT_COMMA_VERSION \
    MILE_PROJECT_DEFINE_COMMA_VERSION( \
        MILE_PROJECT_VERSION_MAJOR, \
        MILE_PROJECT_VERSION_MINOR, \
        MILE_PROJECT_VERSION_BUILD, \
        MILE_PROJECT_VERSION_REVISION)
#endif

#ifndef MILE_PROJECT_DOT_VERSION
#define MILE_PROJECT_DOT_VERSION \
    MILE_PROJECT_DEFINE_DOT_VERSION( \
        MILE_PROJECT_VERSION_MAJOR, \
        MILE_PROJECT_VERSION_MINOR, \
        MILE_PROJECT_VERSION_BUILD, \
        MILE_PROJECT_VERSION_REVISION)
#endif

#ifndef MILE_PROJECT_COMMA_VERSION_STRING
#define MILE_PROJECT_COMMA_VERSION_STRING \
    MILE_PROJECT_MACRO_TO_STRING(MILE_PROJECT_COMMA_VERSION)
#endif

#ifndef MILE_PROJECT_DOT_VERSION_STRING
#define MILE_PROJECT_DOT_VERSION_STRING \
    MILE_PROJECT_MACRO_TO_STRING(MILE_PROJECT_DOT_VERSION)
#endif

#ifndef MILE_PROJECT_COMMA_VERSION_UTF8_STRING
#define MILE_PROJECT_COMMA_VERSION_UTF8_STRING \
    MILE_PROJECT_MACRO_TO_UTF8_STRING(MILE_PROJECT_COMMA_VERSION)
#endif

#ifndef MILE_PROJECT_DOT_VERSION_UTF8_STRING
#define MILE_PROJECT_DOT_VERSION_UTF8_STRING \
    MILE_PROJECT_MACRO_TO_UTF8_STRING(MILE_PROJECT_DOT_VERSION)
#endif

#ifndef MILE_PROJECT_DEFINE_SIMPLE_VERSION
#define MILE_PROJECT_DEFINE_SIMPLE_VERSION(MAJOR,MINOR) \
    MAJOR.MINOR
#endif

#ifndef MILE_PROJECT_SIMPLE_VERSION
#define MILE_PROJECT_SIMPLE_VERSION \
    MILE_PROJECT_DEFINE_SIMPLE_VERSION( \
        MILE_PROJECT_VERSION_MAJOR, \
        MILE_PROJECT_VERSION_MINOR)
#endif

#ifndef MILE_PROJECT_SIMPLE_VERSION_STRING
#define MILE_PROJECT_SIMPLE_VERSION_STRING \
    MILE_PROJECT_MACRO_TO_STRING(MILE_PROJECT_SIMPLE_VERSION)
#endif

#ifdef MILE_PROJECT_VERSION_TAG
#define MILE_PROJECT_VERSION_TAG_STRING L" " MILE_PROJECT_VERSION_TAG
#else
#define MILE_PROJECT_VERSION_TAG_STRING
#endif

#ifndef MILE_PROJECT_VERSION_STRING
#define MILE_PROJECT_VERSION_STRING \
    MILE_PROJECT_SIMPLE_VERSION_STRING \
    MILE_PROJECT_VERSION_TAG_STRING
#endif

#ifndef MILE_PROJECT_SIMPLE_VERSION_UTF8_STRING
#define MILE_PROJECT_SIMPLE_VERSION_UTF8_STRING \
    MILE_PROJECT_MACRO_TO_UTF8_STRING(MILE_PROJECT_SIMPLE_VERSION)
#endif

#ifdef MILE_PROJECT_VERSION_TAG
#define MILE_PROJECT_VERSION_TAG_UTF8_STRING " " MILE_PROJECT_VERSION_TAG
#else
#define MILE_PROJECT_VERSION_TAG_UTF8_STRING
#endif

#ifndef MILE_PROJECT_VERSION_UTF8_STRING
#define MILE_PROJECT_VERSION_UTF8_STRING \
    MILE_PROJECT_SIMPLE_VERSION_UTF8_STRING \
    MILE_PROJECT_VERSION_TAG_UTF8_STRING
#endif

#endif
