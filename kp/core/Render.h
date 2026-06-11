#pragma once
#include <cstddef>
#include <string>
#include <vector>

struct TableColumn {
    int x;
    int width;
    std::string title;
};

// отрисовка простой ASCII-рамки из + - |
void drawBox(int x, int y, int w, int h, int color = 7);
// очистка экрана консоли
void clearScreen();
// установка курсора в нужные координаты x y
void setCursor(int x, int y);
// смена цвета текста и фона
void setColor(int color);
// горизонтальная линия вида +-----+ : x,y — левый угол, innerWidth — число дефисов
void drawHLineAt(int x, int y, int innerWidth, int color = 7);
// форматирование суммы с двумя знаками после запятой ("123.45")
std::string formatMoney(double value);
// ASCII-логотип "MOBILE HELPER" в фиксированной позиции (x=16, строки 2..13)
void drawLogo();
// функция для ввода текста с проверками
// allowUnicode=true разрешает кириллицу (для ФИО, тарифов и т.п.);
// при false принимается только латиница (логины, пароли)
// allowedChars — если не пусто, принимаются только перечисленные символы
// (например "0123456789+" для номера телефона)
std::string processInput(int x, int y, int width, std::string currentInput, bool isPassword, int& exitKey, int warningY = 0, bool allowUnicode = false, const std::string& allowedChars = "");

// доп функции для менюшек
// отрисовка подсказок внизу экрана
void drawFooter(int y, bool hasBack = true);
// отрисовка содержимого поля ввода
void drawInputContent(int x, int y, int width, std::string input, bool isPassword, bool isActive);
void clearLine(int x, int y, int width, int color = 7);
std::string truncateText(std::string value, size_t maxLen);
void drawTableCell(int x, int y, int width, std::string value, int color = 7);
void drawTableHeader(int y, const std::vector<TableColumn>& columns, const std::vector<int>& separatorXs, int textColor = 15);
void drawTableSeparator(int x, int y, int width, const std::vector<int>& separatorXs, int color = 7);
// окно для вопроса да или нет
bool showConfirmation(const std::string& message);
