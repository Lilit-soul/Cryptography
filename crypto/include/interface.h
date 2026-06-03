#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <cstdint>

struct EncData {
    std::vector<uint8_t> data;
    std::string meta;
};

class Cipher {
public:
    virtual ~Cipher() = default;
    virtual EncData encrypt(const std::vector<uint8_t>& data, const std::string& key) = 0;
    virtual std::vector<uint8_t> decrypt(const EncData& data, const std::string& key) = 0;
    
    virtual std::string name() const = 0;
    virtual int keySize() const = 0;
    virtual int nonceSize() const = 0;
};

#endif
