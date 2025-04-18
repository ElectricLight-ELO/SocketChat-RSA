#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c);


std::string strToBase64(const std::string& str);

std::string base64ToStr(const std::string& b64);


std::string computeMD5(const std::string& data);


std::string string_to_hex(const std::string& input);

std::string hex_to_string(const std::string& hex);


// debug func

std::string ConvertToUTF8(const std::string& input);

void PrintConsoleEncoding();