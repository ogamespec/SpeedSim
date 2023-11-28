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

#include "SpeedSim.h"


DLUInfo g_DLUData;
vector<EspHistItem> g_vRaidHistory;

vector<sRaidListItem> g_vRaidList;
map<UINT, HWND> g_mTooltipWnd;
int g_iCurRaidlist = -1;
HWND g_hwndStatusBar = NULL;
WNDPROC OldLVProc = NULL;

void AddFleetToRaidlist()
{
	UpdateKernel();
	sRaidListItem rli;
	int iFleetID = 0;
	// get all acs fleets
	SItem att[T_END], def[T_END];
	for(iFleetID = 0; iFleetID < MAX_PLAYERS_PER_TEAM; iFleetID++)
	{
		sim.GetSetFleet(att, def, iFleetID);
		for(int i = 0; i < T_END; i++) {
			if(att[i].Num)
				rli.vAttItems[iFleetID].push_back(att[i]);
			if(def[i].Num)
				rli.vDefItems[iFleetID].push_back(def[i]);
		}
		sim.GetTechs(&rli.TechsAtt[iFleetID], &rli.TechsDef[iFleetID], iFleetID);
		sim.GetOwnPosition(rli.StartPos[iFleetID], iFleetID);
	}

	rli.battle_res = sim.GetBattleResult();
	for(iFleetID = 0; iFleetID < MAX_PLAYERS_PER_TEAM; iFleetID++) {
		sim.GetFleetAfterSim(rli.AttResult[iFleetID], rli.DefResult[iFleetID], iFleetID);
	}

	rli.TargetPos = sim.GetBattleResult().Position;
	rli.iUsedFleet = ReadFromEdit(IDC_FLEET_ID) - 1;
	
	g_vRaidList.push_back(rli);
	if(g_vRaidList.size() >= MAX_RAIDLIST_FLEETS) {
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_ADD_TARGETINFO), false);
		//g_vRaidList.erase(g_vRaidList.begin());
	}
	g_iCurRaidlist = g_vRaidList.size() - 1;
	UpdateRaidlistInfo();
}

bool RemoveCurFromRaidlist()
{
	if(g_vRaidList.size() < g_iCurRaidlist || g_vRaidList.size() == 0) {
		UpdateRaidlistInfo();
		return false;
	}
	vector<sRaidListItem>::iterator it;
	it = g_vRaidList.begin() + g_iCurRaidlist;
	g_vRaidList.erase(it);
	g_iCurRaidlist--;
	if(g_vRaidList.size() > 0 && g_iCurRaidlist < 0)
		g_iCurRaidlist++;
	if(g_vRaidList.size() < MAX_RAIDLIST_FLEETS)
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_ADD_TARGETINFO), true);
	SetRaidlistFleet(g_iCurRaidlist);
	return true;
}

bool SetRaidlistFleet(int iID)
{
	if(iID > g_vRaidList.size() - 1 || g_vRaidList.size() == 0) {
		UpdateRaidlistInfo();
		return false;
	}
	int iFleetID = 0;

	for(iFleetID = 0; iFleetID < MAX_PLAYERS_PER_TEAM; iFleetID++) {
		// add dummy item
		SItem it;
		it.Num = 0;
		it.OwnerID = iFleetID;
		it.Type = T_NONE;
		// restore fleet information
		g_vRaidList[iID].vDefItems[iFleetID].push_back(it);
		g_vRaidList[iID].vAttItems[iFleetID].push_back(it);
		sim.SetFleet(&g_vRaidList[iID].vAttItems[iFleetID], &g_vRaidList[iID].vDefItems[iFleetID]);
		sim.SetTechs(&g_vRaidList[iID].TechsAtt[iFleetID], &g_vRaidList[iID].TechsDef[iFleetID], iFleetID);
		sim.SetOwnPosition(g_vRaidList[iID].StartPos[iFleetID], iFleetID);
	}
	iFleetID = ReadFromEdit(IDC_FLEET_ID)-1;
	TargetInfo ti = sim.GetTargetInfo(iFleetID);
	ti.Pos = g_vRaidList[iID].TargetPos;
	sim.SetTargetInfo(ti, iFleetID);
	
	g_iCurRaidlist = iID;
	UpdateRaidlistInfo();
	UpdateEditFields(true, true);
	iFleetID = g_vRaidList[iID].iUsedFleet;
	ShowFightResults(g_vRaidList[iID].battle_res, g_vRaidList[iID].AttResult[iFleetID], g_vRaidList[iID].DefResult[iFleetID], false);
	return true;
}

void SetNextRaidlistFleet()
{
	if(g_iCurRaidlist < g_vRaidList.size())
		SetRaidlistFleet(g_iCurRaidlist + 1);
	else
		SetRaidlistFleet(g_iCurRaidlist);
}

void SetPrevRaidListFleet()
{
	if(g_iCurRaidlist > 0)
		SetRaidlistFleet(g_iCurRaidlist - 1);
	else
		SetRaidlistFleet(g_iCurRaidlist);
}

void UpdateRaidlistInfo()
{
	TCHAR c[256];
	_stprintf(c, _T("%d / %d"), g_iCurRaidlist + 1, g_vRaidList.size());
	SetDlgItemText(g_hwndDlg, IDC_RAIDLISTINFO, c);
}

// code by www.catch22.net
void ForceSetForegroundWindow(HWND hwnd)
{
	DWORD dwFGThreadId, dwFGProcessId;
	DWORD dwThisThreadId;
	
	HWND hwndForeground = GetForegroundWindow();
	
	dwFGThreadId = GetWindowThreadProcessId(hwndForeground, &dwFGProcessId);
	
	dwThisThreadId = GetCurrentThreadId();
	
	AttachThreadInput(dwThisThreadId, dwFGThreadId, TRUE);

	SetForegroundWindow(hwnd);
	BringWindowToTop(hwnd);
	SetFocus(hwnd);
	
	AttachThreadInput(dwThisThreadId, dwFGThreadId, FALSE);
}

HFONT CreateFont(LPCTSTR lpszName, INT iSize, WORD wFontFormat) {
	// create font
	BOOL bBold, bIt, bUl, bStrikeout;
	INT iWeight;
	HDC hDC = GetDC(NULL);
	HFONT hFont;

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
	
	return hFont;
}

bool CreateSystrayIcon(bool bCreate) {
	TCHAR szTip[256];
	_tcscpy(szTip, g_TrayText[0].c_str());
	if(g_Options.HookClipboard)
		_tcscat(szTip, g_TrayText[2].c_str());
	else
		_tcscat(szTip, g_TrayText[3].c_str());
	_tcscat(szTip, _T("\n"));
	_tcscat(szTip, g_TrayText[1].c_str());
	if(g_Options.bPopUpOnText)
		_tcscat(szTip, g_TrayText[2].c_str());
	else
		_tcscat(szTip, g_TrayText[3].c_str());
	NOTIFYICONDATA TrayIcon;
	TrayIcon.cbSize = sizeof(TrayIcon);
	TrayIcon.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));
	TrayIcon.hWnd = g_hwndDlg;
	// cut the string down, if too long for systray text
	if(_tcslen(szTip) > 63)
		szTip[63] = '\0';
	_tcscpy(TrayIcon.szTip, szTip);
	TrayIcon.uCallbackMessage = TRAY_MESSAGE;
	TrayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	TrayIcon.uID = 0x1;

	if(bCreate) {
		Shell_NotifyIcon(NIM_ADD, &TrayIcon);
	}
	else
		Shell_NotifyIcon(NIM_DELETE, &TrayIcon);
	return true;
}

bool CreatePopupInfo(BattleResult& br, vector<SItem> AttFleet, int iRaidlistItem) {
	// create popwindow with information for attacker
	int xPos, yPos;
	DEVMODE dev;
	UpdateKernel();
	// destroy old popup
	if(g_PopupShown && g_hwndPopup)
		DestroyWindow(g_hwndPopup);
	dev.dmSize = sizeof(DEVMODE);
	dev.dmDriverExtra = 0;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dev);
	
	TCHAR text[512];
	TCHAR tmp[512];
	TCHAR pos[50];
	ZeroMemory(text, 512);
		
	GetDlgItemText(g_hwndDlg, IDC_OWN_POS, tmp, 20);
	if(br.Position.bMoon)
		_tcscpy(text, g_PopupStrings[4].c_str());
	else
		_tcscpy(text, _T(""));
	_stprintf(pos, _T("[%d:%d:%d%s]"), br.Position.Gala, br.Position.Sys, br.Position.Pos, text);
	_stprintf(text, _T("%s: [%s] | %s: %s"), g_PopupStrings[0].c_str(), tmp, g_PopupStrings[1].c_str(), pos);
	_tcscat(text, _T("\n~~~~~~~~~~~~~~~~~~\n"));
	int lines = 2;
	for(int i = 0; i < AttFleet.size(); i++) {
		TCHAR name[50];
		sim.ItemName(AttFleet[i].Type, name);
		if(AttFleet[i].Num > 0 && AttFleet[i].Type != T_NONE) {
			TCHAR num[20];
			AddPointsToNumber(AttFleet[i].Num, num);
			_stprintf(tmp, _T("%s%s: %s\n"), text, name, num);
			_tcscpy(text, tmp);
			lines++;
		}
	}
	if(br.MaxNumRecs) {
		_stprintf(tmp, _T("\n%s: %d\n"), g_PopupStrings[3].c_str(), br.MaxNumRecs);
		_tcscat(text, tmp);
		lines += 2;
	}
	_tcscat(text, _T("\n"));
	_tcscat(text, g_PopupStrings[2].c_str());
	if(iRaidlistItem > 0) {
		_stprintf(tmp, _T("  [%d / %d]"), iRaidlistItem, g_vRaidList.size());
		_tcscat(text, tmp);
	}
	lines++;
	
	g_hwndPopup = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_POPUP), NULL, PopupInfoProc, (LPARAM)text);
	SetWindowLongPtr(g_hwndPopup, GWL_STYLE, WS_VISIBLE);
	
	RECT r;
	GetWindowRect(g_hwndPopup, &r);
	// set the height of the window (border + 9 pixel per line + space between lines)
	r.bottom = 23 + 9 * lines + 4 * lines;
	r.right = 190;
	xPos = dev.dmPelsWidth - r.right - 5;
	yPos = dev.dmPelsHeight - r.bottom - 40;
	InvalidateRect(g_hwndPopup, NULL, TRUE);
	SetWindowPos(g_hwndPopup, HWND_TOP, xPos, yPos, r.right, r.bottom, SWP_FRAMECHANGED);
	g_PopupShown = true;
	return true;
}

HWND CreateTooltip(HWND hwnd, TCHAR* szTTText, UINT uTTID)
{
	HWND hwndTT;
	TOOLINFO ti;
	RECT rect;

	hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInst, NULL);

	GetClientRect(hwnd, &rect);
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwnd;
	ti.hinst = g_hInst;
	ti.uId = uTTID;
	ti.lpszText = szTTText;
	
	// ToolTip control will cover the whole window
	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
	return hwndTT;
}

void UpdateTooltip(HWND hwnd, HWND hwndTT, const TCHAR* szTTText, UINT uTTID)
{
	TOOLINFO ti;
	
	if(!hwnd || !hwndTT)
		return;
	
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwnd;
	ti.hinst = g_hInst;
	ti.uId = uTTID;
	ti.lpszText = (TCHAR*)szTTText;
	SendMessage(hwndTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM)TTDT_INITIAL, (LPARAM)80);
}

void CreateTooltips()
{
	TCHAR szText[] = _T("Tooltip");
	HWND wnd;

	wnd = GetDlgItem(g_hwndDlg, IDC_TF);
	g_mTooltipWnd[TT_ID_DEBRIS] = CreateTooltip(wnd, szText, TT_ID_DEBRIS);
	SendMessage(g_mTooltipWnd[TT_ID_DEBRIS], TTM_SETMAXTIPWIDTH, 0, 500);
	
	wnd = GetDlgItem(g_hwndDlg, IDC_VERL_A);
	g_mTooltipWnd[TT_ID_LOSSES_ATT] = CreateTooltip(wnd, szText, TT_ID_LOSSES_ATT);
	
	wnd = GetDlgItem(g_hwndDlg, IDC_VERL_V);
	g_mTooltipWnd[TT_ID_LOSSES_DEF] = CreateTooltip(wnd, szText, TT_ID_LOSSES_DEF);
	
	wnd = GetDlgItem(g_hwndDlg, IDC_BEUT_GES);
	g_mTooltipWnd[TT_ID_WAVES] = CreateTooltip(wnd, szText, TT_ID_WAVES);
	
	wnd = GetDlgItem(g_hwndDlg, IDC_BEUT_TH);
	g_mTooltipWnd[TT_ID_TH_PLUNDER] = CreateTooltip(wnd, szText, TT_ID_TH_PLUNDER);
	SendMessage(g_mTooltipWnd[TT_ID_TH_PLUNDER], TTM_SETMAXTIPWIDTH, 0, 500);

	wnd = GetDlgItem(g_hwndDlg, IDC_IS_MOON);
	g_mTooltipWnd[TT_ID_MOON] = CreateTooltip(wnd, (TCHAR*)g_ScanEditStr[4].c_str(), TT_ID_MOON);
}

void UpdateTooltips()
{
	TCHAR szText[512];
	TCHAR szTmp[512];

	GetDlgItemText(g_hwndDlg, IDC_TF, szText, 512);
	// recycler info
	BattleResult r = sim.GetBattleResult();
	_tcscat(szText, _T("\r\n")); 
	_stprintf(szTmp, g_ResString[14].c_str(), r.MaxNumRecs, r.NumRecs, r.MinNumRecs);
	_tcscat(szText, szTmp);
	UpdateTooltip(GetDlgItem(g_hwndDlg, IDC_TF), g_mTooltipWnd[TT_ID_DEBRIS], szText, TT_ID_DEBRIS);

	GetDlgItemText(g_hwndDlg, IDC_VERL_A, szText, 512);
	UpdateTooltip(GetDlgItem(g_hwndDlg, IDC_VERL_A), g_mTooltipWnd[TT_ID_LOSSES_ATT], szText, TT_ID_LOSSES_ATT);

	GetDlgItemText(g_hwndDlg, IDC_VERL_V, szText, 512);
	UpdateTooltip(GetDlgItem(g_hwndDlg, IDC_VERL_V), g_mTooltipWnd[TT_ID_LOSSES_DEF], szText, TT_ID_LOSSES_DEF);
	
	GetDlgItemText(g_hwndDlg, IDC_BEUT_GES, szText, 512);
	UpdateTooltip(GetDlgItem(g_hwndDlg, IDC_BEUT_GES), g_mTooltipWnd[TT_ID_WAVES], szText, TT_ID_WAVES);

	UpdateTooltip(GetDlgItem(g_hwndDlg, IDC_IS_MOON), g_mTooltipWnd[TT_ID_MOON], g_ScanEditStr[4].c_str(), TT_ID_MOON);

	// create info for theoretic plunder
	TCHAR szShip[128];
	sim.ItemName(T_KT, szShip);
	_stprintf(szTmp, _T("%s: %d\r\n"), szShip, g_iNumShipsForPlunderNeeded[T_KT]);
	_tcscpy(szText, szTmp);
	
	sim.ItemName(T_GT, szShip);
	_stprintf(szTmp, _T("%s: %d\r\n"), szShip, g_iNumShipsForPlunderNeeded[T_GT]);
	_tcscat(szText, szTmp);

	/*sim.ItemName(T_SPIO, szShip);
	_stprintf(szTmp, _T("%s: %d\r\n"), szShip, g_iNumShipsForPlunderNeeded[T_SPIO]);
	_tcscat(szText, szTmp);*/
	
	sim.ItemName(T_SS, szShip);
	_stprintf(szTmp, _T("%s: %d\r\n"), szShip, g_iNumShipsForPlunderNeeded[T_SS]);
	_tcscat(szText, szTmp);
	
	sim.ItemName(T_ZER, szShip);
	_stprintf(szTmp, _T("%s: %d\r\n"), szShip, g_iNumShipsForPlunderNeeded[T_ZER]);
	_tcscat(szText, szTmp);

	sim.ItemName(T_IC, szShip);
	_stprintf(szTmp, _T("%s: %d"), szShip, g_iNumShipsForPlunderNeeded[T_IC]);
	_tcscat(szText, szTmp);

	UpdateTooltip(GetDlgItem(g_hwndDlg, IDC_BEUT_TH), g_mTooltipWnd[TT_ID_TH_PLUNDER], szText, TT_ID_TH_PLUNDER);
}

void OpenURL(TCHAR* url) {
	HKEY hKey;
	TCHAR name[128];
	DWORD size = 128;
	// check, if IE
	RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("http\\shell\\open\\ddeexec\\Application"), 0, KEY_QUERY_VALUE, &hKey);
	RegQueryValueEx(hKey, _T(""), 0, 0, (LPBYTE)name, &size);
	RegCloseKey(hKey);
	if(!_tcsnicmp(name, _T("IExplore"), 8)) {
		ShellExecute(NULL, _T("open"), _T("about:blank"), NULL, NULL, SW_SHOWNORMAL);
	}
	ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOWNORMAL);
}

COLORREF StrToRGB(TCHAR *str) {
	int r = 0, g = 0, b = 0;
	genstring s = str;
	genstring num;
	genstring::size_type f = 0, f2 = 0;
	
	f = s.find(_T(","));
	num = s.substr(0);
	r = _ttoi(num.c_str());
	f2 = s.find(_T(","), f+1) + 1;
	num = s.substr(f+1);
	g = _ttoi(num.c_str());
	f = f2;
	f2 = s.length();
	num = s.substr(f);
	b = _ttoi(num.c_str());
	return RGB(r, g, b);
}

void ShowSkinInfo(TCHAR *inipath) {
	TCHAR buffer[512];
	genstring str;
	genstring::size_type f = 0;
	if(!GetPrivateProfileString(_T("General"), _T("SkinDesc"), _T(""), buffer, 512, inipath ))
		return;
	str = buffer;
	while(f != string::npos) {
		f = str.find(_T("§"), f);
		if(f != string::npos)
			str[f] = '\n';
	}
	MessageBox(g_hwndDlg, str.c_str(), _T("Skin-Info"), MB_OK);
}

void ShowBilanz()
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_BALANCE), g_hwndDlg, BalanceProc);
}

bool IsCharNum(TCHAR c)
{
  if((IsCharAlphaNumeric(c)==1) && (IsCharAlpha(c)==0))
	return true;
  else
	return false;
}

int ReadFromEdit(int ResID) {
	return GetDlgItemInt(g_hwndDlg, ResID, NULL, false);
}

bool SetWinStartUp(bool bCreate) {
	TCHAR file[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), file, MAX_PATH);
	
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return false;
	if(bCreate)
		RegSetValueEx(hKey, _T("SpeedSim"), 0, REG_SZ, (LPBYTE)file, _tcslen(file));
	else
		RegDeleteValue(hKey, _T("SpeedSim"));
	RegCloseKey(hKey);
	return true;
}

bool IsInStartup() {
	HKEY hKey;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &hKey);
	long res = RegQueryValueEx(hKey, _T("SpeedSim"), 0, NULL, NULL, NULL);
	RegCloseKey(hKey);
	return res == ERROR_SUCCESS;
}

BOOL CALLBACK WndEnumProc(HWND wnd, LPARAM lParam) {
	SendMessage(wnd, WM_SETFONT, lParam, 0);
	return TRUE;
}

bool SetFontForAllChilds(HWND wnd, HFONT hFont) {
	if(!hFont)
		return false;
	SendMessage(wnd, WM_SETFONT, (WPARAM)hFont, 0);
	EnumChildWindows(wnd, WndEnumProc, (LPARAM)hFont);
	return true;
}

genstring InputBox(genstring Text) {
	static int count = 0;
	if(!count) {
		count++;
		DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_INPUTBOX), g_hwndDlg, InputBoxProc, (LPARAM)&Text);
		count--;
	}
	else
		return _T("");
	return Text;
}

int CALLBACK FontFam(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	if(lpelfe && lpntme)
		return 0;
	return 1;
}

bool CheckFont(TCHAR* font) {
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy(&lf.lfFaceName[0], font);
	lf.lfPitchAndFamily = 0;
	HDC hDC = GetDC(NULL);
	int res = EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)FontFam, 0, 0);
	ReleaseDC(NULL, hDC);
	return (res != 1);
}

bool IsLanguageFile(TCHAR* IniFileName) {
	bool res = true;
	TCHAR buff[256];
	TCHAR inipath[MAX_PATH];
	_tcscpy(inipath, g_CurrDir);
	_tcscat(inipath, IniFileName);
	if(!GetPrivateProfileString(_T("General"), _T("ATT"), _T(""), buff, 256, inipath))
		res = false;
	if(!GetPrivateProfileString(_T("General"), _T("DEF"), _T(""), buff, 256, inipath))
		res = false;
	if(!GetPrivateProfileString(_T("Fleet"), _T("REC"), _T(""), buff, 256, inipath))
		res = false;
	return res;
}

bool IsSkinFile(TCHAR* IniFileName) {
	bool res = true;
	TCHAR buff[256];
	TCHAR inipath[MAX_PATH];
	_tcscpy(inipath, g_CurrDir);
	_tcscat(inipath, IniFileName);
	if(!GetPrivateProfileString(_T("Bitmaps"), _T("Background"), _T(""), buff, 256, inipath))
		res = false;
	return res;
}


typedef BOOL (WINAPI *LWFCT)(HWND, COLORREF, BYTE, DWORD);

#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

bool TransparentWindow(HWND hWnd, BYTE iFactor)
{
	static LWFCT pSLWA = NULL;
	static bool bIsInit = false;
	
	if (!bIsInit)
	{
		// get function
		pSLWA = (LWFCT)GetProcAddress(g_hUser32DLL, "SetLayeredWindowAttributes");
		bIsInit = true;
	}
	if(!pSLWA)
		return false;

	// set layered style
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	return (pSLWA(hWnd, 0, 255 - (BYTE)(255.f * iFactor / 100.f) , LWA_ALPHA) == TRUE);
}

void SetSkin(TCHAR *szSkinFile)
{    
	const TCHAR *szCurFile = g_Skin.GetCurrentDataFile();
	// if same skin, do not set skin
	if(!_tcsncmp(szCurFile, szSkinFile, _tcslen(szSkinFile)))
	{
		if(_tcslen(szCurFile) == _tcslen(szSkinFile))
			return;
	}
	if(_tcslen(szCurFile))
		g_Skin.Free();
	if(!szSkinFile || !_tcslen(szSkinFile))
	{
		TCHAR css1[MAX_PATH], css2[MAX_PATH];
		_tcsncpy(css1, g_CurrDir, MAX_PATH);
		_tcsncpy(css2, g_CurrDir, MAX_PATH);
		_tcscat(css1, _T("cr.css"));
		_tcscat(css2, _T("bw.css"));
		//sim.SetCSSFiles(NULL, NULL);
		sim.SetCSSFiles(css1, css2);
		ActivateIPMMode(g_IPMMode);
		return;
	}
	if(g_IPMMode)
	{
		EnableFleet(true);
		SendDlgItemMessage(g_hwndDlg, IDC_NUM_SIM, EM_SETREADONLY, FALSE, 0);
		SendDlgItemMessage(g_hwndDlg, IDC_FLEET_ID, EM_SETREADONLY, FALSE, 0);
	}
	g_Skin.AddWindow(g_hwndDlg, WT_BG);
	g_Skin.LoadDataFile(szSkinFile);
	g_Skin.AddAllChildWindows(g_hwndDlg);
	ReadSkinIni(szSkinFile);
	ActivateIPMMode(g_IPMMode);
}

void CreateStatusBar()
{
	if(g_hwndStatusBar)
		return;
	
	HWND hwndStatus = CreateStatusWindow(WS_CHILD|WS_VISIBLE, _T(""), g_hwndDlg, ID_STATUSBAR);
	if(!hwndStatus)
		return;
	// get height and add to window size
	RECT r, wr;
	GetWindowRect(g_hwndDlg, &wr);
	SendMessage(hwndStatus, SB_GETRECT, (WPARAM)0, (LPARAM)&r);
	wr.bottom += r.bottom - r.top;
	SetWindowPos(g_hwndDlg, NULL, 0, 0, wr.right - wr.left, wr.bottom - wr.top, SWP_NOZORDER|SWP_NOMOVE);
	// adjust to correct position
	SendMessage(hwndStatus, WM_SIZE, (WPARAM)SIZE_RESTORED, (LPARAM)MAKELPARAM(wr.right - wr.left, wr.bottom - wr.top));
	if(_tcslen(g_Options.SkinFileName))
		g_Skin.AddWindow(hwndStatus, WT_STATUSBAR);
	g_hwndStatusBar = hwndStatus;
	UpdateStatusBar();
}


// in development
void UpdateStatusBar()
{
	RECT r;
	SendMessage(g_hwndStatusBar, SB_GETRECT, (WPARAM)0, (LPARAM)&r);
	int xPos[] = {150, r.right - 40, r.right - 20, r.right};
	SendMessage(g_hwndStatusBar, SB_SETPARTS, (WPARAM)4, (LPARAM)&xPos);
	SendMessage(g_hwndStatusBar, SB_GETRECT, (WPARAM)3, (LPARAM)&r);
	SendMessage(g_hwndStatusBar, SB_SETICON, 3, (LPARAM)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, r.right-r.left, r.bottom-r.top-2, LR_DEFAULTCOLOR));
}

void ActivateIPMMode(bool set_active)
{
	
	UpdateKernel();
	int i;
	// hide wave buttons
	ShowWindow(GetDlgItem(g_hwndDlg, IDC_DELWAVESTATE), set_active ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(g_hwndDlg, IDC_LOAD_REM_DEF), set_active ? SW_HIDE : SW_SHOW);
	
	HMENU hMenu = GetMenu(g_hwndDlg);
	if(set_active)
	{
		ModifyMenu(hMenu, IDM_IPMMODE, MF_BYCOMMAND|MF_STRING, IDM_IPMMODE, g_IPModeStr[3].c_str());
		SetDlgItemInt(g_hwndDlg, IDC_FLEET_ID, 1, FALSE);
		EnableFleet(false);
		SendDlgItemMessage(g_hwndDlg, IDC_FLEET_ID, EM_SETREADONLY, TRUE, 0);
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_NEXT_FL), false);
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_PREV_FL), false);
		SendDlgItemMessage(g_hwndDlg, IDC_NUM_SIM, EM_SETREADONLY, TRUE, 0);
		
		for(i = 0; i < T_END - T_SHIPEND; i++)
		{
			ShowWindow(GetDlgItem(g_hwndDlg, ID_FIRST_RADIO_IPM + i), SW_SHOW);
		}
		SetWindowText(GetDlgItem(g_hwndDlg, IDC_ATT2), g_IPModeStr[0].c_str());
		SetWindowText(GetDlgItem(g_hwndDlg, ID_NAME_IPM), g_IPModeStr[1].c_str());

		ShowWindow(GetDlgItem(g_hwndDlg, ID_NAME_IPM), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_INPUT_IPM), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_INPUT_ABM), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_IPM_ARROW), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_ABM_ARROW), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_OUTPUT_IPM), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_OUTPUT_ABM), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_TAKE_OVER_IPM_RES), SW_SHOW);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_OUTPUT_NEEDED_IPM), SW_SHOW);
	}
	else
	{
		SendDlgItemMessage(g_hwndDlg, IDC_FLEET_ID, EM_SETREADONLY, FALSE, 0);
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_NEXT_FL), true);
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_PREV_FL), true);
		SendDlgItemMessage(g_hwndDlg, IDC_NUM_SIM, EM_SETREADONLY, FALSE, 0);
		EnableFleet(true);
		ModifyMenu(hMenu, IDM_IPMMODE, MF_BYCOMMAND|MF_STRING, IDM_IPMMODE, g_IPModeStr[4].c_str());
		for(i = 0; i < T_END - T_SHIPEND; i++)
		{
			ShowWindow(GetDlgItem(g_hwndDlg, ID_FIRST_RADIO_IPM + i), SW_HIDE);
		}
		TCHAR* txt[50];
		GetDlgItemText(g_hwndDlg, IDC_ATT1, (TCHAR*)txt, 50);
		SetWindowText(GetDlgItem(g_hwndDlg, IDC_ATT2), (TCHAR*)txt);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_NAME_IPM), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_INPUT_IPM), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_INPUT_ABM), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_IPM_ARROW), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_ABM_ARROW), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_OUTPUT_IPM), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_OUTPUT_ABM), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_TAKE_OVER_IPM_RES), SW_HIDE);
		ShowWindow(GetDlgItem(g_hwndDlg, ID_OUTPUT_NEEDED_IPM), SW_HIDE);
	}
	DrawMenuBar(g_hwndDlg);
}

void CreateIPMWindows()
{
	int i, xl, yl;
	float size_fac = (float)g_DLUData.xPx / 8.f;
	RECT r;

	int ydiff = 0, y0, x0, x02;
	// get y-diff between names
	POINT p1, p2;
	p1.x = p1.y = p2.x = p2.y = 0;
	ClientToScreen(GetDlgItem(g_hwndDlg, IDC_D1), &p1);
	ClientToScreen(GetDlgItem(g_hwndDlg, IDC_D2), &p2);
	ydiff = p2.y - p1.y;
	// get start pos
	p1.x = p1.y = p2.x = p2.y = 0;
	ClientToScreen(GetDlgItem(g_hwndDlg, IDC_S_NONE), &p1);
	ClientToScreen(g_hwndDlg, &p2);
	x0 = p1.x - p2.x;
	y0 = p1.y - p2.y;
	// radio buttons
	for(i = 0; i < T_END - T_SHIPEND; i++)
	{
		int y;
		y = y0 + i * ydiff;
		xl = g_DLUData.xPx * 2; yl = g_DLUData.yPx;
		CreateWindow(_T("BUTTON"), _T(""), WS_CHILD|BS_AUTOCHECKBOX, x0, y, xl, yl, g_hwndDlg, (HMENU)(ID_FIRST_RADIO_IPM + i), g_hInst, NULL);
	}
	SendDlgItemMessage(g_hwndDlg, ID_FIRST_RADIO_IPM, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	g_iCheckedIPMBox = 1;

	// name
	p1.x = p1.y = 0;
	GetWindowRect(GetDlgItem(g_hwndDlg, IDC_S14), &r);
	ClientToScreen(GetDlgItem(g_hwndDlg, IDC_D8), &p1);
	x0 = p1.x - p2.x;
	y0 = p1.y - p2.y + ydiff;
	xl = r.right - r.left; yl = r.bottom - r.top;
	CreateWindow(_T("STATIC"), _T(""), WS_CHILD|SS_LEFT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_NAME_IPM, g_hInst, 0);
	
	// input (ABM)
	p1.x = p1.y = 0;
	ClientToScreen(GetDlgItem(g_hwndDlg, IDC_KS_V), &p1);
	GetWindowRect(GetDlgItem(g_hwndDlg, IDC_GS_V), &r);
	y0 = r.top - p2.y + ydiff;
	x02 = x0 = r.left - p2.x;
	xl = r.right - r.left; yl = r.bottom - r.top;    
	CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD|ES_NUMBER|ES_RIGHT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_INPUT_ABM, g_hInst, 0);
	SendDlgItemMessage(g_hwndDlg, ID_INPUT_ABM, EM_SETLIMITTEXT, (WPARAM)3, 0);

	// input (IPM)
	p1.x = p1.y = 0;
	ClientToScreen(GetDlgItem(g_hwndDlg, IDC_IC_A), &p1);
	x0 = p1.x - p2.x + xl / 4;
	CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD|ES_NUMBER|ES_RIGHT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_INPUT_IPM, g_hInst, 0);
	SendDlgItemMessage(g_hwndDlg, ID_INPUT_IPM, EM_SETLIMITTEXT, (WPARAM)3, 0);

	// arrow ABM
	y0 = 2 * y0 + yl;
	GetWindowRect(GetDlgItem(g_hwndDlg, IDC_S_ARROW), &r);
	x0 = r.left - p2.x;
	xl = r.right - r.left; yl = r.bottom - r.top;
	y0 = (y0 - yl) / 2;
	CreateWindow(_T("STATIC"), _T("->"), WS_CHILD|SS_LEFT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_ABM_ARROW, g_hInst, 0);
	// arrow IPM
	GetWindowRect(GetDlgItem(g_hwndDlg, ID_INPUT_IPM), &r);
	x0 = r.right - p2.x + 2;
	CreateWindow(_T("STATIC"), _T("->"), WS_CHILD|SS_LEFT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_IPM_ARROW, g_hInst, 0);
	
	// output IPM res
	x0 += xl;
	xl = g_DLUData.xPx * 4;
	CreateWindow(_T("STATIC"), _T(""), WS_CHILD|SS_LEFT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_OUTPUT_IPM, g_hInst, 0);
	// output ABM res
	GetWindowRect(GetDlgItem(g_hwndDlg, ID_ABM_ARROW), &r);
	x0 = r.right - p2.x;
	CreateWindow(_T("STATIC"), _T(""), WS_CHILD|SS_LEFT, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_OUTPUT_ABM, g_hInst, 0);
	
	// button 'overtake results'
	GetWindowRect(GetDlgItem(g_hwndDlg, ID_INPUT_ABM), &r);
	x0 = r.left - p2.x;
	y0 = r.bottom + 2 - p2.y;
	yl = g_DLUData.yPx;
	GetWindowRect(GetDlgItem(g_hwndDlg, IDC_GB_DEF), &r);
	xl = r.right - x0 - p2.x - 2;
	CreateWindow(_T("BUTTON"), g_IPModeStr[2].c_str(), WS_CHILD|BS_CENTER, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_TAKE_OVER_IPM_RES, g_hInst, NULL);
	// needed IPMs
	GetWindowRect(GetDlgItem(g_hwndDlg, ID_INPUT_IPM), &r);
	xl = r.left - r.right;
	x0 = 2 * (r.left - p2.x) + xl;
	xl = g_DLUData.xPx * 5;
	x0 = (x0 + xl) / 2;
	y0 = r.bottom - p2.y + 2;
	yl = g_DLUData.yPx;
	CreateWindow(_T("STATIC"), _T(""), WS_CHILD|SS_CENTER, x0, y0, xl, yl, g_hwndDlg, (HMENU)ID_OUTPUT_NEEDED_IPM, g_hInst, 0);
}

DWORD GetEditIdByType(ITEM_TYPE Type, bool Attacker)
{
	if(Attacker)
	{
		switch(Type)
		{
		case T_KT:
			return IDC_KT_A;
		case T_GT:
			return IDC_GT_A;
		case T_LJ:
			return IDC_LJ_A;
		case T_SJ:
			return IDC_SJ_A;
		case T_KREUZER:
			return IDC_KR_A;
		case T_SS:
			return IDC_SS_A;
		case T_KOLO:
			return IDC_KOL_A;
		case T_REC:
			return IDC_REC_A;
		case T_SPIO:
			return IDC_SPIO_A;
		case T_BOMBER:
			return IDC_BOMB_A;
		case T_ZER:
			return IDC_ZERR_A;
		case T_TS:
			return IDC_TS_A;
		case T_IC:
			return IDC_IC_A;
		case T_REAP:
			return IDC_REAP_A;
		case T_PF:
			return IDC_PF_A;
		default:
			return (DWORD)-1;
		}
	}
	else
	{
		switch(Type)
		{
		case T_KT:
			return IDC_KT_V;
		case T_GT:
			return IDC_GT_V;
		case T_LJ:
			return IDC_LJ_V;
		case T_SJ:
			return IDC_SJ_V;
		case T_KREUZER:
			return IDC_KR_V;
		case T_SS:
			return IDC_SS_V;
		case T_KOLO:
			return IDC_KOL_V;
		case T_REC:
			return IDC_REC_V;
		case T_SPIO:
			return IDC_SPIO_V;
		case T_BOMBER:
			return IDC_BOMB_V;
		case T_SAT:
			return IDC_SOL_V;
		case T_ZER:
			return IDC_ZERR_V;
		case T_TS:
			return IDC_TS_V;
		case T_IC:
			return IDC_IC_V;
		case T_CRA:
			return IDC_CRA_V;
		case T_REAP:
			return IDC_REAP_V;
		case T_PF:
			return IDC_PF_V;
		case T_RAK:
			return IDC_RAK_V;
		case T_LL:
			return IDC_LL_V;
		case T_SL:
			return IDC_SL_V;
		case T_GAUSS:
			return IDC_GA_V;
		case T_IONEN:
			return IDC_IO_V;
		case T_PLASMA:
			return IDC_PLASMA_V;
		case T_KS:
			return IDC_KS_V;
		case T_GS:
			return IDC_GS_V;
		default:
			return (DWORD)-1;
		}
	}
}

DWORD GetLabelIdByType(ITEM_TYPE Type, bool Attacker)
{
	if(Attacker)
	{
		switch(Type)
		{
		case T_KT:
			return IDC_KT_A_E;
		case T_GT:
			return IDC_GT_A_E;
		case T_LJ:
			return IDC_LJ_A_E;
		case T_SJ:
			return IDC_SJ_A_E;
		case T_KREUZER:
			return IDC_KR_A_E;
		case T_SS:
			return IDC_SS_A_E;
		case T_KOLO:
			return IDC_KOL_A_E;
		case T_REC:
			return IDC_REC_A_E;
		case T_SPIO:
			return IDC_SPIO_A_E;
		case T_BOMBER:
			return IDC_BOMB_A_E;
		case T_ZER:
			return IDC_ZERR_A_E;
		case T_TS:
			return IDC_TS_A_E;
		case T_IC:
			return IDC_IC_A_E;
		case T_REAP:
			return IDC_REAP_A_E;
		case T_PF:
			return IDC_PF_A_E;
		default:
			return (DWORD)-1;
		}
	}
	else
	{
		switch(Type)
		{
		case T_KT:
			return IDC_KT_V_E;
		case T_GT:
			return IDC_GT_V_E;
		case T_LJ:
			return IDC_LJ_V_E;
		case T_SJ:
			return IDC_SJ_V_E;
		case T_KREUZER:
			return IDC_KR_V_E;
		case T_SS:
			return IDC_SS_V_E;
		case T_KOLO:
			return IDC_KOL_V_E;
		case T_REC:
			return IDC_REC_V_E;
		case T_SPIO:
			return IDC_SPIO_V_E;
		case T_BOMBER:
			return IDC_BOMB_V_E;
		case T_SAT:
			return IDC_SOL_V_E;
		case T_ZER:
			return IDC_ZERR_V_E;
		case T_TS:
			return IDC_TS_V_E;
		case T_IC:
			return IDC_IC_V_E;
		case T_CRA:
			return IDC_CRA_V_E;
		case T_REAP:
			return IDC_REAP_V_E;
		case T_PF:
			return IDC_PF_V_E;
		case T_RAK:
			return IDC_RAK_V_E;
		case T_LL:
			return IDC_LL_V_E;
		case T_SL:
			return IDC_SL_V_E;
		case T_GAUSS:
			return IDC_GA_V_E;
		case T_IONEN:
			return IDC_IO_V_E;
		case T_PLASMA:
			return IDC_PLASMA_V_E;
		case T_KS:
			return IDC_KS_V_E;
		case T_GS:
			return IDC_GS_V_E;
		default:
			return (DWORD)-1;
		}
	}
}

map<genstring, genstring> UpdateLangFiles()
{
	TCHAR buff[256];
	TCHAR dir[MAX_PATH];
	TCHAR name[MAX_PATH];
	WIN32_FIND_DATA fd;

	ZeroMemory(buff, 256);
	ZeroMemory(name, MAX_PATH);

	g_LangFiles.clear();

	_tcscpy(dir, g_CurrDir);
	_tcscat(dir, _T("*.ini"));
	
	HANDLE hFF = FindFirstFile(dir, &fd);
	
	map<genstring, genstring> files;
	if(IsLanguageFile(fd.cFileName))
	{
		_tcscpy(name, g_CurrDir);
		_tcscat(name, fd.cFileName);
		GetPrivateProfileString(_T("General"), _T("LANG_DESC"), fd.cFileName, buff, 128, name);
		files[buff] = fd.cFileName; 
	}

	while(FindNextFile(hFF, &fd))
	{
		if(IsLanguageFile(fd.cFileName))
		{
			_tcscpy(name, g_CurrDir);
			_tcscat(name, fd.cFileName);
			GetPrivateProfileString(_T("General"), _T("LANG_DESC"), fd.cFileName, buff, 128, name);
			files[buff] = fd.cFileName;
		}
	}
	FindClose(hFF);
	map<genstring, genstring>::const_iterator it = files.begin();
	for(; it != files.end(); it++)
	{
		g_LangFiles.push_back(it->second);
	}
	return files;
}

void CheckResolution()
{
	DEVMODE dev;
	dev.dmSize = sizeof(DEVMODE);
	dev.dmDriverExtra = 0;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dev);
	RECT r;
	GetWindowRect(g_hwndDlg, &r);
	// if resolution too low
	if(dev.dmPelsHeight < r.bottom - r.top)
	{
		g_bLowRes = true;
		ShowScrollBar(g_hwndDlg, SB_VERT, true);
		ShowScrollBar(g_hwndDlg, SB_HORZ, false);
		g_WndRect.right = dev.dmPelsWidth;
		g_WndRect.bottom = (dev.dmPelsHeight*7.0f/8.0f);
		//SetScrollRange(g_hwndDlg, SB_HORZ, 0, g_WndRect.right*0.18f, true);
		SetScrollRange(g_hwndDlg, SB_VERT, 0, g_WndRect.bottom*0.35f, true);
		SetWindowPos(g_hwndDlg, NULL, (dev.dmPelsWidth-g_WndRect.right)/2, (dev.dmPelsHeight-g_WndRect.bottom)/2, g_WndRect.right, g_WndRect.bottom, SWP_NOZORDER|SWP_FRAMECHANGED);
		// deactivate Skins
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_CHG_SKIN), false);
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_USE_SKIN), false);
		g_Options.SkinFileName[0] = '\0';
	}
	
	SIZE size;
	TEXTMETRIC tm;
	HDC hdc = GetDC(g_hwndDlg);
	//HFONT Font = (HFONT)SendMessage(g_hwndDlg, WM_GETFONT, 0, 0);
	GetTextMetrics(hdc, &tm);
	GetTextExtentPoint32(hdc, _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52, &size);
	ReleaseDC(g_hwndDlg, hdc);
	int avgWidth = (size.cx/26+1)/2;
	int avgHeight = (WORD)tm.tmHeight;
	
	g_DLUData.xPx = avgWidth;
	g_DLUData.yPx = avgHeight;
}

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER     0x00010000
#endif
void InitEspHistory()
{
	SetWindowText(g_hwndEspHist, g_EspHistStrings[0].c_str());
	RECT r;
	// prepare ListView
	HWND hwndLV = GetDlgItem(g_hwndEspHist, IDC_REPORT_LIST);
	GetClientRect(hwndLV, &r);
	ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP|LVS_EX_DOUBLEBUFFER);
	if(fnSetWndThm)
		fnSetWndThm(hwndLV, L"Explorer", NULL);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = (TCHAR*)g_EspHistStrings[1].c_str();
	lvc.cx = (r.right - r.left) / 2;
	lvc.cchTextMax = g_EspHistStrings[1].length() + 1;
	ListView_InsertColumn(hwndLV, 0, &lvc);
	lvc.pszText = (TCHAR*)g_EspHistStrings[2].c_str();
	lvc.cx = (r.right - r.left) / 4;
	lvc.cchTextMax = g_EspHistStrings[2].length() + 1;
	ListView_InsertColumn(hwndLV, 1, &lvc);
	lvc.pszText = (TCHAR*)g_EspHistStrings[7].c_str();
	lvc.cx = (r.right - r.left) / 4;
	lvc.cchTextMax = g_EspHistStrings[7].length() + 1;
	ListView_InsertColumn(hwndLV, 2, &lvc);

	// set lang stuff
	SetDlgItemText(g_hwndEspHist, IDC_S_INFO_ESP, g_EspHistStrings[3].c_str());
	SetDlgItemText(g_hwndEspHist, IDC_DISABLE_REP_HIST, g_EspHistStrings[4].c_str());
	SetDlgItemText(g_hwndEspHist, IDC_DEL_CUR_REP, g_EspHistStrings[5].c_str());
	SetDlgItemText(g_hwndEspHist, IDC_CLOSE_ESP_HIST, g_MiscStr[2].c_str());
	SetDlgItemText(g_hwndEspHist, IDC_SAVE_REP, g_EspHistStrings[10].c_str());
	SetDlgItemText(g_hwndEspHist, IDC_NUM_REP_T, g_EspHistStrings[11].c_str());

	// subclass listview to get WM_HSCROLL
	OldLVProc = (WNDPROC)SetWindowLongPtr(hwndLV, GWLP_WNDPROC, (LONG_PTR)ListViewProc);

	// load old reports?
	RefreshReportHistory();

	OnResizeHistWindow(g_hwndEspHist, r, 0, 0, true);

	// load saved pos
	RECT r2;
	GetWindowRect(g_hwndEspHist, &r);

	r2.left = g_Options.EspHistPos[0];
	r2.top = g_Options.EspHistPos[1];
	if(g_Options.EspHistSize[0] > r.right - r.left)
		r2.right = r2.left + g_Options.EspHistSize[0];
	else
		r2.right = r2.left + r.right - r.left;
	if(g_Options.EspHistSize[1] > r.bottom - r.top)
		r2.bottom = r2.top + g_Options.EspHistSize[1];
	else
		r2.bottom = r2.top + r.bottom - r.top;
	
	// use default values, if window out of sight
	if(r2.right < 0 || r2.bottom < 0)
	{
		memcpy(&r2, &r, sizeof(RECT));
	}
	

	if(g_Options.cHistInFile > 0)
	{
		EnableWindow(GetDlgItem(g_hwndEspHist, IDC_NUM_REP), TRUE);
		SetDlgItemInt(g_hwndEspHist, IDC_NUM_REP, g_Options.cHistInFile, FALSE);
		CheckDlgButton(g_hwndEspHist, IDC_SAVE_REP, BST_CHECKED);
	}
	else
	{
		EnableWindow(GetDlgItem(g_hwndEspHist, IDC_NUM_REP), FALSE);
		CheckDlgButton(g_hwndEspHist, IDC_SAVE_REP, BST_UNCHECKED);
	}
	MoveWindow(g_hwndEspHist, r2.left, r2.top, r2.right - r2.left, r2.bottom - r2.top, TRUE);
}


bool AddReportToHistory(TargetInfo ti, time_t* t)
{
	EspHistItem hi;
	if(t)
		hi.ctime = *t;
	else
		time(&hi.ctime);
	hi.Target = ti;
	
	// check if coords already available
	size_t i;
	for(i = 0; i < g_vRaidHistory.size(); i++)
	{
		if(ti.Pos == g_vRaidHistory[i].Target.Pos)
		{
			g_vRaidHistory.erase(g_vRaidHistory.begin() + i);
			break;
		}
	}
	g_vRaidHistory.push_back(hi);
	return true;
}

void RefreshReportHistory()
{
	HWND hwndLV = GetDlgItem(g_hwndEspHist, IDC_REPORT_LIST);
	ListView_DeleteAllItems(hwndLV);
	size_t i;
	for(i = 0; i < g_vRaidHistory.size(); i++)
	{
		genstrstream str;
		str << _T("[") << g_vRaidHistory[i].Target.Pos.ToString() << (g_vRaidHistory[i].Target.Pos.bMoon ? g_PopupStrings[4] : _T("")) << _T("] ") << g_vRaidHistory[i].Target.Name;
		TCHAR time_str[128];
		tm time_s, *time_now;
		memcpy(&time_s, localtime(&g_vRaidHistory[i].ctime), sizeof(tm));
		time_t now;
		time(&now); time_now = localtime(&now);
		
		if(time_s.tm_yday != time_now->tm_yday || time_s.tm_year != time_now->tm_year)
			_tcsftime(time_str, 128, _T("%x, %X"), &time_s);
		else
			_tcsftime(time_str, 128, _T("%X"), &time_s);
		
		TCHAR c[128];
		_tcsncpy(c, str.str().c_str(), 128);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT|LVIF_PARAM;
		lvi.lParam = i;
		lvi.iSubItem = 0;
		lvi.pszText = c;
		lvi.cchTextMax = 128;
		lvi.iItem = 0;
		ListView_InsertItem(hwndLV, &lvi);
		
		lvi.mask = LVIF_TEXT;
		lvi.pszText = time_str;
		lvi.iSubItem = 1;
		ListView_SetItem(hwndLV, &lvi);
		
		lvi.mask = LVIF_TEXT;
		lvi.pszText = g_vRaidHistory[i].Checked ? _T("X") : _T("");
		lvi.iSubItem = 2;
		ListView_SetItem(hwndLV, &lvi);
	}
	UpdateHistTooltips();
}

void UpdateHistTooltips()
{   
	HWND hwndLV = GetDlgItem(g_hwndEspHist, IDC_REPORT_LIST);
	if(!hwndLV)
		return;

	static HWND hwndTT = NULL;
	static int last_cnt = 0;

	if(!hwndTT)
	{
		hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndLV, NULL, g_hInst, NULL);
		SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 300);
		
		SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		SendMessage(hwndTT, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELONG(20000, 0));
	}

	// remove old windows
	size_t rows = ListView_GetItemCount(hwndLV);
	int i;
	for(i = 0; i < last_cnt; i++)
	{
		TOOLINFO tooli;
		tooli.uId = i;
		tooli.hwnd = hwndLV;
		SendMessage(hwndTT, TTM_DELTOOL, 0, (LPARAM)&tooli);
	}
	
	RECT RectLV;
	GetClientRect(hwndLV, &RectLV);
	
	last_cnt = rows = g_vRaidHistory.size();
	// get row rect and install tooltip there
	for(i = 0; i < rows; i++)
	{
		RECT rcItem;
		ListView_GetItemRect(hwndLV, i, &rcItem, LVIR_BOUNDS);
		if(rcItem.top < RectLV.top || rcItem.bottom > RectLV.bottom)
			continue;
		// set tooltip into row
		
		// generate text
		TargetInfo ti = g_vRaidHistory[rows - i - 1].Target;
		genstrstream tttxt;
		TCHAR num[128];
		tttxt << ti.Name << _T(" [") << ti.Pos.ToString() << (ti.Pos.bMoon ? g_PopupStrings[4] : _T("")) << _T("]\r\n");
		tttxt << _T(" --- ") << g_EspTitles[0] << _T(" ---") << _T("\r\n");
		tttxt << g_ResNames[0] << _T(": ") << AddPointsToNumber(ti.Resources.met, num) << _T("\r\n");
		tttxt << g_ResNames[1] << _T(": ") << AddPointsToNumber(ti.Resources.kris, num) << _T("\r\n");
		tttxt << g_ResNames[2] << _T(": ") << AddPointsToNumber(ti.Resources.deut, num);

		if(ti.Fleet.size())
			tttxt << _T("\r\n\r\n --- ") << g_EspTitles[1] << _T(" --- ") << _T("\r\n");
		size_t j;
		for(j = 0; j < ti.Fleet.size(); j++)
		{
			TCHAR item[128];
			sim.ItemName(ti.Fleet[j].Type, item);
			tttxt << item << _T(" ") << AddPointsToNumber(ti.Fleet[j].Num, num);
			if(j != ti.Fleet.size() - 1)
			{
				tttxt << _T("\r\n");
			}
		}
		if(ti.Defence.size() || ti.NumABM)
			tttxt << _T("\r\n\r\n --- ") << g_EspTitles[2] << _T(" ---") << _T("\r\n");
		for(j = 0; j < ti.Defence.size(); j++)
		{
			TCHAR item[128];
			sim.ItemName(ti.Defence[j].Type, item);
			tttxt << item << _T(" ") << AddPointsToNumber(ti.Defence[j].Num, num);
			if(j != ti.Defence.size() - 1)
			{
				tttxt << _T("\r\n");
			}
		}
		if(ti.NumABM)
		{
			tttxt << _T("\r\n");
			TCHAR item[128];
			sim.ItemName(T_END, item);
			tttxt << item << _T(" ") << AddPointsToNumber(ti.NumABM, num);
		}
		if(ti.Techs.Armour || ti.Techs.Shield || ti.Techs.Weapon)
		{
			tttxt << _T("\r\n\r\n --- ") << g_EspTitles[3] << _T(" ---") << _T("\r\n");
			tttxt << g_TechNames[0] << _T(" ") << ti.Techs.Weapon << _T("\r\n");
			tttxt << g_TechNames[1] << _T(" ") << ti.Techs.Shield << _T("\r\n");
			tttxt << g_TechNames[2] << _T(" ") << ti.Techs.Armour;
		}
		_tcsncpy(g_vRaidHistory[i].TipText, tttxt.str().c_str(), 1024);
		
		TOOLINFO tooltip;
		tooltip.cbSize = sizeof(TOOLINFO);
		tooltip.uFlags = TTF_SUBCLASS;
		tooltip.hwnd = hwndLV;
		tooltip.hinst = g_hInst;
		tooltip.uId = i;
		tooltip.lpszText = g_vRaidHistory[i].TipText;
		tooltip.rect = rcItem;

		SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&tooltip);
	}
}

genstring CorrectTabs(genstrstream stream)
{
	genstring line;
	genstring::size_type maxp = 0;
	genstrstream tmpstr;
	// get maximum position of tab
	while (getline(stream, line))
	{
		genstring::size_type f;
		if(line.length() == 0)
			continue;
		if((f = line.find(_T("\t"))) != genstring::npos)
		{
			if(f > maxp)
				maxp = f;
		}
		else if(line.length() - 1 > maxp)
			maxp = line.length() - 1;
	}
	maxp -= maxp % 4;
	// add tabs, if needed
	stream.clear();
	stream.seekg(0, ios::beg);
	while (getline(stream, line))
	{
		genstring::size_type f;
		genstring line2 = line;
		f = line2.find(_T("\t"));
		if(f != genstring::npos)
		{
			int diff = abs((int)(maxp - f)) / 4;
			if(diff)
				line2.insert(f, diff, '\t');
		}
		tmpstr << line2 << _T("\n");
	}
	tmpstr << '\0';
	stream.clear();
	stream.seekg(0, ios::beg);
	return tmpstr.str();
}

void MyMoveWnd(HWND wnd, HWND parent, int xdiff, int ydiff)
{
	RECT r;
	GetWindowRect(wnd, &r);
	MapWindowPoints(HWND_DESKTOP, parent, (LPPOINT)&r, 2);
	r.top += ydiff; r.bottom += ydiff;
	r.left += xdiff; r.right += xdiff;
	MoveWindow(wnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
}

void SetDefCSSFiles()
{
	TCHAR css1[INTERNET_MAX_URL_LENGTH], css2[INTERNET_MAX_URL_LENGTH];
	u_long l = INTERNET_MAX_URL_LENGTH;
	HRESULT r = UrlCreateFromPath(g_CurrDir, css1, &l, NULL);
	_tcscat(css1, _T("cr.css"));
	l = INTERNET_MAX_URL_LENGTH;
	UrlCreateFromPath(g_CurrDir, css2, &l, NULL);
	_tcscat(css2, _T("bwc.css"));

	sim.SetCSSFiles(css1, css2);
}

int CompareVersions(const TCHAR* version_1, const TCHAR* version_2)
{
	genstring v1(version_1), v2(version_2);
	int i;
	for(i = 0; i < 4; i++)
	{
		size_t f = 0, f2 = 0;
		int n1, n2;
		n1 = _ttoi(v1.c_str());
		n2 = _ttoi(v2.c_str());
		if(n1 > n2)
			return 1;
		if(n1 < n2)
			return -1;
		
		f = v1.find(_T("."));
		if(f != genstring::npos)
		{
			v1.erase(0, f);
		}
		v1.erase(0, 1);
		
		f2 = v2.find(_T("."));
		if(f2 != genstring::npos)
		{
			v2.erase(0, f2);
		}
		v2.erase(0, 1);
	}
	return 0;
}

void InstallURLHandler()
{

}
