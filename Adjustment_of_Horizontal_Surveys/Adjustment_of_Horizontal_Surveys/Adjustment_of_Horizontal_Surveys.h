
// Adjustment_of_Horizontal_Surveys.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CAdjustment_of_Horizontal_SurveysApp:
// �� Ŭ������ ������ ���ؼ��� Adjustment_of_Horizontal_Surveys.cpp�� �����Ͻʽÿ�.
//

class CAdjustment_of_Horizontal_SurveysApp : public CWinApp
{
public:
	CAdjustment_of_Horizontal_SurveysApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CAdjustment_of_Horizontal_SurveysApp theApp;