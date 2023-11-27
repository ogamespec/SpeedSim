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

#undef STRICT
#include "SkinClass.h"

CSkinClass* ptr = NULL;
BOOL CALLBACK EnumProc(HWND wnd, LPARAM lParam);

CSkinClass::CSkinClass()
{
	ptr = this;
	m_Bitmaps.resize(50);
    m_OldWndProc = NULL;
    m_BGWnd = NULL;
    m_iStatusBarHeight = 0;
    
    m_ThemedApp = false;
    m_hThemeUX = LoadLibrary(_T("uxtheme.dll"));

    m_pSetWindowTheme = NULL;
    m_pIsAppThemed = NULL;
    m_pDrawThemeParentBackground = NULL;
    if(m_hThemeUX)
    {
        m_pSetWindowTheme = (SETWINDOWTHEME*)GetProcAddress(m_hThemeUX, "SetWindowTheme");
        m_pIsAppThemed = (ISAPPTHEMED*)GetProcAddress(m_hThemeUX, "IsAppThemed");
        m_pDrawThemeParentBackground = (DRAWTHEMEPARENTBACKGROUND*)GetProcAddress(m_hThemeUX, "DrawThemeParentBackground");
    }
}

CSkinClass::~CSkinClass()
{
    Free();
    FreeLibrary(m_hThemeUX);
}

bool CSkinClass::Free()
{
	size_t i = 0;

	map<HWND, SkinnedWnd>::iterator it = m_Windows.begin();
	while(it != m_Windows.end())
    {
        // free memory
        if(it->second.UseBrush)
            DeleteObject(it->second.UseBrush);
        if(it->second.UseFont)
            DeleteObject(it->second.UseFont);
        if(!IsWindow(it->second.wnd))
        {
            it++;
            continue;
        }
        // set back subclasses
        if(it->second.oldWndProc)
            SetWindowLongPtr(it->second.wnd, GWLP_WNDPROC, (LONG)it->second.oldWndProc);
        // set back to normal style
        if(it->second.wnd != m_BGWnd)
            SetWindowLongPtr(it->second.wnd, GWL_STYLE, (LONG)it->second.OldStyle);
        // set back themed state
        if(m_ThemedApp && (it->second.type == WT_CHKBOX || it->second.type == WT_RADIOBTN))
        {
            DeactivateTheme(it->second.wnd, false);
        }
        SendMessage(it->second.wnd, WM_SETFONT, (WPARAM)it->second.OldFont, 0);
        SetWindowPos(it->second.wnd, 0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
		it++;
	}

    for(i = 0; i < m_Bitmaps.size(); i++) {
        if(m_Bitmaps[i].handle)
		    DeleteObject(m_Bitmaps[i].handle);
    }

    m_BGWnd = NULL;
    m_OldWndProc = NULL;
    m_DataFile = _T("");
    m_iStatusBarHeight = 0;

    m_Windows.clear();
	return true;
}

//////////////////////////////////
/// Window Functions
bool CSkinClass::AddWindow(HWND wnd, WNDTYPE type)
{
	if(type == WT_NONE)
		return false;

	if(!wnd)
		return false;
    
    // don't add same window twice
    map<HWND, SkinnedWnd>::iterator it = m_Windows.find(wnd);
    if(it != m_Windows.end())
        return false;

	WNDPROC oldWndProc = NULL;
    if(m_pIsAppThemed && !m_ThemedApp)
    {
        m_ThemedApp = m_pIsAppThemed() != 0;
    }

	SkinnedWnd w;

	w.UseBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    w.OldStyle = GetWindowLongPtr(wnd, GWL_STYLE);
    w.OldFont = (HFONT)SendMessage(wnd, WM_GETFONT, 0, NULL);

	if(type == WT_BG)
	{
		oldWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (long)SubClassProc);
	}
	if(type == WT_BTN)
	{
		// set to ownerdraw
		SetWindowLongPtr(wnd, GWL_STYLE, GetWindowLongPtr(wnd, GWL_STYLE) | BS_OWNERDRAW);
		w.BkColor = 0;
		w.BkMode = TRANSPARENT;
		w.TextColor = RGB(0, 0, 0);
	}
	if(type == WT_GROUPBOX)
	{
		w.BkColor = 0;
		w.BkMode = TRANSPARENT;
		w.TextColor = RGB(255, 255, 255);
		oldWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (long)GroupBoxSubClassProc);
	}
	if(type == WT_CHKBOX || type == WT_RADIOBTN)
	{
		w.BkColor = 0;
		w.BkMode = TRANSPARENT;
		w.TextColor = RGB(0, 0, 0);
        DeactivateTheme(wnd, true);
	}
	if(type == WT_EDIT)
	{
        //DeactivateTheme(wnd, true);
		oldWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (long)EditSubClassProc);
		w.BkColor = 0;
		w.BkMode = TRANSPARENT;
		w.TextColor = RGB(0, 0, 0);
	}
	if(type == WT_LABEL)
	{
		oldWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (long)LabelSubClassProc);
        w.BkColor = 0;
		w.BkMode = TRANSPARENT;
		w.TextColor = RGB(0, 0, 0);
	}
	if(type == WT_STATUSBAR)
    {
        RECT r;
        SendMessage(wnd, SB_GETRECT, (WPARAM)0, (LPARAM)&r);
        m_iStatusBarHeight = r.bottom - r.top; 
    }

	w.type = type;
	w.wnd = wnd;
	w.oldWndProc = oldWndProc;
	m_Windows[wnd] = w;

	if(type == WT_BG)
	{
		m_OldWndProc = oldWndProc;
		m_BGWnd = wnd;
	}
	return true;
}

BOOL CALLBACK EnumProc(HWND wnd, LPARAM lParam)
{
    TCHAR ClassName[256];

    DWORD Style = GetWindowLongPtr(wnd, GWL_STYLE);
    GetClassName(wnd, ClassName, 255);
    if(!_tcscmp(ClassName, _T("Button")))
    {

        if(((Style & BS_GROUPBOX)^BS_GROUPBOX) == 0)
            ptr->AddWindow(wnd, WT_GROUPBOX);
        else if(((Style & BS_AUTOCHECKBOX)^BS_AUTOCHECKBOX) == 0 || ((Style & BS_CHECKBOX)^BS_CHECKBOX) == 0)
            ptr->AddWindow(wnd, WT_CHKBOX);
        else if(((Style & BS_AUTORADIOBUTTON)^BS_AUTORADIOBUTTON) == 0 || ((Style & BS_AUTORADIOBUTTON)^BS_AUTORADIOBUTTON) == 0)
            ptr->AddWindow(wnd, WT_RADIOBTN);
        else
            ptr->AddWindow(wnd, WT_BTN);
    }
    if(!_tcscmp(ClassName, _T("Static")))
    {
        ptr->AddWindow(wnd, WT_LABEL);
    }
    if(!_tcscmp(ClassName, _T("Edit")))
    {
        if(((Style & ES_READONLY) ^ES_READONLY) != 0)
            ptr->AddWindow(wnd, WT_EDIT);
        else
            ptr->AddWindow(wnd, WT_LABEL);
    }
    if(!_tcscmp(ClassName, STATUSCLASSNAME))
    {
        ptr->AddWindow(wnd, WT_STATUSBAR);
    }
    return true;
}

bool CSkinClass::AddAllChildWindows(HWND ParentWnd)
{
	if(!ParentWnd)
		return false;
	return (EnumChildWindows(ParentWnd, EnumProc, (LPARAM)NULL) == TRUE);
}


bool CSkinClass::DrawBGWindow(HWND wnd)
{
	PAINTSTRUCT ps;
	HDC dc;
	HBITMAP bmp;
	SIZE s;
	SIZE w;
	RECT r;
	
	GetClientRect(wnd, &r);
    r.bottom -= m_iStatusBarHeight;
	w.cx = r.right - r.left;
	w.cy = r.bottom - r.top;

	bmp = m_Bitmaps[BT_BG].handle;

	BeginPaint(wnd, &ps);
		dc = CreateCompatibleDC(0);
		HBITMAP oldbmp = (HBITMAP)SelectObject(dc, bmp);
		s.cx = m_Bitmaps[BT_BG].info.bmWidth;
		s.cy = m_Bitmaps[BT_BG].info.bmHeight;

        BitBlt(ps.hdc, 0, 0, w.cx, w.cy, dc, 0, 0, SRCCOPY);

		// restore old
		SelectObject(dc, oldbmp);
		DeleteDC(dc);
	EndPaint(wnd, &ps);
	
	return true;
}

bool CSkinClass::DrawWindow(DRAWITEMSTRUCT* item)
{
	map<HWND, SkinnedWnd>::iterator i;
	if((i = m_Windows.find(item->hwndItem)) == m_Windows.end())
		return false;
	
	SkinnedWnd wnd = i->second;
	if(wnd.type == WT_LABEL)
		return DrawWndText(item);	

	const BMP& bmpinfo = m_Bitmaps[GetBitmapType(item)];
	HBITMAP bmp = bmpinfo.handle;
	
	// prepare painting
	SIZE b, w;
	RECT r;

    int oldStetchBltMode = SetStretchBltMode(item->hDC, COLORONCOLOR);

	if(wnd.type == WT_BTN)
	{
		HDC dc = CreateCompatibleDC(0);
		HBITMAP oldbmp = NULL;
        if(bmp)
            oldbmp = (HBITMAP)SelectObject(dc, bmp);

		// calculate dimensions
		b.cx = bmpinfo.info.bmWidth;
		b.cy = bmpinfo.info.bmHeight;
		w.cx = item->rcItem.right - item->rcItem.left;
		w.cy = item->rcItem.bottom - item->rcItem.top;
        DrawWndBG(item);

		// blit bitmap on button
        if(wnd.bTransColor)
            TransparentBltU(item->hDC, 0, 0, w.cx, w.cy, dc, 0, 0, b.cx, b.cy, wnd.TransColor);
        else
		    StretchBlt(item->hDC, 0, 0, w.cx, w.cy, dc, 0, 0, b.cx, b.cy, SRCCOPY);

		if(oldbmp)
            SelectObject(dc, oldbmp);
		
		// draw text
		DrawWndText(item);
		DeleteDC(dc);
	}
	if(wnd.type == WT_CHKBOX)
	{
		HBITMAP old;
		// first draw background
		GetWindowRect(item->hwndItem, &r);
		RECT r2;
		GetClientRect(item->hwndItem, &r2);

		HDC bg2 = CreateCompatibleDC(NULL);
		old = (HBITMAP)SelectObject(bg2, bmp);
		// draw box
		if(StretchBlt(item->hDC, 2, r2.bottom > 16 ? (r2.bottom - 16) / 2 : 0,
								16, r2.bottom > 16 ? 16 : r2.bottom, 
								bg2, 0, 0, 16, 16, SRCCOPY))
			DWORD e = GetLastError();

		SelectObject(bg2, old);
		DeleteDC(bg2);
		// draw text
		DrawWndText(item);
	}
    if(wnd.type == WT_GROUPBOX)
    {
        RECT r;
        SIZE s;
        TCHAR c[64];
        GetClientRect(item->hwndItem, &r);
        item->rcItem = r;
        DWORD num = SendMessage(item->hwndItem, WM_GETTEXT, 64, (LPARAM)c);
        c[num] = 0;
        HPEN pen = CreatePen(PS_SOLID, 1, wnd.BkColor);
        HPEN op = (HPEN)SelectObject(item->hDC, pen);
        HFONT f;
        if(wnd.UseFont)
            f = wnd.UseFont;
        else
            f = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        HFONT of = (HFONT)SelectObject(item->hDC, f);
        
        GetTextExtentPoint32(item->hDC, c, num, &s);

        MoveToEx(item->hDC, 6, 6, NULL);
        LineTo(item->hDC, 1, 6);
        LineTo(item->hDC, 1, r.bottom - 1);
        LineTo(item->hDC, r.right - 1, r.bottom - 1);
        LineTo(item->hDC, r.right - 1, 6);
        LineTo(item->hDC, s.cx + 10, 6);

        DrawWndText(item);

		SelectObject(item->hDC, op);
        SelectObject(item->hDC, of);
        if(!wnd.UseFont)
            DeleteObject(f);
        DeleteObject(pen);
    }

    // restore old mode
    SetStretchBltMode(item->hDC, oldStetchBltMode);
	return true;
}

bool CSkinClass::DrawWndBG(DRAWITEMSTRUCT* item)
{
	RECT client;
	POINT p1, p2;
	RECT src;
    GetClientRect(item->hwndItem, &client);
	p1.x = 0; p1.y = 0; p2.x = 0; p2.y = 0;
	ClientToScreen(m_BGWnd, &p1);
	ClientToScreen(item->hwndItem, &p2);
	p1.x = p2.x - p1.x;
	p1.y = p2.y - p1.y;
	src.top = (p1.y) / m_BGImScaleY;
	src.bottom = (p1.y + client.bottom) / m_BGImScaleY;
	src.left = (p1.x) / m_BGImScaleX;
	src.right = (p1.x + client.right) / m_BGImScaleX;
	
    HDC bg = CreateCompatibleDC(NULL);
    HBITMAP old = (HBITMAP)SelectObject(bg, m_Bitmaps[BT_BG].handle);
    if(!StretchBlt(item->hDC, 0, 0, client.right, client.bottom, bg, src.left, src.top, src.right - src.left, src.bottom - src.top, SRCCOPY))
        DWORD e = GetLastError();
    SelectObject(bg, old);
    DeleteDC(bg);

	return true;
}

bool CSkinClass::DrawWndText(DRAWITEMSTRUCT* s)
{
	if(!s)
		return false;

	map<HWND, SkinnedWnd>::iterator i = m_Windows.find(s->hwndItem);
	if(i == m_Windows.end())
		return false;

	SkinnedWnd w = i->second;

	TCHAR text[256];
	GetWindowText(s->hwndItem, text, 256);
	RECT r = s->rcItem;
	
	if(w.type == WT_BTN)
	{
		if(s->itemState & ODS_SELECTED)
		{
			r.top += 2;
			r.left += 2;
		}

		if(s->itemState & ODS_DISABLED)
			SetTextColor(s->hDC, RGB(128, 128, 128));
		else
			SetTextColor(s->hDC, w.TextColor);

		int m = SetBkMode(s->hDC, w.BkMode);
		COLORREF cl = SetBkColor(s->hDC, w.BkColor);
		DrawText(s->hDC, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(s->hDC, m);
		SetBkColor(s->hDC, cl);
	}
	if(w.type == WT_CHKBOX)
	{
		int m = SetBkMode(s->hDC, w.BkMode);
		COLORREF c = SetTextColor(s->hDC, w.TextColor);
		int bc = SetBkColor(s->hDC, w.BkColor);
		r.left += 19;
		DrawText(s->hDC, text, -1, &r, DT_VCENTER | DT_SINGLELINE);
		SetBkMode(s->hDC, m);
		SetTextColor(s->hDC, c);
		SetBkColor(s->hDC, bc);
	}
    if(w.type == WT_GROUPBOX)
    {
        TCHAR c[64];
        RECT r;
        r.left = 9;
        r.top = 0;
        r.bottom = 20;
        r.right = 200;
        DWORD num = SendMessage(s->hwndItem, WM_GETTEXT, 64, (LPARAM)c);
        c[num] = 0;
        int m = SetBkMode(s->hDC, w.BkMode);
		COLORREF tx = SetTextColor(s->hDC, w.TextColor);
        DrawText(s->hDC, c, -1, &r, DT_SINGLELINE);
		SetBkMode(s->hDC, m);
		SetTextColor(s->hDC, tx);
    }

	return true;
}

int CSkinClass::GetBitmapType(DRAWITEMSTRUCT* s)
{
	map<HWND, SkinnedWnd>::iterator i = m_Windows.find(s->hwndItem);
	if(i == m_Windows.end())
		return 0;
	
	LRESULT r;

	WNDTYPE t = i->second.type;
	switch(t)
	{
	case WT_BTN:
		if(s->itemState & ODS_SELECTED)
			return BT_BTN_PRESSED;
		else
			return BT_BTN_NORMAL;
		break;
	case WT_CHKBOX:
		if(r = SendMessage(s->hwndItem, BM_GETCHECK, 0, 0))
			return BT_CHK_SEL;
		else
			return BT_CHK_NORMAL;
		break;
	default:
		return 0;
	}
	return 0;
}

bool CSkinClass::SetColor(HWND wnd, DWORD mode, DWORD Value)
{
	WNDTYPE type;
	map<HWND, SkinnedWnd>::iterator i;
	DWORD w = (DWORD)wnd;
	switch(w)
	{
	case (DWORD)SCWND_ALLBTNS:
		type = WT_BTN;
		break;
	case (DWORD)SCWND_ALLCHK:
		type = WT_CHKBOX;
		break;
	case (DWORD)SCWND_ALLLABELS:
		type = WT_LABEL;
		break;
	case (DWORD)SCWND_ALLEDITS:
		type = WT_EDIT;
		break;
	case (DWORD)SCWND_ALLGROUPBOXES:
		type = WT_GROUPBOX;
		break;
	default:
		i = m_Windows.find(wnd);
        type = i->second.type;
		if(i != m_Windows.end())
		{
			switch(mode)
			{
			case SCMODE_COLOR:
				m_Windows[wnd].TextColor = Value;
				break;
			case SCMODE_BKCOLOR:
				m_Windows[wnd].BkColor = Value;
				break;
			case SCMODE_BKMODE:
                if(type == WT_EDIT && Value == TRANSPARENT && m_ThemedApp)
                {
                    RECT client;
                    POINT p1, p2;
                    COLORREF col;
                    GetClientRect(m_Windows[wnd].wnd, &client);
                    p1.x = 0; p1.y = 0; p2.x = 0; p2.y = 0;
                    ClientToScreen(m_BGWnd, &p1);
                    ClientToScreen(m_Windows[wnd].wnd, &p2);
                    p1.x = p2.x - p1.x;
                    p1.y = p2.y - p1.y;
                    HDC hdc = CreateCompatibleDC(NULL);
                    SelectObject(hdc, (HGDIOBJ)m_Bitmaps[BT_BG].handle);
                    col = GetPixel(hdc, p1.x + client.left + 1, p1.y + client.top + 1);
                    DeleteDC(hdc);
                    i->second.BkColor = col;
                    i->second.BkMode = OPAQUE;
                    DeleteObject(m_Windows[wnd].UseBrush);
                    LOGBRUSH br;
                    br.lbStyle = BS_SOLID;
                    br.lbHatch = 0;
                    br.lbColor = col;
                    m_Windows[wnd].UseBrush = (HBRUSH)CreateBrushIndirect(&br);
                }
                else
                    m_Windows[wnd].BkMode = Value;
				break;
			case SCMODE_BRUSH:
				DeleteObject(m_Windows[wnd].UseBrush);
				m_Windows[wnd].UseBrush = (HBRUSH)CreateBrushIndirect((LOGBRUSH*)Value);
				break;
			default:
				return false;
				break;
			}
			return true;
		}
		return false;
		break;
	}
	i = m_Windows.begin();
	while(i != m_Windows.end())
	{
		if(i->second.type != type)
		{
			i++;
			continue;
		}
		switch(mode)
		{
		case SCMODE_COLOR:
			i->second.TextColor = Value;
			break;
		case SCMODE_BKCOLOR:
			i->second.BkColor = Value;
			break;
		case SCMODE_BKMODE:
            if(type == WT_EDIT && Value == TRANSPARENT && m_ThemedApp)
            {
                RECT client;
                POINT p1, p2;
                COLORREF col;
                GetClientRect(i->second.wnd, &client);
                p1.x = 0; p1.y = 0; p2.x = 0; p2.y = 0;
                ClientToScreen(m_BGWnd, &p1);
	            ClientToScreen(i->second.wnd, &p2);
	            p1.x = p2.x - p1.x;
	            p1.y = p2.y - p1.y;
                HDC hdc = CreateCompatibleDC(NULL);
                SelectObject(hdc, (HGDIOBJ)m_Bitmaps[BT_BG].handle);
                col = GetPixel(hdc, p1.x + client.left + 1, p1.y + client.top + 1);
                DeleteDC(hdc);
                i->second.BkColor = col;
                i->second.BkMode = OPAQUE;
                DeleteObject(i->second.UseBrush);
                LOGBRUSH br;
                br.lbStyle = BS_SOLID;
                br.lbHatch = 0;
                br.lbColor = col;
			    i->second.UseBrush = (HBRUSH)CreateBrushIndirect(&br);
            }
            else
			    i->second.BkMode = Value;
			break;
		case SCMODE_BRUSH:
			DeleteObject(i->second.UseBrush);
			i->second.UseBrush = (HBRUSH)CreateBrushIndirect((LOGBRUSH*)Value);
			break;
		default:
			return false;
			break;
		}
		i++;
	}
	return true;
}

bool CSkinClass::SetBtnTransparentColor(DWORD color)
{
    map<HWND, SkinnedWnd>::iterator i = m_Windows.begin();
    for (; i != m_Windows.end(); i++)
    {
        if (i->second.type == WT_BTN)
        {
            i->second.bTransColor = true;
            i->second.TransColor = color;
        }
    }
    return true;
}

bool CSkinClass::SetTransparentColor(HWND wnd, DWORD color)
{
    map<HWND, SkinnedWnd>::iterator i = m_Windows.find(wnd);
    if(i == m_Windows.end())
        return false;
    i->second.bTransColor = true;
    i->second.TransColor = color;
    return true;
}

//////////////////////////////////////////
// Bitmap functions
bool CSkinClass::SetBitmap(HBITMAP bmp, BMPTYPE type)
{
	if(!bmp)
		return false;

	if(m_Bitmaps[type].handle)
		DeleteObject(m_Bitmaps[type].handle);

	BMP b;
	b.handle = bmp;
	if(!GetObject(bmp, sizeof(BITMAP), (void*)&b.info))
		return false;

	if(type == BT_BG && m_BGWnd)
	{
		
		RECT r;
		GetClientRect(m_BGWnd,&r);
		m_BGImScaleX = float(r.right) / b.info.bmWidth;
		m_BGImScaleY = float(r.bottom) / b.info.bmHeight;
	}
	m_Bitmaps[type] = b;
	
	return true;
}

bool CSkinClass::SetFont(HWND wnd, LPCTSTR lpszName, INT iSize, WORD wFontFormat)
{
    // create font
    BOOL bBold, bIt, bUl, bStrikeout; 
    INT iWeight; 
    HFONT  hFont; 
    HDC hDC = GetDC(NULL);

    bBold = wFontFormat & FF_BOLD;
    bIt = wFontFormat & FF_ITALIC;
    bUl = wFontFormat & FF_UNDERLINE;
    bStrikeout = wFontFormat & FF_STRIKEOUT;
    iWeight = bBold ? FW_BOLD : FW_NORMAL;

    hFont = CreateFont(-MulDiv(iSize, GetDeviceCaps(hDC, LOGPIXELSY), 72),
                       0, 0, 0, iWeight, bIt, bUl, bStrikeout, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                       DEFAULT_PITCH, lpszName);
    ReleaseDC(NULL, hDC);
    if(!hFont)
        return false;
    
    WNDTYPE type;
	map<HWND, SkinnedWnd>::iterator i;
	DWORD w = (DWORD)wnd;
	switch(w)
	{
	case (DWORD)SCWND_ALLBTNS:
		type = WT_BTN;
		break;
	case (DWORD)SCWND_ALLCHK:
		type = WT_CHKBOX;
		break;
	case (DWORD)SCWND_ALLLABELS:
		type = WT_LABEL;
		break;
	case (DWORD)SCWND_ALLEDITS:
		type = WT_EDIT;
		break;
	case (DWORD)SCWND_ALLGROUPBOXES:
		type = WT_GROUPBOX;
		break;
	default:
		i = m_Windows.find(wnd);
        if(i != m_Windows.end()) {
            if(m_Windows[wnd].UseFont)
                DeleteObject(m_Windows[wnd].UseFont);
            m_Windows[wnd].UseFont = hFont;
            SendMessage(wnd, WM_SETFONT, (WPARAM)hFont, TRUE);
			return true;
        }
        return false;
	}
    i = m_Windows.begin();
	while(i != m_Windows.end())
	{
        if(i->second.type != type)
		{
			i++;
			continue;
		}
        if(i->second.UseFont)
            DeleteObject(i->second.UseFont);
        i->second.UseFont = hFont;
        SendMessage(i->second.wnd, WM_SETFONT, (WPARAM)hFont, TRUE);
		i++;
	}
	return true;
}


bool CSkinClass::LoadBitmap(TCHAR* name, BMPTYPE type)
{
	HBITMAP hBmp;
	if(!name)
		return false;
    if(type != BT_BG || !m_BGWnd)
	    hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    else
    {
        // scale to correct size
        RECT r;
        GetClientRect(m_BGWnd, &r);
        hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), name, IMAGE_BITMAP, r.right - r.left, r.bottom - r.top - m_iStatusBarHeight, LR_LOADFROMFILE);
    }
	if(!hBmp)
		return false;

	return SetBitmap(hBmp, type);
}

bool CSkinClass::LoadDataFile(TCHAR* name)
{
	if(!name)
		return false;
    
    m_DataFile = name;

	TCHAR path[MAX_PATH];
	DWORD num = GetCurrentDirectory(MAX_PATH, path);
	path[num] = 0;
	_tcscat(path, _T("\\"));
	_tcscat(path, name);
	
	TCHAR buffer[MAX_PATH];
	num = GetPrivateProfileString(_T("Bitmaps"), _T("Background"), _T(""), buffer, MAX_PATH, path);
	if(num)
		LoadBitmap(buffer, BT_BG);
	num = GetPrivateProfileString(_T("Bitmaps"), _T("Button_normal"), _T(""), buffer, MAX_PATH, path);
	if(num)
		LoadBitmap(buffer, BT_BTN_NORMAL);
	num = GetPrivateProfileString(_T("Bitmaps"), _T("Button_pressed"), _T(""), buffer, MAX_PATH, path);
	if(num)
		LoadBitmap(buffer, BT_BTN_PRESSED);
	num = GetPrivateProfileString(_T("Bitmaps"), _T("Checkbox_normal"), _T(""), buffer, MAX_PATH, path);
	if(num)
		LoadBitmap(buffer, BT_CHK_NORMAL);
	num = GetPrivateProfileString(_T("Bitmaps"), _T("Checkbox_selected"), _T(""), buffer, MAX_PATH, path);
	if(num)
		LoadBitmap(buffer, BT_CHK_SEL);

    m_OldBGWndSyle = GetWindowLongPtr(m_BGWnd, GWL_STYLE);

	// Soll caption angezeigt werden?
	num = GetPrivateProfileString(_T("General"), _T("showTitleBar"), _T("1"), buffer, MAX_PATH, path);
	buffer[num] = 0;
	if(!_ttoi(buffer))
		SetWindowLongPtr(m_BGWnd, GWL_STYLE, GetWindowLongPtr(m_BGWnd, GWL_STYLE) & ~(WS_CAPTION));
    
	SetWindowPos(m_BGWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

	return true;
}

LRESULT CALLBACK CSkinClass::SubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SkinnedWnd w;
    ZeroMemory(&w, sizeof(w));
    if(!ptr->m_Windows.size())
        return FALSE;
	map<HWND, SkinnedWnd>::iterator i = ptr->m_Windows.find((HWND)lParam);
	if(i != ptr->m_Windows.end())
	    w = i->second;

	switch(msg)
	{
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
        if(w.wnd)
        {
            SetBkColor((HDC)wParam, w.BkColor);
            SetBkMode((HDC)wParam, w.BkMode);
            SetTextColor((HDC)wParam, w.TextColor);
            return (LRESULT)w.UseBrush;
        }
        break;
	case WM_PAINT:
		ptr->DrawBGWindow(wnd);
        return 0;
		break;
	case WM_DRAWITEM:
		ptr->DrawWindow((DRAWITEMSTRUCT*)lParam);
        return TRUE;
		break;
	case WM_COMMAND:
		if(HIWORD(wParam) == EN_HSCROLL)
			InvalidateRect((HWND)lParam, NULL, TRUE);
		break;
	}
	
	return CallWindowProc(ptr->m_OldWndProc, wnd, msg, wParam, lParam);
}

LRESULT CALLBACK CSkinClass::EditSubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	map<HWND, SkinnedWnd>::iterator i = ptr->m_Windows.find(wnd);

	int r;
    WNDPROC owp = i->second.oldWndProc;
    RECT r2;
    DRAWITEMSTRUCT s;
    s.hwndItem = wnd;

    if(ptr->m_ThemedApp)
        return CallWindowProc(owp, wnd, msg, wParam, lParam);

	switch(msg)
	{
    case WM_KEYDOWN:
        s.hDC = GetDC(wnd);
		GetWindowRect(wnd, &r2);
        s.rcItem = r2;
        ptr->DrawWndBG(&s);
        r = CallWindowProc(owp, wnd, msg, wParam, lParam);
        ReleaseDC(wnd, s.hDC);
        InvalidateRect(wnd, NULL, TRUE);
		return r;
        break;
    case WM_ERASEBKGND:
        s.hDC = GetDC(wnd);
        GetWindowRect(wnd, &r2);
        s.rcItem = r2;
	    ptr->DrawWndBG(&s);
		ReleaseDC(wnd, s.hDC);
        return 1;
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_KILLFOCUS:
    case WM_VSCROLL:
        InvalidateRect(wnd, NULL, TRUE);
        break;
	case WM_PAINT:
        s.hDC = GetDC(wnd);
        GetWindowRect(wnd, &r2);
        s.rcItem = r2;
	    ptr->DrawWndBG(&s);
		ReleaseDC(wnd, s.hDC);
		break;
   	}
	return CallWindowProc(owp, wnd, msg, wParam, lParam);
}

LRESULT CALLBACK CSkinClass::GroupBoxSubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	map<HWND, SkinnedWnd>::iterator i = ptr->m_Windows.find(wnd);

    WNDPROC owp = i->second.oldWndProc;
	PAINTSTRUCT ps;
	DRAWITEMSTRUCT s;

	if(msg == WM_PAINT)
	{
		BeginPaint(wnd, &ps);
		s.hwndItem = wnd;
		s.hDC = ps.hdc;
		ptr->DrawWindow(&s);
		EndPaint(wnd, &ps);
		return 0;
	}
    if(msg == WM_ENABLE)
    {
        return 0;
    }

	return CallWindowProc(owp, wnd, msg, wParam, lParam);
}

LRESULT CALLBACK CSkinClass::LabelSubClassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	map<HWND, SkinnedWnd>::iterator i = ptr->m_Windows.find(wnd);

    WNDPROC owp = i->second.oldWndProc;
	DRAWITEMSTRUCT s;
	
    switch(msg)
    {
    case WM_SETTEXT:
        s.hwndItem = wnd;
		s.hDC = GetDC(wnd);
		ptr->DrawWndBG(&s);
		ReleaseDC(wnd, s.hDC);
    	break;
    case WM_VSCROLL:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        InvalidateRect(wnd, NULL, TRUE);
    	break;
    case WM_PAINT:
        DRAWITEMSTRUCT s;
		s.hDC = GetDC(wnd);
		s.hwndItem = wnd;
		ptr->DrawWndBG(&s);
		ReleaseDC(wnd, s.hDC);
		InvalidateRect(wnd, NULL, TRUE);
        break;
    case WM_ENABLE:
        return 0;
        break;
    default:
        break;
    }
	
    return CallWindowProc(owp, wnd, msg, wParam, lParam);
}

const TCHAR* CSkinClass::GetCurrentDataFile()
{
    return m_DataFile.c_str();
}

// from http://www.ddj.com/dept/windows/184416353
bool CSkinClass::TransparentBltU(HDC dcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, 
                                 HDC dcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, 
                                 UINT crTransparent)
{
     if (nWidthDest < 1) return false;
     if (nWidthSrc < 1) return false; 
     if (nHeightDest < 1) return false;
     if (nHeightSrc < 1) return false;

     HDC dc = CreateCompatibleDC(NULL);
     HBITMAP bitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, GetDeviceCaps(dc, BITSPIXEL), NULL);

     if (bitmap == NULL)
         return false;

     HBITMAP oldBitmap = (HBITMAP)SelectObject(dc, bitmap);

     if (!BitBlt(dc, 0, 0, nWidthSrc, nHeightSrc, dcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY))
         return false;

     HDC maskDC = CreateCompatibleDC(NULL);
     HBITMAP maskBitmap = CreateBitmap(nWidthSrc, nHeightSrc, 1, 1, NULL);

     if (maskBitmap == NULL)
         return false;

     HBITMAP oldMask = (HBITMAP)SelectObject(maskDC, maskBitmap);
 
     SetBkColor(maskDC, RGB(0,0,0));
     SetTextColor(maskDC, RGB(255,255,255));
     if (!BitBlt(maskDC, 0, 0, nWidthSrc, nHeightSrc, NULL, 0,0,BLACKNESS))
         return false;

     SetBkColor(dc, crTransparent);
     BitBlt(maskDC, 0,0,nWidthSrc,nHeightSrc,dc,0,0,SRCINVERT);

     SetBkColor(dc, RGB(0,0,0));
     SetTextColor(dc, RGB(255,255,255));
     BitBlt(dc, 0,0,nWidthSrc,nHeightSrc,maskDC,0,0,SRCAND); 

     HDC newMaskDC = CreateCompatibleDC(NULL);
     HBITMAP newMask;
     newMask = CreateBitmap(nWidthDest, nHeightDest, 1, GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);

     if (newMask == NULL)
     {
         SelectObject(dc, oldBitmap);
         DeleteDC(dc);
         SelectObject(maskDC, oldMask);
         DeleteDC(maskDC);
         DeleteDC(newMaskDC);
         return false;
     }

     SetStretchBltMode(newMaskDC, COLORONCOLOR);
     HBITMAP oldNewMask = (HBITMAP) SelectObject(newMaskDC, newMask);
     StretchBlt(newMaskDC, 0, 0, nWidthDest, nHeightDest, maskDC, 0, 0, nWidthSrc, nHeightSrc, SRCCOPY);

     SelectObject(maskDC, oldMask);
     DeleteDC(maskDC);
    
     HDC newImageDC = CreateCompatibleDC(NULL);
     HBITMAP newImage = CreateBitmap(nWidthDest, nHeightDest, 1, GetDeviceCaps(newMaskDC, BITSPIXEL), NULL);

     if (newImage == NULL)
     {
         SelectObject(dc, oldBitmap);
         DeleteDC(dc);
         DeleteDC(newMaskDC);
         return false;
     }

     HBITMAP oldNewImage = (HBITMAP)SelectObject(newImageDC, newImage);
     StretchBlt(newImageDC, 0, 0, nWidthDest, nHeightDest, dc, 0, 0, nWidthSrc, nHeightSrc, SRCCOPY);

     SelectObject(dc, oldBitmap);
     DeleteDC(dc);

     BitBlt(dcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, newMaskDC, 0, 0, SRCAND);

     BitBlt(dcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, newImageDC, 0, 0, SRCPAINT);

     SelectObject(newImageDC, oldNewImage);
     DeleteDC(newImageDC);
     SelectObject(newMaskDC, oldNewMask);
     DeleteDC(newMaskDC);

     return true;
}

void CSkinClass::DeactivateTheme(HWND wnd, bool deactivate)
{
    if(!m_ThemedApp)
        return;
    if(deactivate)
        m_pSetWindowTheme(wnd, L" ", L" ");
    else
        m_pSetWindowTheme(wnd, NULL, NULL);
}
