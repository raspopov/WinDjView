// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinDjView.h"
#include "SettingsDlg.h"

#include "AppSettings.h"
#include "MainFrm.h"


// CSettingsDlg dialog

IMPLEMENT_DYNAMIC(CSettingsDlg, CDialog)
CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	m_bRestoreAssocs = CAppSettings::bRestoreAssocs;
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_RESTORE_ASSOCS, m_bRestoreAssocs);
	DDX_Control(pDX, IDC_STATIC_ABOUT, m_ctlAbout);
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_ASSOCIATE, OnAssociate)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSettingsDlg message handlers

void CSettingsDlg::OnAssociate()
{
	if (theApp.RegisterShellFileTypes())
	{
		AfxMessageBox(
			_T("Now you will be able to open .djvu files ")
			_T("with WinDjView\nby double-clicking them in ")
			_T("the explorer."), MB_ICONINFORMATION | MB_OK);
	}
	else
	{
		AfxMessageBox(
			_T("An error occurred while trying to register file associations.\n")
			_T("You may not have enough permissions to write system registry."),
			MB_ICONERROR | MB_OK);
	}
}

void CSettingsDlg::OnOK()
{
	if (!UpdateData())
		return;

	CAppSettings::bRestoreAssocs = !!m_bRestoreAssocs;

	CDialog::OnOK();
}

BOOL CSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CFont fnt;
	CreateSystemDialogFont(fnt);
	LOGFONT lf;
	fnt.GetLogFont(&lf);

	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	lf.lfHeight = -MulDiv(7, dcScreen.GetDeviceCaps(LOGPIXELSY), 72);
	m_font.CreateFontIndirect(&lf);

	m_ctlAbout.SetFont(&m_font);

	return TRUE;
}

HBRUSH CSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetSafeHwnd() == m_ctlAbout.m_hWnd)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_3DSHADOW));
	}

	return brush;
}
