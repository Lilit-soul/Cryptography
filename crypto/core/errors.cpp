#include "errors.h"
#include <iostream>

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
        default: return "Неизвестная ошибка";
    }
}

void safeShowError(ErrorCode code, const string& details) {
    try {
        cout << "ОШИБКА " << (int)code << ": " << errorToString(code);
        if (!details.empty()) {
            cout << " (" << details << ")";
        }
        cout << "\n";
    } catch (...) {
        cout << "Критическая ошибка\n";
    }
}
