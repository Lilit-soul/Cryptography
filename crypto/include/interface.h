#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <cstdint>
#include <utility>  // для std::pair

struct EncData {
    std::vector<uint8_t> data;
    std::string meta;
};

class Cipher {
public:
    virtual ~Cipher() = default;
    
    // Основные операции
    virtual EncData encrypt(const std::vector<uint8_t>& data, const std::string& key) = 0;
    virtual std::vector<uint8_t> decrypt(const EncData& data, const std::string& key) = 0;
    
    // Информация о шифре
    virtual std::string name() const = 0;
    virtual std::string description() const { return name(); }
    
    // Тип шифра
    enum class Type {
        SYMMETRIC,      // один ключ
        ASYMMETRIC,     // public/private
        KEY_EXCHANGE,   // Diffie-Hellman
        HASH           // хеш-функция
    };
    
    virtual Type getType() const { return Type::SYMMETRIC; }
    
    // Для асимметричных шифров
    virtual std::pair<std::string, std::string> generateKeyPair() {
        return {"", ""};
    }
    virtual std::string getPublicKey(const std::string& privateKey) { return ""; }
    
    // Для Diffie-Hellman
    virtual std::string generatePrivateKey() { return ""; }
    virtual std::string computePublicKey(const std::string& privateKey) { return ""; }
    virtual std::string computeSharedSecret(const std::string& privateKey, 
                                            const std::string& otherPublic) { return ""; }
    
    // Параметры
    virtual int keySize() const = 0;
    virtual int nonceSize() const { return 0; }
};

#endif