#include "Render.h"
#include <iostream>
#include <windows.h>

using namespace std;

void static setCursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsole, coord); 
}

void static setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void static clearScreen() {
    system("cls"); 
}

void static drawBox(int x, int y, int w, int h, int color) { 
    setColor(color);
    setCursor(x, y);

    cout << "+";
    for (int i = 0; i < w - 2; i++) {
        cout << "-"; 
    }
    cout << "+";

    for (int i = 1; i < h - 1; i++) {
        setCursor(x, y + i);
        cout << "|";
        setCursor(x + w - 1, y + i);
        cout << "|";
    }

    setCursor(x, y + h - 1);
    cout << "+";
    for (int i = 0; i < w - 2; i++) {
        cout << "-";
    }
    cout << "+";

    setColor(7); 
}

void static drawTextBox(int x, int y, int w, int h, string text, int textColor, int boxColor, int padding) {
    drawBox(x, y, w, h, boxColor);

    int textY = y + (h / 2);
    int textX = x + padding;

    int maxWidth = w - padding - 1;

    string textToPrint = text;
    if (textToPrint.length() > maxWidth) {
        textToPrint = textToPrint.substr(0, maxWidth - 3) + "...";
    }

    setColor(textColor); 
    setCursor(textX, textY);  
    cout << textToPrint; 

    setColor(7);
}