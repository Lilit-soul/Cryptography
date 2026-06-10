#include <iostream>

#include "core/mgr.h"
#include "core/ui.h"
#include "core/auth.h"
#include "core/errors.h"
#include "core/menu_actions.h"


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

    ensureDataDir();

    getEncryptionKey(key);
    
    MenuAction action = MenuAction::EXIT;
    int choice;
    ErrorCode result;
    
    do {
        clearScreen();

        cout << "Ключ шифрования: " << (key.empty() ? "(пустой)" : key) << "\n";
        if (mgr.info() != "Шифр не выбран!") {
            cout << "Шифр: " << mgr.info() << "\n";
        }

        showMenu();
        
        choice = getMenuChoice();
        if (choice == -1) {
            safeShowError(ErrorCode::ERR_INVALID_MENU_CHOICE);
            waitForEnter();
            continue;
        }

        action = static_cast<MenuAction>(choice);
        result = ErrorCode::SUCCESS;
        
        switch (action) {
            case MenuAction::SELECT_CIPHER:
                result = selectCipher(mgr);
                if (result == ErrorCode::SUCCESS && mgr.getCipher()) {
                    mgr.getCipher()->setup(key);
                }
                break;

            case MenuAction::TEST_TEXT:
                result = testCipher(mgr, key);
                break;
                
            case MenuAction::PROCESS_FILE:
                result = processFile(mgr, key);
                break;
                
            case MenuAction::LIST_CIPHERS:
                result = listCiphers(mgr);
                break;
                
            case MenuAction::VIEW_ENCRYPTED:
                result = viewEncryptedFile();
                break;
                
            case MenuAction::CHANGE_PASSWORD:
                auth.changePassword();
                break;
            
            case MenuAction::CHANGE_KEY:
                changeKey(key);
                break;
                
            case MenuAction::EXIT:
                cout << "Выход. До свидания!\n";
                break;
                
            default:
                result = ErrorCode::ERR_INVALID_CHOICE;
                break;
        }
        
        // Обработка ошибок
        if (isError(result)) {
            safeShowError(result);
        }
        
        // ОДНО ожидание для всех действий (кроме выхода)
        if (action != MenuAction::EXIT) {
            waitForEnter();
        }
        
    } while (action != MenuAction::EXIT);
    
    return 0;
}