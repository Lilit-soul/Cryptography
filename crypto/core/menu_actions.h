#ifndef MENU_ACTIONS_H
#define MENU_ACTIONS_H

#include <string>

enum class MenuAction : int {
    EXIT = 0,
    SELECT_CIPHER = 1,
    TEST_TEXT = 2,
    PROCESS_FILE = 3,
    LIST_CIPHERS = 4,
    VIEW_ENCRYPTED = 5,
    CHANGE_PASSWORD = 6,
    CHANGE_KEY = 7
};

inline std::string menuActionToString(MenuAction action) {
    switch(action) {
        case MenuAction::EXIT: return "Выход";
        case MenuAction::SELECT_CIPHER: return "Выбрать шифр";
        case MenuAction::TEST_TEXT: return "Тест шифрования";
        case MenuAction::PROCESS_FILE: return "Шифрование файлов";
        case MenuAction::LIST_CIPHERS: return "Список шифров";
        case MenuAction::VIEW_ENCRYPTED: return "Просмотр файла";
        case MenuAction::CHANGE_PASSWORD: return "Смена пароля";
        case MenuAction::CHANGE_KEY: return "Смена ключа";
        default: return "Неизвестно";
    }
}

#endif