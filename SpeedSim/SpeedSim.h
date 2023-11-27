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

#ifndef SPEEDSIM_H
#define SPEEDSIM_H

//#define UNICODE

#pragma warning(disable: 4786 4503)

#ifdef UNICODE
	#ifndef _UNICODE
	#define _UNICODE
	#endif
#endif

#define WIDEN(x) _T(x)
#define __TDATE__ WIDEN(__DATE__)
#define __TTIME__ WIDEN(__TIME__)

#include <windows.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <wininet.h>
#include <ZMouse.h>
#include <tchar.h>
#include <process.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <Uxtheme.h>

#include <fstream>
#include <string>
#include <map>

#include "SkinClass.h"
#include "resource.h"

#include "..\SpeedKernel\SpeedKernel.h"

#include "SSim_Tools.h"

#define SCAN_MSG_TXT _T("SPEEDSIM")
#define SPEEDSIM_VERSION _T("0.9.8.1b")
#define SPEEDSIM_NEEDED_LANG_VERSION _T("0.9.8.1b")
#define SPEEDSIM_SERVER _T("speedsim.net")
#define SPEEDSIM_UPDATE_FILE _T("Changelog.txt")
#define SPEEDSIM_UPDATE_FILE_ENG _T("Changelog_eng.txt")
#define SPEEDSIM_SAVEFILE _T("SpeedSim.cfg")
#define SPEEDSIM_MUTEX _T("SPEEDSIM_MUTEX")
#define SPEEDSIM_CLASSNAME _T("SPEEDSIM_CLASS")
#define GERMAN_LANG _T("German/Deutsch")
#define TRAY_MESSAGE (WM_USER+1)

// Defines for own menu items
#define IDM_FIRST_ITEM 41000
#define IDM_FIRST_OPTSAVE 41000
#define IDM_FIRST_OPTLOAD 41050
#define IDM_FIRST_ATT_FLEETSAVE 41100
#define IDM_FIRST_ATT_FLEETLOAD 41150
#define IDM_FIRST_DEF_FLEETSAVE 41200
#define IDM_FIRST_DEF_FLEETLOAD 41250
#define IDM_FIRST_FLEETDELETE 41300
#define IDM_FIRST_OPTDELETE 41350
#define IDM_FIRST_LANG 41400
#define IDM_LAST_ITEM 41450
#define IDM_CHECKRAIDED 41451
#define IDM_UNCHECKRAIDED 41452

// Font Formats for changing font
#define FF_BOLD 2
#define FF_ITALIC 4
#define FF_UNDERLINE 8
#define FF_STRIKEOUT 16

// ToolTip ID's
#define TT_ID_FIRST_ATT 100
#define TT_ID_FIRST_DEF 150
#define TT_ID_DEBRIS 10
#define TT_ID_LOSSES_ATT 11
#define TT_ID_LOSSES_DEF 12
#define TT_ID_WAVES 13
#define TT_ID_TH_PLUNDER 14
#define TT_ID_MOON 15

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall OptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall UpdateProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall SpioProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall TabPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall LangProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int __stdcall InputBoxProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
int __stdcall PopupInfoProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
int __stdcall BalanceProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
int __stdcall ReportHistProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ListViewProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

struct SaveData;

extern HWND g_hwndDlg, g_hwndOpt, g_hwndUpd, g_hwndDown, g_hwndSave, g_hwndPopup;
extern HWND g_hwndEspHist;
extern HINSTANCE g_hInst;
extern unsigned long g_UpdThread, g_SimThread, g_LastSimThread, g_CheckThread;
extern WNDPROC g_oldEditProc;
extern HFONT g_Font;
extern HWND g_hwndNextCBWnd;

extern HBITMAP g_hPopupBMP;
extern HFONT g_hPopupFont;

extern vector<genstring> g_LangFiles;

extern int g_SimCount;
extern int g_LastID;
extern HANDLE g_hMutex;
extern bool g_bLowRes;

extern CSpeedKernel sim;
extern CSkinClass g_Skin;

extern UINT SCAN_MSG;
extern RECT g_WndRect, g_ClntRect;

extern TCHAR g_CurrDir[MAX_PATH];
extern DWORD g_CurSaveData;
extern int g_NumFleets;
extern bool g_PopupShown;
extern int g_iNumShipsForPlunderNeeded[T_RAK];
extern int g_iCurRaidlist;

extern bool g_IPMMode;
extern HMODULE g_hUser32DLL, g_hUXThemeDLL;
extern int g_iCheckedIPMBox;

extern int g_GeradeEingefuegt;

void ReadValuesAndSim(void*);
void ShowFightResults(BattleResult& batres, SItem *Att, SItem *Def, bool bCreateReports);
void SimulateIPMAttack();

// procedures for handling of window messeges
void InitMainDialog();
void InitTabpages(HWND hwndDlg);
void OnScroll(UINT uMsg, LPARAM lParam, WPARAM wParam);
int OnTrayMessage(UINT uMsg, LPARAM lParam, WPARAM wParam);
bool IsMenuMsg(UINT uMsg, LPARAM lParam, WPARAM wParam);
void OnMenuMsg(LPARAM lParam, WPARAM wParam);
void SwapFleets();
void OnOwnPos(LPARAM lParam, WPARAM wParam);
void OnScanMessage();
int OnSize(LPARAM lParam, WPARAM wParam);
void OnFleetID(LPARAM lParam, WPARAM wParam);
void OnAccelerator(LPARAM lParam, WPARAM wParam);
int OnPaintPopup(UINT uMsg, LPARAM lParam, WPARAM wParam, TCHAR* text);
int OnNotify(WPARAM wParam, LPARAM lParam);

void GeneralRead();

void UpdateKernel(int FleetID = ReadFromEdit(IDC_FLEET_ID)-1);
void ResetAtt();
void ResetDef();
void EnableDefense(bool bDo);
void EnableFleet(bool bDo);
void ClearResult();
void UpdateEditFields(bool doAtt, bool doDef);
void InitMenu();
void AddFleetSlot(int num, const TCHAR* name);

int SaveFleet(int num, bool SaveAtt);
int SaveOpts(int num);
int SaveTechs(int num);
bool LoadOpts(int num);
bool LoadFleet(int num, bool LoadAtt);
bool LoadTechs(int num, bool LoadAtt, bool bOnlyEngines = false);

bool SetOptSaveName(TCHAR* file, int num, const TCHAR *Name);
bool SetFleetSaveName(TCHAR* file, int num, const TCHAR *Name);
TCHAR* GetOptSaveName(TCHAR* file, int num, TCHAR *out);
TCHAR* GetFleetSaveName(TCHAR* file, int num, TCHAR *out);

int NewSetSaveData(TCHAR* file, int num, const SaveData &data);
bool NewGetSaveData(TCHAR* file, int num, SaveData &data);
bool NewGetFleetData(TCHAR* file, int num, vector<SItem> &Fleet, ShipTechs &Techs);
int NewSetFleetData(TCHAR* file, int num, const vector<SItem> &Fleet, ShipTechs Techs);
bool DeleteSaveData(TCHAR* file, int num);
bool DeleteFleetData(TCHAR* file, int num);

genstr GetSaveFolder();

bool LoadEspHistory(int save_id);
bool SaveEspHistory(int save_id);

LPTSTR AddPointsToNumber(__int64 value, LPTSTR out);
void CopyResultToClipboard();
void EnableAllWindows(bool how);

void CallBack(int sim, int round);

void CreateTabs();

void __cdecl TestNewerVersion(void*);
void __cdecl AutoUpdate(void*);

bool GetUpdateFile(genstring &out, LPTSTR UpdFileName = SPEEDSIM_UPDATE_FILE, LPTSTR UpdServer = SPEEDSIM_SERVER);

void ProcessClipboardChg();

void ReadSkinIni(LPTSTR inifile);

void LoadLang(char* langfile);
map<HWND, genstring> GetGerLang(HWND wnd);
void SetGerLang(HWND wnd, map<HWND, genstring> strings);
void CheckLangFileVersion(TCHAR* ver_str);

bool IsLanguageFile(TCHAR* IniFileName);
bool IsSkinFile(TCHAR* IniFileName);

#undef _stprintf
int _stprintf(TCHAR * target, const TCHAR * format, ...);

extern genstring g_strSim[3], g_strSimState[2];
// output strings of battle result
extern genstring g_ResString[15];
// 1-verb.fehler 2-aktuellste version 3-veraltete version 4-version(inet) 5-updatefenster 
// 6-tägl auf update, 7-Update-Check
extern genstring g_UpdaterStr[7];
// Miscellaneous strings
extern genstring g_MiscStr[10], g_IPModeStr[5];
extern genstring g_ScanEditStr[6];
extern genstring g_ResNames[3], g_TechNames[6];
extern genstring g_OptPage1[11];
extern genstring g_OptPage2[12];
extern genstring g_OptPage3[5];
extern genstring g_TrayText[7];
extern genstring g_PopupStrings[5], g_BilanzStrings[5];
extern genstring g_EspHistStrings[12], g_EspTitles[4];

extern TCHAR g_DecSeperator[10];

extern map<HWND, genstring> g_GerStringsMain;

extern HBRUSH g_TabBkBrush;
#define MAX_POS 15

typedef HRESULT (WINAPI * ETDTProc) (HWND, DWORD);
extern ETDTProc fnETDT;
typedef HRESULT (WINAPI * SetWndThmProc) (HWND, LPCWSTR, LPCWSTR);
extern SetWndThmProc fnSetWndThm;

struct TechOpts
{
    ShipTechs STechs;
    BYTE Combust;
    BYTE Impulse;
    BYTE Hyper;
    TechOpts()
    {
        Combust = Impulse = Hyper = 0;
    }
};

struct SaveData {
    TechOpts Techs;
    bool bCheckTechs;
    BYTE RFType;
    bool bDelDefOnSwitch;
    bool DefTF;
    bool BWCase;
	bool HookClipboard;
    WORD NumSims;
    TCHAR PlanPos[MAX_POS][9];
	BYTE SelPos;
	TCHAR LastUpdCheck[16];
    bool bDoUpdate;
	TCHAR SkinFileName[64];
    char LangFileName[64];
    char RFFileName[64];
    char ShipDataFile[64];
    int iSpeedFactor;
    int iDefRebuild;
    BYTE bTrayIcon;
    BYTE bPopUpOnText;
    BYTE bMinimizeOnStart;
    BYTE bSingleInst;
    BYTE bRebuildSmallDef;
    ITEM_TYPE PlunderShip;
    int iDlgTransparency;
    int iPopUpTransparency;
    REBUILD_OPTION RebuildOpt;
    bool bShowEspHist;
    TCHAR LastLangCheckVer[64];
    bool bUseOldBS;
    int EspHistPos[2];
    int EspHistSize[2];
    int cHistInFile;
    int StructureToDF;
    SaveData()
    {
        bUseOldBS = false;
        bCheckTechs = false;
        RFType = RF_075;
        bDelDefOnSwitch = false;
        DefTF = false;
        BWCase = false;
        HookClipboard = false;
        NumSims = 100;
        memset(PlanPos, 0, (9 * MAX_POS) * sizeof(TCHAR));
        SelPos = 0;
        memset(LastUpdCheck, 0, 16 * sizeof(TCHAR));
        bDoUpdate = true;
        memset(SkinFileName, 0, 64 * sizeof(TCHAR));
        memset(LangFileName, 0, 64);
        memset(RFFileName, 0, 64);
        memset(ShipDataFile, 0, 64);
        memset(LastLangCheckVer, 0, 64);
        iSpeedFactor = 1;
        iDefRebuild = 75;
        bTrayIcon = false;
        bPopUpOnText = false;
        bMinimizeOnStart = false;
        bSingleInst = false;
        bRebuildSmallDef = false;
        PlunderShip = T_GT;
        iDlgTransparency = 0;
        iPopUpTransparency = 0;
        RebuildOpt = REBUILD_AVG;
        bShowEspHist = true;
        cHistInFile = 10;
        StructureToDF = 30;
        EspHistPos[0] = 0;
        EspHistPos[1] = 0;
        memset(EspHistSize, 0, 2 * sizeof(int));
    }
};

extern SaveData g_Options;

#define C_PAGES 3

struct TabInfo { 
    HWND hwndDisplay[C_PAGES];
    BYTE lastSel;
};

struct DLUInfo
{
    int xPx;
    int yPx;
};

#endif
