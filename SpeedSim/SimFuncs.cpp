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

LPTSTR AddPointsToNumber(__int64 value, LPTSTR out) {
	bool neg = false;
	if(value < INT_MAX && value > INT_MIN)
		_stprintf(out, _T("%d"), value);
	else {
		if((value % 1000 == 0) && (value / 1000 < INT_MAX) && ((value / 1000 > INT_MIN))) {
			value /= 1000;
			_stprintf(out, _T("%d"), value);
			_tcscat(out, _T("000"));
		}
		else
			_stprintf(out, _T("%I64d"), value);
	}
	genstring c = out;

	div_t res;
	if(value < 0) {
		c.erase(0, 1);
		neg = true;
	}
	res = div(c.length(), 3);
	if(res.rem == 0)
		res.quot--;
	for(int i = 1; i <= res.quot; i++) {
		c.insert(c.length() - i * 4 + 1, g_DecSeperator);
	}
	if(neg)
		c.insert(0, _T("-"));
	_tcscpy(out, c.data());
	return out;
}

void ReadValuesAndSim(void* p) {
	g_SimCount++;
	if(g_SimCount > 1) {
		g_SimCount--;
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_SIM), true);
		g_SimThread = g_LastSimThread;
		g_LastSimThread = NULL;
		ExitThread(0);
		return;
	}
	if(g_LastSimThread) {
		g_LastSimThread = 0;
	}
	// prevent user from entering anything
	EnableAllWindows(false);
	SendDlgItemMessage(g_hwndDlg, IDC_SIM, WM_SETTEXT, 0, (LPARAM)g_strSim[1].c_str());
	UpdateKernel();
	//sim.SetNewShield(false);
	UpdateEditFields(true, true);
	ClearResult();
	sim.SetCallBack(CallBack);
	// simulate battle
	bool res = sim.Simulate(ReadFromEdit(IDC_NUM_SIM));
	// show result
	if(res) {
		SItem att[T_END], def[T_END];
		sim.GetFleetAfterSim(att, def, ReadFromEdit(IDC_FLEET_ID)-1);
		ShowFightResults(sim.GetBattleResult(), att, def, true);
	}
	g_SimCount--;
	EnableAllWindows(true);
	if(g_bLowRes)
	{
		// deactivate Skins
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_CHG_SKIN), false);
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_USE_SKIN), false);
	}
	if(ReadFromEdit(IDC_FLEET_ID) > 1)
		EnableDefense(false);
	if(!res)
		EnableWindow(GetDlgItem(g_hwndDlg, IDC_LOAD_REM_DEF), false);
	SendDlgItemMessage(g_hwndDlg, IDC_SIM, WM_SETTEXT, 0, (LPARAM)g_strSim[0].c_str());
	SendDlgItemMessage(g_hwndDlg, IDC_RUNNING_NUM, WM_SETTEXT, 0, (LPARAM)g_strSimState[0].c_str());
	InvalidateRect(g_hwndDlg, NULL, true);
	ExitThread(0);
	return;
}

void SimulateIPMAttack()
{
	int i;
	UpdateKernel();
	UpdateEditFields(true, true);
	int nIPM, nABM;
	nIPM = ReadFromEdit(ID_INPUT_IPM);
	nABM = ReadFromEdit(ID_INPUT_ABM);

	int target = T_RAK;
	for(i = 0; i < T_END - T_SHIPEND; i++)
	{
		if(SendDlgItemMessage(g_hwndDlg, ID_FIRST_RADIO_IPM + i, BM_GETSTATE, 0, 0) & BST_CHECKED)
		{
			target = i + T_SHIPEND;
			break;
		}
	}

	IPMBattleResult br = sim.SimulateIPM(nIPM, nABM, 0, (ITEM_TYPE)target);
	
	SItem att[T_END], def[T_END];
	sim.GetFleetAfterSim(att, def, ReadFromEdit(IDC_FLEET_ID)-1);
	BattleResult b = BattleResult();
	ShowFightResults(b, att, def, false);
	if(nIPM)
		SetDlgItemInt(g_hwndDlg, ID_OUTPUT_IPM, br.NumIPM, false);
	else
		SetDlgItemText(g_hwndDlg, ID_OUTPUT_IPM, _T(""));
	if(nABM)
		SetDlgItemInt(g_hwndDlg, ID_OUTPUT_ABM, br.NumABM, false);
	else
		SetDlgItemText(g_hwndDlg, ID_OUTPUT_ABM, _T(""));
	
	TCHAR c[256], tmp[256], tmp2[64], tmp3[64];
	// needed IPMs
	_stprintf(c, _T("[%d]"), br.NeededMissiles);
	SetDlgItemText(g_hwndDlg, ID_OUTPUT_NEEDED_IPM, c);
	// losses on both sides
	// attacker
	_stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(br.LossesAttacker.met, tmp), 
		AddPointsToNumber(br.LossesAttacker.kris, tmp2), AddPointsToNumber(br.LossesAttacker.deut, tmp3));
	SendDlgItemMessage(g_hwndDlg, IDC_VERL_A, WM_SETTEXT, 0,(LPARAM)c);
	// defender
	_stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(br.LossesDefender.met, tmp), 
		AddPointsToNumber(br.LossesDefender.kris, tmp2), AddPointsToNumber(br.LossesDefender.deut, tmp3));
	SendDlgItemMessage(g_hwndDlg, IDC_VERL_V, WM_SETTEXT, 0,(LPARAM)c);
	// name / coordinates of planet
	if(!_tcslen(br.PlaniName) && !br.Position.Gala)
		_tcscpy(tmp, g_ResString[10].c_str());
	else if(_tcslen(br.PlaniName)) {
		_tcsncpy(tmp, br.PlaniName, 63);
		_tcscat(tmp, _T(" "));
	}
	if(br.Position.Gala) {
		_stprintf(tmp2, _T("[%s]"), br.Position.ToString().c_str(), 63);
		_tcsncat(tmp, tmp2, 63);
	}
	// moon check
	if(br.Position.bMoon)
		SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_CHECKED, NULL);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_UNCHECKED, NULL);

	UpdateTooltips();

}

void ShowFightResults(BattleResult& batres, SItem *Att, SItem *Def, bool bCreateReports)
{
	//SItem Att[T_END], Def[T_END];
	//sim.GetFleetAfterSim(Att, Def, ReadFromEdit(IDC_FLEET_ID)-1);
	int FleetID = ReadFromEdit(IDC_FLEET_ID) - 1;
	if(FleetID < 0 || FleetID > 15)
		FleetID = 0;
	ClearResult();
	// output fleet
	TCHAR c[256], d[256];
	
	// attacker
	int i = 0;
	for(i = 0; i < T_SHIPEND; i++)
	{
		if(Att[i].Type == T_SAT || Att[i].Type == T_NONE)
			continue;
		if(Att[i].Num == 0 && !ReadFromEdit(GetEditIdByType((ITEM_TYPE)i, true)))
			c[0] = '\0';
		else if(ceil(Att[i].Num) == Att[i].Num)
			_stprintf(c, _T("%.0f"), Att[i].Num);
		else
			_stprintf(c, _T("%.1f"), Att[i].Num);
		SendDlgItemMessage(g_hwndDlg, GetLabelIdByType(Att[i].Type, true), WM_SETTEXT, 0, (LPARAM)c);
	}
	// defender
	for(i = 0; i < T_END; i++)
	{
		if(Def[i].Type == T_NONE)
			continue;
		if(Def[i].Num == 0 && !ReadFromEdit(GetEditIdByType((ITEM_TYPE)i, false)))
			c[0] = '\0';
		else if(ceil(Def[i].Num) == Def[i].Num)
			_stprintf(c, _T("%.0f"), Def[i].Num);
		else
			_stprintf(c, _T("%.1f"), Def[i].Num);
		if(g_IPMMode && Def[i].Type < T_SHIPEND)
			continue;
		SendDlgItemMessage(g_hwndDlg, GetLabelIdByType(Def[i].Type, false), WM_SETTEXT, 0, (LPARAM)c);
	}
	
	// debris
	TCHAR tmp[256], tmp2[64], tmp3[64];
	ZeroMemory(tmp, 64);ZeroMemory(tmp2, 64);ZeroMemory(tmp3, 64);

	_stprintf(c, g_ResString[4].c_str(), AddPointsToNumber(batres.TF.met, tmp), batres.PercentInTFMet,
		AddPointsToNumber(batres.TF.kris, tmp2), batres.PercentInTFKris, batres.MaxNumRecs);
	SendDlgItemMessage(g_hwndDlg, IDC_TF, WM_SETTEXT, 0,(LPARAM)c); 
	
	// losses on both sides
	// attacker
	_stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(batres.VerlusteAngreifer.met, tmp), 
		AddPointsToNumber(batres.VerlusteAngreifer.kris, tmp2), AddPointsToNumber(batres.VerlusteAngreifer.deut, tmp3));
	SendDlgItemMessage(g_hwndDlg, IDC_VERL_A, WM_SETTEXT, 0,(LPARAM)c);
	// defender
	_stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(batres.VerlusteVerteidiger.met, tmp), 
		AddPointsToNumber(batres.VerlusteVerteidiger.kris, tmp2), AddPointsToNumber(batres.VerlusteVerteidiger.deut, tmp3));
	SendDlgItemMessage(g_hwndDlg, IDC_VERL_V, WM_SETTEXT, 0,(LPARAM)c); 
	
	// plunder (real)
	_stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(batres.Beute.met, tmp), 
		AddPointsToNumber(batres.Beute.kris, tmp2), AddPointsToNumber(batres.Beute.deut, tmp3));
	_stprintf(d, g_ResString[6].c_str(), c, (int)batres.Ausbeute);
	SendDlgItemMessage(g_hwndDlg, IDC_BEUT_PRAK, WM_SETTEXT, 0,(LPARAM)d);
	
	Res th = batres.RessDa;
	th /= 2;
	int iNeededCap = batres.iNeededCapacity;
	
	for(i = 0; i < T_SHIPEND; i++)
	{
		int iShipCap = sim.GetShipCapacity((ITEM_TYPE)i);
		if(iShipCap > 0)
			g_iNumShipsForPlunderNeeded[i] = ceil(iNeededCap / (float)iShipCap);
	}
	// theoretical plunder
	TCHAR szShipName[100];
	sim.ItemName(g_Options.PlunderShip, szShipName);
	_stprintf(d, g_ResString[5].c_str(), AddPointsToNumber(th.met, tmp), AddPointsToNumber(th.kris, tmp2), AddPointsToNumber(th.deut, tmp3));
	_stprintf(c, g_ResString[7].c_str(), d, g_iNumShipsForPlunderNeeded[g_Options.PlunderShip], szShipName);
	SendDlgItemMessage(g_hwndDlg, IDC_BEUT_TH, WM_SETTEXT, 0,(LPARAM)c);
	
	// chance for getting moon
	int moonchance = (batres.TF.met + batres.TF.kris) / 100000;
	if(moonchance < 1)
		moonchance = 0;
	else if(moonchance > 20)
		moonchance = 20;
	if(moonchance > 0)
		_stprintf(c, g_ResString[8].c_str(), moonchance);
	else
		_tcscpy(c, g_ResString[9].c_str());
	SendDlgItemMessage(g_hwndDlg, IDC_MOND, WM_SETTEXT, 0,(LPARAM)c);
	
	// battle results
	c[0] = '\0';
	if(batres.AttWon > 0)
		_stprintf(c, g_ResString[0].c_str(), batres.AttWon * 100);
	if(batres.DefWon > 0)
		_stprintf(c, g_ResString[1].c_str(), c, batres.DefWon * 100);
	if(batres.Draw > 0)
		_stprintf(c, g_ResString[2].c_str(), c, batres.Draw * 100);
	if(_tcslen(c))
		_stprintf(c, g_ResString[3].c_str(), c, batres.NumRounds);
	SendDlgItemMessage(g_hwndDlg, IDC_END, WM_SETTEXT, 0,(LPARAM)c);
	
	ZeroMemory(tmp, 63); ZeroMemory(tmp2, 63);
	// name / coordinates of planet
	if(!_tcslen(batres.PlaniName) && !batres.Position.Gala)
		_tcscpy(tmp, g_ResString[10].c_str());
	else if(_tcslen(batres.PlaniName)) {
		_tcsncpy(tmp, batres.PlaniName, 63);
		_tcscat(tmp, _T(" "));
	}
	if(batres.Position.Gala) {
		_stprintf(tmp2, _T("[%s]"), batres.Position.ToString().c_str(), 63);
		_tcsncat(tmp, tmp2, 63);
	}
	_stprintf(c, g_ResString[11].c_str(), tmp);

	SendDlgItemMessage(g_hwndDlg, IDC_SCAN_END, WM_SETTEXT, 0,(LPARAM)c);
	SendDlgItemMessage(g_hwndDlg, IDC_SCAN_END, EM_LINESCROLL, 64, 0);
	if(batres.Position.bMoon)
		SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_CHECKED, NULL);
	else
		SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_UNCHECKED, NULL);
	

	// needed fuel
	_stprintf(c, g_ResString[12].c_str(), AddPointsToNumber(batres.SpritVerbrauch, tmp));
	_stprintf(tmp, g_ResString[12].c_str(), AddPointsToNumber(batres.RecFuel[FleetID], tmp2));
	_stprintf(tmp2, _T(" (%s: %s)"), sim.ItemName(T_REC).c_str(), tmp);
	_tcsncat(c, tmp2, 255);
	SendDlgItemMessage(g_hwndDlg, IDC_FLYCOST, WM_SETTEXT, 0,(LPARAM)c);
	// flying time
	int min, h, sec;
	div_t res;
	DWORD ft = batres.FlyTime;
	if(g_Options.iSpeedFactor < 1)
		g_Options.iSpeedFactor = 1;
	ft /= g_Options.iSpeedFactor;
	res = div(ft, 3600);
	h = res.quot;
	ft -= h * 3600;
	res = div(ft, 60);
	min = res.quot;
	ft -= min * 60;
	sec = ft;
	_stprintf(c, _T("%.2d:%.2d:%.2d h"), h, min, sec);
	ft = batres.RecFlyTime[FleetID] / g_Options.iSpeedFactor;
	res = div(ft, 3600);
	h = res.quot;
	ft -= h * 3600;
	res = div(ft, 60);
	min = res.quot;
	ft -= min * 60;
	sec = ft;
	_stprintf(tmp, _T("%s (%s: %.2d:%.2d:%.2d h)"), c, sim.ItemName(T_REC).c_str(), h, min, sec);
	SendDlgItemMessage(g_hwndDlg, IDC_FLUGZEIT, WM_SETTEXT, 0,(LPARAM)tmp);
	// wave loot
	_stprintf(d, g_ResString[5].c_str(), AddPointsToNumber(batres.GesamtBeute.met, tmp), 
		AddPointsToNumber(batres.GesamtBeute.kris, tmp2), AddPointsToNumber(batres.GesamtBeute.deut, tmp3));
	_stprintf(c, g_ResString[13].c_str(), d, batres.NumAtts, batres.GesamtRecs);
	SendDlgItemMessage(g_hwndDlg, IDC_BEUT_GES, WM_SETTEXT, 0,(LPARAM)c);

	if(bCreateReports)
	{
		char c_file[MAX_PATH];
		// combat report
		genstr file = GetSaveFolder() + _T("cr.htm");
#ifdef UNICODE
		wcstombs(c_file, file.c_str(), MAX_PATH);
#else
		strncpy(c_file, file.c_str(), MAX_PATH);
#endif
		sim.GenerateCR(c_file);

		// write B/W-Case into ile
		file = GetSaveFolder() + _T("BW.htm");
#ifdef UNICODE
		wcstombs(c_file, file.c_str(), MAX_PATH);
#else
		strncpy(c_file, file.c_str(), MAX_PATH);
#endif
		sim.GenerateBWC(c_file);
		// show up Best/Worse Case
		if(IsDlgButtonChecked(g_hwndDlg, IDC_BW_CASE)) {
			HINSTANCE hi = ShellExecute(NULL, _T("opennew"), file.c_str(), NULL, NULL, SW_SHOWNORMAL);
			// if not IE..
			if((int)hi <= 32) {
				ShellExecute(NULL, _T("open"), file.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}

	UpdateTooltips();
	return;
}
