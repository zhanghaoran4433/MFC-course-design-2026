#pragma once

#include "DataManager.h"
#include "Resource.h"

class CMissionManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMissionManagerDlg)

public:
	CMissionManagerDlg(CDataManager* pDataManager,
		CString* pCurrentMissionId,
		CWnd* pParent = nullptr);

	enum { IDD = IDD_MISSION_MANAGER_DIALOG };

	virtual BOOL OnInitDialog();
	void RefreshMissionList();
	int GetSelectedMissionIndex() const;
	afx_msg void OnBnClickedMissionAdd();
	afx_msg void OnBnClickedMissionModify();
	afx_msg void OnBnClickedMissionDelete();
	afx_msg void OnBnClickedMissionSelect();
	afx_msg void OnBnClickedMissionSave();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	CDataManager* m_pDataManager = nullptr;
	CString* m_pCurrentMissionId = nullptr;
	CListCtrl m_missionList;
};
