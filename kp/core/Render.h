#pragma once
#include <string>

void drawBox(int x, int y, int w, int h, int color = 7);
void drawDoubleBox(int x, int y, int w, int h, int color = 7);
void clearScreen();
void setCursor(int x, int y);
void setColor(int color);
void drawTextBox(int x, int y, int w, int h, std::string text, int textColor, int boxColor, int padding);
std::string inputField(int x, int y, int width, bool isPassword = false);
std::string processInput(int x, int y, int width, std::string currentInput, bool isPassword, int& exitKey);

// Новые функции
void drawInputContent(int x, int y, int width, std::string input, bool isPassword, bool isActive);