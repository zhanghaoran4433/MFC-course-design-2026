#pragma once

#include "DataManager.h"
#include "Resource.h"

class CLaunchCheckDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLaunchCheckDlg)

public:
	CLaunchCheckDlg(CDataManager* pDataManager,
		const CString& missionId,
		CWnd* pParent = nullptr);

	enum { IDD = IDD_LAUNCH_CHECK_DIALOG };

	virtual BOOL OnInitDialog();
	void RefreshReadiness();
	BOOL AreAllChecksPassed() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	CDataManager* m_pDataManager = nullptr;
	CString m_missionId;
	BOOL m_propulsionReady = FALSE;
	BOOL m_navigationReady = FALSE;
	BOOL m_communicationReady = FALSE;
	BOOL m_powerReady = FALSE;
	BOOL m_weatherReady = FALSE;
	CString m_remarks;
	CProgressCtrl m_readinessProgress;

	BOOL IsCheckEditingAllowed(const MissionInfo* pMission) const;
	void LoadSavedCheckResult();
	void RefreshMissionText(const MissionInfo* pMission);
	void RefreshControlAvailability();
	void RestoreSavedCheckDisplay();
	void ShowSaveBlockedMessage(MissionStatus status) const;
	void ShowSimulationBlockedMessage(MissionStatus status) const;

	afx_msg void OnCheckChanged();
	afx_msg void OnBnClickedCheckAllPass();
	afx_msg void OnBnClickedCheckSave();
	afx_msg void OnBnClickedEnterSimulation();
};
