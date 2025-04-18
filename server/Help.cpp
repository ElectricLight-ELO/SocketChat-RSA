#include "Help.h"



// ������� ������� ��������� �� �������������
size_t parse_index(const std::string& spec) {
    for (char c : spec) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("Invalid format specifier: " + spec);
        }
    }
    return static_cast<size_t>(std::stoul(spec));
}

std::tuple<std::string, std::string, std::string, std::string, std::string>
parseLogin(const std::string& text) {
    boost::regex pattern("<login>(.*?)</login>"
        "<passwd>(.*?)</passwd>"
        "<type>(.*?)</type>"
        "<publicKey>(.*?)</publicKey>"
        "<modulus>(.*?)</modulus>");

    boost::smatch match;
    if (boost::regex_search(text, match, pattern)) {
        // match[0] �������� �� ����������, � � match[1] �� match[5] � ������ ������
        return std::make_tuple(match[1].str(), match[2].str(), match[3].str(),
            match[4].str(), match[5].str());
    }
    // ���� ���������� �� �������, ����� ��������� ���������� ��� ������� ������ � ������� ��������
    return std::make_tuple("", "", "", "", "");
}

std::tuple<std::string, std::string, std::string, std::string, std::string, std::string>
parse_resp(const std::string& text) {
    boost::regex pattern(
        "<from>(.*?)</from>"
        "<to>(.*?)</to>"
        "<type>(.*?)</type>"
        "<data>(.*?)</data>"
        "<publicKey>(.*?)</publicKey>"
        "<modulus>(.*?)</modulus>");

    boost::smatch match;
    if (boost::regex_search(text, match, pattern)) {
        // match[0] �������� �� ����������, � � match[1] �� match[5] � ������ ������
        return std::make_tuple(match[1].str(), match[2].str(), match[3].str(),
            match[4].str(), match[5].str(), match[6].str());
    }
    // ���� ���������� �� �������, ����� ��������� ���������� ��� ������� ������ � ������� ��������
    return std::make_tuple("", "", "", "", "", "");
}

using boost::asio::ip::tcp;
constexpr size_t CHUNK_SIZE = 1024;

std::string vectorCharToString(const std::vector<char>& vec) {
    return std::string(vec.begin(), vec.end());
}

// ����� ��� �������������� string � vector<char>
std::vector<char> stringToVectorChar(const std::string& str) {
    return std::vector<char>(str.begin(), str.end());
}

void send_all(tcp::socket& socket, const std::vector<char>& data)
{
    // �������� ������� ������
    uint32_t data_length = static_cast<uint32_t>(data.size());
    uint32_t network_length = htonl(data_length);
    boost::asio::write(socket, boost::asio::buffer(&network_length, sizeof(network_length)));

    // �������� ������ �������
    size_t bytes_sent = 0;
    size_t chunk_size = 1024;

    while (bytes_sent < data.size()) {
        size_t remaining = data.size() - bytes_sent;
        size_t current_chunk = std::min(chunk_size, remaining);

        bytes_sent += boost::asio::write(socket, boost::asio::buffer(data.data() + bytes_sent, current_chunk));
    }
}

std::string read_all(tcp::socket& socket)
{
    // ������ ������� ������
    uint32_t network_length;
    boost::asio::read(socket, boost::asio::buffer(&network_length, sizeof(network_length)));
    uint32_t data_length = ntohl(network_length);

    // ������ ������ �������
    std::vector<char> buffer(data_length);
    size_t bytes_read = 0;
    size_t chunk_size = 1024;

    while (bytes_read < data_length) {
        size_t remaining = data_length - bytes_read;
        size_t current_chunk = std::min(chunk_size, remaining);

        bytes_read += boost::asio::read(socket, boost::asio::buffer(buffer.data() + bytes_read, current_chunk));
    }

    // �������������� � ������
    return std::string(buffer.begin(), buffer.end());
}

void send_message(tcp::socket& socket, const std::string& message)
{
    std::vector<char> data(message.begin(), message.end());
    try {
        send_all(socket, data);
    }
    catch (const boost::system::system_error& ex) {
        std::cerr << "Error sending message: " << ex.what() << std::endl;
    }
}

char getch() {
    char ch = 0;
    struct termios oldSettings, newSettings;

    // �������� ������� ��������� ���������
    if (tcgetattr(STDIN_FILENO, &oldSettings) < 0) {
        perror("tcgetattr");
        return ch;
    }

    newSettings = oldSettings;
    // ��������� ������������ ����� � ���
    newSettings.c_lflag &= ~(ICANON | ECHO);
    newSettings.c_cc[VMIN] = 1;  // ������� ���� �� 1 ������
    newSettings.c_cc[VTIME] = 0; // ��� ��������

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newSettings) < 0) {
        perror("tcsetattr");
        return ch;
    }

    // ������ ���� ������
    if (read(STDIN_FILENO, &ch, 1) < 0) {
        perror("read");
    }

    // ��������������� �������� ��������� ���������
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings) < 0) {
        perror("tcsetattr");
    }

    return ch;
}