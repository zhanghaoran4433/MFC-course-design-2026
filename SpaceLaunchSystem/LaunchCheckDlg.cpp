#include "pch.h"
#include "LaunchCheckDlg.h"
#include "LaunchSimulationDlg.h"

IMPLEMENT_DYNAMIC(CLaunchCheckDlg, CDialogEx)

CLaunchCheckDlg::CLaunchCheckDlg(CDataManager* pDataManager,
	const CString& missionId,
	CWnd* pParent)
	: CDialogEx(IDD_LAUNCH_CHECK_DIALOG, pParent)
	, m_pDataManager(pDataManager)
	, m_missionId(missionId)
{
}

void CLaunchCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_PROPULSION, m_propulsionReady);
	DDX_Check(pDX, IDC_CHECK_NAVIGATION, m_navigationReady);
	DDX_Check(pDX, IDC_CHECK_COMMUNICATION, m_communicationReady);
	DDX_Check(pDX, IDC_CHECK_POWER, m_powerReady);
	DDX_Check(pDX, IDC_CHECK_WEATHER, m_weatherReady);
	DDX_Text(pDX, IDC_EDIT_CHECK_REMARKS, m_remarks);
	DDX_Control(pDX, IDC_PROGRESS_READINESS, m_readinessProgress);
}

BEGIN_MESSAGE_MAP(CLaunchCheckDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_PROPULSION, &CLaunchCheckDlg::OnCheckChanged)
	ON_BN_CLICKED(IDC_CHECK_NAVIGATION, &CLaunchCheckDlg::OnCheckChanged)
	ON_BN_CLICKED(IDC_CHECK_COMMUNICATION, &CLaunchCheckDlg::OnCheckChanged)
	ON_BN_CLICKED(IDC_CHECK_POWER, &CLaunchCheckDlg::OnCheckChanged)
	ON_BN_CLICKED(IDC_CHECK_WEATHER, &CLaunchCheckDlg::OnCheckChanged)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_ALL_PASS, &CLaunchCheckDlg::OnBnClickedCheckAllPass)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_SAVE, &CLaunchCheckDlg::OnBnClickedCheckSave)
	ON_BN_CLICKED(IDC_BUTTON_ENTER_SIMULATION, &CLaunchCheckDlg::OnBnClickedEnterSimulation)
END_MESSAGE_MAP()

BOOL CLaunchCheckDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_readinessProgress.SetRange(0, 100);

	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		CString missionText;
		if (m_missionId.IsEmpty())
		{
			missionText = L"当前任务：未提供任务 ID";
		}
		else
		{
			missionText.Format(L"当前任务：不存在（任务 ID：%s）",
				static_cast<LPCTSTR>(m_missionId));
		}
		SetDlgItemText(IDC_STATIC_CHECK_MISSION, missionText);
		GetDlgItem(IDC_BUTTON_CHECK_SAVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ENTER_SIMULATION)->EnableWindow(FALSE);
		RefreshReadiness();
		AfxMessageBox(L"当前任务不存在，无法保存发射检查或进入发射模拟。",
			MB_OK | MB_ICONERROR);
		return TRUE;
	}

	CString missionText;
	missionText.Format(L"当前任务：%s（任务 ID：%s）",
		static_cast<LPCTSTR>(pMission->missionName),
		static_cast<LPCTSTR>(pMission->missionId));
	SetDlgItemText(IDC_STATIC_CHECK_MISSION, missionText);

	CheckResult* pCheckResult = m_pDataManager->FindCheckResult(m_missionId);
	if (pCheckResult != nullptr)
	{
		m_propulsionReady = pCheckResult->propulsionReady;
		m_navigationReady = pCheckResult->navigationReady;
		m_communicationReady = pCheckResult->communicationReady;
		m_powerReady = pCheckResult->powerReady;
		m_weatherReady = pCheckResult->weatherReady;
		m_remarks = pCheckResult->remarks;
	}

	UpdateData(FALSE);
	RefreshReadiness();

	return TRUE;
}

void CLaunchCheckDlg::RefreshReadiness()
{
	const int passedCount = (m_propulsionReady ? 1 : 0) +
		(m_navigationReady ? 1 : 0) +
		(m_communicationReady ? 1 : 0) +
		(m_powerReady ? 1 : 0) +
		(m_weatherReady ? 1 : 0);
	const int readiness = passedCount * 20;
	m_readinessProgress.SetPos(readiness);

	CString readinessText;
	readinessText.Format(L"%d%%", readiness);
	SetDlgItemText(IDC_STATIC_READINESS, readinessText);
}

BOOL CLaunchCheckDlg::AreAllChecksPassed() const
{
	return m_propulsionReady && m_navigationReady &&
		m_communicationReady && m_powerReady && m_weatherReady;
}

void CLaunchCheckDlg::OnCheckChanged()
{
	if (UpdateData(TRUE))
	{
		RefreshReadiness();
	}
}

void CLaunchCheckDlg::OnBnClickedCheckAllPass()
{
	m_propulsionReady = TRUE;
	m_navigationReady = TRUE;
	m_communicationReady = TRUE;
	m_powerReady = TRUE;
	m_weatherReady = TRUE;
	UpdateData(FALSE);
	RefreshReadiness();
}

void CLaunchCheckDlg::OnBnClickedCheckSave()
{
	if (!UpdateData(TRUE))
	{
		AfxMessageBox(L"保存检查失败：无法读取界面数据。", MB_OK | MB_ICONERROR);
		return;
	}

	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"保存检查失败：当前任务不存在。", MB_OK | MB_ICONERROR);
		GetDlgItem(IDC_BUTTON_CHECK_SAVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ENTER_SIMULATION)->EnableWindow(FALSE);
		return;
	}

	if (m_remarks.FindOneOf(L"\t\r\n") >= 0)
	{
		AfxMessageBox(L"保存检查失败：检查备注不能包含制表符、回车或换行。",
			MB_OK | MB_ICONERROR);
		return;
	}

	MissionInfo updatedMission = *pMission;
	updatedMission.status = AreAllChecksPassed()
		? MissionStatus::Ready
		: MissionStatus::Planned;
	CString errorMessage;
	if (!m_pDataManager->UpdateMission(m_missionId, updatedMission, errorMessage))
	{
		CString message;
		message.Format(L"保存检查失败：%s", static_cast<LPCTSTR>(errorMessage));
		AfxMessageBox(message, MB_OK | MB_ICONERROR);
		return;
	}

	CheckResult result;
	result.missionId = m_missionId;
	result.propulsionReady = m_propulsionReady;
	result.navigationReady = m_navigationReady;
	result.communicationReady = m_communicationReady;
	result.powerReady = m_powerReady;
	result.weatherReady = m_weatherReady;
	result.remarks = m_remarks;
	m_pDataManager->SetCheckResult(result);

	AfxMessageBox(L"发射检查已保存到内存，程序关闭时将统一处理磁盘保存。",
		MB_OK | MB_ICONINFORMATION);
}

void CLaunchCheckDlg::OnBnClickedEnterSimulation()
{
	if (!UpdateData(TRUE))
	{
		AfxMessageBox(L"无法读取当前检查状态。", MB_OK | MB_ICONERROR);
		return;
	}

	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"当前任务不存在，无法进入发射模拟。", MB_OK | MB_ICONERROR);
		GetDlgItem(IDC_BUTTON_CHECK_SAVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_ENTER_SIMULATION)->EnableWindow(FALSE);
		return;
	}

	if (!AreAllChecksPassed())
	{
		AfxMessageBox(L"发射检查未达到 100%，不能进入发射模拟。",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	CheckResult* pSavedResult = m_pDataManager->FindCheckResult(m_missionId);
	const BOOL savedChecksPassed = pSavedResult != nullptr &&
		pSavedResult->propulsionReady && pSavedResult->navigationReady &&
		pSavedResult->communicationReady && pSavedResult->powerReady &&
		pSavedResult->weatherReady;
	if (!savedChecksPassed || pMission->status != MissionStatus::Ready)
	{
		AfxMessageBox(L"请先保存全部通过的发射检查，再进入发射模拟。",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	CLaunchSimulationDlg dialog(m_pDataManager, m_missionId, this);
	dialog.DoModal();
}
