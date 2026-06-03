#include "core/mgr.h"
#include "core/ui.h"
#include <iostream>

using namespace std;

int main() {
    Mgr mgr;
    string key;
    
    clearScreen();
    cout << "Введите ключ: ";
    getline(cin, key);
    
    int ch;
    do {
        clearScreen();
        cout << "Ключ: " << key << "\n";
        if (mgr.info() != "Шифр не выбран") {
            cout << "Шифр: " << mgr.info() << "\n";
        }
        menu();
        cin >> ch;
        cin.ignore();
        
        switch (ch) {
            case 1: selectCipher(mgr); break;
            case 2: encryptText(mgr, key); break;
            case 3: decryptText(mgr, key); break;
            case 4: encryptFile(mgr, key); break;
            case 5: decryptFile(mgr, key); break;
            case 6: listCiphers(mgr); break;
            case 7: viewEncryptedFile(); break;
            case 0: cout << "До свидания\n"; break;
            default: cout << "Неверный выбор\n"; waitForEnter();
        }
    } while (ch != 0);
    
    return 0;
}
