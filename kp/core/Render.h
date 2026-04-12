#pragma once
#include <string>
void drawBox(int x, int y, int w, int h, int color = 7);
void clearScreen();
void setCursor(int x, int y);
void setColor(int color);
void drawTextBox(int x, int y, int w, int h, std::string text, int textColor, int boxColor, int padding);