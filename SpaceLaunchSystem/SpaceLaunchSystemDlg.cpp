
// SpaceLaunchSystemDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SpaceLaunchSystem.h"
#include "SpaceLaunchSystemDlg.h"
#include "afxdialogex.h"
#include "MissionManagerDlg.h"
#include "LaunchCheckDlg.h"
#include "LaunchSimulationDlg.h"
#include "HistoryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
BOOL AreSavedChecksPassed(CDataManager& dataManager, const CString& missionId)
{
	CheckResult* pResult = dataManager.FindCheckResult(missionId);
	return pResult != nullptr && pResult->propulsionReady &&
		pResult->navigationReady && pResult->communicationReady &&
		pResult->powerReady && pResult->weatherReady;
}
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSpaceLaunchSystemDlg 对话框



CSpaceLaunchSystemDlg::CSpaceLaunchSystemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SPACELAUNCHSYSTEM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSpaceLaunchSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSpaceLaunchSystemDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_MISSION_MANAGER, &CSpaceLaunchSystemDlg::OnBnClickedMissionManager)
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_CHECK, &CSpaceLaunchSystemDlg::OnBnClickedLaunchCheck)
	ON_BN_CLICKED(IDC_BUTTON_LAUNCH_SIMULATION, &CSpaceLaunchSystemDlg::OnBnClickedLaunchSimulation)
	ON_BN_CLICKED(IDC_BUTTON_HISTORY, &CSpaceLaunchSystemDlg::OnBnClickedHistory)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CSpaceLaunchSystemDlg 消息处理程序

BOOL CSpaceLaunchSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	if (!m_dataManager.Initialize())
	{
		AfxMessageBox(L"数据初始化失败，部分数据可能无法加载，但主窗口仍可使用。",
			MB_OK | MB_ICONWARNING);
	}

	RefreshMainSummary();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSpaceLaunchSystemDlg::RefreshMainSummary()
{
	MissionInfo* pMission = nullptr;
	if (!m_currentMissionId.IsEmpty())
	{
		pMission = m_dataManager.FindMission(m_currentMissionId);
	}

	if (pMission == nullptr)
	{
		m_currentMissionId.Empty();
		SetDlgItemText(IDC_STATIC_CURRENT_MISSION, L"当前任务：未选择");
		SetDlgItemText(IDC_STATIC_SYSTEM_STATUS, L"系统状态：等待选择任务");
		GetDlgItem(IDC_BUTTON_LAUNCH_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_LAUNCH_SIMULATION)->EnableWindow(FALSE);
		return;
	}

	CString missionSummary;
	missionSummary.Format(L"当前任务：%s（任务 ID：%s）",
		static_cast<LPCTSTR>(pMission->missionName),
		static_cast<LPCTSTR>(pMission->missionId));
	SetDlgItemText(IDC_STATIC_CURRENT_MISSION, missionSummary);

	CString systemStatus;
	systemStatus.Format(L"系统状态：%s",
		static_cast<LPCTSTR>(MissionStatusToString(pMission->status)));
	SetDlgItemText(IDC_STATIC_SYSTEM_STATUS, systemStatus);

	GetDlgItem(IDC_BUTTON_LAUNCH_CHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_LAUNCH_SIMULATION)->EnableWindow(
		pMission->status == MissionStatus::Ready &&
		AreSavedChecksPassed(m_dataManager, pMission->missionId));
}

void CSpaceLaunchSystemDlg::OnBnClickedMissionManager()
{
	CMissionManagerDlg dialog(&m_dataManager, &m_currentMissionId, this);
	dialog.DoModal();
	RefreshMainSummary();
}

void CSpaceLaunchSystemDlg::OnBnClickedLaunchCheck()
{
	if (m_currentMissionId.IsEmpty() ||
		m_dataManager.FindMission(m_currentMissionId) == nullptr)
	{
		RefreshMainSummary();
		AfxMessageBox(L"请先选择当前任务。", MB_OK | MB_ICONINFORMATION);
		return;
	}

	CLaunchCheckDlg dialog(&m_dataManager, m_currentMissionId, this);
	dialog.DoModal();
	RefreshMainSummary();
}

void CSpaceLaunchSystemDlg::OnBnClickedLaunchSimulation()
{
	MissionInfo* pMission = nullptr;
	if (!m_currentMissionId.IsEmpty())
	{
		pMission = m_dataManager.FindMission(m_currentMissionId);
	}

	if (pMission == nullptr)
	{
		RefreshMainSummary();
		AfxMessageBox(L"请先选择当前任务。", MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (pMission->status == MissionStatus::Launching)
	{
		AfxMessageBox(L"任务已经处于发射状态，不能重复进入模拟。",
			MB_OK | MB_ICONINFORMATION);
		RefreshMainSummary();
		return;
	}
	if (pMission->status == MissionStatus::Completed)
	{
		AfxMessageBox(L"任务已完成，不能再次发射。",
			MB_OK | MB_ICONINFORMATION);
		RefreshMainSummary();
		return;
	}
	if (pMission->status == MissionStatus::Aborted)
	{
		AfxMessageBox(L"任务已中止，不能再次发射。",
			MB_OK | MB_ICONINFORMATION);
		RefreshMainSummary();
		return;
	}
	if (pMission->status != MissionStatus::Ready ||
		!AreSavedChecksPassed(m_dataManager, pMission->missionId))
	{
		AfxMessageBox(L"请先完成并保存全部发射检查。", MB_OK | MB_ICONINFORMATION);
		RefreshMainSummary();
		return;
	}

	CLaunchSimulationDlg dialog(&m_dataManager, m_currentMissionId, this);
	dialog.DoModal();
	RefreshMainSummary();
}

void CSpaceLaunchSystemDlg::OnBnClickedHistory()
{
	CHistoryDlg dialog(&m_dataManager, this);
	dialog.DoModal();
}

void CSpaceLaunchSystemDlg::OnClose()
{
	HandleCloseRequest();
}

void CSpaceLaunchSystemDlg::OnCancel()
{
	HandleCloseRequest();
}

void CSpaceLaunchSystemDlg::HandleCloseRequest()
{
	if (!m_dataManager.ConfirmSaveBeforeClose(this))
	{
		return;
	}

	CDialogEx::OnCancel();
}

void CSpaceLaunchSystemDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSpaceLaunchSystemDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSpaceLaunchSystemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

