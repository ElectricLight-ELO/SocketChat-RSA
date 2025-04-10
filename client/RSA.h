#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <sstream>

#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;
using namespace std;
class RSA {
private:
    // Ключевые параметры
    cpp_int public_key;   // e
    cpp_int modulus;      // n
    cpp_int private_key;  // d
    

    int keyLength;
    // --------------------------
    // Вспомогательные функции
    // --------------------------

    // Расширенный алгоритм Евклида для нахождения обратного по модулю
    cpp_int modInverse(const cpp_int& a, const cpp_int& m) {
    cpp_int m0 = m;   // сохраняем оригинальное значение m для последующего использования
    cpp_int M = m;    // создаём модифицируемую копию m
    cpp_int A = a;
    cpp_int x0 = 0, x1 = 1;
    
    while (A > 1) {
        cpp_int q = A / M;
        cpp_int t = M;
        M = A % M;
        A = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    
    if (x1 < 0)
        x1 += m0;
    
    return x1;
}

    // Вычисление длины числа в битах (простая реализация)
    int bitLength(const cpp_int& n) {
        int bits = 0;
        cpp_int temp = n;
        while (temp != 0) {
            temp /= 2;
            bits++;
        }
        return bits;
    }

    // Генерация случайного числа типа cpp_int из диапазона [min, max]
    cpp_int randomBetween(const cpp_int& min, const cpp_int& max) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        cpp_int range = max - min + 1;
        int bits = bitLength(range);
        cpp_int result = 0;
        int num_blocks = (bits + 63) / 64;  // сколько 64-битных блоков нужно
        while (true) {
            result = 0;
            for (int i = 0; i < num_blocks; i++) {
                uint64_t rand_val = std::uniform_int_distribution<uint64_t>(0, std::numeric_limits<uint64_t>::max())(gen);
                result = (result << 64) | rand_val;
            }
            if (result < range)
                break;
        }
        return min + result;
    }

    // Вероятностный тест простоты Миллера–Рабина
    bool isProbablePrime(const cpp_int& n, int iterations = 25) {
        if (n < 2)
            return false;
        // Проверка на делимость небольшими простыми числами
        static int smallPrimes[12] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37 };
        for (int prime : smallPrimes) {
            if (n == prime)
                return true;
            if (n % prime == 0)
                return false;
        }
        // Представляем n-1 в виде d * 2^s
        cpp_int d = n - 1;
        int s = 0;
        while (d % 2 == 0) {
            d /= 2;
            s++;
        }
        for (int i = 0; i < iterations; i++) {
            cpp_int a = randomBetween(2, n - 2);
            cpp_int x = powm(a, d, n);  // powm — возведение в степень по модулю (функция Boost)
            if (x == 1 || x == n - 1)
                continue;
            bool cont = false;
            for (int r = 1; r < s; r++) {
                x = powm(x, 2, n);
                if (x == n - 1) {
                    cont = true;
                    break;
                }
            }
            if (!cont)
                return false;
        }
        return true;
    }

    // Генерация простого числа заданной битовой длины
    cpp_int generatePrime(int bit_length) {
        cpp_int lower_bound = 1;
        lower_bound <<= (bit_length - 1);  // 2^(bit_length-1)
        cpp_int upper_bound = 1;
        upper_bound <<= bit_length;        // 2^(bit_length)
        while (true) {
            cpp_int candidate = randomBetween(lower_bound, upper_bound - 1);
            candidate |= 1;  // гарантируем, что число нечётное
            if (isProbablePrime(candidate))
                return candidate;
        }
    }

    // Генерация публичной экспоненты (e) случайным образом.
    // В данном примере выбирается случайное 17-битное число (в диапазоне [3, 2^17 - 1]),
    // удовлетворяющее условию: e < φ и gcd(e, φ) = 1.
    cpp_int generatePublicExponent(const cpp_int& phi) {
        cpp_int publicKey;
        do {
            publicKey = randomBetween(2, phi - 1); // Случайное число в диапазоне [2, phi - 1]
            // Для отладки можно раскомментировать следующую строку:
            // std::cout << "Trying e: " << publicKey << std::endl;
        } while (gcd(publicKey, phi) != 1);
        return publicKey;
    }

    // Преобразование строки (байтового блока) в число
    cpp_int stringToNumber(const std::string& block) {
        cpp_int result = 0;
        for (unsigned char c : block) {
            result = (result << 8) + c;  // сдвиг на 8 бит и прибавление байта
        }
        return result;
    }

    // Преобразование числа в строку фиксированной длины (дополняем слева нулями)
    std::string numberToString(cpp_int num, size_t block_length) {
        std::string block(block_length, '\0');
        for (size_t i = 0; i < block_length; i++) {
            block[block_length - i - 1] = static_cast<char>(static_cast<unsigned int>(num & 0xFF));
            num >>= 8;
        }
        return block;
    }

public:
    // Конструктор: генерирует ключи RSA с указанной длиной ключа (в битах: 512/1024/2048)
    RSA(int key_length) {
        if (key_length < 16)
            throw std::invalid_argument("Key length too short");

        keyLength = key_length;
    }

    void GenerateKey()
    {
        // Генерируем два простых числа (p и q) примерно одинаковой битовой длины
        int prime_bits = keyLength / 2;
        cpp_int p = generatePrime(prime_bits);
        cpp_int q = generatePrime(prime_bits);
        while (q == p) {  // если случайно совпали, генерируем новое
            q = generatePrime(prime_bits);
        }
        modulus = p * q;
        cpp_int phi = (p - 1) * (q - 1);

        // Генерируем публичную экспоненту (e) случайным образом
        public_key = generatePublicExponent(phi);

        // Вычисляем приватную экспоненту (d) как обратную к e по модулю φ
        private_key = modInverse(public_key, phi);
    }


    // Метод шифрования: принимает строку (plaintext) и возвращает строку зашифрованного текста.
    // Зашифрованные блоки представлены в 16-ричном виде, разделённые пробелами.
    // В зашифрованный текст также включается длина исходного сообщения (в байтах) для восстановления.
    std::string encrypt(const std::string& plaintext) {
        // Определяем максимальный размер блока в байтах:
        // block_size = floor((bitLength(n)-1)/8)
        int mod_bits = bitLength(modulus);
        size_t block_size = (mod_bits - 1) / 8;
        if (block_size == 0)
            block_size = 1;

        std::vector<std::string> encrypted_blocks;
        // Первый токен – длина исходного сообщения (в байтах)
        encrypted_blocks.push_back(std::to_string(plaintext.size()));

        // Разбиваем сообщение на блоки по block_size байт
        for (size_t i = 0; i < plaintext.size(); i += block_size) {
            std::string block = plaintext.substr(i, block_size);
            cpp_int m = stringToNumber(block);
            cpp_int c = powm(m, public_key, modulus);
            // Преобразуем число c в 16-ричную строку
            std::stringstream ss;
            ss << std::hex << c;
            encrypted_blocks.push_back(ss.str());
        }
        // Объединяем блоки через пробел
        std::stringstream result;
        for (size_t i = 0; i < encrypted_blocks.size(); i++) {
            result << encrypted_blocks[i];
            if (i != encrypted_blocks.size() - 1)
                result << " ";
        }
        return result.str();
    }

    std::string encrypt(const std::string& plaintext, cpp_int pubK, cpp_int modK) {
        // Определяем максимальный размер блока в байтах:
        // block_size = floor((bitLength(n)-1)/8)
        int mod_bits = bitLength(modulus);
        size_t block_size = (mod_bits - 1) / 8;
        if (block_size == 0)
            block_size = 1;

        std::vector<std::string> encrypted_blocks;
        // Первый токен – длина исходного сообщения (в байтах)
        encrypted_blocks.push_back(std::to_string(plaintext.size()));

        // Разбиваем сообщение на блоки по block_size байт
        for (size_t i = 0; i < plaintext.size(); i += block_size) {
            std::string block = plaintext.substr(i, block_size);
            cpp_int m = stringToNumber(block);
            cpp_int c = powm(m, pubK, modK);
            // Преобразуем число c в 16-ричную строку
            std::stringstream ss;
            ss << std::hex << c;
            encrypted_blocks.push_back(ss.str());
        }
        // Объединяем блоки через пробел
        std::stringstream result;
        for (size_t i = 0; i < encrypted_blocks.size(); i++) {
            result << encrypted_blocks[i];
            if (i != encrypted_blocks.size() - 1)
                result << " ";
        }
        return result.str();
    }

    // Метод дешифрования: принимает строку зашифрованного текста (токены – 16-ричные числа, разделённые пробелами)
    // и возвращает исходное сообщение.
    std::string decrypt(const std::string& ciphertext) {
        std::istringstream iss(ciphertext);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.empty())
            throw std::invalid_argument("Empty ciphertext");

        // Первый токен – исходная длина сообщения (в байтах)
        size_t original_length = std::stoull(tokens[0]);

        // Определяем размер блока, использованный при шифровании
        int mod_bits = bitLength(modulus);
        size_t block_size = (mod_bits - 1) / 8;
        if (block_size == 0)
            block_size = 1;

        std::string plaintext;
        size_t num_blocks = tokens.size() - 1;  // без первого токена

        for (size_t i = 0; i < num_blocks; i++) {
            // Преобразуем 16-ричное представление зашифрованного блока в число
            std::stringstream ss;
            ss << std::hex << tokens[i + 1];
            cpp_int c;
            ss >> c;
            // Дешифруем: m = c^d mod n
            cpp_int m = powm(c, private_key, modulus);
            std::string block = numberToString(m, block_size);
            // Для последнего блока учитываем, что реальная длина может быть меньше block_size
            if (i == num_blocks - 1) {
                size_t remaining = original_length - block_size * (num_blocks - 1);
                plaintext += block.substr(block.size() - remaining, remaining);
            }
            else {
                plaintext += block;
            }
        }
        return plaintext;
    }

    cpp_int getPublicKey() const { return public_key; }
    cpp_int getPrivateKey() const { return private_key; }
    cpp_int getModulus() const { return modulus; }

    void setKeys(cpp_int publicKey, cpp_int modulusK, cpp_int privateKey)
    {
        public_key = publicKey;
        modulus = modulusK;
        private_key = privateKey;
    }

    string getFullKeys() {
        std::ostringstream oss;
        oss << "<publicKey>" << getPublicKey() << "</publicKey><modulus>" << getModulus() << "</modulus><privateKey>" << getPrivateKey() << "</privateKey>";
        return oss.str();
    }

    string getPubKeys()
    {
        std::ostringstream oss;
        oss << "<publicKey>" << getPublicKey() << "</publicKey><modulus>" << getModulus() << "</modulus>";
        return oss.str();
    }
};