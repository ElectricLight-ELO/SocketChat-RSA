#include "authData.h"

string filename = "settings.xml";
std::string login;
std::string passwd;
RSA rsa(1024);

std::tuple<string, string, string, string, cpp_int, cpp_int> parse_resp(const std::string& input) {
    
    const regex pattern(
        "<from>(.*?)</from>"
        "<to>(.*?)</to>"
        "<type>(.*?)</type>"
        "<data>(.*?)</data>"
        "<publicKey>(.*?)</publicKey>"
        "<modulus>(.*?)</modulus>"
    );

    smatch matches;

    if (!regex_search(input, matches, pattern) || matches.size() != 7) {
        throw std::runtime_error("Invalid input format");
    }

    // Преобразование строк в cpp_int
    string fromID(matches[1].str());
    string toID(matches[2].str());
    string type(matches[3].str());
    string data(matches[4].str());
    cpp_int public_key = (matches[5].str() == "null" ? cpp_int(0) : cpp_int(matches[5].str()));
    cpp_int modulus = (matches[6].str() == "null" ? cpp_int(0) : cpp_int(matches[6].str()));

    /*cpp_int public_key(NULL);
    cpp_int modulus(NULL);
    string c5 = matches[5].str();
    string c6 = matches[6].str();
    if (c5 != "null" || c6 != "null")
    {
        public_key = cpp_int(c5);
        modulus = cpp_int(c6);
    }*/
    

    return std::make_tuple(fromID, toID, type, data, public_key, modulus);
}

std::tuple<string, string, cpp_int, cpp_int, cpp_int> parse_keys(const std::string& input) {
    // Регулярное выражение для захвата значений
    const regex pattern(
        "<user>(.*?)</user>"
        "<passwd>(.*?)</passwd>"
        "<publicKey>(.*?)</publicKey>"
        "<modulus>(.*?)</modulus>"
        "<privateKey>(.*?)</privateKey>"
    );

    smatch matches;

    if (!regex_search(input, matches, pattern) || matches.size() != 6) {
        throw std::runtime_error("Invalid input format");
    }

    // Преобразование строк в cpp_int
    string login(matches[1].str());
    string passwd(matches[2].str());
    cpp_int public_key(matches[3].str());
    cpp_int modulus(matches[4].str());
    cpp_int private_key(matches[5].str());

    return std::make_tuple(login, passwd, public_key, modulus, private_key);
}

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename); // Открываем файл для чтения
    if (!file.is_open()) {         // Проверяем, успешно ли открыт файл
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return "";
    }

    // Используем stringstream для считывания всего содержимого файла
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str(); // Возвращаем содержимое файла в виде строки
}

bool loadAuthenticatorFile() {
    // Если файл уже существует – считываем его содержимое и настраиваем ключи
    if (std::filesystem::exists(filename)) {
        const std::string content = readFileToString(filename);
        auto [storedLogin, storedPass, pubKey, mod, privKey] = parse_keys(content);
        login = storedLogin;
        passwd = storedPass;
        rsa.setKeys(pubKey, mod, privKey);
        return false;
    }

    // Файл не существует – запрашиваем данные у пользователя и генерируем ключи
    std::cout << "Authenticator file does not exist...\n"
        << "Login/Register\n"
        << "Enter login: " << std::flush;
    std::cin >> login;
    std::cout << "Enter passwd: " << std::flush;
    std::cin >> passwd;

    // Открываем файл для записи (RAII: закрытие файла произойдёт автоматически)
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return true;
    }

    std::cout << "Pls wait... Chat generating keys" << std::endl;
    rsa.GenerateKey();

    // Формируем строку с данными аутентификации и ключами
    const std::string authenticData = std::vformat("<user>{}</user><passwd>{}</passwd>{}", std::make_format_args(login, passwd, rsa.getFullKeys()));
    file << authenticData;
    std::cout << "New authenticator data saved" << std::endl;

    return true;
}