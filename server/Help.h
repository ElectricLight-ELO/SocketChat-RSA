#pragma once

#include <unistd.h>
#include <termios.h>
#include <cstdio>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <boost/regex.hpp>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <cctype>
#include <boost/multiprecision/cpp_int.hpp>

// Объявления нешаблонных функций
size_t parse_index(const std::string& spec);
std::tuple<std::string, std::string, std::string, std::string, std::string> parseLogin(const std::string& text);
std::tuple<std::string, std::string, std::string, std::string, std::string, std::string> parse_resp(const std::string& text);
std::string vectorCharToString(const std::vector<char>& vec);
std::vector<char> stringToVectorChar(const std::string& str);
void send_all(boost::asio::ip::tcp::socket& socket, const std::vector<char>& data);
std::string read_all(boost::asio::ip::tcp::socket& socket);
void send_message(boost::asio::ip::tcp::socket& socket, const std::string& message);
char getch();

// Шаблонные функции
template <typename T>
std::string arg_to_string(T&& value) {
    std::ostringstream oss;
    oss << std::forward<T>(value);
    return oss.str();
}

template <typename... Args>
std::string format(const std::string& fmt, Args&&... args) {
    std::vector<std::string> arguments = { arg_to_string(std::forward<Args>(args))... };
    std::string result;
    size_t current_arg = 0;
    size_t len = fmt.size();

    for (size_t i = 0; i < len; ++i) {
        if (fmt[i] == '{') {
            if (i + 1 < len && fmt[i + 1] == '{') {
                result += '{';
                ++i;
            }
            else {
                size_t end = fmt.find('}', i);
                if (end == std::string::npos) {
                    throw std::runtime_error("Unclosed '{' at position " + std::to_string(i));
                }
                std::string spec = fmt.substr(i + 1, end - i - 1);
                size_t index = spec.empty() ? current_arg++ : parse_index(spec);

                if (index >= arguments.size()) {
                    throw std::out_of_range("Argument index " + std::to_string(index) + " out of range");
                }

                result += arguments[index];
                i = end;
            }
        }
        else if (fmt[i] == '}') {
            if (i + 1 < len && fmt[i + 1] == '}') {
                result += '}';
                ++i;
            }
            else {
                throw std::runtime_error("Unopened '}' at position " + std::to_string(i));
            }
        }
        else {
            result += fmt[i];
        }
    }

    return result;
}