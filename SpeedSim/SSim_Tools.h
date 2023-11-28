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

#define MAX_RAIDLIST_FLEETS 20
// ctrl ids
#define ID_STATUSBAR 5000
#define ID_FIRST_RADIO_IPM 4500
#define ID_INPUT_IPM 5201
#define ID_INPUT_ABM 5202
#define ID_OUTPUT_IPM 5203
#define ID_OUTPUT_ABM 5204
#define ID_MOVE_IP_RES 5205
#define ID_NAME_IPM 5206
#define ID_IPM_ARROW 5207
#define ID_ABM_ARROW 5208
#define ID_TAKE_OVER_IPM_RES 5209
#define ID_OUTPUT_NEEDED_IPM 5210

int ReadFromEdit(int ResID);
bool CheckFont(TCHAR* font);
void CheckResolution();

COLORREF StrToRGB(TCHAR *str);
void ShowSkinInfo(TCHAR *inipath);
void SetSkin(TCHAR *szSkinFile);

void OpenURL(TCHAR* url);
void ForceSetForegroundWindow(HWND hwnd);
genstring InputBox(genstring Text);

bool IsCharNum(TCHAR c);
bool CreateSystrayIcon(bool bCreate);
bool SetWinStartUp(bool bCreate);
bool IsInStartup();
void ShowBilanz();

bool CreatePopupInfo(BattleResult& br, vector<SItem> AttFleet, int iRaidlistItem);
HWND CreateTooltip(HWND hwnd, TCHAR* szTTText, UINT uTTID);
void UpdateTooltip(HWND hwnd, HWND hwndTT, const TCHAR* szTTText, UINT uTTID);
void CreateTooltips();
void UpdateTooltips();

void CreateStatusBar();
void UpdateStatusBar();

void AddFleetToRaidlist();
bool RemoveCurFromRaidlist();
bool SetRaidlistFleet(int iID);
void UpdateRaidlistInfo();
void SetNextRaidlistFleet();
void SetPrevRaidListFleet();

HFONT CreateFont(LPCTSTR lpszName, INT iSize, WORD wFontFormat);
bool SetFontForAllChilds(HWND wnd, HFONT hFont);
bool TransparentWindow(HWND hWnd, BYTE iFactor);

void ActivateIPMMode(bool set_active);
void CreateIPMWindows();

DWORD GetEditIdByType(ITEM_TYPE Type, bool Attacker);
DWORD GetLabelIdByType(ITEM_TYPE Type, bool Attacker);

void InitEspHistory();
bool AddReportToHistory(TargetInfo ti, time_t* t = NULL);
void UpdateHistTooltips();
void RefreshReportHistory();

void MyMoveWnd(HWND wnd, HWND parent, int xdiff, int ydiff);
void OnResizeHistWindow(HWND hwnd, RECT org_size, int xdiff, int ydiff, bool reset = false);

genstring CorrectTabs(genstrstream stream);
int CompareVersions(const TCHAR* version_1, const TCHAR* version_2);

void SetDefCSSFiles();

map<genstring, genstring> UpdateLangFiles();

struct sRaidListItem {
    vector<SItem> vAttItems[MAX_PLAYERS_PER_TEAM];
    vector<SItem> vDefItems[MAX_PLAYERS_PER_TEAM];
    PlaniPos StartPos[MAX_PLAYERS_PER_TEAM];
    PlaniPos TargetPos;
    ShipTechs TechsAtt[MAX_PLAYERS_PER_TEAM];
    ShipTechs TechsDef[MAX_PLAYERS_PER_TEAM];
    BattleResult battle_res;
    SItem AttResult[MAX_PLAYERS_PER_TEAM][T_END];
    SItem DefResult[MAX_PLAYERS_PER_TEAM][T_END];
    BYTE iUsedFleet;
    sRaidListItem() {
        for (int i = 0; i < MAX_PLAYERS_PER_TEAM; i++) {
            StartPos[i] = PlaniPos();
        }
        TargetPos = PlaniPos();
        memset(TechsAtt, 0, sizeof(ShipTechs) * MAX_PLAYERS_PER_TEAM);
        memset(TechsDef, 0, sizeof(ShipTechs) * MAX_PLAYERS_PER_TEAM);
        memset(AttResult, 0, sizeof(SItem) * MAX_PLAYERS_PER_TEAM * T_END);
        memset(DefResult, 0, sizeof(SItem) * MAX_PLAYERS_PER_TEAM * T_END);
    }
};

struct EspHistItem
{
    TargetInfo Target;
    time_t ctime;
    TCHAR TipText[1024];
    bool Checked;
    EspHistItem()
    {
        memset(TipText, 0, 1024 * sizeof(TCHAR));
        Checked = false;
        ctime = 0;
    }
};
extern vector<EspHistItem> g_vRaidHistory;
extern WNDPROC OldLVProc;
