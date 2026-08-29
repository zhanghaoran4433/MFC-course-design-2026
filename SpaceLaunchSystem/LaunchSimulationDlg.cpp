#include "pch.h"
#include "LaunchSimulationDlg.h"

IMPLEMENT_DYNAMIC(CLaunchSimulationDlg, CDialogEx)

CLaunchSimulationDlg::CLaunchSimulationDlg(CDataManager* pDataManager,
	const CString& missionId,
	CWnd* pParent)
	: CDialogEx(IDD_LAUNCH_SIMULATION_DIALOG, pParent)
	, m_pDataManager(pDataManager)
	, m_missionId(missionId)
{
}

void CLaunchSimulationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_LAUNCH, m_launchProgress);
	DDX_Control(pDX, IDC_PROGRESS_FUEL, m_fuelProgress);
}

BEGIN_MESSAGE_MAP(CLaunchSimulationDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CLaunchSimulationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_launchProgress.SetRange(0, 100);
	m_launchProgress.SetPos(0);
	m_fuelProgress.SetRange(0, 100);
	m_fuelProgress.SetPos(100);

	CString missionText;
	missionText.Format(L"当前任务：%s", static_cast<LPCWSTR>(m_missionId));
	SetDlgItemText(IDC_STATIC_SIM_MISSION, missionText);
	SetDlgItemInt(IDC_STATIC_COUNTDOWN, static_cast<UINT>(m_countdown), FALSE);
	SetDlgItemText(IDC_STATIC_STAGE, L"当前阶段：等待发射");
	SetDlgItemText(IDC_STATIC_ALTITUDE, L"高度：0.0 km");
	SetDlgItemText(IDC_STATIC_SPEED, L"速度：0.0 km/h");

	return TRUE;
}
