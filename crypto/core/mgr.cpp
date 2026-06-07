#include "mgr.h"
#include "../include/loader.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cstring>

using namespace std;

string toLower(const string& s) {
    string result = s;
    for (char& c : result) {
        c = tolower(c);
    }
    return result;
}

string normalizeName(const string& name) {
    string result;
    for (char c : name) {
        if (!isspace(c)) {
            result += tolower(c);
        }
    }
    return result;
}

bool Mgr::select(const string& name) {
    auto list = Loader::instance().list();
    string normalizedInput = normalizeName(name);
    
    for (const auto& cipherName : list) {
        if (normalizeName(cipherName) == normalizedInput) {
            auto c = Loader::instance().create(cipherName);
            if (c) {
                cur = move(c);
                return true;
            }
        }
    }
    return false;
}

vector<string> Mgr::list() const {
    return Loader::instance().list();
}

string Mgr::info() const {
    if (!cur) return "Шифр не выбран";
    return cur->name() + " | ключ: " + to_string(cur->keySize()) + " байт | nonce: " + to_string(cur->nonceSize()) + " байт";
}

string Mgr::encText(const string& text, const string& key) {
    if (!cur) {
        cout << "Ошибка: шифр не выбран\n";
        return "";
    }
    
    if (text.empty()) {
        cout << "Ошибка: текст пуст\n";
        return "";
    }
    
    // Проверка key не нужна - она уже есть в main
    vector<uint8_t> in(text.begin(), text.end());
    auto enc = cur->encrypt(in, key);
    
    string result;
    uint32_t ivSize = enc.meta.size();
    result.append(reinterpret_cast<const char*>(&ivSize), sizeof(ivSize));
    result.append(enc.meta);
    result.append(enc.data.begin(), enc.data.end());
    
    return result;
}

string Mgr::decText(const string& data, const string& key) {
    if (!cur) {
        cout << "Ошибка: шифр не выбран\n";
        return "";
    }
    
    if (data.size() < sizeof(uint32_t)) {
        cout << "Ошибка: зашифрованные данные слишком короткие\n";
        return "";
    }
    
    uint32_t ivSize;
    memcpy(&ivSize, data.data(), sizeof(ivSize));
    
    if (ivSize > 1024) {
        cout << "Ошибка: неверный размер метаданных\n";
        return "";
    }
    
    if (data.size() < sizeof(ivSize) + ivSize) {
        cout << "Ошибка: данные повреждены\n";
        return "";
    }
    
    string meta = data.substr(sizeof(ivSize), ivSize);
    vector<uint8_t> encData(data.begin() + sizeof(ivSize) + ivSize, data.end());
    
    EncData enc;
    enc.data = encData;
    enc.meta = meta;
    
    auto dec = cur->decrypt(enc, key);
    return string(dec.begin(), dec.end());
}

bool Mgr::encFile(const string& in, const string& out, const string& key) {
    if (!cur) {
        cout << "Ошибка: шифр не выбран\n";
        return false;
    }
    
    ifstream f_in(in, ios::binary);
    if (!f_in) {
        cout << "Ошибка: файл '" << in << "' не найден\n";
        return false;
    }
    
    f_in.seekg(0, ios::end);
    if (f_in.tellg() == 0) {
        cout << "Ошибка: файл пуст\n";
        f_in.close();
        return false;
    }
    f_in.seekg(0, ios::beg);
    
    vector<uint8_t> data((istreambuf_iterator<char>(f_in)), istreambuf_iterator<char>());
    f_in.close();
    
    auto enc = cur->encrypt(data, key);
    
    ofstream f_out(out, ios::binary);
    if (!f_out) {
        cout << "Ошибка: не удалось создать файл '" << out << "'\n";
        return false;
    }
    
    size_t msize = enc.meta.size();
    f_out.write(reinterpret_cast<const char*>(&msize), sizeof(msize));
    f_out.write(enc.meta.c_str(), msize);
    f_out.write(reinterpret_cast<const char*>(enc.data.data()), enc.data.size());
    
    f_out.close();
    cout << "Файл успешно зашифрован\n";
    return true;
}

bool Mgr::decFile(const string& in, const string& out, const string& key) {
    if (!cur) {
        cout << "Ошибка: шифр не выбран\n";
        return false;
    }
    
    ifstream f_in(in, ios::binary);
    if (!f_in) {
        cout << "Ошибка: файл '" << in << "' не найден\n";
        return false;
    }
    
    f_in.seekg(0, ios::end);
    streamoff fileSize = f_in.tellg();
    f_in.seekg(0, ios::beg);
    
    if (fileSize == 0) {
        cout << "Ошибка: файл пуст\n";
        f_in.close();
        return false;
    }
    
    if (fileSize < (streamoff)sizeof(size_t)) {
        cout << "Ошибка: файл слишком маленький\n";
        f_in.close();
        return false;
    }
    
    size_t msize;
    f_in.read(reinterpret_cast<char*>(&msize), sizeof(msize));
    
    if (msize > 1024 || msize == 0) {
        cout << "Ошибка: файл поврежден или не является зашифрованным\n";
        f_in.close();
        return false;
    }
    
    if (fileSize < (streamoff)(sizeof(msize) + msize)) {
        cout << "Ошибка: файл поврежден\n";
        f_in.close();
        return false;
    }
    
    string meta(msize, '\0');
    f_in.read(&meta[0], msize);
    
    if (!f_in) {
        cout << "Ошибка: не удалось прочитать метаданные\n";
        f_in.close();
        return false;
    }
    
    vector<uint8_t> data((istreambuf_iterator<char>(f_in)), istreambuf_iterator<char>());
    f_in.close();
    
    EncData enc;
    enc.data = data;
    enc.meta = meta;
    
    try {
        auto dec = cur->decrypt(enc, key);
        
        ofstream f_out(out, ios::binary);
        if (!f_out) {
            cout << "Ошибка: не удалось создать файл '" << out << "'\n";
            return false;
        }
        
        f_out.write(reinterpret_cast<const char*>(dec.data()), dec.size());
        f_out.close();
        
        cout << "Файл успешно расшифрован\n";
        return true;
        
    } catch (const exception& e) {
        cout << "Ошибка расшифровки: неверный ключ или файл поврежден\n";
        return false;
    }
}