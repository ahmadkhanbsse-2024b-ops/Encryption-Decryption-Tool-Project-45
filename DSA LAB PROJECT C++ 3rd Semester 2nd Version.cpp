#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>  
#include <windows.h>
using namespace std;
bool fileExists(const string& filename) {
    WIN32_FIND_DATAA findData;
    HANDLE handle = FindFirstFileA(filename.c_str(), &findData);
    if (handle == INVALID_HANDLE_VALUE) return false;
    FindClose(handle);
    return true;
}
char caesarShift(char c, int shift, bool encrypt) {
    if (!isalpha(c)) return c;
    char base = isupper(c) ? 'A' : 'a';
    int offset = encrypt ? shift : (26 - shift) % 26;   
    return char((c - base + offset + 26) % 26 + base);
}
void caesarProcess(const string& inFile, const string& outFile, int shift, bool encrypt) {
    ifstream in(inFile, ios::binary);
    ofstream out(outFile, ios::binary);
    if (!in || !out) {
        cout << "Error opening files!\n";
        return;
    }
    char c;
    while (in.get(c)) {
        out.put(caesarShift(c, shift, encrypt));
    }
}
void vigenereProcess(const string& inFile, const string& outFile, string key, bool encrypt) {
    ifstream in(inFile, ios::binary);
    ofstream out(outFile, ios::binary);   
    if (!in || !out) {
        cout << "Error opening files!\n";
        return;
    }
    key.erase(remove_if(key.begin(), key.end(), [](char c) { return !isalpha(c); }), key.end());
    if (key.empty()) key = "KEY";
    char c;
    int keyIndex = 0;
    while (in.get(c)) {
        if (isalpha(c)) {
            char keyChar = toupper(key[keyIndex % key.length()]);
            char base = isupper(c) ? 'A' : 'a';
            int offset = keyChar - 'A';
            
            if (encrypt) {
                out.put(char((c - base + offset) % 26 + base));
            } else {
                out.put(char((c - base - offset + 26) % 26 + base));
            }
            keyIndex++;
        } else {
            out.put(c);
        }
    }
}
void hillProcess(const string& inFile, const string& outFile, const string& keyStr, bool encrypt) {
    int a, b, c, d;
    stringstream ss(keyStr);  
    if (!(ss >> a >> b >> c >> d)) {
        a = 3; b = 3; c = 2; d = 5; 
    }
    int det = (a * d - b * c) % 26;
    if (det < 0) det += 26;
    ifstream in(inFile, ios::binary);
    ofstream out(outFile, ios::binary);
    
    if (!in || !out) {
        cout << "Error opening files!\n";
        return;
    }
    string content;
    char ch;  
    while (in.get(ch)) {
        content += ch;
    }
    string result;
    vector<int> letters;
    for (char current : content) {
        if (isalpha(current)) {
            letters.push_back(toupper(current) - 'A');
            if (letters.size() == 2) {
                int x = letters[0];
                int y = letters[1];
                int rx, ry;
                if (encrypt) {
                    rx = (a * x + b * y) % 26;
                    ry = (c * x + d * y) % 26;
                } else {
                    int detInv = -1;
                    for (int i = 1; i < 26; i++) {
                        if ((det * i) % 26 == 1) {
                            detInv = i;
                            break;
                        }
                    }
                    if (detInv == -1) {
                        cout << "Matrix not invertible! Using default.\n";
                        a = 3; b = 3; c = 2; d = 5;
                        det = 1;
                        detInv = 1;
                    }
                    int ia = (d * detInv) % 26;
                    int ib = (-b * detInv) % 26;
                    int ic = (-c * detInv) % 26;
                    int id = (a * detInv) % 26;
                    if (ib < 0) ib += 26;
                    if (ic < 0) ic += 26;
                    
                    rx = (ia * x + ib * y) % 26;
                    ry = (ic * x + id * y) % 26;
                }
                char char1 = char(rx + 'A');
                char char2 = char(ry + 'A');
                result += char1;
                result += char2;
                
                letters.clear();
            }
        } else {
            if (!letters.empty()) {
                letters.push_back('X' - 'A');
                int x = letters[0];
                int y = letters[1];
                int rx, ry;
                if (encrypt) {
                    rx = (a * x + b * y) % 26;
                    ry = (c * x + d * y) % 26;
                } else {
                    int detInv = -1;
                    for (int i = 1; i < 26; i++) {
                        if ((det * i) % 26 == 1) detInv = i;
                    }
                    if (detInv == -1) detInv = 1;
                    int ia = (d * detInv) % 26;
                    int ib = (-b * detInv) % 26;
                    int ic = (-c * detInv) % 26;
                    int id = (a * detInv) % 26;
                    if (ib < 0) ib += 26;
                    if (ic < 0) ic += 26;
                    
                    rx = (ia * x + ib * y) % 26;
                    ry = (ic * x + id * y) % 26;
                }
                result += char(rx + 'A');
                result += char(ry + 'A');
                letters.clear();
            }
            result += current;}}
    out << result;
}
int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    cout << "FILE ENCRYPTION/DECRYPTION TOOL\n";
    cout << "================================\n\n";
    int op;
    cout << "1. Encrypt\n2. Decrypt\nChoose (1/2): ";
    while (!(cin >> op) || (op != 1 && op != 2)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid! Enter 1 or 2: ";
    }
    cin.ignore();
    int cipher;
    cout << "\n1. Caesar\n2. Vigenere\n3. Hill (2x2)\nChoose cipher (1-3): ";
    while (!(cin >> cipher) || cipher < 1 || cipher > 3) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid! Enter 1-3: ";
    }
    cin.ignore();
    string inFile, outFile;
    cout << "\nInput file path: ";
    getline(cin, inFile);
    replace(inFile.begin(), inFile.end(), '/', '\\');
    if (!fileExists(inFile)) {
        cout << "File not found!\n";
        return 1;
    }
    cout << "Output file path: ";
    getline(cin, outFile);
    replace(outFile.begin(), outFile.end(), '/', '\\');
    if (fileExists(outFile)) {
        cout << "File exists! Overwrite? (y/n): ";
        char choice;
        cin >> choice;
        if (tolower(choice) != 'y') return 1;
        cin.ignore();
    }
    bool encrypt = (op == 1);
    try {
        switch (cipher) {
            case 1: { 
                int shift;
                cout << "Shift amount (1-25): ";
                cin >> shift;
                if (shift < 1 || shift > 25) {
                    cout << "Using default shift 3.\n";
                    shift = 3;
                }
                caesarProcess(inFile, outFile, shift, encrypt);
                break;
            }
            case 2: { 
                string key;
                cout << "Keyword: ";
                getline(cin, key);
                vigenereProcess(inFile, outFile, key, encrypt);
                break;
            }
            case 3: { // Hill
                string key;
                cout << "Enter 4 numbers for 2x2 matrix (e.g., 3 3 2 5): ";
                getline(cin, key);
                hillProcess(inFile, outFile, key, encrypt);
                break;
            }
        }
        cout << "\n? Operation successful!\n";
        cout << "Input: " << inFile << "\n";
        cout << "Output: " << outFile << "\n";
    } catch (...) {
        cout << "Error processing file!\n";
        return 1;
    }
    cout << "\nPress Enter to exit...";
    cin.ignore();
    cin.get();
    return 0;
}
