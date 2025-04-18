#pragma once
#include <string>
#include <vector>
#include <memory>
#include <format>
#include <iostream>
#include <tuple>
#include <map>
#include <fstream>
#define FILE_LOG "data.tmp"
using namespace std;




class Message {
protected:
    std::string owner;
    std::string content;
    std::string date;
public:
    Message(const std::string& owner, const std::string& content, const std::string& date)
        : owner(owner), content(content), date(date) {}

    virtual ~Message() = default;

    virtual void show() const = 0;

    virtual string getMSG_str() const = 0;
};

class ReceivedMessage : public Message {
public:
    ReceivedMessage(const std::string& owner, const std::string& content, const std::string& date)
        : Message(owner, content, date) {}
    void show() const override;
    string getMSG_str() const override;
};

class SentMessage : public Message {
public:
    SentMessage(const std::string& owner, const std::string& content, const std::string& date)
        : Message(owner, content, date) {}

    void show() const override;
    string getMSG_str() const override;
};


// класс управления сообщениями
class MessageControl {
   // std::vector<std::unique_ptr<Message>> chats;
    multimap< tuple<string, string>, std::unique_ptr<Message> > chats;
    bool existChat(string user1, string user2);
    vector<std::unique_ptr<Message>> getHistory(string user1, string user2);
    mutex mtx_read;
    mutex mtx_write;
public:

    void loadMsgBase();
    void saveMsgBase();

    // добавление сообщений
    void addMessage(string user1, string user2, string data_context, string date, bool sending);
    // вывод
    void showMessages(string user1, string user2);
};
