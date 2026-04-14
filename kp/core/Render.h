#pragma once
#include <string>

// отрисовка рамки из обычных палочек
void drawBox(int x, int y, int w, int h, int color = 7);
// отрисовка двойной рамки чтобы было красивее
void drawDoubleBox(int x, int y, int w, int h, int color = 7);
// очистка экрана консоли
void clearScreen();
// установка курсора в нужные координаты x y
void setCursor(int x, int y);
// смена цвета текста и фона
void setColor(int color);
// текст внутри рамки
void drawTextBox(int x, int y, int w, int h, std::string text, int textColor, int boxColor, int padding);
// старая функция ввода текста
std::string inputField(int x, int y, int width, bool isPassword = false, int warningY = 0);
// новая функция для ввода текста с проверками
std::string processInput(int x, int y, int width, std::string currentInput, bool isPassword, int& exitKey, int warningY = 0);

// доп функции для менюшек
// отрисовка подсказок внизу экрана
void drawFooter(int y, bool hasBack = true);
// отрисовка содержимого поля ввода
void drawInputContent(int x, int y, int width, std::string input, bool isPassword, bool isActive);
// показ заглушки если раздел не готов
void showPlaceholder(const std::string& title);
// окно для вопроса да или нет
bool showConfirmation(const std::string& message);