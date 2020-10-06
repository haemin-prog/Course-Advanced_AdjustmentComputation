
//과제명: 2차원상사좌표변환 프로그램 작성(일반최소제곱법 적용)
//	1. 입력 자료
//	- 변환 후 기준점 좌표값과 정밀도(표준편차)
//	- 변환 전 기준점 좌표값과 정밀도
//	- 변환 전 점 좌표값과 정밀도
//	2. 출력 자료
//	- 좌표값과 정밀도 등 입력자료
//	- 표준최소제곱법으로 계산된 변환계수(이 변환계수가 초기값이 됨)
//	- 매 반복 계산(iteration) 후의 초기값에 대한 조정량
//	- 매 반복 계산 후의 기준점 좌표의 잔차와 
//	- 반복 계산 종료 후, 최종 조정된 변환계수와 그 정밀도
//	- 변환된 좌표와 그 정밀도
//	3. 기타
//	- 프로그램 언어 : Matlab 또는 Visual C++(Visual Studio)
//	- 프로그램의 모듈화 : 각 모듈의 기능, 프로그램의 각 줄의 의미 등을 상세히 朱記
//	- 제출물 : 프로그램 코드 Soft Copy, 입력과 출력자료의 Hard Copy
//	- 프로그램 검정용 자료 1 : 교과서 22.4(p471)Example 22.2

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

struct obs_p
{
	int point;
	float x, y;
	float x_std, y_std;
};

void calc_invmat(vector<vector<double>> &mat, vector<vector<double>> &inv_mat);
void calc_transmat(vector<vector<double>> &mat, vector<vector<double>> &trans_mat);
void calc_matrix(int value, vector<vector<double>> &mat1, vector<vector<double>> &mat2, vector<vector<double>> &result_mat);
void show_mat(string name, vector<vector<double>> &mat);
void show_mat(string name, vector<string> &mat);
void show_mat(string name, vector<double> &mat);
void show_mat_dig(string name, vector<vector<double>> &mat);

#define CALC_MULT 0
#define CALC_SUM 1 
#define CALC_DIFF 2

int main()
{
	vector<obs_p> after_p;

	obs_p before_buf, after_buf;

	char FilePath_input_after[1024];
	char FilePath_input_before[1024];
	// strcpy(FilePath_input, "input.txt");

	ofstream log("result.txt");

	cout << "Input File Path of After Observation Data (.txt)" << endl;
	cin >> FilePath_input_after;
	strcpy(FilePath_input_after, "input_after.txt");

	cout << "Input File Path of Before Observation Data (.txt)" << endl;
	cin >> FilePath_input_before;
	strcpy(FilePath_input_before, "input_before.txt");

	cout << "\n============================== ";
	cout << " 자료 입력 ";
	cout << "==============================\n" << endl;

	log << "\n============================== ";
	log << " 자료 입력 ";
	log << "==============================\n" << endl;


	cout << "Point   X   std(X)   Y   std(Y)" << endl;
	log << "Point   X   std(X)   Y   std(Y)" << endl;

	FILE *fptr = fopen(FilePath_input_after, "r");
	if (fptr != NULL) {
		while (!feof(fptr)) {
			fscanf(fptr, "%d %f %f %f %f", &(after_buf.point), &(after_buf.x), &(after_buf.x_std), &(after_buf.y), &(after_buf.y_std));
			after_p.push_back(after_buf);

			cout << after_buf.point << "  " << after_buf.x << "  " << after_buf.x_std << "  " << after_buf.y << "  " << after_buf.y_std << endl;
			log << after_buf.point << "  " << after_buf.x << "  " << after_buf.x_std << "  " << after_buf.y << "  " << after_buf.y_std << endl;
		}
		cout << endl;
		log << endl; 
	}
	fclose(fptr);

	vector<obs_p> before_p(after_p.size());

	cout << "Point   x   std(x)   y   std(y)" << endl;
	log << "Point   x   std(x)   y   std(y)" << endl;

	fptr = fopen(FilePath_input_before, "r");
	if (fptr != NULL) {
		while (!feof(fptr)) {
			fscanf(fptr, "%d %f %f %f %f", &(before_buf.point), &(before_buf.x), &(before_buf.x_std), &(before_buf.y), &(before_buf.y_std));

			for (int i = 0; i < after_p.size(); i++) {
				if (after_p[i].point == before_buf.point) {
					before_p[i].point = before_buf.point;
					before_p[i].x = before_buf.x; before_p[i].x_std = before_buf.x_std;
					before_p[i].y = before_buf.y; before_p[i].y_std = before_buf.y_std;
				}
			}
			cout << before_buf.point << "  " << before_buf.x << "  " << before_buf.x_std << "  " << before_buf.y << "  " << before_buf.y_std << endl;
			log << before_buf.point << "  " << before_buf.x << "  " << before_buf.x_std << "  " << before_buf.y << "  " << before_buf.y_std << endl;
		}
		cout << endl;
		log << endl;
	}
	fclose(fptr);

	int max_iter = 3;
	float min_corr = 0.0001, min_std_corr = 0.01;
	float max_std_corr = 20; 
	float RMSE_old = 0;

	//cout << "반복횟수 : ";
	//cin >> max_iter; 
	//
	//cout << "\n최소 조정량 : ";
	//cin >> min_corr;

	//cout << "\n최소 표준편차 변화율 : ";
	//cin >> min_std_corr;

	log << "반복횟수 : " << max_iter << endl;
	log << "최소 조정량 : " << min_corr << endl;
	log << "최소 표준편차 변화율 : " << min_std_corr << endl << endl; 
	
	cout.setf(ios::fixed);
	cout.precision(5);

	log.setf(ios::fixed);
	log.precision(5);

	int num = before_p.size();
	int m = num * 2, m1 = num * 4, n = num;

	// 초기값 계산 (표준최소제곱법)

	cout << "============================== ";
	cout << " 초기값 계산 ";
	cout << "==============================\n" << endl;

	log << "============================== ";
	log << " 초기값 계산 ";
	log << "==============================\n" << endl;

	vector < vector < double > > mat_A(m, vector<double>(n, 0));
	vector < vector < double > > mat_L(m, vector<double>(1, 0));
	vector < vector < double > > mat_X(n, vector<double>(1, 0));

	for (int i = 0; i < m; i++) {

		if (i % 2 == 1) {
			mat_A[i][0] = before_p[i / 2].y;
			mat_A[i][1] = before_p[i / 2].x;
			mat_A[i][2] = 0;
			mat_A[i][3] = 1;

			mat_L[i][0] = after_p[i / 2].y;
		}
		else {
			mat_A[i][0] = before_p[i / 2].x;
			mat_A[i][1] = -before_p[i / 2].y;
			mat_A[i][2] = 1;
			mat_A[i][3] = 0;

			mat_L[i][0] = after_p[i / 2].x;
		}
	}

	vector < vector <double> > mat_AT(n, vector<double>(m, 0));
	vector < vector <double> > mat_ATA(n, vector<double>(n, 0));
	vector < vector <double> > mat_inv_ATA(n, vector<double>(n, 0));
	vector < vector <double> > mat_ATL(n, vector<double>(1, 0));
	vector < vector <double> > mat_init_X(n, vector<double>(1, 0));

	calc_transmat(mat_A, mat_AT);
	calc_matrix(CALC_MULT, mat_AT, mat_A, mat_ATA);
	calc_matrix(CALC_MULT, mat_AT, mat_L, mat_ATL);
	calc_invmat(mat_ATA, mat_inv_ATA);
	calc_matrix(CALC_MULT, mat_inv_ATA, mat_ATL, mat_init_X);


	double a0 = mat_init_X[0][0], b0 = mat_init_X[1][0], c0 = mat_init_X[2][0], d0 = mat_init_X[3][0];



	cout << "a0 : " << a0 << "  b0 : " << b0 << "  c0 : " << c0 << "  d0 : " << d0 << endl << endl; 
	log << "a0 : " << a0 << "  b0 : " << b0 << "  c0 : " << c0 << "  d0 : " << d0 << endl << endl;

	cout << "< 초기 좌표>" << endl;
	for (int i = 0; i < num; i++) {
		cout << "X" << i + 1 << " = " << mat_init_X[0][0] * before_p[i].x - mat_init_X[1][0] * before_p[i].y + mat_init_X[2][0] << "   ";
		cout << "Y" << i + 1 << " = " << mat_init_X[1][0] * before_p[i].x + mat_init_X[0][0] * before_p[i].y + mat_init_X[3][0] << endl;
	}
	log << "< 초기 좌표>" << endl;
	for (int i = 0; i < num; i++) {
		log << "X" << i + 1 << " = " << mat_init_X[0][0] * before_p[i].x - mat_init_X[1][0] * before_p[i].y + mat_init_X[2][0] << "   ";
		log << "Y" << i + 1 << " = " << mat_init_X[1][0] * before_p[i].x + mat_init_X[0][0] * before_p[i].y + mat_init_X[3][0] << endl;
	}
	cout << endl; log << endl;

	

	// 일반최소제곱법

	double a = mat_init_X[0][0], b = mat_init_X[1][0], c = mat_init_X[2][0], d = mat_init_X[3][0];
	vector < vector <double> > cov(n, vector<double>(n, 0));
	vector <double> cobs(m, 0);

	for (int it = 1; it < max_iter+1; it++){

		cout << "============================== ";
		cout << it << " 번째 반복  "; 
		cout << "==============================" << endl;

		log << "============================== ";
		log << it << " 번째 반복  ";
		log << "==============================" << endl;

		vector < vector <double> > mat_B(m, vector<double>(m1, 0));
		vector < vector <double> > mat_Q(m1, vector<double>(m1, 0));
		vector < vector <double> > mat_J(m, vector<double>(n, 0));
		vector < vector <double> > mat_K(m, vector<double>(1, 0));

		// B행렬에 원소 저장
		for (int j = 0; j < num; j++) {

			int i = j * 4;
			int ii = j * 2;

			mat_B[ii][i] = a;
			mat_B[ii][i + 1] = -b;
			mat_B[ii][i + 2] = -1;
			mat_B[ii][i + 3] = 0;

			mat_B[ii + 1][i] = b;
			mat_B[ii + 1][i + 1] = a;
			mat_B[ii + 1][i + 2] = 0;
			mat_B[ii + 1][i + 3] = -1;
		}
		//show_mat("MATRIX B", mat_B);

		// Q행렬에 원소 저장
		for (int j = 0; j < num; j++) {

			int i = j * 4;

			mat_Q[i][i] = before_p[j].x_std * before_p[j].x_std;
			mat_Q[i + 1][i + 1] = before_p[j].y_std * before_p[j].y_std;
			mat_Q[i + 2][i + 2] = after_p[j].x_std * after_p[j].x_std;
			mat_Q[i + 3][i + 3] = after_p[j].y_std * after_p[j].y_std;
		}
		//show_mat("MATRIX Q", mat_Q);

		// J행렬에 원소 저장
		for (int j = 0; j < num; j++) {

			int i = j * 2;

			mat_J[i][0] = before_p[j].x;
			mat_J[i][1] = -1 * before_p[j].y;
			mat_J[i][2] = 1;
			mat_J[i][3] = 0;

			mat_J[i + 1][0] = before_p[j].y;
			mat_J[i + 1][1] = before_p[j].x;
			mat_J[i + 1][2] = 0;
			mat_J[i + 1][3] = 1;

		}
		//show_mat("MATRIX J", mat_J);

		// K행렬에 원소 저장
		for (int j = 0; j < num; j++) {

			int i = j * 2;

			mat_K[i][0] = after_p[j].x - (a * before_p[j].x - b * before_p[j].y + c);
			mat_K[i + 1][0] = after_p[j].y - (b * before_p[j].x + a * before_p[j].y + d);

		}
		//show_mat("MATRIX K", mat_K);

		// W행렬 계산
		// W = BQBT
		vector < vector <double> > mat_BQ(m, vector<double>(m1, 0));
		vector < vector <double> > mat_BT(m1, vector<double>(m, 0));
		vector < vector <double> > mat_BQBT(m, vector<double>(m, 0));
		vector < vector <double> > mat_W(m, vector<double>(m, 0));

		calc_transmat(mat_B, mat_BT);
		calc_matrix(CALC_MULT, mat_B, mat_Q, mat_BQ);
		calc_matrix(CALC_MULT, mat_BQ, mat_BT, mat_BQBT);
		calc_invmat(mat_BQBT, mat_W);

		// X행렬 계산
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

		calc_matrix(CALC_MULT, mat_inv_JTWJ, mat_JTWK, mat_X);
		//show_mat("MATRIX X", mat_X); // 너무 크게 나오는데? 

		// V 행렬 계산
		// V = JX - K
		vector < vector<double> > mat_JX(m, vector<double>(1, 0));
		vector < vector<double> > mat_V(m, vector<double>(1, 0));

		calc_matrix(CALC_MULT, mat_J, mat_X, mat_JX);
		//show_mat("MATIRX AX", mat_JX);
		calc_matrix(CALC_DIFF, mat_JX, mat_K, mat_V);
		//show_mat("MATRIX V", mat_V);

		// RMSE 계산
		// VTWV를 이용한 RMSE 계산
		vector < vector<double> > mat_VT(1, vector<double>(m, 0));
		vector < vector<double> > mat_VTW(1, vector<double>(m, 0));
		vector < vector<double> > mat_VTWV(1, vector<double>(1, 0));

		calc_transmat(mat_V, mat_VT);
		//show_mat("MATRIX VT", mat_VT);

		calc_matrix(CALC_MULT, mat_VT, mat_W, mat_VTW);
		calc_matrix(CALC_MULT, mat_VTW, mat_V, mat_VTWV);
		//show_mat("MATRIX VTWV", mat_VTWV);

		double RMSE = 0, VAR = 0;
		VAR = mat_VTWV[0][0] / (m - n);
		RMSE = sqrt(VAR);
		//cout << "< VAR > " << VAR << endl << endl;
		//cout << "< RMSE > " << RMSE << endl << endl;

		// 최확값의 표준편차 계산
		// COV = VAR * inv(ATWA)
		vector < vector <double> > mat_COV(n, vector<double>(n, 0));

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				mat_COV[i][j] = VAR * mat_inv_JTWJ[i][j];
				cov[i][j] = mat_COV[i][j];
			}
		}
		//show_mat_dig("SD of MOST PROBABLE VALUE", mat_COV); // 대각선 원소 출력


		// 계산된 관측값의 표준편차 계산
		// COBS = A*COV*AT
		vector < vector <double> > mat_COBS(m, vector<double>(m, 0));
		vector < vector <double> > mat_JCOV(m, vector<double>(n, 0));
		calc_matrix(CALC_MULT, mat_J, mat_COV, mat_JCOV);
		calc_matrix(CALC_MULT, mat_JCOV, mat_JT, mat_COBS);
		for (int i = 0; i < m; i++) cobs[i] = mat_COBS[i][i];
		//show_mat_dig("SD of CALCULATED OBS VALUE", mat_COBS); // 대각선 원소 출력


		cout << "\n< 기준점 좌표의 잔차 >" << endl;
		for (int j = 0; j < num; j++) {
			int i = j * 2;
			cout << "X" << j + 1 << " : " << mat_V[i][0];
			cout << "  Y" << j + 1 << " : " << mat_V[i + 1][0] << endl;
		}

		cout << "\n< 기준점 좌표의 RMSE >" << endl;
		cout << RMSE << endl;

		// 출력
		log << "\n< 기준점 좌표의 잔차 >" << endl;
		for (int j = 0; j < num; j++) {
			int i = j * 2;
			log << "X" << j + 1 << " : " << mat_V[i][0];
			log << "  Y" << j + 1 << " : " << mat_V[i + 1][0] << endl;
		}

		log << "\n< 기준점 좌표의 RMSE >" << endl;
		log << RMSE << endl;


		// 중단조건
		double max = 0;
		for (int i = 0; i < 4; i++) {
			if (max < mat_X[i][0]) max = mat_X[i][0];
		}

		if (min_corr > abs(max) && it != 1) {

			cout << "\n< 초기값 조정량 >" << endl;
			cout << "da : " << mat_X[0][0] << "  db : " << mat_X[1][0] << "  dc : " << mat_X[2][0] << "  dd : " << mat_X[3][0] << endl;
			cout << "\n조정량이 최소 조정량( " << min_corr << " ) 보다 작습니다." << endl;

			log << "\n< 초기값 조정량 >" << endl;
			log << "da : " << mat_X[0][0] << "  db : " << mat_X[1][0] << "  dc : " << mat_X[2][0] << "  dd : " << mat_X[3][0] << endl;
			log << "\n조정량이 최소 조정량( " << min_corr << " ) 보다 작습니다." << endl;


			break;
		}

		if (min_std_corr > abs(RMSE - RMSE_old) && it != 1) {
			cout << "\n표준편차 변화량이 최소 표준편차 변화량보다 작습니다." << endl;
			cout << "표준편차 변화량 : " << (RMSE_old - RMSE) << "  최소 표준편차 변화량 : " << min_std_corr << endl << endl;

			log << "\n표준편차 변화량이 최소 표준편차 변화량보다 작습니다." << endl;
			log << "표준편차 변화량 : " << (RMSE_old - RMSE) << "  최소 표준편차 변화량 : " << min_std_corr << endl << endl;
			break;
		}

		if (max_std_corr < abs(RMSE) && it != 1) {
			cout << "\n표준편차가 최대 표준편차 기준보다 큽니다." << endl;
			cout << "표준편차 : " << (RMSE) << "  최대 표준편차 기준 : " << max_std_corr << endl << endl;

			log << "\n표준편차가 최대 표준편차 기준보다 큽니다." << endl;
			log << "표준편차 : " << (RMSE) << "  최대 표준편차 기준 : " << max_std_corr << endl << endl;

			cout << "\n프로그램을 종료합니다." << endl; 
			log << "\n프로그램을 종료합니다." << endl;

			return 0;
		}

		a += mat_X[0][0];
		b += mat_X[1][0];
		c += mat_X[2][0];
		d += mat_X[3][0];

		RMSE_old = RMSE;

		cout << "\n< 초기값 조정량 >" << endl;
		cout << "da : " << mat_X[0][0] << "  db : " << mat_X[1][0] << "  dc : " << mat_X[2][0] << "  dd : " << mat_X[3][0] << endl;

		cout << "\n< 갱신된 변환계수 >" << endl;
		cout << "a : " << a << "  b : " << b << "  c : " << c << "  d : " << d << endl << endl;

		log << "\n< 초기값 조정량 >" << endl;
		log << "da : " << mat_X[0][0] << "  db : " << mat_X[1][0] << "  dc : " << mat_X[2][0] << "  dd : " << mat_X[3][0] << endl;

		log << "\n< 갱신된 변환계수 >" << endl;
		log << "a : " << a << "  b : " << b << "  c : " << c << "  d : " << d << endl << endl;


	}

	cout << "\n============================== ";
	cout << " 반복 종료 ";
	cout << "==============================\n" << endl;

	cout << "< 최종 조정된 변환계수 >" << endl; 
	cout << "a : " << a << "  b : " << b << "  c : " << c << "  d : " << d << endl << endl;

	cout << "< 최종 조정된 변환계수의 정밀도 >" << endl; 
	cout << "a : " << cov[0][0] << "  b : " << cov[1][1] << "  c : " << cov[2][2] << "  d : " << cov[3][3] << endl; 

	cout << "\n< 변환된 좌표>" << endl;
	for (int i = 0; i < num; i++) {
		cout << "X" << i + 1 << " = " << a * before_p[i].x - b*before_p[i].y + c << "   ";
		cout << "Y" << i + 1 << " = " << b * before_p[i].x + a*before_p[i].y + d << endl;
	}

	cout << "\n< 변환된 좌표의 정밀도 >" << endl;
	for (int i = 0; i < m; i+=2) {
		cout << "X" << i + 1 << " : " << cobs[i] << "   ";
		cout << "Y" << i + 1 << " : " << cobs[i+1] << endl;
	}
	cout << endl;

	log << "\n============================== ";
	log << " 반복 종료 ";
	log << "==============================\n" << endl;

	log << "< 최종 조정된 변환계수 >" << endl;
	log << "a : " << a << "  b : " << b << "  c : " << c << "  d : " << d << endl << endl;

	log << "< 최종 조정된 변환계수의 정밀도 >" << endl;
	log << "a : " << cov[0][0] << "  b : " << cov[1][1] << "  c : " << cov[2][2] << "  d : " << cov[3][3] << endl;

	log << "\n< 변환된 좌표>" << endl;
	for (int i = 0; i < num; i++) {
		log << "X" << i + 1 << " = " << a * before_p[i].x - b * before_p[i].y + c << "   ";
		log << "Y" << i + 1 << " = " << b * before_p[i].x + a * before_p[i].y + d << endl;
	}

	log << "\n< 변환된 좌표의 정밀도 >" << endl;
	for (int i = 0; i < m; i += 2) {
		log << "X" << i + 1 << " : " << cobs[i] << "   ";
		log << "Y" << i + 1 << " : " << cobs[i + 1] << endl;
	}
	log << endl;

	return 0;
}

void calc_invmat(vector<vector<double>> &mat, vector<vector<double>> &inv_mat)
{
	int n = mat.size();
	vector < vector <double>> calc_inv(n, vector<double>(2 * n, 0));

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			calc_inv[i][j] = mat[i][j];
		}
	}

	for (int i = 0; i < n; i++) {
		for (int j = n; j < 2 * n; j++) {
			if (i == j - n) calc_inv[i][j] = 1;
		}
	}

	for (int i = 0; i<n; i++) {
		float t = calc_inv[i][i];
		for (int j = i; j < 2 * n; j++) calc_inv[i][j] = calc_inv[i][j] / t;

		for (int j = 0; j < n; j++) {
			if (i != j) {
				t = calc_inv[j][i];
				for (int k = 0; k<2 * n; k++) calc_inv[j][k] = calc_inv[j][k] - t*calc_inv[i][k];
			}
		}
	}

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) inv_mat[i][j] = calc_inv[i][j + n];
	}

}

void calc_transmat(vector<vector<double>> &mat, vector<vector<double>> &trans_mat)
{
	int rows = mat.size();
	int cols = mat[0].size();

	vector< vector<double> > buff_mat(cols, vector<double>(rows, 0));
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			buff_mat[j][i] = mat[i][j];
		}
	}
	trans_mat = buff_mat;

}

void calc_matrix(int value, vector<vector<double>> &mat1, vector<vector<double>> &mat2, vector<vector<double>> &result_mat)
{

	if (value == CALC_MULT) {
		int p = mat1.size();
		int q = mat2.size();
		int r = mat2[0].size();

		for (int i = 0; i < p; i++) {
			for (int j = 0; j < r; j++) {
				for (int k = 0; k < q; k++) {
					result_mat[i][j] += mat1[i][k] * mat2[k][j];
				}
			}
		}
	}
	else {
		int rows = mat1.size();
		int cols = mat1[0].size();

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (value == CALC_DIFF) result_mat[i][j] = mat1[i][j] - mat2[i][j];
				if (value == CALC_SUM) result_mat[i][j] = mat1[i][j] + mat2[i][j];
			}
		}
	}

}


void show_mat(string name, vector<vector<double>> &mat) {

	cout << "< " << name << " >\n";

	int rows = mat.size();
	int cols = mat[0].size();

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			cout << mat[i][j] << "  ";
		}
		cout << endl;
	}

	cout << endl;

}

void show_mat(string name, vector<double> &mat) {

	cout << "< " << name << " >\n";

	int rows = mat.size();

	for (int i = 0; i < rows; i++) {
		cout << mat[i] << "  " << endl;
	}

	cout << endl;

}

void show_mat(string name, vector<string> &mat) {

	cout << "< " << name << " >\n";

	int rows = mat.size();

	for (int i = 0; i < rows; i++) {
		cout << mat[i] << "  " << endl;
	}

	cout << endl;

}

void show_mat_dig(string name, vector<vector<double>> &mat)
{

	cout << "< " << name << " >\n";
	int rows = mat.size();
	for (int i = 0; i < rows; i++) {
		cout << mat[i][i] << endl;
	}

	cout << endl;
}