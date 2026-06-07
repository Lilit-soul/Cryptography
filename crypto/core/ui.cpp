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
    cout << "\n    === Программа шифрования ===     \n";
    cout << "┌─────────────────────────────────────┐\n";
    cout << "│ 1. Выбрать шифр                     │\n";
    cout << "│ 2. Шифрование + расшифровка текста  │\n";
    cout << "│ 3. Зашифровать файл                 │\n";
    cout << "│ 4. Расшифровать файл                │\n";
    cout << "│ 5. Список шифров                    │\n";
    cout << "│ 6. Просмотреть зашифрованный файл   │\n";
    cout << "│ 0. Выход                            │\n";
    cout << "└─────────────────────────────────────┘\n";
    cout << "Выбор: ";
}

void waitForEnter() {
    cout << "\nНажмите Enter для продолжения...";
    cin.get();
}

void selectCipher(Mgr& mgr) {
    auto list = mgr.list();
    if (list.empty()) {
        cout << "Ошибка: нет доступных шифров\n";
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
        cout << "Ошибка: название не введено\n";
        waitForEnter();
        return;
    }
    
    if (mgr.select(inp))
        cout << "Шифр выбран\n";
    else
        cout << "Шифр не найден\n";
    
    waitForEnter();
}

void testCipher(Mgr& mgr, const string& key) {
    
    if (mgr.list().empty()) {
        cout << "Ошибка: нет доступных шифров\n";
        waitForEnter();
        return;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        cout << "Ошибка: сначала выберите шифр (пункт 1)\n";
        waitForEnter();
        return;
    }
    
    cout << "Введите текст: ";
    string txt;
    getline(cin, txt);
    
    if (txt.empty()) {
        cout << "Ошибка: текст не введён\n";
        waitForEnter();
        return;
    }
    
    // Шифруем
    string encrypted = mgr.encText(txt, key);
    
    if (encrypted.empty()) {
        cout << "Ошибка: не удалось зашифровать текст\n";
        waitForEnter();
        return;
    }
    
    cout << "\nЗашифровано (hex): ";
    printHex(encrypted);
    cout << endl;
    
    // Расшифровываем
    string decrypted = mgr.decText(encrypted, key);
    
    if (decrypted.empty()) {
        cout << "\nОшибка: не удалось расшифровать текст\n";
        waitForEnter();
        return;
    }
    
    cout << "\n\nРасшифровано: " << decrypted;
    
    if (txt == decrypted) {
        cout << "\nОтлично, всё совпало!\n";
    } else {
        cout << "\nОШИБКА: исходный и расшифрованный текст не совпадают!\n";
        cout << "Возможные причины: проблема с шифром или передачей IV\n";
    }
    
    waitForEnter();
}

void encryptFile(Mgr& mgr, const string& key) {

    if (mgr.list().empty()) {
        cout << "Ошибка: нет доступных шифров\n";
        waitForEnter();
        return;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        cout << "Ошибка: сначала выберите шифр (пункт 1)\n";
        waitForEnter();
        return;
    }
    
    string in, out;
    cout << "Входной файл: ";
    getline(cin, in);
    
    if (in.empty()) {
        cout << "Ошибка: имя файла не введено\n";
        waitForEnter();
        return;
    }
    
    cout << "Выходной файл (рекомендуется .enc): ";
    getline(cin, out);
    
    if (out.empty()) {
        cout << "Ошибка: имя выходного файла не введено\n";
        waitForEnter();
        return;
    }
    
    mgr.encFile(in, out, key);
    waitForEnter();
}

void decryptFile(Mgr& mgr, const string& key) {
    
    if (mgr.list().empty()) {
        cout << "Ошибка: нет доступных шифров\n";
        waitForEnter();
        return;
    }
    
    if (mgr.info() == "Шифр не выбран") {
        cout << "Ошибка: сначала выберите шифр (пункт 1)\n";
        waitForEnter();
        return;
    }
    
    string in, out;
    cout << "Входной файл: ";
    getline(cin, in);
    
    if (in.empty()) {
        cout << "Ошибка: имя файла не введено\n";
        waitForEnter();
        return;
    }
    
    cout << "Выходной файл: ";
    getline(cin, out);
    
    if (out.empty()) {
        cout << "Ошибка: имя выходного файла не введено\n";
        waitForEnter();
        return;
    }
    
    mgr.decFile(in, out, key);
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
        cout << "Ошибка: имя файла не введено\n";
        waitForEnter();
        return;
    }
    
    ifstream file(filename, ios::binary);
    if (!file) {
        cout << "Ошибка: файл '" << filename << "' не найден\n";
        cout << "Проверьте правильность имени и пути к файлу\n";
        waitForEnter();
        return;
    }
    
    // Проверка на пустой файл
    file.seekg(0, ios::end);
    if (file.tellg() == 0) {
        cout << "Ошибка: файл пуст\n";
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
        cout << "Ошибка: файл повреждён (не удалось прочитать метаданные)\n";
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
