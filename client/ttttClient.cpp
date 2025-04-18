#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <conio.h>

#include <future>
#include <chrono>
#include <locale>

#include <boost/multiprecision/cpp_int.hpp>

#include "b64Enc.h"
#include "Help.h"
#include "RSA.h"
#include "authData.h"
#include "UsersKeyControl.h"
#include "cFormRequest.h"
#include "msgControl.h"

using namespace boost;
using namespace std;

UsersKeyControl users_keys;
MessageControl msg_controller;
// RSA rsa(2048);
//extern string login;
//extern string passwd;
//extern string filename;

bool AuthorizateClient(tcp::socket& sock)
{
    std::string message = std::format("<login>{}</login><passwd>{}</passwd><type>login</type>{}", login, passwd, rsa.getPubKeys());

    sendText(sock, message);
    std::string data = read_all(sock);
    cout << data << endl;
    if (data == "authorization successful")
    {
        return true;
    }
    return false;
}

void listenerResponse(tcp::socket& sock)
{
    while (true)
    {
        std::string data = read_all(sock);
   //     data = ConvertToUTF8(data);
      //  data = base64ToStr(data);
        auto [from, to, type, dataStr, pubK, modK] = parse_resp(data);
        if (type == "msg")
        {
            /*cout << "after recived: " << dataStr << endl;
            cout << "md5: " << computeMD5(dataStr) << endl;*/
          //  dataStr = ConvertToUTF8(dataStr);
            dataStr = rsa.decrypt(dataStr);
            dataStr = hex_to_string(dataStr);
            string date = getCurrentDateTime();
            std::string msg = format("{} {} Message from {}: {}", computeMD5(dataStr), date, from, dataStr);
            cout << msg << endl;
            msg_controller.addMessage(from, to, dataStr, date, false);
        }
        else if (type == "confirmation")
        {
            cout << "message delivered" << endl;
        }
        else if (type == "publKeys")
        {
            users_keys.append(from, cpp_int(pubK), cpp_int(modK));
        }
    }
}



void controller(tcp::socket& sock)
{
 //  SetConsoleOutputCP(CP_UTF8);
    cout << "1. Write user \n 2. Show chats \n 3. Exit" << endl;
    while (true)
    {
        int i = readCommand();

        if (i == 1)
        {
            UsersKeyControl::Keys keys;
            cout << "Enter user name:" << endl;
            string user;
            cin>> user;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            string req1 = FormRequest::fPubKeys(user, "null", "null");
           // req1 = strToBase64(req1);
            send_all(sock, stringToVectorChar(req1));

            const auto start = chrono::steady_clock::now();
            const auto timeout = chrono::seconds(5);
            bool keysReceived = false;

            cout << "Start wait keys..." << endl;
            // Проверяем каждые 500 мс в течение 5 секунд
            while (chrono::steady_clock::now() - start < timeout) {  // chrono::steady_clock::now() - start < timeout
                keys = users_keys.getKeys(user);
                if (keys != nullptr) {
                    keysReceived = true;
                    break;
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }
            if (keysReceived)
            {
                cout << format("Encryption keys received for user {}", user) << endl;
                cout << format("publicKey: {}", to_string(keys.publicK)) << endl;
                
                msg_controller.showMessages(login, user);

                while (true)
                {
                    cout << format("Enter message for {} :", user) << endl;
                    string message;
                    std::getline(std::cin, message);

                 //   message = ConvertToUTF8(message);
                    msg_controller.addMessage(login, user, message, getCurrentDateTime(), true);
                    if (message == "/close")
                    {
                        cout << format("Exit from chat with {}", user) << endl;
                        break;
                    }
                  //  message = strToBase64(message);
                    cout << "md5: " << computeMD5(message) << endl;
                  //  cout << "msg before send bs64: " << strToBase64(message) << endl;
                   
                    
                    
                    message = string_to_hex(message);
                    message = rsa.encrypt(message, keys.publicK, keys.modulusK);

              //      req = strToBase64(req);
                    send_all(sock, stringToVectorChar(FormRequest::fMessage(user, message)));
                }
            }
            else
            {
               cout << format("Keys not found for user {} or user not exist", user) << endl;
               continue;
            }
        }
        else if (i == 2)
        {
            
        }
        else if (i == 3)
        {
            exit(0);
        }
        else
        {
            cout << "Unknown command" << endl;
        }
    }
}


int main() {
    //SetConsoleOutputCP(CP_UTF8);
    //SetConsoleCP(CP_UTF8);    // *
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);



    loadAuthenticatorFile(); // загрузка Login, pass, RSA ключей(создание)
    
    cout << "data: " << rsa.getPubKeys() << endl;

    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("81.177.142.197", std::to_string(1212));

        tcp::socket socket(io_context);

        bool connected = false;

        const auto start = chrono::steady_clock::now();
        const auto timeout = chrono::seconds(5);
        
        while (chrono::steady_clock::now() - start < timeout)
        {
            try {
                boost::asio::connect(socket, endpoints);
                connected = true;
                break;
            }
            catch (const boost::system::system_error& e) {
                std::cerr << "Ошибка подключения: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
        if (!connected)
        {
            cout << "Cant connect to server" << endl;
            _getch();
            return 0;
        }
        if (AuthorizateClient(socket))
        {
            std::thread th_client1([&]() {
                listenerResponse(socket);
                });
            th_client1.detach();

            std::thread th_client2([&]() {
                controller(socket);
                });
            th_client2.join();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Ошибка клиента: " << e.what() << std::endl;
    }

    return 0;
}