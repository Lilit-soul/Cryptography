#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cerrno>
#include <exception>
#include <stdexcept>

#include "mgr.h"
#include "../include/loader.h"

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
    try {
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
    } catch (const exception& e) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, e.what());
    } catch (...) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "неизвестная ошибка");
    }
    return false;
}

Cipher::Type Mgr::getCipherType() const {
    if (!cur) return Cipher::Type::SYMMETRIC;
    return cur->getType();
}

pair<string, string> Mgr::generateKeyPair() {
    if (!cur) return {"", ""};
    return cur->generateKeyPair();
}

string Mgr::getPublicKey(const string& privateKey) {
    if (!cur) return "";
    return cur->getPublicKey(privateKey);
}

string Mgr::generatePrivateKey() {
    if (!cur) return "";
    return cur->generatePrivateKey();
}

string Mgr::computePublicKey(const string& privateKey) {
    if (!cur) return "";
    return cur->computePublicKey(privateKey);
}

string Mgr::computeSharedSecret(const string& privateKey, const string& otherPublic) {
    if (!cur) return "";
    return cur->computeSharedSecret(privateKey, otherPublic);
}

// Защищённый вызов методов шифра
template<typename T>
T safeCall(Cipher* cipher, const string& methodName, std::function<T()> func, T defaultValue = T{}) {
    if (!cipher) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "Шифр не выбран");
        return defaultValue;
    }
    
    try {
        return func();
    } catch (const std::exception& e) {
        safeShowError(ErrorCode::ERR_CORRUPTED_CIPHER, 
                     "Ошибка при вызове " + methodName + ": " + e.what());
        return defaultValue;
    } catch (...) {
        safeShowError(ErrorCode::ERR_CORRUPTED_CIPHER, 
                     "Неизвестная ошибка при вызове " + methodName);
        return defaultValue;
    }
}

vector<string> Mgr::list() const {
    try {
        return Loader::instance().list();
    } catch (...) {
        return vector<string>();
    }
}

string Mgr::info() const {
    if (!cur) return "Шифр не выбран";
    
    try {
        string name = cur->name();
        int kSize = cur->keySize();
        int nSize = cur->nonceSize();
        return name + " | ключ: " + to_string(kSize) + 
               " байт | nonce (IV): " + to_string(nSize) + " байт";
    } catch (const std::exception& e) {
        safeShowError(ErrorCode::ERR_CORRUPTED_CIPHER, e.what());
        return "Шифр повреждён";
    } catch (...) {
        safeShowError(ErrorCode::ERR_CORRUPTED_CIPHER, "неизвестная ошибка");
        return "Шифр повреждён";
    }
    return cur->name() + " | ключ: " + to_string(cur->keySize()) + " байт | nonce (IV): " + to_string(cur->nonceSize()) + " байт";
}

ErrorCode Mgr::encText(const string& text, const string& key, string& output) {
    if (!cur) return ErrorCode::ERR_NO_CIPHER;
    
    if (text.empty()) return ErrorCode::ERR_INVALID_FORMAT;
    
    try {
        vector<uint8_t> in(text.begin(), text.end());
        auto enc = cur->encrypt(in, key);
        
        string result;
        uint32_t ivSize = enc.meta.size();
        result.append(reinterpret_cast<const char*>(&ivSize), sizeof(ivSize));
        result.append(enc.meta);
        result.append(enc.data.begin(), enc.data.end());
        
        output = result;
        return ErrorCode::SUCCESS;
        
    } catch (const bad_alloc& e) {
        return ErrorCode::ERR_NO_MEMORY;
    } catch (const exception& e) {
        return ErrorCode::ERR_INVALID_KEY;
    } catch (...) {
        return ErrorCode::ERR_INVALID_KEY;
    }
}

ErrorCode Mgr::decText(const string& data, const string& key, string& output) {
    if (!cur) return ErrorCode::ERR_NO_CIPHER;
    
    if (data.size() < sizeof(uint32_t)) return ErrorCode::ERR_INVALID_FORMAT;
    
    try {
        uint32_t ivSize;
        memcpy(&ivSize, data.data(), sizeof(ivSize));
        
        if (ivSize > 1024 || ivSize == 0) return ErrorCode::ERR_INVALID_FORMAT;
        
        if (data.size() < sizeof(ivSize) + ivSize) return ErrorCode::ERR_INVALID_FORMAT;
        
        string meta = data.substr(sizeof(ivSize), ivSize);
        vector<uint8_t> encData(data.begin() + sizeof(ivSize) + ivSize, data.end());
        
        EncData enc;
        enc.data = encData;
        enc.meta = meta;
        
        auto dec = cur->decrypt(enc, key);
        output = string(dec.begin(), dec.end());
        return ErrorCode::SUCCESS;
        
    } catch (const bad_alloc& e) {
        return ErrorCode::ERR_NO_MEMORY;
    } catch (const exception& e) {
        return ErrorCode::ERR_DECRYPT_FAIL;
    } catch (...) {
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
}

// Безопасные обёртки (не выбрасывают исключения наружу)
bool Mgr::encTextSafe(const string& text, const string& key, string& output) {
    try {
        ErrorCode code = encText(text, key, output);
        if (code != ErrorCode::SUCCESS) {
            safeShowError(code);
            return false;
        }
        return true;
    } catch (...) {
        safeShowError(ErrorCode::ERR_INVALID_KEY);
        return false;
    }
}

bool Mgr::decTextSafe(const string& data, const string& key, string& output) {
    try {
        ErrorCode code = decText(data, key, output);
        if (code != ErrorCode::SUCCESS) {
            safeShowError(code);
            return false;
        }
        return true;
    } catch (...) {
        safeShowError(ErrorCode::ERR_DECRYPT_FAIL);
        return false;
    }
}

// mgr.cpp
ErrorCode Mgr::encFile(const string& in, const string& out, const string& key) {
    if (!cur) return ErrorCode::ERR_NO_CIPHER;
    
    try {
        ifstream f_in(in, ios::binary);
        if (!f_in) {
            return ErrorCode::ERR_FILE_NOT_FOUND;
        }
        
        f_in.seekg(0, ios::end);
        if (f_in.tellg() == 0) {
            f_in.close();
            return ErrorCode::ERR_FILE_EMPTY;
        }
        f_in.seekg(0, ios::beg);
        
        vector<uint8_t> data((istreambuf_iterator<char>(f_in)), istreambuf_iterator<char>());
        f_in.close();
        
        // Защищённый вызов encrypt
        EncData enc;
        try {
            enc = cur->encrypt(data, key);
        } catch (const std::exception& e) {
            safeShowError(ErrorCode::ERR_CORRUPTED_CIPHER, 
                         "Ошибка при шифровании: " + string(e.what()));
            return ErrorCode::ERR_INVALID_KEY;
        } catch (...) {
            safeShowError(ErrorCode::ERR_CORRUPTED_CIPHER, 
                         "Неизвестная ошибка при шифровании");
            return ErrorCode::ERR_INVALID_KEY;
        }
        
        ofstream f_out(out, ios::binary);
        if (!f_out) {
            return ErrorCode::ERR_CANNOT_CREATE;
        }
        
        size_t msize = enc.meta.size();
        f_out.write(reinterpret_cast<const char*>(&msize), sizeof(msize));
        f_out.write(enc.meta.c_str(), msize);
        f_out.write(reinterpret_cast<const char*>(enc.data.data()), enc.data.size());
        
        f_out.close();
        return ErrorCode::SUCCESS;
        
    } catch (const bad_alloc& e) {
        return ErrorCode::ERR_NO_MEMORY;
    } catch (const std::exception& e) {
        safeShowError(ErrorCode::ERR_INVALID_KEY, e.what());
        return ErrorCode::ERR_INVALID_KEY;
    } catch (...) {
        safeShowError(ErrorCode::ERR_INVALID_KEY, "неизвестная ошибка");
        return ErrorCode::ERR_INVALID_KEY;
    }
}

ErrorCode Mgr::decFile(const string& in, const string& out, const string& key) {
    if (!cur) return ErrorCode::ERR_NO_CIPHER;
    
    try {
        ifstream f_in(in, ios::binary);
        if (!f_in) {
            return ErrorCode::ERR_FILE_NOT_FOUND;
        }
        
        f_in.seekg(0, ios::end);
        streamoff fileSize = f_in.tellg();
        f_in.seekg(0, ios::beg);
        
        if (fileSize == 0) {
            f_in.close();
            return ErrorCode::ERR_FILE_EMPTY;
        }
        
        if (fileSize < (streamoff)sizeof(size_t)) {
            f_in.close();
            return ErrorCode::ERR_FILE_CORRUPTED;
        }
        
        size_t msize;
        f_in.read(reinterpret_cast<char*>(&msize), sizeof(msize));
        
        if (msize > 1024 || msize == 0) {
            f_in.close();
            return ErrorCode::ERR_FILE_CORRUPTED;
        }
        
        if (fileSize < (streamoff)(sizeof(msize) + msize)) {
            f_in.close();
            return ErrorCode::ERR_FILE_CORRUPTED;
        }
        
        string meta(msize, '\0');
        f_in.read(&meta[0], msize);
        
        if (!f_in) {
            f_in.close();
            return ErrorCode::ERR_FILE_CORRUPTED;
        }
        
        vector<uint8_t> data((istreambuf_iterator<char>(f_in)), istreambuf_iterator<char>());
        f_in.close();
        
        EncData enc;
        enc.data = data;
        enc.meta = meta;
        
        auto dec = cur->decrypt(enc, key);
        
        ofstream f_out(out, ios::binary);
        if (!f_out) {
            return ErrorCode::ERR_CANNOT_CREATE;
        }
        
        f_out.write(reinterpret_cast<const char*>(dec.data()), dec.size());
        f_out.close();
        
        return ErrorCode::SUCCESS;
        
    } catch (const bad_alloc& e) {
        return ErrorCode::ERR_NO_MEMORY;
    } catch (const exception& e) {
        return ErrorCode::ERR_DECRYPT_FAIL;
    } catch (...) {
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
}

bool Mgr::encFileSafe(const string& in, const string& out, const string& key) {
    try {
        ErrorCode code = encFile(in, out, key);
        if (code != ErrorCode::SUCCESS) {
            safeShowError(code, in);
            return false;
        }
        cout << "Файл успешно зашифрован\n";
        return true;
    } catch (...) {
        safeShowError(ErrorCode::ERR_INVALID_KEY);
        return false;
    }
}

bool Mgr::decFileSafe(const string& in, const string& out, const string& key) {
    try {
        ErrorCode code = decFile(in, out, key);
        if (code != ErrorCode::SUCCESS) {
            safeShowError(code, in);
            return false;
        }
        cout << "Файл успешно расшифрован\n";
        return true;
    } catch (...) {
        safeShowError(ErrorCode::ERR_DECRYPT_FAIL);
        return false;
    }
}