#include "UsersKeyControl.h"

void UsersKeyControl::append(string name, cpp_int pub, cpp_int mod) {
    dictUsersKeys[name] = Keys(pub, mod);
}

UsersKeyControl::Keys UsersKeyControl::getKeys(string name) {
    auto it = dictUsersKeys.find(name);
    return (it != dictUsersKeys.end()) ? it->second : Keys();
}

