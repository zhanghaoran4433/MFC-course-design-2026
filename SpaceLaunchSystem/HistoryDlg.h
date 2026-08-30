#pragma once

#include "DataManager.h"
#include "Resource.h"

class CHistoryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHistoryDlg)

public:
	CHistoryDlg(CDataManager* pDataManager, CWnd* pParent = nullptr);

	enum { IDD = IDD_HISTORY_DIALOG };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	CDataManager* m_pDataManager = nullptr;
	CListCtrl m_historyList;
	CString m_keyword;
	int m_resultFilter = 0;
	std::vector<size_t> m_visibleRecordIndexes;

	void RefreshHistoryList();
	int GetSelectedVisibleIndex() const;
	afx_msg void OnBnClickedHistorySearch();
	afx_msg void OnBnClickedHistoryDetail();
	afx_msg void OnBnClickedHistoryDelete();
	afx_msg void OnBnClickedHistoryExport();
};
