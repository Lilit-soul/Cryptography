#include "../include/interface.h"
#include "../include/loader.h"
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>

using namespace std;

class ShamirCipher : public Cipher {
private:
    uint32_t gcd(uint32_t a, uint32_t b) {
        while (b != 0) {
            uint32_t t = a % b;
            a = b;
            b = t;
        }
        return a;
    }

    bool isPrime(uint32_t n) {
        if (n < 2) {
            return false;
        }
        if (n == 2) {
            return true;
        }
        if (n % 2 == 0) {
            return false;
        }

        for (uint32_t i = 3; i * i <= n; i += 2) {
            if (n % i == 0) {
                return false;
            }
        }

        return true;
    }

    uint32_t modPow(uint32_t base, uint32_t exp, uint32_t mod) {
        uint64_t result = 1;
        uint64_t value = base % mod;

        while (exp > 0) {
            if (exp & 1) {
                result = (result * value) % mod;
            }
            value = (value * value) % mod;
            exp >>= 1;
        }

        return result;
    }

    uint32_t modInverse(uint32_t a, uint32_t mod) {
        int32_t t = 0;
        int32_t newT = 1;
        int32_t r = mod;
        int32_t newR = a;

        while (newR != 0) {
            int32_t q = r / newR;

            int32_t tempT = t - q * newT;
            t = newT;
            newT = tempT;

            int32_t tempR = r - q * newR;
            r = newR;
            newR = tempR;
        }

        if (r != 1) {
            throw invalid_argument("ключ не имеет обратного значения по модулю p-1");
        }

        if (t < 0) {
            t += mod;
        }

        return t;
    }

    vector<string> splitKey(const string& key) {
        vector<string> parts;
        string current;
        for (char c : key) {
            if (c == ',' || c == '|') {
                parts.push_back(current);
                current.clear();
            } else if (c != ' ') {
                current += c;
            }
        }
        parts.push_back(current);
        return parts;
    }

    uint32_t parseNumber(const string& part, const string& name) {
        if (part.empty()) {
            throw invalid_argument("параметр " + name + " не задан");
        }
        try {
            size_t used = 0;
            uint32_t value = stoul(part, &used);
            if (used == part.size()) {
                return value;
            }
        } catch (const exception&) {
            throw invalid_argument("параметр " + name + " должен быть числом");
        }

        throw invalid_argument("параметр " + name + " должен быть числом");
    }

    // Формат ключа: "простое число,ключ Алисы,ключ Боба", например "257,7,5".
    void parseKey(const string& key, uint32_t& p, uint32_t& aliceC, uint32_t& bobC) {
        vector<string> parts = splitKey(key);
        if (parts.size() != 3) {
            throw invalid_argument("ключ Шамира должен иметь формат: простое число, ключ Алисы, ключ Боба");
        }
        p = parseNumber(parts[0], "p");
        if (p <= 255) {
            throw invalid_argument("параметр p должен быть больше 255");
        }
        if (p > 65535) {
            throw invalid_argument("параметр p должен быть не больше 65535");
        }
        if (!isPrime(p)) {
            throw invalid_argument("параметр p должен быть простым числом");
        }

        uint32_t phi = p - 1;
        aliceC = parseNumber(parts[1], "Ca");
        bobC = parseNumber(parts[2], "Cb");
        if (aliceC <= 1 || aliceC >= phi) {
            throw invalid_argument("ключ Ca должен быть больше 1 и меньше p-1");
        }
        if (bobC <= 1 || bobC >= phi) {
            throw invalid_argument("ключ Cb должен быть больше 1 и меньше p-1");
        }
        if (gcd(aliceC, phi) != 1 || gcd(bobC, phi) != 1) {
            throw invalid_argument("ключи Ca и Cb должны быть взаимно простыми с p-1");
        }
    }

public:
    bool setup(string& key) override {
        while (true) {
            cout << "\nДля шифра Шамира введите три числа через запятую:\n";
            cout << "1) простое число больше 255 и не больше 65535\n";
            cout << "2) секретный ключ Алисы, взаимно простой с первым числом минус 1\n";
            cout << "3) секретный ключ Боба, взаимно простой с первым числом минус 1\n";
            cout << "Пример: 257,7,5\n\n";
            cout << "Введите параметры Шамира: ";
            string input;
            getline(cin, input);
            if (input.empty() && !key.empty()) {
                input = key;
            }
            try {
                uint32_t p = 0;
                uint32_t aliceC = 0;
                uint32_t bobC = 0;
                parseKey(input, p, aliceC, bobC);
                key = input;
                cout << "Параметры Шамира приняты: p=" << p
                     << ", Ca=" << aliceC
                     << ", Cb=" << bobC << "\n";
                return true;
            } catch (const exception& e) {
                cout << "Ошибка ввода параметров Шамира: " << e.what() << "\n";
            }
        }
    }

    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;
        if (data.empty()) {
            return result;
        }
        uint32_t p = 0;
        uint32_t aliceC = 0;
        uint32_t bobC = 0;
        parseKey(key, p, aliceC, bobC);
        uint32_t phi = p - 1;
        uint32_t aliceD = modInverse(aliceC, phi);
        vector<uint8_t> encrypted;
        encrypted.reserve(data.size() * 2);
        for (uint8_t byte : data) {
            uint32_t x1 = modPow(byte, aliceC, p);
            uint32_t x2 = modPow(x1, bobC, p);
            uint32_t x3 = modPow(x2, aliceD, p);
            encrypted.push_back(x3 & 0xFF);
            encrypted.push_back((x3 >> 8) & 0xFF);
        }
        result.data = encrypted;
        result.meta = "";
        return result;
    }
   
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        vector<uint8_t> result;
        if (data.data.empty()) {
            return result;
        }
        if (data.data.size() % 2 != 0) {
            throw invalid_argument("некорректный размер зашифрованных данных Шамира");
        }
        uint32_t p = 0;
        uint32_t aliceC = 0;
        uint32_t bobC = 0;
        parseKey(key, p, aliceC, bobC);
        uint32_t phi = p - 1;
        uint32_t bobD = modInverse(bobC, phi);
        result.reserve(data.data.size() / 2);
        for (size_t i = 0; i < data.data.size(); i += 2) {
            uint32_t value = data.data[i] | (data.data[i + 1] << 8);
            if (value >= p) {
                throw invalid_argument("зашифрованное значение не может быть больше или равно p");
            }
            uint32_t plain = modPow(value, bobD, p);
            if (plain > 255) {
                throw invalid_argument("некорректный байт исходных данных Шамира");
            }
            result.push_back(plain);
        }
        return result;
    }
   
    string name() const override { return "Shamir 3-pass"; }
    int keySize() const override { return 2; }
    int nonceSize() const override { return 0; }
    Type getType() const override { return Type::SYMMETRIC; }
};

REG_CIPHER(ShamirCipher, "shamir")
