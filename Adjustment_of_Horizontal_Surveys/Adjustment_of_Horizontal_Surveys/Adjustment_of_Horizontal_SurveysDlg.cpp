
// Adjustment_of_Horizontal_SurveysDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "Adjustment_of_Horizontal_Surveys.h"
#include "Adjustment_of_Horizontal_SurveysDlg.h"
#include "afxdialogex.h"

#include <fstream>
#include <io.h>
#include <cstringt.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <conio.h>
#include <string>
#include <iomanip>
#include <algorithm>
#include <math.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Struct 
struct project_info {
	char Project_Name[1024];
	char YYMMDD[6], etc[1024];
};

struct station {
	char Name[100];
	int Name_idx;
	float X, Y, SD_X, SD_Y;
};

struct distance_obs {
	char From[100], To[100];
	int From_idx, To_idx;
	float Distance, SD;
};

struct angle_obs {
	char Back[100], Occu[100], Fore[100];
	int Back_idx, Occu_idx, Fore_idx;
	float ang;
	float deg, min, sec;
	float SD;
};

struct azimuth_obs {
	char From[100], To[100];
		int From_idx, To_idx;
	float ang;
	float deg, min, sec;
	float SD;
};

class A_hor {

public:
	// Parameters
	CString OutputPath = "output.txt";
	CString ControlStation_p = "input_controlstations.txt", InitCoord_p = "input_initcoord.txt", DistObs_p = "input_distobs.txt", AngleObs_p = "input_angleobs.txt", AzimuthObs_p = "input_azimuthobs.txt";

	int max_iter = 10;
	float min_corr = 0.0001, min_std_corr = 0.0001;
	float max_std_corr = 20;
	float RMSE_old = 0;

	int size_cs, size_is, size_do, size_ano, size_azo;
	int size_st;

	int it = 1;
	string modify = "none";

	// vectors
	vector <station> vec_cs;
	vector <station> vec_is;
	vector <station> vec_st;
	vector <distance_obs> vec_do;
	vector <angle_obs> vec_ano;
	vector <azimuth_obs> vec_azo;

	ofstream log;
	ofstream output;

	// Functions
	void init_setting();
	void read_init();
	void output_init();
	void sort_name();
	double DMS2DEG(double deg, double min, double sec);
	double DMS2RAD(double deg, double min, double sec);
	void Rad2DMS(double rad, double &d, double& m, double& s);

};


#define CALC_MULT 0
#define CALC_SUM 1 
#define CALC_DIFF 2

#define M_PI 3.14159265358979323846

void calc_invmat(vector<vector<double>> &mat, vector<vector<double>> &inv_mat);
void calc_transmat(vector<vector<double>> &mat, vector<vector<double>> &trans_mat);
void calc_matrix(int value, vector<vector<double>> &mat1, vector<vector<double>> &mat2, vector<vector<double>> &result_mat);
void show_mat(string name, vector<vector<double>> &mat);
void show_mat(string name, vector<string> &mat);
void show_mat(string name, vector<double> &mat);
void show_mat_dig(string name, vector<vector<double>> &mat);

A_hor hm;

// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAdjustment_of_Horizontal_SurveysDlg ��ȭ ����

CAdjustment_of_Horizontal_SurveysDlg::CAdjustment_of_Horizontal_SurveysDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAdjustment_of_Horizontal_SurveysDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAdjustment_of_Horizontal_SurveysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_Edit_main);
}

BEGIN_MESSAGE_MAP(CAdjustment_of_Horizontal_SurveysDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT2, &CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, &CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit5)
	ON_BN_CLICKED(IDC_BUTTON4, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton8)
END_MESSAGE_MAP()


// CAdjustment_of_Horizontal_SurveysDlg �޽��� ó����

BOOL CAdjustment_of_Horizontal_SurveysDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CAdjustment_of_Horizontal_SurveysDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CAdjustment_of_Horizontal_SurveysDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CAdjustment_of_Horizontal_SurveysDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

double A_hor::DMS2DEG(double deg, double min, double sec){

	double result = deg + (min / 60) + (sec / 3600);
	return result; 
}


double A_hor::DMS2RAD(double deg, double min, double sec){

	double rad = (deg + (min / 60) + (sec / 3600)) * M_PI / 180;
	return rad; 
}

void A_hor::Rad2DMS(double rad, double &d, double& m, double& s) {
	double ang = rad * 180 / M_PI;
	m = ang - int(ang);
	d = ang - m;
	m *= 60;
	s = m - int(m);
	m = m - s;
	s *= 60;
}

void A_hor::read_init(){

	// CString ControlStation_p = "input_controlstations.txt", InitCoord_p = "input_initcoord.txt", DistObs_p = "input_distobs.txt", AngleObs_p = "input_angleobs.txt", AzimuthObs_p = "input_azimuthobs.txt";
	
	hm.output << "\n ****************** " << endl;
	hm.output << "[ �Է��ڷ� ]" << endl;
	hm.output << " ****************** " << endl;

	station buf_is;

	hm.output << "\n[ Initial Coordination of Stations ]" << endl;
	hm.output << "Name" << "	" << "X" << "	" << "Y" << "	" << "SD_X" << "	" << "SD_Y" << endl;
	FILE *fptr2 = fopen(InitCoord_p, "r");
	if (fptr2 != NULL){
		while (!feof(fptr2)){
			fscanf(fptr2, "%s %f %f %f %f", buf_is.Name, &(buf_is.X), &(buf_is.Y), &(buf_is.SD_X), &(buf_is.SD_Y));
			hm.output << buf_is.Name << "	" << buf_is.X << "	" << buf_is.Y << "	" << buf_is.SD_X << "	" << buf_is.SD_Y << endl;
			vec_st.push_back(buf_is);
			vec_is.push_back(buf_is);

		}
	}
	fclose(fptr2);


	station buf_cs;

	hm.output << "\n[ Control Stations ]" << endl;
	hm.output << "Name" << "	" << "X" << "	" << "Y" << "	" << "SD_X" << "	" << "SD_Y" << endl;

	FILE *fptr = fopen(ControlStation_p, "r");
	if (fptr != NULL){
		while (!feof(fptr)){
			fscanf(fptr, "%s %f %f %f %f", buf_cs.Name, &(buf_cs.X), &(buf_cs.Y), &(buf_cs.SD_X), &(buf_cs.SD_Y));
			hm.output << buf_cs.Name << "	" << buf_cs.X << "	" << buf_cs.Y << "	" << buf_cs.SD_X << "	" << buf_cs.SD_Y << endl;
			vec_st.push_back(buf_cs);
			vec_cs.push_back(buf_cs);
		}
	}
	fclose(fptr);
	size_cs = vec_cs.size();

	size_is = vec_is.size();
	size_st = vec_st.size();

	distance_obs buf_do;

	hm.output << "\n[ Distance Observation ]" << endl;
	hm.output << "From" << "	" << "To" << "	" << "Dist" << "	" << "SD" << endl;

	FILE *fptr3 = fopen(DistObs_p, "r");
	if (fptr3 != NULL){
		while (!feof(fptr3)){
			fscanf(fptr3, "%s %s %f %f", buf_do.From, buf_do.To, &(buf_do.Distance), &(buf_do.SD));
			hm.output << buf_do.From << "	" << buf_do.To << "	" << buf_do.Distance << "	" << buf_do.SD << endl;
			vec_do.push_back(buf_do);
		}
	}
	fclose(fptr3);
	size_do = vec_do.size();

	double buf, ang;
	angle_obs buf_ano;

	hm.output << "\n[ Angle Observation ]" << endl;
	hm.output << "Back" << "	" << "Occu" << "	" << "Fore" << "	" << "deg" << "	" << "min" << "	" << "sec" << "	" << "SD" << endl;

	FILE *fptr4 = fopen(AngleObs_p, "r");
	if (fptr4 != NULL){
		while (!feof(fptr4)){
			fscanf(fptr4, "%s %s %s %f %f %f %f", buf_ano.Back, buf_ano.Occu, buf_ano.Fore, &(buf_ano.deg), &(buf_ano.min), &(buf_ano.sec), &(buf_ano.SD));

			hm.output << buf_ano.Back << "	" << buf_ano.Occu << "	" << buf_ano.Fore << "	" << buf_ano.deg << "	" << buf_ano.min << "	" << buf_ano.sec << "	" << buf_ano.SD << endl; 

			ang = DMS2RAD(buf_ano.deg, buf_ano.min, buf_ano.sec);
			buf_ano.ang = ang;

			buf = DMS2RAD(0, 0, buf_ano.SD);
			buf_ano.SD = buf;

			vec_ano.push_back(buf_ano);
		}
	}
	fclose(fptr4);
	size_ano = vec_ano.size();

	azimuth_obs buf_azo;

	hm.output << "\n[ Azimuth Observation ]" << endl;
	hm.output << "From" << "	" << "To" << "	" << "deg" << "	" << "min" << "	" << "sec" << "	" << "SD" << endl;

	FILE *fptr5 = fopen(AzimuthObs_p, "r");
	if (fptr5 != NULL){
		while (!feof(fptr5)){
			fscanf(fptr5, "%s %s %f %f %f %f", buf_azo.From, buf_azo.To, &(buf_azo.deg), &(buf_azo.min), &(buf_azo.sec), &(buf_azo.SD));

			hm.output << buf_azo.From << "	" << buf_azo.To << "	" << buf_azo.deg << "	" << buf_azo.min << "	" << buf_azo.sec << "	" << buf_azo.SD << endl;

			ang = DMS2RAD(buf_azo.deg, buf_azo.min, buf_azo.sec);
			buf_azo.ang = ang;

			buf = DMS2RAD(0, 0, buf_azo.SD);
			buf_azo.SD = buf;

			vec_azo.push_back(buf_azo);
		}
	}
	fclose(fptr5);
	size_azo = vec_azo.size();

	hm.output << "\n ****************** " << endl;
	hm.output << "[ �ݺ��������� ]" << endl;
	hm.output << " ****************** \n" << endl;
	hm.output << "�ִ� �ݺ� Ƚ�� : " << max_iter << endl;
	hm.output << "������ �ּ� ������ : " << min_corr << endl;
	hm.output << "ǥ������ �ּ� ������ : " << min_std_corr << endl;

}

void A_hor::sort_name()
{
	vector<string> sorted;
	for (int i = 0; i < hm.size_st; i++) sorted.push_back(hm.vec_st[i].Name);

	// Station (CS + IS)
	for (int i = 0; i < hm.size_st; i++){
		for (int j = 0; j < sorted.size(); j++){
			if (hm.vec_st[i].Name == sorted[j]) hm.vec_st[i].Name_idx = j;
		}
	}

	// CS
	for (int i = 0; i < hm.size_cs; i++){
		for (int j = 0; j < sorted.size(); j++){
			if (hm.vec_cs[i].Name == sorted[j]) hm.vec_cs[i].Name_idx = j;
		}
	}

	// IS
	for (int i = 0; i < hm.size_is; i++){
		for (int j = 0; j < sorted.size(); j++){
			if (hm.vec_is[i].Name == sorted[j]) hm.vec_is[i].Name_idx = j;
		}
	}

	// Distance 
	for (int i = 0; i < hm.size_do; i++){
		for (int j = 0; j < sorted.size(); j++){
			if (hm.vec_do[i].From == sorted[j]) hm.vec_do[i].From_idx = j;
			if (hm.vec_do[i].To == sorted[j]) hm.vec_do[i].To_idx = j;
		}
	}

	// Angle
	for (int i = 0; i < hm.size_ano; i++){
		for (int j = 0; j < sorted.size(); j++){
			if (hm.vec_ano[i].Back == sorted[j]) hm.vec_ano[i].Back_idx = j;
			if (hm.vec_ano[i].Occu == sorted[j]) hm.vec_ano[i].Occu_idx = j;
			if (hm.vec_ano[i].Fore == sorted[j]) hm.vec_ano[i].Fore_idx = j;
		}
	}

	// Azimuth
	for (int i = 0; i < hm.size_azo; i++){
		for (int j = 0; j < sorted.size(); j++){
			if (hm.vec_azo[i].From == sorted[j]) hm.vec_azo[i].From_idx = j;
			if (hm.vec_azo[i].To == sorted[j]) hm.vec_azo[i].To_idx = j;
		}
	}

	show_mat("string", sorted);
}

void A_hor::init_setting(){
	
	vec_cs.clear();
	vec_is.clear();
	vec_st.clear();
	vec_do.clear();
	vec_ano.clear();
	vec_azo.clear();

	size_cs = size_is = size_do = size_ano = size_azo = 0;
	it = 1;
	RMSE_old = 0;

	

}

// RUN Button
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton1()
{

	hm.init_setting();

	hm.log.open("log.txt");
	hm.output.open(hm.OutputPath);

	// 1. �Է�, �Է��ڷ� ��� �� �ݺ� �������� ���

	if (hm.vec_st.size() == 0){ hm.read_init(); };

	// 2. ������ �̸� ���� �� Index �ο�
	hm.sort_name();

	hm.log << "�Ÿ� ���� �� : " << hm.size_do << endl; 
	hm.log << "���� ���� �� : " << hm.size_ano << endl;
	hm.log << "������ ���� �� : " << hm.size_azo << endl;
	hm.log << "Control Station ���� �� : " << hm.size_cs << endl << endl;

	int m = hm.size_do + hm.size_ano + hm.size_azo + 2 * hm.size_cs;
	int n = 2 * hm.size_st;

	hm.log << "m : " << m << endl;
	hm.log << "n : " << n << endl << endl; 

	// 3. W ��� ����
	vector< vector<double> > mat_W(m, vector<double>(m, 0));

	int w_idx = 0;
	// Distance Obs
	for (int i = 0; i < hm.size_do; i++){
		mat_W[i][i] = 1 / ((hm.vec_do[i].SD) * (hm.vec_do[i].SD));
	}
	w_idx += hm.size_do;

	// Angle Obs
	for (int i = 0; i < hm.size_ano; i++){
		mat_W[i + w_idx][i + w_idx] = 1 / ((hm.vec_ano[i].SD) * (hm.vec_ano[i].SD));
	}
	w_idx += hm.size_ano;

	// Azimuth Obs
	for (int i = 0; i < hm.size_azo; i++){
		mat_W[i + w_idx][i + w_idx] = 1 / ((hm.vec_azo[i].SD) * (hm.vec_azo[i].SD));
	}
	w_idx += hm.size_azo;

	// Control Stations
	for (int i = 0; i < hm.size_cs; i++){
		mat_W[2 * i + w_idx][2 * i + w_idx] = 1 / ((hm.vec_cs[i].SD_X) * (hm.vec_cs[i].SD_X));
		mat_W[2 * i + 1 + w_idx][2 * i + 1 + w_idx] = 1 / ((hm.vec_cs[i].SD_Y) * (hm.vec_cs[i].SD_Y));
	}
	show_mat("Matrix W", mat_W);

	vector <double> cov(n, 0);
	vector <double> cobs(m, 0);
	double RMSE = 0, VAR = 0;
	vector < vector <double> > Qxx(n, vector<double>(n, 0));
	vector < vector<double> > jx(m, vector<double>(1, 0));

	vector< vector<double> > L(m, vector<double>(1, 0));

	int l_idx = 0;
	// Distance Obs
	for (int i = 0; i < hm.size_do; i++){
		L[i][0] = hm.vec_do[i].Distance;
	}
	l_idx += hm.size_do;

	// Angle Obs
	for (int i = 0; i < hm.size_ano; i++){
		L[i + l_idx][0] = hm.vec_ano[i].ang;
	}
	l_idx += hm.size_ano;

	// Azimuth Obs
	for (int i = 0; i < hm.size_azo; i++){
		L[i + l_idx][0] = hm.vec_ano[i].ang;
	}
	l_idx += hm.size_azo;

	// Control Stations
	for (int i = 0; i < hm.size_cs; i++){
		L[2 * i + l_idx][0] = hm.vec_cs[i].X;
		L[2 * i + l_idx + 1][0] = hm.vec_cs[i].Y;
	}


	while (true){

		// �ݺ����
		hm.output << "\n ****************** " << endl;
		hm.output << "[ " << hm.it << " ȸ �ݺ� ]" << endl;
		hm.output << " ****************** " << endl;

		// 4. J���, K��� ����
		int a_idx = 0;
		vector< vector<double> > mat_J(m, vector<double>(n, 0));
		vector< vector<double> > mat_K(m, vector<double>(1, 0));

		// Distance Obs
		for (int i = 0; i < hm.size_do; i++){

			int from = hm.vec_do[i].From_idx;
			int to = hm.vec_do[i].To_idx;

			double deno;
			deno = ((hm.vec_st[from].X - hm.vec_st[to].X) * (hm.vec_st[from].X - hm.vec_st[to].X)) + ((hm.vec_st[from].Y - hm.vec_st[to].Y)*(hm.vec_st[from].Y - hm.vec_st[to].Y));
			deno = sqrt(deno);

			mat_J[i][2 * from] = (hm.vec_st[from].X - hm.vec_st[to].X) / deno;
			mat_J[i][2 * from + 1] = (hm.vec_st[from].Y - hm.vec_st[to].Y) / deno;

			mat_J[i][2 * to] = (hm.vec_st[to].X - hm.vec_st[from].X) / deno;
			mat_J[i][2 * to + 1] = (hm.vec_st[to].Y - hm.vec_st[from].Y) / deno;

			mat_K[i][0] = hm.vec_do[i].Distance - deno;

		}
		a_idx += hm.size_do;

		// Angle Obs
		for (int i = 0; i < hm.size_ano; i++){

			int back = hm.vec_ano[i].Back_idx;
			int occu = hm.vec_ano[i].Occu_idx;
			int fore = hm.vec_ano[i].Fore_idx;

			// Back-Occu
			double IB2 = (hm.vec_st[back].X - hm.vec_st[occu].X) * (hm.vec_st[back].X - hm.vec_st[occu].X) + (hm.vec_st[back].Y - hm.vec_st[occu].Y) * (hm.vec_st[back].Y - hm.vec_st[occu].Y);
			// Occu-Fore
			double IF2 = (hm.vec_st[occu].X - hm.vec_st[fore].X) * (hm.vec_st[occu].X - hm.vec_st[fore].X) + (hm.vec_st[occu].Y - hm.vec_st[fore].Y) * (hm.vec_st[occu].Y - hm.vec_st[fore].Y);

			mat_J[i + a_idx][2 * back] = (hm.vec_st[occu].Y - hm.vec_st[back].Y) / IB2;
			mat_J[i + a_idx][2 * back + 1] = (hm.vec_st[back].X - hm.vec_st[occu].X) / IB2;

			mat_J[i + a_idx][2 * occu] = ((hm.vec_st[back].Y - hm.vec_st[occu].Y) / IB2) - ((hm.vec_st[fore].Y - hm.vec_st[occu].Y) / IF2);
			mat_J[i + a_idx][2 * occu + 1] = ((hm.vec_st[occu].X - hm.vec_st[back].X) / IB2) - ((hm.vec_st[occu].X - hm.vec_st[fore].X) / IF2);

			mat_J[i + a_idx][2 * fore] = (hm.vec_st[fore].Y - hm.vec_st[occu].Y) / IF2;
			mat_J[i + a_idx][2 * fore + 1] = (hm.vec_st[occu].X - hm.vec_st[fore].X) / IF2;

			double instAng = atan2(hm.vec_st[fore].X - hm.vec_st[occu].X, hm.vec_st[fore].Y - hm.vec_st[occu].Y) - atan2(hm.vec_st[back].X - hm.vec_st[occu].X, hm.vec_st[back].Y - hm.vec_st[occu].Y);
			if (instAng < 0){ instAng += 2 * M_PI; }

			mat_K[i + a_idx][0] = hm.vec_ano[i].ang - instAng;

		}
		a_idx += hm.size_ano;

		// Azimuth Obs
		for (int i = 0; i < hm.size_azo; i++){

			int from = hm.vec_azo[i].From_idx;
			int to = hm.vec_azo[i].To_idx;

			double deno2 = ((hm.vec_st[from].X - hm.vec_st[to].X) * (hm.vec_st[from].X - hm.vec_st[to].X)) + ((hm.vec_st[from].Y - hm.vec_st[to].Y) * (hm.vec_st[from].Y - hm.vec_st[to].Y));

			mat_J[i + a_idx][2 * from] = (hm.vec_st[from].Y - hm.vec_st[to].Y) / deno2;
			mat_J[i + a_idx][2 * from + 1] = (hm.vec_st[to].X - hm.vec_st[from].X) / deno2;

			mat_J[i + a_idx][2 * to] = (hm.vec_st[to].Y - hm.vec_st[from].Y) / deno2;
			mat_J[i + a_idx][2 * to + 1] = (hm.vec_st[from].X - hm.vec_st[to].X) / deno2;

			mat_K[i + a_idx][0] = hm.vec_azo[i].ang - atan2((hm.vec_st[to].X - hm.vec_st[from].X), (hm.vec_st[to].Y - hm.vec_st[from].Y));

		}
		a_idx += hm.size_azo;

		// Control Stations
		for (int i = 0; i < hm.size_cs; i++){

			int st = hm.vec_cs[i].Name_idx;

			mat_J[2 * i + a_idx][2 * st] = 1;
			mat_J[2 * i + a_idx + 1][2 * st + 1] = 1;

		}
		show_mat("Matrix J", mat_J);
		show_mat("Matrix K", mat_K);

		// 5. X��� ���
		// X = inv(JTWJ)*(JTWK);
		vector < vector <double> > mat_JT(n, vector<double>(m, 0));
		vector < vector <double> > mat_JTW(n, vector<double>(m, 0));
		vector < vector <double> > mat_JTWJ(n, vector<double>(n, 0));
		vector < vector <double> > mat_inv_JTWJ(n, vector<double>(n, 0));
		vector < vector <double> > mat_JTWK(n, vector<double>(1, 0));
		vector < vector <double> > mat_X(n, vector<double>(1, 0));

		calc_transmat(mat_J, mat_JT);
		calc_matrix(CALC_MULT, mat_JT, mat_W, mat_JTW);
		calc_matrix(CALC_MULT, mat_JTW, mat_J, mat_JTWJ);
		calc_matrix(CALC_MULT, mat_JTW, mat_K, mat_JTWK);
		calc_invmat(mat_JTWJ, mat_inv_JTWJ);
		calc_invmat(mat_JTWJ, Qxx);

		calc_matrix(CALC_MULT, mat_inv_JTWJ, mat_JTWK, mat_X);
		show_mat("MATRIX X", mat_X);

		// 6. V ��� ���, RMSE ���
		// V = JX - K
		vector < vector<double> > mat_JX(m, vector<double>(1, 0));
		vector < vector<double> > mat_V(m, vector<double>(1, 0));

		calc_matrix(CALC_MULT, mat_J, mat_X, mat_JX);
		calc_matrix(CALC_MULT, mat_J, mat_X, jx);
		calc_matrix(CALC_DIFF, mat_JX, mat_K, mat_V);
		show_mat("MATRIX V", mat_V);

		// RMSE ���
		// VTWV�� �̿��� RMSE ���
		vector < vector<double> > mat_VT(1, vector<double>(m, 0));
		vector < vector<double> > mat_VTW(1, vector<double>(m, 0));
		vector < vector<double> > mat_VTWV(1, vector<double>(1, 0));

		calc_transmat(mat_V, mat_VT);
		calc_matrix(CALC_MULT, mat_VT, mat_W, mat_VTW);
		calc_matrix(CALC_MULT, mat_VTW, mat_V, mat_VTWV);

		
		VAR = mat_VTWV[0][0] / (m - n);
		RMSE = sqrt(VAR);

		// 7. COV��� ���, COBS��� ���, 
		// ��Ȯ���� ǥ������ ���
		// COV = VAR * inv(ATWA)
		vector < vector <double> > mat_COV(n, vector<double>(n, 0));

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				mat_COV[i][j] = VAR * mat_inv_JTWJ[i][j];
			}
			cov[i] = mat_COV[i][i];
		}
		show_mat_dig("Matrix COV", mat_COV);

		// ���� �������� ǥ������ ���
		// COBS = A*COV*AT
		vector < vector <double> > mat_COBS(m, vector<double>(m, 0));
		vector < vector <double> > mat_JCOV(m, vector<double>(n, 0));
		calc_matrix(CALC_MULT, mat_J, mat_COV, mat_JCOV);
		calc_matrix(CALC_MULT, mat_JCOV, mat_JT, mat_COBS);
		for (int i = 0; i < m; i++) cobs[i] = mat_COBS[i][i];
		show_mat_dig("Matrix COBS", mat_COBS);

		// 8. Itertation ����� ���

		hm.output.setf(ios::fixed);
		hm.output.precision(6);

		hm.output << "\n[ ������ ������ ]" << endl;
		for (int i = 0; i < hm.size_st; i++){
			hm.output << hm.vec_st[i].Name;
			hm.output << "		dX : " << mat_X[2 * i][0];
			hm.output << "		dY : " << mat_X[2 * i + 1][0] << "\n";
		}

		hm.output << "\n[ �������� ���� ]" << endl;
		for (int i = 0; i < hm.size_st; i++){
			hm.output << hm.vec_st[i].Name;
			hm.output << "		X : " << mat_X[2 * i][0];
			hm.output << "		Y : " << mat_X[2 * i + 1][0] << " \n";
		}

		hm.output << "\n[ �������� �л� ] " << VAR << endl;
		hm.output << "[ �������� ǥ������ ] " << RMSE << endl;
		

		// 9. �ݺ� ���� ���� ����
		double max = 0;
		for (int i = 0; i < n; i++){
			if (max < abs(mat_X[i][0])) max = mat_X[i][0];
		}

		if (hm.min_corr > abs(max) && hm.it != 1){
			hm.output << "\n������ ������ (" << abs(max) << ") �� �ּ� ������ (" << hm.min_corr << ") ���� �۽��ϴ�." << endl;
			break;
		}
		if (hm.min_std_corr > abs(hm.RMSE_old - RMSE) && hm.it != 1){
			hm.output << "\nǥ������ ������ ( " << abs(hm.RMSE_old - RMSE) << " �� �ּ� ������ ( " << hm.min_std_corr 
				<< " ) ���� �۽��ϴ�." << endl;
			break;
		}
		if (hm.it == hm.max_iter){
			hm.output << endl << hm.it << " ȸ �ݺ��Ǿ����ϴ�." << endl;
			break;
		}

		//10. �ʱⰪ ����
		for (int i = 0; i < hm.size_st; i++){
			hm.vec_st[i].X += mat_X[2 * i][0];
			hm.vec_st[i].Y += mat_X[2 * i + 1][0];
		}

		hm.RMSE_old = RMSE;
		hm.it += 1;

	}

	// 11. ���� ��� ���
	hm.output << "�ݺ��� �����մϴ�." << endl;

	hm.output << "\n ****************** " << endl;
	hm.output << "[ ������� ]" << endl;
	hm.output << " ****************** \n" << endl;


	hm.output << "[ ���� ������ ��ǥ�� ���е� ]" << endl;
	for (int i = 0; i < hm.size_st; i++){
		hm.output << hm.vec_st[i].Name;
		hm.output << "		X : " << hm.vec_st[i].X << "  ��  " << sqrt(cov[2 * i]);
		hm.output << "		Y : " << hm.vec_st[i].Y << "  ��  " << sqrt(cov[2 * i + 1]) << endl;
	}

	hm.output << "\n[ ���� ���� �������� ���е� ]" << endl;
	int idx = 0;
	hm.output << "\n[ Distance Observation ]" << endl;
	for (int i = 0; i < hm.size_do; i++){
		hm.output << hm.vec_do[i].From << " - " << hm.vec_do[i].To;
		hm.output << " :	" << L[i][0] + jx[i][0] << "  ��  " << sqrt(cobs[i]) << endl;
	}
	idx += hm.size_do;
	hm.output << "\n[ Angle Observation ]" << endl;
	for (int i = 0; i < hm.size_ano; i++){

		double ang_buf = L[i + idx][0] + jx[i + idx][0];
		double d, m, s;
		hm.Rad2DMS(ang_buf, d, m, s);

		hm.output << hm.vec_ano[i].Back << " - " << hm.vec_ano[i].Occu << " - " << hm.vec_ano[i].Fore;
		hm.output << " :	" << L[i + idx][0] + jx[i + idx][0] << "  ��  " << sqrt(cobs[i + idx]) << endl;
	}
	idx += hm.size_ano;
	hm.output << "\n[ Azimuth Observation ]" << endl;
	for (int i = 0; i < hm.size_azo; i++){

		hm.output << hm.vec_azo[i].From << " - " << hm.vec_azo[i].To;
		hm.output << " :	" << L[i + idx][0] + jx[i + idx][0] << "  ��  " << sqrt(cobs[i + idx]) << endl;
	}
	idx += hm.size_azo;
	hm.output << "\n[ Control Station ]" << endl;
	for (int i = 0; i < hm.size_cs; i++){
		hm.output << hm.vec_st[i].Name; 
		hm.output << " X :	" << L[i + idx][0] + jx[2 * i + idx][0] << "  ��  " << sqrt(cobs[2 * i + idx]) << endl;
		hm.output << hm.vec_st[i].Name;
		hm.output << " Y :	" << L[i + idx][0] + jx[2 * i + idx + 1][0] << "  ��  " << sqrt(cobs[2 * i + idx + 1]) << endl;
	}

	// 12. ����Ÿ�� ���

	hm.output << "\n ****************** " << endl;
	hm.output << "[ ����Ÿ�� ]" << endl;
	hm.output << " ****************** \n" << endl;

	vector <double> sin2t (hm.size_st, 0);
	vector <double> cos2t(hm.size_st, 0);
	vector <double> tan2t(hm.size_st, 0);
	vector <double> t(hm.size_st, 0);
	vector <double> Su(hm.size_st, 0);
	vector <double> Sv(hm.size_st, 0);

	for (int i = 0; i < hm.size_st; i++){

		sin2t[i] = 2 * Qxx[2 * i][2 * i + 1];
		cos2t[i] = Qxx[2 * i + 1][2 * i + 1] - Qxx[2 * i][2 * i];
		tan2t[i] = sin2t[i] / cos2t[i];

		if ((sin2t[i] > 0) && (cos2t[i] > 0))		t[i] = atan(tan2t[i]) / 2;
		else if ((sin2t[i] > 0) && (cos2t[i] < 0))	t[i] = (atan(tan2t[i]) + M_PI) / 2;
		else if ((sin2t[i] < 0) && (cos2t[i] < 0))	t[i] = (atan(tan2t[i]) + M_PI) / 2;
		else if ((sin2t[i] < 0) && (cos2t[i] > 0))	t[i] = (atan(tan2t[i]) + 2 * M_PI) / 2;

		Su[i] = RMSE * RMSE * sqrt(Qxx[2 * i][2 * i] * sin(t[i]) * sin(t[i]) + 2 * Qxx[2 * i][2 * i + 1] * cos(t[i]) * sin(t[i]) + Qxx[2 * i + 1][2 * i + 1] * cos(t[i]) * cos(t[i]));
		Sv[i] = RMSE * RMSE * sqrt(Qxx[2 * i][2 * i] * cos(t[i])*cos(t[i]) - 2 * Qxx[2 * i][2 * i + 1] * cos(t[i]) * sin(t[i]) + Qxx[2 * i + 1][2 * i + 1] * sin(t[i]) * sin(t[i]));

		hm.output << hm.vec_st[i].Name << "	t : " << t[i] << "	Su : " << Su[i] << "	Sv : " << Sv[i] << endl;

	}

	// 13. Chi-Square Test ���

	hm.output << "\n ****************** " << endl;
	hm.output << "[ ����Test ]" << endl;
	hm.output << " ****************** \n" << endl;

	double DOF = m - n;
	double Value = DOF * RMSE;

	ifstream chi_Fin("chi_square_table.txt");
	double chi_idx, chi_995, chi_005;
	while (chi_Fin >> chi_idx){
		if (chi_idx == DOF){
			chi_Fin >> chi_005 >> chi_995;
			break;
		}
	}

	hm.output << "DOF : " << DOF << endl;
	hm.output << "RMSE : " << RMSE << endl;
	hm.output << "Value : " << Value << endl;
	hm.output << "Lower Value : " << chi_005 << endl;
	hm.output << "Upper Value : " << chi_995 << endl;

	if (chi_005 > Value || Value > chi_995) hm.output << "\n����Test Failed" << endl;
	else hm.output << "\n����Test Success" << endl;


	hm.output.close();

	CFile File;
	if (File.Open(hm.OutputPath, CFile::modeRead | CFile::typeText))
	{
		DWORD dwSize = File.GetLength();
		TCHAR *str = new TCHAR[dwSize + 1];

		if (str)
		{
			File.Read(str, dwSize*sizeof(TCHAR));
			str[dwSize] = 0;
			GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
			delete[] str;
		}
		File.Close();
	}

	hm.output.unsetf(ios::fixed);
}



// Input Path Button of Control Stations
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton3()
{
	
	hm.modify = "cs";
	
	CString str = _T("All files(*.*)|*.*|"); // ��� ���� ǥ��
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str, this);

	if (dlg.DoModal() == IDOK) { hm.ControlStation_p = dlg.GetPathName(); }

	CFile File; 
	if (File.Open(hm.ControlStation_p, CFile::modeRead | CFile::typeText))
	{
		DWORD dwSize = File.GetLength();
		TCHAR *str = new TCHAR[dwSize + 1];

		if (str)
		{
			File.Read(str, dwSize*sizeof(TCHAR));
			str[dwSize] = 0;
			GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
			delete[] str;
		}
		File.Close();
	}

}

// Init Coord. 
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton4()
{
	
	hm.modify = "is";
	
	CString str = _T("All files(*.*)|*.*|"); // ��� ���� ǥ��
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str, this);

	if (dlg.DoModal() == IDOK) { hm.InitCoord_p = dlg.GetPathName(); }


	CFile File;
	if (File.Open(hm.InitCoord_p, CFile::modeRead | CFile::typeText))
	{
		DWORD dwSize = File.GetLength();
		TCHAR *str = new TCHAR[dwSize + 1];

		if (str)
		{
			File.Read(str, dwSize*sizeof(TCHAR));
			str[dwSize] = 0;
			GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
			delete[] str;
		}
		File.Close();
	}

}

// Dist. Obs.
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton5()
{
	
	hm.modify = "do";
	
	CString str = _T("All files(*.*)|*.*|"); // ��� ���� ǥ��
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str, this);

	if (dlg.DoModal() == IDOK) { hm.DistObs_p = dlg.GetPathName(); }

	CFile File;
	if (File.Open(hm.DistObs_p, CFile::modeRead | CFile::typeText))
	{
		DWORD dwSize = File.GetLength();
		TCHAR *str = new TCHAR[dwSize + 1];

		if (str)
		{
			File.Read(str, dwSize*sizeof(TCHAR));
			str[dwSize] = 0;
			GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
			delete[] str;
		}
		File.Close();
	}
}

// Angle Obs.
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton6()
{
	hm.modify = "ano";
	
	CString str = _T("All files(*.*)|*.*|"); // ��� ���� ǥ��
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str, this);

	if (dlg.DoModal() == IDOK) { hm.AngleObs_p = dlg.GetPathName(); }

	CFile File;
	if (File.Open(hm.AngleObs_p, CFile::modeRead | CFile::typeText))
	{
		DWORD dwSize = File.GetLength();
		TCHAR *str = new TCHAR[dwSize + 1];

		if (str)
		{
			File.Read(str, dwSize*sizeof(TCHAR));
			str[dwSize] = 0;
			GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
			delete[] str;
		}
		File.Close();
	}

}

// Azimuth Obs.
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton7()
{
	hm.modify = "azo";
	
	CString str = _T("All files(*.*)|*.*|"); // ��� ���� ǥ��
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str, this);

	if (dlg.DoModal() == IDOK) { hm.AzimuthObs_p = dlg.GetPathName(); }

	CFile File;
	if (File.Open(hm.AzimuthObs_p, CFile::modeRead | CFile::typeText))
	{
		DWORD dwSize = File.GetLength();
		TCHAR *str = new TCHAR[dwSize + 1];

		if (str)
		{
			File.Read(str, dwSize*sizeof(TCHAR));
			str[dwSize] = 0;
			GetDlgItem(IDC_EDIT2)->SetWindowTextA(str);
			delete[] str;
		}
		File.Close();
	}

}

// Output Path Button
void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton2()
{
	BROWSEINFO BrInfo;
	char szBuffer[512];

	::ZeroMemory(&BrInfo, sizeof(BROWSEINFO));
	::ZeroMemory(szBuffer, 512);

	BrInfo.hwndOwner = GetSafeHwnd();
	BrInfo.lpszTitle = "Select Data file to load data";
	BrInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pItemIdList = ::SHBrowseForFolder(&BrInfo);
	::SHGetPathFromIDList(pItemIdList, szBuffer);

	//hm.OutputPath.Format(szBuffer);
	char op[20] = "\\output.txt";
	strcat(szBuffer, op);
	hm.OutputPath.Format(szBuffer);
}

void CAdjustment_of_Horizontal_SurveysDlg::OnBnClickedButton8()
{
	CString cs; 
	GetDlgItemText(IDC_EDIT2, cs);
	
	ofstream mo;

	if (hm.modify == "cs"){
		mo.open(hm.ControlStation_p);
		mo << cs;
	}
	else if (hm.modify == "is"){
		mo.open(hm.InitCoord_p);
		mo << cs;
	}
	else if (hm.modify == "do"){
		mo.open(hm.DistObs_p);
		mo << cs;
	}
	else if (hm.modify == "ano"){
		mo.open(hm.AngleObs_p);
		mo << cs;
	}
	else if (hm.modify == "azo"){
		mo.open(hm.AzimuthObs_p);
		mo << cs;
	}





}


// m_Edit_main
void CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit2()
{
	
}


//  Max Iteration
void CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit3()
{
	CString cs1; 
	GetDlgItemText(IDC_EDIT3, cs1);
	hm.max_iter = atof(cs1.GetString());

}

// Min correction Value
void CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit4()
{
	CString cs2;
	GetDlgItemText(IDC_EDIT4, cs2);
	hm.min_corr = atof(cs2.GetString());
}

// Min Std Correction Value
void CAdjustment_of_Horizontal_SurveysDlg::OnEnChangeEdit5()
{
	CString cs3;
	GetDlgItemText(IDC_EDIT5, cs3);
	hm.min_std_corr = atof(cs3.GetString());
}

void calc_invmat(vector<vector<double>> &mat, vector<vector<double>> &inv_mat)
{
	int n = mat.size();
	vector < vector <double>> calc_inv(n, vector<double>(2 * n, 0));

	for (int i = 0; i < n; i++){
		for (int j = 0; j < n; j++){
			calc_inv[i][j] = mat[i][j];
		}
	}

	for (int i = 0; i < n; i++){
		for (int j = n; j < 2 * n; j++){
			if (i == j - n) calc_inv[i][j] = 1;
		}
	}

	for (int i = 0; i<n; i++){
		float t = calc_inv[i][i];
		for (int j = i; j < 2 * n; j++) calc_inv[i][j] = calc_inv[i][j] / t;

		for (int j = 0; j < n; j++){
			if (i != j){
				t = calc_inv[j][i];
				for (int k = 0; k<2 * n; k++) calc_inv[j][k] = calc_inv[j][k] - t*calc_inv[i][k];
			}
		}
	}

	for (int i = 0; i < n; i++){
		for (int j = 0; j < n; j++) inv_mat[i][j] = calc_inv[i][j + n];
	}

}

void calc_transmat(vector<vector<double>> &mat, vector<vector<double>> &trans_mat)
{
	int rows = mat.size();
	int cols = mat[0].size();

	vector< vector<double> > buff_mat(cols, vector<double>(rows, 0));
	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			buff_mat[j][i] = mat[i][j];
		}
	}
	trans_mat = buff_mat;

}

void calc_matrix(int value, vector<vector<double>> &mat1, vector<vector<double>> &mat2, vector<vector<double>> &result_mat)
{

	if (value == CALC_MULT){
		int p = mat1.size();
		int q = mat2.size();
		int r = mat2[0].size();

		for (int i = 0; i < p; i++){
			for (int j = 0; j < r; j++){
				for (int k = 0; k < q; k++){
					result_mat[i][j] += mat1[i][k] * mat2[k][j];
				}
			}
		}
	}
	else {
		int rows = mat1.size();
		int cols = mat1[0].size();

		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				if (value == CALC_DIFF) result_mat[i][j] = mat1[i][j] - mat2[i][j];
				if (value == CALC_SUM) result_mat[i][j] = mat1[i][j] + mat2[i][j];
			}
		}
	}

}

void show_mat(string name, vector<vector<double>> &mat){

	hm.log << "< " << name << " >\n";

	int rows = mat.size();
	int cols = mat[0].size();

	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			hm.log << mat[i][j] << "  ";
		}
		hm.log << endl;
	}

	hm.log << endl;

}

void show_mat(string name, vector<double> &mat){

	hm.log << "< " << name << " >\n";

	int rows = mat.size();

	for (int i = 0; i < rows; i++){
		hm.log << mat[i] << "  " << endl;
	}

	hm.log << endl;

}

void show_mat(string name, vector<string> &mat){

	hm.log << "< " << name << " >\n";

	int rows = mat.size();

	for (int i = 0; i < rows; i++){
		hm.log << mat[i] << "  " << endl;
	}

	hm.log << endl;

}

void show_mat_dig(string name, vector<vector<double>> &mat)
{

	hm.log << "< " << name << " >\n";
	int rows = mat.size();
	for (int i = 0; i < rows; i++){
		hm.log << mat[i][i] << endl;
	}

	hm.log << endl;
}

