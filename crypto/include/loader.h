#ifndef LOADER_H
#define LOADER_H

#include "interface.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

using CipherMaker = std::function<std::unique_ptr<Cipher>()>;

class Loader {
private:
    std::unordered_map<std::string, CipherMaker> makers;
    
public:
    static Loader& instance() {
        static Loader inst;
        return inst;
    }
    
    void reg(const std::string& name, CipherMaker maker) {
        makers[name] = maker;
    }
    
    std::unique_ptr<Cipher> create(const std::string& name) {
        auto it = makers.find(name);
        if (it != makers.end())
            return it->second();
        return nullptr;
    }
    
    std::vector<std::string> list() const {
        std::vector<std::string> names;
        for (const auto& p : makers)
            names.push_back(p.first);
        return names;
    }
};

#define REG_CIPHER(Class, Name) \
    namespace { \
        struct Reg##Class { \
            Reg##Class() { \
                Loader::instance().reg(Name, \
                    []() -> std::unique_ptr<Cipher> { \
                        return std::make_unique<Class>(); \
                    }); \
            } \
        }; \
        static Reg##Class reg_##Class; \
    }

#endif
