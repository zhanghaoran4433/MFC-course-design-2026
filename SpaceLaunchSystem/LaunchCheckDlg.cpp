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

	RestoreSavedCheckDisplay();
	RefreshControlAvailability();

	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"当前任务不存在，无法保存发射检查或进入发射模拟。",
			MB_OK | MB_ICONERROR);
	}

	return TRUE;
}

BOOL CLaunchCheckDlg::IsCheckEditingAllowed(const MissionInfo* pMission) const
{
	return pMission != nullptr &&
		(pMission->status == MissionStatus::Planned ||
			pMission->status == MissionStatus::Ready);
}

void CLaunchCheckDlg::LoadSavedCheckResult()
{
	m_propulsionReady = FALSE;
	m_navigationReady = FALSE;
	m_communicationReady = FALSE;
	m_powerReady = FALSE;
	m_weatherReady = FALSE;
	m_remarks.Empty();

	CheckResult* pCheckResult = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindCheckResult(m_missionId);
	if (pCheckResult == nullptr)
	{
		return;
	}

	m_propulsionReady = pCheckResult->propulsionReady;
	m_navigationReady = pCheckResult->navigationReady;
	m_communicationReady = pCheckResult->communicationReady;
	m_powerReady = pCheckResult->powerReady;
	m_weatherReady = pCheckResult->weatherReady;
	m_remarks = pCheckResult->remarks;
}

void CLaunchCheckDlg::RefreshMissionText(const MissionInfo* pMission)
{
	CString missionText;
	if (pMission == nullptr)
	{
		if (m_missionId.IsEmpty())
		{
			missionText = L"当前任务：未提供任务 ID";
		}
		else
		{
			missionText.Format(L"当前任务：不存在（任务 ID：%s）",
				static_cast<LPCTSTR>(m_missionId));
		}
	}
	else
	{
		const BOOL readOnly = !IsCheckEditingAllowed(pMission);
		missionText.Format(readOnly
			? L"当前任务：%s（任务 ID：%s，状态：%s，发射检查只读）"
			: L"当前任务：%s（任务 ID：%s，状态：%s）",
			static_cast<LPCTSTR>(pMission->missionName),
			static_cast<LPCTSTR>(pMission->missionId),
			static_cast<LPCTSTR>(MissionStatusToString(pMission->status)));
	}
	SetDlgItemText(IDC_STATIC_CHECK_MISSION, missionText);
}

void CLaunchCheckDlg::RefreshControlAvailability()
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	const BOOL canEdit = IsCheckEditingAllowed(pMission);

	const UINT checkControlIds[] = {
		IDC_CHECK_PROPULSION,
		IDC_CHECK_NAVIGATION,
		IDC_CHECK_COMMUNICATION,
		IDC_CHECK_POWER,
		IDC_CHECK_WEATHER
	};
	for (UINT controlId : checkControlIds)
	{
		GetDlgItem(controlId)->EnableWindow(canEdit);
	}

	CEdit* pRemarks = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_CHECK_REMARKS));
	pRemarks->SetReadOnly(!canEdit);
	pRemarks->EnableWindow(pMission != nullptr);
	GetDlgItem(IDC_BUTTON_CHECK_ALL_PASS)->EnableWindow(canEdit);
	GetDlgItem(IDC_BUTTON_CHECK_SAVE)->EnableWindow(canEdit);

	CheckResult* pSavedResult = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindCheckResult(m_missionId);
	const BOOL savedChecksPassed = pSavedResult != nullptr &&
		pSavedResult->propulsionReady && pSavedResult->navigationReady &&
		pSavedResult->communicationReady && pSavedResult->powerReady &&
		pSavedResult->weatherReady;
	const BOOL canEnterSimulation = pMission != nullptr &&
		pMission->status == MissionStatus::Ready &&
		AreAllChecksPassed() && savedChecksPassed;
	GetDlgItem(IDC_BUTTON_ENTER_SIMULATION)->EnableWindow(canEnterSimulation);

	RefreshMissionText(pMission);
}

void CLaunchCheckDlg::RestoreSavedCheckDisplay()
{
	LoadSavedCheckResult();
	UpdateData(FALSE);
	RefreshReadiness();
}

void CLaunchCheckDlg::ShowSaveBlockedMessage(MissionStatus status) const
{
	switch (status)
	{
	case MissionStatus::Launching:
		AfxMessageBox(L"任务正在发射中，不能修改或重新保存发射检查。",
			MB_OK | MB_ICONINFORMATION);
		break;
	case MissionStatus::Completed:
		AfxMessageBox(L"任务已完成，不能修改检查结果或再次发射。",
			MB_OK | MB_ICONINFORMATION);
		break;
	case MissionStatus::Aborted:
		AfxMessageBox(L"任务已中止，不能修改检查结果或再次发射。",
			MB_OK | MB_ICONINFORMATION);
		break;
	default:
		break;
	}
}

void CLaunchCheckDlg::ShowSimulationBlockedMessage(MissionStatus status) const
{
	switch (status)
	{
	case MissionStatus::Launching:
		AfxMessageBox(L"任务已经处于发射状态，不能重复进入模拟。",
			MB_OK | MB_ICONINFORMATION);
		break;
	case MissionStatus::Completed:
		AfxMessageBox(L"任务已完成，不能再次发射。",
			MB_OK | MB_ICONINFORMATION);
		break;
	case MissionStatus::Aborted:
		AfxMessageBox(L"任务已中止，不能再次发射。",
			MB_OK | MB_ICONINFORMATION);
		break;
	default:
		break;
	}
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
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (!IsCheckEditingAllowed(pMission))
	{
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
		return;
	}

	if (UpdateData(TRUE))
	{
		RefreshReadiness();
		RefreshControlAvailability();
	}
}

void CLaunchCheckDlg::OnBnClickedCheckAllPass()
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (!IsCheckEditingAllowed(pMission))
	{
		RefreshControlAvailability();
		return;
	}

	m_propulsionReady = TRUE;
	m_navigationReady = TRUE;
	m_communicationReady = TRUE;
	m_powerReady = TRUE;
	m_weatherReady = TRUE;
	UpdateData(FALSE);
	RefreshReadiness();
	RefreshControlAvailability();
}

void CLaunchCheckDlg::OnBnClickedCheckSave()
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"保存检查失败：当前任务不存在。", MB_OK | MB_ICONERROR);
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
		return;
	}

	if (!IsCheckEditingAllowed(pMission))
	{
		ShowSaveBlockedMessage(pMission->status);
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
		return;
	}

	if (!UpdateData(TRUE))
	{
		AfxMessageBox(L"保存检查失败：无法读取界面数据。", MB_OK | MB_ICONERROR);
		return;
	}

	if (m_remarks.FindOneOf(L"\t\r\n") >= 0)
	{
		AfxMessageBox(L"保存检查失败：检查备注不能包含制表符、回车或换行。",
			MB_OK | MB_ICONERROR);
		return;
	}

	pMission = m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"保存检查失败：当前任务不存在。", MB_OK | MB_ICONERROR);
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
		return;
	}
	if (!IsCheckEditingAllowed(pMission))
	{
		ShowSaveBlockedMessage(pMission->status);
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
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
	RefreshControlAvailability();

	AfxMessageBox(L"发射检查已保存到内存，程序关闭时将统一处理磁盘保存。",
		MB_OK | MB_ICONINFORMATION);
}

void CLaunchCheckDlg::OnBnClickedEnterSimulation()
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"当前任务不存在，无法进入发射模拟。", MB_OK | MB_ICONERROR);
		RefreshControlAvailability();
		return;
	}

	if (!IsCheckEditingAllowed(pMission))
	{
		ShowSimulationBlockedMessage(pMission->status);
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
		return;
	}

	if (pMission->status != MissionStatus::Ready)
	{
		AfxMessageBox(L"请先保存全部通过的发射检查，再进入发射模拟。",
			MB_OK | MB_ICONINFORMATION);
		RefreshControlAvailability();
		return;
	}

	if (!UpdateData(TRUE))
	{
		AfxMessageBox(L"无法读取当前检查状态。", MB_OK | MB_ICONERROR);
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
	if (!savedChecksPassed)
	{
		AfxMessageBox(L"请先保存全部通过的发射检查，再进入发射模拟。",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	pMission = m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"当前任务不存在，无法进入发射模拟。", MB_OK | MB_ICONERROR);
		RefreshControlAvailability();
		return;
	}
	if (pMission->status != MissionStatus::Ready)
	{
		if (!IsCheckEditingAllowed(pMission))
		{
			ShowSimulationBlockedMessage(pMission->status);
		}
		else
		{
			AfxMessageBox(L"请先保存全部通过的发射检查，再进入发射模拟。",
				MB_OK | MB_ICONINFORMATION);
		}
		RestoreSavedCheckDisplay();
		RefreshControlAvailability();
		return;
	}

	CLaunchSimulationDlg dialog(m_pDataManager, m_missionId, this);
	dialog.DoModal();

	RestoreSavedCheckDisplay();
	RefreshControlAvailability();
}
