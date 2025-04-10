#pragma once

#include <iostream>
#include <format>

using namespace std;

class FormRequest
{

public:
	static string fPubKeys(string targetName, string pK, string mK);
	static string fMessage(string to, string data);
};
