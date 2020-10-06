// Adjustment_of_Level_Nets_Hae-Min_LEE.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.

#include "stdafx.h"
#include <stdio.h>
#include <locale.h>

using namespace std;

struct input_data_station
{
	int Line; 
	char From[1024], To[1024];
	int From_idx, To_idx;
	float Elevation, SD;
};

struct input_data_BM
{
	char Name[1024];
	int Name_idx;
	float Elevation, SD;
};

void sort_name(vector<input_data_station> &input_data_station, vector<input_data_BM> &input_data_BM, vector<string> &sorted);

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
	// 구조체 벡터 선언  
	vector<input_data_station> ids_set; 
	vector<input_data_BM> idb_set;

	// 벡터 입력위한 버퍼 구조체 선언
	input_data_station ids_buf;
	input_data_BM idb_buf;

	// 파일 읽어 구조체 저장
	char FilePath_OB[1024], FilePath_BM[1024];

	cout << "Adjustment of Level Nets" << endl;
	cout << "Input File Path of Observation Data (.txt)" << endl;
	cin >> FilePath_OB;
	cout << "Input File Path of BenchMark Data (.txt)" << endl;
	cin >> FilePath_BM;

	if (FilePath_OB[0] == '1') { strcpy(FilePath_OB, "Observation_data_1.txt"); strcpy(FilePath_BM, "BenchMark_data_1.txt"); }
	else if (FilePath_OB[0] == '2') { strcpy(FilePath_OB, "Observation_data_2.txt"); strcpy(FilePath_BM, "BenchMark_data_2.txt"); }
	else if (FilePath_OB[0] == '3') { strcpy(FilePath_OB, "Observation_data_3(12.14).txt"); strcpy(FilePath_BM, "BenchMark_data_3(12.14).txt"); }

	cout << "< READ FILE >" << endl;
	FILE *fptr = fopen(FilePath_OB, "r");
	int buf_input = 1;
	if (fptr != NULL){ 
		while (!feof(fptr)){
			fscanf(fptr, "%s %s %f %f", ids_buf.To, ids_buf.From, &(ids_buf.Elevation), &(ids_buf.SD));
			ids_buf.Line = buf_input;
			ids_set.push_back(ids_buf);
			buf_input++; 
		}
		cout << endl; 
	}
	fclose(fptr);

	FILE *fptr_ = fopen(FilePath_BM, "r");
	if (fptr_ != NULL){
		while (!feof(fptr_)){
			fscanf(fptr_, "%s %f %f", idb_buf.Name, &(idb_buf.Elevation), &(idb_buf.SD));
			idb_set.push_back(idb_buf);
			buf_input++;
		}
		cout << endl;
	}
	fclose(fptr_);

	// Station Name을 순서에 맞춰 정렬 (Default : 입력순서)
	// 정렬된 순서에 맞게 Station Name에 따른 인덱스를 각각 From, To에 대하여 ids_set, idb_set에 저장
	vector<string> sorted;
	sort_name(ids_set, idb_set, sorted); 
	show_mat("SORT STATION NAME", sorted);

	// 구조체에 저장된 값을 기준으로 관측수, 수준점 수, 기준점 수 등을 정리하여 사용.
	int line_num = ids_set.size(); 
	int observ_num = ids_set.size() + idb_set.size(); 
	int station_num = sorted.size();
	int BM_num = idb_set.size(); 
	int m = observ_num;  int n = station_num;

	cout << "< INFO >" << endl;
	cout << "관측 수 : " << line_num << "\n수준점 수 : " << station_num << "\n기준점 수 : " << BM_num << endl << endl;

	// A행렬, W행렬, L행렬 dobule 형 vector로 선언
	vector< vector<double> > mat_A(m, vector<double>(n, 0)); // m*n
	vector< vector<double> > mat_W(m, vector<double>(m, 0)); // m*m
	vector< vector<double> > mat_L(m, vector<double>(1, 0)); // m*1

	// A행렬에 원소 저장
	// buf를 이용해 ids_set에 저장된 원소값 불러온 이후 idb_set에 저장된 원소값 불러옴
	int buf_idx = 0;
	for (int i = 0; i < line_num; i++){
		int From_idx = ids_set[i].From_idx;
		int To_idx = ids_set[i].To_idx;

		mat_A[i][From_idx] = 1; 
		mat_A[i][To_idx] = -1; 
		buf_idx++;
	}
	for (int i = 0; i < BM_num; i++){
		int BM_idx = idb_set[i].Name_idx;
		mat_A[buf_idx+i][BM_idx] = 1;
		
	}
	show_mat("MATRIX A", mat_A);

	// W행렬에 원소 저장 // buf를 이용해 ids_set에 저장된 원소값 불러온 이후 idb_set에 저장된 원소값 불러옴
	// W = 1 / 관측값 표준편차^2 
	int buf_w = 0; int buf_ww = 0;
	for (int i = 0; i < line_num; i++){
		buf_ww = 1 / ((ids_set[i].SD)*(ids_set[i].SD));
		mat_W[i][i] = buf_ww;
		buf_w++;
	}
	for (int i = 0; i < BM_num; i++){
		buf_ww = 1 / ((idb_set[i].SD)*(idb_set[i].SD));
		mat_W[buf_w + i][buf_w + i] = buf_ww;
	}
	show_mat("MATRIX W", mat_W);

	// L행렬에 원소 저장 // buf를 이용해 ids_set에 저장된 원소값 불러온 이후 idb_set에 저장된 원소값 불러옴
	int buf_elev = 0; 
	for (int i = 0; i < line_num; i++){
		mat_L[i][0] = ids_set[i].Elevation;
		buf_elev++;
	}
	for (int j = 0; j < BM_num; j++){
		mat_L[buf_elev + j][0] = idb_set[j].Elevation;
	}
	show_mat("MATRIX L", mat_L);


	// ATWA행렬, ATWL행렬 계산 
	vector< vector<double> > mat_ATWA(n, vector<double>(n, 0));
	vector< vector<double> > mat_ATWL(n, vector<double>(1, 0));

	// 행렬 연산의 간편함을 위해 강의자료를 참고해 원소 단위로
	// ATWA와 ATWL 계산
	for (int i = 0; i < line_num ; i++){

		int from = ids_set[i].From_idx;
		int to = ids_set[i].To_idx;

		// ATWA
		mat_ATWA[from][from] += (-1) * (-1) * mat_W[i][i];
		mat_ATWA[to][to] += (1) * (1) * mat_W[i][i];
		if (from != to) {
			mat_ATWA[from][to] += (-1) * (1) * mat_W[i][i];
			mat_ATWA[to][from] += (-1) * (1) * mat_W[i][i];
		}
		// ATWL
		mat_ATWL[from][0] += (1) * mat_W[i][i] * mat_L[i][0];
		mat_ATWL[to][0] += (-1) * mat_W[i][i] * mat_L[i][0];
	}

	for (int j = 0; j < BM_num; j++){

		int bm = idb_set[j].Name_idx;

		mat_ATWA[bm][bm] += (1) * (1) * mat_W[buf_elev + j][buf_elev + j];
		mat_ATWL[bm][0] += (1) * mat_W[buf_elev + j][buf_elev + j] * mat_L[buf_elev + j][0];
	}

	show_mat("MATRIX ATWA", mat_ATWA);
	show_mat("MATRIX ATWL", mat_ATWL);

	// X 행렬 계산
	// X = inv(ATWA)*ATWL
	vector< vector<double> > mat_inv_ATWA(n, vector<double>(n, 0));
	calc_invmat(mat_ATWA, mat_inv_ATWA);
	show_mat("MATRIX INV(ATWA)", mat_inv_ATWA);

	vector< vector<double> > mat_X(n, vector<double>(1, 0));
	calc_matrix(CALC_MULT, mat_inv_ATWA, mat_ATWL, mat_X);
	show_mat("MATRIX X", mat_X);
	
	// V 행렬 계산
	// V = AX - L
	vector< vector<double> > mat_AX(m, vector<double>(1, 0));
	vector< vector<double> > mat_V(m, vector<double>(1, 0));
	calc_matrix(CALC_MULT, mat_A, mat_X, mat_AX);
	show_mat("MATIRX AX", mat_AX);
	//show_mat("MATRIX L", mat_L);

	calc_matrix(CALC_DIFF, mat_AX, mat_L, mat_V);
	show_mat("MATRIX V", mat_V);

	// RMSE 계산
	// VTWV를 이용한 RMSE 계산
	vector< vector<double> > mat_VT(1, vector<double>(m, 0));
	calc_transmat(mat_V, mat_VT);
	show_mat("MATRIX VT", mat_VT);

	vector< vector<double> > mat_VTW(1, vector<double>(m, 0));
	vector< vector<double> > mat_VTWV(1, vector<double>(1, 0));
	calc_matrix(CALC_MULT, mat_VT, mat_W, mat_VTW);
	calc_matrix(CALC_MULT, mat_VTW, mat_V, mat_VTWV);
	show_mat("MATRIX VTWV", mat_VTWV);

	double RMSE = 0, VAR = 0;
	VAR = mat_VTWV[0][0] / (m - n);
	RMSE = sqrt(VAR);
	cout << "< VAR > " << VAR << endl << endl; 
	cout << "< RMSE > " << RMSE << endl << endl; 
	
	// 최확값의 표준편차 계산
	// COV = VAR * inv(ATWA)
	vector < vector <double> > mat_COV(n, vector<double>(n, 0)); 

	for (int i = 0; i < n; i++){
		for (int j = 0; j < n; j++){
			mat_COV[i][j] = VAR * mat_inv_ATWA[i][j];
		}
	}
	show_mat_dig("SD of MOST PROBABLE VALUE", mat_COV); // 대각선 원소 출력

	
	// 계산된 관측값의 표준편차 계산
	// COBS = A*COV*AT
	vector < vector <double> > mat_COBS(m, vector<double>(m, 0));
	vector < vector <double> > mat_ACOV(m, vector<double>(n, 0));
	vector< vector<double> > mat_AT(n, vector<double>(m, 0));
	calc_transmat(mat_A, mat_AT);
	calc_matrix(CALC_MULT, mat_A, mat_COV, mat_ACOV);
	calc_matrix(CALC_MULT, mat_ACOV, mat_AT, mat_COBS);

	show_mat_dig("SD of CALCULATED OBS VALUE",mat_COBS); // 대각선 원소 출력

	// 계산 결과 출력
	string buf(FilePath_OB);
	ofstream log("RESULT_" + buf);
	cout << "Save Output (.txt)" << endl;
	log << "Adjustment_of_Level_Nets 12161136 이해민" << endl;
	log << "Result of " << buf << endl; 

	log << "\n입력 자료" << endl; 
	for (int i = 0; i < ids_set.size(); i++){
		log << ids_set[i].Line << "  " << ids_set[i].From << "  " << ids_set[i].To << 
			"  " << ids_set[i].Elevation << "  " << ids_set[i].SD << endl;
	}
	for (int i = 0; i < idb_set.size(); i++){
		log << ids_set.size() +1 << "  " << idb_set[i].Name << "  " << idb_set[i].Elevation << "  " << idb_set[i].SD << endl;
	}

	log << "\n최종 조정된 표고와 정밀도" << endl;
	for (int i = 0; i < mat_X.size(); i++){ 
		log << sorted[i] << "  " << mat_X[i][0] << "  " << mat_COV[i][i] << endl;
	}

	log << "\n단위 무게의 분산과 표준편차" << endl;
	log << "VAR " << VAR << "\nRMSE " << RMSE << endl; 

	log << "\n조정 전 관측값과 조정 후 관측값" << endl; 
	for (int i = 0; i < mat_V.size(); i++){
		log << i + 1 << "  " << mat_L[i][0] << "  " << mat_AX[i][0] << endl;
	}

	log << "\n조정 관측값의 잔차와 정밀도" << endl;
	for (int i = 0; i < mat_V.size(); i++){
		log << i + 1 << "  " << mat_V[i][0] << "  " << mat_COBS[i][i] << endl;
	}

	return 0;
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

void sort_name(vector<input_data_station> &input_data_station, vector<input_data_BM> &input_data_BM, vector<string> &sorted)
{

	// BenchMarch 이름을 sorted에 저장. 
	for (int i = 0; i < input_data_BM.size(); i++){
		sorted.push_back(input_data_BM[i].Name);
	}
	// Station name을 sorted에 저장. 
	// 중복없이 Station name을 저장하기 위해 if문 사용
	for (int i = 0; i < input_data_station.size(); i++){
		string from_str = input_data_station[i].From;
		string to_str = input_data_station[i].To;
		if (find(sorted.begin(), sorted.end(), from_str) == sorted.end()) sorted.push_back(from_str);
		if (find(sorted.begin(), sorted.end(), to_str) == sorted.end()) sorted.push_back(to_str);
	}

	// sort(sorted.begin(), sorted.end(), less<string>()); // 이름 내림차순 정렬

	// 정렬된 순서에 맞게 Station Name에 따른 인덱스를 각각 From, To에 대하여 ids_set, idb_set에 저장
	for (int i = 0; i < input_data_station.size(); i++){
		for (int j = 0; j < sorted.size(); j++){
			if (input_data_station[i].From == sorted[j]) input_data_station[i].From_idx = j;
			if (input_data_station[i].To == sorted[j]) input_data_station[i].To_idx = j;
		}
	}
	for (int i = 0; i < input_data_BM.size(); i++){
		for (int j = 0; j < sorted.size(); j++){
			if (input_data_BM[i].Name == sorted[j]) input_data_BM[i].Name_idx = j;
		}
	}
}

void show_mat(string name, vector<vector<double>> &mat){

	cout << "< " << name << " >\n";

	int rows = mat.size();
	int cols = mat[0].size();

	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			cout << mat[i][j] << "  ";
		}
		cout << endl;
	}

	cout << endl;

}

void show_mat(string name, vector<double> &mat){

	cout << "< " << name << " >\n";

	int rows = mat.size();

	for (int i = 0; i < rows; i++){
		cout << mat[i] << "  " << endl;
	}

	cout << endl;

}

void show_mat(string name, vector<string> &mat){

	cout << "< " << name << " >\n";

	int rows = mat.size();

	for (int i = 0; i < rows; i++){
		cout << mat[i] << "  " << endl;
	}

	cout << endl;

}

void show_mat_dig(string name, vector<vector<double>> &mat)
{

	cout << "< " << name << " >\n";
	int rows = mat.size();
	for (int i = 0; i < rows; i++){
		cout << mat[i][i] << endl;
	}

	cout << endl;
}