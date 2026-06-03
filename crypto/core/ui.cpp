#include "ui.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <vector>

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
    cout << "\n=== Программа шифрования ===\n";
    cout << "1. Выбрать шифр\n";
    cout << "2. Зашифровать текст\n";
    cout << "3. Расшифровать текст\n";
    cout << "4. Зашифровать файл\n";
    cout << "5. Расшифровать файл\n";
    cout << "6. Список шифров\n";
    cout << "7. Просмотреть зашифрованный файл \n";
    cout << "0. Выход\n";
    cout << "Выбор: ";
}

void waitForEnter() {
    cout << "\nНажмите Enter для продолжения...";
    cin.get();
}

void selectCipher(Mgr& mgr) {
    auto list = mgr.list();
    if (list.empty()) {
        cout << "Нет доступных шифров\n";
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
    
    if (mgr.select(inp))
        cout << "Шифр выбран\n";
    else
        cout << "Шифр не найден\n";
    
    waitForEnter();
}

void encryptText(Mgr& mgr, const string& key) {
    cout << "Текст: ";
    string txt;
    getline(cin, txt);
    string enc = mgr.encText(txt, key);
    cout << "Зашифровано (hex): ";
    printHex(enc);
    cout << "\n";
    waitForEnter();
}

void decryptText(Mgr& mgr, const string& key) {
    cout << "Зашифрованный текст (hex): ";
    string hex;
    getline(cin, hex);
    
    string enc;
    for (size_t i = 0; i < hex.length(); i += 2) {
        if (i+1 < hex.length()) {
            string byte = hex.substr(i, 2);
            enc += (char)strtol(byte.c_str(), nullptr, 16);
        }
    }
    
    string dec = mgr.decText(enc, key);
    cout << "Расшифровано: " << dec << "\n";
    waitForEnter();
}

void encryptFile(Mgr& mgr, const string& key) {
    string in, out;
    cout << "Входной файл: ";
    getline(cin, in);
    cout << "Выходной файл (рекомендуется .enc): ";
    getline(cin, out);
    
    if (mgr.encFile(in, out, key))
        cout << "Файл зашифрован\n";
    else
        cout << "Ошибка\n";
    
    waitForEnter();
}

void decryptFile(Mgr& mgr, const string& key) {
    string in, out;
    cout << "Входной файл: ";
    getline(cin, in);
    cout << "Выходной файл: ";
    getline(cin, out);
    
    if (mgr.decFile(in, out, key))
        cout << "Файл расшифрован\n";
    else
        cout << "Ошибка\n";
    
    waitForEnter();
}

void listCiphers(Mgr& mgr) {
    auto list = mgr.list();
    cout << "\nШифры (" << list.size() << "):\n";
    for (const auto& c : list)
        cout << "  - " << c << "\n";
    waitForEnter();
}

void viewEncryptedFile() {
    string filename;
    cout << "Введите имя зашифрованного файла: ";
    getline(cin, filename);
    
    ifstream file(filename, ios::binary);
    if (!file) {
        cout << "Ошибка: файл не найден\n";
        waitForEnter();
        return;
    }
    
    size_t msize;
    file.read(reinterpret_cast<char*>(&msize), sizeof(msize));
    
    if (!file || msize > 1024) {
        cout << "Это не зашифрованный файл программы\n";
        cout << "Просмотр невозможен\n";
        file.close();
        waitForEnter();
        return;
    }
    
    string meta(msize, '\0');
    file.read(&meta[0], msize);
    
    if (!file) {
        cout << "Файл поврежден\n";
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
    
    file.seekg(0, ios::end);
    long long fileSize = static_cast<long long>(file.tellg());
    long long dataSize = fileSize - static_cast<long long>(sizeof(msize)) - static_cast<long long>(msize);
    
    cout << "\nРазмер данных: " << dataSize << " байт\n";
    cout << "Размер IV: " << msize << " байт\n";
    
    file.close();
    waitForEnter();
}
