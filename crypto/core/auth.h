#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class Auth {
private:
    string passwordHash;
    const string CONFIG_FILE = ".encrypt_config";
    
    string hashPassword(const string& password) {
        // Простая хеш-функция (для демонстрации)
        // В реальном проекте используйте SHA256
        unsigned long hash = 5381;
        for (char c : password) {
            hash = ((hash << 5) + hash) + c;
        }
        return to_string(hash);
    }
    
public:
    bool isFirstRun() {
        ifstream f(CONFIG_FILE);
        return !f.good();
    }
    
    bool setupMasterPassword() {
        string pwd1, pwd2;
        cout << "\n=== Первый запуск ===\n";
        cout << "Создайте мастер-пароль: ";
        getline(cin, pwd1);
        
        if (pwd1.empty()) {
            cout << "Ошибка: пароль не может быть пустым\n";
            return false;
        }
        
        cout << "Повторите пароль: ";
        getline(cin, pwd2);
        
        if (pwd1 != pwd2) {
            cout << "Ошибка: пароли не совпадают\n";
            return false;
        }
        
        passwordHash = hashPassword(pwd1);
        
        ofstream f(CONFIG_FILE);
        if (!f) return false;
        
        f << passwordHash << "\n";
        f.close();
        return true;
    }
    
    bool login() {
        string pwd;
        cout << "\n=== Вход в программу ===\n";
        cout << "Введите мастер-пароль: ";
        getline(cin, pwd);
        
        ifstream f(CONFIG_FILE);
        if (!f) return false;
        
        getline(f, passwordHash);
        f.close();
        
        return hashPassword(pwd) == passwordHash;
    }
    
    bool changePassword() {
        string oldPwd, newPwd1, newPwd2;
        
        cout << "\n=== Смена пароля ===\n";
        cout << "Введите старый пароль: ";
        getline(cin, oldPwd);
        
        if (hashPassword(oldPwd) != passwordHash) {
            cout << "Ошибка: неверный старый пароль\n";
            return false;
        }
        
        cout << "Введите новый пароль: ";
        getline(cin, newPwd1);
        
        if (newPwd1.empty()) {
            cout << "Ошибка: пароль не может быть пустым\n";
            return false;
        }
        
        cout << "Повторите новый пароль: ";
        getline(cin, newPwd2);
        
        if (newPwd1 != newPwd2) {
            cout << "Ошибка: пароли не совпадают\n";
            return false;
        }
        
        passwordHash = hashPassword(newPwd1);
        
        ofstream f(CONFIG_FILE);
        if (!f) return false;
        
        f << passwordHash << "\n";
        f.close();
        
        cout << "Пароль успешно изменён\n";
        return true;
    }
};

#endif
