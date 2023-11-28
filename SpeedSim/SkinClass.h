/*
SpeedSim - a OGame (www.ogame.org) combat simulator
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

#define _CRT_SECURE_NO_DEPRECATE

#ifdef UNICODE
	#ifndef _UNICODE
		#define _UNICODE 
	#endif
#endif

#include <map>
#include <vector>
#include <tchar.h>
#include <windows.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <string>

using namespace std;

typedef basic_string<TCHAR, char_traits<TCHAR>, allocator<TCHAR> > genstring;

enum WNDTYPE
{
	WT_NONE = 0,
	WT_BG,
	WT_BTN,
	WT_EDIT,
	WT_CHKBOX,
	WT_GROUPBOX,
	WT_LABEL,
	WT_STATUSBAR,
	WT_RADIOBTN
};

enum BMPTYPE
{
	BT_BG,
	BT_BTN_NORMAL,
	BT_BTN_PRESSED,
	BT_CHK_NORMAL,
	BT_CHK_SEL
};

#define SCWND_ALLBTNS ((HWND)-1)
#define SCWND_ALLCHK ((HWND)-2)
#define SCWND_ALLLABELS ((HWND)-3)
#define SCWND_ALLEDITS ((HWND)-4)
#define SCWND_ALLGROUPBOXES ((HWND)-5)

#define SCMODE_COLOR 0
#define SCMODE_BKCOLOR 1
#define SCMODE_BKMODE 2
#define SCMODE_BRUSH 3

#define FF_BOLD 2
#define FF_ITALIC 4
#define FF_UNDERLINE 8
#define FF_STRIKEOUT 16

typedef HRESULT (WINAPI SETWINDOWTHEME)(HWND, LPCWSTR, LPCWSTR);
typedef HRESULT (WINAPI ISAPPTHEMED)();
typedef HRESULT (WINAPI DRAWTHEMEPARENTBACKGROUND)(HWND, HDC, RECT*);

class CSkinClass
{
public:
	CSkinClass();
	~CSkinClass();

	bool AddWindow(HWND wnd, WNDTYPE type);
	bool AddAllChildWindows(HWND ParentWnd);
	bool SetColor(HWND wnd, DWORD mode, DWORD Value);
	
	bool SetTransparentColor(HWND wnd, DWORD color);
	bool SetBtnTransparentColor(DWORD color);

	bool SetFont(HWND hwnd, LPCTSTR lpszName, INT iSize, WORD wFontFormat);
	bool LoadBitmap(TCHAR* name, BMPTYPE type);
	bool LoadDataFile(TCHAR* name);
	const TCHAR* GetCurrentDataFile();
	bool SetBitmap(HBITMAP bmp, BMPTYPE type);
	bool DrawWindow(DRAWITEMSTRUCT* item);
	bool DrawBGWindow(HWND wnd);
	bool DrawWndBG(DRAWITEMSTRUCT* item);

	bool Free();

private:
	struct SkinnedWnd
	{
		HWND wnd;
		WNDTYPE type;
		WNDPROC oldWndProc;
		DWORD OldStyle;
		DWORD BkColor;
		DWORD BkMode;
		DWORD TextColor;
		HBRUSH UseBrush;
		HFONT UseFont;
		HFONT OldFont;
		BOOL bTransColor;
		UINT TransColor;
		SkinnedWnd() {
			ZeroMemory(this, sizeof(SkinnedWnd));
		}
	};
	
	struct BMP
	{
		HBITMAP handle;
		BITMAP info;
		BMP() {
			handle = NULL;
			ZeroMemory(&info, sizeof(info));
		}
	};

	int GetBitmapType(DRAWITEMSTRUCT* s);
	bool DrawWndText(DRAWITEMSTRUCT* s);
	bool TransparentBltU(HDC dcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC dcSrc, int nXOriginSrc, int nYOriginSrc, 
		int nWidthSrc, int nHeightSrc, UINT crTransparent);
	void DeactivateTheme(HWND wnd, bool deactivate);


	static LRESULT CALLBACK SubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK EditSubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK GroupBoxSubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LabelSubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

	map<HWND, SkinnedWnd> m_Windows;
	vector<BMP> m_Bitmaps;

	float m_BGImScaleX, m_BGImScaleY;
	int m_iStatusBarHeight;

	HWND m_BGWnd;
	WNDPROC m_OldWndProc;
	DWORD m_OldBGWndSyle;
	
	bool m_ThemedApp;
	HMODULE m_hThemeUX;
	SETWINDOWTHEME* m_pSetWindowTheme;
	ISAPPTHEMED* m_pIsAppThemed;
	DRAWTHEMEPARENTBACKGROUND* m_pDrawThemeParentBackground;

	genstring m_DataFile;
};
