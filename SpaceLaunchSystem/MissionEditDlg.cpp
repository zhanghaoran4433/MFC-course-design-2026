#include "pch.h"
#include "MissionEditDlg.h"

namespace
{
BOOL ContainsInvalidFieldCharacter(const CString& value)
{
	return value.FindOneOf(_T("\t\r\n")) >= 0;
}

BOOL IsLaunchTimeFormatValid(const CString& value)
{
	if (value.GetLength() != 16 || value[4] != _T('-') ||
		value[7] != _T('-') || value[10] != _T(' ') ||
		value[13] != _T(':'))
	{
		return FALSE;
	}

	for (int index = 0; index < value.GetLength(); ++index)
	{
		if (index == 4 || index == 7 || index == 10 || index == 13)
		{
			continue;
		}
		if (value[index] < _T('0') || value[index] > _T('9'))
		{
			return FALSE;
		}
	}
	return TRUE;
}
}

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
	if (m_isModifyMode)
	{
		CEdit* pMissionIdEdit =
			static_cast<CEdit*>(GetDlgItem(IDC_EDIT_MISSION_ID));
		if (pMissionIdEdit != nullptr)
		{
			pMissionIdEdit->SetReadOnly(TRUE);
		}
	}
	return TRUE;
}

void CMissionEditDlg::OnOK()
{
	if (!UpdateData(TRUE))
	{
		return;
	}

	m_mission.missionId.Trim(_T(' '));
	m_mission.missionName.Trim(_T(' '));
	m_mission.rocketName.Trim(_T(' '));
	m_mission.payloadName.Trim(_T(' '));
	m_mission.destination.Trim(_T(' '));
	m_mission.launchTime.Trim(_T(' '));

	CString errorMessage;
	if (!ValidateInput(errorMessage))
	{
		MessageBox(errorMessage, L"输入校验", MB_OK | MB_ICONWARNING);
		return;
	}

	CDialogEx::OnOK();
}

BOOL CMissionEditDlg::ValidateInput(CString& errorMessage) const
{
	errorMessage.Empty();
	if (m_mission.missionId.IsEmpty() || m_mission.missionName.IsEmpty() ||
		m_mission.rocketName.IsEmpty() || m_mission.payloadName.IsEmpty() ||
		m_mission.destination.IsEmpty() || m_mission.launchTime.IsEmpty())
	{
		errorMessage = L"所有任务字段均不能为空。";
		return FALSE;
	}

	if (ContainsInvalidFieldCharacter(m_mission.missionId) ||
		ContainsInvalidFieldCharacter(m_mission.missionName) ||
		ContainsInvalidFieldCharacter(m_mission.rocketName) ||
		ContainsInvalidFieldCharacter(m_mission.payloadName) ||
		ContainsInvalidFieldCharacter(m_mission.destination) ||
		ContainsInvalidFieldCharacter(m_mission.launchTime))
	{
		errorMessage = L"字段不能包含制表符、回车或换行。";
		return FALSE;
	}

	if (!IsLaunchTimeFormatValid(m_mission.launchTime))
	{
		errorMessage = L"计划时间格式必须为 YYYY-MM-DD HH:MM。";
		return FALSE;
	}
	return TRUE;
}
