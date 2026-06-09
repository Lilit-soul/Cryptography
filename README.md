# FlexCipher

Универсальная программа для шифрования файлов и текста с поддержкой различных криптографических алгоритмов. Программа позволяет динамически подключать новые шифры без изменения основного кода.

## Возможности

- Авторизация с мастер-паролем (первый запуск)

- Шифрование и расшифровка текста

- Шифрование и расшифровка файлов

- Автоматическое определение типа файла при расшифровке

- Поддержка различных типов шифров:

- Симметричные (ChaCha20, Rabbit, Caesar, Shamir)

-  Асимметричные (ElGamal)

- Протоколы обмена ключами (Diffie-Hellman)

- Логирование всех действий

- Цветное отображение в терминале

- Динамическая загрузка шифров



## Структура проекта
``` 
crypto/
├── core/                  # Основные модули
│   ├── mgr.h/cpp         # Менеджер шифров
│   ├── ui.h/cpp          # Пользовательский интерфейс
│   ├── auth.h            # Авторизация
│   ├── errors.h/cpp      # Обработка ошибок
│   ├── logger.h          # Логирование
│   └── menu_actions.h    # Действия меню
├── include/               # Заголовочные файлы
│   ├── interface.h       # Интерфейс шифров
│   └── loader.h          # Динамическая загрузка
├── ciphers/               # Реализации шифров
│   ├── chacha20.cpp
│   ├── rabbit.cpp
│   └── ...
├── main.cpp              # Точка входа
├── Makefile              # Сборка проекта
└── README.md             # Документация
``` 

# Установка и компиляция

### Установка

git clone git@github.com:Lilit-soul/Cryptography.git

cd crypto

### Компиляция

make

make run - запуск программы

make clean  - очистка объектных файлов (если нужно заново скомпилировать проект)

# Просьба сначала скидывать изменения на свою ветку !!!


# Как добавить шифр

### Шаг 1: Создайте файл ciphers/название.cpp:
``` 
#include "../include/interface.h"
#include "../include/loader.h"

using namespace std;

class MyCipher : public Cipher {
public:
    EncData encrypt(const vector<uint8_t>& data, const string& key) override {
        EncData result;

        // Ваша реализация шифрования
        
        return result;
    }
    
    vector<uint8_t> decrypt(const EncData& data, const string& key) override {
        
        // Ваша реализация расшифровки
        
        return {};
    }
    
    string name() const override { return "mycipher"; }
    int keySize() const override { return 16; }
    int nonceSize() const override { return 8; }
    Type getType() const override { return Type::SYMMETRIC; }
};

REG_CIPHER(MyCipher, "mycipher")
``` 

### Шаг 2: Добавьте файл в Makefile

` SRCS = ... ciphers/mycipher.cpp `

### Шаг 3: Пересоберите программу
```
make clean
make 
``` 
### Запуск:

| Способ | Команда  | Когда использовать |
|--------|----------|-------------------|
| Прямой запуск | `./encrypt_fw`  | После сборки |
| Через Make | `make run`  | Удобно, не нужно помнить имя |
| С отладкой | `gdb ./encrypt_fw`  | Поиск ошибок |
| Ручная компиляция | `g++ ...`  | Без Makefile |
 

## Обработка ошибок
Программа использует гибридный подход к обработке ошибок:

+ ErrorCode - для ожидаемых ошибок (файл не найден, неверный ключ)

+ Try-catch - для неожиданных ошибок (повреждение памяти, исключения шифров)

Все ошибки логируются в файл _encrypt.log._
