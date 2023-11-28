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

//----------------------SpeedSim, the OGame combat simulator----------------

#include "SpeedSim.h"

HWND g_hwndDlg = NULL, g_hwndOpt = NULL, g_hwndUpd = NULL, g_hwndDown = NULL, g_hwndSave = NULL, g_hwndPopup = NULL;
HWND g_hwndEspHist = NULL;
HINSTANCE g_hInst = NULL;
unsigned long g_UpdThread = 0, g_SimThread = 0, g_LastSimThread = 0, g_CheckThread = 0;
WNDPROC g_oldEditProc = NULL, g_oldWebsiteProc = NULL , g_oldIRCProc = NULL;
HFONT g_Font = NULL, g_nonUCFont = NULL, g_hPopupFont = NULL;
HANDLE g_hMutex = NULL;
HBITMAP g_hPopupBMP = NULL;

genstring g_strSim[3], g_strSimState[2], g_ResString[15], g_BilanzStrings[5];
genstring g_ResNames[3], g_UpdaterStr[7], g_MiscStr[10], g_IPModeStr[5];
genstring g_ScanEditStr[6], g_OptPage1[11], g_OptPage2[12], g_TrayText[7];
genstring g_PopupStrings[5], g_OptPage3[5], g_EspHistStrings[12], g_TechNames[6];
genstring g_EspTitles[4];
TCHAR g_DecSeperator[10];

map<HWND, genstring> g_GerStringsMain;
ETDTProc fnETDT = NULL;
SetWndThmProc fnSetWndThm = NULL;

bool g_PopupShown = false;

bool g_bLowRes = false;
bool g_IPMMode = false;
HWND g_hwndNextCBWnd = NULL;
int g_SimCount = 0;
int g_LastID = 0;

TCHAR g_CurrDir[MAX_PATH] = _T("");
DWORD g_CurSaveData = 1;
int g_NumFleets = 0;

int g_iNumShipsForPlunderNeeded[T_RAK];

HMODULE g_hUser32DLL = NULL, g_hUXThemeDLL = NULL;
int g_iCheckedIPMBox = 0;

CSpeedKernel sim;
CSkinClass g_Skin;

UINT SCAN_MSG = 0;
SaveData g_Options;

vector<genstring> g_LangFiles;

RECT g_WndRect, g_ClntRect;

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow )
{
	g_hInst = hInstance;
	sim.Reset();
/*#ifdef UNICODE
	// check for unicode font
	if(!CheckFont(_T("Microsoft Sans Serif"))) {
		MessageBox(NULL, _T("Missing Unicode font."), _T("Error"), MB_OK|MB_ICONERROR);
		return 0;
	}
#endif*/
	SCAN_MSG = RegisterWindowMessage(SCAN_MSG_TXT);
	if(!SCAN_MSG) {
		MessageBox(NULL, _T("Could not register window message!"), _T("Error"), NULL);
		//return 0;
	}

	// locale
	setlocale(LC_ALL, "");
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, g_DecSeperator, 10);

	// set current path to exe-path
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	int pos = _tcsrchr(path, '\\') - path;
	path[pos] = _T('\0');
	SetCurrentDirectory(path);
	_tcscpy(g_CurrDir, path);
	_tcscat(g_CurrDir, _T("\\"));
	//LoadLang(NULL);
	
	// Prevent SpeedSim from starting a second time, if wished
	TCHAR iniFile[MAX_PATH];
	genstr nfile = GetSaveFolder() + SPEEDSIM_SAVEFILE;
	_tcsncpy(iniFile, nfile.c_str(), MAX_PATH);
	g_CurSaveData = GetPrivateProfileInt(_T("General"), _T("LastOpt"), 1, iniFile);
	g_NumFleets = GetPrivateProfileInt(_T("General"), _T("NumFleets"), 0, iniFile);

	g_hMutex = CreateMutex(NULL, TRUE, SPEEDSIM_MUTEX);
	if(GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(g_hMutex);
		g_hMutex = NULL;
	}
	if(!g_hMutex) {
		SaveData save;
		NewGetSaveData(SPEEDSIM_SAVEFILE, g_CurSaveData, save);
		if(save.bSingleInst == TRUE) {
			// set old window to foreground and exit
			HWND wnd = FindWindow(SPEEDSIM_CLASSNAME, NULL);
			ShowWindow(wnd, SW_RESTORE);
			ForceSetForegroundWindow(wnd);
			return 0;
		}
	}
	
	// common controls
	INITCOMMONCONTROLSEX icc{};
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES|ICC_WIN95_CLASSES|ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icc);
	
	g_hUser32DLL = LoadLibrary(_T("user32.dll"));
	g_hUXThemeDLL = LoadLibrary(_T("uxtheme.dll"));
	if(g_hUXThemeDLL)
	{
		fnETDT = (ETDTProc)GetProcAddress(g_hUXThemeDLL, "EnableThemeDialogTexture");
		fnSetWndThm = (SetWndThmProc)GetProcAddress(g_hUXThemeDLL, "SetWindowTheme");
	}
	
	HACCEL hAcc = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_ACCEL));
	// register window class
	MSG msg;
	WNDCLASS wc{};
	wc.style = CS_GLOBALCLASS;
	wc.lpszClassName = SPEEDSIM_CLASSNAME;
	wc.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WindowProc;
	wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	wc.hInstance = g_hInst;
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	if (!RegisterClass(&wc)) {
		return 1;
	}

	CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG), NULL, DialogProc);
	// message loop
	while (GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator(g_hwndDlg, hAcc, &msg)) {
			if(!IsDialogMessage(g_hwndDlg, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	if(g_Font)
		DeleteObject(g_Font);
	if(g_nonUCFont)
		DeleteObject(g_nonUCFont);
	// remove tray symbol
	CreateSystrayIcon(false);
	// kill Threads
	if(g_Options.HookClipboard)
		ChangeClipboardChain(g_hwndDlg, g_hwndNextCBWnd);
	if(g_hMutex) {
		ReleaseMutex(g_hMutex);
		CloseHandle(g_hMutex);
	}
	DestroyWindow(g_hwndEspHist);
	DestroyWindow(g_hwndDlg);
	if (g_hUser32DLL)
		FreeLibrary(g_hUser32DLL);
	if(g_hUXThemeDLL)
		FreeLibrary(g_hUXThemeDLL);

	return 0;
}

// gives fleet and other information to kernel
void UpdateKernel(int FleetID)
{
	vector<SItem> att, def;
	SItem item;
	int i;
	if(FleetID > 15 || FleetID < 0)
		FleetID = 0;
	
	// -------------Fleet-------------
	item.OwnerID = FleetID;
	for(i = 0; i < T_END; i++)
	{
		item.Type = (ITEM_TYPE)i;
		if(i < T_SHIPEND && i != T_SAT)
		{
			item.Num = ReadFromEdit(GetEditIdByType((ITEM_TYPE)i, true));
			att.push_back(item);
		}
		item.Num = ReadFromEdit(GetEditIdByType((ITEM_TYPE)i, false));
		def.push_back(item);
	}

	// read in technologies
	ShipTechs tech[2];
	tech[0].Weapon = ReadFromEdit(IDC_WAFF_A);
	tech[0].Shield = ReadFromEdit(IDC_SCHILD_A);
	tech[0].Armour = ReadFromEdit(IDC_PANZ_A);
	tech[1].Weapon = ReadFromEdit(IDC_WAFF_V);
	tech[1].Shield = ReadFromEdit(IDC_SCHILD_V);
	tech[1].Armour = ReadFromEdit(IDC_PANZ_V);
	for(i = 0; i < 2; i++) {
		if(tech[i].Weapon > 99)
			tech[i].Weapon = 99;
		else if(tech[i].Weapon < 0)
			tech[i].Weapon = 0;

		if(tech[i].Shield > 99)
			tech[i].Shield = 99;
		else if(tech[i].Shield < 0)
			tech[i].Shield = 0;
		
		if(tech[i].Armour > 99)
			tech[i].Armour = 99;
		else if(tech[i].Armour< 0)
			tech[i].Armour = 0;
	}

	// get RF
	if(IsDlgButtonChecked(g_hwndDlg, IDC_RAPID) == BST_CHECKED)
	{
		sim.SetRF(RF_075);
		if(strlen(g_Options.RFFileName))
		{
			sim.LoadRFFile(g_Options.RFFileName);
			sim.SetRF(RF_USER);
		}
	}
	else
		sim.SetRF(RF_NONE);
	
	if(strlen(g_Options.ShipDataFile)) {
		sim.LoadShipData(g_Options.ShipDataFile);
		sim.SetUseShipDataFromFile(true);
	}
	else
		sim.SetUseShipDataFromFile(false);
	
	// give information to kernel
	TCHAR c[10];
	SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, WM_GETTEXT, 10, (LPARAM)c);
	PlaniPos pos(c);
	sim.SetOwnPosition(pos, FleetID);
	bool DefInTF = (IsDlgButtonChecked(g_hwndDlg, IDC_DEFTF) == BST_CHECKED);
	sim.SetDefInDebris(DefInTF);
	sim.SetFleet(&att, &def);
	sim.SetTechs(&tech[0], &tech[1], FleetID);
	int speed = (10 - (int)SendDlgItemMessage(g_hwndDlg, IDC_SPEED, CB_GETCURSEL, 0, 0)) * 10;
	sim.SetSpeed(speed, ReadFromEdit(IDC_VERBRENNUNG), ReadFromEdit(IDC_IMPULS), ReadFromEdit(IDC_HYPERRAUM), FleetID);
	sim.RebuildSmallDefense(g_Options.bRebuildSmallDef == TRUE);
	sim.SetDefRebuildFactor(g_Options.iDefRebuild / 100.0f);
	sim.UseOldBattleShip(g_Options.bUseOldBS);
	if(FleetID == 0) {
		int nABM = ReadFromEdit(ID_INPUT_ABM);
		TargetInfo ti = sim.GetTargetInfo(0);
		ti.NumABM = nABM;
		sim.SetTargetInfo(ti, 0, false);
	}
	sim.SetPercLossesToDF(g_Options.StructureToDF);
	return;
}

void GeneralRead() {
	TCHAR c[65000];
	int numc = SendDlgItemMessage(g_hwndDlg, IDC_GEN_READ, WM_GETTEXT, (WPARAM)65000, (LPARAM)c);
	c[numc] = '\0';
	genstring Info = c;

	int id = ReadFromEdit(IDC_FLEET_ID)-1;
	if(id < 0 || id > 15) {
		id = 0;
	}
	// ignore battle results from speedsim
	TCHAR d[256];
	GetDlgItemText(g_hwndDlg, IDC_S_SCAN, d, 256);
	genstring s = d;
	s += _T(":\t\t");
	if(Info.find(s) != string::npos) {
		return;
	}

	// forward text to kernel
	TEXTTYPE text = sim.GeneralRead(Info, id);
	
	if((IsDlgButtonChecked(g_hwndDlg, IDC_CHECKTECHS) == BST_CHECKED) && text == TEXT_ESPIO) {
		// if esp. report and techs = 0 -> set own techs to 0
		g_Options.bCheckTechs = true;
		ShipTechs techs;
		sim.GetTechs(NULL, &techs, id);
		if(techs.Weapon == 0 && techs.Shield == 0 && techs.Armour == 0)
			sim.SetTechs(&techs, NULL, id);
		else
			LoadTechs(g_CurSaveData, true);
	}
	else
		g_Options.bCheckTechs = false;
	if(text == TEXT_ESPIO)
	{
		if(sim.GetBattleResult().Position.bMoon)
			SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_CHECKED, NULL);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_UNCHECKED, NULL);
		SetDlgItemText(g_hwndDlg, ID_OUTPUT_NEEDED_IPM, _T(""));

		// add to history
		TargetInfo tis[50];
		int n = sim.MultiReadEspReport(Info, tis, 50);
		if(!IsDlgButtonChecked(g_hwndEspHist, IDC_DISABLE_REP_HIST))
		{
			for(int i = 0; i < n; i++)
				AddReportToHistory(tis[i]);
			RefreshReportHistory();
		}
	}
	// set window to foreground
	if((text == TEXT_ESPIO || text == TEXT_COMBAT || text == TEXT_OVERVIEW) && g_Options.bPopUpOnText)
	{
		if(g_Options.bTrayIcon)
		{
			CreateSystrayIcon(false);
		}
		ShowWindow(g_hwndDlg, SW_RESTORE);
		SetForegroundWindow(g_hwndDlg);
		//ForceSetForegroundWindow(g_hwndDlg);
	}
	return;
}

// Updates GUI
void UpdateEditFields(bool doAtt, bool doDef) {
	SItem Att[T_END], Def[T_END];
	ShipTechs techs[2];
	TCHAR c[10];
	if(doAtt)
		ResetAtt();
	if(doDef)
		ResetDef();
	int FleetID = ReadFromEdit(IDC_FLEET_ID)-1;
	
	// Position
	PlaniPos p;
	sim.GetOwnPosition(p, FleetID);
	genstring pos = p.ToString();
	SetDlgItemText(g_hwndDlg, IDC_OWN_POS, pos.c_str());

	// Technologies
	sim.GetTechs(&techs[0], &techs[1], FleetID);
	if(doAtt) {
		if(techs[0].Weapon)
			SetDlgItemInt(g_hwndDlg, IDC_WAFF_A, techs[0].Weapon, false);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_WAFF_A, WM_SETTEXT, 0, (LPARAM)_T(""));
		
		if(techs[0].Shield)
			SetDlgItemInt(g_hwndDlg, IDC_SCHILD_A, techs[0].Shield, false);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_A, WM_SETTEXT, 0, (LPARAM)_T(""));
		
		if(techs[0].Armour)
			SetDlgItemInt(g_hwndDlg, IDC_PANZ_A, techs[0].Armour, false);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_PANZ_A, WM_SETTEXT, 0, (LPARAM)_T(""));
	}
	if(doDef) {
		if(techs[1].Weapon)
			SetDlgItemInt(g_hwndDlg, IDC_WAFF_V, techs[1].Weapon, false);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_WAFF_V, WM_SETTEXT, 0, (LPARAM)_T(""));
		
		if(techs[1].Shield)
			SetDlgItemInt(g_hwndDlg, IDC_SCHILD_V, techs[1].Shield, false);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_V, WM_SETTEXT, 0, (LPARAM)_T(""));
		
		if(techs[1].Armour)
			SetDlgItemInt(g_hwndDlg, IDC_PANZ_V, techs[1].Armour, false);
		else
			SendDlgItemMessage(g_hwndDlg, IDC_PANZ_V, WM_SETTEXT, 0, (LPARAM)_T(""));
	}
	// Engines
	int v = 0, i = 0, h = 0, perc;
	sim.GetSpeed(perc, v, i, h, FleetID);
	if(v)
		SetDlgItemInt(g_hwndDlg, IDC_VERBRENNUNG, v, false);
	else
		SetDlgItemText(g_hwndDlg, IDC_VERBRENNUNG, _T(""));
	if(i)
		SetDlgItemInt(g_hwndDlg, IDC_IMPULS, i, false);
	else
		SetDlgItemText(g_hwndDlg, IDC_IMPULS, _T(""));
	if(h)
		SetDlgItemInt(g_hwndDlg, IDC_HYPERRAUM, h, false);
	else
		SetDlgItemText(g_hwndDlg, IDC_HYPERRAUM, _T(""));
	int sppos = 10 - (perc / 10);
	SendDlgItemMessage(g_hwndDlg, IDC_SPEED, CB_SETCURSEL, (WPARAM)sppos, 0);
	
	// fleet    
	sim.GetSetFleet(Att, Def, FleetID);
	
	if(doAtt)
	{
		for(int i = 0; i < T_END; i++)
		{
			if(Att[i].Type == T_SAT || Att[i].Type == T_NONE || Att[i].Type >= T_SHIPEND)
				continue;
			if(Att[i].Num == 0)
				c[0] = '\0';
			else
				_stprintf(c, _T("%.0f"), Att[i].Num);
			SendDlgItemMessage(g_hwndDlg, GetEditIdByType(Att[i].Type, true), WM_SETTEXT, 0, (LPARAM)c);
		}
	}
	if(doDef)
	{
		if(FleetID == 0)
		{
			TargetInfo ti = sim.GetTargetInfo(FleetID);
			if(ti.NumABM > 0)
				SetDlgItemInt(g_hwndDlg, ID_INPUT_ABM, ti.NumABM, false);
			else
				SetDlgItemText(g_hwndDlg, ID_INPUT_ABM, _T(""));
		}
		for(int i = 0; i < T_END; i++)
		{
			// skip defender, if no 'real' defender
			if((FleetID > 0 && Def[i].Type >= T_RAK) || Def[i].Type == T_NONE)
				continue;
			if(Def[i].Num == 0)
				c[0] = '\0';
			else
				_stprintf(c, _T("%.0f"), Def[i].Num);
			SendDlgItemMessage(g_hwndDlg, GetEditIdByType(Def[i].Type, false), WM_SETTEXT, 0, (LPARAM)c);
		}
	}
	return;
}

void EnableAllWindows(bool how)
{
	int val = how ? FALSE : TRUE;
	// fleet
	for(int i = 0; i < T_END; i++)
	{

		if(i < T_SHIPEND && i != T_SAT)
			SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, true), EM_SETREADONLY, val, 0);
		SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, false), EM_SETREADONLY, val, 0);
	}
	// techs
	SendDlgItemMessage(g_hwndDlg, IDC_WAFF_A, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_WAFF_V, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_A, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_V, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_PANZ_A, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_PANZ_V, EM_SETREADONLY, val, 0);
	
	SendDlgItemMessage(g_hwndDlg, IDC_VERBRENNUNG, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_IMPULS, EM_SETREADONLY, val, 0);
	SendDlgItemMessage(g_hwndDlg, IDC_HYPERRAUM, EM_SETREADONLY, val, 0);

	// acs slot
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_PREV_FL), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_NEXT_FL), how);
	SendDlgItemMessage(g_hwndDlg, IDC_FLEET_ID, EM_SETREADONLY, val, 0);

	// raid list
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_NEXT_TARGET), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_PREV_TARGET), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_REMOVETARGETINFO), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_ADD_TARGETINFO), how);

	// fleet buttons
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_RESET_ATT), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_RESET_DEF), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_SWITCH_FLEET), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_DELWAVESTATE), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_LOAD_REM_DEF), how);

	// options
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_USE_SKIN), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_CHG_SKIN), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_USE_LANG), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_CHG_LANG), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_RAPID), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_DEFTF), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_HOOK_CB), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_BILANZ), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_BW_CASE), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_SHOW_BW), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_CHECKTECHS), how);

	// beneath options
	SendDlgItemMessage(g_hwndDlg, IDC_GEN_READ, EM_SETREADONLY, val, 0);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_READ_GEN), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_DEL_GEN), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_SPEED), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_OWN_POS), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_DEL_COORD), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_NUM_SIM), how);
	
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_IS_MOON), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_EDIT_SPIO), how);

	// bottom buttons
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_UPDATE), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_SHOW_KB), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_CLIP_COPY), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_SHOWPOPUP), how);
	EnableWindow(GetDlgItem(g_hwndDlg, IDC_EXIT), how);
	
	return;
}

void ResetDef()
{
	TCHAR c = '\0';

	for(int i = 0; i < T_END; i++)
		SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, false), WM_SETTEXT, 0, (LPARAM)c);

	SendDlgItemMessage(g_hwndDlg, IDC_WAFF_V, WM_SETTEXT, 0, (LPARAM)c);
	SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_V, WM_SETTEXT, 0, (LPARAM)c);
	SendDlgItemMessage(g_hwndDlg, IDC_PANZ_V, WM_SETTEXT, 0, (LPARAM)c);
}

void ResetAtt()
{
	TCHAR c = '\0';
	for(int i = 0; i < T_SHIPEND; i++)
	{
		if(i == T_SAT)
			continue;
		SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, true), WM_SETTEXT, 0, (LPARAM)c);
	}

	SendDlgItemMessage(g_hwndDlg, IDC_WAFF_A, WM_SETTEXT, 0, (LPARAM)c);
	SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_A, WM_SETTEXT, 0, (LPARAM)c);
	SendDlgItemMessage(g_hwndDlg, IDC_PANZ_A, WM_SETTEXT, 0, (LPARAM)c);
}

void ClearResult()
{
	TCHAR c = '\0';
	
	for(int i = 0; i < T_END; i++)
	{
		// attacker
		if(i < T_SHIPEND && i != T_SAT)
			SendDlgItemMessage(g_hwndDlg, GetLabelIdByType((ITEM_TYPE)i, true), WM_SETTEXT, 0, (LPARAM)c);
		// defender
		SendDlgItemMessage(g_hwndDlg, GetLabelIdByType((ITEM_TYPE)i, false), WM_SETTEXT, 0, (LPARAM)c);
	}
}

void __cdecl TestNewerVersion(void* p)
{
	genstring INetText, tmp;
	genstring::size_type f, f2;
	
	bool bNewVer = true;
	TCHAR dlfile[128];
	// if language file loaded, download english changelog
	if(!g_Options.LangFileName || !strlen(g_Options.LangFileName))
		_tcscpy(dlfile, SPEEDSIM_UPDATE_FILE);
	else
		_tcscpy(dlfile, SPEEDSIM_UPDATE_FILE_ENG);
	if(!GetUpdateFile(INetText, dlfile))
		INetText = g_UpdaterStr[0];
	else {
		// only show newer versions
		TCHAR ver[128];
		f = INetText.find(_T("SpeedSim.exe"), 0);
		f2 = INetText.find(_T("SpeedKernel.dll"), 0);
		if(f2 == genstring::npos || f == genstring::npos) {
			SendDlgItemMessage(g_hwndUpd, IDC_UPD_OUT, WM_SETTEXT, 0, (LPARAM)_T("Error parsing update file!"));
			return;
		}
		genstring ssdata = INetText.substr(f, f2-f);
		genstring kerneldata = INetText.substr(f2);
		bool isnewer = false;
		// read latest version from server
		f = ssdata.find(_T("v"));
		f2 = ssdata.find(_T("b"), f);
		if(f != genstring::npos && f2 != genstring::npos)
		{
			f++;
			genstring v = ssdata.substr(f, f2 - f);
			if(CompareVersions(SPEEDSIM_VERSION, v.c_str()) > 0)
			{
				isnewer = true;
				tmp = ssdata.substr(0, f - 1);
				tmp += g_UpdaterStr[1];
				tmp += _T("\n\n");
			}
		}
		if(!isnewer)
		{
			_tcscpy(ver, SPEEDSIM_VERSION);
			_tcscat(ver, _T("\n"));
			f = ssdata.find(ver);
			tmp = ssdata.substr(0, f-1);
			if(tmp[tmp.length() - 4] == _T('~')) {
				tmp += g_UpdaterStr[1];
				tmp += _T("\n\n");
				bNewVer = false;
			}
		}
		isnewer = false;
		sim.GetVersion(ver);
		f = kerneldata.find(_T("v"));
		f2 = kerneldata.find(_T("b"), f-1);
		if(f != genstring::npos && f2 != genstring::npos)
		{
			f++;
			genstring v = kerneldata.substr(f, f2 - f);
			if(CompareVersions(ver, v.c_str()) > 0)
			{
				isnewer = true;
				tmp += kerneldata.substr(0, f - 1);
				tmp += g_UpdaterStr[1];
				tmp += _T("\n\n");
			}
		}
		if(!isnewer)
		{
			sim.GetVersion(ver);
			_tcscat(ver, _T("\n"));
			f = kerneldata.find(ver);
			tmp += kerneldata.substr(0, f);
			if(tmp[tmp.length() - 4] == _T('~')) {
				tmp += g_UpdaterStr[1];
				bNewVer = false;
			}
		}
		f = 0;
		while(f != genstring::npos) {
			if((f = tmp.find(_T("\n"), f)) != genstring::npos) {
				tmp.erase(f, 1);
				tmp.insert(f, _T("\r\n"));
				f += 2;
			}
		}
	}
	SendDlgItemMessage(g_hwndUpd, IDC_UPD_OUT, WM_SETTEXT, 0, (LPARAM)tmp.c_str());
	SendDlgItemMessage(g_hwndUpd, IDC_START_UPD, WM_SETTEXT, 0, (LPARAM)_T("Update-Check"));
}

void __cdecl AutoUpdate(void* p)
{
	genstring INetText, SpeedVer, KernVer;
	
	TCHAR tmp[16];
	if(!_tcscmp(g_Options.LastUpdCheck, _tstrdate(tmp)))
		return;

	_tcscpy(g_Options.LastUpdCheck, _tstrdate(tmp));
	if(!GetUpdateFile(INetText))
		return;

	string::size_type f, f2;
	sim.GetVersion(tmp);
	f = INetText.find(_T("v")) + 1;
	f2 = INetText.find(_T("\n"), f);
	SpeedVer = INetText.substr(f, f2 - f);
	f = INetText.find(_T("SpeedKernel.dll"));
	f = INetText.find(_T("v"), f) + 1;
	f2 = INetText.find(_T("\n"), f);
	KernVer = INetText.substr(f, f2 - f);
	int kernver = CompareVersions(KernVer.c_str(), tmp);
	int guiver = CompareVersions(SpeedVer.c_str(), SPEEDSIM_VERSION);
	if(kernver <= 0 && guiver <= 0) {
		SaveOpts(g_CurSaveData);
		return;
	}

	TCHAR Ausgabe[512];
	_stprintf(Ausgabe, _T("%s\n\n\t%s\n") 
		_T("GUI:\t%s\t\t%s\n")
		_T("Kernel:\t%s\t\t%s\n%s"), 
		g_UpdaterStr[2].c_str(), g_UpdaterStr[3].c_str(), SPEEDSIM_VERSION, SpeedVer.c_str(), tmp, KernVer.c_str(),
		g_UpdaterStr[4].c_str());

	int r = MessageBox(g_hwndDlg, Ausgabe, _T("AutoUpdate"), MB_YESNO);
	if(r == IDYES)
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_UPDATE), g_hwndDlg, UpdateProc);
	/*else
		g_LastUpdCheck[0] = 0;*/

	SaveOpts(g_CurSaveData);
}

bool GetUpdateFile(genstring &out, TCHAR *UpdFileName /*=SPEEDSIM_UPDATE_FILE*/, TCHAR* UpdServer /*=SPEEDSIM_SERVER*/) {
	out.erase();
	string dat;
	bool bSendReq = false;
	HINTERNET hINet = NULL, hINetConn = NULL, hINetReq = NULL;
	DWORD code = HTTP_STATUS_OK;
	
	// open internet connection and request file
	hINet = InternetOpen(_T("SPEEDSIM-Updater"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	if(hINet)
		hINetConn = InternetConnect(hINet, UpdServer, INTERNET_DEFAULT_HTTP_PORT, _T(""), _T(""), INTERNET_SERVICE_HTTP, NULL, NULL);
	if(hINetConn)
	{
		hINetReq = HttpOpenRequest(hINetConn, NULL, UpdFileName, NULL, NULL, NULL, 
			INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, NULL);
		// get status code
		/*DWORD size = sizeof(DWORD);
		HttpQueryInfo(hINetReq, HTTP_QUERY_STATUS_CODE, &code, &size, NULL);*/
	}
	if(hINetReq /*&& code == HTTP_STATUS_OK*/)
		bSendReq = (HttpSendRequest(hINetReq, NULL, NULL, NULL, NULL) == TRUE);
	
	if(bSendReq) {
		DWORD dwNumBytesAvail = 0;
		do {
			InternetQueryDataAvailable(hINetReq, &dwNumBytesAvail, NULL, NULL);
			if(dwNumBytesAvail > 0) {
				// get data
				char* buff = new char[dwNumBytesAvail + 1];
				DWORD dwBytesRead;
				InternetReadFile(hINetReq, buff, dwNumBytesAvail, &dwBytesRead);
				buff[dwBytesRead] = '\0';

				dat += buff;
				delete buff;
			}
		} while(dwNumBytesAvail > 0);
	}
	
	// close all handles
	if (hINetReq)
		InternetCloseHandle(hINetReq);
	if (hINetConn)
		InternetCloseHandle(hINetConn);
	if (hINet)
		InternetCloseHandle(hINet);
	if(!bSendReq)
		return false;
	
	TCHAR *file = new TCHAR[dat.length()+1];
#ifdef UNICODE
	mbstowcs(file, dat.c_str(), dat.length()+1);
#else
	memcpy(file, dat.c_str(), dat.length()+1);
#endif
	out = file;
	delete file;
	return true;
}

void ReadSkinIni(TCHAR* inifile) {
	TCHAR buffer[64], buffer2[64];
	TCHAR path[MAX_PATH];
	_tcscpy(path, g_CurrDir);
	_tcscat(path, inifile);
	LOGBRUSH br{};
	br.lbStyle = BS_SOLID;
	br.lbHatch = 0;

	if(GetPrivateProfileString(_T("Colors"), _T("Button_TextColor"), NULL, buffer, 64, path))
		g_Skin.SetColor(SCWND_ALLBTNS, SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("Colors"), _T("Button_TransColor"), NULL, buffer, 64, path))
		g_Skin.SetBtnTransparentColor(StrToRGB(buffer));
	if(GetPrivateProfileString(_T("Colors"), _T("Edit_TextColor"), _T(""), buffer, 64, path))
		g_Skin.SetColor(SCWND_ALLEDITS, SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("Colors"), _T("CheckBox_TextColor"), _T(""), buffer, 64, path))
		g_Skin.SetColor(SCWND_ALLCHK, SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("Colors"), _T("Label_TextColor"), _T(""), buffer, 64, path))
		g_Skin.SetColor(SCWND_ALLLABELS, SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("Colors"), _T("GroupBox_TextColor"), _T(""), buffer, 64, path ))
		g_Skin.SetColor(SCWND_ALLGROUPBOXES, SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("Colors"), _T("GroupBox_FrameColor"), _T(""), buffer, 64, path))
		g_Skin.SetColor(SCWND_ALLGROUPBOXES, SCMODE_BKCOLOR, StrToRGB(buffer));

	GetPrivateProfileString(_T("Colors"), _T("Edit_BG_Mode"), _T(""), buffer, 64, path);
	if(!_tcscmp(buffer, _T("TRANSPARENT")))
		g_Skin.SetColor(SCWND_ALLEDITS, SCMODE_BKMODE, TRANSPARENT);
	else {
		GetPrivateProfileString(_T("Colors"), _T("Edit_BG_Color"), _T(""), buffer, 64, path);
		br.lbStyle = BS_SOLID;
		br.lbColor = StrToRGB(buffer);
		g_Skin.SetColor(SCWND_ALLEDITS, SCMODE_BRUSH, br.lbColor );
	}
	GetPrivateProfileString(_T("Colors"), _T("CheckBox_BG_Mode"), _T(""), buffer, 64, path);
	if(!_tcscmp(buffer, _T("TRANSPARENT")))
		g_Skin.SetColor(SCWND_ALLCHK, SCMODE_BKMODE, TRANSPARENT);
	else {
		GetPrivateProfileString(_T("Colors"), _T("CheckBox_BG_Color"), _T(""), buffer, 64, path);
		br.lbStyle = BS_SOLID;
		br.lbColor = StrToRGB(buffer);
		g_Skin.SetColor(SCWND_ALLCHK, SCMODE_BRUSH, br.lbColor );
	}
	GetPrivateProfileString(_T("Colors"), _T("Label_BG_Mode"), _T(""), buffer, 64, path);
	if(!_tcscmp(buffer, _T("TRANSPARENT")))
		g_Skin.SetColor(SCWND_ALLLABELS, SCMODE_BKMODE, TRANSPARENT);
	else {
		GetPrivateProfileString(_T("Colors"), _T("Label_BG_Color"), _T(""), buffer, 64, path);       
		br.lbStyle = BS_SOLID;
		br.lbColor = StrToRGB(buffer);
		g_Skin.SetColor(SCWND_ALLLABELS, SCMODE_BRUSH, br.lbColor);
	}

	// read out label colours
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Attacker"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_ATT1), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_ATT2), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_ATT3), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Defender"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_DEF1), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_DEF2), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_DEF3), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Ships"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S1), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S2), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S3), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S4), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S5), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S6), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S7), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S8), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S9), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S10), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S11), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S12), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S13), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S14), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S15), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S16), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S17), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Ships_Res_Att"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KT_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_GT_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_LJ_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SJ_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KR_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KOL_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_REC_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SPIO_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_BOMB_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_ZERR_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_TS_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_IC_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_REAP_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_PF_A_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, ID_OUTPUT_NEEDED_IPM), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, ID_OUTPUT_IPM), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Ships_Res_Def"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KT_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_GT_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_LJ_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SJ_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KR_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KOL_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_REC_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SPIO_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_BOMB_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SPIO_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_ZERR_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_TS_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_IC_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_REAP_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_PF_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, ID_OUTPUT_ABM), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Ships_Res_Def2"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_RAK_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_LL_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SL_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_IO_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_GA_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_PLASMA_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_KS_V_E), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_GS_V_E), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Defense"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D1), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D2), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D3), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D4), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D5), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D6), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D7), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_D8), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, ID_NAME_IPM), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Techs"), _T(""), buffer, 64, path))
	{
		COLORREF col = StrToRGB(buffer);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_T1), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_T2), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_T3), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_VERBR), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_IMPULS), SCMODE_COLOR, col);
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_HYPER), SCMODE_COLOR, col);
	}
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Scan"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_SCAN_END), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Win"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_END), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_TF"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_TF), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Loss_Att"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_VERL_A), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Loss_Def"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_VERL_V), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Moon"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_MOND), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Th_Beut"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_BEUT_TH), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Pr_Beut"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_BEUT_PRAK), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Sprit"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_FLYCOST), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Welle"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_BEUT_GES), SCMODE_COLOR, StrToRGB(buffer));
	
	// label colors for result titles
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_Res_Titles"), _T(""), buffer, 64, path))
	{
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_SCAN), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_END), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_TF), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_MOON), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_LOSS_ATT), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_LOSSDEF), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_THBEUT), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_PRAKBEUT), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_SPRIT), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_FLYTIME), SCMODE_COLOR, StrToRGB(buffer));
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_WELLBEUT), SCMODE_COLOR, StrToRGB(buffer));
	}
	
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_ReadInTitle"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_GEN_READ), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_NumSims"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_NUMSIM), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_OwnPos"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_POS), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_ACSSlot"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_FLEETS), SCMODE_COLOR, StrToRGB(buffer));
	if(GetPrivateProfileString(_T("LabelColors"), _T("LC_FleetSpeed"), _T(""), buffer, 64, path))
		g_Skin.SetColor(GetDlgItem(g_hwndDlg, IDC_S_FLEETSPEED), SCMODE_COLOR, StrToRGB(buffer));

	// ---------------------Font----------------
	TCHAR Font[128];
//#ifdef UNICODE
	//_tcscpy(Font, _T("Microsoft Sans Serif"));
	_tcscpy(Font, _T("MS Shell Dlg"));
/*#else
	_tcscpy(Font, _T("MS Sans Serif"));
#endif*/

	int FSize = 8;
	DWORD format = 0;
	if(GetPrivateProfileString(_T("Font"), _T("FontName"), _T(""), buffer, 64, path )) {
		_tcscpy(Font, buffer);
	}
	if(GetPrivateProfileString(_T("Font"), _T("FontHeight"), _T(""), buffer, 64, path))
		FSize = _ttoi(buffer);
	if(GetPrivateProfileString(_T("Font"), _T("FF_Ships_Techs"), _T(""), buffer, 64, path )) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S1), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S2), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S3), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S4), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S5), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S6), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S7), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S8), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S9), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S10), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S11), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S12), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S13), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S14), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S15), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S16), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S17), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D1), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D2), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D3), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D4), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D5), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D6), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D7), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_D8), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_T1), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_T2), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_T3), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_VERBR), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_IMPULS), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_HYPER), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, ID_NAME_IPM), Font, FSize, format);
	
	format = 0;
	if(GetPrivateProfileString(_T("Font"), _T("FF_Titles"), _T(""), buffer, 64, path)) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_ATT1), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_ATT2), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_ATT3), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_DEF1), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_DEF2), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_DEF3), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_ENGINES), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_FLEETS), Font, FSize, format);
	format = 0;

	if(GetPrivateProfileString(_T("Font"), _T("FF_Result"), _T(""), buffer, 64, path)) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_SCAN_END), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_END), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_TF), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_VERL_A), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_VERL_V), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_MOND), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_BEUT_TH), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_BEUT_PRAK), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_FLYCOST), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_BEUT_GES), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_SCAN), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_END), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_TF), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_LOSS_ATT), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_LOSSDEF), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_THBEUT), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_PRAKBEUT), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_SPRIT), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_FLYTIME), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_WELLBEUT), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_MOON), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_FLUGZEIT), Font, FSize, format);
	
	format = 0;
	if(GetPrivateProfileString(_T("Font"), _T("FF_Options"), _T(""), buffer, 64, path )) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_USE_SKIN), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_USE_LANG), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_BW_CASE), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_RAPID), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_DEFTF), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_CHECKTECHS), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_HOOK_CB), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_OLDSHIELD), Font, FSize, format);

	format = 0;
	if(GetPrivateProfileString(_T("Font"), _T("FF_Buttons"), _T(""), buffer, 64, path )) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(SCWND_ALLBTNS, Font, FSize, format);

	format = 0;
	if(GetPrivateProfileString(_T("Font"), _T("FF_Misc"), _T(""), buffer, 64, path)) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_GEN_READ), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_NUMSIM), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_RUNNING_NUM), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_FLEETSPEED), Font, FSize, format);
	g_Skin.SetFont(GetDlgItem(g_hwndDlg, IDC_S_POS), Font, FSize, format);

	format = 0;
	if(GetPrivateProfileString(_T("Font"), _T("FF_GroupBoxes"), _T(""), buffer, 64, path )) {
		if(_tcsstr(buffer, _T("BOLD")))
			format |= FF_BOLD;
		if(_tcsstr(buffer, _T("ITALIC")))
			format |= FF_ITALIC;
		if(_tcsstr(buffer, _T("UNDERLINE")))
			format |= FF_UNDERLINE;
		if(_tcsstr(buffer, _T("STRIKEOUT")))
			format |= FF_STRIKEOUT;
	}
	g_Skin.SetFont(SCWND_ALLGROUPBOXES, Font, FSize, format);

	TCHAR name[MAX_PATH], name2[MAX_PATH];
	TCHAR *c1, *c2;

	u_long l = MAX_PATH;
	UrlCreateFromPath(g_CurrDir, name, &l, NULL);
	l = MAX_PATH;
	UrlCreateFromPath(g_CurrDir, name2, &l, NULL);

	if(GetPrivateProfileString(_T("General"), _T("CSS_CR"), NULL, buffer, 64, path)) {
		_tcscat(name, buffer);
		c1 = &name[0];
	}
	else
	{
		_tcscat(name, _T("cr.css"));
		c1 = &name[0];
	}
		
	if(GetPrivateProfileString(_T("General"), _T("CSS_BWC"), NULL, buffer2, 64, path)) {
		_tcscat(name2, buffer2);
		c2 = &name2[0];
	}
	else
	{
		_tcscat(name2, _T("bwc.css"));
		c2 = &name2[0];
	}
	
	sim.SetCSSFiles(c1, c2);
	//ActivateIPMMode(g_IPMMode);

	InvalidateRect(g_hwndDlg, NULL, TRUE);
	return;
}

void LoadLang(char* langfile) {
	
	if(langfile)
	{
		// validate file
		FILE* f = fopen(langfile, "r");
		if(!f) {
			return;
		}
		fclose(f);
	}
	sim.LoadLangFile(langfile);
	TCHAR FleetItems[T_END][128]{};
	
	for(int i = 0; i < T_END; i++)
		sim.ItemName((ITEM_TYPE)i, FleetItems[i]);
	// ship names
	SetDlgItemText(g_hwndDlg, IDC_S1, FleetItems[T_KT]);
	SetDlgItemText(g_hwndDlg, IDC_S2, FleetItems[T_GT]);
	SetDlgItemText(g_hwndDlg, IDC_S3, FleetItems[T_LJ]);
	SetDlgItemText(g_hwndDlg, IDC_S4, FleetItems[T_SJ]);
	SetDlgItemText(g_hwndDlg, IDC_S5, FleetItems[T_KREUZER]);
	SetDlgItemText(g_hwndDlg, IDC_S6, FleetItems[T_SS]);
	SetDlgItemText(g_hwndDlg, IDC_S7, FleetItems[T_KOLO]);
	SetDlgItemText(g_hwndDlg, IDC_S8, FleetItems[T_REC]);
	SetDlgItemText(g_hwndDlg, IDC_S9, FleetItems[T_SPIO]);
	SetDlgItemText(g_hwndDlg, IDC_S10, FleetItems[T_BOMBER]);
	SetDlgItemText(g_hwndDlg, IDC_S11, FleetItems[T_SAT]);
	SetDlgItemText(g_hwndDlg, IDC_S12, FleetItems[T_ZER]);
	SetDlgItemText(g_hwndDlg, IDC_S13, FleetItems[T_TS]);
	SetDlgItemText(g_hwndDlg, IDC_S14, FleetItems[T_IC]);
	SetDlgItemText(g_hwndDlg, IDC_S15, FleetItems[T_REAP]);
	SetDlgItemText(g_hwndDlg, IDC_S16, FleetItems[T_PF]);
	SetDlgItemText(g_hwndDlg, IDC_S17, FleetItems[T_CRA]);
	// defense
	SetDlgItemText(g_hwndDlg, IDC_D1, FleetItems[T_RAK]);
	SetDlgItemText(g_hwndDlg, IDC_D2, FleetItems[T_LL]);
	SetDlgItemText(g_hwndDlg, IDC_D3, FleetItems[T_SL]);
	SetDlgItemText(g_hwndDlg, IDC_D4, FleetItems[T_GAUSS]);
	SetDlgItemText(g_hwndDlg, IDC_D5, FleetItems[T_IONEN]);
	SetDlgItemText(g_hwndDlg, IDC_D6, FleetItems[T_PLASMA]);
	SetDlgItemText(g_hwndDlg, IDC_D7, FleetItems[T_KS]);
	SetDlgItemText(g_hwndDlg, IDC_D8, FleetItems[T_GS]);
	if(!langfile) {
		// german language strings
		g_strSim[0] = _T("Kampf simulieren");
		g_strSim[1] = _T("Initialisiere Simulation");
		g_strSim[2] = _T("Simulation anhalten");
		g_strSimState[0] = _T("Warte...");
		g_strSimState[1] = _T("Simuliere Durchlauf %d ; Runde %d");
		g_ResString[0] = _T("| Angreifer gewinnt (%.0f%%) |");
		g_ResString[1] = _T("%s| Verteidiger gewinnt (%.0f%%) |");
		g_ResString[2] = _T("%s| Unentschieden (%.0f%%) |");
		g_ResString[3] = _T("%s nach ~ %.0f Runden");
		g_ResString[4] = _T("%s Metall (%.0f%%), %s Kristall (%.0f%%) max. ~ %d Recycler");
		g_ResString[5] = _T("%s Metall, %s Kristall und %s Deuterium");
		g_ResString[6] = _T("%s (%i%% Ausbeute)");
		g_ResString[7] = _T("%s ~ %d %s");
		g_ResString[8] = _T("Die Chance einer Mondentstehung beträgt %d%%");
		g_ResString[9] = _T("Zu wenig Trümmer vorhanden.");
		g_ResString[10] = _T("Unbekannt [x:xxx:xx]");
		g_ResString[11] = _T("Kampf auf %s");
		// fuel
		g_ResString[12] = _T("%s Deuterium");
		g_ResString[13] = _T("%s nach %d Angriffen (max. %d Recycler)");
		g_ResString[14] = _T("Recycler -> Max: %d | Durchschnitt: %d | Min: %d");
		g_BilanzStrings[0] = _T("Bilanz nach Angriff auf %s");
		g_BilanzStrings[1] = _T("Ohne Einsammeln des TF");
		g_BilanzStrings[2] = _T("Beim Einsammeln des halben TF");
		g_BilanzStrings[3] = _T("Beim Einsammeln des gesamten TF");
		g_BilanzStrings[4] = _T("Gesamtgewinn (Wellenangriffe)");
		g_ResNames[0] = _T("Metall");
		g_ResNames[1] = _T("Kristall");
		g_ResNames[2] = _T("Deuterium");
		g_UpdaterStr[0] = _T("Fehler beim Verbinden!");
		g_UpdaterStr[1] = _T("Aktuellste Version wird verwendet.");
		g_UpdaterStr[2] = _T("Du verwendest eine veraltete SpeedSim-Version!");
		g_UpdaterStr[3] = _T("Deine Version\tVersion im INet");
		g_UpdaterStr[4] = _T("Möchtest du jetzt das Update-Fenster aufrufen?");
		g_UpdaterStr[5] = _T("Täglich auf Update prüfen");
		g_UpdaterStr[6] = _T("Update-Check");
		g_MiscStr[0] = _T("Die Änderungen werden erst nach einem Neustart von Speedsim wirksam");
		g_MiscStr[1] = _T("SpeedSim jetzt neustarten?");
		g_MiscStr[2] = _T("Schließen");
		g_MiscStr[3] = _T("Abbrechen");
		g_MiscStr[4] = _T("Bearbeiten");
		g_MiscStr[5] = _T("Die Datei liegt nicht im SpeedSim-Ordner - Vorgang wird abgebrochen");
		g_MiscStr[6] = _T("Achtung - möglicherweise ungetestete/falsche Werte die zu inkorrekten Kampfergebnissen führen können");
		g_MiscStr[7] = _T("Optionen");
		g_MiscStr[8] = _T("Speichernamen eingeben:");
		g_IPModeStr[0] = _T("Primärziel");
		g_IPModeStr[1] = _T("IP- / Abwehrraketen");
		g_IPModeStr[2] = _T("Ergebnis übernehmen");
		g_IPModeStr[3] = _T("IPRak-Modus an");
		g_IPModeStr[4] = _T("IPRak-Modus aus");
		g_ScanEditStr[0] = _T("Wellenstatus löschen");
		g_ScanEditStr[1] = _T("Position");
		g_ScanEditStr[2] = _T("Ressoucen");
		g_ScanEditStr[3] = _T("Spionagebericht");
		g_ScanEditStr[4] = _T("Ist das Ziel ein Mond?");
		g_ScanEditStr[5] = _T("Planetenname");
		g_OptPage1[0] = _T("Wiederaufbauwert der Verteidigung");
		g_OptPage1[1] = _T("Neue Treibstoffberechnung verwenden (v0.68a)");
		g_OptPage1[2] = _T("-fache Geschwindigkeit");
		g_OptPage1[3] = _T("SpeedSim");
		g_OptPage1[4] = _T("Beim Minimieren ins Systray verkleinern");
		g_OptPage1[5] = _T("Popup, wenn etwas aus OGame eingelesen wurde");
		g_OptPage1[6] = _T("(funktioniert nur, wenn Zwischenablage überwacht wird)");
		g_OptPage1[7] = _T("Zerstörte Verteidigung kleiner 10 vollständig wiederaufbauen");
		g_OptPage1[8] = _T("Transparenz für PopUp");
		g_OptPage1[9] = _T("Transparenz für SpeedSim (Hauptfenster)");
		g_OptPage1[10] = _T("Transparenz (nur Win 2k/XP)");
		g_OptPage1[8] = _T("Transparenz für PopUp");
		g_OptPage2[0] = _T("OGame");
		g_OptPage2[1] = _T("SpeedSim mit Windows starten");
		g_OptPage2[2] = _T("Beim Start minimieren");
		g_OptPage2[3] = _T("Nur eine Instanz von SpeedSim erlauben");
		g_OptPage2[4] = _T("Schiffstyp zur Berechnung der theroretischen Beute:");
		g_OptPage2[5] = _T("Wellenangriffe");
		g_OptPage2[6] = _T("Verteidigung Aufbauen basieren auf...");
		g_OptPage2[7] = _T("Durchschnitt");
		g_OptPage2[8] = _T("besten Fall");
		g_OptPage2[9] = _T("schlechtesten Fall");
		g_OptPage2[10] = _T("Alte Schlachtschiffdaten verwenden");
		g_OptPage2[11] = _T("% der Strukturpunkte gehen ins TF (normal: 30%)");
		g_OptPage3[0] = _T("Ideenplanet");
		g_OptPage3[1] = _T("Eigenes RF");
		g_OptPage3[2] = _T("Eigene Schiffsdaten");
		g_OptPage3[3] = _T("Gelade RF-Datei:");
		g_OptPage3[4] = _T("Geladene SD-Datei:");
		g_TrayText[0] = _T("Überwachen der Zwischenablage: ");
		g_TrayText[1] = _T("PopUp bei OGame-Text: ");
		g_TrayText[2] = _T("an");
		g_TrayText[3] = _T("aus");
		g_TrayText[4] = _T("Wiederherstellen");
		g_TrayText[5] = _T("SpeedSim beenden");
		g_TrayText[6] = _T("Info");
		g_PopupStrings[0] = _T("Start");
		g_PopupStrings[1] = _T("Ziel");
		g_PopupStrings[2] = _T("[Schließen mit Klick]");
		g_PopupStrings[3] = _T("Max. Recycler für TF");
		g_PopupStrings[4] = _T("M");
		g_EspHistStrings[0] = _T("Letzte Spionageberichte");
		g_EspHistStrings[1] = _T("Name/Koordinaten");
		g_EspHistStrings[2] = _T("Hinzugefügt");
		g_EspHistStrings[3] = _T("Doppelklick, um Bericht zu laden");
		g_EspHistStrings[4] = _T("Verlauf deaktivieren");
		g_EspHistStrings[5] = _T("Markierte löschen");
		g_EspHistStrings[6] = _T("Schließen");
		g_EspHistStrings[7] = _T("Angegriffen");
		g_EspHistStrings[8] = _T("Als 'angegriffen' markieren");
		g_EspHistStrings[9] = _T("'Angegriffen'-Markierung entfernen");
		g_EspHistStrings[10] = _T("Berichte speichern");
		g_EspHistStrings[11] = _T("Anzahl");
		g_TechNames[0] = _T("Waffentechnik");
		g_TechNames[1] = _T("Schildtechnik");
		g_TechNames[2] = _T("Raumschiffpanzerung");
		g_TechNames[3] = _T("Verbrennungstriebwerk");
		g_TechNames[4] = _T("Impulstriebwerk");
		g_TechNames[5] = _T("Hyperraumantrieb");
		g_EspTitles[0] = _T("Ressourcen");
		g_EspTitles[1] = _T("Flotte");
		g_EspTitles[2] = _T("Verteidigung");
		g_EspTitles[3] = _T("Technologien");
		
		if(g_hwndEspHist)
		{
			bool vis = false;
			if(IsWindowVisible(g_hwndEspHist))
				vis = true;
			EndDialog(g_hwndEspHist, 0);
			CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_ESP_REPORTS), g_hwndDlg, ReportHistProc);
			if(vis)
				ShowWindow(g_hwndEspHist, SW_SHOWNORMAL);
		}
		SetGerLang(g_hwndDlg, g_GerStringsMain);
		InitMenu();
		InvalidateRect(g_hwndDlg, NULL, true);
		return;
	}
	TCHAR inipath[MAX_PATH];
	TCHAR buff[256];
	_tcscpy(inipath, g_CurrDir);
	TCHAR file[256];
#ifdef UNICODE    
	mbstowcs(file, langfile, 256);
#else
	strcpy(file, langfile);
#endif
	_tcscat(inipath, file);

	if(GetPrivateProfileString(_T("General"), _T("FLEETID"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_S_FLEETS, buff);
	}
	if(GetPrivateProfileString(_T("General"), _T("ATT"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_ATT1, buff);
		SetDlgItemText(g_hwndDlg, IDC_ATT2, buff);
		SetDlgItemText(g_hwndDlg, IDC_ATT3, buff);
	}
	if(GetPrivateProfileString(_T("General"), _T("DEF"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_DEF1, buff);
		SetDlgItemText(g_hwndDlg, IDC_DEF2, buff);
		SetDlgItemText(g_hwndDlg, IDC_DEF3, buff);
	}
	// technologies
	if(GetPrivateProfileString(_T("Techs"), _T("WEAP"), _T(""), buff, 256, inipath))
	{
		SetDlgItemText(g_hwndDlg, IDC_T1, buff);
		g_TechNames[0] = buff;
	}
	if(GetPrivateProfileString(_T("Techs"), _T("SHIELD"), _T(""), buff, 256, inipath))
	{
		SetDlgItemText(g_hwndDlg, IDC_T2, buff);
		g_TechNames[1] = buff;
	}
	if(GetPrivateProfileString(_T("Techs"), _T("HULL"), _T(""), buff, 256, inipath))
	{
		SetDlgItemText(g_hwndDlg, IDC_T3, buff);
		g_TechNames[2] = buff;
	}
	if(GetPrivateProfileString(_T("Techs"), _T("VERBR"), _T(""), buff, 256, inipath))
	{
		g_TechNames[3] = buff;
		SetDlgItemText(g_hwndDlg, IDC_S_VERBR, buff);
	}
	if(GetPrivateProfileString(_T("Techs"), _T("IMPULS"), _T(""), buff, 256, inipath))
	{
		g_TechNames[4] = buff;
		SetDlgItemText(g_hwndDlg, IDC_S_IMPULS, buff);
	}
	if(GetPrivateProfileString(_T("Techs"), _T("HYPER"), _T(""), buff, 256, inipath))
	{
		g_TechNames[5] = buff;
		SetDlgItemText(g_hwndDlg, IDC_S_HYPER, buff);
	}
	if(GetPrivateProfileString(_T("Techs"), _T("ENGINES"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_ENGINES, buff);
	// group boxes
	if(GetPrivateProfileString(_T("GUI"), _T("FLEET"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_GB_FLEET, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("DEFENSE"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_GB_DEF, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("TECHS"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_GB_TECHS, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("RESULT"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_GB_RES, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("OPTIONS"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_GB_OPT, buff);
		g_MiscStr[7] = buff;
	}
	if(GetPrivateProfileString(_T("GUI"), _T("RAIDLIST"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_GB_RAIDLIST, buff);

	// options
	if(GetPrivateProfileString(_T("GUI"), _T("RF"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_RAPID, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("USEOLDSHIELD"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_OLDSHIELD, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("OLDSHIELD"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_OLDSHIELD, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("DEFINTF"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_DEFTF, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("CHECKTECHS"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_CHECKTECHS, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("USESKIN"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_USE_SKIN, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("CHGSKIN"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_CHG_SKIN, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("USELANG"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_USE_LANG, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("CHGLANG"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_CHG_LANG, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("AUTOSHOW_BW"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_BW_CASE, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("SHOW_NOW"), _T(""), buff, 256, inipath)) 
		SetDlgItemText(g_hwndDlg, IDC_SHOW_BW, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("DELDEFOC"), _T(""), buff, 256, inipath)) 
		SetDlgItemText(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("DELWAVESTATE"), _T(""), buff, 256, inipath)) {
		g_ScanEditStr[0] = buff;
		SetDlgItemText(g_hwndDlg, IDC_DELWAVESTATE, buff);
	}
	if(GetPrivateProfileString(_T("GUI"), _T("POSITION"), _T(""), buff, 256, inipath))
		g_ScanEditStr[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("CANCEL"), _T(""), buff, 256, inipath))
		g_MiscStr[3] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("RESS"), _T(""), buff, 256, inipath))
		g_ScanEditStr[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("ESPTITLE"), _T(""), buff, 256, inipath))
		g_ScanEditStr[3] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("PLANETNAME"), _T(""), buff, 256, inipath))
		g_ScanEditStr[5] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("EDIT"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_EDIT_SPIO, buff);
	}
	if(GetPrivateProfileString(_T("GUI"), _T("NOTINSAMEDIR"), _T(""), buff, 256, inipath))
		g_MiscStr[5] = buff;

	if(GetPrivateProfileString(_T("GUI"), _T("UEBCB"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_HOOK_CB, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("BAL"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_BILANZ, buff);
	
	// reading stuff
	if(GetPrivateProfileString(_T("GUI"), _T("READ"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_READ_GEN, buff);
	}
	if(GetPrivateProfileString(_T("GUI"), _T("DELETE"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_DEL_GEN, buff);        
	}
	if(GetPrivateProfileString(_T("GUI"), _T("READFIELD"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_GEN_READ, buff);

	// own pos, simulation
	if(GetPrivateProfileString(_T("GUI"), _T("OWNPOS"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_POS, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("NUMSIM"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_NUMSIM, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("STARTSIM"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_SIM, buff);
		g_strSim[0] = buff;
	}
	if(GetPrivateProfileString(_T("GUI"), _T("INITSIM"), _T(""), buff, 256, inipath))
		g_strSim[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("STOPSIM"), _T(""), buff, 256, inipath))
		g_strSim[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("WAIT"), _T(""), buff, 256, inipath)) {
		g_strSimState[0] = buff;
		SetDlgItemText(g_hwndDlg, IDC_RUNNING_NUM, buff);
	}
	if(GetPrivateProfileString(_T("GUI"), _T("SIMSTATE"), _T(""), buff, 256, inipath))
		g_strSimState[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("NEXTWAVE"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_LOAD_REM_DEF, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("FLEETSPEED"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_FLEETSPEED, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("ENTERSLOTNAME"), _T(""), buff, 256, inipath))
		g_MiscStr[8] = buff;
	
	// battle result
	if(GetPrivateProfileString(_T("GUIRes"), _T("SCAN"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_SCAN, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("STATS"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_END, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("RES_TF"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_TF, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("LOSS_ATT"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_LOSS_ATT, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("LOSS_DEF"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_LOSSDEF, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("MOONCHANCE"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_MOON, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("PLUNDER_TH"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_THBEUT, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("PLUNDER_PR"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_PRAKBEUT, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("FUEL"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_SPRIT, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("PLUNDER_WAVE"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_WELLBEUT, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("FLYTIME"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_S_FLYTIME, buff);
	if(GetPrivateProfileString(_T("GUIRes"), _T("MOON"), _T(""), buff, 256, inipath)) {
		SetDlgItemText(g_hwndDlg, IDC_IS_MOON, buff);
		g_PopupStrings[4] = buff;
	}
	if(GetPrivateProfileString(_T("GUIRes"), _T("MOON_TIP"), _T(""), buff, 256, inipath))
		g_ScanEditStr[4] = buff;

	// battle result (non-static texts)
	if(GetPrivateProfileString(_T("GUIRes"), _T("ATT_WINCHAN"), _T(""), buff, 256, inipath))
		g_ResString[0] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("DEF_WINCHAN"), _T(""), buff, 256, inipath))
		g_ResString[1] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("DRAW_CHAN"), _T(""), buff, 256, inipath))
		g_ResString[2] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("AFTERROUNDS"), _T(""), buff, 256, inipath))
		g_ResString[3] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("TF_PLUNDER"), _T(""), buff, 256, inipath))
		g_ResString[4] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("MET_KRIS_DEUT"), _T(""), buff, 256, inipath))
		g_ResString[5] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("AUSB"), _T(""), buff, 256, inipath))
		g_ResString[6] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("CHANCE_ON_MOON"), _T(""), buff, 256, inipath))
		g_ResString[8] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("NOTENOUGHRUBBLE"), _T(""), buff, 256, inipath))
		g_ResString[9] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("UNKNOWNPLANET"), _T(""), buff, 256, inipath))
		g_ResString[10] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("FIGHT_ON"), _T(""), buff, 256, inipath))
		g_ResString[11] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("FUEL_DEUT"), _T(""), buff, 256, inipath))
		g_ResString[12] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("AFTER_ATTS"), _T(""), buff, 256, inipath))
		g_ResString[13] = buff;
	if(GetPrivateProfileString(_T("GUIRes"), _T("REC_INFO"), _T(""), buff, 256, inipath))
		g_ResString[14] = buff;

	// button bar
	if(GetPrivateProfileString(_T("GUI"), _T("UPDATECHECK"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_UPDATE, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("BATTLEREPORT"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_SHOW_KB, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("FIGHTINCLIPB"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_CLIP_COPY, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("EXIT"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_EXIT, buff);
	if(GetPrivateProfileString(_T("GUI"), _T("SHOWPOPUP"), _T(""), buff, 256, inipath))
		SetDlgItemText(g_hwndDlg, IDC_SHOWPOPUP, buff);

	// balance
	if(GetPrivateProfileString(_T("BAL"), _T("TITLE"), _T(""), buff, 256, inipath))
		g_BilanzStrings[0] = buff;
	if(GetPrivateProfileString(_T("BAL"), _T("T1"), _T(""), buff, 256, inipath))
		g_BilanzStrings[1] = buff;
	if(GetPrivateProfileString(_T("BAL"), _T("T2"), _T(""), buff, 256, inipath))
		g_BilanzStrings[2] = buff;
	if(GetPrivateProfileString(_T("BAL"), _T("T3"), _T(""), buff, 256, inipath))
		g_BilanzStrings[3] = buff;
	if(GetPrivateProfileString(_T("BAL"), _T("T4"), _T(""), buff, 256, inipath))
		g_BilanzStrings[4] = buff;
	if(GetPrivateProfileString(_T("RESS"), _T("MET"), _T(""), buff, 256, inipath))
		g_ResNames[0] = buff;
	if(GetPrivateProfileString(_T("RESS"), _T("KRIS"), _T(""), buff, 256, inipath))
		g_ResNames[1] = buff;
	if(GetPrivateProfileString(_T("RESS"), _T("DEUT"), _T(""), buff, 256, inipath))
		g_ResNames[2] = buff;

	// Updater Strings
	if(GetPrivateProfileString(_T("GUI"), _T("UPD1"), _T(""), buff, 256, inipath))
		g_UpdaterStr[0] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("UPD2"), _T(""), buff, 256, inipath))
		g_UpdaterStr[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("UPD3"), _T(""), buff, 256, inipath))
		g_UpdaterStr[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("UPD4"), _T(""), buff, 256, inipath))
		g_UpdaterStr[3] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("UPD5"), _T(""), buff, 256, inipath))
		g_UpdaterStr[4] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("UPD6"), _T(""), buff, 256, inipath))
		g_UpdaterStr[5] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("UPD7"), _T(""), buff, 256, inipath))
		g_UpdaterStr[6] = buff;

	// new options dialog
	// page 1
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_DEFREBUILD"), _T(""), buff, 256, inipath))
		g_OptPage1[0] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_NEWFUEL"), _T(""), buff, 256, inipath))
		g_OptPage1[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_SPEED_FAC"), _T(""), buff, 256, inipath))
		g_OptPage1[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TITLE"), _T(""), buff, 256, inipath))
		g_OptPage1[3] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TOSYSTRAY"), _T(""), buff, 256, inipath))
		g_OptPage1[4] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_POPUPONTEXT"), _T(""), buff, 256, inipath))
		g_OptPage1[5] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_POPUPONTEXT2"), _T(""), buff, 256, inipath))
		g_OptPage1[6] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_REBUILDSMALLDEF"), _T(""), buff, 256, inipath))
		g_OptPage1[7] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TRANSPARENCY"), _T(""), buff, 256, inipath))
		g_OptPage1[8] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TRANSPARENCY_DLG"), _T(""), buff, 256, inipath))
		g_OptPage1[9] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TRANSPARENCY_BOX"), _T(""), buff, 256, inipath))
		g_OptPage1[10] = buff;
	// page 2
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TITLE2"), _T(""), buff, 256, inipath))
		g_OptPage2[0] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_WINSTART"), _T(""), buff, 256, inipath))
		g_OptPage2[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_STARTMINI"), _T(""), buff, 256, inipath))
		g_OptPage2[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_ONEINST"), _T(""), buff, 256, inipath))
		g_OptPage2[3] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_PLUNDERSHIP"), _T(""), buff, 256, inipath))
		g_OptPage2[4] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_WAVES"), _T(""), buff, 256, inipath))
		g_OptPage2[5] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_REBUILDBASE"), _T(""), buff, 256, inipath))
		g_OptPage2[6] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_AVERAGE"), _T(""), buff, 256, inipath))
		g_OptPage2[7] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_WORSTCASE"), _T(""), buff, 256, inipath))
		g_OptPage2[8] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_BESTCASE"), _T(""), buff, 256, inipath))
		g_OptPage2[9] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_OLDBS"), _T(""), buff, 256, inipath))
		g_OptPage2[10] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_BIGDF"), _T(""), buff, 256, inipath))
		g_OptPage2[11] = buff;
	// page 3
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_TITLE3"), _T(""), buff, 256, inipath))
		g_OptPage3[0] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_OWN_RF"), _T(""), buff, 256, inipath))
		g_OptPage3[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_OWN_SD"), _T(""), buff, 256, inipath))
		g_OptPage3[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_LOADED_RF"), _T(""), buff, 256, inipath))
		g_OptPage3[3] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OPT_LOADED_SD"), _T(""), buff, 256, inipath))
		g_OptPage3[4] = buff;
	

	// tray strings
	if(GetPrivateProfileString(_T("GUI"), _T("TRAY_CLIPBOARD"), _T(""), buff, 256, inipath))
		g_TrayText[0] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("TRAY_POPUPONTEXT"), _T(""), buff, 256, inipath))
		g_TrayText[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("TRAY_ON"), _T(""), buff, 256, inipath))
		g_TrayText[2] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("TRAY_OFF"), _T(""), buff, 256, inipath))
		g_TrayText[3] = buff;

	// misc strings
	if(GetPrivateProfileString(_T("GUI"), _T("RESTART1"), _T(""), buff, 256, inipath))
		g_MiscStr[0] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("RESTART2"), _T(""), buff, 256, inipath))
		g_MiscStr[1] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("CLOSE"), _T(""), buff, 256, inipath))
		g_MiscStr[2] = buff;

	// IP Mode
	if(GetPrivateProfileString(_T("GUI"), _T("PRIMARY_TARGET"), _T(""), buff, 256, inipath))
	{
		g_IPModeStr[0] = buff;
		if(g_IPMMode)
			SetDlgItemText(g_hwndDlg, IDC_ATT2, buff);
	}
	if(GetPrivateProfileString(_T("GUI"), _T("IPM_ABM"), _T(""), buff, 256, inipath))
	{
		g_IPModeStr[1] = buff;
		SetDlgItemText(g_hwndDlg, ID_NAME_IPM, buff);
	}
	if(GetPrivateProfileString(_T("GUI"), _T("OVERTAKE_RES"), _T(""), buff, 256, inipath))
	{
		g_IPModeStr[2] = buff;
		SetDlgItemText(g_hwndDlg, ID_TAKE_OVER_IPM_RES, buff);
	}
	if(GetPrivateProfileString(_T("MENU"), _T("IP_MODE_ON"), _T(""), buff, 256, inipath))
		g_IPModeStr[3] = buff;
	if(GetPrivateProfileString(_T("MENU"), _T("IP_MODE_OFF"), _T(""), buff, 256, inipath))
		g_IPModeStr[4] = buff;

	// TrayMenu
	if(GetPrivateProfileString(_T("Menu"), _T("TRAY_RESTORE"), _T(""), buff, 256, inipath))
		g_TrayText[4] = buff;
	if(GetPrivateProfileString(_T("Menu"), _T("TRAY_EXIT"), _T(""), buff, 256, inipath))
		g_TrayText[5] = buff;
	if(GetPrivateProfileString(_T("Menu"), _T("ABOUT"), _T(""), buff, 256, inipath))
		g_TrayText[6] = buff;
	
	// Popup
	if(GetPrivateProfileString(_T("PopUp"), _T("START"), _T(""), buff, 256, inipath))
		g_PopupStrings[0] = buff;
	if(GetPrivateProfileString(_T("PopUp"), _T("TARGET"), _T(""), buff, 256, inipath))
		g_PopupStrings[1] = buff;
	if(GetPrivateProfileString(_T("PopUp"), _T("CLOSEWITHCLICK"), _T(""), buff, 256, inipath))
		g_PopupStrings[2] = buff;
	if(GetPrivateProfileString(_T("PopUp"), _T("RECSFORDEBRIS"), _T(""), buff, 256, inipath))
		g_PopupStrings[3] = buff;
	
	// espionage history
	if(GetPrivateProfileString(_T("ReadStrings"), _T("ESP_RES"), _T(""), buff, 256, inipath))
		g_EspTitles[0] = buff;
	if(GetPrivateProfileString(_T("ReadStrings"), _T("ESP_FLEETS"), _T(""), buff, 256, inipath))
		g_EspTitles[1] = buff;
	if(GetPrivateProfileString(_T("ReadStrings"), _T("ESP_DEF"), _T(""), buff, 256, inipath))
		g_EspTitles[2] = buff;
	if(GetPrivateProfileString(_T("ReadStrings"), _T("ESP_RESEARCH"), _T(""), buff, 256, inipath))
		g_EspTitles[3] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("TITLE"), _T(""), buff, 256, inipath))
		g_EspHistStrings[0] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("NAMECOORD"), _T(""), buff, 256, inipath))
		g_EspHistStrings[1] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("ADDED"), _T(""), buff, 256, inipath))
		g_EspHistStrings[2] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("LOAD"), _T(""), buff, 256, inipath))
		g_EspHistStrings[3] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("DISABLE"), _T(""), buff, 256, inipath))
		g_EspHistStrings[4] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("DELSEL"), _T(""), buff, 256, inipath))
		g_EspHistStrings[5] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("ATTACKED"), _T(""), buff, 256, inipath))
		g_EspHistStrings[7] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("CHECKATTACKED"), _T(""), buff, 256, inipath))
		g_EspHistStrings[8] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("UNCHECKATTACKED"), _T(""), buff, 256, inipath))
		g_EspHistStrings[9] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("SAVE_REP"), _T(""), buff, 256, inipath))
		g_EspHistStrings[10] = buff;
	if(GetPrivateProfileString(_T("EspHist"), _T("NUM_REP"), _T(""), buff, 256, inipath))
		g_EspHistStrings[11] = buff;
	
	if(g_hwndEspHist)
	{
		bool vis = false;
		if(IsWindowVisible(g_hwndEspHist))
			vis = true;
		EndDialog(g_hwndEspHist, 0);
		CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_ESP_REPORTS), g_hwndDlg, ReportHistProc);
		if(vis)
			ShowWindow(g_hwndEspHist, SW_SHOWNORMAL);
	}

	// language version check
	g_MiscStr[9] = _T("Your language file seems to be outdated.\n")
		_T("Please visit the website to check for an updated language file.");
	if(GetPrivateProfileString(_T("GUI"), _T("OLDLANG"), _T(""), buff, 256, inipath))
		g_MiscStr[9] = buff;
	if(GetPrivateProfileString(_T("GUI"), _T("OLDLANG2"), _T(""), buff, 256, inipath))
	{
		g_MiscStr[9] += _T("\n");
		g_MiscStr[9] += buff;
	}
	GetPrivateProfileString(_T("General"), _T("LANG_VER"), _T(""), buff, 256, inipath);
	CheckLangFileVersion(buff);

	// Update menu strings
	InitMenu();
	InvalidateRect(g_hwndDlg, NULL, true);
	return;
}

void ProcessClipboardChg() {
	bool bUnicode = IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE;
	if(!IsClipboardFormatAvailable(CF_OEMTEXT) && !IsClipboardFormatAvailable(CF_TEXT) 
		&& !bUnicode)
		return;
	if(OpenClipboard(g_hwndDlg)) {
		if(bUnicode) {
			wchar_t* szBuff = (wchar_t*)GetClipboardData(CF_UNICODETEXT);
			SetDlgItemTextW(g_hwndDlg, IDC_GEN_READ, szBuff);
		}
		else {
			char* szBuff = (char*)GetClipboardData(CF_TEXT);
			SetDlgItemTextA(g_hwndDlg, IDC_GEN_READ, szBuff);
		}
		CloseClipboard();
	}
	UpdateKernel();
	GeneralRead();
	UpdateEditFields(true, true);
	SetDlgItemText(g_hwndDlg, IDC_GEN_READ, _T(""));
	return;
}

void EnableDefense(bool bDo)
{
	int state = bDo ? FALSE : TRUE;
	for(int i = T_SHIPEND; i < T_END; i++)
		SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, false), EM_SETREADONLY, state, 0);
}

void EnableFleet(bool bDo) {
	int state = bDo ? FALSE : TRUE;
	int i;
	for(i = 0; i < T_SHIPEND; i++)
	{
		//EnableWindow(GetDlgItem(, GetEditIdByType((ITEM_TYPE)i, false)), bDo);
		SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, false), EM_SETREADONLY, state, 0);
		if(i != T_SAT && i < T_RAK)
			SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, true), EM_SETREADONLY, state, 0);
	}
}

// menu initialisation
void InitMenu()
{
	static HMENU hMainMenu = NULL;
	static bool start = true;
	int i;
	// destroy old menu and assign new
	if(!hMainMenu)
		hMainMenu = GetMenu(g_hwndDlg);
	DestroyMenu(hMainMenu);
	hMainMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	SetMenu(g_hwndDlg, hMainMenu);
	
	HMENU hMenu = GetMenu(g_hwndDlg);
	HMENU hFileMenu = GetSubMenu(hMenu, 0);
	HMENU hOptSaveMenu = GetSubMenu(hFileMenu, 0);
	HMENU hOptLoadMenu = GetSubMenu(hFileMenu, 1);
	HMENU hDeleteOptMenu = GetSubMenu(hFileMenu, 2);
	
	TCHAR iniFile[MAX_PATH];
	genstr nfile = GetSaveFolder() + SPEEDSIM_SAVEFILE;
	_tcsncpy(iniFile, nfile.c_str(), MAX_PATH);
	// get number of saved options/fleets
	int NumSaveData = GetPrivateProfileInt(_T("General"), _T("NumOpts"), 0, iniFile);
	
	DeleteMenu(hOptLoadMenu, ID_DATEI_EINSTELLUNGENLADEN_KEINE, MF_BYCOMMAND);
	DeleteMenu(hDeleteOptMenu, ID_DATEI_EINSTELLUNGENLSCHEN_KEINE, MF_BYCOMMAND);
	
	/*if(NumSaveData == 0 && !start) {
		PostMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(ID_DATEI_EINSTELLUNGENSPEICHERN_NEU, 0), 0);
	}
	else*/ if(NumSaveData >= 1)
	{
		for(i = 1; i <= NumSaveData; i++) {
			TCHAR c[64];
			TCHAR num[5];
			ZeroMemory(c, 64);
			ZeroMemory(num, 5);
			GetOptSaveName(SPEEDSIM_SAVEFILE, i, c);
			_itot(i, num, 10);
			if(!c || !_tcslen(c))
				_tcscpy(c, num);
			AppendMenu(hOptSaveMenu, MF_STRING, IDM_FIRST_OPTSAVE + i, c);
			AppendMenu(hDeleteOptMenu, MF_STRING, IDM_FIRST_OPTDELETE + i, c);
			if(i <= 5) {
				// add [Ctrl]+Num for loading
				_tcscat(c, _T("\tCtrl+"));
				_tcscat(c, num);
			}
			AppendMenu(hOptLoadMenu, MF_STRING, IDM_FIRST_OPTLOAD+i, c);
		}
		CheckMenuItem(hOptSaveMenu, g_CurSaveData, MF_BYPOSITION|MF_CHECKED);
	}
	HMENU hFleetMenu = GetSubMenu(hMenu, 1);
	HMENU hFleetSaveMenu = GetSubMenu(hFleetMenu, 0);
	HMENU hFleetLoadMenu = GetSubMenu(hFleetMenu, 1);
	HMENU hFleetDeleteMenu = GetSubMenu(hFleetMenu, 2);
	HMENU hAttMenu = GetSubMenu(hFleetSaveMenu, 0);
	HMENU hDefMenu = GetSubMenu(hFleetSaveMenu, 1);
	for(i = 1; i <= g_NumFleets; i++) {
		TCHAR c[64];
		GetFleetSaveName(SPEEDSIM_SAVEFILE, i, c);
		if(!c || !_tcslen(c))
			_itot(i, c, 10);
		AppendMenu(hAttMenu, MF_STRING, IDM_FIRST_ATT_FLEETSAVE + i, c);
		AppendMenu(hDefMenu, MF_STRING, IDM_FIRST_DEF_FLEETSAVE + i, c);
	}
	hAttMenu = GetSubMenu(hFleetLoadMenu, 0);
	hDefMenu = GetSubMenu(hFleetLoadMenu, 1);
	if(g_NumFleets > 0) {
		DeleteMenu(hAttMenu, 0, MF_BYPOSITION);
		DeleteMenu(hDefMenu, 0, MF_BYPOSITION);
		DeleteMenu(hFleetDeleteMenu, ID_FLOTTEN_LSCHEN_KEINE, MF_BYCOMMAND);
	}
	for(i = 1; i <= g_NumFleets; i++) {
		TCHAR c[64];
		GetFleetSaveName(SPEEDSIM_SAVEFILE, i, c);
		if(!c || !_tcslen(c))
			_itot(i, c, 10);
		AppendMenu(hAttMenu, MF_STRING, IDM_FIRST_ATT_FLEETLOAD + i, c);
		AppendMenu(hDefMenu, MF_STRING, IDM_FIRST_DEF_FLEETLOAD + i, c);
		AppendMenu(hFleetDeleteMenu, MF_STRING, IDM_FIRST_FLEETDELETE + i, c);
	}
	// load language stuff
	TCHAR inipath[MAX_PATH];
	TCHAR buff[256];
	_tcscpy(inipath, g_CurrDir);
#ifdef UNICODE
	mbstowcs(&inipath[_tcslen(inipath)], g_Options.LangFileName, strlen(g_Options.LangFileName)+1);
#else
	_tcscat(inipath, g_Options.LangFileName);
#endif
	if(GetPrivateProfileString(_T("Menu"), _T("FILE"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hMainMenu, 0, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFileMenu, buff);
	}
	// file menu
	HMENU hOptMenu = GetSubMenu(hFileMenu, 0);
	if(GetPrivateProfileString(_T("Menu"), _T("SAVEOPTS"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFileMenu, 0, MF_BYPOSITION|MF_STRING, (UINT_PTR)hOptMenu, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("NEWSLOT"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hOptMenu, ID_DATEI_EINSTELLUNGENSPEICHERN_NEU, MF_BYCOMMAND|MF_STRING, ID_DATEI_EINSTELLUNGENSPEICHERN_NEU, buff);
	}
	hOptMenu = GetSubMenu(hFileMenu, 1);
	if(GetPrivateProfileString(_T("Menu"), _T("LOADOPTS"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFileMenu, 1, MF_BYPOSITION|MF_STRING, (UINT_PTR)hOptMenu, buff);
	}
	hOptMenu = GetSubMenu(hFileMenu, 2);
	if(GetPrivateProfileString(_T("Menu"), _T("DELOPTS"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFileMenu, 2, MF_BYPOSITION|MF_STRING, (UINT_PTR)hOptMenu, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("EXTOPTS"), _T(""), buff, 256, inipath)) {
		_tcscat(buff, _T("\tF4"));
		ModifyMenu(hFileMenu, ID_DATEI_ERWITERTEEINSTELLUNGEN, MF_BYCOMMAND|MF_STRING, ID_DATEI_ERWITERTEEINSTELLUNGEN, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("ESPHIST"), _T(""), buff, 256, inipath)) {
		_tcscat(buff, _T("\tF2"));
		ModifyMenu(hFileMenu, IDM_SHOW_ESPHIST, MF_BYCOMMAND|MF_STRING, IDM_SHOW_ESPHIST, buff);
	}
	if(IsWindowVisible(g_hwndEspHist) && g_hwndEspHist)
	{
		CheckMenuItem(hFileMenu, IDM_SHOW_ESPHIST, MF_BYCOMMAND|MF_CHECKED);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("EXIT"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFileMenu, IDM_CLOSE, MF_BYCOMMAND|MF_STRING, IDM_CLOSE, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("SAVETECHS"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFileMenu, ID_DATEI_TECHNOLOGIEENSPEICHERN, MF_BYCOMMAND|MF_STRING, ID_DATEI_TECHNOLOGIEENSPEICHERN, buff);
	}
	// fleet menu
	hFleetMenu = GetSubMenu(hMainMenu, 1);
	if(GetPrivateProfileString(_T("Menu"), _T("FLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hMainMenu, 1, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetMenu, buff);
	}
	HMENU hFleetOpt = GetSubMenu(hFleetMenu, 0);
	if(GetPrivateProfileString(_T("Menu"), _T("SAVEFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetMenu, 0, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetOpt, buff);
	}
	HMENU hFleetSlots = GetSubMenu(hFleetOpt, 0);
	if(GetPrivateProfileString(_T("Menu"), _T("ATTACKINGFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, 0, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetSlots, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("NEWSLOT"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, ID_FLOTTEN_SPEICHERN_ANGREIFER_NEU, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_SPEICHERN_ANGREIFER_NEU, buff);
	}
	hFleetSlots = GetSubMenu(hFleetOpt, 1);
	if(GetPrivateProfileString(_T("Menu"), _T("DEFENDINGFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, 1, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetSlots, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("NEWSLOT"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, ID_FLOTTEN_SPEICHERN_VERTEIDIGER_NEU, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_SPEICHERN_VERTEIDIGER_NEU, buff);
	}
	
	hFleetOpt = GetSubMenu(hFleetMenu, 1);
	if(GetPrivateProfileString(_T("Menu"), _T("LOADFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetMenu, 1, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetOpt, buff);
	}
	hFleetSlots = GetSubMenu(hFleetOpt, 0);
	if(GetPrivateProfileString(_T("Menu"), _T("TOATTACKER"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, 0, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetSlots, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("NOFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, ID_FLOTTEN_LADEN_ANGREIFER_1, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_LADEN_ANGREIFER_1, buff);
	}
	hFleetSlots = GetSubMenu(hFleetOpt, 1);
	if(GetPrivateProfileString(_T("Menu"), _T("TODEFENDER"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetOpt, 1, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetSlots, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("NOFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetSlots, ID_FLOTTEN_LADEN_VERTEIDIGER_1, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_LADEN_VERTEIDIGER_1, buff);
	}
	hFleetSlots = GetSubMenu(hFleetMenu, 2);
	if(GetPrivateProfileString(_T("Menu"), _T("DELFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetMenu, 2, MF_BYPOSITION|MF_STRING, (UINT_PTR)hFleetSlots, buff);
	}
	if(GetPrivateProfileString(_T("Menu"), _T("NOFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetSlots, ID_FLOTTEN_LSCHEN_KEINE, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_LSCHEN_KEINE, buff);
	}

	if(GetPrivateProfileString(_T("Menu"), _T("RESETFLEETS"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetMenu, ID_FLOTTEN_ALLEFLOTTENSLOTSLEEREN, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_ALLEFLOTTENSLOTSLEEREN, buff);
	}
	// Language
	HMENU hLang = GetSubMenu(hMainMenu, 2);
	if(GetPrivateProfileString(_T("Menu"), _T("LANGUAGE"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hMainMenu, 2, MF_BYPOSITION|MF_STRING, (UINT_PTR)hLang, buff);
	}
	
	// IP Mode
	if(g_IPMMode)
		ModifyMenu(hMainMenu, IDM_IPMMODE, MF_BYCOMMAND|MF_STRING, IDM_IPMMODE, g_IPModeStr[3].c_str());
	else
		ModifyMenu(hMainMenu, IDM_IPMMODE, MF_BYCOMMAND|MF_STRING, IDM_IPMMODE, g_IPModeStr[4].c_str());
	
	// About
	if(GetPrivateProfileString(_T("Menu"), _T("ABOUT"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hMainMenu, IDM_ABOUT, MF_BYCOMMAND|MF_STRING, IDM_ABOUT, buff);
	}
	hFleetSlots = GetSubMenu(hFleetOpt, 3);
	if(GetPrivateProfileString(_T("Menu"), _T("NOFLEET"), _T(""), buff, 256, inipath)) {
		ModifyMenu(hFleetSlots, ID_FLOTTEN_LADEN_VERTEIDIGER_1, MF_BYCOMMAND|MF_STRING, ID_FLOTTEN_LADEN_VERTEIDIGER_1, buff);
	}
	
	DeleteMenu(hLang, 0, MF_BYPOSITION);
	AppendMenu(hLang, MF_STRING, IDM_FIRST_LANG, _T("Deutsch"));
	// language menu
	TCHAR CurLang[MAX_PATH];
	ZeroMemory(CurLang, MAX_PATH);
#ifdef UNICODE
	mbstowcs(CurLang, g_Options.LangFileName, strlen(g_Options.LangFileName)+1);
#else
	strcpy(CurLang, g_Options.LangFileName);
#endif

	map<genstring, genstring> files = UpdateLangFiles();
	map<genstring, genstring>::const_iterator it_lng = files.begin();
	i = 0;
	for(; it_lng != files.end(); it_lng++)
	{
		AppendMenu(hLang, MF_STRING, IDM_FIRST_LANG + i + 1, it_lng->first.c_str());
		if(!_tcscmp(CurLang, it_lng->second.c_str()))
			CheckMenuItem(hLang, IDM_FIRST_LANG + i + 1, MF_BYCOMMAND|MF_CHECKED);
		i++;
	}
	
	if(!_tcslen(CurLang))
	{
		CheckMenuItem(hLang, IDM_FIRST_LANG, MF_BYCOMMAND|MF_CHECKED);
	}
	DrawMenuBar(g_hwndDlg);
	start = false;
}

void AddFleetSlot(int num, const TCHAR* name) {
	HMENU hMenu = GetMenu(g_hwndDlg);
	HMENU hFleetMenu = GetSubMenu(hMenu, 1);
	HMENU hFleetSaveMenu = GetSubMenu(hFleetMenu, 0);
	HMENU hFleetLoadMenu = GetSubMenu(hFleetMenu, 1);
	HMENU hDelMenu = GetSubMenu(hFleetMenu, 2);
	HMENU hAttMenu = GetSubMenu(hFleetSaveMenu, 0);
	HMENU hDefMenu = GetSubMenu(hFleetSaveMenu, 1);
	TCHAR c[64];
	if(!name || !_tcslen(name))
		_itot(num, c, 10);
	else
		_tcscpy(c, name);
	AppendMenu(hAttMenu, MF_STRING, IDM_FIRST_ATT_FLEETSAVE+num, c);
	AppendMenu(hDefMenu, MF_STRING, IDM_FIRST_DEF_FLEETSAVE+num, c);
	hAttMenu = GetSubMenu(hFleetLoadMenu, 0);
	hDefMenu = GetSubMenu(hFleetLoadMenu, 1);
	// remove '(none)' Item
	RemoveMenu(hAttMenu, ID_FLOTTEN_LADEN_ANGREIFER_1, MF_BYCOMMAND);
	RemoveMenu(hDefMenu, ID_FLOTTEN_LADEN_VERTEIDIGER_1, MF_BYCOMMAND);
	RemoveMenu(hDelMenu, ID_FLOTTEN_LSCHEN_KEINE, MF_BYCOMMAND);
	AppendMenu(hAttMenu, MF_STRING, IDM_FIRST_ATT_FLEETLOAD+num, c);
	AppendMenu(hDefMenu, MF_STRING, IDM_FIRST_DEF_FLEETLOAD+num, c);
	AppendMenu(hDelMenu, MF_STRING, IDM_FIRST_FLEETDELETE+num, c);
	SetFleetSaveName(SPEEDSIM_SAVEFILE, num, name);
	DrawMenuBar(g_hwndDlg);
}

bool IsPureNumber(TCHAR* str)
{
	for(size_t i = 0; i < _tcslen(str); i++)
	{
		if(IsCharAlpha(str[i]) && str[i] != ' ' && str[i] != '-')
			return false;
	}
	return true;
}

BOOL CALLBACK LangChildWnd(HWND wnd, LPARAM lParam)
{
	map<HWND, genstring> *str = (map<HWND, genstring>*)lParam;
	TCHAR c[256];
	GetWindowText(wnd, c, 256);
	if(!_tcslen(c) || IsPureNumber(c))
		return TRUE;
	(*str)[wnd] = c;
	return TRUE;
}

map<HWND, genstring> GetGerLang(HWND wnd)
{
	map<HWND, genstring> strings;
	EnumChildWindows(wnd, LangChildWnd, (LPARAM)&strings);
	return strings;
}

void SetGerLang(HWND wnd, map<HWND, genstring> strings)
{
	map<HWND, genstring>::iterator it = strings.begin();
	for(; it != strings.end(); it++)
	{
		if(GetParent(it->first) == wnd)
			SetWindowText(it->first, it->second.c_str());
	}
}

void CheckLangFileVersion(TCHAR* ver_str)
{
	// did we already checked with that speedsim version?
	if(!_tcscmp(g_Options.LastLangCheckVer, SPEEDSIM_VERSION))
		return;
	int r = CompareVersions(ver_str, SPEEDSIM_NEEDED_LANG_VERSION);
	if(r < 0)
	{
		MessageBox(g_hwndDlg, g_MiscStr[9].c_str(), _T("Error"), MB_OK|MB_ICONWARNING);
	}
	_tcscpy(g_Options.LastLangCheckVer, SPEEDSIM_VERSION);
}
