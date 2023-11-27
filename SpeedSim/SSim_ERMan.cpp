/*
SpeedSim - a OGame (www.ogame.org) combat simulator
Copyright (C) 2004-2007 Maximialian Matthé & Nicolas Höft

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

HWND hWndERManPages[3], hPropSheet = NULL;
vector<TargetInfo> vBestTargets;

// in development
void ShowEspioManager()
{
    HPROPSHEETPAGE hPages[3];
    PROPSHEETPAGE psp;
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hInst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ESPIO_MANAG1);
    psp.pfnDlgProc = (DLGPROC)ERManPage1Proc;
    hPages[0] = CreatePropertySheetPage(&psp);
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ESPIO_MANAG2);
    psp.pfnDlgProc = (DLGPROC)ERManPage2Proc;
    hPages[1] = CreatePropertySheetPage(&psp);
    psp.pfnDlgProc = (DLGPROC)ERManPage3Proc;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ESPIO_MANAG3);
    hPages[2] = CreatePropertySheetPage(&psp);
    PROPSHEETHEADER psh;
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_WIZARD_LITE;
    psh.hwndParent = g_hwndDlg;
    psh.hInstance = g_hInst;
    psh.nPages = 3;
    psh.phpage = hPages;
    PropertySheet(&psh);
}

void ERManGetText()
{
    TCHAR* szReports = NULL;
    bool bUnicode = IsClipboardFormatAvailable(CF_UNICODETEXT) == TRUE;
    if(!IsClipboardFormatAvailable(CF_OEMTEXT) && !IsClipboardFormatAvailable(CF_TEXT) && !bUnicode)
        return;
    if(OpenClipboard(g_hwndDlg))
    {
        if(bUnicode) {
            wchar_t* szBuff = (wchar_t*)GetClipboardData(CF_UNICODETEXT);
            szReports = new TCHAR[wcslen(szBuff) + 1];
#ifndef UNICODE
            wcstombs(szReports, szBuff, wcslen(szBuff) + 1);
#else
            wcscpy(szReports, szBuff);
#endif
        }
        else {
            char* szBuff = (char*)GetClipboardData(CF_TEXT);
            szReports = new TCHAR[strlen(szBuff) + 1];
#ifdef UNICODE
            mbstowcs(szReports, szBuff, strlen(szBuff) + 1);
#else
            strcpy(szReports, szBuff);
#endif
        }
        CloseClipboard();
    }
    genstring r = szReports;
    delete[] szReports;

    TargetInfo tis[50];
    int nER = sim.MultiReadEspReport(r, tis, 50);
    if(nER > 0 && SendDlgItemMessage(hWndERManPages[0], IDC_DONTDEL, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
        // clean up
        ListView_DeleteAllItems(GetDlgItem(hWndERManPages[0], IDC_ESPIOAVAIL));
        ListView_DeleteAllItems(GetDlgItem(hWndERManPages[0], IDC_ESPIONEXT));
    }
    
    AddERToManager(tis, nER);
}

void AddERToManager(TargetInfo *tis, int nTI, HWND hwndLV /*= NULL*/)
{
    HWND hLV = GetDlgItem(hWndERManPages[0], IDC_ESPIOAVAIL);
    HWND hLV2 = GetDlgItem(hWndERManPages[0], IDC_ESPIONEXT);
    for(int i = 0; i < nTI; i++)
    {
        LVITEM lvi;
        TCHAR szPos[64];
        szPos[63] = '\0';
        TargetInfo *ti = new TargetInfo;
        *ti = tis[i];

        HWND hLVTmp;
        if(hwndLV)
            hLVTmp = hwndLV;
        else if(tis[i].State == REPORT_ALL)
            hLVTmp = hLV2;
        else
            hLVTmp = hLV;
        
        // coordinates
        lvi.mask = LVIF_TEXT|LVIF_PARAM;
        lvi.iSubItem = 0;
        _tcsncpy(szPos, tis[i].Pos.ToString().c_str(), 63);
        lvi.pszText = szPos;
        lvi.cchTextMax = tis[i].Pos.ToString().length();
        lvi.lParam = (LPARAM)ti;
        lvi.iItem = ListView_InsertItem(hLVTmp, &lvi);        
        
        // planet name
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 1;
        lvi.pszText = (TCHAR*)tis[i].Name;
        lvi.cchTextMax = 64;
        ListView_SetItem(hLVTmp, &lvi);
    }
}

void GetBestReports()
{
    int i;
    vBestTargets.clear();
    vector<TargetInfo> vAvailTargets;
    // get options
    bool bRes = SendDlgItemMessage(hWndERManPages[1], IDC_USE_RES, BM_GETCHECK, 0, 0) == BST_CHECKED;
    bool bFleet = SendDlgItemMessage(hWndERManPages[1], IDC_USE_FLEET, BM_GETCHECK, 0, 0) == BST_CHECKED;
    bool bDef = SendDlgItemMessage(hWndERManPages[1], IDC_USE_DEF, BM_GETCHECK, 0, 0) == BST_CHECKED;
    int cMaxReports = SendDlgItemMessage(hWndERManPages[1], IDC_NUMSPIN, UDM_GETPOS32, 0, 0);

    if(!cMaxReports)
        return;

    // get chosen targets
    HWND hLV = GetDlgItem(hWndERManPages[0], IDC_ESPIONEXT);
    int nAvailTargets = ListView_GetItemCount(hLV);
    for(i = 0; i < nAvailTargets; i++)
    {
        LVITEM lvi;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM;
        ListView_GetItem(hLV, &lvi);

        TargetInfo ti;
        ti = *(TargetInfo*)lvi.lParam;
        vAvailTargets.push_back(ti);
    }

    if(!bDef && !bFleet && !bRes)
    {
        for(i = 0; i < cMaxReports && i < nAvailTargets; i++)
            vBestTargets.push_back(vAvailTargets[i]);
        return;
    }
}

void FinishERMan()
{

}

genstring LVTitles[] = { _T("Koords"), _T("Name"), _T("Status") };


bool InitListView(HWND hwndParent, int ctrlID)
{
    HWND hwndLV = GetDlgItem(hwndParent, ctrlID);
    if(!hwndLV)
        return false;
    
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    
    switch(ctrlID)
    {
    case IDC_ESPIOAVAIL:
    case IDC_ESPIONEXT:
    case IDC_ESPIO_RESULT:
        lvc.pszText = (TCHAR*)LVTitles[0].c_str();
        lvc.cchTextMax = LVTitles[0].length() + 1;
        lvc.cx = 50;
        ListView_InsertColumn(hwndLV, 0, &lvc);
        
        lvc.pszText = (TCHAR*)LVTitles[1].c_str();
        lvc.cchTextMax = LVTitles[1].length() + 1;
        lvc.cx = 125;
        ListView_InsertColumn(hwndLV, 1, &lvc);

        break;
    default:
        return false;
    }
    ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
    return true;
}

bool MoveSelItems(int ctrlStart, int ctrlTarget)
{
    HWND hLV1 = GetDlgItem(hWndERManPages[0], ctrlStart);
    HWND hLV2 = GetDlgItem(hWndERManPages[0], ctrlTarget);
    
    int nSelItems = ListView_GetSelectedCount(hLV1);
    int nItems = ListView_GetItemCount(hLV1);
    int curItem = 0;

    TargetInfo *tis = new TargetInfo[nSelItems];

    // get selected items and move to second list view
    for(int i = nItems - 1; i >= 0; i--)
    {
        LVITEM lvi;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_STATE|LVIF_PARAM;
        lvi.stateMask = LVIS_SELECTED;
        ListView_GetItem(hLV1, &lvi);
        if(lvi.state == LVIS_SELECTED)
        {
            tis[curItem++] = *(TargetInfo*)lvi.lParam;
            ListView_DeleteItem(hLV1, i);
        }
    }
    AddERToManager(tis, curItem, hLV2);
    
    return true;
}

void UpdateResultInfo(TargetInfo* ti)
{
    // output the target information
    TCHAR out[1024];
    size_t i;
    // resources
    _stprintf(out, _T("%s on [%s]\r\n"), ti->Name, ti->Pos.ToString().c_str());

    // fleet
    _tcsncat(out, _T("Fleets\r\n"), 1024);
    for(i = 0; i < ti->Fleet.size(); i++)
    {
        TCHAR tmp[256];
        
        _stprintf(tmp, _T("%s: %d\r\n"), sim.ItemName(ti->Fleet[i].Type).c_str(), (int)ti->Fleet[i].Num);
        _tcsncat(out, tmp, 1024);
    }
    
    // defence
    _tcsncat(out, _T("Defence\r\n"), 1024);
    for(i = 0; i < ti->Defence.size(); i++)
    {
        TCHAR tmp[256];
        _stprintf(tmp, _T("%s: %d\r\n"), sim.ItemName(ti->Defence[i].Type).c_str(), (int)ti->Defence[i].Num);
        _tcsncat(out, tmp, 1024);
    }
    SetWindowText(GetDlgItem(hWndERManPages[2], IDC_ESPIO_INFO), out);
}

void UpdateReportResult()
{
    HWND hLV = GetDlgItem(hWndERManPages[2], IDC_ESPIO_RESULT);
    ListView_DeleteAllItems(hLV);
    
    size_t size = vBestTargets.size();
    if(!size)
        return;
    TargetInfo* tis = new TargetInfo[size];
    
    for(size_t i = 0; i < size; i++)
        tis[i] = vBestTargets[i];

    AddERToManager(tis, size, hLV);
    delete[] tis;

    LVITEM lvi;
    lvi.iItem = 0; lvi.iSubItem = 0;
    lvi.mask = LVIF_PARAM;
    ListView_GetItem(hLV, &lvi);
    tis = (TargetInfo*)lvi.lParam;
}

int __stdcall ERManPage1Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        hWndERManPages[0] = hWnd;
        hPropSheet = GetParent(hWnd);
        InitListView(hWnd, IDC_ESPIOAVAIL);
        InitListView(hWnd, IDC_ESPIONEXT);
        ERManGetText();
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_GETTEXT:
            ERManGetText();
        	break;
        case IDC_MOVEALL:
            ListView_SetItemState(GetDlgItem(hWnd, IDC_ESPIOAVAIL), -1, LVIS_SELECTED, LVIS_SELECTED);
        case IDC_MOVERIGHT:
            MoveSelItems(IDC_ESPIOAVAIL, IDC_ESPIONEXT);
            break;
        case IDC_MOVELEFT:
            MoveSelItems(IDC_ESPIONEXT, IDC_ESPIOAVAIL);
            break;
        default:
            return FALSE;
        }
        break;
    case WM_DESTROY:
        break;
    case WM_NOTIFY:
        switch(((NMHDR*)lParam)->code)
        {
        case LVN_DELETEITEM:
            {
                // free memory
                NMLISTVIEW* nmlv = (NMLISTVIEW*)lParam;
                delete (TargetInfo*)nmlv->lParam;
                if(nmlv->hdr.idFrom == IDC_ESPIONEXT)
                {
                    // disable 'Next', if no items left
                    int nItems = ListView_GetItemCount(nmlv->hdr.hwndFrom);
                    // 1, because item is not yet removed
                    if(nItems == 1)
                        PropSheet_SetWizButtons(hPropSheet, 0);
                }
            }
            break;
        case LVN_INSERTITEM:
            if(((NMHDR*)lParam)->idFrom == IDC_ESPIONEXT)
                PropSheet_SetWizButtons(hPropSheet, PSWIZB_NEXT);
            break;
        case PSN_SETACTIVE:
            {
                int nItems = ListView_GetItemCount(GetDlgItem(hWnd, IDC_ESPIONEXT));
                if(nItems)
                    PropSheet_SetWizButtons(hPropSheet, PSWIZB_NEXT);
                else
                    PropSheet_SetWizButtons(hPropSheet, 0);
            }
            break;
        default:
            return FALSE;
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

int __stdcall ERManPage2Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        hWndERManPages[1] = hWnd;
        SendDlgItemMessage(hWnd, IDC_NUMSPIN, UDM_SETRANGE, 0, MAKELONG(50, 1));
        SetDlgItemInt(hWnd, IDC_MAX_ESPIO, 1, FALSE);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_AS_STANDARD:
            break;
        default:
            return FALSE;
        }
        break;
    case WM_DESTROY:
        break;
    case WM_NOTIFY:
        switch(((NMHDR*)lParam)->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(hPropSheet, PSWIZB_NEXT|PSWIZB_BACK);
        default:
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

int __stdcall ERManPage3Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        hWndERManPages[2] = hWnd;
        InitListView(hWnd, IDC_ESPIO_RESULT);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_TO_RAIDLIST:
            break;
        case IDC_TO_SIM:
            break;
        default:
            return FALSE;
        }
        break;
    case WM_DESTROY:
        break;
    case WM_NOTIFY:
        switch(((NMHDR*)lParam)->code)
        {
        case LVN_DELETEITEM:
            // free memory
            delete (TargetInfo*)((NMLISTVIEW*)lParam)->lParam;
            break;
        case LVN_ITEMCHANGED:
            UpdateResultInfo((TargetInfo*)((NMLISTVIEW*)lParam)->lParam);
            break;
        case PSN_WIZFINISH:
            FinishERMan();
            break;
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(hPropSheet, PSWIZB_FINISH|PSWIZB_BACK);
            GetBestReports();
            UpdateReportResult();
            break;
        default:
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
