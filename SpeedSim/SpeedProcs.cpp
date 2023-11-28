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

extern genstring g_LoadSaveDlg[16];

int g_GeradeEingefuegt;
COLORREF g_TabColor;
HBRUSH g_TabBkBrush = NULL;

INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // messages by other programs
    if(uMsg == SCAN_MSG) {
        OnScanMessage();
        return TRUE;
    }

    switch(uMsg) {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;
        InitMainDialog();
        break;
    case WM_CLOSE:
        SaveOpts(g_CurSaveData);
        PostQuitMessage(0);
        break;
    case TRAY_MESSAGE:
        return OnTrayMessage(uMsg, lParam, wParam);
        break;
    case WM_SIZE:
        OnSize(lParam, wParam);
        break;
    case WM_COMMAND:
        if(IsMenuMsg(uMsg, lParam, wParam))
            return TRUE;
        // checkboxes
        if(LOWORD(wParam) >= ID_FIRST_RADIO_IPM && LOWORD(wParam) < ID_FIRST_RADIO_IPM + T_END - T_SHIPEND)
        {
            if(g_iCheckedIPMBox + ID_FIRST_RADIO_IPM - 1 != LOWORD(wParam))
            {
                SendDlgItemMessage(hwndDlg, g_iCheckedIPMBox + ID_FIRST_RADIO_IPM - 1, BM_SETCHECK, BST_UNCHECKED, 0);
                g_iCheckedIPMBox = LOWORD(wParam) + 1 - ID_FIRST_RADIO_IPM;
            }
        }
        switch(LOWORD(wParam)) 
        {
        case ID_DATEI_TECHNOLOGIEENSPEICHERN:
        case ID_DATEI_EINSTELLUNGENSPEICHERN_NEU:
        case ID_FLOTTEN_SPEICHERN_ANGREIFER_NEU:
        case ID_FLOTTEN_SPEICHERN_VERTEIDIGER_NEU:
        case ID_DATEI_ERWITERTEEINSTELLUNGEN:
        case ID_FLOTTEN_ALLEFLOTTENSLOTSLEEREN:
        case IDM_CLOSE:
        case IDM_IPMMODE:
        case IDM_SHOW_ESPHIST:
            OnMenuMsg(lParam, wParam);
            break;
        case IDM_ESPIO_MANAG:
            //ShowEspioManager();
            break;
        case IDC_EXIT:
            PostMessage(g_hwndDlg, WM_CLOSE, 0, 0);
            break;
        case IDC_SIM:
            if(HIWORD(wParam) != BN_CLICKED)
                break;
            // start/cancel simulation
            if(g_IPMMode)
            {
                SimulateIPMAttack();
            }
            else
            {
                if(g_SimCount < 1) {
                    if(g_SimCount < 0)
                        g_SimCount = 0;
                    g_LastSimThread = g_SimThread;
                    EnableWindow(GetDlgItem(g_hwndDlg, IDC_SIM), false);
                    g_SimThread = _beginthread(ReadValuesAndSim, 0, NULL);
                }
                else {
                    sim.AbortSim();
                }
            }
            break;
        case ID_SHOWPOPUP:
            // Ctrl+Enter -> fall through
            if(g_SimCount)
                break;
        case IDC_SHOWPOPUP:
            if(HIWORD(wParam) != BN_CLICKED && LOWORD(wParam) != ID_SHOWPOPUP)
                break;
            {
                UpdateKernel();
                SItem att[T_END];
                sim.GetSetFleet(att, NULL, ReadFromEdit(IDC_FLEET_ID)-1);
                ShowWindow(g_hwndDlg, SW_MINIMIZE);
                vector<SItem> vAtt;
                for(int i = 0; i < T_END; i++) {
                    vAtt.push_back(att[i]);
                }
                CreatePopupInfo(sim.GetBattleResult(), vAtt, -1);
            }
            break;
        case IDC_IS_MOON:
            {
                int FleetID = ReadFromEdit(IDC_FLEET_ID)-1;
                TargetInfo ti = sim.GetTargetInfo(FleetID);
                if(IsDlgButtonChecked(g_hwndDlg, IDC_IS_MOON))
                    ti.Pos.bMoon = true;
                else
                    ti.Pos.bMoon = false;
                sim.SetTargetInfo(ti, FleetID, false);
            }
            break;
        case IDC_SWITCH_FLEET:
            SwapFleets();
            break;
        case IDC_READ_GEN:
            UpdateKernel();
			GeneralRead();
            UpdateEditFields(true, true);
            break;
        case IDC_DEL_GEN:
            SendDlgItemMessage(hwndDlg, IDC_GEN_READ, WM_SETTEXT, 0, (LPARAM)_T(""));
            break;
        case IDC_RESET_ATT:
            ResetAtt();
            break;
        case IDC_RESET_DEF:
            ResetDef();
            break;
        case IDC_CLIP_COPY:
            CopyResultToClipboard();
            break;
		case IDC_SHOW_KB:
            {
            genstr s = GetSaveFolder() + _T("cr.htm");
			HINSTANCE hi = ShellExecute(NULL, _T("opennew"), s.c_str(), NULL, NULL, SW_SHOWNORMAL);
            // if not IE
            if((INT_PTR)hi <= 32)
                ShellExecute(NULL, _T("open"), s.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
			break;
        case IDC_SHOW_BW:
            {
            genstr s = GetSaveFolder() + _T("BW.htm");
            HINSTANCE hi = ShellExecute(NULL, _T("opennew"), s.c_str(), NULL, NULL, SW_SHOWNORMAL);
            // if not IE
            if((INT_PTR)hi <= 32)
                ShellExecute(NULL, _T("open"), s.c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            break;
        case IDC_LOAD_REM_DEF:
            sim.SetRemainingItemsInDef();
            UpdateEditFields(true, true);
            EnableWindow(GetDlgItem(g_hwndDlg, IDC_LOAD_REM_DEF), false);
            break;
        case IDC_DELWAVESTATE:
            sim.ResetWaveState();
            UpdateEditFields(false, true);
            break;
        case IDC_UPDATE:
            if(HIWORD(wParam) != BN_CLICKED)
                break;
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_UPDATE), g_hwndDlg, UpdateProc);
            break;
        case IDC_PREV_TARGET:
            SetNextRaidlistFleet();
            break;
        case IDC_NEXT_TARGET:
            SetPrevRaidListFleet();
            break;
        case IDC_ADD_TARGETINFO:
            AddFleetToRaidlist();
            break;
        case IDC_REMOVETARGETINFO:
            RemoveCurFromRaidlist();
            break;
		case IDC_USE_SKIN:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				// use skin?
				if(!IsDlgButtonChecked(g_hwndDlg, IDC_USE_SKIN))
				{
					_tcscpy(g_Options.SkinFileName, _T(""));
                    SetSkin(_T(""));
                    
                    SetFontForAllChilds(g_hwndDlg, g_Font);
                    InvalidateRect(g_hwndDlg, NULL, TRUE);
				}
                else {
                    // choose skin
                    SendMessage(g_hwndDlg, WM_COMMAND, IDC_CHG_SKIN, 0);
                }
			}
			break;
        case IDC_CHG_SKIN:
            {
            OPENFILENAME of;
			TCHAR filename[MAX_PATH];
			filename[0] = 0;
			of.lStructSize = sizeof(OPENFILENAME);
			of.hwndOwner = g_hwndDlg;
			of.hInstance = NULL;
			of.lpstrFilter = _T("SpeedSim skins (*.ini)\0*.ini\0All Files(*.*)\0*.*\0\0");
			of.lpstrCustomFilter = NULL;
			of.nMaxCustFilter = 0;
			of.nFilterIndex = 1;
			of.lpstrFile = filename;
			of.nMaxFile = MAX_PATH * sizeof(TCHAR);
			of.lpstrFileTitle = NULL;
			of.nMaxFileTitle = 0;
			of.lpstrInitialDir = g_CurrDir;
			of.lpstrTitle = NULL;
			of.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			of.lpstrDefExt = _T("ini");
			if(GetOpenFileName(&of))
			{
				_tcscpy(g_Options.SkinFileName, &filename[of.nFileOffset]);
                int l1 = (int)_tcslen(g_CurrDir), l2 = of.nFileOffset;
                int len = l1 > l2 ? l1 : l2;
                if(_tcsncicmp(g_CurrDir, filename, len)) {
                    _tcscpy(g_Options.SkinFileName, _T(""));
                    MessageBox(g_hwndDlg, g_MiscStr[5].c_str(), _T("Error"), MB_OK|MB_ICONERROR);
                    SendDlgItemMessage(g_hwndDlg, IDC_USE_SKIN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
                    break;
                }
                if(!IsSkinFile(g_Options.SkinFileName)) {
                    // abort if no skin file
                    _tcscpy(g_Options.SkinFileName, _T(""));
                    SendDlgItemMessage(g_hwndDlg, IDC_USE_SKIN, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                }
                ShowSkinInfo(filename);
                SetSkin(g_Options.SkinFileName);
                
                SendDlgItemMessage(g_hwndDlg, IDC_USE_SKIN, BM_SETCHECK, BST_CHECKED, 0);
            }
            else 
			    SendDlgItemMessage(g_hwndDlg, IDC_USE_SKIN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
            }
            break;
        case IDC_USE_LANG:
            if(HIWORD(wParam) == BN_CLICKED)
			{
				if(!IsDlgButtonChecked(g_hwndDlg, IDC_USE_LANG))
				{
					strcpy(g_Options.LangFileName, "");
					LoadLang(NULL);
				}
                else
                    SendMessage(g_hwndDlg, WM_COMMAND, IDC_CHG_LANG, 0);
			}
            break;
        case IDC_CHG_LANG:
            // new language change dialog
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CHOOSELANG), g_hwndDlg, LangProc);
            break;
        case IDC_DEL_COORD:
            {
                SetDlgItemText(g_hwndDlg, IDC_OWN_POS, _T(""));
                int n = SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_GETCURSEL, 0, 0);
                SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_DELETESTRING, n, 0);
            }
            break;    
		case IDC_OWN_POS:
            OnOwnPos(lParam, wParam);
            break;
        case IDC_HOOK_CB:
            if(IsDlgButtonChecked(g_hwndDlg, IDC_HOOK_CB) && !g_Options.HookClipboard) {
                g_hwndNextCBWnd = SetClipboardViewer(g_hwndDlg);
                g_Options.HookClipboard = true;
            }
            else if(g_Options.HookClipboard) {
                ChangeClipboardChain(g_hwndDlg, g_hwndNextCBWnd);
                g_Options.HookClipboard = false;
            }
            break;
        case IDC_EDIT_SPIO:
            UpdateKernel();
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CHGSPIO), NULL, SpioProc);
            break;
        case IDC_BILANZ:
            ShowBilanz();
            break;
        case IDC_NEXT_FL:
            {
            int id = ReadFromEdit(IDC_FLEET_ID)+1;
            if(id > 16)
                id = 16;
            SetDlgItemInt(g_hwndDlg, IDC_FLEET_ID, id, false);
            }
            break;
        case IDC_PREV_FL:
            {
            int id = ReadFromEdit(IDC_FLEET_ID)-1;
            if(id < 1)
                id = 1;
            SetDlgItemInt(g_hwndDlg, IDC_FLEET_ID, id, false);
            }
            break;
        case IDC_FLEET_ID:
            OnFleetID(lParam, wParam);
            break;
        case ID_TAKE_OVER_IPM_RES:
            {
                ShipTechs tech;
                tech.Weapon = ReadFromEdit(IDC_WAFF_V);
                tech.Shield = ReadFromEdit(IDC_SCHILD_V);
                tech.Armour = ReadFromEdit(IDC_PANZ_V);
                sim.SetTechs(NULL, &tech, 0);
                SItem def[T_END];
                sim.GetFleetAfterSim(NULL, def, 0);
                sim.SetFleet(NULL, def, 0, T_END);
                UpdateEditFields(false, true);
                u_int abm = GetDlgItemInt(g_hwndDlg, ID_OUTPUT_ABM, NULL, FALSE);
                if(abm)
                    SetDlgItemInt(g_hwndDlg, ID_INPUT_ABM, abm, FALSE);
                else
                    SetDlgItemText(g_hwndDlg, ID_INPUT_ABM, _T(""));
                u_int ipm = GetDlgItemInt(g_hwndDlg, ID_OUTPUT_IPM, NULL, FALSE);
                if(ipm)
                    SetDlgItemInt(g_hwndDlg, ID_INPUT_IPM, ipm, FALSE);
                else
                    SetDlgItemText(g_hwndDlg, ID_INPUT_IPM, _T(""));
            }
            break;
        case IDM_ABOUT:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), g_hwndDlg, AboutProc);
            break;
        // Accelerator Messages
        case ID_SLOT1:
        case ID_SLOT2:
        case ID_SLOT3:
        case ID_SLOT4:
        case ID_SLOT5:
        case ID_ENTER:
        case ID_ESCAPE:
        case ID_NEWINSTANCE:
        case ID_SHOWOPTS:
        case ID_NEXTFLEET:
        case ID_PREVFLEET:
        case ID_SHOW_HIST:
        case ID_EDIT_ESP:
            OnAccelerator(lParam, wParam);
            break;
        default:
            return 0;
            break;
        }
		return 0;
        break;
    case WM_DRAWCLIPBOARD:
        ProcessClipboardChg();
        SendMessage(g_hwndNextCBWnd, uMsg, wParam, lParam);
        break;
    case WM_CHANGECBCHAIN:
        if((HWND)wParam == g_hwndNextCBWnd)
            g_hwndNextCBWnd = (HWND) lParam;
        else if (g_hwndNextCBWnd != NULL)
            SendMessage(g_hwndNextCBWnd, uMsg, wParam, lParam); 
        return 0;
        break;
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_MOUSEWHEEL:
        OnScroll(uMsg, lParam, wParam);
        break;
    case WM_CTLCOLORSTATIC:
        if(GetDlgItem(g_hwndDlg, IDC_BEUT_PRAK)==(HWND)lParam) {
            SetTextColor((HDC)wParam, RGB(0,110,0));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        if(GetDlgItem(g_hwndDlg, IDC_FLYCOST)==(HWND)lParam) {
            SetTextColor((HDC)wParam, RGB(0,0,255));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        if(GetDlgItem(g_hwndDlg, IDC_VERL_A)==(HWND)lParam || GetDlgItem(g_hwndDlg, IDC_VERL_V)==(HWND)lParam) {
            SetTextColor((HDC)wParam, RGB(255,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        return 0;
        break;
    case WM_CTLCOLORMSGBOX:
        SetTextColor((HDC)wParam, RGB(255,0,0));
        SetBkColor((HDC)wParam, (COLORREF)GetSysColor(COLOR_BTNFACE));
        return (LRESULT)GetStockObject(NULL_BRUSH);
        break;
    case WM_NOTIFY:
        return OnNotify(wParam, lParam);
        break;
    default:
        return 0;
        break;
    }
    return TRUE;
}

INT_PTR EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == WM_KEYDOWN || uMsg == WM_CHAR || uMsg == WM_KEYUP){
        if(IsCharNum((TCHAR)wParam) || (TCHAR)wParam == ':' || wParam == VK_BACK ){
            return CallWindowProc(g_oldEditProc, hwnd, uMsg, wParam, lParam);
        }
        else
            return 0;
    }
    return CallWindowProc(g_oldEditProc, hwnd, uMsg, wParam, lParam);
}

INT_PTR UpdateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        if(g_Options.bDoUpdate)
            SendDlgItemMessage(hwndDlg, IDC_DAYLY_UPD, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
        else
            SendDlgItemMessage(hwndDlg, IDC_DAYLY_UPD, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
        SetDlgItemText(hwndDlg, IDC_DAYLY_UPD, g_UpdaterStr[5].c_str());
        SetDlgItemText(hwndDlg, IDC_CLOSE, g_MiscStr[2].c_str());
        SetDlgItemText(hwndDlg, IDC_START_UPD, g_UpdaterStr[6].c_str());
        g_hwndUpd = hwndDlg;
        //SetFontForAllChilds(hwndDlg, g_Font);
        break;
    case WM_CLOSE:
        if(IsDlgButtonChecked(hwndDlg, IDC_DAYLY_UPD) == BST_CHECKED)
            g_Options.bDoUpdate = true;
        else
            g_Options.bDoUpdate = false;
        if(g_CheckThread) {
            g_CheckThread = 0;
        }
        EndDialog(hwndDlg, 0);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_CLOSE:
            if(IsDlgButtonChecked(hwndDlg, IDC_DAYLY_UPD) == BST_CHECKED)
                g_Options.bDoUpdate = true;
            else
                g_Options.bDoUpdate = false;
            if(g_CheckThread) {
                g_CheckThread = 0;
            }
            EndDialog(hwndDlg, 0);
            break;
        case IDC_START_UPD:
            EnableWindow(GetDlgItem(g_hwndUpd, IDC_START_UPD), false);
            g_CheckThread = _beginthread(TestNewerVersion, 0, NULL);
            break;
        case IDC_SPEED_HOME:
            {
            // start homepage
            TCHAR hp[128];
            _tcscpy(hp, _T("http://www.speedsim.net"));
            OpenURL(hp);
            }
            break;
        default:
            return 0;
            break;
        }
    default:
        return 0;
        break;
    }
    return 1;
}

INT_PTR SpioProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    
    TCHAR c[64];
    Res r;
    BattleResult b;
    PlaniPos p;
    TargetInfo ti;

    switch(uMsg) {
    case WM_SHOWWINDOW:
    case WM_INITDIALOG:
        //b = sim.GetBattleResult();
        ti = sim.GetTargetInfo(0);
        SetDlgItemInt(hwnd, IDC_SPIOMET, ti.Resources.met, false);
        SetDlgItemInt(hwnd, IDC_SPIOKRIS, ti.Resources.kris, false);
        SetDlgItemInt(hwnd, IDC_SPIODEUT, ti.Resources.deut, false);
        SetDlgItemText(hwnd, IDC_SPIOPOS, ti.Pos.ToString().c_str());
        if(ti.Pos.bMoon)
            SendDlgItemMessage(hwnd, IDC_IS_MOON, BM_SETCHECK, BST_CHECKED, NULL);
        SetDlgItemText(hwnd, IDC_SPIONAME, ti.Name);
        // translation
        SetDlgItemText(hwnd, IDC_DELWAVESTATE, g_ScanEditStr[0].c_str());
        SetDlgItemText(hwnd, IDC_POSITION, g_ScanEditStr[1].c_str());        
        SetDlgItemText(hwnd, IDCANCEL, g_MiscStr[3].c_str());
        SetDlgItemText(hwnd, IDC_METAL, g_ResNames[0].c_str());
        SetDlgItemText(hwnd, IDC_CRYSTAL, g_ResNames[1].c_str());
        SetDlgItemText(hwnd, IDC_DEUTERIUM, g_ResNames[2].c_str());
        SetDlgItemText(hwnd, IDC_RES, g_ScanEditStr[2].c_str());
        SetDlgItemText(hwnd, IDC_S_NAME, g_ScanEditStr[5].c_str());
        SetWindowText(hwnd, g_ScanEditStr[3].c_str());
        SetDlgItemText(hwnd, IDC_IS_MOON, g_PopupStrings[4].c_str());
        /*if(uMsg == WM_INITDIALOG)
            SetFontForAllChilds(hwnd, g_Font);*/
        SendDlgItemMessage(hwnd, IDC_SPIONAME, EM_SETLIMITTEXT, 63, 0);
        break;
    case WM_CLOSE:
            EndDialog(hwnd, 0);
            break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK:
            {
                UpdateKernel();
                ti = sim.GetTargetInfo(0);
                GetDlgItemText(hwnd, IDC_SPIOPOS, c, 63);
                p = PlaniPos(c);
                GetDlgItemText(hwnd, IDC_SPIONAME, c, 63);
                _tcsncpy(ti.Name, c, 63);
                r.met = GetDlgItemInt(hwnd, IDC_SPIOMET, NULL, false);
                r.kris = GetDlgItemInt(hwnd, IDC_SPIOKRIS, NULL, false);
                r.deut = GetDlgItemInt(hwnd, IDC_SPIODEUT, NULL, false);
                if(IsDlgButtonChecked(hwnd, IDC_IS_MOON)) {
                    p.bMoon = true;
                    SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_CHECKED, NULL);
                }
                else
                    SendDlgItemMessage(g_hwndDlg, IDC_IS_MOON, BM_SETCHECK, BST_UNCHECKED, NULL);
                ti.Pos = p;
                ti.Resources = r;
                sim.SetTargetInfo(ti, 0, true);
                UpdateEditFields(true, true);
            }
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;
        case IDC_DELWAVESTATE:
            sim.ResetWaveState();
            SendMessage(hwnd, WM_INITDIALOG, 0, 0);
            UpdateEditFields(false, true);
            break;
        default:
            return 0;
            break;
        }
    default:
        return 0;
        break;
    }
    return 1;
}

void CallBack(int sim, int round)
{
	TCHAR c[64];
    if(sim == 1 && round == 1) {
        SendMessageTimeout(GetDlgItem(g_hwndDlg, IDC_SIM), WM_SETTEXT, 0, (LPARAM)g_strSim[2].c_str(), SMTO_BLOCK, 1, NULL);
        EnableWindow(GetDlgItem(g_hwndDlg, IDC_SIM), true);
    }
	_stprintf(c, g_strSimState[1].c_str(), sim, round);
    
    SendMessageTimeout(GetDlgItem(g_hwndDlg, IDC_RUNNING_NUM), WM_SETTEXT, 0, (LPARAM)c, SMTO_BLOCK, 1, NULL);
}

INT_PTR OptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_INITDIALOG:
        g_hwndOpt = hwndDlg;
        CreateTabs();
        SetWindowText(hwndDlg, g_MiscStr[7].c_str());
        SetDlgItemText(hwndDlg, IDC_OPT_CANCEL, g_MiscStr[3].c_str());
        //SetFontForAllChilds(hwndDlg, g_Font);
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_OPT_OK:
            {
                // save all options
                TabInfo *pTInfo = (TabInfo*)GetWindowLongPtr(g_hwndOpt, GWLP_USERDATA);
                g_Options.iDefRebuild = GetDlgItemInt(pTInfo->hwndDisplay[1], IDC_DEF_REBUILD, NULL, false);
                g_Options.iSpeedFactor = GetDlgItemInt(pTInfo->hwndDisplay[1], IDC_SPEED_FAC_IN, NULL, false);
                sim.SetDefRebuildFactor(g_Options.iSpeedFactor / 100.f);
                g_Options.PlunderShip = (ITEM_TYPE)SendDlgItemMessage(pTInfo->hwndDisplay[1], IDC_SHIPTYPE, CB_GETCURSEL, 0, NULL);
                if(g_Options.PlunderShip >= T_SPIO)
                {
                    int tmp = (int)g_Options.PlunderShip;
                    g_Options.PlunderShip = (ITEM_TYPE)(tmp + 1);
                }
                if(g_Options.PlunderShip >= T_SAT)
                {
                    int tmp = (int)g_Options.PlunderShip;
                    g_Options.PlunderShip = (ITEM_TYPE)(tmp + 1);
                }
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[0], IDC_SYSTRAY) == BST_CHECKED)
                    g_Options.bTrayIcon = TRUE;
                else
                    g_Options.bTrayIcon = FALSE;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[0], IDC_TRAY_POPUP) == BST_CHECKED)
                    g_Options.bPopUpOnText = TRUE;
                else
                    g_Options.bPopUpOnText = FALSE;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[0], IDC_AUTOSTART) == BST_CHECKED)
                    SetWinStartUp(true);
                else
                    SetWinStartUp(false);
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[0], IDC_TRAY_ON_START) == BST_CHECKED)
                    g_Options.bMinimizeOnStart = true;
                else
                    g_Options.bMinimizeOnStart = false;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[0], IDC_SINGLE_INST) == BST_CHECKED)
                    g_Options.bSingleInst = TRUE;
                else
                    g_Options.bSingleInst = FALSE;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[1], IDC_REB_SMALLDEF) == BST_CHECKED)
                    g_Options.bRebuildSmallDef = TRUE;
                else
                    g_Options.bRebuildSmallDef = false;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[2], IDC_OWNRF) == BST_CHECKED)
                    GetDlgItemTextA(pTInfo->hwndDisplay[2], IDC_S_LOADEDRF, g_Options.RFFileName, 64);
                else
                    strcpy(g_Options.RFFileName, "");
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[2], IDC_OWNSHIPDATA) == BST_CHECKED)
                    GetDlgItemTextA(pTInfo->hwndDisplay[2], IDC_S_LOADEDSD, g_Options.ShipDataFile, 64);
                else
                    strcpy(g_Options.ShipDataFile, "");

                int iPos = SendDlgItemMessage(pTInfo->hwndDisplay[0], IDC_PERC_TRACKBAR, TBM_GETPOS, 0, NULL);
                g_Options.iPopUpTransparency = iPos * 2;
                iPos = SendDlgItemMessage(pTInfo->hwndDisplay[0], IDC_PERC_TRACKBAR2, TBM_GETPOS, 0, NULL);
                g_Options.iDlgTransparency = iPos * 2;
                if(iPos * 2)
                    TransparentWindow(g_hwndDlg, iPos * 2);
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[1], IDC_REB_AVG) == BST_CHECKED)
                    g_Options.RebuildOpt = REBUILD_AVG;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[1], IDC_REB_BEST) == BST_CHECKED)
                    g_Options.RebuildOpt = REBUILD_BESTCASE;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[1], IDC_REB_WORST) == BST_CHECKED)
                    g_Options.RebuildOpt = REBUILD_WORSTCASE;
                if(IsDlgButtonChecked(pTInfo->hwndDisplay[1], IDC_OLD_BS) == BST_CHECKED)
                    g_Options.bUseOldBS = true;
                else
                    g_Options.bUseOldBS = false;
                g_Options.StructureToDF = GetDlgItemInt(pTInfo->hwndDisplay[1], IDC_BIG_DF, NULL, FALSE);
                EndDialog(hwndDlg, 0);
            }
            break;
        case IDC_OPT_CANCEL:
            EndDialog(hwndDlg, 0);
            break;
        default:
            return 0;
            break;
        }
        break;
    case WM_NOTIFY:
        switch(wParam) {
        case IDC_OPT_TAB:
            if(((NMHDR*)lParam)->code == TCN_SELCHANGE){
                TabInfo *pTInfo = (TabInfo*)GetWindowLongPtr(g_hwndOpt, GWLP_USERDATA);
                // hide old dialog
                ShowWindow(pTInfo->hwndDisplay[pTInfo->lastSel], SW_HIDE);
                int sel = TabCtrl_GetCurSel(GetDlgItem(g_hwndOpt, IDC_OPT_TAB));
                // align
                RECT r, r2;
                GetWindowRect(GetDlgItem(g_hwndOpt, IDC_OPT_TAB), &r);
                GetWindowRect(g_hwndOpt, &r2);
                r2.bottom = r.bottom-r2.bottom;
                r2.left = r.left-r2.left;
                r2.right = r.right-r2.right;
                r2.top = r.top-r2.top+2;
                GetWindowRect(pTInfo->hwndDisplay[sel], &r);
                MoveWindow(pTInfo->hwndDisplay[sel], r2.left, r2.top, r.right-r.left, r.bottom-r.top, true);
                // show new
                ShowWindow(pTInfo->hwndDisplay[sel], SW_NORMAL);
                pTInfo->lastSel = sel;
            }
        	break;
        default:
            return 0;
            break;
        }
        break;
    case WM_DESTROY:
        {
            // free memory
            TabInfo *pTInfo = (TabInfo*)GetWindowLongPtr(g_hwndOpt, GWLP_USERDATA);
            for(int i = 0; i < C_PAGES; i++) {
                DestroyWindow(pTInfo->hwndDisplay[i]);
            }
            LocalFree((HLOCAL)pTInfo);
        }
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

INT_PTR TabPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_INITDIALOG:
        InitTabpages(hwndDlg);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDC_LOADSHIPDATA:
            {
            OPENFILENAME of;
			TCHAR filename[MAX_PATH];
            TCHAR sd_file[64];
			filename[0] = 0;
			of.lStructSize = sizeof(OPENFILENAME);
			of.hwndOwner = hwndDlg;
			of.hInstance = NULL;
			of.lpstrFilter = _T("ShipData (*.sd)\0*.sd\0All Files(*.*)\0*.*\0\0");
			of.lpstrCustomFilter = NULL;
			of.nMaxCustFilter = 0;
			of.nFilterIndex = 1;
			of.lpstrFile = filename;
			of.nMaxFile = MAX_PATH * sizeof(TCHAR);
			of.lpstrFileTitle = NULL;
			of.nMaxFileTitle = 0;
			of.lpstrInitialDir = g_CurrDir;
			of.lpstrTitle = NULL;
			of.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			of.lpstrDefExt = _T("sd");
			if(GetOpenFileName(&of))
			{
                _tcsncpy(sd_file, &filename[of.nFileOffset], 64);
                int l1 = (int)_tcslen(g_CurrDir), l2 = of.nFileOffset;
                int len = l1 > l2 ? l1 : l2;
                if(_tcsncicmp(g_CurrDir, filename, len)) {
                    _tcscpy(sd_file, _T(""));
                    MessageBox(hwndDlg, g_MiscStr[5].c_str(), _T("Error"), MB_OK|MB_ICONERROR);
                    CheckDlgButton(hwndDlg, IDC_OWNSHIPDATA, BST_UNCHECKED);
                    SetDlgItemText(hwndDlg, IDC_S_LOADEDSD, _T(""));
                    break;
                }
                SetDlgItemText(hwndDlg, IDC_S_LOADEDSD, sd_file);
            }
            else {
                CheckDlgButton(hwndDlg, IDC_OWNSHIPDATA, BST_UNCHECKED);
                _tcscpy(sd_file, _T(""));
                SetDlgItemText(hwndDlg, IDC_S_LOADEDSD, _T(""));
            }
            }
            break;
        case IDC_LOADRFDATA:
            {
            OPENFILENAME of;
			TCHAR filename[MAX_PATH];
            TCHAR rf_file[64];
			filename[0] = 0;
			of.lStructSize = sizeof(OPENFILENAME);
			of.hwndOwner = hwndDlg;
			of.hInstance = NULL;
			of.lpstrFilter = _T("RapidFire Files (*.rf)\0*.rf\0All Files(*.*)\0*.*\0\0");
			of.lpstrCustomFilter = NULL;
			of.nMaxCustFilter = 0;
			of.nFilterIndex = 1;
			of.lpstrFile = filename;
			of.nMaxFile = MAX_PATH * sizeof(TCHAR);
			of.lpstrFileTitle = NULL;
			of.nMaxFileTitle = 0;
			of.lpstrInitialDir = g_CurrDir;
			of.lpstrTitle = NULL;
			of.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			of.lpstrDefExt = _T("rf");
			if(GetOpenFileName(&of))
			{
                _tcsncpy(rf_file, &filename[of.nFileOffset], 64);
                int l1 = (int)_tcslen(g_CurrDir), l2 = of.nFileOffset;
                int len = l1 > l2 ? l1 : l2;
                if(_tcsncicmp(g_CurrDir, filename, len)) {
                    _tcscpy(rf_file, _T(""));
                    MessageBox(hwndDlg, g_MiscStr[5].c_str(), _T("Error"), MB_OK|MB_ICONERROR);
                    CheckDlgButton(hwndDlg, IDC_OWNRF, BST_UNCHECKED);
                    SetDlgItemText(hwndDlg, IDC_S_LOADEDRF, _T(""));
                    break;
                }
                SetDlgItemText(hwndDlg, IDC_S_LOADEDRF, rf_file);
            } 
            else {
                CheckDlgButton(hwndDlg, IDC_OWNRF, BST_UNCHECKED);
                _tcscpy(rf_file, _T(""));
                SetDlgItemText(hwndDlg, IDC_S_LOADEDRF, _T(""));
            }
            }
            break;
        case IDC_OWNRF:
            if(IsDlgButtonChecked(hwndDlg, IDC_OWNRF) == BST_CHECKED)
                SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LOADRFDATA, 0), 0);
            else
                SetDlgItemText(hwndDlg, IDC_S_LOADEDRF, _T(""));
            break;
        case IDC_OWNSHIPDATA:
            if(IsDlgButtonChecked(hwndDlg, IDC_OWNSHIPDATA) == BST_CHECKED)
                SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LOADSHIPDATA, 0), 0);
            else
                SetDlgItemText(hwndDlg, IDC_S_LOADEDSD, _T(""));
            break;
        default:
            return 0;
        }
        break;
    case WM_HSCROLL:
        {
            TCHAR szInfo[64];
            // change of trackbar
            // for popup window
            int iPos = SendDlgItemMessage(hwndDlg, IDC_PERC_TRACKBAR, TBM_GETPOS, 0, NULL);
            _stprintf(szInfo, _T("%d %%"), iPos * 2);
            SetDlgItemText(hwndDlg, IDC_PERC_OUT, szInfo);

            // for main dialog
            iPos = SendDlgItemMessage(hwndDlg, IDC_PERC_TRACKBAR2, TBM_GETPOS, 0, NULL);
            _stprintf(szInfo, _T("%d %%"), iPos * 2);
            SetDlgItemText(hwndDlg, IDC_PERC_OUT2, szInfo);
            TransparentWindow(g_hwndDlg, iPos * 2);
        }
        break;
    case WM_DESTROY:
        // restore transparency
        TransparentWindow(g_hwndDlg, g_Options.iDlgTransparency);
        if(g_TabBkBrush)
        {
            DeleteObject((HGDIOBJ)g_TabBkBrush);
            g_TabBkBrush = NULL;
        }
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

void CreateTabs() {
    TCITEM tie{};
    TabInfo *pTInfo = (TabInfo*)LocalAlloc(LPTR, sizeof(TabInfo));
    SetWindowLongPtr(g_hwndOpt, GWLP_USERDATA, (LONG_PTR)pTInfo);
    
    HWND hwndTab = GetDlgItem(g_hwndOpt, IDC_OPT_TAB);
    // add tabs
    tie.mask = TCIF_TEXT;
    tie.pszText = (TCHAR*)g_OptPage1[3].c_str();
    TabCtrl_InsertItem(hwndTab, 0, &tie);

    tie.pszText = (TCHAR*)g_OptPage2[0].c_str();
    TabCtrl_InsertItem(hwndTab, 1, &tie);

    tie.pszText = (TCHAR*)g_OptPage3[0].c_str();
    TabCtrl_InsertItem(hwndTab, 2, &tie);

    // create tab dialogs
    pTInfo->hwndDisplay[0] = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_PAGE1), g_hwndOpt, TabPageProc);
    pTInfo->hwndDisplay[1] = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_PAGE2), g_hwndOpt, TabPageProc);
    pTInfo->hwndDisplay[2] = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_OPT_PAGE3), g_hwndOpt, TabPageProc);

    // send message for changing tabs to paint the first
    NMHDR nmhdr{};
    nmhdr.code = TCN_SELCHANGE;
    nmhdr.hwndFrom = 0;
    nmhdr.idFrom = 0;
    SendMessage(g_hwndOpt, WM_NOTIFY, IDC_OPT_TAB, (LPARAM)&nmhdr);
}

INT_PTR AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT newFont;
    static bool bLinkClick = false;
    static bool bIRCClick = false;
    switch(uMsg) {
    case WM_INITDIALOG:
        {
        //SetFontForAllChilds(hwndDlg, g_Font);
        // add build date
        TCHAR buff[60];
        _stprintf(buff, _T("Build %s, %s"), __TDATE__, __TTIME__);
        SetDlgItemText(hwndDlg, IDC_BUILD, buff);
        // underline IRC and website
        newFont = CreateFont(_T("MS Shell Dlg"), 8, FF_UNDERLINE);
        SendMessage(GetDlgItem(hwndDlg, IDC_WEBSITE), WM_SETFONT, (WPARAM)newFont, 0);
        SendMessage(GetDlgItem(hwndDlg, IDC_IRC), WM_SETFONT, (WPARAM)newFont, 0);
        // if language file used, change to english website
        if(strlen(g_Options.LangFileName))
            SetDlgItemText(hwndDlg, IDC_WEBSITE, _T("http://www.speedsim.net"));
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK:
            DeleteObject(newFont);
            EndDialog(hwndDlg, 0);
            break;
        case IDC_SUPPORT:
            OpenURL(_T("http://sourceforge.net/donate/index.php?group_id=122497"));
            break;
        default:
            return 0;
        }
        break;
    case WM_CLOSE:
        DeleteObject(newFont);
        EndDialog(hwndDlg, 0);
        break;
    case WM_CTLCOLORSTATIC:
        if(GetDlgItem(hwndDlg, IDC_IRC) == (HWND)lParam || GetDlgItem(hwndDlg, IDC_WEBSITE) == (HWND)lParam) {
            if((!bIRCClick && GetDlgItem(hwndDlg, IDC_IRC) == (HWND)lParam) || 
                (!bLinkClick && GetDlgItem(hwndDlg, IDC_WEBSITE) == (HWND)lParam))
                SetTextColor((HDC)wParam, RGB(0,0,255));
            else if((bIRCClick && GetDlgItem(hwndDlg, IDC_IRC) == (HWND)lParam) ||
                (bLinkClick && GetDlgItem(hwndDlg, IDC_WEBSITE) == (HWND)lParam))
                SetTextColor((HDC)wParam, RGB(255,0,0));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        return FALSE;
        break;
    case WM_LBUTTONDOWN:
        {
            POINTS p2 = MAKEPOINTS(lParam);
            POINT p;
            p.x = p2.x;
            p.y = p2.y;
            if(ChildWindowFromPoint(hwndDlg, p) == GetDlgItem(hwndDlg, IDC_IRC)) {
                bIRCClick = true;
                InvalidateRect(GetDlgItem(hwndDlg, IDC_IRC), NULL, true);
            }
            else if(ChildWindowFromPoint(hwndDlg, p) == GetDlgItem(hwndDlg, IDC_WEBSITE)) {
                bLinkClick = true;
                InvalidateRect(GetDlgItem(hwndDlg, IDC_WEBSITE), NULL, true);
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            POINTS p2 = MAKEPOINTS(lParam);
            POINT p;
            p.x = p2.x;
            p.y = p2.y;
            if(ChildWindowFromPoint(hwndDlg, p) == GetDlgItem(hwndDlg, IDC_IRC) && bIRCClick) {
                ShellExecute(NULL, _T("open"), _T("irc://irc.ogamenet.net/speedsim"), NULL, NULL, SW_SHOWNORMAL);
            }
            else if(ChildWindowFromPoint(hwndDlg, p) == GetDlgItem(hwndDlg, IDC_WEBSITE) && bLinkClick) {
                TCHAR site[128];
                GetDlgItemText(hwndDlg, IDC_WEBSITE, site, 128);
                OpenURL(site);
            }
            bIRCClick = bLinkClick = false;
            InvalidateRect(GetDlgItem(hwndDlg, IDC_WEBSITE), NULL, true);
            InvalidateRect(GetDlgItem(hwndDlg, IDC_IRC), NULL, true);
        }
        break;
    case WM_MOUSEMOVE:
        {
            if(wParam == MK_LBUTTON)
            {
                POINTS p2 = MAKEPOINTS(lParam);
                POINT p;
                p.x = p2.x;
                p.y = p2.y;
                if(ChildWindowFromPoint(hwndDlg, p) == GetDlgItem(hwndDlg, IDC_IRC))
                {
                    bIRCClick = true;
                    bLinkClick = false;
                }
                else if(ChildWindowFromPoint(hwndDlg, p) == GetDlgItem(hwndDlg, IDC_WEBSITE))
                {
                    bLinkClick = true;
                    bIRCClick = false;
                }
                else
                    bIRCClick = bLinkClick = false;
                InvalidateRect(GetDlgItem(hwndDlg, IDC_WEBSITE), NULL, true);
                InvalidateRect(GetDlgItem(hwndDlg, IDC_IRC), NULL, true);
            }
        }
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

INT_PTR LangProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_INITDIALOG:
        {
            int i = 0;
            size_t j;
            map<genstring, genstring> files = UpdateLangFiles();
            map<genstring, genstring>::reverse_iterator it = files.rbegin();
            for(; it != files.rend(); it++)
            {
                TCHAR c[256];
                _stprintf(c, _T("%s (%s)"), it->first.c_str(), it->second.c_str());
                i = SendDlgItemMessage(hwndDlg, IDC_LANGFILES, LB_ADDSTRING, 0, (LPARAM)c);
                size_t p = 0;
                for(j = 0; j < g_LangFiles.size(); j++)
                {
                    if(g_LangFiles[j] == it->second)
                    {
                        p = j;
                        break;
                    }
                }
                SendDlgItemMessage(hwndDlg, IDC_LANGFILES, LB_SETITEMDATA,  (WPARAM)i, (LPARAM)p);
            }
            SendDlgItemMessage(hwndDlg, IDC_LANGFILES, LB_INSERTSTRING, 0, (LPARAM)GERMAN_LANG);
        }
        break;
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_SET_LANG || HIWORD(wParam) == LBN_DBLCLK) {
            int res = SendDlgItemMessage(hwndDlg, IDC_LANGFILES, LB_GETCURSEL, 0, 0);
            if(res > 0)
            {
                // when not german, load chosen language
                int p = SendDlgItemMessage(hwndDlg, IDC_LANGFILES, LB_GETITEMDATA, (WPARAM)res, 0);
#ifdef UNICODE
                wcstombs(g_Options.LangFileName, g_LangFiles[p].c_str(), g_LangFiles[p].length() + 1);
#else
                strcpy(g_Options.LangFileName, g_LangFiles[p].c_str());
#endif
                CheckDlgButton(g_hwndDlg, IDC_USE_LANG, BST_CHECKED);
                LoadLang(g_Options.LangFileName);
            }
            else {
                strcpy(g_Options.LangFileName, "");
                LoadLang(NULL);
                CheckDlgButton(g_hwndDlg, IDC_USE_LANG, BST_UNCHECKED);
            }
            // Update 'results' field
            SItem att[T_END], def[T_END];
            sim.GetFleetAfterSim(att, def, ReadFromEdit(IDC_FLEET_ID)-1);
            ShowFightResults(sim.GetBattleResult(), att, def, false);
            EndDialog(hwndDlg, 0);
        }
        if(LOWORD(wParam) == IDC_GETLANG)
        {
            TCHAR url[256];
            _tcscpy(url, _T("http://forum.speedsim.net/viewforum.php?f=5"));
            OpenURL(url);
        }
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        break;
    default:
        return 0;
    }
    return 1;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_CREATE:
        return 0;
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }
    return DefDlgProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR InputBoxProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam) {
    static genstring* out = NULL;
    
    switch(Msg) {
    case WM_INITDIALOG:
        out = (genstring*)lParam;
        SetWindowText(hDlg, out->c_str());
        SetDlgItemText(hDlg, IDC_TITLE, out->c_str());
        SetFocus(GetDlgItem(hDlg, IDC_EINGABE));
        return false;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case ID_OK:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            break;
        }
        return true;
        break;
        case WM_CLOSE:
            {
                TCHAR c[128];
                GetDlgItemText(hDlg, IDC_EINGABE, c, 128);
                *out = c;
            }
            EndDialog(hDlg, 0);
            return false;
            break;
        default:
            return false;
    }
    return true;
}

extern vector<sRaidListItem> g_vRaidList;

INT_PTR PopupInfoProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    static TCHAR text[512];
    static int iCurRL = -1;
    switch(Msg) {
    case WM_INITDIALOG:
        g_hwndPopup = hDlg;
        TransparentWindow(g_hwndPopup, g_Options.iPopUpTransparency);
        // load bitmap / font
        g_hPopupBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POPUPBG), IMAGE_BITMAP, 0, 0, 0);
#ifndef UNICODE
        g_hPopupFont = CreateFont(_T("MS Sans Serif"), 8, 0);
#else
        g_hPopupFont = CreateFont(_T("Microsoft Sans Serif"), 8, 0);
#endif
        //SendMessage(hDlg, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
        _tcsncpy(text, (TCHAR*)lParam, 512);
        break;
    case WM_DESTROY:
        // release bitmap and font
        DeleteObject(g_hPopupBMP);
        DeleteObject(g_hPopupFont);
        g_PopupShown = false;
        g_hwndPopup = NULL;
        break;
    case WM_RBUTTONUP:
        {
            iCurRL++;
            if(iCurRL < g_vRaidList.size()) {
                int iFleet = g_vRaidList[iCurRL].iUsedFleet;
                CreatePopupInfo(g_vRaidList[iCurRL].battle_res, g_vRaidList[iCurRL].vAttItems[iFleet], iCurRL + 1);
            }
            else {
                iCurRL = -1;
                ShowWindow(g_hwndDlg, SW_RESTORE);
                ForceSetForegroundWindow(g_hwndDlg);
            }
        }
    case WM_LBUTTONUP:
        EndDialog(hDlg, 0);
        break;
    case WM_ERASEBKGND:
    case WM_PAINT:
        return OnPaintPopup(Msg, lParam, wParam, text);
        break;
    case WM_QUIT:
        EndDialog(hDlg, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR BalanceProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg) 
    {
    case WM_INITDIALOG:
        {
            genstrstream out;
            BattleResult r = sim.GetBattleResult();
            
            TCHAR c[512];
            TCHAR d[3][64];
            TCHAR titel[128];
            out << g_BilanzStrings[1] << _T("\r\n");
            _stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(r.GewinnOhneTF.met, d[0]), AddPointsToNumber(r.GewinnOhneTF.kris, d[1]), AddPointsToNumber(r.GewinnOhneTF.deut, d[2]));
            out << _T("\t") << c << _T("\r\n");
            out << g_BilanzStrings[2].c_str() << _T("\r\n");
            _stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(r.GewinnMitHalfTF.met, d[0]), AddPointsToNumber(r.GewinnMitHalfTF.kris, d[1]), AddPointsToNumber(r.GewinnMitHalfTF.deut, d[2]));
            out << _T("\t") << c << _T("\r\n");
            out << g_BilanzStrings[3].c_str() << _T("\r\n");
            _stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(r.GewinnMitTF.met, d[0]), AddPointsToNumber(r.GewinnMitTF.kris, d[1]), AddPointsToNumber(r.GewinnMitTF.deut, d[2]));
            out << _T("\t") << c << _T("\r\n");
            out << g_BilanzStrings[4].c_str() << _T("\r\n");

            Res GesamtGewinn = r.GesTF + r.GesamtBeute - r.GesVerlust;
            GesamtGewinn.deut -= r.TotalFuel;
            _stprintf(c, g_ResString[5].c_str(), AddPointsToNumber(GesamtGewinn.met, d[0]), AddPointsToNumber(GesamtGewinn.kris, d[1]), AddPointsToNumber(GesamtGewinn.deut, d[2]));
            out << _T("\t") << c << _T("\r\n");
            SetDlgItemText(hDlg, IDC_BALANCE_OUT, out.str().c_str());
            
            _stprintf(titel, g_BilanzStrings[0].c_str(), _tcslen(r.PlaniName) ? r.PlaniName : g_ResString[10].c_str());
            SetWindowText(hDlg, titel);
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
        case IDC_BAL_OK:
            EndDialog(hDlg, 0);
            break;
        }
        break;
    case WM_QUIT:
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR ReportHistProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    static RECT org_rect;
    switch(Msg) 
    {
    case WM_INITDIALOG:
        {
            g_hwndEspHist = hDlg;
            GetWindowRect(hDlg, &org_rect);
            InitEspHistory();
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
        case IDC_DEL_CUR_REP:
        {
            HWND hwndLV = GetDlgItem(g_hwndEspHist, IDC_REPORT_LIST);
            int cnt = ListView_GetItemCount(hwndLV);
            int i;
            // remove items
            for(i = 0; i < cnt; i++)
            {
                if(ListView_GetItemState(hwndLV, i, LVIS_SELECTED))
                {
                    g_vRaidHistory.erase(g_vRaidHistory.begin() + cnt - i - 1);
                }
            }
            RefreshReportHistory();
            break;
        }
        case IDC_CLOSE_ESP_HIST:
            ShowWindow(hDlg, SW_HIDE);
            break;
        case IDC_SAVE_REP:
            if(IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED)
            {
                EnableWindow(GetDlgItem(g_hwndEspHist, IDC_NUM_REP), TRUE);
            }
            else
                EnableWindow(GetDlgItem(g_hwndEspHist, IDC_NUM_REP), FALSE);

            break;
        }
        break;
    case WM_NOTIFY:
    {
        switch(LOWORD(wParam))
        {
        case IDC_REPORT_LIST:
            if(((LPNMHDR)lParam)->code == NM_DBLCLK)
            {
                NMITEMACTIVATE *nma = (NMITEMACTIVATE*)lParam;
                if(!ListView_GetItemCount(nma->hdr.hwndFrom))
                    return TRUE;
                if(nma->iItem == -1)
                    return TRUE;
                // load selected report
                ClearResult();
                UpdateKernel();
                LVITEM lvi;
                lvi.mask = LVIF_PARAM;
                lvi.iItem = nma->iItem;
                lvi.iSubItem = 0;
                ListView_GetItem(nma->hdr.hwndFrom, &lvi);
                int id = ReadFromEdit(IDC_FLEET_ID) - 1;
                sim.SetTargetInfo(g_vRaidHistory[lvi.lParam].Target, id);
                // check option to remove techs
                if((IsDlgButtonChecked(g_hwndDlg, IDC_CHECKTECHS) == BST_CHECKED))
                {
                    // if esp. report and techs = 0 -> set own techs to 0
                    g_Options.bCheckTechs = true;
                    ShipTechs techs = g_vRaidHistory[lvi.lParam].Target.Techs;
                    if(techs.Weapon == 0 && techs.Shield == 0 && techs.Armour == 0)
                        sim.SetTechs(&techs, NULL, id);
                    else
                        LoadTechs(g_CurSaveData, true);
                }
                else
                    g_Options.bCheckTechs = false;
                
                UpdateEditFields(true, true);
            }
            break;
        default:
            return FALSE;
        }
        break;
    }
    case WM_SHOWWINDOW:
        {
        HMENU hFileMenu = GetSubMenu(GetMenu(g_hwndDlg), 0);
        if(wParam == TRUE)
            CheckMenuItem(hFileMenu, IDM_SHOW_ESPHIST, MF_BYCOMMAND|MF_CHECKED);
        else
            CheckMenuItem(hFileMenu, IDM_SHOW_ESPHIST, MF_BYCOMMAND|MF_UNCHECKED);
        }
        break;
    case WM_QUIT:
    case WM_CLOSE:
    case WM_DESTROY:
        {
        HMENU hFileMenu = GetSubMenu(GetMenu(g_hwndDlg), 0);
        CheckMenuItem(hFileMenu, IDM_SHOW_ESPHIST, MF_BYCOMMAND|MF_UNCHECKED);
        EndDialog(hDlg, 0);
        }
        break;
    case WM_SIZE:
        //if(wParam == SIZE_RESTORED)
        {
            RECT r;
            GetWindowRect(hDlg, &r);
            r.right = r.right - r.left - (org_rect.right - org_rect.left);
            r.bottom = r.bottom - r.top - (org_rect.bottom - org_rect.top);
            OnResizeHistWindow(hDlg, org_rect, r.right, r.bottom);
            InvalidateRect(GetDlgItem(hDlg, IDC_S_INFO_ESP), NULL, TRUE);
            InvalidateRect(GetDlgItem(hDlg, IDC_DISABLE_REP_HIST), NULL, TRUE);
            InvalidateRect(GetDlgItem(hDlg, IDC_NUM_REP), NULL, TRUE);
            InvalidateRect(GetDlgItem(hDlg, IDC_NUM_REP_T), NULL, TRUE);
            InvalidateRect(GetDlgItem(hDlg, IDC_SAVE_REP), NULL, TRUE);
            UpdateHistTooltips();
        }
        break;
    case WM_SIZING:
        {
            RECT *r;
            
            r = (RECT*)lParam;
            
            if(r->bottom - r->top < org_rect.bottom - org_rect.top)
            {
                if(wParam == WMSZ_TOP || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_TOPLEFT)
                    r->top = r->bottom - (org_rect.bottom - org_rect.top);
                else
                    r->bottom = r->top + org_rect.bottom - org_rect.top;
            }
            if(r->right - r->left < org_rect.right - org_rect.left)
            {
                if(wParam == WMSZ_LEFT || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_TOPLEFT)
                    r->left = r->right - (org_rect.right - org_rect.left);
                else
                    r->right = r->left + org_rect.right - org_rect.left;
            }
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

LRESULT CALLBACK ListViewProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if((Msg == WM_VSCROLL && LOWORD(wParam) == SB_ENDSCROLL))
    {
        UpdateHistTooltips();
    }
    if(Msg == WM_MOUSEWHEEL)
    {
        int v = CallWindowProc(OldLVProc, hDlg, Msg, wParam, lParam);
        UpdateHistTooltips();
        return v;
    }
    if(Msg == WM_CONTEXTMENU)
    { 
        int v = CallWindowProc(OldLVProc, hDlg, Msg, wParam, lParam);
        // display menu to check marked items
        POINT p;
        HMENU menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING, IDM_CHECKRAIDED, g_EspHistStrings[8].c_str());
        AppendMenu(menu, MF_STRING, IDM_UNCHECKRAIDED, g_EspHistStrings[9].c_str());
        GetCursorPos(&p);
        int r = TrackPopupMenu(menu, TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, hDlg, NULL);
        if(r == IDM_CHECKRAIDED || r == IDM_UNCHECKRAIDED)
        {
            HWND hwndLV = GetDlgItem(g_hwndEspHist, IDC_REPORT_LIST);
            int cnt = ListView_GetItemCount(hwndLV);
            int i;
            // remove items
            for(i = 0; i < cnt; i++)
            {
                if(ListView_GetItemState(hwndLV, i, LVIS_SELECTED))
                {
                    LVITEM lvi;
                    int pos = cnt - i - 1;
                    if(r == IDM_CHECKRAIDED)
                    {
                        g_vRaidHistory[pos].Checked = true;
                        lvi.pszText = _T("X");
                    }
                    else
                    {
                        g_vRaidHistory[pos].Checked = false;
                        lvi.pszText = _T("");
                    }
                
                    // add check mark
                    lvi.mask = LVIF_TEXT;
                    lvi.iItem = i;
                    
                    lvi.iSubItem = 2;
                    ListView_SetItem(GetDlgItem(g_hwndEspHist, IDC_REPORT_LIST), &lvi);
                }
            }
            RefreshReportHistory();
        }
        DestroyMenu(menu);
        return v;

    }
    return CallWindowProc(OldLVProc, hDlg, Msg, wParam, lParam);
}
