#include <iostream>
#include <iomanip>
#include <fstream>
#include <limits>
#include <dirent.h>
#include <vector>
#include <sys/stat.h>

#include "ui.h"
#include "auth.h"
#include "errors.h"
#include "logger.h"

using namespace std;

// Цвета для терминала
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

void clearScreen() {
    system("clear");
}

void printHex(const string& s) {
    for (unsigned char c : s)
        cout << hex << setw(2) << setfill('0') << (int)c;
    cout << dec;
}

void showMenu() {
    cout << "\n" << COLOR_MAGENTA << "               FlexCipher     " << COLOR_RESET << "\n";
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│ 1. Выбрать шифр                     │\n";
    cout << "│ 2. Шифрование + расшифровка текста  │\n";
    cout << "│ 3. Шифрование + расшифровка файла   │\n";
    cout << "│ 4. Список шифров                    │\n";
    cout << "│ 5. Просмотреть зашифрованный файл   │\n";
    cout << "│ 6. Сменить мастер-пароль            │\n";
    cout << "│ 7. Сменить ключ шифрования          │\n";
    cout << "│ 0. Выход                            │\n";
    cout << "└─────────────────────────────────────┘\n";
    cout << "Выбор: ";
}

void waitForEnter() {
    cout << "\nНажмите Enter для продолжения...";
    cin.get(); 
}

int getMenuChoice() {
    string input;
    getline(cin, input);
    
    if (input.empty()) return -1;
    
    try {
        size_t pos;
        int choice = stoi(input, &pos);
        
        if (pos != input.length()) return -1;
        if (choice < 0 || choice > 7) return -1;
        
        return choice;
    } catch (const exception&) {
        return -1;
    }
}

bool runAuth(Auth& auth) {
    clearScreen();
    
    if (auth.isFirstRun()) {
        if (!auth.setupMasterPassword()) {
            cout << COLOR_RED << "Ошибка создания пароля. Программа завершена." << COLOR_RESET << "\n";
            return false;
        }
        cout << COLOR_GREEN << "\nМастер-пароль создан!" << COLOR_RESET << "\n";
        waitForEnter();
        clearScreen();
    }
    
    int attempts = 0;
    while (!auth.login()) {
        attempts++;
        if (attempts >= 3) {
            cout << COLOR_RED << "Превышено количество попыток. Программа завершена." << COLOR_RESET << "\n";
            return false;
        }
        cout << COLOR_RED << "Неверный пароль." << COLOR_RESET << " Осталось попыток: " << 3 - attempts << "\n";
    }
    
    return true;
}

void getEncryptionKey(string& key) {
    clearScreen();
    cout << COLOR_GREEN << "Добро пожаловать!" << COLOR_RESET << "\n";
    cout << "Введите ключ шифрования: ";
    getline(cin, key);
    
    if (key.empty()) {
        cout << COLOR_YELLOW << "Предупреждение: ключ пуст. Шифрование будет небезопасным!" << COLOR_RESET << "\n";
        waitForEnter();
    }
}

bool isRegularFile(const string& path) {
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0) {
        return false;  // не существует или ошибка
    }
    return S_ISREG(path_stat.st_mode);  // true только для обычных файлов
}

ErrorCode selectCipher(Mgr& mgr) {
    auto list = mgr.list();
    if (list.empty()) {
        return ErrorCode::ERR_NO_CIPHER;
    }
    
    cout << "\nДоступные шифры:\n";
    for (size_t i = 0; i < list.size(); i++) {
        cout << "  " << i+1 << ". " << list[i] << "\n";
    }
    
    cout << "\nВведите название: ";
    string inp;
    getline(cin, inp);
    
    if (inp.empty()) {
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    if (mgr.select(inp)) {
        // Проверяем, что шифр корректен
        try {
            string info = mgr.info();
            if (info.find("повреждён") != string::npos) {
                cout << COLOR_YELLOW << "Шифр выбран, но может работать некорректно" << COLOR_RESET << "\n";
            } else {
                cout << COLOR_GREEN << "Шифр выбран" << COLOR_RESET << "\n";
            }
        } catch (const std::exception& e) {
            cout << COLOR_RED << "Ошибка: выбранный шифр повреждён (" << e.what() << ")" << COLOR_RESET << "\n";
            return ErrorCode::ERR_CORRUPTED_CIPHER;
        } catch (...) {
            cout << COLOR_RED << "Ошибка: выбранный шифр повреждён" << COLOR_RESET << "\n";
            return ErrorCode::ERR_CORRUPTED_CIPHER;
        }
        
        Logger::log("Cipher selected: " + inp);
        return ErrorCode::SUCCESS;
    } else {
        return ErrorCode::ERR_NO_CIPHER;
    }
}

ErrorCode testCipher(Mgr& mgr, const string& key) {
    auto list = mgr.list();
    if (list.empty()) {
        return ErrorCode::ERR_NO_CIPHER;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        return ErrorCode::ERR_NO_CIPHER;
    }
    
    cout << "Введите текст: ";
    string txt;
    getline(cin, txt);
    
    if (txt.empty()) {
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    string encrypted;
    if (!mgr.encTextSafe(txt, key, encrypted)) {
        return ErrorCode::ERR_INVALID_KEY;
    }
    
    cout << "\nЗашифровано (hex): ";
    printHex(encrypted);
    
    string decrypted;
    if (!mgr.decTextSafe(encrypted, key, decrypted)) {
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
    
    cout << "\nРасшифровано: " << decrypted;
    
    if (txt == decrypted) {
        cout << "\n" << COLOR_GREEN << "Отлично, всё совпало!" << COLOR_RESET << "\n";
        Logger::log("Test cipher: SUCCESS");
        return ErrorCode::SUCCESS;
    } else {
        Logger::log("Test cipher: FAILED - texts don't match");
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
}

ErrorCode processFile(Mgr& mgr, const string& key) {
    auto list = mgr.list();
    if (list.empty()) {
        return ErrorCode::ERR_NO_CIPHER;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        return ErrorCode::ERR_NO_CIPHER;
    }
    
    // Показываем список файлов
    cout << "\nФайлы в текущей директории:\n";
    
    DIR* dir = opendir(".");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            string name = entry->d_name;
            if (name != "." && name != "..") {
                cout << "   - " << name << "\n";
            }
        }
        closedir(dir);
    }
    
    cout << "\nВведите имя файла: ";
    string filename;
    getline(cin, filename);
    
    if (filename.empty()) {
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    // Проверяем, что файл не является уже зашифрованным
    if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".enc") {
        cout << COLOR_RED << "Нельзя шифровать уже зашифрованный файл!" << COLOR_RESET << "\n";
        cout << "Файлы с расширением .enc - это зашифрованные файлы.\n";
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    // Проверяем, что это не директория
    struct stat path_stat;
    if (stat(filename.c_str(), &path_stat) != 0) {
        return ErrorCode::ERR_FILE_NOT_FOUND;
    }
    
    if (S_ISDIR(path_stat.st_mode)) {
        cout << COLOR_RED << "Ошибка: \"" << filename << "\" является директорией, а не файлом!" << COLOR_RESET << "\n";
        return ErrorCode::ERR_INVALID_FORMAT;
    }

    // Проверяем существование файла
    ifstream test(filename);
    if (!test) {
        return ErrorCode::ERR_FILE_NOT_FOUND;
    }
    test.close();
    
    // === 1. Читаем исходный файл ===
    ifstream inFile(filename, ios::binary);
    vector<uint8_t> originalData((istreambuf_iterator<char>(inFile)), 
                                   istreambuf_iterator<char>());
    inFile.close();
    
    // Сохраняем оригинальное расширение
    string originalExt;
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != string::npos) {
        originalExt = filename.substr(dotPos);
    }
    
    cout << "\nИсходный файл: " << COLOR_CYAN << filename << COLOR_RESET << " (расширение: " << originalExt << ")\n";
    
    // Шифруем данные
    string encData;
    if (!mgr.encTextSafe(string(originalData.begin(), originalData.end()), key, encData)) {
        return ErrorCode::ERR_INVALID_KEY;
    }    
    
    // Сохраняем зашифрованный файл
    string encFilename;
    if (dotPos != string::npos) {
        encFilename = filename.substr(0, dotPos) + ".enc";
    } else {
        encFilename = filename + ".enc";
    }

    ofstream encFile(encFilename, ios::binary);
    if (!encFile) {
        return ErrorCode::ERR_CANNOT_CREATE;
    }
    encFile.write(encData.c_str(), encData.size());
    encFile.close();
    cout << COLOR_GREEN << "Зашифровано: " << COLOR_RESET << encFilename << "\n";
    
    // Расшифровываем для проверки
    string decrypted;
    if (!mgr.decTextSafe(encData, key, decrypted)) {
        cout << COLOR_RED << "Ошибка: не удалось расшифровать (возможно, неверный ключ)" << COLOR_RESET << "\n";
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
    
    // Проверяем, не существует ли уже файл decr_
    string decFilename = "decr_" + filename;
    ifstream existing(decFilename);
    if (existing) {
        cout << COLOR_YELLOW << "Файл " << decFilename << " уже существует." << COLOR_RESET << "\n";
        cout << "Перезаписать? (y/N): ";
        string answer;
        getline(cin, answer);
        if (answer != "y" && answer != "Y") {
            cout << COLOR_YELLOW << "Операция отменена." << COLOR_RESET << "\n";
            return ErrorCode::ERR_USER_CANCEL;
        }
    }
    existing.close();
    
    // Сохраняем расшифрованный файл с префиксом decr_
    ofstream decFile(decFilename, ios::binary);
    if (!decFile) {
        return ErrorCode::ERR_CANNOT_CREATE;
    }
    decFile.write(decrypted.c_str(), decrypted.size());
    decFile.close();
    cout << COLOR_GREEN << "Расшифровано (проверка): " << COLOR_RESET << decFilename << "\n";
    
    // Сравниваем с оригиналом
    if (originalData.size() != decrypted.size()) {
        cout << COLOR_YELLOW << "ВНИМАНИЕ: Размер не совпадает!" << COLOR_RESET << "\n";
        cout << " Оригинал: " << originalData.size() << " байт\n";
        cout << " Расшифровано: " << decrypted.size() << " байт\n";
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
    
    bool match = true;
    for (size_t i = 0; i < originalData.size(); i++) {
        if (originalData[i] != (unsigned char)decrypted[i]) {
            match = false;
            break;
        }
    }
    
    if (match) {
        cout << "\n" << COLOR_GREEN << "УСПЕХ! Файл зашифрован и успешно расшифрован." << COLOR_RESET << "\n";
        Logger::log("File processed: " + filename + " -> " + encFilename + " (verified)");
        return ErrorCode::SUCCESS;
    } else {
        cout << "\n" << COLOR_RED << "ОШИБКА: Расшифрованные данные не совпадают с оригиналом!" << COLOR_RESET << "\n";
        cout << "   Возможно, проблема с шифром или ключом.\n";
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
}

ErrorCode listCiphers(Mgr& mgr) {
    auto list = mgr.list();
    
    if (list.empty()) {
        cout << "\n" << COLOR_YELLOW << "Нет доступных шифров" << COLOR_RESET << "\n";
        cout << "Проверьте, что файлы шифров (.cpp) есть в папке ciphers/\n";
        return ErrorCode::ERR_NO_CIPHER;
    } else {
        cout << "\n" << COLOR_CYAN << "Доступные шифры (" << list.size() << "):" << COLOR_RESET << "\n";
        for (const auto& c : list)
            cout << "  - " << c << "\n";
        return ErrorCode::SUCCESS;
    }
}

void handleAsymmetricCipher(Mgr& mgr, string& key) {
    cout << "\n" << COLOR_CYAN << "=== Асимметричный шифр ===" << COLOR_RESET << "\n";
    
    auto [privKey, pubKey] = mgr.generateKeyPair();
    
    cout << "Закрытый ключ (ваш): " << COLOR_YELLOW << privKey << COLOR_RESET << "\n";
    cout << "Открытый ключ (для других): " << COLOR_GREEN << pubKey << COLOR_RESET << "\n";
    
    cout << "\nВведите открытый ключ собеседника: ";
    string otherPub;
    getline(cin, otherPub);
    
    if (!otherPub.empty()) {
        key = otherPub;  // для шифрования используем открытый ключ
        cout << COLOR_GREEN << "Открытый ключ установлен для шифрования" << COLOR_RESET << "\n";
    }
    
    waitForEnter();
}

void handleKeyExchange(Mgr& mgr, string& key) {
    cout << "\n" << COLOR_CYAN << "=== Diffie-Hellman обмен ключами ===" << COLOR_RESET << "\n";
    
    string myPrivate = mgr.generatePrivateKey();
    string myPublic = mgr.computePublicKey(myPrivate);
    
    cout << "Ваш закрытый ключ: " << COLOR_YELLOW << myPrivate << COLOR_RESET << "\n";
    cout << "Ваш открытый ключ: " << COLOR_GREEN << myPublic << COLOR_RESET << "\n";
    
    cout << "\nВведите открытый ключ собеседника: ";
    string otherPublic;
    getline(cin, otherPublic);
    
    if (!otherPublic.empty()) {
        string sharedSecret = mgr.computeSharedSecret(myPrivate, otherPublic);
        key = sharedSecret;
        cout << COLOR_GREEN << "Общий секрет: " << sharedSecret << COLOR_RESET << "\n";
        cout << "Теперь используйте этот ключ для шифрования" << COLOR_RESET << "\n";
    }
    
    waitForEnter();
}

ErrorCode viewEncryptedFile() {
    string filename;
    cout << "Введите имя зашифрованного файла: ";
    getline(cin, filename);
    
    if (filename.empty()) {
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    ifstream file(filename, ios::binary);
    if (!file) {
        return ErrorCode::ERR_FILE_NOT_FOUND;
    }
    
    file.seekg(0, ios::end);
    if (file.tellg() == 0) {
        file.close();
        return ErrorCode::ERR_FILE_EMPTY;
    }
    file.seekg(0, ios::beg);
    
    size_t msize;
    file.read(reinterpret_cast<char*>(&msize), sizeof(msize));
    
    if (!file || msize > 1024 || msize == 0) {
        cout << COLOR_YELLOW << "Это не зашифрованный файл программы" << COLOR_RESET << "\n";
        cout << "Файл не содержит корректных метаданных\n";
        file.close();
        return ErrorCode::ERR_FILE_CORRUPTED;
    }
    
    string meta(msize, '\0');
    file.read(&meta[0], msize);
    
    if (!file) {
        file.close();
        return ErrorCode::ERR_FILE_CORRUPTED;
    }
    
    cout << "\nМетаданные (IV): ";
    printHex(meta);
    cout << "\n\nЗашифрованные данные (первые 256 байт):\n";
    
    vector<unsigned char> data(256, 0);
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
    return ErrorCode::SUCCESS;
}

void changeKey(string& key) {
    clearScreen();
    
    cout << "\n" << COLOR_CYAN << "=== СМЕНА КЛЮЧА ШИФРОВАНИЯ ===" << COLOR_RESET << "\n";
    cout << "Текущий ключ: " << (key.empty() ? "(пустой)" : key) << "\n\n";
    
    cout << COLOR_YELLOW << "ВНИМАНИЕ: Все файлы, зашифрованные старым ключом," << COLOR_RESET << "\n";
    cout << COLOR_YELLOW << "   нельзя будет расшифровать новым ключом!" << COLOR_RESET << "\n\n";
    
    cout << "Введите новый ключ: ";
    string newKey;
    getline(cin, newKey);
    
    key = newKey;
    cout << "\n" << COLOR_GREEN << "Ключ шифрования успешно изменён!" << COLOR_RESET << "\n";
    cout << COLOR_GREEN << "Новый ключ: " << (key.empty() ? "(пустой)" : key) << COLOR_RESET << "\n";
    Logger::log("Encryption key changed");
}