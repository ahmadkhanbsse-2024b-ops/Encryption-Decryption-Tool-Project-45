#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>  // Added for stringstream
#include <windows.h>
using namespace std;

// ================ UTILITY FUNCTIONS ================
bool fileExists(const string& filename) {
    WIN32_FIND_DATAA findData;
    HANDLE handle = FindFirstFileA(filename.c_str(), &findData);
    if (handle == INVALID_HANDLE_VALUE) return false;
    FindClose(handle);
    return true;
}

// ================ CIPHER FUNCTIONS ================
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
    
    // Clean key
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
    // Parse 2x2 matrix
    int a, b, c, d;
    stringstream ss(keyStr);  // Fixed: removed extra parenthesis
    
    // Use default matrix if parsing fails
    if (!(ss >> a >> b >> c >> d)) {
        a = 3; b = 3; c = 2; d = 5; // Default matrix
    }
    
    // Calculate determinant
    int det = (a * d - b * c) % 26;
    if (det < 0) det += 26;
    
    ifstream in(inFile, ios::binary);
    ofstream out(outFile, ios::binary);
    
    if (!in || !out) {
        cout << "Error opening files!\n";
        return;
    }
    
    string content;
    char ch;  // Fixed: was "class ch"
    
    // Read entire file
    while (in.get(ch)) {
        content += ch;
    }
    
    // Process alphabetic characters
    string result;
    vector<int> letters;
    
    for (char current : content) {
        if (isalpha(current)) {
            letters.push_back(toupper(current) - 'A');
            
            // Process when we have 2 letters
            if (letters.size() == 2) {
                int x = letters[0];
                int y = letters[1];
                int rx, ry;
                
                if (encrypt) {
                    // Encrypt: multiply by matrix
                    rx = (a * x + b * y) % 26;
                    ry = (c * x + d * y) % 26;
                } else {
                    // Decrypt: need inverse matrix
                    // Find modular inverse of determinant
                    int detInv = -1;
                    for (int i = 1; i < 26; i++) {
                        if ((det * i) % 26 == 1) {
                            detInv = i;
                            break;
                        }
                    }
                    
                    // If no inverse, use default
                    if (detInv == -1) {
                        cout << "Matrix not invertible! Using default.\n";
                        a = 3; b = 3; c = 2; d = 5;
                        det = 1;
                        detInv = 1;
                    }
                    
                    // Calculate inverse matrix
                    int ia = (d * detInv) % 26;
                    int ib = (-b * detInv) % 26;
                    int ic = (-c * detInv) % 26;
                    int id = (a * detInv) % 26;
                    
                    // Ensure positive values
                    if (ib < 0) ib += 26;
                    if (ic < 0) ic += 26;
                    
                    rx = (ia * x + ib * y) % 26;
                    ry = (ic * x + id * y) % 26;
                }
                
                // Convert back to letters
                char char1 = char(rx + 'A');
                char char2 = char(ry + 'A');
                
                // Add to result
                result += char1;
                result += char2;
                
                letters.clear();
            }
        } else {
            // If we have pending letters, pad with 'X'
            if (!letters.empty()) {
                letters.push_back('X' - 'A');
                int x = letters[0];
                int y = letters[1];
                int rx, ry;
                
                if (encrypt) {
                    rx = (a * x + b * y) % 26;
                    ry = (c * x + d * y) % 26;
                } else {
                    // Same decryption logic as above
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
            result += current;
        }
    }
    
    // Write result to output file
    out << result;
}

// ================ MAIN PROGRAM ================
int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    cout << "FILE ENCRYPTION/DECRYPTION TOOL\n";
    cout << "================================\n\n";
    
    // Get operation
    int op;
    cout << "1. Encrypt\n2. Decrypt\nChoose (1/2): ";
    while (!(cin >> op) || (op != 1 && op != 2)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid! Enter 1 or 2: ";
    }
    cin.ignore();
    
    // Get cipher type
    int cipher;
    cout << "\n1. Caesar\n2. Vigenere\n3. Hill (2x2)\nChoose cipher (1-3): ";
    while (!(cin >> cipher) || cipher < 1 || cipher > 3) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid! Enter 1-3: ";
    }
    cin.ignore();
    
    // Get file paths
    string inFile, outFile;
    cout << "\nInput file path: ";
    getline(cin, inFile);
    
    // Fix path for Windows
    replace(inFile.begin(), inFile.end(), '/', '\\');
    if (!fileExists(inFile)) {
        cout << "File not found!\n";
        return 1;
    }
    
    cout << "Output file path: ";
    getline(cin, outFile);
    replace(outFile.begin(), outFile.end(), '/', '\\');
    
    // Check if output exists
    if (fileExists(outFile)) {
        cout << "File exists! Overwrite? (y/n): ";
        char choice;
        cin >> choice;
        if (tolower(choice) != 'y') return 1;
        cin.ignore();
    }
    
    // Process based on cipher
    bool encrypt = (op == 1);
    
    try {
        switch (cipher) {
            case 1: { // Caesar
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
            case 2: { // Vigenere
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
