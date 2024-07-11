// GateServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include "boost/lexical_cast.hpp"
int main()
{
	using namespace std;
	cout << "Enter your weight: ";
	float weight;
	cin >> weight;
	string gain = "A 10% increase raises ";
	string wt = boost::lexical_cast<string> (weight);
	gain = gain + wt + " to ";      // string operator()
	weight = 1.1 * weight;
	gain = gain + boost::lexical_cast<string>(weight) + ".";
	cout << gain << endl;
	system("pause");
	return 0;
}

