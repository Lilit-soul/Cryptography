#include "../include/interface.h"
#include "../include/loader.h"
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

class ShamirCipher : public Cipher {
private:
    // Разбор ключа: ожидается формат "aliceKey,bobKey" или "aliceKey|bobKey"
    void parseKey(const string& key, uint8_t& aliceKey, uint8_t& bobKey) {
        size_t pos = key.find_first_of(",|");
        if (pos != string::npos) {
            aliceKey = static_cast<uint8_t>(stoi(key.substr(0, pos)));
            bobKey = static_cast<uint8_t>(stoi(key.substr(pos + 1)));
        } else {
            // Если разделителя нет, оба ключа одинаковые
            aliceKey = static_cast<uint8_t>(stoi(key));
            bobKey = aliceKey;
        }
    }

public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;
        
        if (data.empty()) {
            return result;
        }
        
        uint8_t aliceKey = 0;
        uint8_t bobKey = 0;
        parseKey(key, aliceKey, bobKey);
        
        // 3-pass протокол
        vector<uint8_t> encrypted = data;
        
        // Алиса шифрует
        for (size_t i = 0; i < encrypted.size(); i++) {
            encrypted[i] ^= aliceKey;
        }
        
        // Боб шифрует
        for (size_t i = 0; i < encrypted.size(); i++) {
            encrypted[i] ^= bobKey;
        }
        
        // Алиса расшифровывает
        for (size_t i = 0; i < encrypted.size(); i++) {
            encrypted[i] ^= aliceKey;
        }
        
        result.data = encrypted;
        result.meta = to_string(bobKey);  // сохраняем ключ Боба
        
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        vector<uint8_t> result = data.data;
        
        if (result.empty()) {
            return result;
        }
        
        // Для расшифровки нужен только ключ Боба
        uint8_t bobKey = 0;
        if (!data.meta.empty()) {
            bobKey = static_cast<uint8_t>(stoi(data.meta));
        } else if (!key.empty()) {
            // Если метаданных нет, берём из ключа
            size_t pos = key.find_first_of(",|");
            if (pos != string::npos) {
                bobKey = static_cast<uint8_t>(stoi(key.substr(pos + 1)));
            } else {
                bobKey = static_cast<uint8_t>(stoi(key));
            }
        }
        
        // Расшифровываем ключом Боба
        for (size_t i = 0; i < result.size(); i++) {
            result[i] ^= bobKey;
        }
        
        return result;
    }
    
    string name() const override { return "Shamir 3-pass (XOR, 2 keys)"; }
    int keySize() const override { return 2; }  // два ключа
    int nonceSize() const override { return 0; }
    Type getType() const override { return Type::SYMMETRIC; }
};

REG_CIPHER(ShamirCipher, "shamir")