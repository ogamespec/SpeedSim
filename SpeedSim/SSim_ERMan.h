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

#pragma once

void ShowEspioManager();

void ERManGetText();
void AddERToManager(TargetInfo *tis, int nTI, HWND hwndLV = NULL);
void FinishERMan();
void GetBestReports();
void UpdateResultInfo(TargetInfo* ti);
void UpdateReportResult();

INT_PTR ERManPage1Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR ERManPage2Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR ERManPage3Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool InitListView(HWND hwndParent, int ctrlID);
bool MoveSelItems(int ctrlStart, int ctrlTarget);
