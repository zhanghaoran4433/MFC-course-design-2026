#include "pch.h"
#include "HistoryDlg.h"

IMPLEMENT_DYNAMIC(CHistoryDlg, CDialogEx)

CHistoryDlg::CHistoryDlg(CDataManager* pDataManager, CWnd* pParent)
	: CDialogEx(IDD_HISTORY_DIALOG, pParent)
	, m_pDataManager(pDataManager)
{
}

void CHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HISTORY, m_historyList);
	DDX_Text(pDX, IDC_EDIT_HISTORY_KEYWORD, m_keyword);
	DDX_CBIndex(pDX, IDC_COMBO_HISTORY_RESULT, m_resultFilter);
}

BEGIN_MESSAGE_MAP(CHistoryDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CHistoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_historyList.SetExtendedStyle(
		m_historyList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_historyList.InsertColumn(0, L"任务 ID", LVCFMT_LEFT, 75);
	m_historyList.InsertColumn(1, L"任务名称", LVCFMT_LEFT, 120);
	m_historyList.InsertColumn(2, L"时间", LVCFMT_LEFT, 115);
	m_historyList.InsertColumn(3, L"结果", LVCFMT_LEFT, 70);
	m_historyList.InsertColumn(4, L"最大高度", LVCFMT_LEFT, 80);

	CComboBox* pResultCombo = static_cast<CComboBox*>(
		GetDlgItem(IDC_COMBO_HISTORY_RESULT));
	if (pResultCombo != nullptr)
	{
		pResultCombo->AddString(L"全部");
		pResultCombo->AddString(L"成功");
		pResultCombo->AddString(L"已中止");
		pResultCombo->SetCurSel(m_resultFilter);
	}

	return TRUE;
}
