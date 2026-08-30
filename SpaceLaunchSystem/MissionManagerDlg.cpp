#include "pch.h"
#include "MissionManagerDlg.h"
#include "MissionEditDlg.h"

namespace
{
void SelectMissionRow(CListCtrl& missionList, int rowIndex)
{
	if (rowIndex < 0 || rowIndex >= missionList.GetItemCount())
	{
		return;
	}

	missionList.SetItemState(rowIndex, LVIS_SELECTED | LVIS_FOCUSED,
		LVIS_SELECTED | LVIS_FOCUSED);
	missionList.SetSelectionMark(rowIndex);
	missionList.EnsureVisible(rowIndex, FALSE);
}
}

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
	ON_BN_CLICKED(IDC_BUTTON_MISSION_ADD, &CMissionManagerDlg::OnBnClickedMissionAdd)
	ON_BN_CLICKED(IDC_BUTTON_MISSION_MODIFY, &CMissionManagerDlg::OnBnClickedMissionModify)
	ON_BN_CLICKED(IDC_BUTTON_MISSION_DELETE, &CMissionManagerDlg::OnBnClickedMissionDelete)
	ON_BN_CLICKED(IDC_BUTTON_MISSION_SELECT, &CMissionManagerDlg::OnBnClickedMissionSelect)
	ON_BN_CLICKED(IDC_BUTTON_MISSION_SAVE, &CMissionManagerDlg::OnBnClickedMissionSave)
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

	RefreshMissionList();
	return TRUE;
}

void CMissionManagerDlg::RefreshMissionList()
{
	m_missionList.DeleteAllItems();
	if (m_pDataManager == nullptr)
	{
		return;
	}

	const std::vector<MissionInfo>& missions = m_pDataManager->GetMissions();
	for (size_t index = 0; index < missions.size(); ++index)
	{
		const MissionInfo& mission = missions[index];
		const int item = m_missionList.InsertItem(
			static_cast<int>(index), mission.missionId);
		m_missionList.SetItemText(item, 1, mission.missionName);
		m_missionList.SetItemText(item, 2, mission.rocketName);
		m_missionList.SetItemText(item, 3, mission.payloadName);
		m_missionList.SetItemText(item, 4, mission.destination);
		m_missionList.SetItemText(item, 5, mission.launchTime);
		m_missionList.SetItemText(item, 6, MissionStatusToString(mission.status));
	}
}

int CMissionManagerDlg::GetSelectedMissionIndex() const
{
	const int selectedIndex = m_missionList.GetNextItem(-1, LVNI_SELECTED);
	if (selectedIndex < 0 || m_pDataManager == nullptr ||
		static_cast<size_t>(selectedIndex) >= m_pDataManager->GetMissions().size())
	{
		return -1;
	}
	return selectedIndex;
}

void CMissionManagerDlg::OnBnClickedMissionAdd()
{
	if (m_pDataManager == nullptr)
	{
		return;
	}

	CMissionEditDlg dialog(this);
	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	CString errorMessage;
	if (!m_pDataManager->AddMission(dialog.m_mission, errorMessage))
	{
		MessageBox(errorMessage, L"添加任务", MB_OK | MB_ICONWARNING);
		return;
	}
	RefreshMissionList();
	SelectMissionRow(m_missionList,
		static_cast<int>(m_pDataManager->GetMissions().size()) - 1);
}

void CMissionManagerDlg::OnBnClickedMissionModify()
{
	const int selectedIndex = GetSelectedMissionIndex();
	if (selectedIndex < 0)
	{
		MessageBox(L"请先选择要修改的任务。", L"修改任务",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	const MissionInfo originalMission =
		m_pDataManager->GetMissions()[static_cast<size_t>(selectedIndex)];
	CMissionEditDlg dialog(this);
	dialog.m_mission = originalMission;
	dialog.m_isModifyMode = TRUE;
	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	CString errorMessage;
	if (!m_pDataManager->UpdateMission(originalMission.missionId,
		dialog.m_mission, errorMessage))
	{
		MessageBox(errorMessage, L"修改任务", MB_OK | MB_ICONWARNING);
		return;
	}
	RefreshMissionList();
	SelectMissionRow(m_missionList, selectedIndex);
}

void CMissionManagerDlg::OnBnClickedMissionDelete()
{
	const int selectedIndex = GetSelectedMissionIndex();
	if (selectedIndex < 0)
	{
		MessageBox(L"请先选择要删除的任务。", L"删除任务",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	const CString missionId =
		m_pDataManager->GetMissions()[static_cast<size_t>(selectedIndex)].missionId;
	CString message;
	message.Format(L"确定删除任务 %s 吗？", missionId.GetString());
	if (MessageBox(message, L"删除确认", MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	if (!m_pDataManager->DeleteMission(missionId))
	{
		MessageBox(L"任务删除失败。", L"删除任务", MB_OK | MB_ICONERROR);
		return;
	}
	if (m_pCurrentMissionId != nullptr && *m_pCurrentMissionId == missionId)
	{
		m_pCurrentMissionId->Empty();
	}
	RefreshMissionList();
	const int remainingCount = m_missionList.GetItemCount();
	SelectMissionRow(m_missionList,
		remainingCount > 0 && selectedIndex < remainingCount
			? selectedIndex : remainingCount - 1);
}

void CMissionManagerDlg::OnBnClickedMissionSelect()
{
	const int selectedIndex = GetSelectedMissionIndex();
	if (selectedIndex < 0)
	{
		MessageBox(L"请先选择一个任务。", L"当前任务",
			MB_OK | MB_ICONINFORMATION);
		return;
	}
	if (m_pCurrentMissionId == nullptr)
	{
		return;
	}

	*m_pCurrentMissionId =
		m_pDataManager->GetMissions()[static_cast<size_t>(selectedIndex)].missionId;
	RefreshMissionList();
	SelectMissionRow(m_missionList, selectedIndex);
	MessageBox(L"已设置当前任务。", L"当前任务", MB_OK | MB_ICONINFORMATION);
}

void CMissionManagerDlg::OnBnClickedMissionSave()
{
	if (m_pDataManager == nullptr)
	{
		return;
	}
	if (!m_pDataManager->SaveAll())
	{
		MessageBox(L"保存失败，请检查数据目录的写入权限。", L"保存任务",
			MB_OK | MB_ICONERROR);
		return;
	}
	MessageBox(L"任务数据已保存。", L"保存任务", MB_OK | MB_ICONINFORMATION);
}
