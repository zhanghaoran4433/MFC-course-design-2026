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
	double m_altitudeKm = 0.0;
	double m_speedKmh = 0.0;
	BOOL m_recordCreated = FALSE;
	CProgressCtrl m_launchProgress;
	CProgressCtrl m_fuelProgress;
};
