// NBRG.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>
#include <cstdlib>
using namespace std;

int find_word_in_vc(string &str, vector<string> &string_vc)
{
	vector<string>::iterator it = find(string_vc.begin(), string_vc.end(), str);
	if (it == string_vc.end())
	{
		return -1;
	}
	return it - string_vc.begin();
}
void normalize_6(double* x)
{
	double sum = 0;
	for (int i = 0; i < 6; i++)
	{
		sum += x[i];
	}
	for (int i = 0; i < 6; i++)
	{
		x[i] = x[i] / sum;
	}
}


class trainRow
{
public:
	int number_of_words;
	int dictSize;
	int* data;
	double* emotion;//anger,disgust,fear,joy,sad,surprise

	trainRow();
	trainRow(int dictSize);
	trainRow(const trainRow& TR);//带指针数组的类一定要有拷贝构造函数
	~trainRow();
};
trainRow::trainRow()
{
	dictSize = 0;
	number_of_words = 0;
	emotion = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion[i] = 0;
	}
}
trainRow::trainRow(int dictSize)
{
	number_of_words = 0;
	this->dictSize = dictSize;
	data = new int[dictSize];
	for (int i = 0; i < dictSize; i++)
	{
		data[i] = 0;
	}
	emotion = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion[i] = 0;
	}
}
trainRow::trainRow(const trainRow& TR)
{
	number_of_words = TR.number_of_words;
	dictSize = TR.dictSize;
	data = new int[TR.dictSize];
	for (int i = 0; i < TR.dictSize; i++)
	{
		data[i] = TR.data[i];
	}
	emotion = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion[i] = TR.emotion[i];
	}
}
trainRow::~trainRow()
{
	delete[] data;
	delete[] emotion;
}

class trainCase
{
public:
	int dictSize;			//不重复记录单词数，矩阵的列数 
	int rowCnt;				//一共多少训练用的文章，矩阵的行数 
	vector<string> wordsVC;	//不重复记录单词 
	vector<trainRow*> matrix;//onehot矩阵 TF矩阵 感情向量

	trainCase();
	trainCase(const string &filename);
	~trainCase();
	void get_words(const string &filename);	//获取wordVC向量,同时充当了构造函数
	void write_matrix(const string &filename);//获取onehot矩阵，emotions数组
	friend void operator<<(ostream& os, const trainCase& TC);
};
trainCase::trainCase()
{
	dictSize = rowCnt = 0;
}
trainCase::trainCase(const string &filename)
{
	get_words(filename);//Initialize dictSize/rowCnt/wordsVC
	write_matrix(filename);//Initialize matrix
}
trainCase::~trainCase()
{
	for (int i = 0; i < matrix.size(); i++)
	{
		delete matrix[i];
	}
}
void trainCase::get_words(const string &filename)
{
	rowCnt = 0;
	ifstream fin(filename.c_str());
	string s;
	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		rowCnt++;
		int wordCnt = 0;
		string words, word;
		trainRow* TR = new trainRow();

		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		ss.str(words.c_str());
		while (ss >> word)
		{
			wordCnt++;
			if (find(wordsVC.begin(), wordsVC.end(), word) == wordsVC.end())//从未出现过的单词 
			{
				wordsVC.push_back(word);//记录到vector里
			}
		}
		TR->number_of_words = wordCnt;
		matrix.push_back(TR);
	}
	dictSize = wordsVC.size();
}
void trainCase::write_matrix(const string &filename)
{
	//打开要读的文件 
	ifstream fin(filename);
	int currRow = 0;//当前处理的行号
	string s;//用于记录读取到的每一行的字符串 

	getline(fin, s);//去掉说明 
	while (getline(fin, s))
	{
		matrix[currRow]->dictSize = dictSize;
		matrix[currRow]->data = new int[dictSize];
		for (int i = 0; i < dictSize; i++)
		{
			matrix[currRow]->data[i] = 0;
		}

		string words, word;
		istringstream ss(s);
		getline(ss, words, ',');//获得单词序列
		istringstream Wss(words);
		while (Wss >> word)//获得当前行的记录 
		{
			//在 string_vc里找出outstr，返回索引
			int location = find_word_in_vc(word, wordsVC);
			if (location != -1)
			{
				matrix[currRow]->data[location] += 1;
			}
		}
		for (int i = 0; i < 6; i++)
		{
			string tmpEmotion;
			getline(ss, tmpEmotion, ',');
			matrix[currRow]->emotion[i] = atof(tmpEmotion.c_str());
		}
		currRow++;
	}
}
void operator<<(ostream& os, const trainCase& TC)
{
	for (int i = 0; i < TC.rowCnt; i++)
	{
		for (int j = 0; j < TC.dictSize; j++)
		{
			os << TC.matrix[i]->data[j] << ' ';
		}
		os << endl;
		for (int j = 0; j < 6; j++)
		{
			os << TC.matrix[i]->emotion[j] << ' ';
		}
		os << endl;
	}
}

class testCase
{
public:
	double* emotion_posibility;//6个感情的预测概率
	//vector<string> test_words_vc;//所有词
	string words;//测试文本
	int newWords;//出现的词典里没有的新词数,不重复
	queue<int> words_location;

	testCase();
	testCase(trainCase& TC, string& words, double lp);
	testCase(const testCase& TC);
	~testCase();

	int test_word_in_trainRow(int location, trainRow* TR, vector<string> &VC);
	void get_words_location(vector<string> &VC);
	void newWords_counter(vector<string>& VC);//对于读出的一个词考虑是否加到newWords
	//返回<分子，分母>
	pair<double, double> row_cal_emotion(vector<string> &VC, trainRow* TR, double lp);
	friend void operator<< (ostream& os, const testCase& TC);
};
testCase::testCase()
{
	emotion_posibility = new double[6];
}
testCase::testCase(const testCase& TC)
{
	words = TC.words;
	newWords = TC.newWords;
	emotion_posibility = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion_posibility[i] = TC.emotion_posibility[i];
	}
}
testCase::testCase(trainCase& TC, string& words, double lp)
{
	this->words = words;
	get_words_location(TC.wordsVC);
	emotion_posibility = new double[6];
	for (int i = 0; i < 6; i++)
	{
		emotion_posibility[i] = 0;
	}
	double nu[630];//对每一行的训练数据算出的分子
	double de[630];//对每一行的训练数据算出的分母

	newWords_counter(TC.wordsVC);
	pair<double, double> tmp_pair;
	for (int i = 0; i < TC.rowCnt; i++)
	{
		tmp_pair = row_cal_emotion(TC.wordsVC, TC.matrix[i], lp);
		nu[i] = tmp_pair.first;
		de[i] = tmp_pair.second;
	}
	//debug
	//for (int i = 0; i < TC.rowCnt; i++)
	//{
	//	cout << nu[i] << ' ' << de[i] << endl;
	//}

	//计算出六个感情
	for (int i = 0; i < TC.rowCnt; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			emotion_posibility[j] += (nu[i] / de[i]) * TC.matrix[i]->emotion[j];
		}
	}
	//使感情概率和为1
	normalize_6(emotion_posibility);
}
testCase::~testCase()
{
	delete[] emotion_posibility;
}
int testCase::test_word_in_trainRow(int location, trainRow* TR, vector<string> &VC)
{
	if (location == -1)
	{
		return 0;
	}
	else
	{
		return TR->data[location];
	}
}
void testCase::get_words_location(vector<string> &VC)
{
	string word;
	istringstream iss(words);
	while (iss >> word)
	{
		words_location.push(find_word_in_vc(word, VC));
	}
}
void testCase::newWords_counter(vector<string> &VC)
{
	vector<int> newWordsVC;
	int it_end = words_location.size();
	for (int it = 0; it < it_end; it++)
	{
		int tmp_location = words_location.front();
		if (tmp_location == -1 && find(newWordsVC.begin(), newWordsVC.end(),tmp_location) == newWordsVC.end())
		{
			newWordsVC.push_back(tmp_location);
			newWords++;
		}
		words_location.pop();
		words_location.push(tmp_location);
	}
}
pair<double, double> testCase::row_cal_emotion(vector<string> &VC, trainRow* TR, double lp)
{
	double tmp_de = 1;//分母
	double tmp_nu = 1;//分子

	//分母每次乘上去的值=测试文本对于训练集不重复新词个数*lp+训练集的不重复个数*lp+训练文本词总数
	double tmp_de_factor = (newWords + VC.size()) * lp + TR->number_of_words;
	int it_end = words_location.size();
	for (int it = 0; it < it_end; it++)
	{
		int tmp_location = words_location.front();
		tmp_nu *= test_word_in_trainRow(tmp_location, TR, VC) + lp;//词出现的词数+lp
		tmp_de *= tmp_de_factor;
		words_location.pop();
		words_location.push(tmp_location);
	}

	pair<double, double> res(tmp_nu, tmp_de);
	return res;
}
void operator<< (ostream& os, const testCase& TC)
{
	for (int i = 0; i < 6; i++)
	{
		os << TC.emotion_posibility[i] << '\t';
	}
	os << endl;
}

void validHandle(ostream &os, string &valid_file_name, trainCase &TC, double lp)
{
	ifstream fin(valid_file_name);
	string s;
	getline(fin, s);//去掉说明
	while (getline(fin, s))
	{
		string words;
		istringstream iss(s);
		getline(iss, words, ',');
		iss.clear();
		testCase testcase(TC, words, lp);
		os << testcase;
	}
}
void testHandle(ostream &os, string &test_file_name, trainCase &TC, double lp)
{
	ifstream fin(test_file_name);
	string s;
	getline(fin, s);//去掉说明
	while (getline(fin, s))
	{
		string words;
		istringstream iss(s);
		getline(iss, words, ',');//去掉数字
		getline(iss, words, ',');
		testCase testcase(TC, words, lp);
		os << testcase;
	}
}

int main()
{
	string train_file_name = "train_set.csv";
	string valid_file_name = "validation_set.csv";
	string test_file_name = "test_set.csv";
	string validres_file_name = "";
	string testres_file_name = "test_res.txt";
	double lp = 0.031;
	int loglp = 0;
	int norm_type = 0;

	trainCase traincase(train_file_name);

	//ofstream fout_t("train_data.txt");
	//fout_t << traincase;

	cout << "log(lp) to ?" << endl;
	cin >> loglp;

	for (int i = 0; i < loglp; i++)
	{
		stringstream ss;
		ss << "fv";
		ss << i;
		ss << ".txt";
		ss >> validres_file_name;
		ofstream f_valid_out(validres_file_name);

		validHandle(f_valid_out, valid_file_name, traincase, lp);
		cout << i << ':' << "lp = " << lp << " is completed!" << endl;
		lp = lp + 0.0001;
	}

	//ofstream f_test_out(testres_file_name);
	//testHandle(f_test_out, valid_file_name, traincase, lp, norm_type);

	system("pause");
	return 0;
}

