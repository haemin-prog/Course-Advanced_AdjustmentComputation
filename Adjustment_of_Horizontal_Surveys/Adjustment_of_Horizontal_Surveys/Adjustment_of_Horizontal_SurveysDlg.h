
// Adjustment_of_Horizontal_SurveysDlg.h : ��� ����
//

#pragma once
#include <fstream>
#include <io.h>
#include <cstringt.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <conio.h>
#include <string>
#include <iomanip>


// CAdjustment_of_Horizontal_SurveysDlg ��ȭ ����
class CAdjustment_of_Horizontal_SurveysDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CAdjustment_of_Horizontal_SurveysDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ADJUSTMENT_OF_HORIZONTAL_SURVEYS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();

	CEdit m_Edit_main;
	CButton m_Radio;
	afx_msg void OnBnClickedButton8();
};
