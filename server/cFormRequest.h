#pragma once
#include <iostream>

using namespace std;

class FormRequest
{

public:
	static string fPubKeys(string from, string pK, string mK);
	static string fMessage(string from, string to, string data);
};
