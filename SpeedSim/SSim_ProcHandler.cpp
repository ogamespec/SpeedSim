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

void InitMainDialog()
{
    int i;
    TCHAR c[256];
    bool bReportInCB = false;
    CheckResolution();
    SetDefCSSFiles();
    LoadLang(NULL);
    CreateIPMWindows();
//#ifdef UNICODE
    // set unicode font
    g_Font = CreateFont(_T("Microsoft Sans Serif"), 8, 0);
    //g_Font = CreateFont(_T("MS Shell Dlg"), 8, 0);
/*#else
    g_Font = CreateFont(_T("MS Shell Dlg"), 8, 0);
#endif*/
    SetFontForAllChilds(g_hwndDlg, g_Font);
    SendMessage(g_hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON)));
    SendMessage(g_hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON)));
    
    SendDlgItemMessage(g_hwndDlg, IDC_RAPID, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_CHECKTECHS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
    
    SendDlgItemMessage(g_hwndDlg, IDC_NUM_SIM, WM_SETTEXT, 0, (LPARAM)_T("100"));
    SendDlgItemMessage(g_hwndDlg, IDC_BW_CASE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
    for(i = 0; i < T_END; i++)
    {
        if(i < T_SHIPEND && i != T_SAT)
            SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, true), EM_SETLIMITTEXT, 7, 0);
        SendDlgItemMessage(g_hwndDlg, GetEditIdByType((ITEM_TYPE)i, false), EM_SETLIMITTEXT, 7, 0);
    }
    SendDlgItemMessage(g_hwndDlg, IDC_GEN_READ, EM_SETLIMITTEXT, 0, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_WAFF_A, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_WAFF_V, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_A, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_SCHILD_V, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_PANZ_A, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_PANZ_V, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_NUM_SIM, EM_SETLIMITTEXT, 4, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_LIMITTEXT, 8, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_VERBRENNUNG, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_IMPULS, EM_SETLIMITTEXT, 2, 0);
    SendDlgItemMessage(g_hwndDlg, IDC_HYPERRAUM, EM_SETLIMITTEXT, 2, 0);
    for(i = 10; i > 0; i--) {
        TCHAR c[5];
        _stprintf(c, _T("%d%%"), i * 10);
        SendDlgItemMessage(g_hwndDlg, IDC_SPEED, CB_ADDSTRING, 0, (LPARAM)c);
    }
    SendDlgItemMessage(g_hwndDlg, IDC_SPEED, CB_SETCURSEL, 0, 0);
    g_GerStringsMain = GetGerLang(g_hwndDlg);
    _tcscpy(g_Options.SkinFileName, _T(""));
    SetDlgItemText(g_hwndDlg, IDC_FLEET_ID, _T("1"));
    LoadOpts(g_CurSaveData);
    // check whether anything has been read in
    ShipTechs t;
    sim.GetTechs(&t, NULL, 0);
    Res a, d;
    sim.GetFleetWorth(a, d);
    int worth = a.deut + a.kris + a.met + d.deut + d.kris + d.met;
    if((t.Armour == 0 && t.Shield == 0 && t.Weapon == 0 && worth == 0) 
        || (IsDlgButtonChecked(g_hwndDlg, IDC_CHECKTECHS) == BST_UNCHECKED)) {
        LoadTechs(g_CurSaveData, true);
    }
    else {
        bReportInCB = true;
        LoadTechs(g_CurSaveData, true, true);
    }
    g_oldEditProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(g_hwndDlg, IDC_OWN_POS), GWLP_WNDPROC, (LONG_PTR)EditProc);
    {
        TCHAR tmp[128];
        sim.GetVersion(tmp);
        _stprintf(c, _T("SpeedSim - GUI v%s | Kernel v%s  ||  by RainOfPain & TheButcher"), SPEEDSIM_VERSION, tmp);
#ifdef _DEBUG
        _stprintf(c, _T("%s - DEBUG Version -"), c);
#endif
#ifdef UNICODE
        _stprintf(c, _T("%s - Unicode Version"), c);
#endif
        SendMessage(g_hwndDlg, WM_SETTEXT, 0, (LPARAM)c);
        
    }
    GetClientRect(g_hwndDlg, &g_ClntRect);
    GetWindowRect(g_hwndDlg, &g_WndRect);

#ifndef _DEBUG
    if(g_Options.bDoUpdate) {
        g_UpdThread = _beginthread(AutoUpdate, 0, NULL);
    }
#endif
    SetSkin(g_Options.SkinFileName);
    CreateTooltips();
    //CreateStatusBar();
    g_Options.SelPos = -1;
    g_GeradeEingefuegt = -1;
    
    SItem att[T_END], def[T_END];
    sim.GetFleetAfterSim(att, def, ReadFromEdit(IDC_FLEET_ID)-1);
    ShowFightResults(sim.GetBattleResult(), att, def, false);
    UpdateRaidlistInfo();
    
    // first start?
    TCHAR iniFile[MAX_PATH];
    genstr nfile = GetSaveFolder() + SPEEDSIM_SAVEFILE;
    _tcsncpy(iniFile, nfile.c_str(), MAX_PATH);
    int NumSaveData = GetPrivateProfileInt(_T("General"), _T("NumOpts"), 0, iniFile);
    if(NumSaveData == 0)
        PostMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(ID_DATEI_EINSTELLUNGENSPEICHERN_NEU, 0), 0);

    InitMenu();
    if(g_Options.bMinimizeOnStart && !bReportInCB) {
        if(g_Options.bTrayIcon)
            CreateSystrayIcon(true);
        else
            ShowWindow(g_hwndDlg, SW_MINIMIZE);
    }
    else
        ShowWindow(g_hwndDlg, SW_SHOW);
    CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_ESP_REPORTS), g_hwndDlg, ReportHistProc);
    RefreshReportHistory();
    if(g_Options.bShowEspHist && !(g_Options.bMinimizeOnStart && !bReportInCB))
        ShowWindow(g_hwndEspHist, SW_SHOW);

    InvalidateRect(g_hwndDlg, NULL, FALSE);
}

void InitTabpages(HWND hwndDlg)
{
    int i;

    // enable correct XP tab page texture
    if(fnETDT)
    {
        fnETDT(hwndDlg, ETDT_ENABLETAB);
	}

    SetDlgItemInt(hwndDlg, IDC_DEF_REBUILD, g_Options.iDefRebuild, false);
    SendDlgItemMessage(hwndDlg, IDC_SPIN_SP_FAC, UDM_SETRANGE, 0, MAKELONG(25, 1));
    SetDlgItemText(hwndDlg, IDC_S_SPEED_FACTOR, g_OptPage1[2].c_str());
    SetDlgItemInt(hwndDlg, IDC_SPEED_FAC_IN, g_Options.iSpeedFactor, FALSE);
    
    if(g_Options.bTrayIcon)
        CheckDlgButton(hwndDlg, IDC_SYSTRAY, BST_CHECKED);
    else
        CheckDlgButton(hwndDlg, IDC_SYSTRAY, BST_UNCHECKED);
    if(!g_Options.HookClipboard)
        EnableWindow(GetDlgItem(hwndDlg, IDC_TRAY_POPUP), FALSE);
    if(g_Options.bPopUpOnText)
        CheckDlgButton(hwndDlg, IDC_TRAY_POPUP, BST_CHECKED);
    else
        CheckDlgButton(hwndDlg, IDC_TRAY_POPUP, BST_UNCHECKED);
    
    SetDlgItemText(hwndDlg, IDC_S_DEF_REBUILD, g_OptPage1[0].c_str());
    //
    if(g_OptPage1[4].length())
        SetDlgItemText(hwndDlg, IDC_SYSTRAY, g_OptPage1[4].c_str());
    if(g_OptPage1[5].length())
        SetDlgItemText(hwndDlg, IDC_TRAY_POPUP, g_OptPage1[5].c_str());
    if(g_OptPage1[6].length())
        SetDlgItemText(hwndDlg, IDC_S_TRAY_POPUP, g_OptPage1[6].c_str());
    if(g_OptPage1[7].length())
        SetDlgItemText(hwndDlg, IDC_REB_SMALLDEF, g_OptPage1[7].c_str());
    if(g_OptPage1[8].length())
        SetDlgItemText(hwndDlg, IDC_PERC_INFO, g_OptPage1[8].c_str());
    if(g_OptPage1[9].length())
        SetDlgItemText(hwndDlg, IDC_PERC_INFO2, g_OptPage1[9].c_str());
    if(g_OptPage1[10].length())
        SetDlgItemText(hwndDlg, IDC_OPT_TRANS, g_OptPage1[10].c_str());
    
    if(g_OptPage2[1].length())
        SetDlgItemText(hwndDlg, IDC_AUTOSTART, g_OptPage2[1].c_str());
    if(IsInStartup())
        CheckDlgButton(hwndDlg, IDC_AUTOSTART, BST_CHECKED);
    else
        CheckDlgButton(hwndDlg, IDC_AUTOSTART, BST_UNCHECKED);
    
    if(g_OptPage2[2].length())
        SetDlgItemText(hwndDlg, IDC_TRAY_ON_START, g_OptPage2[2].c_str());
    if(g_Options.bMinimizeOnStart)
        CheckDlgButton(hwndDlg, IDC_TRAY_ON_START, BST_CHECKED);
    else
        CheckDlgButton(hwndDlg, IDC_TRAY_ON_START, BST_UNCHECKED);
    
    if(g_OptPage2[3].length())
        SetDlgItemText(hwndDlg, IDC_SINGLE_INST, g_OptPage2[3].c_str());
    if(g_OptPage2[4].length())
        SetDlgItemText(hwndDlg, IDC_S_PLUNDERTYPE, g_OptPage2[4].c_str());
    
    
    if(g_OptPage3[1].length())
        SetDlgItemText(hwndDlg, IDC_OWNRF, g_OptPage3[1].c_str());
    if(g_OptPage3[2].length())
        SetDlgItemText(hwndDlg, IDC_OWNSHIPDATA, g_OptPage3[2].c_str());
    if(g_OptPage3[3].length())
        SetDlgItemText(hwndDlg, IDC_S_LOADED_RF, g_OptPage3[3].c_str());
    if(g_OptPage3[4].length())
        SetDlgItemText(hwndDlg, IDC_S_LOADED_SD, g_OptPage3[4].c_str());

    if(g_Options.bSingleInst)
        CheckDlgButton(hwndDlg, IDC_SINGLE_INST, BST_CHECKED);
    else
        CheckDlgButton(hwndDlg, IDC_SINGLE_INST, BST_UNCHECKED);

    if(g_OptPage2[11].length())
        SetDlgItemText(hwndDlg, IDC_BIG_DF_S, g_OptPage2[11].c_str());
    SetDlgItemInt(hwndDlg, IDC_BIG_DF, g_Options.StructureToDF, FALSE);
    
    // own rf / sd
    if(strlen(g_Options.RFFileName)) {
        CheckDlgButton(hwndDlg, IDC_OWNRF, BST_CHECKED);
    }
    if(strlen(g_Options.ShipDataFile))
        CheckDlgButton(hwndDlg, IDC_OWNSHIPDATA, BST_CHECKED);
    
    SetDlgItemTextA(hwndDlg, IDC_S_LOADEDRF, g_Options.RFFileName);
    SetDlgItemTextA(hwndDlg, IDC_S_LOADEDSD, g_Options.ShipDataFile);

    if(g_Options.bRebuildSmallDef)
        CheckDlgButton(hwndDlg, IDC_REB_SMALLDEF, BST_CHECKED);
    //SetFontForAllChilds(hwndDlg, g_Font);
    // add ships for plunder to drop down list
    for (i = 0; i < T_SHIPEND; i++) {
        TCHAR szShip[256];
        sim.ItemName((ITEM_TYPE)i, szShip);
        if(i == T_SPIO || i == T_SAT)
            //_tcscpy(szShip, _T(" --- "));
            continue;
        SendDlgItemMessage(hwndDlg, IDC_SHIPTYPE, CB_ADDSTRING, 0, (LPARAM)szShip);
    }

    int tmp = (int)g_Options.PlunderShip;
    if(g_Options.PlunderShip >= T_SPIO)
        tmp--;
    if(g_Options.PlunderShip >= T_SAT)
        tmp--;
    SendDlgItemMessage(hwndDlg, IDC_SHIPTYPE, CB_SETCURSEL, (WPARAM)tmp, NULL);
    

    // set option for waves
    switch(g_Options.RebuildOpt)
    {
    case REBUILD_AVG:
        CheckDlgButton(hwndDlg, IDC_REB_AVG, BST_CHECKED);
        break;
    case REBUILD_WORSTCASE:
        CheckDlgButton(hwndDlg, IDC_REB_WORST, BST_CHECKED);
        break;
    case REBUILD_BESTCASE:
        CheckDlgButton(hwndDlg, IDC_REB_BEST, BST_CHECKED);
        break;
    }
    SetDlgItemText(hwndDlg, IDC_S_NEXTWAVE, g_OptPage2[5].c_str());
    SetDlgItemText(hwndDlg, IDC_S_REBUILD, g_OptPage2[6].c_str());
    SetDlgItemText(hwndDlg, IDC_REB_AVG, g_OptPage2[7].c_str());
    SetDlgItemText(hwndDlg, IDC_REB_WORST, g_OptPage2[8].c_str());
    SetDlgItemText(hwndDlg, IDC_REB_BEST, g_OptPage2[9].c_str());
    SetDlgItemText(hwndDlg, IDC_OLD_BS, g_OptPage2[10].c_str());
    if(g_Options.bUseOldBS)
        CheckDlgButton(hwndDlg, IDC_OLD_BS, BST_CHECKED);


    // init Trackbar
    HWND hTB[2];
    hTB[0] = GetDlgItem(hwndDlg, IDC_PERC_TRACKBAR);
    hTB[1] = GetDlgItem(hwndDlg, IDC_PERC_TRACKBAR2);
    if(hTB[0]) {
        if(!TransparentWindow(GetParent(hwndDlg), 0)) {
            EnableWindow(hTB[0], false);
            EnableWindow(GetDlgItem(hwndDlg, IDC_PERC_INFO), false);
            EnableWindow(hTB[1], false);
            EnableWindow(GetDlgItem(hwndDlg, IDC_PERC_INFO2), false);
        }
        SendMessage(hTB[0], TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(0, 50));
        SendMessage(hTB[0], TBM_SETPOS, (WPARAM)TRUE, g_Options.iPopUpTransparency / 2);
        SendMessage(hTB[1], TBM_SETRANGE, (WPARAM)TRUE, MAKELONG(0, 50));
        SendMessage(hTB[1], TBM_SETPOS, (WPARAM)TRUE, g_Options.iDlgTransparency / 2);
        TCHAR szInfo[64];
        _stprintf(szInfo, _T("%d %%"), g_Options.iPopUpTransparency);
        SetDlgItemText(hwndDlg, IDC_PERC_OUT, szInfo);
        // transparency for mail dlg window
        _stprintf(szInfo, _T("%d %%"), g_Options.iDlgTransparency);
        SetDlgItemText(hwndDlg, IDC_PERC_OUT2, szInfo);
    }
}

void OnScroll(UINT uMsg, LPARAM lParam, WPARAM wParam)
{
    int min, max;
    switch(uMsg) {
    case WM_HSCROLL:
        GetScrollRange(g_hwndDlg, SB_HORZ, &min, &max);
        switch(LOWORD(wParam)) {
        case SB_LINELEFT:
            if(!(GetScrollPos(g_hwndDlg, SB_HORZ)-10 < min)) {
                ScrollWindow(g_hwndDlg, 10, 0, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_HORZ, GetScrollPos(g_hwndDlg, SB_HORZ)-10, true);
            }
            else{
                ScrollWindow(g_hwndDlg, GetScrollPos(g_hwndDlg, SB_HORZ), 0, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_HORZ, min, true);
            }
            break;
        case SB_LINERIGHT:
            if(!(GetScrollPos(g_hwndDlg, SB_HORZ)+10 > max)) {
                ScrollWindow(g_hwndDlg, -10, 0, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_HORZ, GetScrollPos(g_hwndDlg, SB_HORZ)+10, true);
            }
            else {
                ScrollWindow(g_hwndDlg, GetScrollPos(g_hwndDlg, SB_HORZ)-max, 0, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_HORZ, max, true);
            }
            break;
        case SB_THUMBPOSITION:
            ScrollWindow(g_hwndDlg, GetScrollPos(g_hwndDlg, SB_HORZ)-HIWORD(wParam), 0,  NULL, NULL);
            SetScrollPos(g_hwndDlg, SB_HORZ, HIWORD(wParam), true);
            break;
        }
        break;
    case WM_VSCROLL:
        GetScrollRange(g_hwndDlg, SB_VERT, &min, &max);
        switch(LOWORD(wParam)) {
        case SB_LINEDOWN:
            if(!(GetScrollPos(g_hwndDlg, SB_VERT)+10 > max)){
                ScrollWindow(g_hwndDlg, 0, -10, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_VERT, GetScrollPos(g_hwndDlg, SB_VERT)+10, true);
            }
            else{
                ScrollWindow(g_hwndDlg, 0, GetScrollPos(g_hwndDlg, SB_VERT)-max, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_VERT, max, true);
            }
            break;
        case SB_LINEUP:
            if(!(GetScrollPos(g_hwndDlg, SB_VERT)-10 < min)){
                ScrollWindow(g_hwndDlg, 0, 10, NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_VERT, GetScrollPos(g_hwndDlg, SB_VERT)-10, true);
            }
            else {
                ScrollWindow(g_hwndDlg, 0, GetScrollPos(g_hwndDlg, SB_VERT), NULL, NULL);
                SetScrollPos(g_hwndDlg, SB_VERT, min, true);
            }
            break;
        case SB_THUMBPOSITION:
            ScrollWindow(g_hwndDlg, 0, GetScrollPos(g_hwndDlg, SB_VERT)-HIWORD(wParam), NULL, NULL );
            SetScrollPos(g_hwndDlg, SB_VERT, HIWORD(wParam), true);
            break;
        }
        break;
    case WM_MOUSEWHEEL:
        // mousewheel moved
        if((short)HIWORD(wParam) > 0)
            SendMessage(g_hwndDlg, WM_VSCROLL, SB_LINEUP, 0);
        else
            SendMessage(g_hwndDlg, WM_VSCROLL, SB_LINEDOWN, 0);
        break;
    default:
        return;
        break;
    }
    UpdateWindow(g_hwndDlg);
}

int OnTrayMessage(UINT uMsg, LPARAM lParam, WPARAM wParam)
{
    switch(lParam) {
    case WM_LBUTTONDBLCLK:
        // destroy icon and show up speedsim again
        CreateSystrayIcon(false);
        ShowWindow(g_hwndDlg, SW_RESTORE);
        ForceSetForegroundWindow(g_hwndDlg);
        InvalidateRect(g_hwndDlg, NULL, true);
        DrawMenuBar(g_hwndDlg);
        break;
    case WM_RBUTTONUP:
        {
            POINT p;
            HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_TRAY_MENU));
            HMENU hTrayMenu = GetSubMenu(hMenu, 0);
            if(g_TrayText[4].length())
                ModifyMenu(hTrayMenu, ID__WIEDERHERSTELLEN, MF_BYCOMMAND|MF_STRING, ID__WIEDERHERSTELLEN, g_TrayText[4].c_str());
            if(g_TrayText[5].length()) {
                ModifyMenu(hTrayMenu, ID__SPEEDSIMBEENDEN, MF_BYCOMMAND|MF_STRING, ID__SPEEDSIMBEENDEN, g_TrayText[5].c_str());
                if(g_SimCount > 0)
                    ModifyMenu(hTrayMenu, ID__SPEEDSIMBEENDEN, MF_BYCOMMAND|MF_GRAYED, ID__SPEEDSIMBEENDEN, NULL);
            }
            if(g_TrayText[6].length())
                ModifyMenu(hTrayMenu, ID_TRAY_INFO, MF_BYCOMMAND|MF_STRING, ID_TRAY_INFO, g_TrayText[6].c_str());
            GetCursorPos(&p);
            // show menu
            SetForegroundWindow(g_hwndDlg);
            int res = TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON|TPM_RETURNCMD, p.x, p.y, 0, g_hwndDlg, NULL);
            PostMessage(g_hwndDlg, WM_NULL, 0, 0);
            DestroyMenu(hMenu);
            if(res == ID__WIEDERHERSTELLEN) {
                //CreateSystrayIcon(false);
                ShowWindow(g_hwndDlg, SW_RESTORE);
                ForceSetForegroundWindow(g_hwndDlg);
            }
            else if(res == ID__SPEEDSIMBEENDEN) {
                CreateSystrayIcon(false);
                SaveOpts(g_CurSaveData);
                PostQuitMessage(0);
            }
            else if(res == ID_TRAY_INFO) {
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), g_hwndDlg, AboutProc);
            }
        }
        break;
    default:
        return 0;
        break;
    }
    return TRUE;
}

bool IsMenuMsg(UINT uMsg, LPARAM lParam, WPARAM wParam)
{
    if(LOWORD(wParam) > IDM_FIRST_ITEM && LOWORD(wParam) <= IDM_LAST_ITEM) {
        // if own menu items
        if(LOWORD(wParam) > IDM_FIRST_OPTSAVE && LOWORD(wParam) < IDM_FIRST_OPTSAVE+50) {
            // save options/techs
            int tmp = g_CurSaveData;
            HMENU hMenu = GetMenu(g_hwndDlg);
            hMenu = GetSubMenu(hMenu, 0);
            hMenu = GetSubMenu(hMenu, 0);
            // uncheck old
            CheckMenuItem(hMenu, tmp, MF_BYPOSITION|MF_UNCHECKED);
            // check new
            CheckMenuItem(hMenu, LOWORD(wParam), MF_BYCOMMAND|MF_CHECKED);
            DrawMenuBar(g_hwndDlg);
            
            SaveOpts(LOWORD(wParam)-IDM_FIRST_OPTSAVE);
            SaveTechs(LOWORD(wParam)-IDM_FIRST_OPTSAVE);
            g_CurSaveData = LOWORD(wParam)-IDM_FIRST_OPTSAVE;
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_OPTLOAD && LOWORD(wParam) < IDM_FIRST_OPTLOAD+50) {
            // save old options
            SaveOpts(g_CurSaveData);
            // load options
            int tmp = g_CurSaveData;
            if(!LoadOpts(LOWORD(wParam)-IDM_FIRST_OPTLOAD))
                return true;
            LoadTechs(LOWORD(wParam)-IDM_FIRST_OPTLOAD, true);
            SetSkin(g_Options.SkinFileName);
            // set check in save menu
            HMENU hMenu = GetMenu(g_hwndDlg);
            hMenu = GetSubMenu(hMenu, 0);
            hMenu = GetSubMenu(hMenu, 0);
            // uncheck old item
            CheckMenuItem(hMenu, tmp, MF_BYPOSITION|MF_UNCHECKED);
            // check new
            CheckMenuItem(hMenu, g_CurSaveData, MF_BYPOSITION|MF_CHECKED);
            DrawMenuBar(g_hwndDlg);
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_OPTDELETE && LOWORD(wParam) < IDM_FIRST_OPTDELETE+50) {
            DeleteSaveData(SPEEDSIM_SAVEFILE, LOWORD(wParam)-IDM_FIRST_OPTDELETE);
            InitMenu();
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_ATT_FLEETLOAD && LOWORD(wParam) < IDM_FIRST_ATT_FLEETLOAD+50) {
            // load attacking fleet
            LoadFleet(LOWORD(wParam)-IDM_FIRST_ATT_FLEETLOAD, true);
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_DEF_FLEETLOAD && LOWORD(wParam) < IDM_FIRST_DEF_FLEETLOAD+50) {
            // load defending fleet
            LoadFleet(LOWORD(wParam)-IDM_FIRST_DEF_FLEETLOAD, false);
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_ATT_FLEETSAVE && LOWORD(wParam) < IDM_FIRST_ATT_FLEETSAVE+50) {
            // save attacking fleet
            SaveFleet(LOWORD(wParam) - IDM_FIRST_ATT_FLEETSAVE, true);
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_DEF_FLEETSAVE && LOWORD(wParam) < IDM_FIRST_DEF_FLEETSAVE+50) {
            // save defending fleet
            SaveFleet(LOWORD(wParam) - IDM_FIRST_DEF_FLEETSAVE, false);
            return true;
        }
        if(LOWORD(wParam) > IDM_FIRST_FLEETDELETE && LOWORD(wParam) < IDM_FIRST_FLEETDELETE+50) {
            DeleteFleetData(SPEEDSIM_SAVEFILE, LOWORD(wParam)-IDM_FIRST_FLEETDELETE);
            InitMenu();
            return true;
        }
        if(LOWORD(wParam) >= IDM_FIRST_LANG && LOWORD(wParam) < IDM_FIRST_LANG+50) {
            int pos = LOWORD(wParam) - IDM_FIRST_LANG;
            if(pos < 1) {
                // german language
                if(strlen(g_Options.LangFileName))
                {
                    strcpy(g_Options.LangFileName, "");
                    LoadLang(NULL);
                }
                return true;
            }
            ZeroMemory(g_Options.LangFileName, 64);
#ifdef UNICODE
            wcstombs(g_Options.LangFileName, g_LangFiles[pos - 1].c_str(), g_LangFiles[pos - 1].length()+1);
#else
            strncpy(g_Options.LangFileName, g_LangFiles[pos - 1].c_str(), 63);
#endif
            LoadLang(g_Options.LangFileName);
            return true;
        }
    }
    return false;
}

void SwapFleets()
{
    // switch fleets
    int i, id;
    UpdateKernel();
    ResetAtt(); ResetDef();
    vector<SItem> att_v, def_v;
    SItem att[T_END], def[T_END], tmp;
    ShipTechs techs[2];
    // dummy unit
    id = ReadFromEdit(IDC_FLEET_ID) - 1;
    tmp.OwnerID = id;
    tmp.Type = T_NONE;
    def_v.push_back(tmp);
    att_v.push_back(tmp);
    sim.GetSetFleet(att, def, id);
    sim.GetTechs(&techs[0], &techs[1], id);
    for(i = 0; i < T_END; i++) {
		if(def[i].Type < T_SHIPEND)
			att_v.push_back(def[i]);
        def_v.push_back(att[i]);
    }
    if(!IsDlgButtonChecked(g_hwndDlg, IDC_DEL_DEF_ON_SWITCH)) {
		for(i = 0; i < T_END; i++) {
			if(def[i].Type >= T_RAK)
				def_v.push_back(def[i]);
		}
    }
    sim.SetFleet(&att_v, &def_v);
    sim.SetTechs(&techs[1], &techs[0], id);
    UpdateEditFields(true, true);
}

void OnOwnPos(LPARAM lParam, WPARAM wParam)
{
    if(HIWORD(wParam) == CBN_KILLFOCUS)
    {
        TCHAR c[128];
        GetDlgItemText(g_hwndDlg, IDC_OWN_POS, c, 128);
        PlaniPos p(c);
        if(_tcslen(c) > 0 && SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_GETCOUNT, 0, 0) < MAX_POS)
        {
            // already exists?
            if(SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_FINDSTRINGEXACT, 0, (LPARAM)p.ToString().c_str()) == CB_ERR)
            {
                // doesn't exist -> add
                int nr = SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_ADDSTRING, 0, (LPARAM)p.ToString().c_str());
                SetDlgItemText(g_hwndDlg, IDC_OWN_POS, p.ToString().c_str());
                g_GeradeEingefuegt = nr;
            }
        }
        else
        {
            // delete current element
            SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_DELETESTRING, g_Options.SelPos, 0);
            SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_SETCURSEL, 0, 0);
        }
    }
    if(HIWORD(wParam) == CBN_SELCHANGE)
    {
        int n = SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_GETCURSEL, 0, 0);
        g_Options.SelPos = n;
        LPARAM sel = MAKELPARAM(-1, 0);
        n = SendDlgItemMessage(g_hwndDlg, IDC_OWN_POS, CB_SETEDITSEL, 0, sel);
    }
}

void OnScanMessage()
{
    if(OpenClipboard(g_hwndDlg)) {
        TCHAR* buff = (TCHAR*)GetClipboardData(CF_TEXT);
        CloseClipboard();
        if(buff) {
            ResetDef();
            UpdateKernel();
            int id = ReadFromEdit(IDC_FLEET_ID)-1;
            genstring Spio = buff;
            
            sim.ReadEspReport(Spio, id);
            if(IsDlgButtonChecked(g_hwndDlg, IDC_CHECKTECHS) == BST_CHECKED) {
                g_Options.bCheckTechs = true;
                ShipTechs techs;
                sim.GetTechs(NULL, &techs, id);
                if(techs.Armour == 0 && techs.Shield == 0 && techs.Weapon == 0 )
                    sim.SetTechs(&techs, NULL, id);
                else
                    LoadTechs(g_CurSaveData, true);
            }
            else
                g_Options.bCheckTechs = false;
            UpdateEditFields(true, true);
            if(g_Options.bTrayIcon) {
                CreateSystrayIcon(false);
                ShowWindow(g_hwndDlg, SW_RESTORE);
            }
            ForceSetForegroundWindow(g_hwndDlg);
        }
    }
}

int OnSize(LPARAM lParam, WPARAM wParam)
{
    static bool was_vis = false;
    // when minizing, hide the window
    switch(wParam) {
    case SIZE_MINIMIZED:
        if(g_Options.bTrayIcon) {
            CreateSystrayIcon(true);
            ShowWindow(g_hwndDlg, SW_HIDE);
        }
        else
            ShowWindow(g_hwndDlg, SW_MINIMIZE);
        was_vis = IsWindowVisible(g_hwndEspHist) == TRUE;
        ShowWindow(g_hwndEspHist, SW_HIDE);
        break;
    case SIZE_RESTORED:
        if(g_Options.bTrayIcon)
            CreateSystrayIcon(false);
        if(g_PopupShown)
            DestroyWindow(g_hwndPopup);
        InvalidateRect(g_hwndDlg, NULL, TRUE);
        if(was_vis)
            ShowWindow(g_hwndEspHist, SW_SHOW);
        break;
    case SIZE_MAXIMIZED:
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}

void OnMenuMsg(LPARAM lParam, WPARAM wParam)
{
    switch(LOWORD(wParam))
    {
    case ID_DATEI_TECHNOLOGIEENSPEICHERN:
        SaveTechs(g_CurSaveData);
        break;
    case ID_DATEI_EINSTELLUNGENSPEICHERN_NEU:
        {
            genstring name = InputBox(g_MiscStr[8]);
            if(!name.length()) {
                break;
            }
            HMENU hFileMenu = GetMenu(g_hwndDlg);
            hFileMenu = GetSubMenu(hFileMenu, 0);
            HMENU hMenu = GetSubMenu(hFileMenu, 0);
            CheckMenuItem(hMenu, g_CurSaveData, MF_BYPOSITION|MF_UNCHECKED);
            // save options and techs
            g_CurSaveData = SaveOpts(256);
            SaveTechs(g_CurSaveData);
            // get and save name            
            SetOptSaveName(SPEEDSIM_SAVEFILE, g_CurSaveData, name.c_str());
            TCHAR c[64];
            if(name.length())
                _tcscpy(c, name.c_str());
            else
                _itot(g_CurSaveData, c, 10);
            // save menu
            AppendMenu(hMenu, MF_STRING, IDM_FIRST_OPTSAVE + g_CurSaveData, c);
            CheckMenuItem(hMenu, IDM_FIRST_OPTSAVE + g_CurSaveData, MF_BYCOMMAND|MF_CHECKED);
            // delete menu
            hMenu = GetSubMenu(hFileMenu, 2);
            AppendMenu(hMenu, MF_STRING, IDM_FIRST_OPTDELETE + g_CurSaveData, c);
            // load menu
            hMenu = GetSubMenu(hFileMenu, 1);
            if(g_CurSaveData <= 5) {
                TCHAR num[5];
                _itot(g_CurSaveData, num, 10);
                // add [Ctrl]+Num for loading
                _tcscat(c, _T("\tCtrl+"));
                _tcscat(c, num);
            }
            AppendMenu(hMenu, MF_STRING, IDM_FIRST_OPTLOAD + g_CurSaveData, c);
            DrawMenuBar(g_hwndDlg);
        }
        break;
    case ID_FLOTTEN_SPEICHERN_ANGREIFER_NEU:
        {
            // get name
            genstring name = InputBox(g_MiscStr[8]);
            if(!name.length())
                break;
            g_NumFleets = SaveFleet(256, true);
            // update fleet slot
            AddFleetSlot(g_NumFleets, name.c_str());
        }
        break;
    case ID_FLOTTEN_SPEICHERN_VERTEIDIGER_NEU:
        {
            // get name
            genstring name = InputBox(g_MiscStr[8]);
            if(!name.length())
                break;
            g_NumFleets = SaveFleet(256, false);
            // update fleet slot
            AddFleetSlot(g_NumFleets, name.c_str());
        }
        break;
    case ID_DATEI_ERWITERTEEINSTELLUNGEN:
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPTIONS), g_hwndDlg, OptionsProc);
        break;
    case ID_FLOTTEN_ALLEFLOTTENSLOTSLEEREN:
        {
            SetDlgItemInt(g_hwndDlg, IDC_FLEET_ID, 1, false);
            sim.Reset();
            //LoadTechs(g_CurSaveData, true);
            //LoadOpts(g_CurSaveData);
            UpdateEditFields(true, true);
        }
        break;
    case IDM_CLOSE:
        PostMessage(g_hwndDlg, WM_CLOSE, 0, 0);
        break;
    case IDM_IPMMODE:
        g_IPMMode = !g_IPMMode;
        ActivateIPMMode(g_IPMMode);
        break;
    case IDM_SHOW_ESPHIST:
        if(IsWindowVisible(g_hwndEspHist))
            ShowWindow(g_hwndEspHist, SW_HIDE);
        else
            ShowWindow(g_hwndEspHist, SW_SHOWNORMAL);
        break;
    default:
        break;
    }
}

void OnFleetID(LPARAM lParam, WPARAM wParam)
{
    if(HIWORD(wParam) == EN_CHANGE) {
        int id = ReadFromEdit(IDC_FLEET_ID)-1;
        if(id > 15)
            SetDlgItemInt(g_hwndDlg, IDC_FLEET_ID, 16, false);
        if(id < 0)
            SetDlgItemInt(g_hwndDlg, IDC_FLEET_ID, 1, false);
        if (id != 0)
            EnableDefense(false);
        else
            EnableDefense(true);
        UpdateKernel(g_LastID);
        UpdateEditFields(true, true);
        SItem att[T_END], def[T_END];
        sim.GetFleetAfterSim(att, def, id);
        ShowFightResults(sim.GetBattleResult(), att, def, false);
        g_LastID = id;
    }
}

void OnAccelerator(LPARAM lParam, WPARAM wParam)
{
    // ignore these while simulating
    if(g_SimCount && LOWORD(wParam) != ID_ENTER)
        return;

    switch(LOWORD(wParam))
    {
    case ID_SLOT1:
        SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDM_FIRST_OPTLOAD+1, 0), 0);
        break;
    case ID_SLOT2:
        SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDM_FIRST_OPTLOAD+2, 0), 0);
        break;
    case ID_SLOT3:
        SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDM_FIRST_OPTLOAD+3, 0), 0);
        break;
    case ID_SLOT4:
        SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDM_FIRST_OPTLOAD+4, 0), 0);
        break;
    case ID_SLOT5:
        SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDM_FIRST_OPTLOAD+5, 0), 0);
        break;
    case ID_ENTER:
        SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SIM, BN_CLICKED), 0);
        break;
    case ID_ESCAPE:
        if(!g_Options.bTrayIcon) {
            SaveOpts(g_CurSaveData);
            PostQuitMessage(0);
        }
        else
            ShowWindow(g_hwndDlg, SW_MINIMIZE);
        break;
    case ID_NEWINSTANCE:
        {
            TCHAR szFilename[MAX_PATH];
            GetModuleFileName(GetModuleHandle(NULL), szFilename, MAX_PATH);
            ShellExecute(NULL, _T("open"), szFilename, NULL, NULL, SW_SHOWNORMAL);
        }
        break;
    case ID_SHOWOPTS:
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPTIONS), g_hwndDlg, OptionsProc);
        break;
    case ID_NEXTFLEET:
        if(!g_IPMMode)
            SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_NEXT_FL, BN_CLICKED), (LPARAM)GetDlgItem(g_hwndDlg, IDC_NEXT_FL));
        break;
    case ID_PREVFLEET:
        if(!g_IPMMode)
            SendMessage(g_hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_PREV_FL, BN_CLICKED), (LPARAM)GetDlgItem(g_hwndDlg, IDC_PREV_FL));
        break;
    case ID_SHOW_HIST:
        if(IsWindowVisible(g_hwndEspHist))
            ShowWindow(g_hwndEspHist, SW_HIDE);
        else
            ShowWindow(g_hwndEspHist, SW_SHOWNORMAL);
        break;
    case ID_EDIT_ESP:
        UpdateKernel();
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CHGSPIO), NULL, SpioProc);
        break;
    default:
        break;
    }
}

int OnPaintPopup(UINT uMsg, LPARAM lParam, WPARAM wParam, TCHAR* text)
{
    HDC dc = CreateCompatibleDC(NULL);
    HDC hdc;
    if(uMsg == WM_ERASEBKGND)
        hdc = (HDC)wParam;
    else
        hdc = GetDC(g_hwndPopup);
    RECT r;
    GetClientRect(g_hwndPopup, &r);
    // load image
    SIZE bmpSize;
    bmpSize.cx = 191;
    bmpSize.cy = 254;
    
    HBITMAP oldbmp = (HBITMAP)SelectObject(dc, g_hPopupBMP);
    HFONT oldFont = (HFONT)SelectObject(hdc, g_hPopupFont);
    // blit image to window
    //StretchBlt(hdc, 0, 0, r.right, r.bottom, dc, 0, 0, bmpSize.cx, bmpSize.cy, SRCCOPY);
    BitBlt(hdc, 0, 0, r.right, r.bottom, dc, 0, 0, SRCCOPY);
    
    // draw Text
    r.left += 5;
    r.top += 5;
    r.bottom -= 5;
    r.right -= 5;
    // set text options
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, text, _tcslen(text), &r, DT_LEFT|DT_TOP);
    // release everything
    SelectObject(dc, oldbmp);
    SelectObject(hdc, oldFont);
    ReleaseDC(g_hwndPopup, hdc);
    DeleteDC(dc);
    if(uMsg == WM_ERASEBKGND)
        return TRUE;
    else
        return FALSE;
}

int OnNotify(WPARAM wParam, LPARAM lParam)
{
    DWORD idCtrl = (DWORD)wParam;
    LPNMHDR pnmh = (LPNMHDR)lParam;
    switch(pnmh->idFrom)
    {
    case ID_STATUSBAR:
        if(pnmh->code == NM_CLICK)
        {
            LPNMMOUSE pNMM = (LPNMMOUSE)lParam;
            /*if(pNMM->dwItemSpec == 3)
                MessageBox(NULL, _T(""), "", MB_OK);
            if(pNMM->dwItemSpec == 2)
                MessageBox(NULL, "blah", "", MB_OK);*/
        }
    	break;
    default:
        return 0;
        break;
    }
    return 0;
}

void OnResizeHistWindow(HWND hwnd, RECT org_size, int xdiff, int ydiff, bool reset)
{
    static int lastxdiff = 0, lastydiff = 0;
    RECT r2;
    if(reset)
    {
        lastxdiff = 0;
        lastydiff = 0;
    }
    if((xdiff - lastxdiff) == 0 && (ydiff - lastydiff) == 0)
        return;

    MyMoveWnd(GetDlgItem(hwnd, IDC_CLOSE_ESP_HIST), hwnd, xdiff - lastxdiff, ydiff - lastydiff);
    MyMoveWnd(GetDlgItem(hwnd, IDC_DEL_CUR_REP), hwnd, 0, ydiff - lastydiff);
    MyMoveWnd(GetDlgItem(hwnd, IDC_DISABLE_REP_HIST), hwnd, 0, ydiff - lastydiff);
    MyMoveWnd(GetDlgItem(hwnd, IDC_S_INFO_ESP), hwnd, 0, ydiff - lastydiff);
    MyMoveWnd(GetDlgItem(hwnd, IDC_SAVE_REP), hwnd, 0, ydiff - lastydiff);
    MyMoveWnd(GetDlgItem(hwnd, IDC_NUM_REP), hwnd, xdiff - lastxdiff, ydiff - lastydiff);
    MyMoveWnd(GetDlgItem(hwnd, IDC_NUM_REP_T), hwnd, xdiff - lastxdiff, ydiff - lastydiff);
    
    // set new size for list
    HWND lv = GetDlgItem(hwnd, IDC_REPORT_LIST);
    GetWindowRect(lv, &r2);
    MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&r2, 2);
    r2.right = r2.right - r2.left + xdiff - lastxdiff;
    r2.bottom = r2.bottom - r2.top + ydiff - lastydiff;
    SetWindowPos(lv, NULL, 0, 0, r2.right, r2.bottom, SWP_NOMOVE|SWP_NOZORDER);
    
    // arrange columns
    int w;
    w = ListView_GetColumnWidth(lv, 0);
    ListView_SetColumnWidth(lv, 0, r2.right / 2 - 1);
    w = ListView_GetColumnWidth(lv, 1);
    ListView_SetColumnWidth(lv, 1, r2.right / 4 + 8);
    w = ListView_GetColumnWidth(lv, 2);
    ListView_SetColumnWidth(lv, 2, r2.right / 4 - 12);

    lastxdiff = xdiff;
    lastydiff = ydiff;
}
