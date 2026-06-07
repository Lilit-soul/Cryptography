#include "ui.h"
#include "auth.h"
#include <errors.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

void clearScreen() {
    cout << "\033[2J\033[1;1H";
    cout.flush();
}

void printHex(const string& s) {
    for (unsigned char c : s)
        cout << hex << setw(2) << setfill('0') << (int)c;
    cout << dec;
}

void menu() {
    cout << "\n\033[1;35m               FlexCipher     \033[0m\n";
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│ 1. Выбрать шифр                     │\n";
    cout << "│ 2. Шифрование + расшифровка текста  │\n";
    cout << "│ 3. Зашифровать файл                 │\n";
    cout << "│ 4. Расшифровать файл                │\n";
    cout << "│ 5. Список шифров                    │\n";
    cout << "│ 6. Просмотреть зашифрованный файл   │\n";
    cout << "│ 7. Сменить мастер-пароль            │\n";
    cout << "│ 0. Выход                            │\n";
    cout << "└─────────────────────────────────────┘\n";
    cout << "Выбор: ";
}


void waitForEnter() {
    cout << "\nНажмите Enter для продолжения...";
    cin.get();
}

bool runAuth(Auth& auth) {
    clearScreen();
    
    // Первый запуск или вход
    if (auth.isFirstRun()) {
        if (!auth.setupMasterPassword()) {
            cout << "Ошибка создания пароля. Программа завершена.\n";
            return false;
        }
        cout << "\nМастер-пароль создан!\n";
        waitForEnter();
        clearScreen();
    }
    
    // Вход в программу
    int attempts = 0;
    while (!auth.login()) {
        attempts++;
        if (attempts >= 3) {
            cout << "Превышено количество попыток. Программа завершена.\n";
            return false;
        }
        cout << "\033[31mНеверный пароль.\033[0m Осталось попыток: " << 3 - attempts << "\n";
    }
    
    return true;
}

string getEncryptionKey() {
    string key;
    cout << "Введите ключ шифрования: ";
    getline(cin, key);
    
    if (key.empty()) {
        cout << "Предупреждение: ключ пуст. Шифрование будет небезопасным!\n";
        waitForEnter();
    }
    
    return key;
}

void selectCipher(Mgr& mgr) {
    auto list = mgr.list();
    if (list.empty()) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "нет доступных шифров");
        waitForEnter();
        return;
    }
    
    cout << "\nДоступные шифры:\n";
    for (size_t i = 0; i < list.size(); i++) {
        cout << "  " << i+1 << ". " << list[i] << "\n";
    }
    
    cout << "\nВведите название: ";
    string inp;
    getline(cin, inp);
    
    if (inp.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "название не введено");
        waitForEnter();
        return;
    }
    
    if (mgr.select(inp))
        cout << "Шифр выбран\n";
    else
        safeShowError(ErrorCode::ERR_NO_CIPHER, "шифр не найден");
    
    waitForEnter();
}

void testCipher(Mgr& mgr, const string& key) {
    if (mgr.list().empty()) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "нет доступных шифров");
        waitForEnter();
        return;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "шифр не выбран");
        waitForEnter();
        return;
    }
    
    cout << "Введите текст: ";
    string txt;
    getline(cin, txt);
    
    if (txt.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "текст не введён");
        waitForEnter();
        return;
    }
    
    string encrypted;
    if (!mgr.encTextSafe(txt, key, encrypted)) {
        waitForEnter();
        return;
    }
    
    cout << "\nЗашифровано (hex): ";
    printHex(encrypted);
    
    string decrypted;
    if (!mgr.decTextSafe(encrypted, key, decrypted)) {
        waitForEnter();
        return;
    }
    
    cout << "\nРасшифровано: " << decrypted;
    
    if (txt == decrypted) {
        cout << "\n✓ Отлично, всё совпало!\n";
    } else {
        safeShowError(ErrorCode::ERR_DECRYPT_FAIL, "тексты не совпадают");
    }
    
    waitForEnter();
}

void encryptFile(Mgr& mgr, const string& key) {
    if (mgr.list().empty()) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "нет доступных шифров");
        waitForEnter();
        return;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "шифр не выбран");
        waitForEnter();
        return;
    }
    
    string in, out;
    cout << "Входной файл: ";
    getline(cin, in);
    
    if (in.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "имя файла не введено");
        waitForEnter();
        return;
    }
    
    cout << "Выходной файл (рекомендуется .enc): ";
    getline(cin, out);
    
    if (out.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "имя выходного файла не введено");
        waitForEnter();
        return;
    }
    
    mgr.encFileSafe(in, out, key);
    waitForEnter();
}

void decryptFile(Mgr& mgr, const string& key) {
    if (mgr.list().empty()) {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "нет доступных шифров");
        waitForEnter();
        return;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        safeShowError(ErrorCode::ERR_NO_CIPHER, "шифр не выбран");
        waitForEnter();
        return;
    }
    
    string in, out;
    cout << "Входной файл: ";
    getline(cin, in);
    
    if (in.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "имя файла не введено");
        waitForEnter();
        return;
    }
    
    cout << "Выходной файл: ";
    getline(cin, out);
    
    if (out.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "имя выходного файла не введено");
        waitForEnter();
        return;
    }
    
    mgr.decFileSafe(in, out, key);
    waitForEnter();
}

void listCiphers(Mgr& mgr) {
    auto list = mgr.list();
    
    if (list.empty()) {
        cout << "\nНет доступных шифров\n";
        cout << "Проверьте, что файлы шифров (.cpp) есть в папке ciphers/\n";
    } else {
        cout << "\nДоступные шифры (" << list.size() << "):\n";
        for (const auto& c : list)
            cout << "  - " << c << "\n";
    }
    
    waitForEnter();
}

void viewEncryptedFile() {
    string filename;
    cout << "Введите имя зашифрованного файла: ";
    getline(cin, filename);
    
    if (filename.empty()) {
        safeShowError(ErrorCode::ERR_INVALID_FORMAT, "имя файла не введено");
        waitForEnter();
        return;
    }
    
    ifstream file(filename, ios::binary);
    if (!file) {
        safeShowError(ErrorCode::ERR_FILE_NOT_FOUND, filename);
        waitForEnter();
        return;
    }
    
    // Проверка на пустой файл
    file.seekg(0, ios::end);
    if (file.tellg() == 0) {
        safeShowError(ErrorCode::ERR_FILE_EMPTY, filename);
        file.close();
        waitForEnter();
        return;
    }
    file.seekg(0, ios::beg);
    
    size_t msize;
    file.read(reinterpret_cast<char*>(&msize), sizeof(msize));
    
    if (!file || msize > 1024 || msize == 0) {
        cout << "Это не зашифрованный файл программы\n";
        cout << "Файл не содержит корректных метаданных\n";
        file.close();
        waitForEnter();
        return;
    }
    
    string meta(msize, '\0');
    file.read(&meta[0], msize);
    
    if (!file) {
        safeShowError(ErrorCode::ERR_FILE_CORRUPTED, "не удалось прочитать метаданные");
        file.close();
        waitForEnter();
        return;
    }
    
    cout << "\nМетаданные (IV): ";
    printHex(meta);
    cout << "\n\nЗашифрованные данные (первые 256 байт):\n";
    
    vector<uint8_t> data(256, 0);
    file.read(reinterpret_cast<char*>(data.data()), 256);
    size_t bytesRead = file.gcount();
    data.resize(bytesRead);
    
    if (data.empty()) {
        cout << "(нет данных)\n";
    } else {
        for (size_t i = 0; i < data.size(); i++) {
            cout << hex << setw(2) << setfill('0') << (int)data[i];
            if ((i + 1) % 16 == 0)
                cout << "\n";
            else if ((i + 1) % 8 == 0)
                cout << "  ";
            else
                cout << " ";
        }
        cout << dec << "\n";
    }
    
    file.seekg(0, ios::end);
    long long fileSize = static_cast<long long>(file.tellg());
    long long dataSize = fileSize - static_cast<long long>(sizeof(msize)) - static_cast<long long>(msize);
    
    cout << "\nРазмер данных: " << dataSize << " байт\n";
    cout << "Размер IV: " << msize << " байт\n";
    
    file.close();
    waitForEnter();
}