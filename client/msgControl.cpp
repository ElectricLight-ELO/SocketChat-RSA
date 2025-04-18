#include "msgControl.h"





void ReceivedMessage::show() const {
    std::cout << std::format("{} Message from {}: {}", date, owner, content) << endl;
}

string ReceivedMessage::getMSG_str() const
{
    return std::format("<from>{}</from><date>{}</date><content>{}</content>", owner, date, content);
}

void SentMessage::show() const {
    std::cout << std::format("{} My message: {}", date, content) << endl;
}

string SentMessage::getMSG_str() const
{
    return std::format("<from>MY</from><date>{}</date><content>{}</content>", date, content);
}

bool MessageControl::existChat(string user1, string user2)
{
    for (auto& [key, val] : chats)
    {
        auto [u1, u2] = key;
        if ((user1 == u1 && user2 == u2) ||
            (user1 == u2 && user2 == u1))
        {
            return true;
        }
    }

    return false;
}

vector<std::unique_ptr<Message>> MessageControl::getHistory(string user1, string user2)
{
    vector<std::unique_ptr<Message>> hist_ptr;

    for (auto& [key, val] : chats)
    {
        auto [u1, u2] = key;
        if ((user1 == u1 && user2 == u2) ||
            (user1 == u2 && user2 == u1))
        {
            hist_ptr.push_back(move(val));
        }
    }
    return hist_ptr;
}

void MessageControl::addMessage(string user1, string user2, string data_context, string date, bool sending)
{
    tuple<string, string> users = make_tuple(user1, user2);
    
    unique_ptr<Message> msg_ptr;
    if (sending)
    {
        msg_ptr = make_unique<SentMessage>(user2, data_context, date); // my messages
    }
    else
    {
        msg_ptr = make_unique<ReceivedMessage>(user1, data_context, date);
    }
    chats.insert({ users, move(msg_ptr) });
}

void MessageControl::showMessages(string user1, string user2)
{
    cout << "History messages with this user: " << endl;


    auto hist = getHistory(user1, user2);
    if (hist.size() == 0)
    {
        cout << "History messages with this user empty!!!" << endl;
        return;
    }
    for (auto& msgs_ptr : hist)
    {
        msgs_ptr->show();
    }
}

void MessageControl::loadMsgBase(){
    mtx_read.lock();
    ifstream file(FILE_LOG);
    if (file.is_open()) {
        string tmp;
        while (getline(file, tmp))
        {

        }
        file.close();
    }
    mtx_read.unlock();
}

void MessageControl::saveMsgBase() {
    mtx_write.lock();
    string file_data;
    for (auto& msgs : chats) {
       auto& d =  msgs.second;
       file_data += d->getMSG_str();
       file_data += "\n";
    }
    ofstream file(FILE_LOG);
    if (file.is_open()) {
        file << file_data;

        file.close();
    }
    mtx_write.unlock();
}