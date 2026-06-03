#ifndef MGR_H
#define MGR_H

#include "../include/interface.h"
#include <memory>
#include <string>
#include <vector>

class Mgr {
private:
    std::unique_ptr<Cipher> cur;
    
public:
    bool select(const std::string& name);
    std::vector<std::string> list() const;
    std::string info() const;
    
    std::string encText(const std::string& text, const std::string& key);
    std::string decText(const std::string& data, const std::string& key);
    
    std::vector<uint8_t> encData(const std::vector<uint8_t>& data, const std::string& key);
    std::vector<uint8_t> decData(const std::vector<uint8_t>& data, const std::string& key);
    
    bool encFile(const std::string& in, const std::string& out, const std::string& key);
    bool decFile(const std::string& in, const std::string& out, const std::string& key);
};

#endif
