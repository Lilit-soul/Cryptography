#include "../include/interface.h"
#include "../include/loader.h"
#include <vector>
#include <cstring>

using namespace std;

static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

class DiffieHellmanCipher : public Cipher {
private:
    uint64_t p = 23;
    uint64_t g = 5;
    uint64_t bob_priv = 15;
    
    uint64_t passwordToPrivKey(const string& password) {
        if (password.empty()) return 6;
        uint64_t hash = 5381;
        for (char c : password) {
            hash = ((hash << 5) + hash) + (unsigned char)c;
        }
        return 2 + (hash % 21);
    }
    
public:
    string name() const override { return "diffie_hellman"; }
    int keySize() const override { return 32; }
    int nonceSize() const override { return 8; }
    Type getType() const override { return Type::ASYMMETRIC; }
    
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;
        
        if (data.empty() || key.empty()) return result;
        
        uint64_t alice_priv = passwordToPrivKey(key);
        uint64_t alice_pub = mod_pow(g, alice_priv, p);
        uint64_t bob_pub = mod_pow(g, bob_priv, p);
        uint64_t shared_secret = mod_pow(bob_pub, alice_priv, p);
        
        result.meta.resize(8);
        for (int i = 0; i < 8; i++) {
            result.meta[i] = (alice_pub >> (i * 8)) & 0xFF;
        }
        
        uint8_t xor_key = (uint8_t)(shared_secret & 0xFF);
        result.data = data;
        for (size_t i = 0; i < result.data.size(); i++) {
            result.data[i] ^= xor_key;
        }
        
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        vector<uint8_t> result;
        
        if (data.data.empty() || key.empty() || data.meta.size() < 8) {
            return result;
        }
        
        uint64_t alice_pub = 0;
        for (int i = 0; i < 8; i++) {
            alice_pub |= (uint64_t)(unsigned char)data.meta[i] << (i * 8);
        }
        
        uint64_t shared_secret = mod_pow(alice_pub, bob_priv, p);
        
        uint8_t xor_key = (uint8_t)(shared_secret & 0xFF);
        result = data.data;
        for (size_t i = 0; i < result.size(); i++) {
            result[i] ^= xor_key;
        }
        
        return result;
    }
};

REG_CIPHER(DiffieHellmanCipher, "diffie_hellman")
