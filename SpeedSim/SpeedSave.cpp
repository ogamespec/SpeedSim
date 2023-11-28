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

void CopyResultToClipboard()
{
	TCHAR content[256], text[128];
	BattleResult r = sim.GetBattleResult();
	genstrstream data;
	// Name/coords of planet
	SendDlgItemMessage(g_hwndDlg, IDC_SCAN_END, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_SCAN, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// result
	SendDlgItemMessage(g_hwndDlg, IDC_END, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_END, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// debris
	SendDlgItemMessage(g_hwndDlg, IDC_TF, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_TF, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// lossen on both sides
	// attacker
	SendDlgItemMessage(g_hwndDlg, IDC_VERL_A, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_LOSS_ATT, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// defender
	SendDlgItemMessage(g_hwndDlg, IDC_VERL_V, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_LOSSDEF, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// chance of moon arising
	SendDlgItemMessage(g_hwndDlg, IDC_MOND, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_MOON, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// possible booty
	SendDlgItemMessage(g_hwndDlg, IDC_BEUT_TH, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_THBEUT, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// real booty
	SendDlgItemMessage(g_hwndDlg, IDC_BEUT_PRAK, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_PRAKBEUT, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// deut cost
	SendDlgItemMessage(g_hwndDlg, IDC_FLYCOST, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_SPRIT, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// fly time
	SendDlgItemMessage(g_hwndDlg, IDC_FLUGZEIT, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_FLYTIME, text, 128);
	data << text << _T(": ") << content << _T("\r\n");
	// total plunder
	SendDlgItemMessage(g_hwndDlg, IDC_BEUT_GES, WM_GETTEXT, 256,(LPARAM)content);
	GetDlgItemText(g_hwndDlg, IDC_S_WELLBEUT, text, 128);
	data << text << _T(": ") << content;
	if(OpenClipboard(g_hwndDlg))
	{
		genstr str = data.str();
		HGLOBAL clipbuffer;
		TCHAR *buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, (str.length() + 1) * sizeof(TCHAR));
		buffer = (TCHAR*)GlobalLock(clipbuffer);
		_tcscpy(buffer, str.c_str());
		GlobalUnlock(clipbuffer);
#ifdef UNICODE
		SetClipboardData(CF_UNICODETEXT, clipbuffer);
#else
		SetClipboardData(CF_TEXT, clipbuffer);
#endif
		CloseClipboard();
	}
	return;
}

int SaveFleet(int num, bool SaveAtt) {
	UpdateKernel();
	int id = ReadFromEdit(IDC_FLEET_ID)-1;
	SItem fleet[T_END];
	vector<SItem> vFleet;
	ShipTechs techs;
	// get fleet
	if(SaveAtt) {
		sim.GetSetFleet(fleet, NULL, id);
		sim.GetTechs(&techs, NULL, id);
	}
	else {
		sim.GetSetFleet(NULL, fleet, id);
		sim.GetTechs(NULL, &techs, id);
	}
	// convert into vector
	for(int i = 0; i < T_END; i++) {
		if(fleet[i].Num != 0 && fleet[i].Type != T_NONE)
			vFleet.push_back(fleet[i]);
	}    
	
	return NewSetFleetData(SPEEDSIM_SAVEFILE, num, vFleet, techs);
}

int SaveTechs(int num)
{
	SaveData save;
	ZeroMemory(&save, sizeof(save));
	NewGetSaveData(SPEEDSIM_SAVEFILE, num, save);
	save.Techs.STechs.Weapon = GetDlgItemInt(g_hwndDlg, IDC_WAFF_A, NULL, false);
	save.Techs.STechs.Shield = GetDlgItemInt(g_hwndDlg, IDC_SCHILD_A, NULL, false);
	save.Techs.STechs.Armour = GetDlgItemInt(g_hwndDlg, IDC_PANZ_A, NULL, false);
	save.Techs.Combust = GetDlgItemInt(g_hwndDlg, IDC_VERBRENNUNG, NULL, false);
	save.Techs.Impulse = GetDlgItemInt(g_hwndDlg, IDC_IMPULS, NULL, false);
	save.Techs.Hyper = GetDlgItemInt(g_hwndDlg, IDC_HYPERRAUM, NULL, false);
	
	return NewSetSaveData(SPEEDSIM_SAVEFILE, num, save);
}

int SaveOpts(int num)
{
	SaveData old_opts;
	// get techs
	NewGetSaveData(SPEEDSIM_SAVEFILE, num, old_opts);
	g_Options.Techs = old_opts.Techs;

	g_CurSaveData = num;
	
	if(SendDlgItemMessage(g_hwndDlg, IDC_RAPID, BM_GETCHECK, 0, 0) == BST_CHECKED)
		g_Options.RFType = RF_075;
	else
		g_Options.RFType = RF_NONE;
	
	if(strlen(g_Options.RFFileName))
		g_Options.RFType = RF_USER;

	if(SendDlgItemMessage(g_hwndDlg, IDC_DEFTF, BM_GETCHECK, 0, 0) == BST_CHECKED)
		g_Options.DefTF = true;
	else
		g_Options.DefTF = false;
	
	if(SendDlgItemMessage(g_hwndDlg, IDC_BW_CASE, BM_GETCHECK, 0, 0) == BST_CHECKED)
		g_Options.BWCase = true;
	else
		g_Options.BWCase = false;
	
	if(SendDlgItemMessage(g_hwndDlg, IDC_CHECKTECHS, BM_GETCHECK, 0, 0) == BST_CHECKED)
		g_Options.bCheckTechs = true;
	else
		g_Options.bCheckTechs = false;
	if(SendDlgItemMessage(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH, BM_GETCHECK, 0, 0) == BST_CHECKED)
		g_Options.bDelDefOnSwitch = true;
	else
		g_Options.bDelDefOnSwitch = false;
	

	if(SendDlgItemMessage(g_hwndDlg, IDC_HOOK_CB, BM_GETCHECK, 0, 0) == BST_CHECKED)
		g_Options.HookClipboard = true;
	else
		g_Options.HookClipboard = false;
	
	g_Options.NumSims = ReadFromEdit(IDC_NUM_SIM);
	int i = 0;
	for(i = 0; i < MAX_POS; i++)
		_tcscpy(g_Options.PlanPos[i], _T(""));
	i = SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_GETCOUNT, 0, 0);
	for(int j = 0; j < (i > MAX_POS-1 ? MAX_POS : i); j++)
		SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_GETLBTEXT, j, (LPARAM)g_Options.PlanPos[j]);
	g_Options.SelPos = (int)SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_GETCURSEL, 0, 0);
	if(g_Options.SelPos == 255)
		g_Options.SelPos = 0;
	if(IsWindowVisible(g_hwndEspHist))
		g_Options.bShowEspHist = true;
	else
		g_Options.bShowEspHist = false;

	RECT r;
	GetWindowRect(g_hwndEspHist, &r);
	g_Options.EspHistPos[0] = r.left;
	g_Options.EspHistPos[1] = r.top;
	g_Options.EspHistSize[0] = r.right - r.left;
	g_Options.EspHistSize[1] = r.bottom - r.top;

	// get set num of reports
	if(IsDlgButtonChecked(g_hwndEspHist, IDC_SAVE_REP) == BST_CHECKED)
	{
		g_Options.cHistInFile = GetDlgItemInt(g_hwndEspHist, IDC_NUM_REP, NULL, FALSE);
	}
	else
		g_Options.cHistInFile = 0;

	int ret = NewSetSaveData(SPEEDSIM_SAVEFILE, num, g_Options);
	SaveEspHistory(ret);
	return ret;
}

bool LoadTechs(int num, bool doAtt, bool bOnlyEngines) {
	SaveData save;
	ZeroMemory(&save, sizeof(save));
	if(!NewGetSaveData(SPEEDSIM_SAVEFILE, num, save))
		return false;
	int id = ReadFromEdit(IDC_FLEET_ID)-1;
	if (id < 0 || id > MAX_PLAYERS_PER_TEAM-1)
		id = 0;
	
	if(!bOnlyEngines) {
		if(doAtt) {
			if(save.Techs.STechs.Weapon)
				SetDlgItemInt(g_hwndDlg, IDC_WAFF_A, (int)save.Techs.STechs.Weapon, false);
			else
				SetDlgItemText(g_hwndDlg, IDC_WAFF_A, _T(""));
			if(save.Techs.STechs.Shield)
				SetDlgItemInt(g_hwndDlg, IDC_SCHILD_A, (int)save.Techs.STechs.Shield, false);
			else
				SetDlgItemText(g_hwndDlg, IDC_SCHILD_A, _T(""));
			if(save.Techs.STechs.Armour)
				SetDlgItemInt(g_hwndDlg, IDC_PANZ_A, (int)save.Techs.STechs.Armour, false);
			else
				SetDlgItemText(g_hwndDlg, IDC_PANZ_A, _T(""));
			sim.SetTechs(&save.Techs.STechs, NULL, id);
		}
		else {
			if(save.Techs.STechs.Weapon)
				SetDlgItemInt(g_hwndDlg, IDC_WAFF_V, (int)save.Techs.STechs.Weapon, false);
			else
				SetDlgItemText(g_hwndDlg, IDC_WAFF_V, _T(""));
			if(save.Techs.STechs.Shield)
				SetDlgItemInt(g_hwndDlg, IDC_SCHILD_V, (int)save.Techs.STechs.Shield, false);
			else
				SetDlgItemText(g_hwndDlg, IDC_SCHILD_V, _T(""));
			if(save.Techs.STechs.Armour)
				SetDlgItemInt(g_hwndDlg, IDC_PANZ_V, (int)save.Techs.STechs.Armour, false);
			else
				SetDlgItemText(g_hwndDlg, IDC_PANZ_V, _T(""));
			sim.SetTechs(NULL, &save.Techs.STechs, id);
		}
	}
	if(save.Techs.Combust)
		SetDlgItemInt(g_hwndDlg, IDC_VERBRENNUNG, (int)save.Techs.Combust, false);
	else
		SetDlgItemText(g_hwndDlg, IDC_VERBRENNUNG, _T(""));
	if(save.Techs.Impulse)
		SetDlgItemInt(g_hwndDlg, IDC_IMPULS, (int)save.Techs.Impulse, false);
	else
		SetDlgItemText(g_hwndDlg, IDC_IMPULS, _T(""));
	if(save.Techs.Hyper)
		SetDlgItemInt(g_hwndDlg, IDC_HYPERRAUM, (int)save.Techs.Hyper, false);
	else
		SetDlgItemText(g_hwndDlg, IDC_HYPERRAUM, _T(""));
	return true;
}

bool LoadFleet(int num, bool doAtt) {
	UpdateKernel();
	SaveData save;
	ZeroMemory(&save, sizeof(save));
	SItem sFleet[T_END];
	vector<SItem> vFleet;
	ShipTechs techs;
	int id = ReadFromEdit(IDC_FLEET_ID)-1;

	if(!NewGetFleetData(SPEEDSIM_SAVEFILE, num, vFleet, techs))
		return false;
	if(doAtt)
		ResetAtt();
	else
		ResetDef();
	
	for(size_t i = 0; i < vFleet.size(); i++) {
		vFleet[i].OwnerID = id;
	}
	if(doAtt) {
		sim.SetFleet(&vFleet, NULL);
		sim.SetTechs(&techs, NULL, id);
	}
	else {
		sim.SetFleet(NULL, &vFleet);
		sim.SetTechs(NULL, &techs, id);
	}
	UpdateEditFields(true, true);
	return true;
}

bool LoadOpts(int num) 
{
	// remove from chain
	if(g_Options.HookClipboard)
		ChangeClipboardChain(g_hwndDlg, g_hwndNextCBWnd);
	g_hwndNextCBWnd = NULL;
	g_Options.HookClipboard = false;

	if(!NewGetSaveData(SPEEDSIM_SAVEFILE, num, g_Options)) {
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CHOOSELANG), g_hwndDlg, LangProc);
		return false;
	}
	g_CurSaveData = num;
	
	FILE* f = NULL;
	if(strlen(g_Options.LangFileName) && (f = fopen(g_Options.LangFileName, "r+"))) {
		SendDlgItemMessage(g_hwndDlg, IDC_USE_LANG, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);        
		LoadLang(g_Options.LangFileName);
	}
	else {
		SendDlgItemMessage(g_hwndDlg, IDC_USE_LANG, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		strcpy(g_Options.LangFileName, "");
		LoadLang(NULL);
	}
	if(f != NULL)
		fclose(f);

	if(g_Options.PlunderShip == T_SPIO || g_Options.PlunderShip == T_SAT || g_Options.PlunderShip == T_CRA || g_Options.PlunderShip >= T_SHIPEND)
		g_Options.PlunderShip = T_GT;

	if(g_Options.DefTF)
		SendDlgItemMessage(g_hwndDlg, IDC_DEFTF, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_DEFTF, BM_SETCHECK, BST_UNCHECKED, 0);
	
	if(g_Options.BWCase)
		SendDlgItemMessage(g_hwndDlg, IDC_BW_CASE, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_BW_CASE, BM_SETCHECK, BST_UNCHECKED, 0);

	if(g_Options.bCheckTechs)
		SendDlgItemMessage(g_hwndDlg, IDC_CHECKTECHS, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_CHECKTECHS, BM_SETCHECK, BST_UNCHECKED, 0);
	
	if(g_Options.bDelDefOnSwitch)
		SendDlgItemMessage(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH, BM_SETCHECK, BST_UNCHECKED, 0);
	
	if(g_Options.RFType == RF_NONE)
		SendDlgItemMessage(g_hwndDlg, IDC_RAPID, BM_SETCHECK, BST_UNCHECKED, 0);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_RAPID, BM_SETCHECK, BST_CHECKED, 0);

	if(g_Options.HookClipboard)
	{
		SendDlgItemMessage(g_hwndDlg, IDC_HOOK_CB, BM_SETCHECK, BST_CHECKED, 0);
		g_hwndNextCBWnd = SetClipboardViewer(g_hwndDlg);
	}
	else {
		SendDlgItemMessage(g_hwndDlg, IDC_HOOK_CB, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_RESETCONTENT, 0, 0);
	for(int i = 0; i < MAX_POS; i++)
	{
		if(_tcslen(g_Options.PlanPos[i]))
			SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_ADDSTRING, 0, (LPARAM)g_Options.PlanPos[i]);
	}
	SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_SETCURSEL, g_Options.SelPos, 0);
	
	TCHAR c[6];
	_stprintf(c, _T("%d"), g_Options.NumSims);
	SendDlgItemMessage(g_hwndDlg, IDC_NUM_SIM, WM_SETTEXT, 0, (LPARAM)c);

	char skinfile[64];
#ifdef UNICODE
	wcstombs(skinfile, g_Options.SkinFileName, 64);
#else
	strncpy(skinfile, g_Options.SkinFileName, 64);
#endif
	f = NULL;
	if(_tcslen(g_Options.SkinFileName) && (f = fopen(skinfile, "r+"))) {
		SendDlgItemMessage(g_hwndDlg, IDC_USE_SKIN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	}
	else {
		SendDlgItemMessage(g_hwndDlg, IDC_USE_SKIN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		_tcscpy(g_Options.SkinFileName, _T(""));
	}
	if(f != NULL)
		fclose(f);
	if(g_Options.PlunderShip == T_SPIO)
		g_Options.PlunderShip = T_GT;
	TransparentWindow(g_hwndDlg, g_Options.iDlgTransparency);
	g_vRaidHistory.clear();
	LoadEspHistory(num);
	return true;
}

int NewSetSaveData(TCHAR* file, int num, const SaveData &data) {
	TCHAR section[256];
	TCHAR key[256];
	TCHAR CFGFile[MAX_PATH];
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumOpts"), 0, CFGFile);
	if(num > avail) {
		// add new option
		num = ++avail;
		_itot(num, key, 10);
		WritePrivateProfileString(_T("General"), _T("NumOpts"), key, CFGFile);
	}
	_itot(num, key, 10);
	WritePrivateProfileString(_T("General"), _T("LastOpt"), key, CFGFile);
	WritePrivateProfileString(_T("General"), _T("LastSpeedSimVer"), SPEEDSIM_VERSION, CFGFile);

	_stprintf(section, _T("Option%.3d"), num);

	data.bCheckTechs == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("DelTechs"), key, CFGFile);
	data.bDelDefOnSwitch == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("DelDefOnSwich"), key, CFGFile);
	data.bDoUpdate == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("CheckForUpdate"), key, CFGFile);
	data.bMinimizeOnStart == TRUE ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("MinimizeOnStart"), key, CFGFile);
	data.bPopUpOnText == TRUE ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("PopupOnOGameText"), key, CFGFile);
	data.bSingleInst == TRUE ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("SingleInstance"), key, CFGFile);
	data.bTrayIcon == TRUE ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("TrayIcon"), key, CFGFile);
	data.BWCase == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("AutoShowBWCase"), key, CFGFile);
	data.DefTF == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("DefenseToDebris"), key, CFGFile);
	/*data.bBiggerDFs == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("BiggerDFs"), key, CFGFile);*/
	data.bRebuildSmallDef == TRUE ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("RebuildSmallDefComplete"), key, CFGFile);
	_itot(data.iDefRebuild, key, 10);
	WritePrivateProfileString(section, _T("DefRebuildFac"), key, CFGFile);
	data.HookClipboard == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("SuperviseCB"), key, CFGFile);
	_itot((int)data.Techs.Hyper, key, 10);
	WritePrivateProfileString(section, _T("HyperspaceEngine"), key, CFGFile);
	_itot((int)data.Techs.Impulse, key, 10);
	WritePrivateProfileString(section, _T("ImpulseEngine"), key, CFGFile);
	_itot((int)data.Techs.Combust, key, 10);
	WritePrivateProfileString(section, _T("CombustEngine"), key, CFGFile);
	_itot((int)data.iSpeedFactor, key, 10);
	WritePrivateProfileString(section, _T("SpeedFactor"), key, CFGFile);
	_itot((int)data.PlunderShip, key, 10);
	WritePrivateProfileString(section, _T("PlunderShip"), key, CFGFile);
	
#ifdef UNICODE
	mbstowcs(key, data.LangFileName, strlen(data.LangFileName)+1);
#else
	_tcscpy(key, data.LangFileName);
#endif
	WritePrivateProfileString(section, _T("LangFile"), key, CFGFile);

#ifdef UNICODE
	mbstowcs(key, data.ShipDataFile, strlen(data.ShipDataFile)+1);
#else
	_tcscpy(key, data.ShipDataFile);
#endif
	WritePrivateProfileString(section, _T("ShipDataFile"), key, CFGFile);
	WritePrivateProfileString(section, _T("LastUpd"), data.LastUpdCheck, CFGFile);
	_itot((int)data.NumSims, key, 10);
	WritePrivateProfileString(section, _T("NumSim"), key, CFGFile);
	_itot((int)data.RFType, key, 10);
	WritePrivateProfileString(section, _T("RFType"), key, CFGFile);
	if(data.RFType == RF_USER) {
#ifdef UNICODE
		mbstowcs(key, data.RFFileName, strlen(data.RFFileName)+1);
#else
		_tcscpy(key, data.RFFileName);
#endif
		WritePrivateProfileString(section, _T("RFFile"), key, CFGFile);
	}
	else
		WritePrivateProfileString(section, _T("RFFile"), _T(""), CFGFile);

	WritePrivateProfileString(section, _T("SkinFile"), data.SkinFileName, CFGFile);
	_itot((int)data.Techs.STechs.Weapon, key, 10);
	WritePrivateProfileString(section, _T("WeapTech"), key, CFGFile);
	_itot((int)data.Techs.STechs.Shield, key, 10);
	WritePrivateProfileString(section, _T("ShieldTech"), key, CFGFile);
	_itot((int)data.Techs.STechs.Armour, key, 10);
	WritePrivateProfileString(section, _T("HullTech"), key, CFGFile);
	_itot((int)data.SelPos, key, 10);
	WritePrivateProfileString(section, _T("SelPos"), key, CFGFile);
	for(int i = 0; i < MAX_POS; i++) {
		TCHAR name[15];
		_stprintf(name, _T("Planet%.2d"), i+1);
		WritePrivateProfileString(section, name, &data.PlanPos[i][0], CFGFile);
	}
	_itot(data.iDlgTransparency, key, 10);
	WritePrivateProfileString(section, _T("DlgTrans"), key, CFGFile);
	_itot(data.iPopUpTransparency, key, 10);
	WritePrivateProfileString(section, _T("PopUpTrans"), key, CFGFile);
	_itot((int)data.StructureToDF, key, 10);
	WritePrivateProfileString(section, _T("StructureToDF"), key, CFGFile);
	
	data.bShowEspHist == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("ShowEspHist"), key, CFGFile);
	data.bUseOldBS == true ? _tcscpy(key, _T("1")) : _tcscpy(key, _T("0"));
	WritePrivateProfileString(section, _T("OldBSData"), key, CFGFile);

	_itot((int)data.EspHistPos[0], key, 10);
	WritePrivateProfileString(section, _T("EspHistPosX"), key, CFGFile);
	_itot((int)data.EspHistPos[1], key, 10);
	WritePrivateProfileString(section, _T("EspHistPosY"), key, CFGFile);
	_itot((int)data.EspHistSize[0], key, 10);
	WritePrivateProfileString(section, _T("EspHistSizeX"), key, CFGFile);
	_itot((int)data.EspHistSize[1], key, 10);
	WritePrivateProfileString(section, _T("EspHistSizeY"), key, CFGFile);
	_itot(data.cHistInFile, key, 10);
	WritePrivateProfileString(section, _T("SaveReports"), key, CFGFile);
	return num;
}

bool NewGetSaveData(TCHAR* file, int num, SaveData &data)
{
	TCHAR section[256];
	TCHAR key[256];
	TCHAR CFGFile[MAX_PATH];
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);

	char c_file[MAX_PATH];
#ifdef UNICODE
	wcstombs(c_file, CFGFile, MAX_PATH);
#else
	strncpy(c_file, CFGFile, MAX_PATH);
#endif
	FILE* f = fopen(c_file, "r+");
	if(!f)
		return false;
	fclose(f);

	data = SaveData();

	GetPrivateProfileString(_T("General"), _T("LastSpeedSimVer"), _T(""), data.LastLangCheckVer, 64, CFGFile);
	int avail = GetPrivateProfileInt(_T("General"), _T("NumOpts"), -1, CFGFile);
	if(num > avail || avail == -1)
		return false;
	_stprintf(section, _T("Option%.3d"), num);

	data.bCheckTechs = GetPrivateProfileInt(section, _T("DelTechs"), 1, CFGFile) == TRUE;
	data.bDelDefOnSwitch = GetPrivateProfileInt(section, _T("DelDefOnSwich"), 0, CFGFile) == TRUE;
	data.bDoUpdate = GetPrivateProfileInt(section, _T("CheckForUpdate"), 1, CFGFile) == TRUE;
	data.bMinimizeOnStart = GetPrivateProfileInt(section, _T("MinimizeOnStart"), 0, CFGFile) == TRUE;
	data.bPopUpOnText = GetPrivateProfileInt(section, _T("PopupOnOGameText"), 0, CFGFile);
	data.bSingleInst = GetPrivateProfileInt(section, _T("SingleInstance"), 0, CFGFile);
	data.bTrayIcon = GetPrivateProfileInt(section, _T("TrayIcon"), 0, CFGFile);
	data.bRebuildSmallDef = GetPrivateProfileInt(section, _T("RebuildSmallDefComplete"), 0, CFGFile);
	data.BWCase = GetPrivateProfileInt(section, _T("AutoShowBWCase"), 0, CFGFile) == TRUE;
	data.DefTF = GetPrivateProfileInt(section, _T("DefenseToDebris"), 0, CFGFile) == TRUE;
	bool old_big_df = GetPrivateProfileInt(section, _T("BiggerDFs"), 0, CFGFile) == TRUE;
	data.StructureToDF = GetPrivateProfileInt(section, _T("StructureToDF"), -1, CFGFile);
	if(data.StructureToDF == -1)
	{
		if(old_big_df)
			data.StructureToDF = 70;
		else
			data.StructureToDF = 30;
	}

	data.iDefRebuild = (float)GetPrivateProfileInt(section, _T("DefRebuildFac"), 75, CFGFile);
	data.HookClipboard = GetPrivateProfileInt(section, _T("SuperviseCB"), 0, CFGFile) == TRUE;
	data.Techs.Hyper = GetPrivateProfileInt(section, _T("HyperspaceEngine"), 0, CFGFile);
	data.Techs.Impulse = GetPrivateProfileInt(section, _T("ImpulseEngine"), 0, CFGFile);
	data.Techs.Combust = GetPrivateProfileInt(section, _T("CombustEngine"), 0, CFGFile);
	data.iSpeedFactor = GetPrivateProfileInt(section, _T("SpeedFactor"), 1, CFGFile);
	data.NumSims = GetPrivateProfileInt(section, _T("NumSim"), 100, CFGFile);
	data.RFType = GetPrivateProfileInt(section, _T("RFType"), (int)RF_075, CFGFile);
	data.Techs.STechs.Weapon = GetPrivateProfileInt(section, _T("WeapTech"), 0, CFGFile);
	data.Techs.STechs.Shield = GetPrivateProfileInt(section, _T("ShieldTech"), 0, CFGFile);
	data.Techs.STechs.Armour = GetPrivateProfileInt(section, _T("HullTech"), 0, CFGFile);
	data.SelPos = GetPrivateProfileInt(section, _T("SelPos"), 0, CFGFile);
	data.PlunderShip = (ITEM_TYPE)GetPrivateProfileInt(section, _T("PlunderShip"), 1, CFGFile);
	data.bShowEspHist = GetPrivateProfileInt(section, _T("ShowEspHist"), 1, CFGFile) != 0;
	data.bUseOldBS = GetPrivateProfileInt(section, _T("OldBSData"), 0, CFGFile) != 0;
	GetPrivateProfileString(section, _T("LangFile"), _T(""), key, 64, CFGFile);
#ifdef UNICODE
	wcstombs(data.LangFileName, key, wcslen(key)+1);
#else
	strcpy(data.LangFileName, key);
#endif

	GetPrivateProfileString(section, _T("ShipDataFile"), _T(""), key, 64, CFGFile);
#ifdef UNICODE
	wcstombs(data.ShipDataFile, key, wcslen(key)+1);
#else
	strcpy(data.ShipDataFile, key);
#endif

	GetPrivateProfileString(section, _T("SkinFile"), _T(""), data.SkinFileName, 64, CFGFile);
	GetPrivateProfileString(section, _T("LastUpd"), _T(""), data.LastUpdCheck, 16, CFGFile);
	GetPrivateProfileString(section, _T("RFFile"), _T(""), key, 64, CFGFile);
#ifdef UNICODE
	wcstombs(data.RFFileName, key, wcslen(key)+1);
#else
	strcpy(data.RFFileName, key);
#endif
	for(int i = 0; i < MAX_POS; i++) {
		TCHAR name[15];
		_stprintf(name, _T("Planet%.2d"), i+1);
		GetPrivateProfileString(section, name, _T(""), &data.PlanPos[i][0], 9, CFGFile);
	}
	data.iDlgTransparency = GetPrivateProfileInt(section, _T("DlgTrans"), 0, CFGFile);
	data.iPopUpTransparency = GetPrivateProfileInt(section, _T("PopUpTrans"), 0, CFGFile);

	RECT r;
	GetWindowRect(g_hwndDlg, &r);

	data.EspHistPos[0] = GetPrivateProfileInt(section, _T("EspHistPosX"), 0, CFGFile);
	data.EspHistPos[1] = GetPrivateProfileInt(section, _T("EspHistPosY"), r.top, CFGFile);
	data.EspHistSize[0] = GetPrivateProfileInt(section, _T("EspHistSizeX"), 0, CFGFile);
	data.EspHistSize[1] = GetPrivateProfileInt(section, _T("EspHistSizeY"), 0, CFGFile);

	data.cHistInFile = GetPrivateProfileInt(section, _T("SaveReports"), 10, CFGFile);
	return true;
}

bool SetOptSaveName(TCHAR *file, int num, const TCHAR *Name) {
	TCHAR CFGFile[MAX_PATH]{};
	TCHAR section[256];
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumOpts"), -1, CFGFile);
	if(num > avail || avail == -1)
		return false;
	
	_stprintf(section, _T("Option%.3d"), num);
	WritePrivateProfileString(section, _T("SaveName"), Name, CFGFile);
	return true;
}

TCHAR* GetOptSaveName(TCHAR* file, int num, TCHAR *out) {
	TCHAR CFGFile[MAX_PATH]{};
	TCHAR section[256];

	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumOpts"), -1, CFGFile);
	if(num > avail || avail == -1)
		return NULL;
	
	_stprintf(section, _T("Option%.3d"), num);
	GetPrivateProfileString(section, _T("SaveName"), NULL, out, 64, CFGFile);
	return out;
}

bool DeleteSaveData(TCHAR* file, int num) {
	TCHAR CFGFile[MAX_PATH]{};
	TCHAR section[256];
	TCHAR sectionData[65766];
	TCHAR tmp[64];
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumOpts"), -1, CFGFile);
	if(num > avail || avail == -1)
		return false;
	
	_itot(avail-1, tmp, 10);
	WritePrivateProfileString(_T("General"), _T("NumOpts"), tmp, CFGFile);
	if(g_CurSaveData != 1 && g_CurSaveData == avail)
		g_CurSaveData--;
	_stprintf(section, _T("Option%.3d"), num);
	_itot(g_CurSaveData, tmp, 10);
	WritePrivateProfileString(section, _T("LastOpt"), tmp, CFGFile);
	// move
	for(int i = num+1; i <= avail; i++) {
		TCHAR nextSec[256];
		_stprintf(section, _T("Option%.3d"), i-1);
		_stprintf(nextSec, _T("Option%.3d"), i);
		// update file
		GetPrivateProfileSection(nextSec, sectionData, 65766, CFGFile);
		WritePrivateProfileSection(section, sectionData, CFGFile);
	}
	// empty last option
	_stprintf(section, _T("Option%.3d"), avail);
	WritePrivateProfileSection(section, _T("\0\0"), CFGFile);
	
	// remove esp history file
	TCHAR eh_name[30];
	_stprintf(eh_name, _T("EspHist.%d.dat"), num);
	genstring eh_file = GetSaveFolder() + eh_name;
	DeleteFile(eh_file.c_str());
	return true;
}

int NewSetFleetData(TCHAR* file, int num, const vector<SItem> &Fleet, ShipTechs Techs) {
	TCHAR section[256];
	TCHAR key[256];
	TCHAR CFGFile[MAX_PATH]{};
	size_t i;

	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumFleets"), 0, CFGFile);
	if(num > avail) {
		// add new fleet
		num = ++avail;
		_itot(num, key, 10);
		WritePrivateProfileString(_T("General"), _T("NumFleets"), key, CFGFile);
	}
	_stprintf(section, _T("Fleet%.3d"), num);
	
	for(i = 0; i < T_END; i++) {
		TCHAR name[128];
		sim.ItemIniName((ITEM_TYPE)i, name);
		WritePrivateProfileString(section, name, _T("0"), CFGFile);
	}
	
	// ships
	for(i = 0; i < Fleet.size(); i++) {
		TCHAR name[128];
		_itot(Fleet[i].Num, key, 10);
		sim.ItemIniName(Fleet[i].Type, name);
		WritePrivateProfileString(section, name, key, CFGFile);
	}
	// techs
	_itot(Techs.Weapon, key, 10);
	WritePrivateProfileString(section, _T("Weapon"), key, CFGFile);
	_itot(Techs.Shield, key, 10);
	WritePrivateProfileString(section, _T("Shield"), key, CFGFile);
	_itot(Techs.Armour, key, 10);
	WritePrivateProfileString(section, _T("Armour"), key, CFGFile);
	return num;
}

bool NewGetFleetData(TCHAR* file, int num, vector<SItem> &Fleet, ShipTechs &Techs) {
	TCHAR section[256];
	TCHAR CFGFile[MAX_PATH]{};
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumFleets"), 0, CFGFile);
	if(num > avail)
		return false;
	_stprintf(section, _T("Fleet%.3d"), num);
	Fleet.clear();
	for(int i = 0; i < T_END; i++) {
		TCHAR name[128];
		SItem Item;
		Item.Type = (ITEM_TYPE)i;
		sim.ItemIniName((ITEM_TYPE)i, name);
		Item.Num = GetPrivateProfileInt(section, name, 0, CFGFile);
		if(Item.Num)
			Fleet.push_back(Item);
	}

	Techs.Weapon = GetPrivateProfileInt(section, _T("Weapon"), 0, CFGFile);
	Techs.Shield = GetPrivateProfileInt(section, _T("Shield"), 0, CFGFile);
	Techs.Armour = GetPrivateProfileInt(section, _T("Armour"), 0, CFGFile);
	return true;
}

bool SetFleetSaveName(TCHAR *file, int num, const TCHAR *Name) {
	TCHAR CFGFile[MAX_PATH];
	TCHAR section[256];
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumFleets"), -1, CFGFile);
	if(num > avail || avail == -1)
		return false;
	
	_stprintf(section, _T("Fleet%.3d"), num);
	WritePrivateProfileString(section, _T("FleetName"), Name, CFGFile);
	return true;
}

TCHAR* GetFleetSaveName(TCHAR* file, int num, TCHAR *out) {
	TCHAR CFGFile[MAX_PATH]{};
	TCHAR section[256];

	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumFleets"), -1, CFGFile);
	if(num > avail || avail == -1)
		return NULL;
	
	_stprintf(section, _T("Fleet%.3d"), num);
	GetPrivateProfileString(section, _T("FleetName"), NULL, out, 64, CFGFile);
	return out;
}

bool DeleteFleetData(TCHAR* file, int num) {
	TCHAR CFGFile[MAX_PATH]{};
	TCHAR section[256];
	TCHAR sectionData[65766];
	TCHAR tmp[64];
	genstr nfile = GetSaveFolder() + file;
	_tcsncpy(CFGFile, nfile.c_str(), MAX_PATH);
	/*_tcscpy(CFGFile, g_CurrDir);
	_tcscat(CFGFile, file);*/
	int avail = GetPrivateProfileInt(_T("General"), _T("NumFleets"), -1, CFGFile);
	if(num > avail || avail == -1)
		return false;
	g_NumFleets--;
	_itot(avail-1, tmp, 10);
	WritePrivateProfileString(_T("General"), _T("NumFleets"), tmp, CFGFile);
	_stprintf(section, _T("Fleet%.3d"), num);
	// move
	for(int i = num+1; i <= avail; i++) {
		TCHAR nextSec[256];
		_stprintf(section, _T("Fleet%.3d"), i-1);
		_stprintf(nextSec, _T("Fleet%.3d"), i);
		// update file
		GetPrivateProfileSection(nextSec, sectionData, 65766, CFGFile);
		WritePrivateProfileSection(section, sectionData, CFGFile);
	}
	// empty last fleet
	_stprintf(section, _T("Fleet%.3d"), avail);
	WritePrivateProfileSection(section, _T("\0\0"), CFGFile);
	return true;
}

bool SaveEspHistory(int save_id)
{
	genstrstream file;
	file << _T("EspHist.") << save_id << _T(".dat\0");
	genstr s = GetSaveFolder() + file.str();
	const TCHAR* file_n = s.c_str();

	char c_file[MAX_PATH];
#ifdef UNICODE
	wcstombs(c_file, file_n, MAX_PATH);
#else
	strncpy(c_file, file_n, MAX_PATH);
#endif

	FILE* f = fopen(c_file, "w");
	fclose(f);

	vector<EspHistItem>::reverse_iterator it = g_vRaidHistory.rbegin();
	int i = 0;
	for(; it != g_vRaidHistory.rend() && i < g_Options.cHistInFile; it++)
	{
		i++;
		genstrstream tmp, section;
		tmp << i;
		section.str().erase();
		
		section << _T("TargetName=") << it->Target.Name << _T("\r");
		section << _T("TargetPos=") << it->Target.Pos.ToString() << _T("\r");
		section << _T("Metal=") << (long)it->Target.Resources.met << _T("\r");
		section << _T("Crystal=") << (long)it->Target.Resources.kris << _T("\r");
		section << _T("Deuterium=") << (long)it->Target.Resources.deut << _T("\r");
		section << _T("WeaponTech=") << it->Target.Techs.Weapon << _T("\r");
		section << _T("ShieldTech=") << it->Target.Techs.Shield << _T("\r");
		section << _T("ArmourTech=") << it->Target.Techs.Armour << _T("\r");

		// fleet
		vector<SItem>::const_iterator fleet = it->Target.Fleet.begin();
		for(; fleet != it->Target.Fleet.end(); fleet++)
		{
			TCHAR name[256];
			sim.ItemIniName(fleet->Type, name);
			section << name << _T("=") << (long)fleet->Num << _T("\r");
		}
		
		// def
		fleet = it->Target.Defence.begin();
		for(; fleet != it->Target.Defence.end(); fleet++)
		{
			TCHAR name[256];
			sim.ItemIniName(fleet->Type, name);
			section << name << _T("=") << (long)fleet->Num << _T("\r");
		}
		section << _T("ABM=") << it->Target.NumABM << _T("\r");
		section << _T("Time=") << it->ctime << _T("\r");
		section << _T("\r");
		int a = (int)section.str().length();
		TCHAR* data = new TCHAR[a + 1];
		_tcsncpy(data, section.str().c_str(), a);
		data[a] = 0;
		while(a)
		{
			if(data[a] == '\r')
				data[a] = 0;
			a--;
		}
		WritePrivateProfileSection(tmp.str().c_str(), data, file_n);
		delete[] data;
	}
	return true;
}

bool LoadEspHistory(int save_id)
{
	genstrstream file;
	file << _T("EspHist.") << save_id << _T(".dat\0");
	genstr s = GetSaveFolder() + file.str();
	const TCHAR* file_n = s.c_str();

	char c_file[MAX_PATH];
#ifdef UNICODE
	wcstombs(c_file, file_n, MAX_PATH);
#else
	strncpy(c_file, file_n, MAX_PATH);
#endif
	FILE* f = fopen(c_file, "r+");
	if(!f)
		return false;
	fclose(f);
	
	int i;
	for(i = g_Options.cHistInFile; i > 0; i--)
	{
		TargetInfo ti;
		genstrstream tmp; tmp << i;
		TCHAR sec[25]; _tcsncpy(sec, tmp.str().c_str(), 24);
		TCHAR s[256];

		GetPrivateProfileString(sec, _T("TargetName"), _T(""), ti.Name, 64, file_n);
		GetPrivateProfileString(sec, _T("TargetPos"), _T(""), s, 256, file_n);
		if(!_tcslen(s))
			continue;
		ti.Pos = PlaniPos(s);
		ti.Resources.met = GetPrivateProfileInt(sec, _T("Metal"), 0, file_n);
		ti.Resources.kris = GetPrivateProfileInt(sec, _T("Crystal"), 0, file_n);
		ti.Resources.deut = GetPrivateProfileInt(sec, _T("Deuterium"), 0, file_n);
		ti.Techs.Weapon = GetPrivateProfileInt(sec, _T("WeaponTech"), 0, file_n);
		ti.Techs.Shield = GetPrivateProfileInt(sec, _T("ShieldTech"), 0, file_n);
		ti.Techs.Armour = GetPrivateProfileInt(sec, _T("ArmourTech"), 0, file_n);

		int j;
		for(j = 0; j < T_END; j++)
		{
			SItem si;
			si.Type = (ITEM_TYPE)j;
			TCHAR name[256]; sim.ItemIniName((ITEM_TYPE)j, name);
			si.Num = GetPrivateProfileInt(sec, name, 0, file_n);
			if(si.Num)
			{
				if(j < T_SHIPEND)
					ti.Fleet.push_back(si);
				else
					ti.Defence.push_back(si);
			}
		}
		ti.NumABM = GetPrivateProfileInt(sec, _T("ABM"), 0, file_n);

		time_t t;
		t = GetPrivateProfileInt(sec, _T("Time"), 0, file_n);
		if(t == 0)
			AddReportToHistory(ti);
		else
			AddReportToHistory(ti, &t);
	}
	return true;
}

genstr GetSaveFolder()
{
	TCHAR folder[MAX_PATH];
	SHGetSpecialFolderPath(g_hwndDlg, folder, CSIDL_APPDATA, TRUE);
	genstr name = genstr(folder) + _T("\\SpeedSim");
	CreateDirectory(name.c_str(), NULL);
	name += _T("\\");
	return name;
}
