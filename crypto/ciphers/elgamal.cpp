#include "../include/interface.h"
#include "../include/loader.h"
#include <vector>
#include <cstring>
#include <random>
#include <chrono>
 
using namespace std;
 
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
if (mod == 0) return 0;
uint64_t result = 1;
base %= mod;
while (exp > 0) {
if (exp & 1) result = (result * base) % mod;
base = (base * base) % mod;
exp >>= 1;
}
return result;
}
 
class ElGamalCipher : public Cipher {
private:
uint64_t p = 257;
uint64_t g = 3;
mt19937_64 rng;
 
uint64_t randomKey() {
uniform_int_distribution<uint64_t> dis(2, p - 2);
return dis(rng);
}
 
public:
ElGamalCipher() : rng(chrono::steady_clock::now().time_since_epoch().count()) {}
 
string name() const override { return "el_gamal"; }
int keySize() const override { return 32; }
int nonceSize() const override { return 8; }
Type getType() const override { return Type::ASYMMETRIC; }
 
EncData encrypt(const vector<uint8_t>& data, const string& key) override {
EncData result;
 
// При ошибке возвращаем ПУСТОЙ результат
if (data.empty()) {
return result;
}
 
if (key.empty() || key.length() < 4) {
return result;
}
 
try {
// Генерация ключей
uint64_t priv_key = 123;
for (char c : key) {
priv_key = (priv_key * 31 + static_cast<uint64_t>(c)) % (p - 2);
}
if (priv_key < 2) priv_key = 123;
 
uint64_t pub_key = mod_pow(g, priv_key, p);
 
if (pub_key == 0 || pub_key >= p) {
return result;
}
 
// Сохраняем открытый ключ в meta
result.meta.resize(sizeof(pub_key));
memcpy(result.meta.data(), &pub_key, sizeof(pub_key));
 
// Шифруем каждый байт в пару (a,b) → 16 байт
result.data.clear(); // ← result.data, не encryptedData!
result.data.reserve(data.size() * 16);
 
for (uint8_t m : data) {
uint64_t k = randomKey();
uint64_t a = mod_pow(g, k, p);
uint64_t yk = mod_pow(pub_key, k, p);
uint64_t b = (static_cast<uint64_t>(m) * yk) % p;
 
if (a == 0 || b == 0) {
result.data.clear();
result.meta.clear();
return result;
}
 
result.data.insert(result.data.end(), (uint8_t*)&a, (uint8_t*)&a + 8);
result.data.insert(result.data.end(), (uint8_t*)&b, (uint8_t*)&b + 8);
}
 
} catch (...) {
result.data.clear();
result.meta.clear();
}
 
return result;
}
 
vector<uint8_t> decrypt(const EncData& data, const string& key) override {
vector<uint8_t> result;
 
// При ошибке возвращаем ПУСТОЙ вектор
if (data.data.empty()) { // ← data.data, не encryptedData!
return result;
}
 
if (key.empty() || key.length() < 4) {
return result;
}
 
if (data.meta.size() < sizeof(uint64_t)) {
return result;
}
 
if (data.data.size() % 16 != 0) { // ← data.data
return result;
}
 
try {
uint64_t expected_pub;
memcpy(&expected_pub, data.meta.data(), sizeof(expected_pub));
 
if (expected_pub >= p || expected_pub == 0) {
return result;
}
 
uint64_t priv_key = 123;
for (char c : key) {
priv_key = (priv_key * 31 + static_cast<uint64_t>(c)) % (p - 2);
}
if (priv_key < 2) priv_key = 123;
 
uint64_t computed_pub = mod_pow(g, priv_key, p);
 
// Неверный пароль
if (computed_pub != expected_pub) {
return result;
}
 
size_t blocks = data.data.size() / 16; // ← data.data
result.reserve(blocks);
 
for (size_t i = 0; i < blocks; i++) {
uint64_t a, b;
memcpy(&a, &data.data[i * 16], 8); // ← data.data
memcpy(&b, &data.data[i * 16 + 8], 8); // ← data.data
 
if (a >= p || b >= p || a == 0) {
result.clear();
return result;
}
 
uint64_t a_inv = mod_pow(a, p - 1 - priv_key, p);
uint64_t m = (b * a_inv) % p;
result.push_back(static_cast<uint8_t>(m));
}
 
} catch (...) {
result.clear();
}
 
return result;
}
};
REG_CIPHER(ElGamalCipher, "el_gamal")
