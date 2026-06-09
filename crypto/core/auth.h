#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <fstream>
#include <iostream>

class Auth {
private:
    std::string passwordHash;
    const std::string CONFIG_FILE = ".encrypt_config";
    
    std::string hashPassword(const std::string& password) {
        unsigned long hash = 5381;
        for (char c : password) {
            hash = ((hash << 5) + hash) + c;
        }
        return std::to_string(hash);
    }
    
public:
    bool isFirstRun() {
        std::ifstream f(CONFIG_FILE);
        return !f.good();
    }
    
    bool setupMasterPassword() {
        std::string pwd1, pwd2;
        std::cout << "\n=== Первый запуск ===\n";
        std::cout << "Создайте мастер-пароль: ";
        std::getline(std::cin, pwd1);
        
        if (pwd1.empty()) {
            std::cout << "Ошибка: пароль не может быть пустым\n";
            return false;
        }
        
        std::cout << "Повторите пароль: ";
        std::getline(std::cin, pwd2);
        
        if (pwd1 != pwd2) {
            std::cout << "Ошибка: пароли не совпадают\n";
            return false;
        }
        
        passwordHash = hashPassword(pwd1);
        
        std::ofstream f(CONFIG_FILE);
        if (!f) return false;
        
        f << passwordHash << "\n";
        f.close();
        return true;
    }
    
    bool login() {
        std::string pwd;
        std::cout << "\n=== Вход в программу ===\n";
        std::cout << "Введите мастер-пароль: ";
        std::getline(std::cin, pwd);
        
        std::ifstream f(CONFIG_FILE);
        if (!f) return false;
        
        std::getline(f, passwordHash);
        f.close();
        
        return hashPassword(pwd) == passwordHash;
    }
    
    bool changePassword() {
        std::string oldPwd, newPwd1, newPwd2;
        
        std::cout << "\n=== Смена пароля ===\n";
        std::cout << "Введите старый пароль: ";
        std::getline(std::cin, oldPwd);
        
        if (hashPassword(oldPwd) != passwordHash) {
            std::cout << "Ошибка: неверный старый пароль\n";
            return false;
        }
        
        std::cout << "Введите новый пароль: ";
        std::getline(std::cin, newPwd1);
        
        if (newPwd1.empty()) {
            std::cout << "Ошибка: пароль не может быть пустым\n";
            return false;
        }
        
        std::cout << "Повторите новый пароль: ";
        std::getline(std::cin, newPwd2);
        
        if (newPwd1 != newPwd2) {
            std::cout << "Ошибка: пароли не совпадают\n";
            return false;
        }
        
        passwordHash = hashPassword(newPwd1);
        
        std::ofstream f(CONFIG_FILE);
        if (!f) return false;
        
        f << passwordHash << "\n";
        f.close();
        
        std::cout << "Пароль успешно изменён\n";
        return true;
    }
};

#endif
