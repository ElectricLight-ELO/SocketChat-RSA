#include <cstdio>
#include <iostream>
#include "Help.h"
#include <boost/asio.hpp>
#include <boost/regex.hpp>

#include "cUsersControl.h"
#include "cFormRequest.h"

using namespace std;

using boost::asio::ip::tcp;
FormRequest FReq;

auto userContr = make_shared<usersControl>();



void handle_client(std::shared_ptr<tcp::socket> socket) {
    // Используем -1, чтобы показать, что пользователь ещё не добавлен.
 //   setlocale(LC_ALL, "ru_RU.UTF-8");
    
    tcp::endpoint remote_ep = socket->remote_endpoint();
    // Извлекаем IP-адрес
    std::string client_ip = remote_ep.address().to_string();

    std::locale::global(std::locale("ru_RU.UTF-8"));
    int clientInfoIndex = -1;
    bool isAuthenticated = false;
    try {
        while (true)
        {
            string buffer = read_all(*socket);

            std::cout << "Received message: " << buffer << std::endl;

            // Если пользователь не авторизован и получено сообщение для логина
            if (!isAuthenticated && buffer.find("</login>") != std::string::npos) {
                // Предполагается, что parseLogin возвращает кортеж: (login, passwd, type, publicKey, modulus)
                auto [login, passwd, type, publicKey, modulus] = parseLogin(buffer);
                if (!userContr->userExist(login)) {
                    // Создаём нового пользователя и добавляем его в вектор
                    userContr->createNewUser(login, passwd, publicKey, modulus, socket, clientInfoIndex);

                    cout << format("new user registrated, IP: {}", client_ip) << endl;
                }
                else {
                    clientInfoIndex = userContr->userOn(login, passwd, publicKey, modulus, socket);
                    if (clientInfoIndex == -1)
                    {
                        send_all(*socket, stringToVectorChar("Wrong login&pass&enc_keys or this login exist"));
                        break;
                    }
                   cout << format("existing user '{}' connected. Index: {}", login, clientInfoIndex) << endl;
                }
                isAuthenticated = true;

                send_all(*socket, stringToVectorChar("authorization successful"));
            }
            else if (isAuthenticated) {
                auto [from, to, type, data, publicKey, modulus] = parse_resp(buffer);
                if (type == "publKeys")
                {
                    auto [pubK_trg, modK_trg] = userContr->getKeys(to);

                    string req = FormRequest::fPubKeys(to, pubK_trg, modK_trg);
                    send_all(*socket, stringToVectorChar(req));
                }
                else if(type == "msg")
                {
                    std::shared_ptr<tcp::socket> sock = userContr->getUser_socket(to);
                    // FormRequest::fMessage(from, to, data)
                    send_all(*sock, stringToVectorChar(buffer));
                }
                else if(type == "confirmation")
                {

                }
            }
        }
    }
    catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            std::cout << "Client disconnected" << std::endl;
        }
        else {
            std::cerr << "Error in processing client: " << e.what() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in processing client: " << e.what() << std::endl;
    }

    if (clientInfoIndex != -1) {
        userContr->userOff(clientInfoIndex);
        isAuthenticated = false;
        std::cerr << "Client disconnected " << std::endl;
    }
}


int main()
{
   // setlocale(0, "RU");
  //  std::locale::global(std::locale("ru_RU.UTF-8"));
    setlocale(LC_ALL, "ru_RU.CP1251");

    const short port = 1212;


    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

        std::cout << "Server started on port: " << port << std::endl;
        
        while (true) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            std::cout << "New connection" << std::endl;

            // Обработка клиента в отдельном потоке
            std::thread th_client(handle_client, socket);
            th_client.detach();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    getch();
    return 0;
}