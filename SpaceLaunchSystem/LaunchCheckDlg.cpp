#include "pch.h"
#include "LaunchCheckDlg.h"

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
	ON_EN_CHANGE(IDC_EDIT_CHECK_REMARKS, &CLaunchCheckDlg::OnEnChangeEditCheckRemarks)
END_MESSAGE_MAP()

BOOL CLaunchCheckDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_readinessProgress.SetRange(0, 100);
	m_readinessProgress.SetPos(0);

	CString missionText;
	missionText.Format(L"当前任务：%s", static_cast<LPCWSTR>(m_missionId));
	SetDlgItemText(IDC_STATIC_CHECK_MISSION, missionText);

	return TRUE;
}

void CLaunchCheckDlg::OnEnChangeEditCheckRemarks()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
