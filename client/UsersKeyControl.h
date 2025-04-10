#pragma once
#include <map>
#include <string>
#include <cstddef> // Добавляем для std::nullptr_t
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;
using namespace std;

class UsersKeyControl {
public:
    struct Keys {
        cpp_int publicK;
        cpp_int modulusK;
        bool valid; // Флаг валидности

        Keys() : publicK(0), modulusK(0), valid(false) {}
        Keys(cpp_int pK, cpp_int mK) : publicK(pK), modulusK(mK), valid(true) {}

        bool operator!=(std::nullptr_t) const {
            return valid;
        }

        bool operator==(std::nullptr_t) const {
            return !valid;
        }
    };

private:
    map<string, Keys> dictUsersKeys;

public:
    UsersKeyControl() = default;
    void append(string name, cpp_int pub, cpp_int mod);
    Keys getKeys(string name);
};