#ifndef UI_H
#define UI_H

#include <string>

#include "mgr.h"
#include "errors.h"
#include "auth.h"

int getMenuChoice();
bool runAuth(Auth& auth);

void clearScreen();
void printHex(const std::string& s);
void getEncryptionKey(std::string& key);
void showMenu();
void waitForEnter();
void changeKey(std::string& key);

// Функции для разных типов шифров
void handleAsymmetricCipher(Mgr& mgr, std::string& key);
void handleKeyExchange(Mgr& mgr, std::string& key);

// Основные функции
ErrorCode selectCipher(Mgr& mgr);
ErrorCode testCipher(Mgr& mgr, const std::string& key);
ErrorCode processFile(Mgr& mgr, const std::string& key);
ErrorCode listCiphers(Mgr& mgr);
ErrorCode viewEncryptedFile();

#endif