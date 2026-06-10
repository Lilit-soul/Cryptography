#ifndef MGR_H
#define MGR_H

#include <memory>
#include <string>
#include <vector>

#include "../include/interface.h"
#include "errors.h"

class Mgr {
private:
    std::unique_ptr<Cipher> cur;
    
public:
    bool select(const std::string& name);
    std::vector<std::string> list() const;
    std::string info() const;
    
    // Получить тип текущего шифра
    Cipher::Type getCipherType() const;
    Cipher* getCipher() { return cur.get(); }
    
    
    
    // Основные операции
    ErrorCode encText(const std::string& text, const std::string& key, std::string& output);
    ErrorCode decText(const std::string& data, const std::string& key, std::string& output);
    
    ErrorCode encFile(const std::string& in, const std::string& out, const std::string& key);
    ErrorCode decFile(const std::string& in, const std::string& out, const std::string& key);
    
    // Безопасные обёртки
    bool encTextSafe(const std::string& text, const std::string& key, std::string& output);
    bool decTextSafe(const std::string& data, const std::string& key, std::string& output);
    
    bool encFileSafe(const std::string& in, const std::string& out, const std::string& key);
    bool decFileSafe(const std::string& in, const std::string& out, const std::string& key);
};

#endif