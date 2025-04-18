#include "b64Enc.h"

inline bool is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}



std::string strToBase64(const std::string& input) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(input.begin()), It(input.end()));
    return tmp.append((3 - input.size() % 3) % 3, '=');
}

std::string base64ToStr(const std::string& input) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    auto tmp = input;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), '='), tmp.end());
    return std::string(It(tmp.begin()), It(tmp.end()));
}

std::string computeMD5(const std::string& data)
{
    // Создаём объект для вычисления MD5
    boost::uuids::detail::md5 hash;
    boost::uuids::detail::md5::digest_type digest;

    // Обработка входного текста (байты строки передаются на обработку)
    hash.process_bytes(data.data(), data.size());
    hash.get_digest(digest);

    // Получаем указатель на данные хэша (16 байт).
    const char* digestBytes = reinterpret_cast<const char*>(&digest);

    // Преобразуем байтовый массив в шестнадцатеричную строку.
    std::string result;
    boost::algorithm::hex(digestBytes, digestBytes + sizeof(boost::uuids::detail::md5::digest_type),
        std::back_inserter(result));
    return result;
}

std::string string_to_hex(const std::string& input)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (unsigned char c : input) {
        oss << std::setw(2) << static_cast<int>(c);
    }

    return oss.str();
}

std::string hex_to_string(const std::string& hex)
{
    // Проверка на чётность длины
    if (hex.length() % 2 != 0) {
        return "";
    }

    std::string result;
    for (size_t i = 0; i < hex.length(); i += 2) {
        // Проверка валидности символов
        if (!isxdigit(hex[i]) || !isxdigit(hex[i + 1])) {
            return "";
        }

        // Парсинг HEX-пары
        int byte_value =
            (std::isdigit(hex[i]) ? hex[i] - '0' : std::toupper(hex[i]) - 'A' + 10) * 16 +
            (std::isdigit(hex[i + 1]) ? hex[i + 1] - '0' : std::toupper(hex[i + 1]) - 'A' + 10);

        result.push_back(static_cast<char>(byte_value));
    }

    return result;
}

std::string ConvertToUTF8(const std::string& input)
{
#ifdef _WIN32
    // Конвертация из CP-1251 в UTF-8 (если консоль не настроена на UTF-8)
    int size = MultiByteToWideChar(1251, 0, input.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(1251, 0, input.c_str(), -1, &wstr[0], size);

    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8_str(utf8_size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], utf8_size, nullptr, nullptr);
    return utf8_str;
#else
    return input; // На Linux/macOS ввод уже в UTF-8
#endif
}

void PrintConsoleEncoding()
{
    // Получаем кодировки консоли
    UINT input_cp = GetConsoleCP();     // Кодировка ввода
    UINT output_cp = GetConsoleOutputCP(); // Кодировка вывода

    std::cout << "Console Input Codepage: " << input_cp << "\n"
        << "Console Output Codepage: " << output_cp << "\n";

    // Расшифровка основных кодировок
    std::cout << "Decoded:\n";
    std::cout << "Input: ";
    switch (input_cp) {
    case 65001: std::cout << "UTF-8"; break;
    case 1251: std::cout << "Windows-1251 (Cyrillic)"; break;
    case 866: std::cout << "CP-866 (DOS Cyrillic)"; break;
    default: std::cout << "Unknown (" << input_cp << ")";
    }

    std::cout << "\nOutput: ";
    switch (output_cp) {
    case 65001: std::cout << "UTF-8"; break;
    case 1251: std::cout << "Windows-1251 (Cyrillic)"; break;
    case 866: std::cout << "CP-866 (DOS Cyrillic)"; break;
    default: std::cout << "Unknown (" << output_cp << ")";
    }
    std::cout << std::endl;
}
