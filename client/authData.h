#pragma once
#include "RSA.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <boost/regex.hpp>
#include <tuple>

using namespace std;
extern string filename;
extern std::string login;
extern std::string passwd;
extern RSA rsa;

using namespace boost;

std::tuple<string, string, string, string, cpp_int, cpp_int> parse_resp(const std::string& input);
std::tuple<string, string, cpp_int, cpp_int, cpp_int> parse_keys(const std::string& input);
std::string readFileToString(const std::string& filename);
bool loadAuthenticatorFile();