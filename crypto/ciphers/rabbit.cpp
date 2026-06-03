#include "../include/interface.h"
#include "../include/loader.h"
#include <random>
#include <vector>
#include <cstring>

using namespace std;

class Rabbit : public Cipher {
private:
    uint32_t X[8];
    uint32_t C[8];
    uint32_t carry;
    
    void nextState() {
        uint64_t temp;
        
        // Обновление счетчиков
        for (int i = 0; i < 8; i++) {
            temp = (uint64_t)C[i] + (uint64_t)X[i] + (uint64_t)(i == 7 ? carry : 0);
            C[i] = (uint32_t)temp;
            carry = (uint32_t)(temp >> 32);
        }
        
        // Обновление переменных состояния
        for (int i = 0; i < 8; i++) {
            uint32_t g = X[i] + C[i];
            uint32_t g_rot = (g << 16) | (g >> 16);
            X[i] = g_rot + ((X[(i+7)%8] + C[(i+7)%8]) ^ (X[(i+6)%8] + C[(i+6)%8]));
        }
    }
    
    void generateKeystream(uint8_t* output, size_t bytes) {
        size_t generated = 0;
        uint32_t S[8];
        
        while (generated < bytes) {
            nextState();
            
            // Извлечение псевдослучайных байтов
            for (int i = 0; i < 8; i++) {
                S[i] = X[i] ^ (X[(i+4)%8] + C[(i+4)%8]);
            }
            
            for (int i = 0; i < 8 && generated < bytes; i++) {
                for (int j = 0; j < 4 && generated < bytes; j++) {
                    output[generated++] = (uint8_t)(S[i] >> (8 * j));
                }
            }
        }
    }
    
    void keySetup(const string& key) {
        uint32_t K[8] = {0};
        
        // Копируем ключ (16 байт = 128 бит)
        for (size_t i = 0; i < 16 && i < key.size(); i++) {
            K[i/4] |= (uint32_t)(unsigned char)key[i] << (8 * (i % 4));
        }
        
        // Инициализация X и C
        for (int i = 0; i < 8; i++) {
            if (i % 2 == 0) {
                X[i] = K[(i+1) % 8] | (K[i] << 16);
                C[i] = K[(i+4) % 8] | (K[(i+5) % 8] << 16);
            } else {
                X[i] = K[(i+5) % 8] | (K[(i+4) % 8] << 16);
                C[i] = K[i] | (K[(i+1) % 8] << 16);
            }
        }
        
        carry = 0;
        
        // 4 раунда инициализации
        for (int i = 0; i < 4; i++) {
            nextState();
        }
        
        // Корректировка счетчиков
        for (int i = 0; i < 8; i++) {
            C[i] ^= X[(i+4) % 8];
        }
    }
    
    void ivSetup(const string& iv) {
        if (iv.size() >= 8) {
            uint64_t IV = 0;
            for (size_t i = 0; i < 8; i++) {
                IV |= (uint64_t)(unsigned char)iv[i] << (8 * i);
            }
            
            // Применяем IV к счетчикам
            C[0] ^= (uint32_t)(IV >> 32);
            C[1] ^= (uint32_t)(IV & 0xFFFFFFFF);
            C[2] ^= (uint32_t)(IV >> 32);
            C[3] ^= (uint32_t)(IV & 0xFFFFFFFF);
            C[4] ^= (uint32_t)(IV >> 32);
            C[5] ^= (uint32_t)(IV & 0xFFFFFFFF);
            C[6] ^= (uint32_t)(IV >> 32);
            C[7] ^= (uint32_t)(IV & 0xFFFFFFFF);
            
            // 4 раунда после добавления IV
            for (int i = 0; i < 4; i++) {
                nextState();
            }
        }
    }
    
public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;
        
        // Генерируем случайный IV
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 255);
        
        string iv(8, 0);
        for (int i = 0; i < 8; i++) {
            iv[i] = (char)dis(gen);
        }
        
        // Инициализация шифра
        keySetup(key);
        ivSetup(iv);
        
        // Генерируем keystream
        vector<uint8_t> keystream(data.size());
        generateKeystream(keystream.data(), data.size());
        
        // XOR шифрование
        result.data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            result.data[i] = data[i] ^ keystream[i];
        }
        
        result.meta = iv;
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        // Инициализация шифра с тем же ключом
        keySetup(key);
        
        // Используем IV из метаданных
        if (data.meta.size() >= 8) {
            ivSetup(data.meta.substr(0, 8));
        }
        
        // Генерируем тот же keystream
        vector<uint8_t> keystream(data.data.size());
        generateKeystream(keystream.data(), data.data.size());
        
        // XOR дешифрование (то же самое, что шифрование)
        vector<uint8_t> result(data.data.size());
        for (size_t i = 0; i < data.data.size(); i++) {
            result[i] = data.data[i] ^ keystream[i];
        }
        
        return result;
    }
    
    string name() const override { return "rabbit"; }
    int keySize() const override { return 16; }
    int nonceSize() const override { return 8; }
};

REG_CIPHER(Rabbit, "rabbit")
