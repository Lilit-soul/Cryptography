#ifndef UI_H
#define UI_H

#include "mgr.h"

using namespace std;

void clearScreen();
void printHex(const string& s);
void menu();
void waitForEnter();
void selectCipher(Mgr& mgr);
void encryptText(Mgr& mgr, const string& key);
void decryptText(Mgr& mgr, const string& key);
void testCipher(Mgr& mgr, const std::string& key);
void encryptFile(Mgr& mgr, const string& key);
void decryptFile(Mgr& mgr, const string& key);
void listCiphers(Mgr& mgr);
void viewEncryptedFile();

#endif
