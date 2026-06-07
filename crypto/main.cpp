#include "core/mgr.h"
#include "core/ui.h"
#include "core/auth.h"
#include <iostream>

using namespace std;

int main() {
    Mgr mgr;
    Auth auth;
    string key;
    
    clearScreen();
    // Авторизация
    if (!runAuth(auth)) {
        return 1;
    }
    
    clearScreen();
    cout << "Добро пожаловать!\n";
    
    // Запрашиваем ключ шифрования
    cout << "Введите ключ шифрования: ";
    getline(cin, key);
    
    if (key.empty()) {
        cout << "Предупреждение: ключ пуст. Шифрование будет небезопасным!\n";
        waitForEnter();
    }
    
    int ch;
    do {
        clearScreen();
        cout << "Ключ шифрования: " << (key.empty() ? "(пустой)" : key) << "\n";
        if (mgr.info() != "Шифр не выбран") {
            cout << "Шифр: " << mgr.info() << "\n";
        }
        menu();
        cin >> ch;

        cin.ignore();
        
        switch (ch) {
            case 1: selectCipher(mgr); break;
            case 2: testCipher(mgr, key); break;
            case 3: encryptFile(mgr, key); break;
            case 4: decryptFile(mgr, key); break;
            case 5: listCiphers(mgr); break;
            case 6: viewEncryptedFile(); break;
            case 7: auth.changePassword(); waitForEnter(); break;
            case 0: cout << "До свидания\n"; break;
            default: cout << "Неверный выбор\n"; waitForEnter();
        }
    } while (ch != 0);
    
    return 0;
}
