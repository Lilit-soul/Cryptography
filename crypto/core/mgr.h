#ifndef MGR_H
#define MGR_H

#include "../include/interface.h"
#include "errors.h"
#include <memory>
#include <string>
#include <vector>

using namespace std;

class Mgr {
private:
    unique_ptr<Cipher> cur;
    
public:
    bool select(const string& name);
    vector<string> list() const;
    string info() const;
    
    // Функции с возвратом кода ошибки
    ErrorCode encText(const string& text, const string& key, string& output);
    ErrorCode decText(const string& data, const string& key, string& output);
    
    ErrorCode encFile(const string& in, const string& out, const string& key);
    ErrorCode decFile(const string& in, const string& out, const string& key);
    
    // Безопасные обёртки (не выбрасывают исключения)
    bool encTextSafe(const string& text, const string& key, string& output);
    bool decTextSafe(const string& data, const string& key, string& output);
    
    bool encFileSafe(const string& in, const string& out, const string& key);
    bool decFileSafe(const string& in, const string& out, const string& key);
};

#endif