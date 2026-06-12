#include "../include/interface.h"
#include "../include/loader.h"
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

class CaesarCipher : public Cipher {
private:
    int shift;
    
    void setKey(const string& key) {
        if (key.empty()) {
            shift = 3;
        } else {
            shift = (unsigned char)key[0] % 256;
        }
    }

public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        setKey(key);
        EncData result;
        result.data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            result.data[i] = (data[i] + shift) % 256;
        }
        result.meta = "";
        return result;
    }

    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        setKey(key);
        vector<uint8_t> result(data.data.size());
        for (size_t i = 0; i < data.data.size(); i++) {
            result[i] = (data.data[i] - shift + 256) % 256;
        }
        return result;
    }

    string name() const override { return "caesar"; }
    int keySize() const override { return 1; }
    int nonceSize() const override { return 0; }
    Type getType() const override { return Type::SYMMETRIC; }
};

REG_CIPHER(CaesarCipher, "caesar")