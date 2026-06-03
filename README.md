# Шифр (пока нет названия программы)

Фреймворк для шифрования данных с поддержкой подключаемых шифров. Поддерживает шифрование текста и любых файлов (изображения, музыка, видео и т.д.).

## Возможности

- Шифрование/дешифрование текста
- Шифрование/дешифрование любых файлов (бинарных и текстовых)
- Подключаемые шифры в отдельных файлах
- Автоматическая регистрация новых шифров
- Просмотр зашифрованных файлов в hex-формате


## Просьба сначала скидывать изменения в свой репозиторий !!!

## Структура проекта
``` 
crypto/
├── include/
│ ├── interface.h          # Базовые классы (Cipher, EncData)
│ └── loader.h             # Реестр шифров и макрос регистрации
├── core/
│ ├── mgr.h                # Менеджер шифров (заголовок)
│ ├── mgr.cpp              # Менеджер шифров (реализация)
│ ├── ui.h                 # Пользовательский интерфейс (заголовок)
│ └── ui.cpp               # Пользовательский интерфейс (реализация)
├── ciphers/
│ ├── rabbit.cpp                    # Шифр Rabbit
│ └── chacha20.cpp                  # Шифр ChaCha20
├── main.cpp                        # Точка входа
├── Makefile                        # Сборка
├── encrypt_fw                      # Исполняемый файл (после компиляции)
└── *.txt, *.bin, *.enc, *.jpeg     # файлы (текстовые и картинки)
``` 

## Установка и компиляция

#Установка

git clone git@github.com:Lilit-soul/Cryptography.git
cd crypto

#Компиляция

make
make run - запуск программы

make clean  - очистка объектных файлов (если нужно заново скомпилировать проект)


### Как добавить шифр

#Шаг 1: Создайте файл ciphers/название.cpp:
``` 
#include "../include/interface.h"
#include "../include/loader.h"
#include <vector>
#include <string>
#include <random>

using namespace std;

class MyCipher : public Cipher {
private:
    // Внутреннее состояние шифра (если нужно)
    
    void keySetup(const string& key) {
        // Инициализация по ключу
    }
    
    void ivSetup(const string& iv) {
        // Инициализация по вектору инициализации
    }
    
    void generateKeystream(vector<uint8_t>& output, size_t length) {
        // Генерация псевдослучайной последовательности
    }
    
public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;
        
        // 1. Сгенерировать случайный IV (вектор инициализации)
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 255);
        
        string iv(8, 0);  // размер IV зависит от шифра
        for (int i = 0; i < 8; i++) {
            iv[i] = (char)dis(gen);
        }
        
        // 2. Инициализация шифра
        keySetup(key);
        ivSetup(iv);
        
        // 3. Генерация keystream
        vector<uint8_t> keystream;
        generateKeystream(keystream, data.size());
        
        // 4. XOR шифрование
        result.data.resize(data.size());
        for (size_t i = 0; i < data.size(); i++) {
            result.data[i] = data[i] ^ keystream[i];
        }
        
        // 5. Сохраняем IV в метаданные
        result.meta = iv;
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        // 1. Инициализация с тем же ключом
        keySetup(key);
        
        // 2. Используем IV из метаданных
        string iv = data.meta;
        ivSetup(iv);
        
        // 3. Генерация того же keystream
        vector<uint8_t> keystream;
        generateKeystream(keystream, data.data.size());
        
        // 4. XOR дешифрование (обратимая операция)
        vector<uint8_t> result(data.data.size());
        for (size_t i = 0; i < data.data.size(); i++) {
            result[i] = data.data[i] ^ keystream[i];
        }
        
        return result;
    }
    
    string name() const override { return "название"; }
    int keySize() const override { return 16; }   // размер ключа в байтах
    int nonceSize() const override { return 8; }  // размер IV в байтах
};

// Обязательный макрос для регистрации шифра
REG_CIPHER(MyCipher, "название")
``` 

#Шаг 2: Требования к реализации

Метод	              Описание
encrypt()	       Принимает данные и ключ, возвращает EncData (данные + IV)
decrypt()	       Принимает EncData и ключ, возвращает расшифрованные данные
name()         	 Возвращает название шифра (строкой)
keySize()	       Возвращает размер ключа в байтах
nonceSize()	     Возвращает размер IV/nonce в байтах



Шифр не появляется в списке
Проверьте:

Файл лежит в папке ciphers/
В конце файла есть REG_CIPHER(ClassName, "name")
Класс наследуется от Cipher


