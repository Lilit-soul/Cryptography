#ifndef UI_H
#define UI_H

#include "mgr.h"
#include "errors.h"
#include "auth.h"

void clearScreen();
void printHex(const std::string& s);
void menu();
void waitForEnter();
void selectCipher(Mgr& mgr);
void testCipher(Mgr& mgr, const std::string& key);
void encryptFile(Mgr& mgr, const std::string& key);
void decryptFile(Mgr& mgr, const std::string& key);
void listCiphers(Mgr& mgr);
void viewEncryptedFile();


bool runAuth(Auth& auth);

#endif