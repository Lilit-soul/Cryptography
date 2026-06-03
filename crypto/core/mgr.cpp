#include "mgr.h"
#include "../include/loader.h"
#include <fstream>
#include <algorithm>
#include <cctype>

// Функция для приведения строки к нижнему регистру
std::string toLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

// Функция для нормализации имени шифра (игнорируем регистр и пробелы)
std::string normalizeName(const std::string& name) {
    std::string result;
    for (char c : name) {
        if (!std::isspace(c)) {
            result += std::tolower(c);
        }
    }
    return result;
}

bool Mgr::select(const std::string& name) {
    auto list = Loader::instance().list();
    std::string normalizedInput = normalizeName(name);
    
    // Ищем шифр без учета регистра
    for (const auto& cipherName : list) {
        if (normalizeName(cipherName) == normalizedInput) {
            auto c = Loader::instance().create(cipherName);
            if (c) {
                cur = std::move(c);
                return true;
            }
        }
    }
    
    return false;
}

std::vector<std::string> Mgr::list() const {
    return Loader::instance().list();
}

std::string Mgr::info() const {
    if (!cur) return "Шифр не выбран";
    return cur->name() + " | ключ: " + std::to_string(cur->keySize()) + " байт | nonce: " + std::to_string(cur->nonceSize()) + " байт";
}

std::string Mgr::encText(const std::string& text, const std::string& key) {
    if (!cur) return "";
    std::vector<uint8_t> in(text.begin(), text.end());
    auto enc = cur->encrypt(in, key);
    return std::string(enc.data.begin(), enc.data.end());
}

std::string Mgr::decText(const std::string& data, const std::string& key) {
    if (!cur) return "";
    EncData enc;
    enc.data = std::vector<uint8_t>(data.begin(), data.end());
    auto dec = cur->decrypt(enc, key);
    return std::string(dec.begin(), dec.end());
}

std::vector<uint8_t> Mgr::encData(const std::vector<uint8_t>& data, const std::string& key) {
    if (!cur) return {};
    return cur->encrypt(data, key).data;
}

std::vector<uint8_t> Mgr::decData(const std::vector<uint8_t>& data, const std::string& key) {
    if (!cur) return {};
    EncData enc;
    enc.data = data;
    return cur->decrypt(enc, key);
}

bool Mgr::encFile(const std::string& in, const std::string& out, const std::string& key) {
    if (!cur) return false;
    
    std::ifstream f_in(in, std::ios::binary);
    if (!f_in) return false;
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f_in)), std::istreambuf_iterator<char>());
    f_in.close();
    
    auto enc = cur->encrypt(data, key);
    
    std::ofstream f_out(out, std::ios::binary);
    if (!f_out) return false;
    
    size_t msize = enc.meta.size();
    f_out.write(reinterpret_cast<const char*>(&msize), sizeof(msize));
    f_out.write(enc.meta.c_str(), msize);
    f_out.write(reinterpret_cast<const char*>(enc.data.data()), enc.data.size());
    
    f_out.close();
    return true;
}

bool Mgr::decFile(const std::string& in, const std::string& out, const std::string& key) {
    if (!cur) return false;
    
    std::ifstream f_in(in, std::ios::binary);
    if (!f_in) return false;
    
    size_t msize;
    f_in.read(reinterpret_cast<char*>(&msize), sizeof(msize));
    
    std::string meta(msize, '\0');
    f_in.read(&meta[0], msize);
    
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f_in)), std::istreambuf_iterator<char>());
    f_in.close();
    
    EncData enc;
    enc.data = data;
    enc.meta = meta;
    
    auto dec = cur->decrypt(enc, key);
    
    std::ofstream f_out(out, std::ios::binary);
    if (!f_out) return false;
    
    f_out.write(reinterpret_cast<const char*>(dec.data()), dec.size());
    f_out.close();
    
    return true;
}
