#include "../include/interface.h"
#include "../include/loader.h"
#include <random>
#include <vector>
#include <cstring>

#define ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

#define QUARTER_ROUND(a, b, c, d) \
    a += b; d ^= a; d = ROTL(d, 16); \
    c += d; b ^= c; b = ROTL(b, 12); \
    a += b; d ^= a; d = ROTL(d, 8);  \
    c += d; b ^= c; b = ROTL(b, 7);

using namespace std;

class ChaCha20 : public Cipher {
private:
    uint32_t state[16];
    
    void chacha20Block(uint32_t output[16]) {
        uint32_t working[16];
        memcpy(working, state, sizeof(working));
        
        for (int i = 0; i < 10; i++) {
            QUARTER_ROUND(working[0], working[4], working[8], working[12]);
            QUARTER_ROUND(working[1], working[5], working[9], working[13]);
            QUARTER_ROUND(working[2], working[6], working[10], working[14]);
            QUARTER_ROUND(working[3], working[7], working[11], working[15]);
            QUARTER_ROUND(working[0], working[5], working[10], working[15]);
            QUARTER_ROUND(working[1], working[6], working[11], working[12]);
            QUARTER_ROUND(working[2], working[7], working[8], working[13]);
            QUARTER_ROUND(working[3], working[4], working[9], working[14]);
        }
        
        for (int i = 0; i < 16; i++) {
            output[i] = working[i] + state[i];
        }
    }
    
    void keySetup(const string& key) {
        // Константа "expand 32-byte k"
        state[0] = 0x61707865; // "expa"
        state[1] = 0x3320646e; // "nd 3"
        state[2] = 0x79622d32; // "2-by"
        state[3] = 0x6b206574; // "te k"
        
        // Ключ (32 байта)
        for (int i = 0; i < 8; i++) {
            state[4 + i] = 0;
            for (int j = 0; j < 4; j++) {
                size_t idx = i * 4 + j;
                if (idx < key.size()) {
                    state[4 + i] |= (uint32_t)(unsigned char)key[idx] << (8 * j);
                }
            }
        }
    }
    
    void ivSetup(const string& iv) {
        // Counter
        state[12] = 0;
        state[13] = 0;
        
        // Nonce (8 байт)
        state[14] = 0;
        state[15] = 0;
        
        for (size_t i = 0; i < iv.size() && i < 8; i++) {
            if (i < 4) {
                state[14] |= (uint32_t)(unsigned char)iv[i] << (8 * i);
            } else {
                state[15] |= (uint32_t)(unsigned char)iv[i] << (8 * (i - 4));
            }
        }
    }
    
    void generateKeystream(vector<uint8_t>& output, size_t length) {
        output.resize(length);
        uint32_t block[16];
        size_t offset = 0;
        uint64_t counter = 0;
        
        while (offset < length) {
            state[12] = (uint32_t)(counter & 0xFFFFFFFF);
            state[13] = (uint32_t)(counter >> 32);
            
            chacha20Block(block);
            
            for (int i = 0; i < 16 && offset < length; i++) {
                for (int j = 0; j < 4 && offset < length; j++) {
                    output[offset++] = (uint8_t)(block[i] >> (8 * j));
                }
            }
            
            counter++;
        }
    }
    
public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;
        
        // Генерируем случайный IV (8 байт)
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 255);
        
        string iv(8, 0);
        for (int i = 0; i < 8; i++) {
            iv[i] = (char)dis(gen);
        }
        
        // Инициализация
        keySetup(key);
        ivSetup(iv);
        
        // Генерация keystream
        vector<uint8_t> keystream;
        generateKeystream(keystream, data.size());
        
        // XOR шифрование
        result.data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            result.data[i] = data[i] ^ keystream[i];
        }
        
        result.meta = iv;
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        // Инициализация с тем же ключом
        keySetup(key);
        
        // Используем IV из метаданных
        string iv = data.meta;
        if (iv.size() < 8) {
            iv.resize(8, 0);
        }
        ivSetup(iv);
        
        // Генерация keystream
        vector<uint8_t> keystream;
        generateKeystream(keystream, data.data.size());
        
        // XOR дешифрование
        vector<uint8_t> result(data.data.size());
        for (size_t i = 0; i < data.data.size(); i++) {
            result[i] = data.data[i] ^ keystream[i];
        }
        
        return result;
    }
    
    string name() const override { return "chacha20"; }
    int keySize() const override { return 32; }
    int nonceSize() const override { return 8; }
    Type getType() const override { return Type::SYMMETRIC; } 
};

REG_CIPHER(ChaCha20, "chacha20")
