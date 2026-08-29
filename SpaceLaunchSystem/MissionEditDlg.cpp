#include "pch.h"
#include "MissionEditDlg.h"

IMPLEMENT_DYNAMIC(CMissionEditDlg, CDialogEx)

CMissionEditDlg::CMissionEditDlg(CWnd* pParent)
	: CDialogEx(IDD_MISSION_EDIT_DIALOG, pParent)
{
}

void CMissionEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_MISSION_ID, m_mission.missionId);
	DDX_Text(pDX, IDC_EDIT_MISSION_NAME, m_mission.missionName);
	DDX_Text(pDX, IDC_EDIT_ROCKET_NAME, m_mission.rocketName);
	DDX_Text(pDX, IDC_EDIT_PAYLOAD_NAME, m_mission.payloadName);
	DDX_CBString(pDX, IDC_COMBO_DESTINATION, m_mission.destination);
	DDX_Text(pDX, IDC_EDIT_LAUNCH_TIME, m_mission.launchTime);
}

BEGIN_MESSAGE_MAP(CMissionEditDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CMissionEditDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CComboBox* pDestinationCombo =
		static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_DESTINATION));
	if (pDestinationCombo != nullptr)
	{
		pDestinationCombo->AddString(L"近地轨道");
		pDestinationCombo->AddString(L"月球");
		pDestinationCombo->AddString(L"火星");
		pDestinationCombo->AddString(L"空间站");
	}

	UpdateData(FALSE);
	return TRUE;
}
