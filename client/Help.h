#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>

#define FILENAME "logfile.xml"

using boost::asio::ip::tcp;
constexpr size_t CHUNK_SIZE = 1024;

std::string vectorCharToString(const std::vector<char>& vec) {
    return std::string(vec.begin(), vec.end());
}

// Метод для преобразования string в vector<char>
std::vector<char> stringToVectorChar(const std::string& str) {
    return std::vector<char>(str.begin(), str.end());
}

int readCommand()
{
    std::cout << "Enter command: ";
    int i = 0;
    std::cin >> i;

    return i;
}
std::string getCurrentDateTime() {
    using namespace std::chrono;

    // Получаем текущее время с точностью до секунд
    auto now = system_clock::now();
    auto time = system_clock::to_time_t(now);

    // Безопасное преобразование для разных платформ
    std::tm tm_buffer;
#if defined(_WIN32)
    localtime_s(&tm_buffer, &time);
#else
    localtime_r(&time, &tm_buffer); // POSIX-совместимая версия
#endif

// Форматируем в строку
    std::ostringstream oss;
    oss << std::put_time(&tm_buffer, "[%d-%m-%y %H-%M-%S]");
    return oss.str();
}
// Клиент
void send_all(tcp::socket& socket, std::vector<char> data) {
    try {
        // Отправляем длину данных (4 байта)
        uint32_t dataLength = static_cast<uint32_t>(data.size());
        uint32_t lengthNetworkOrder = htonl(dataLength);
        boost::asio::write(socket, boost::asio::buffer(&lengthNetworkOrder, sizeof(lengthNetworkOrder)));

        // Отправляем данные по частям
        size_t bytesSent = 0;
        const size_t chunkSize = 1024;

        while (bytesSent < data.size()) {
            size_t remaining = data.size() - bytesSent;
            size_t currentChunk = std::min(chunkSize, remaining);

            boost::asio::write(socket, boost::asio::buffer(&data[bytesSent], currentChunk));
            bytesSent += currentChunk;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error while sending data: " << ex.what() << std::endl;
    }
}

void sendText(tcp::socket& socket, std::string text)
{
    std::vector<char> bytes = stringToVectorChar(text);
    send_all(socket, bytes);
}

// Чтение фиксированного количества байт
void readBytes(tcp::socket& socket, std::vector<char>& buffer, size_t count) {
    size_t bytesRead = 0;

    while (bytesRead < count) {
        size_t read = boost::asio::read(socket, boost::asio::buffer(&buffer[bytesRead], count - bytesRead));
        if (read == 0) {
            throw std::runtime_error("End of stream reached");
        }
        bytesRead += read;
    }
}

// Функция для чтения всего сообщения
std::string read_all(tcp::socket& socket) {
    try {
        // Читаем длину данных (4 байта)
        uint32_t lengthNetworkOrder;
        boost::asio::read(socket, boost::asio::buffer(&lengthNetworkOrder, sizeof(lengthNetworkOrder)));
        uint32_t dataLength = ntohl(lengthNetworkOrder);

        // Читаем данные по частям
        std::vector<char> buffer(1024);
        size_t bytesRead = 0;
        std::vector<char> data;

        while (bytesRead < dataLength) {
            size_t remaining = dataLength - bytesRead;
            size_t currentChunk = std::min(buffer.size(), remaining);

            size_t read = boost::asio::read(socket, boost::asio::buffer(buffer.data(), currentChunk));
            if (read == 0) {
                throw std::runtime_error("End of stream reached");
            }

            data.insert(data.end(), buffer.begin(), buffer.begin() + read);
            bytesRead += read;
        }

        return std::string(data.begin(), data.end());
    }
    catch (const std::exception& ex) {
        std::cerr << "Error while reading data: " << ex.what() << std::endl;
        return {};
    }
}

std::string readMessage(tcp::socket& socket) {
    try {
        return read_all(socket);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error with reading message: " << ex.what() << std::endl;
        return {};
    }
}