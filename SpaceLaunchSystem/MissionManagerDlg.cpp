#include "pch.h"
#include "MissionManagerDlg.h"

IMPLEMENT_DYNAMIC(CMissionManagerDlg, CDialogEx)

CMissionManagerDlg::CMissionManagerDlg(CDataManager* pDataManager,
	CString* pCurrentMissionId,
	CWnd* pParent)
	: CDialogEx(IDD_MISSION_MANAGER_DIALOG, pParent)
	, m_pDataManager(pDataManager)
	, m_pCurrentMissionId(pCurrentMissionId)
{
}

void CMissionManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MISSIONS, m_missionList);
}

BEGIN_MESSAGE_MAP(CMissionManagerDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MISSIONS, &CMissionManagerDlg::OnLvnItemchangedListMissions)
END_MESSAGE_MAP()

BOOL CMissionManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_missionList.SetExtendedStyle(
		m_missionList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_missionList.InsertColumn(0, L"任务 ID", LVCFMT_LEFT, 70);
	m_missionList.InsertColumn(1, L"任务名称", LVCFMT_LEFT, 100);
	m_missionList.InsertColumn(2, L"运载火箭", LVCFMT_LEFT, 90);
	m_missionList.InsertColumn(3, L"载荷", LVCFMT_LEFT, 90);
	m_missionList.InsertColumn(4, L"目的地", LVCFMT_LEFT, 70);
	m_missionList.InsertColumn(5, L"计划时间", LVCFMT_LEFT, 105);
	m_missionList.InsertColumn(6, L"状态", LVCFMT_LEFT, 65);

	return TRUE;
}

void CMissionManagerDlg::OnLvnItemchangedListMissions(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}
