#include "pch.h"
#include "LaunchSimulationDlg.h"

#include <algorithm>

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
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_START, &CLaunchSimulationDlg::OnBnClickedLaunchStart)
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_PAUSE, &CLaunchSimulationDlg::OnBnClickedLaunchPause)
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_ABORT, &CLaunchSimulationDlg::OnBnClickedLaunchAbort)
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_RESET, &CLaunchSimulationDlg::OnBnClickedLaunchReset)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CLaunchSimulationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_launchProgress.SetRange(0, 100);
	m_fuelProgress.SetRange(0, 100);
	ResetSimulation();

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
		SetDlgItemText(IDC_STATIC_SIM_MISSION, missionText);
		RefreshButtonStates();
		AfxMessageBox(L"当前任务不存在，无法开始发射模拟。",
			MB_OK | MB_ICONERROR);
		return TRUE;
	}

	CString missionText;
	missionText.Format(L"当前任务：%s（任务 ID：%s）",
		static_cast<LPCTSTR>(pMission->missionName),
		static_cast<LPCTSTR>(pMission->missionId));
	SetDlgItemText(IDC_STATIC_SIM_MISSION, missionText);

	if (pMission->status != MissionStatus::Ready)
	{
		RefreshButtonStates();
		AfxMessageBox(L"当前任务不是已就绪状态，无法开始发射模拟。",
			MB_OK | MB_ICONINFORMATION);
	}

	return TRUE;
}

void CLaunchSimulationDlg::ResetSimulation()
{
	StopSimulationTimer();
	m_launchState = LaunchState::Idle;
	m_stateBeforePause = LaunchState::Idle;
	m_countdown = 10;
	m_progress = 0;
	m_remainingFuel = 100;
	m_altitudeKm = 0.0;
	m_speedKmh = 0.0;
	m_maxHeightKm = 0.0;
	m_simulationRunning = FALSE;
	m_recordCreated = FALSE;
	SetDlgItemText(IDC_EDIT_LAUNCH_LOG, CString());
	RefreshSimulationDisplay();
}

void CLaunchSimulationDlg::RefreshSimulationDisplay()
{
	m_countdown = (std::max)(0, (std::min)(10, m_countdown));
	m_progress = (std::max)(0, (std::min)(100, m_progress));
	m_remainingFuel = (std::max)(0, (std::min)(100, m_remainingFuel));
	m_altitudeKm = (std::max)(0.0, m_altitudeKm);
	m_speedKmh = (std::max)(0.0, m_speedKmh);
	m_maxHeightKm = (std::max)(m_maxHeightKm, m_altitudeKm);

	m_launchProgress.SetPos(m_progress);
	m_fuelProgress.SetPos(m_remainingFuel);

	CString countdownText;
	countdownText.Format(L"倒计时：%d 秒", m_countdown);
	SetDlgItemText(IDC_STATIC_COUNTDOWN, countdownText);

	CString altitudeText;
	altitudeText.Format(L"高度：%.1f km", m_altitudeKm);
	SetDlgItemText(IDC_STATIC_ALTITUDE, altitudeText);

	CString speedText;
	speedText.Format(L"速度：%.1f km/h", m_speedKmh);
	SetDlgItemText(IDC_STATIC_SPEED, speedText);

	CString stageText;
	switch (m_launchState)
	{
	case LaunchState::Countdown:
		stageText = L"当前阶段：倒计时";
		break;
	case LaunchState::Ascending:
		stageText = L"当前阶段：飞行中";
		break;
	case LaunchState::Paused:
		stageText = L"当前阶段：已暂停";
		break;
	case LaunchState::Completed:
		stageText = L"当前阶段：成功完成";
		break;
	case LaunchState::Aborted:
		stageText = L"当前阶段：已中止";
		break;
	case LaunchState::Idle:
	default:
		stageText = L"当前阶段：等待发射";
		break;
	}
	SetDlgItemText(IDC_STATIC_STAGE, stageText);

	RefreshButtonStates();
}

void CLaunchSimulationDlg::RefreshButtonStates()
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	const BOOL validMission = pMission != nullptr;

	BOOL enableStart = FALSE;
	BOOL enablePause = FALSE;
	BOOL enableAbort = FALSE;
	BOOL enableReset = FALSE;
	CString startCaption = L"开始";

	if (validMission)
	{
		switch (m_launchState)
		{
		case LaunchState::Idle:
			enableStart = pMission->status == MissionStatus::Ready;
			enableAbort = pMission->status == MissionStatus::Ready;
			enableReset = pMission->status == MissionStatus::Ready;
			break;
		case LaunchState::Countdown:
		case LaunchState::Ascending:
			enablePause = TRUE;
			enableAbort = TRUE;
			enableReset = TRUE;
			break;
		case LaunchState::Paused:
			startCaption = L"继续";
			enableStart = TRUE;
			enableAbort = TRUE;
			enableReset = TRUE;
			break;
		case LaunchState::Completed:
		case LaunchState::Aborted:
		default:
			break;
		}
	}

	GetDlgItem(IDC_BUTTON_LAUNCH_START)->SetWindowText(startCaption);
	GetDlgItem(IDC_BUTTON_LAUNCH_START)->EnableWindow(enableStart);
	GetDlgItem(IDC_BUTTON_LAUNCH_PAUSE)->EnableWindow(enablePause);
	GetDlgItem(IDC_BUTTON_LAUNCH_ABORT)->EnableWindow(enableAbort);
	GetDlgItem(IDC_BUTTON_LAUNCH_RESET)->EnableWindow(enableReset);
}

void CLaunchSimulationDlg::AppendLog(const CString& message)
{
	CEdit* pLog = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_LAUNCH_LOG));
	if (pLog == nullptr)
	{
		return;
	}

	CString logText;
	pLog->GetWindowText(logText);
	if (!logText.IsEmpty())
	{
		logText += L"\r\n";
	}
	logText += message;
	pLog->SetWindowText(logText);
	pLog->SetSel(logText.GetLength(), logText.GetLength());
	pLog->LineScroll(pLog->GetLineCount());
}

BOOL CLaunchSimulationDlg::StartSimulationTimer()
{
	if (m_timerId != 0)
	{
		return TRUE;
	}

	m_timerId = SetTimer(TIMER_LAUNCH, 1000, nullptr);
	if (m_timerId == 0)
	{
		AfxMessageBox(L"启动发射模拟定时器失败。", MB_OK | MB_ICONERROR);
		m_simulationRunning = FALSE;
		return FALSE;
	}

	m_simulationRunning = TRUE;
	return TRUE;
}

void CLaunchSimulationDlg::StopSimulationTimer()
{
	if (m_timerId != 0)
	{
		KillTimer(m_timerId);
		m_timerId = 0;
	}
	m_simulationRunning = FALSE;
}

BOOL CLaunchSimulationDlg::UpdateMissionStatus(MissionStatus status)
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"任务状态更新失败：当前任务不存在。",
			MB_OK | MB_ICONERROR);
		return FALSE;
	}

	MissionInfo updatedMission = *pMission;
	updatedMission.status = status;
	CString errorMessage;
	if (!m_pDataManager->UpdateMission(m_missionId, updatedMission, errorMessage))
	{
		CString message;
		message.Format(L"任务状态更新失败：%s",
			static_cast<LPCTSTR>(errorMessage));
		AfxMessageBox(message, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

void CLaunchSimulationDlg::CreateLaunchRecord(
	const CString& result, const CString& description)
{
	if (m_recordCreated || m_pDataManager == nullptr)
	{
		return;
	}

	MissionInfo* pMission = m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		return;
	}

	LaunchRecord record;
	record.missionId = pMission->missionId;
	record.missionName = pMission->missionName;
	record.launchTime = pMission->launchTime;
	record.result = result;
	record.maxHeightKm = m_maxHeightKm;
	record.description = description;
	m_pDataManager->AddLaunchRecord(record);
	m_recordCreated = TRUE;
}

void CLaunchSimulationDlg::CompleteLaunch()
{
	if (m_launchState == LaunchState::Completed || m_recordCreated)
	{
		return;
	}

	StopSimulationTimer();
	if (!UpdateMissionStatus(MissionStatus::Completed))
	{
		m_stateBeforePause = LaunchState::Ascending;
		m_launchState = LaunchState::Paused;
		AppendLog(L"任务状态更新失败，发射模拟已暂停。可继续以重试完成处理。");
		RefreshSimulationDisplay();
		return;
	}

	CString description;
	description.Format(L"发射模拟成功完成，最大高度 %.1f km。", m_maxHeightKm);
	CreateLaunchRecord(L"成功", description);
	m_launchState = LaunchState::Completed;
	AppendLog(L"发射模拟成功完成，任务已进入完成状态。");
	RefreshSimulationDisplay();
}

void CLaunchSimulationDlg::AbortLaunch()
{
	if (m_launchState == LaunchState::Completed ||
		m_launchState == LaunchState::Aborted || m_recordCreated)
	{
		return;
	}

	const LaunchState previousState = m_launchState;
	const BOOL wasRunning = m_simulationRunning;
	StopSimulationTimer();
	if (!UpdateMissionStatus(MissionStatus::Aborted))
	{
		m_launchState = previousState;
		if (wasRunning && !StartSimulationTimer())
		{
			m_stateBeforePause = previousState;
			m_launchState = LaunchState::Paused;
		}
		RefreshSimulationDisplay();
		return;
	}

	CString description;
	description.Format(L"发射模拟在进度 %d%% 时中止，最大高度 %.1f km。",
		m_progress, m_maxHeightKm);
	CreateLaunchRecord(L"已中止", description);
	m_launchState = LaunchState::Aborted;
	AppendLog(L"发射模拟已中止，任务已进入中止状态。");
	RefreshSimulationDisplay();
}

void CLaunchSimulationDlg::OnTimer(UINT_PTR timerId)
{
	if (timerId != m_timerId || timerId != TIMER_LAUNCH ||
		!m_simulationRunning)
	{
		CDialogEx::OnTimer(timerId);
		return;
	}

	if (m_launchState == LaunchState::Countdown)
	{
		m_countdown = (std::max)(0, m_countdown - 1);
		if (m_countdown == 0)
		{
			m_launchState = LaunchState::Ascending;
			AppendLog(L"点火成功，运载火箭升空。进入上升阶段。");
		}
		RefreshSimulationDisplay();
		return;
	}

	if (m_launchState == LaunchState::Ascending)
	{
		m_progress = (std::min)(100, m_progress + 5);
		const double ratio = m_progress / 100.0;
		m_altitudeKm = 500.0 * ratio * ratio;
		m_speedKmh = 28000.0 * ratio;
		m_remainingFuel = 100 - m_progress;
		m_maxHeightKm = (std::max)(m_maxHeightKm, m_altitudeKm);

		if (m_progress == 50)
		{
			AppendLog(L"飞行进度达到 50%，进入主要飞行阶段。");
		}

		if (m_progress >= 100)
		{
			CompleteLaunch();
			return;
		}

		RefreshSimulationDisplay();
		return;
	}

	StopSimulationTimer();
	RefreshSimulationDisplay();
}

void CLaunchSimulationDlg::OnBnClickedLaunchStart()
{
	MissionInfo* pMission = m_pDataManager == nullptr
		? nullptr
		: m_pDataManager->FindMission(m_missionId);
	if (pMission == nullptr)
	{
		AfxMessageBox(L"当前任务不存在，无法开始或继续发射模拟。",
			MB_OK | MB_ICONERROR);
		RefreshButtonStates();
		return;
	}

	if (m_launchState == LaunchState::Idle)
	{
		if (pMission->status != MissionStatus::Ready)
		{
			AfxMessageBox(L"当前任务不是已就绪状态，无法开始发射模拟。",
				MB_OK | MB_ICONINFORMATION);
			RefreshButtonStates();
			return;
		}

		if (!StartSimulationTimer())
		{
			return;
		}
		if (!UpdateMissionStatus(MissionStatus::Launching))
		{
			StopSimulationTimer();
			RefreshSimulationDisplay();
			return;
		}

		m_launchState = LaunchState::Countdown;
		AppendLog(L"开始 10 秒发射倒计时。");
		RefreshSimulationDisplay();
		return;
	}

	if (m_launchState == LaunchState::Paused)
	{
		if (pMission->status != MissionStatus::Launching)
		{
			AfxMessageBox(L"当前任务状态不是发射中，无法继续模拟。",
				MB_OK | MB_ICONERROR);
			RefreshButtonStates();
			return;
		}
		if (!StartSimulationTimer())
		{
			return;
		}

		m_launchState = m_stateBeforePause;
		AppendLog(L"发射模拟已继续。");
		RefreshSimulationDisplay();
	}
}

void CLaunchSimulationDlg::OnBnClickedLaunchPause()
{
	if (m_launchState != LaunchState::Countdown &&
		m_launchState != LaunchState::Ascending)
	{
		return;
	}

	m_stateBeforePause = m_launchState;
	StopSimulationTimer();
	m_launchState = LaunchState::Paused;
	AppendLog(L"发射模拟已暂停。");
	RefreshSimulationDisplay();
}

void CLaunchSimulationDlg::OnBnClickedLaunchAbort()
{
	if (m_launchState == LaunchState::Completed ||
		m_launchState == LaunchState::Aborted || m_recordCreated)
	{
		return;
	}

	if (AfxMessageBox(L"确定要中止本次发射模拟吗？",
		MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	AbortLaunch();
}

void CLaunchSimulationDlg::OnBnClickedLaunchReset()
{
	if (m_launchState == LaunchState::Completed ||
		m_launchState == LaunchState::Aborted)
	{
		return;
	}

	if (m_launchState == LaunchState::Idle)
	{
		ResetSimulation();
		return;
	}

	if (AfxMessageBox(
		L"确定要重置本次发射模拟吗？当前模拟进度将丢失。",
		MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	const LaunchState previousState = m_launchState;
	const BOOL wasRunning = m_simulationRunning;
	StopSimulationTimer();
	if (!UpdateMissionStatus(MissionStatus::Ready))
	{
		m_launchState = previousState;
		if (wasRunning && !StartSimulationTimer())
		{
			m_stateBeforePause = previousState;
			m_launchState = LaunchState::Paused;
		}
		RefreshSimulationDisplay();
		return;
	}

	ResetSimulation();
}

void CLaunchSimulationDlg::HandleCloseRequest()
{
	if (m_launchState == LaunchState::Countdown ||
		m_launchState == LaunchState::Ascending ||
		m_launchState == LaunchState::Paused)
	{
		if (AfxMessageBox(
			L"发射模拟尚未结束，关闭窗口将中止本次任务，是否继续？",
			MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}

		AbortLaunch();
		if (m_launchState != LaunchState::Aborted)
		{
			return;
		}
	}

	StopSimulationTimer();
	CDialogEx::OnCancel();
}

void CLaunchSimulationDlg::OnCancel()
{
	HandleCloseRequest();
}

void CLaunchSimulationDlg::OnClose()
{
	HandleCloseRequest();
}

void CLaunchSimulationDlg::OnDestroy()
{
	StopSimulationTimer();
	CDialogEx::OnDestroy();
}
