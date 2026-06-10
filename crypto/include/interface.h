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

    virtual bool setup(std::string& key) { return true; }
    
    // Параметры
    virtual int keySize() const = 0;
    virtual int nonceSize() const { return 0; }
};

#endif