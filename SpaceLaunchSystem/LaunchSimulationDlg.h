#pragma once

#include "DataManager.h"
#include "Resource.h"

class CLaunchSimulationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLaunchSimulationDlg)

public:
	CLaunchSimulationDlg(CDataManager* pDataManager,
		const CString& missionId,
		CWnd* pParent = nullptr);

	enum { IDD = IDD_LAUNCH_SIMULATION_DIALOG };

	virtual BOOL OnInitDialog();
	virtual void OnCancel();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	static constexpr UINT_PTR TIMER_LAUNCH = 1;
	CDataManager* m_pDataManager = nullptr;
	CString m_missionId;
	LaunchState m_launchState = LaunchState::Idle;
	int m_countdown = 10;
	int m_progress = 0;
	int m_remainingFuel = 100;
	double m_altitudeKm = 0.0;
	double m_speedKmh = 0.0;
	double m_maxHeightKm = 0.0;
	BOOL m_simulationRunning = FALSE;
	BOOL m_recordCreated = FALSE;
	UINT_PTR m_timerId = 0;
	LaunchState m_stateBeforePause = LaunchState::Idle;
	CProgressCtrl m_launchProgress;
	CProgressCtrl m_fuelProgress;

	void ResetSimulation();
	void RefreshSimulationDisplay();
	void RefreshButtonStates();
	void AppendLog(const CString& message);
	void CompleteLaunch();
	void AbortLaunch();
	BOOL StartSimulationTimer();
	void StopSimulationTimer();
	BOOL UpdateMissionStatus(MissionStatus status);
	void CreateLaunchRecord(const CString& result, const CString& description);
	void HandleCloseRequest();

	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg void OnBnClickedLaunchStart();
	afx_msg void OnBnClickedLaunchPause();
	afx_msg void OnBnClickedLaunchAbort();
	afx_msg void OnBnClickedLaunchReset();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
};
