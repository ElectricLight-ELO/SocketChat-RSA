#pragma once
#include <string>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
using namespace std;

using boost::asio::ip::tcp;

class usersControl
{
    struct cUser
    {
        std::string user;
        std::string passwd;
        std::shared_ptr<tcp::socket> ClientSocket;
        std::string public_key;
        std::string modulus_key;

        cUser(const std::string& user, const std::string& passwd,
            const std::string& public_key, const std::string& modulus_key,
            std::shared_ptr<tcp::socket> ClientSocket)
            : user(user), passwd(passwd), ClientSocket(ClientSocket),
            public_key(public_key), modulus_key(modulus_key)
        {}
    };
    vector<cUser> users;
public:
    void createNewUser(string& us, string& pwd, string& pubK, string& modK, std::shared_ptr<tcp::socket> cliSock, int& indexUser);
    bool userExist(string us);
    void userOff(int ind);
    int userOn(string us, string pwd, string& pubK, string modK, std::shared_ptr<tcp::socket> cliSock);
    std::shared_ptr<tcp::socket> getUser_socket(string us_name);
    std::tuple <std::string, std::string> getKeys(const string& name);
};