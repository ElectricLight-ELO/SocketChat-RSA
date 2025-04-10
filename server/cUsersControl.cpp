#include "cUsersControl.h"

void usersControl::createNewUser(string& us, string& pwd, string& pubK, string& modK, std::shared_ptr<tcp::socket> cliSock, int& indexUser)
{
    cUser newUser(us, pwd, pubK, modK, cliSock);
    users.push_back(newUser);

    indexUser = users.size() - 1;

}

bool usersControl::userExist(string us)
{
    for (int i = 0; i < users.size(); i++)
    {
        if (users[i].user == us)
        {
            return true;
        }
    }
    return false;
}

void usersControl::userOff(int ind)
{
    users[ind].ClientSocket = nullptr;
}

int usersControl::userOn(string us, string pwd, string& pubK, string modK, std::shared_ptr<tcp::socket> cliSock)
{
    for (int i = 0; i < users.size(); i++)  {
        if (users[i].user == us && users[i].passwd == pwd && users[i].public_key == pubK && users[i].modulus_key == modK)
        {
            users[i].ClientSocket = cliSock;
            return i;
        }

            
    }

    return -1;
}

std::shared_ptr<tcp::socket> usersControl::getUser_socket(string us_name)
{
    for (auto& us : users)
    {
        if (us.user == us_name)
        {
            return us.ClientSocket;
        }
    }
    return nullptr;
}

std::tuple<std::string, std::string> usersControl::getKeys(const string& name)
{
    for (auto& us : users)
    {
        if (us.user == name)
        {
            return std::make_tuple(us.public_key, us.modulus_key);
        }
    }

    return std::make_tuple("", "");
}
