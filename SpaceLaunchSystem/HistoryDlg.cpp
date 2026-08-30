#include "pch.h"
#include "HistoryDlg.h"

#include <algorithm>
#include <string>

namespace
{
constexpr int kResultFilterAll = 0;
constexpr int kResultFilterSuccess = 1;
constexpr int kResultFilterAborted = 2;

const CString kSuccessResult = L"成功";
const CString kAbortedResult = L"已中止";
const CString kMissingMissionName = L"任务不存在";

CString GetMissionName(CDataManager* pDataManager, const CString& missionId)
{
	const MissionInfo* pMission = pDataManager == nullptr
		? nullptr
		: pDataManager->FindMission(missionId);
	return pMission == nullptr ? kMissingMissionName : pMission->missionName;
}

BOOL ContainsNoCase(const CString& text, const CString& keyword)
{
	if (keyword.IsEmpty())
	{
		return TRUE;
	}

	CString normalizedText(text);
	CString normalizedKeyword(keyword);
	normalizedText.MakeLower();
	normalizedKeyword.MakeLower();
	return normalizedText.Find(normalizedKeyword) >= 0;
}

BOOL MatchesResultFilter(const LaunchRecord& record, int resultFilter)
{
	switch (resultFilter)
	{
	case kResultFilterSuccess:
		return record.result == kSuccessResult;
	case kResultFilterAborted:
		return record.result == kAbortedResult;
	case kResultFilterAll:
	default:
		return TRUE;
	}
}

BOOL IsValidTsvField(const CString& value)
{
	return value.FindOneOf(L"\t\r\n") < 0;
}

BOOL WriteUtf8Tsv(const CString& path, const CString& text)
{
	const int utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
		text.GetString(), text.GetLength(), nullptr, 0, nullptr, nullptr);
	if (utf8Length == 0 && !text.IsEmpty())
	{
		return FALSE;
	}

	std::string bytes(static_cast<size_t>(utf8Length), '\0');
	if (utf8Length > 0 && WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
		text.GetString(), text.GetLength(), &bytes[0], utf8Length,
		nullptr, nullptr) != utf8Length)
	{
		return FALSE;
	}

	try
	{
		CFile file(path, CFile::modeCreate | CFile::modeWrite |
			CFile::shareExclusive | CFile::typeBinary);
		const BYTE utf8Bom[] = { 0xEF, 0xBB, 0xBF };
		file.Write(utf8Bom, sizeof(utf8Bom));
		if (!bytes.empty())
		{
			file.Write(bytes.data(), static_cast<UINT>(bytes.size()));
		}
		file.Flush();
		file.Close();
		return TRUE;
	}
	catch (CFileException* pException)
	{
		pException->Delete();
		return FALSE;
	}
}
}

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
	ON_BN_CLICKED(IDC_BUTTON_HISTORY_SEARCH, &CHistoryDlg::OnBnClickedHistorySearch)
	ON_BN_CLICKED(IDC_BUTTON_HISTORY_DETAIL, &CHistoryDlg::OnBnClickedHistoryDetail)
	ON_BN_CLICKED(IDC_BUTTON_HISTORY_DELETE, &CHistoryDlg::OnBnClickedHistoryDelete)
	ON_BN_CLICKED(IDC_BUTTON_HISTORY_EXPORT, &CHistoryDlg::OnBnClickedHistoryExport)
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
		m_resultFilter = kResultFilterAll;
		pResultCombo->SetCurSel(m_resultFilter);
	}

	m_keyword.Empty();
	SetDlgItemText(IDC_EDIT_HISTORY_DETAIL, CString());
	RefreshHistoryList();

	return TRUE;
}

void CHistoryDlg::RefreshHistoryList()
{
	m_historyList.DeleteAllItems();
	m_visibleRecordIndexes.clear();
	SetDlgItemText(IDC_EDIT_HISTORY_DETAIL, CString());

	if (m_pDataManager == nullptr)
	{
		return;
	}

	const std::vector<LaunchRecord>& records = m_pDataManager->GetRecords();
	for (size_t recordIndex = 0; recordIndex < records.size(); ++recordIndex)
	{
		const LaunchRecord& record = records[recordIndex];
		const CString missionName = GetMissionName(m_pDataManager, record.missionId);
		if (!MatchesResultFilter(record, m_resultFilter) ||
			(!ContainsNoCase(record.missionId, m_keyword) &&
				!ContainsNoCase(missionName, m_keyword)))
		{
			continue;
		}

		const int itemIndex = m_historyList.InsertItem(
			m_historyList.GetItemCount(), record.missionId);
		if (itemIndex < 0)
		{
			continue;
		}

		m_historyList.SetItemText(itemIndex, 1, missionName);
		m_historyList.SetItemText(itemIndex, 2, record.launchTime);
		m_historyList.SetItemText(itemIndex, 3, record.result);
		CString heightText;
		heightText.Format(L"%.1f km", record.maxHeightKm);
		m_historyList.SetItemText(itemIndex, 4, heightText);
		m_visibleRecordIndexes.push_back(recordIndex);
	}
}

int CHistoryDlg::GetSelectedVisibleIndex() const
{
	POSITION position = m_historyList.GetFirstSelectedItemPosition();
	if (position == nullptr)
	{
		return -1;
	}

	const int visibleIndex = m_historyList.GetNextSelectedItem(position);
	if (visibleIndex < 0 ||
		static_cast<size_t>(visibleIndex) >= m_visibleRecordIndexes.size())
	{
		return -1;
	}
	return visibleIndex;
}

void CHistoryDlg::OnBnClickedHistorySearch()
{
	if (!UpdateData(TRUE))
	{
		return;
	}

	m_keyword.Trim();
	if (m_resultFilter < kResultFilterAll ||
		m_resultFilter > kResultFilterAborted)
	{
		m_resultFilter = kResultFilterAll;
	}
	UpdateData(FALSE);
	RefreshHistoryList();
}

void CHistoryDlg::OnBnClickedHistoryDetail()
{
	const int visibleIndex = GetSelectedVisibleIndex();
	if (visibleIndex < 0)
	{
		AfxMessageBox(L"请先选择一条历史记录。", MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (m_pDataManager == nullptr)
	{
		AfxMessageBox(L"历史记录数据不可用。", MB_OK | MB_ICONERROR);
		return;
	}

	const size_t recordIndex = m_visibleRecordIndexes[visibleIndex];
	const std::vector<LaunchRecord>& records = m_pDataManager->GetRecords();
	if (recordIndex >= records.size())
	{
		AfxMessageBox(L"所选历史记录已失效，请重新查询。", MB_OK | MB_ICONERROR);
		RefreshHistoryList();
		return;
	}

	const LaunchRecord& record = records[recordIndex];
	const CString missionName = GetMissionName(m_pDataManager, record.missionId);
	CString detailText;
	detailText.Format(
		L"任务 ID：%s\r\n"
		L"任务名称：%s\r\n"
		L"时间：%s\r\n"
		L"结果：%s\r\n"
		L"最大高度：%.1f km\r\n"
		L"说明：%s",
		static_cast<LPCTSTR>(record.missionId),
		static_cast<LPCTSTR>(missionName),
		static_cast<LPCTSTR>(record.launchTime),
		static_cast<LPCTSTR>(record.result),
		record.maxHeightKm,
		static_cast<LPCTSTR>(record.description));
	SetDlgItemText(IDC_EDIT_HISTORY_DETAIL, detailText);
}

void CHistoryDlg::OnBnClickedHistoryDelete()
{
	const int visibleIndex = GetSelectedVisibleIndex();
	if (visibleIndex < 0)
	{
		AfxMessageBox(L"请先选择一条历史记录。", MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (m_pDataManager == nullptr)
	{
		AfxMessageBox(L"历史记录数据不可用。", MB_OK | MB_ICONERROR);
		return;
	}

	const size_t recordIndex = m_visibleRecordIndexes[visibleIndex];
	const std::vector<LaunchRecord>& records = m_pDataManager->GetRecords();
	if (recordIndex >= records.size())
	{
		AfxMessageBox(L"所选历史记录已失效，请重新查询。", MB_OK | MB_ICONERROR);
		RefreshHistoryList();
		return;
	}

	const LaunchRecord& record = records[recordIndex];
	CString confirmMessage;
	confirmMessage.Format(L"确定要删除任务 %s 在 %s 的历史记录吗？",
		static_cast<LPCTSTR>(record.missionId),
		static_cast<LPCTSTR>(record.launchTime));
	if (AfxMessageBox(confirmMessage, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	if (!m_pDataManager->DeleteLaunchRecord(recordIndex))
	{
		AfxMessageBox(L"删除历史记录失败，请重新查询后再试。",
			MB_OK | MB_ICONERROR);
		return;
	}

	const int preferredVisibleIndex = visibleIndex;
	RefreshHistoryList();
	const int itemCount = m_historyList.GetItemCount();
	if (itemCount > 0)
	{
		const int newVisibleIndex = (std::min)(preferredVisibleIndex, itemCount - 1);
		m_historyList.SetItemState(newVisibleIndex,
			LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_historyList.EnsureVisible(newVisibleIndex, FALSE);
	}
}

void CHistoryDlg::OnBnClickedHistoryExport()
{
	if (m_pDataManager == nullptr)
	{
		AfxMessageBox(L"历史记录数据不可用。", MB_OK | MB_ICONERROR);
		return;
	}
	if (m_visibleRecordIndexes.empty())
	{
		AfxMessageBox(L"当前没有可导出的历史记录。",
			MB_OK | MB_ICONINFORMATION);
		return;
	}

	const std::vector<LaunchRecord>& records = m_pDataManager->GetRecords();
	CString exportText = L"任务 ID\t任务名称\t时间\t结果\t最大高度（km）\t说明\r\n";
	for (size_t visibleIndex = 0;
		visibleIndex < m_visibleRecordIndexes.size(); ++visibleIndex)
	{
		const size_t recordIndex = m_visibleRecordIndexes[visibleIndex];
		if (recordIndex >= records.size())
		{
			AfxMessageBox(L"历史记录列表已失效，请重新查询后再导出。",
				MB_OK | MB_ICONERROR);
			RefreshHistoryList();
			return;
		}

		const LaunchRecord& record = records[recordIndex];
		const CString missionName = GetMissionName(m_pDataManager, record.missionId);
		if (!IsValidTsvField(record.missionId) ||
			!IsValidTsvField(missionName) ||
			!IsValidTsvField(record.launchTime) ||
			!IsValidTsvField(record.result) ||
			!IsValidTsvField(record.description))
		{
			AfxMessageBox(
				L"历史记录字段包含制表符、回车或换行，无法安全导出为 TSV。",
				MB_OK | MB_ICONERROR);
			return;
		}

		CString heightText;
		heightText.Format(L"%.1f", record.maxHeightKm);
		exportText += record.missionId + L'\t' + missionName + L'\t' +
			record.launchTime + L'\t' + record.result + L'\t' +
			heightText + L'\t' + record.description + L"\r\n";
	}

	CFileDialog fileDialog(FALSE, L"tsv", L"launch_history.tsv",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"TSV 文件 (*.tsv)|*.tsv|所有文件 (*.*)|*.*||", this);
	if (fileDialog.DoModal() != IDOK)
	{
		return;
	}

	const CString path = fileDialog.GetPathName();
	if (!WriteUtf8Tsv(path, exportText))
	{
		AfxMessageBox(L"导出失败，请检查文件路径和写入权限。",
			MB_OK | MB_ICONERROR);
		return;
	}

	CString successMessage;
	successMessage.Format(L"已成功导出 %llu 条历史记录到：\r\n%s",
		static_cast<unsigned long long>(m_visibleRecordIndexes.size()),
		static_cast<LPCTSTR>(path));
	AfxMessageBox(successMessage, MB_OK | MB_ICONINFORMATION);
}
