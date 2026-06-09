#include "../include/interface.h"
#include "../include/loader.h"
#include <cctype>

using namespace std;

class CaesarCipher : public Cipher {
private:
    uint8_t shiftChar(uint8_t c, int s) const {
        // Только для латинских букв
        if (c >= 'a' && c <= 'z') {
            return 'a' + ((c - 'a' + s) % 26);
        }
        if (c >= 'A' && c <= 'Z') {
            return 'A' + ((c - 'A' + s) % 26);
        }
        return c;  // не буквы не меняем
    }
    
public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        int s = 3;  // сдвиг по умолчанию
        if (!key.empty()) {
            try {
                s = stoi(key) % 26;
            } catch (...) {
                s = 3;
            }
        }
        
        EncData result;
        result.data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            result.data[i] = shiftChar(data[i], s);
        }
        result.meta = to_string(s);
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        int s = 3;
        if (!key.empty()) {
            try {
                s = stoi(key) % 26;
            } catch (...) {
                s = 3;
            }
        }
        
        vector<uint8_t> result(data.data.size());
        for (size_t i = 0; i < data.data.size(); i++) {
            result[i] = shiftChar(data.data[i], -s);
        }
        return result;
    }
    
    string name() const override { return "caesar"; }
    string description() const override { return "Caesar shift cipher (0-25)"; }
    Type getType() const override { return Type::SYMMETRIC; }
    int keySize() const override { return 8; }  // число от 0 до 25
    int nonceSize() const override { return 0; }
};

REG_CIPHER(CaesarCipher, "caesar")
