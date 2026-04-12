#include "Renderer.h"
#include <windows.h> // Подключаем системную библиотеку

// Вспомогательная функция, чтобы не писать GetStdHandle каждый раз
void Renderer::setCursor(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, coord);
}

void Renderer::setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Пример функции, которая "прячет" мигающую палочку
void Renderer::showCursor(bool show) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = show; // true - показать, false - скрыть
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}