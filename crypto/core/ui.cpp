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

void ensureDataDir() {
    mkdir("data", 0755);
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
                cout << COLOR_YELLOW << "Шифр выбран, но может работать некорректно!" << COLOR_RESET << "\n";
            } else {
                cout << COLOR_GREEN << "Шифр выбран!" << COLOR_RESET << "\n";
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
    
    // Проверяем существование папки data
    string dataDir = "data";
    mkdir(dataDir.c_str(), 0755);
    
    // Собираем список файлов из папки data (только файлы, не директории)
    cout << "\n" << COLOR_CYAN << "Доступные файлы:" << COLOR_RESET << "\n";
    
    vector<string> files;
    string dataPath = dataDir + "/";
    
    DIR* dir = opendir(dataPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            string name = entry->d_name;
            if (name != "." && name != "..") {
                string fullEntryPath = dataPath + name;
                struct stat st;
                // Проверяем, что это не директория
                if (stat(fullEntryPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                    continue;  // пропускаем директории
                }
                // Пропускаем уже зашифрованные файлы
                if (name.length() > 4 && name.substr(name.length() - 4) == ".enc") {
                    continue;
                }
                files.push_back(name);
            }
        }
        closedir(dir);
    }
    
    if (files.empty()) {
        cout << "   (нет файлов)\n";
        cout << "Поместите файлы в папку 'data/'\n";
        waitForEnter();
        return ErrorCode::ERR_FILE_NOT_FOUND;
    }
    
    // Выводим файлы с номерами
    for (size_t i = 0; i < files.size(); i++) {
        cout << "   " << i + 1 << ". " << files[i] << "\n";
    }
    
    cout << "\n" << COLOR_YELLOW << "Выберите номер: " << COLOR_RESET;
    string choiceStr;
    getline(cin, choiceStr);
    
    if (choiceStr.empty()) {
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    int choice;
    try {
        choice = stoi(choiceStr);
    } catch (...) {
        cout << COLOR_RED << "Ошибка: введите число!" << COLOR_RESET << "\n";
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    if (choice < 1 || choice > (int)files.size()) {
        cout << COLOR_RED << "Ошибка: номер от 1 до " << files.size() << COLOR_RESET << "\n";
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    string filename = files[choice - 1];
    string fullPath = dataPath + filename;
    
    cout << "\nВыбран: " << COLOR_CYAN << filename << COLOR_RESET << "\n";
    
    // === Читаем исходный файл ===
    ifstream inFile(fullPath, ios::binary);
    vector<uint8_t> originalData((istreambuf_iterator<char>(inFile)), 
                                   istreambuf_iterator<char>());
    inFile.close();
    
    // Сохраняем оригинальное расширение
    string originalExt;
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != string::npos) {
        originalExt = filename.substr(dotPos);
    }
    
    cout << "\nИсходный файл: " << COLOR_CYAN << filename << COLOR_RESET 
         << " (расширение: " << originalExt << ")\n";
    
    // Шифруем
    string encData;
    if (!mgr.encTextSafe(string(originalData.begin(), originalData.end()), key, encData)) {
        return ErrorCode::ERR_INVALID_KEY;
    }    
    
    // Сохраняем зашифрованный файл
    string encFilename;
    if (dotPos != string::npos) {
        encFilename = dataPath + filename.substr(0, dotPos) + ".enc";
    } else {
        encFilename = dataPath + filename + ".enc";
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
        cout << COLOR_RED << "Ошибка: не удалось расшифровать" << COLOR_RESET << "\n";
        return ErrorCode::ERR_DECRYPT_FAIL;
    }
    
    // Проверяем, не существует ли уже файл decr_
    string decFilename = dataPath + "decr_" + filename;
    ifstream existing(decFilename);
    if (existing) {
        cout << COLOR_YELLOW << "Файл decr_" << filename << " уже существует." << COLOR_RESET << "\n";
        cout << "Перезаписать? (y/N): ";
        string answer;
        getline(cin, answer);
        if (answer != "y" && answer != "Y") {
            cout << COLOR_YELLOW << "Операция отменена." << COLOR_RESET << "\n";
            return ErrorCode::ERR_USER_CANCEL;
        }
    }
    existing.close();
    
    // Сохраняем расшифрованный файл
    ofstream decFile(decFilename, ios::binary);
    if (!decFile) {
        return ErrorCode::ERR_CANNOT_CREATE;
    }
    decFile.write(decrypted.c_str(), decrypted.size());
    decFile.close();
    cout << COLOR_GREEN << "Расшифровано (проверка): " << COLOR_RESET << "decr_" << filename << "\n";
    
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
        Logger::log("File processed: " + filename);
        return ErrorCode::SUCCESS;
    } else {
        cout << "\n" << COLOR_RED << "ОШИБКА: Расшифрованные данные не совпадают с оригиналом!" << COLOR_RESET << "\n";
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


ErrorCode viewEncryptedFile() {
    string dataDir = "data";
    string dataPath = dataDir + "/";
    
    // Собираем список .enc файлов (только файлы, не директории)
    cout << "\n" << COLOR_CYAN << "Зашифрованные файлы:" << COLOR_RESET << "\n";
    
    vector<pair<string, long long>> encFiles;
    
    DIR* dir = opendir(dataPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            string name = entry->d_name;
            if (name != "." && name != "..") {
                string fullEntryPath = dataPath + name;
                struct stat st;
                // Проверяем, что это не директория
                if (stat(fullEntryPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                    continue;  // пропускаем директории
                }
                if (name.length() > 4 && name.substr(name.length() - 4) == ".enc") {
                    long long size = st.st_size;
                    encFiles.push_back({name, size});
                }
            }
        }
        closedir(dir);
    }
    
    if (encFiles.empty()) {
        cout << "   (нет зашифрованных файлов)\n";
        waitForEnter();
        return ErrorCode::ERR_FILE_NOT_FOUND;
    }
    
    // Выводим файлы с номерами
    for (size_t i = 0; i < encFiles.size(); i++) {
        cout << "   " << i + 1 << ". " << encFiles[i].first 
             << " (" << encFiles[i].second << " байт)\n";
    }
    
    cout << "\n" << COLOR_YELLOW << "Выберите номер: " << COLOR_RESET;
    string choiceStr;
    getline(cin, choiceStr);
    
    if (choiceStr.empty()) {
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    int choice;
    try {
        choice = stoi(choiceStr);
    } catch (...) {
        cout << COLOR_RED << "Ошибка: введите число!" << COLOR_RESET << "\n";
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    if (choice < 1 || choice > (int)encFiles.size()) {
        cout << COLOR_RED << "Ошибка: номер от 1 до " << encFiles.size() << COLOR_RESET << "\n";
        return ErrorCode::ERR_INVALID_FORMAT;
    }
    
    string filename = encFiles[choice - 1].first;
    string fullPath = dataPath + filename;
    
    cout << "\nПросмотр: " << COLOR_CYAN << filename << COLOR_RESET << "\n";
    
    ifstream file(fullPath, ios::binary);
    if (!file) {
        return ErrorCode::ERR_FILE_NOT_FOUND;
    }
    
    // Проверяем минимальный размер файла
    file.seekg(0, ios::end);
    streamoff fileSize = file.tellg();
    file.seekg(0, ios::beg);
    
    if (fileSize < 5) {
        cout << COLOR_YELLOW << "Файл слишком мал для зашифрованного файла программы" << COLOR_RESET << "\n";
        file.close();
        return ErrorCode::ERR_FILE_CORRUPTED;
    }
    
    uint32_t msize;
    file.read(reinterpret_cast<char*>(&msize), sizeof(msize));
    
    if (fileSize < (streamoff)(sizeof(msize) + msize)) {
        cout << COLOR_YELLOW << "Файл повреждён: размер метаданных не соответствует файлу" << COLOR_RESET << "\n";
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