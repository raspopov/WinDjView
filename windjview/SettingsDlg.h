#pragma once


// CSettingsDlg dialog

class CSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = NULL);
	virtual ~CSettingsDlg();

// Dialog Data
	enum { IDD = IDD_SETTINGS };
	BOOL m_bRestoreAssocs;
	CStatic m_ctlAbout;

protected:
	CFont m_font;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnAssociate();
	DECLARE_MESSAGE_MAP()
};
