﻿// TortoiseMerge - a Diff/Patch program

// Copyright (C) 2006, 2011-2014, 2016, 2020-2022 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "FindDlg.h"

// CFindDlg dialog

IMPLEMENT_DYNAMIC(CFindDlg, CStandAloneDialog)

CFindDlg::CFindDlg(CWnd* pParent /*=nullptr*/)
    : CStandAloneDialog(CFindDlg::IDD, pParent)
    , m_findMsg(0)
    , m_bTerminating(false)
    , m_bFindNext(false)
    , m_bMatchCase(FALSE)
    , m_bLimitToDiffs(FALSE)
    , m_bWholeWord(FALSE)
    , m_bSearchUp(FALSE)
    , m_pParent(pParent)
    , m_regMatchCase(L"Software\\TortoiseMerge\\FindMatchCase", FALSE)
    , m_regLimitToDiffs(L"Software\\TortoiseMerge\\FindLimitToDiffs", FALSE)
    , m_regWholeWord(L"Software\\TortoiseMerge\\FindWholeWord", FALSE)
    , m_clrFindStatus(RGB(0, 0, 255))
    , m_bReadonly(false)
    , m_id(0)
{
}

CFindDlg::~CFindDlg()
{
}

void CFindDlg::Create(CWnd* pParent, int id /* = 0 */)
{
    CStandAloneDialog::Create(IDD, pParent);
    if (id && pParent)
    {
        m_id       = id;
        POINT   pt = {0};
        CString sRegPath;
        sRegPath.Format(L"Software\\TortoiseMerge\\FindDlgPosX%d", id);
        pt.x = static_cast<int>(static_cast<DWORD>(CRegDWORD(sRegPath, 0)));
        sRegPath.Format(L"Software\\TortoiseMerge\\FindDlgPosY%d", id);
        pt.y = static_cast<int>(static_cast<DWORD>(CRegDWORD(sRegPath, 0)));
        pParent->ClientToScreen(&pt);
        if (MonitorFromPoint(pt, MONITOR_DEFAULTTONULL))
            SetWindowPos(nullptr, pt.x, pt.y, 0, 0, SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSIZE);
    }
    ShowWindow(SW_SHOW);
    UpdateWindow();
}

CString CFindDlg::GetFindString() const
{
    CString text;
    m_findCombo.GetWindowText(text);
    return text;
}

CString CFindDlg::GetReplaceString() const
{
    CString text;
    m_replaceCombo.GetWindowText(text);
    return text;
}

void CFindDlg::SetFindString(const CString& str)
{
    m_findCombo.SetWindowText(str);
}

void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
    CStandAloneDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_MATCHCASE, m_bMatchCase);
    DDX_Check(pDX, IDC_LIMITTODIFFS, m_bLimitToDiffs);
    DDX_Check(pDX, IDC_WHOLEWORD, m_bWholeWord);
    DDX_Check(pDX, IDC_SEARCHUP, m_bSearchUp);
    DDX_Control(pDX, IDC_FINDCOMBO, m_findCombo);
    DDX_Control(pDX, IDC_REPLACECOMBO, m_replaceCombo);
    DDX_Control(pDX, IDC_FINDSTATUS, m_findStatus);
}

BEGIN_MESSAGE_MAP(CFindDlg, CStandAloneDialog)
    ON_CBN_EDITCHANGE(IDC_FINDCOMBO, &CFindDlg::OnCbnEditchangeFindcombo)
    ON_CBN_EDITCHANGE(IDC_REPLACECOMBO, &CFindDlg::OnCbnEditchangeFindcombo)
    ON_BN_CLICKED(IDC_COUNT, &CFindDlg::OnBnClickedCount)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_REPLACE, &CFindDlg::OnBnClickedReplace)
    ON_BN_CLICKED(IDC_REPLACEALL, &CFindDlg::OnBnClickedReplaceall)
END_MESSAGE_MAP()

// CFindDlg message handlers

void CFindDlg::OnCancel()
{
    CWnd* pParent = m_pParent;
    if (pParent == nullptr)
        pParent = GetParent();
    SaveWindowPos(pParent);

    m_bTerminating = true;
    SetStatusText(L"");
    if (pParent)
        pParent->SendMessage(m_findMsg);
    DestroyWindow();
}

void CFindDlg::PostNcDestroy()
{
    delete this;
}

void CFindDlg::OnOK()
{
    CWnd* pParent = m_pParent;
    if (pParent == nullptr)
        pParent = GetParent();
    SaveWindowPos(pParent);

    UpdateData();
    SetStatusText(L"");
    m_findCombo.SaveHistory();
    m_regMatchCase    = m_bMatchCase;
    m_regLimitToDiffs = m_bLimitToDiffs;
    m_regWholeWord    = m_bWholeWord;

    if (m_findCombo.GetString().IsEmpty())
        return;
    m_bFindNext = true;
    if (pParent)
        pParent->SendMessage(m_findMsg);
    m_bFindNext = false;
}

BOOL CFindDlg::OnInitDialog()
{
    CStandAloneDialog::OnInitDialog();
    m_findMsg = RegisterWindowMessage(FINDMSGSTRING);

    m_bMatchCase    = static_cast<BOOL>(static_cast<DWORD>(m_regMatchCase));
    m_bLimitToDiffs = static_cast<BOOL>(static_cast<DWORD>(m_regLimitToDiffs));
    m_bWholeWord    = static_cast<BOOL>(static_cast<DWORD>(m_regWholeWord));
    UpdateData(FALSE);

    m_findCombo.DisableTrimming();
    m_findCombo.LoadHistory(L"Software\\TortoiseMerge\\History\\Find", L"Search");
    m_findCombo.SetCurSel(0);

    m_replaceCombo.DisableTrimming();
    m_replaceCombo.LoadHistory(L"Software\\TortoiseMerge\\History\\Find", L"Replace");
    m_replaceCombo.SetCurSel(0);

    m_findCombo.SetFocus();

    SetTheme(CTheme::Instance().IsDarkTheme());
    return FALSE;
}

void CFindDlg::OnCbnEditchangeFindcombo()
{
    UpdateData();
    auto s = m_findCombo.GetString();
    GetDlgItem(IDOK)->EnableWindow(!s.IsEmpty());
    GetDlgItem(IDC_REPLACE)->EnableWindow(!m_bReadonly && !s.IsEmpty());
    GetDlgItem(IDC_REPLACEALL)->EnableWindow(!m_bReadonly && !s.IsEmpty());
}

void CFindDlg::OnBnClickedCount()
{
    UpdateData();
    SetStatusText(L"");
    m_findCombo.SaveHistory();
    m_regMatchCase    = m_bMatchCase;
    m_regLimitToDiffs = m_bLimitToDiffs;
    m_regWholeWord    = m_bWholeWord;

    if (m_findCombo.GetString().IsEmpty())
        return;
    m_bFindNext = true;
    if (m_pParent)
        m_pParent->SendMessage(m_findMsg, FindType::Count);
    else if (GetParent())
        GetParent()->SendMessage(m_findMsg, FindType::Count);
    m_bFindNext = false;
}

HBRUSH CFindDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    switch (nCtlColor)
    {
        case CTLCOLOR_STATIC:
            if (pWnd == &m_findStatus)
            {
                HBRUSH hbr = CStandAloneDialog::OnCtlColor(pDC, pWnd, nCtlColor);
                pDC->SetTextColor(m_clrFindStatus);
                pDC->SetBkMode(TRANSPARENT);
                return hbr;
            }
            break;
        default:
            break;
    }
    return CStandAloneDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CFindDlg::SetStatusText(const CString& str, COLORREF color)
{
    m_clrFindStatus = color;
    m_findStatus.SetWindowText(str);
}

void CFindDlg::SetReadonly(bool bReadonly)
{
    m_bReadonly = bReadonly;
    m_replaceCombo.EnableWindow(bReadonly ? FALSE : TRUE);
    GetDlgItem(IDC_REPLACE)->EnableWindow(!m_bReadonly && !m_findCombo.GetString().IsEmpty());
    GetDlgItem(IDC_REPLACEALL)->EnableWindow(!m_bReadonly && !m_findCombo.GetString().IsEmpty());
}

void CFindDlg::OnBnClickedReplace()
{
    UpdateData();
    SetStatusText(L"");
    m_findCombo.SaveHistory();
    m_replaceCombo.SaveHistory();
    m_regMatchCase    = m_bMatchCase;
    m_regLimitToDiffs = m_bLimitToDiffs;
    m_regWholeWord    = m_bWholeWord;

    m_bFindNext = true;
    if (m_pParent)
        m_pParent->SendMessage(m_findMsg, FindType::Replace);
    else if (GetParent())
        GetParent()->SendMessage(m_findMsg, FindType::Replace);
    m_bFindNext = false;
}

void CFindDlg::OnBnClickedReplaceall()
{
    UpdateData();
    SetStatusText(L"");
    m_findCombo.SaveHistory();
    m_replaceCombo.SaveHistory();
    m_regMatchCase    = m_bMatchCase;
    m_regLimitToDiffs = m_bLimitToDiffs;
    m_regWholeWord    = m_bWholeWord;

    m_bFindNext = true;
    if (m_pParent)
        m_pParent->SendMessage(m_findMsg, FindType::ReplaceAll);
    else if (GetParent())
        GetParent()->SendMessage(m_findMsg, FindType::ReplaceAll);
    m_bFindNext = false;
}

void CFindDlg::SaveWindowPos(CWnd* pParent) const
{
    if (pParent && m_id)
    {
        CRect rc;
        GetWindowRect(&rc);
        pParent->ScreenToClient(rc);
        CString sRegPath;
        sRegPath.Format(L"Software\\TortoiseMerge\\FindDlgPosX%d", m_id);
        CRegDWORD regX(sRegPath);
        regX = rc.left;
        sRegPath.Format(L"Software\\TortoiseMerge\\FindDlgPosY%d", m_id);
        CRegDWORD regY(sRegPath);
        regY = rc.top;
    }
}
