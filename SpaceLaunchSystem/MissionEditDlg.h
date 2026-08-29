#pragma once

#include "DataTypes.h"
#include "Resource.h"

class CMissionEditDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMissionEditDlg)

public:
	explicit CMissionEditDlg(CWnd* pParent = nullptr);

	enum { IDD = IDD_MISSION_EDIT_DIALOG };

	MissionInfo m_mission;
	BOOL m_isModifyMode = FALSE;

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
};
