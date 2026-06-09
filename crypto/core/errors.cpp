#include <iostream>

#include "errors.h"
#include "logger.h"

using namespace std;

string errorToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS: return "Успешно";
        case ErrorCode::ERR_NO_CIPHER: return "Шифр не выбран";
        case ErrorCode::ERR_FILE_NOT_FOUND: return "Файл не найден";
        case ErrorCode::ERR_FILE_EMPTY: return "Файл пуст";
        case ErrorCode::ERR_FILE_CORRUPTED: return "Файл поврежден";
        case ErrorCode::ERR_CANNOT_CREATE: return "Не удалось создать файл";
        case ErrorCode::ERR_DECRYPT_FAIL: return "Ошибка расшифровки";
        case ErrorCode::ERR_INVALID_KEY: return "Неверный ключ";
        case ErrorCode::ERR_NO_MEMORY: return "Недостаточно памяти";
        case ErrorCode::ERR_PERMISSION_DENIED: return "Нет прав доступа";
        case ErrorCode::ERR_INVALID_FORMAT: return "Неверный формат данных";
        case ErrorCode::ERR_AUTH_FAILED: return "Ошибка авторизации";
        case ErrorCode::ERR_CONFIG_CORRUPTED: return "Файл конфигурации поврежден";
        case ErrorCode::ERR_USER_CANCEL: return "Операция отменена пользователем";
        case ErrorCode::ERR_INVALID_CHOICE: return "Неверный выбор";
        case ErrorCode::ERR_INVALID_MENU_CHOICE: return "Неверный ввод меню. Введите число от 0 до 7"; 
        case ErrorCode::ERR_CORRUPTED_CIPHER: return "Шифр повреждён (несовместимая версия или ошибка памяти)";
        default: return "Неизвестная ошибка";
    }
}

bool isError(ErrorCode code) {
    return code != ErrorCode::SUCCESS;
}

void safeShowError(ErrorCode code, const string& details) {
    try {
        string msg = errorToString(code);
        cout << "\033[31mОШИБКА " << static_cast<int>(code) << ": " << msg << "\033[0m";
        if (!details.empty()) {
            cout << " (" << details << ")";
        }
        cout << "\n";
        
        // Логируем ошибку
        Logger::log("Error " + to_string(static_cast<int>(code)) + ": " + msg + " - " + details);
    } catch (...) {
        cout << "\033[31mКритическая ошибка при выводе ошибки\033[0m\n";
    }
}