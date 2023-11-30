/*
SpeedSim - an OGame (www.ogame.org) combat simulator
Copyright (C) 2004-2008 Maximialian Matthé & Nicolas Höft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

/*!
	\file
	\brief This file contains several defines
*/

////////////////////////////////////////////////////////
// global defines

// macros, to decide which array is meant
#define ATTER 0
#define DEFFER 1

// version of kernel. Corresponds to the current version of the game on live servers
#define KERNEL_VERSION _T("11.4.0")

//! default defense rebuild factor
#define DEF_AUFBAU_FAKTOR 0.80f
// time the simulator waits for 100k units when aborting a simulation
#define WARTEN_PRO_100K 1000


/*!
	\def RFPERC(x)
	\brief RF(x) converts RF(x) into RF x%
*/
#define RFPERC(x) (x == 0 ? 0 : (100-100.0f/x)*100)

#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) (((a) < (b)) ? (a) : (b))


// maximum number of fleets per team
#define MAX_PLAYERS_PER_TEAM 16

/*!
	\def UNICODE
	\brief Define this, to compile as Unicode version
*/ 
//#define UNICODE 

// remove old defines
#undef _T
#undef _tcscpy
#undef _tcsncpy
#undef _ttoi
#undef _tcscat
#undef _tcsncat
#undef _tcslen
#undef _tfopen
#undef _fgetts
#undef _istdigit

#ifdef UNICODE
	#define _T(x) L ## x
	typedef wchar_t TCHAR;
	#define _tcscpy wcscpy
	#define _tcsncpy wcsncpy
	#define _ttoi(x) wcstol(x, NULL, 10)
	#define _tcscat wcscat
	#define _tcsncat wcsncat
	#define _tcslen wcslen
	#define _tfopen _wfopen
	#define _fgetts fgetws
	#define _istdigit iswdigit
#else
	#define  _T(x) x
	typedef char TCHAR;
	#define _tcscpy strcpy
	#define _tcsncpy strncpy
	#define _ttoi atoi
	#define _tcscat strcat
	#define _tcsncat strncat
	#define _tcslen strlen
	#define _tfopen fopen
	#define _fgetts fgets
	#define _istdigit isdigit
#endif

#undef _stprintf
int _stprintf(TCHAR * target, const TCHAR * format, ...);

/*!
	\var typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > genstring;
	\brief Type for Unicode and non unicode strings

	Use this type for all strings to be unicode compatible
*/
typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > genstring;
typedef basic_stringstream<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > genstrstream;

#define _STR(x) genstring(_T(x))

// Now in the static library.
#define SPEEDKERNEL_API


// Standard windows data types
typedef unsigned int        UINT;
typedef unsigned long       DWORD;
typedef unsigned long       ULONG;
typedef unsigned short int  WORD;
typedef unsigned char       BYTE;

#ifndef _MSC_VER
	typedef long long  __int64;
#endif //Win32

size_t GetNextNumber(genstring str, int& num, size_t pos = 0);
