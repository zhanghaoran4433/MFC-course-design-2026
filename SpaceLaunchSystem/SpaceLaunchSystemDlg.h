
// SpaceLaunchSystemDlg.h: 头文件
//

#pragma once

#include "DataManager.h"


// CSpaceLaunchSystemDlg 对话框
class CSpaceLaunchSystemDlg : public CDialogEx
{
// 构造
public:
	CSpaceLaunchSystemDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPACELAUNCHSYSTEM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void RefreshMainSummary();
	afx_msg void OnBnClickedMissionManager();
	afx_msg void OnBnClickedLaunchCheck();
	afx_msg void OnBnClickedLaunchSimulation();
	afx_msg void OnBnClickedHistory();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

private:
	CDataManager m_dataManager;
	CString m_currentMissionId;
};
