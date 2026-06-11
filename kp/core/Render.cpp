#include "Render.h"
#include <windows.h>
#include <iomanip>
#include <iostream>
#include "InputHandler.h"

using namespace std;

// маленькое окошко для подтверждения действия да или нет
bool showConfirmation(const string& message) {
    // отрисовка небольшого окна в центре
    int w = 46;
    int h = 7;
    int x = (80 - w) / 2;
    int y = 10;

    drawBox(x, y, w, h, 14);  // простая рамка

    // закрашивание внутренней части черным чтобы окно не просвечивало
    setColor(0);
    for (int i = 1; i < h - 1; i++) {
        setCursor(x + 1, y + i);
        for (int j = 0; j < w - 2; j++) {
            cout << " ";
        }
    }

    setCursor(x + (w - (int)message.length()) / 2, y + 2);
    setColor(15);
    cout << message;

    setCursor(x + 10, y + 4);
    setColor(10);
    cout << "[Enter] Да";

    setCursor(x + w - 18, y + 4);
    setColor(12);
    cout << "[Esc] Нет";

    while (true) {
        int key = InputHandler::getExtKey();
        if (key == Key::ENTER || key == 'y' || key == 'Y') {
            return true;
        }
        if (key == Key::ESC || key == 'n' || key == 'N') {
            return false;
        }
    }
}

// отрисовка подсказок по кнопкам в нижней части экрана
void drawFooter(int y, bool hasBack) {
    setColor(8);
    setCursor(2, y);
    std::cout << "[Tab] Навигация  | [Enter] Выбрать";
    if (hasBack) {
        std::cout << "   |  [Esc] Назад";
    } else {
        std::cout << "   |  [Esc] Выход";
    }
}

// подсчёт количества символов (а не байт) в UTF-8 строке;
// кириллица в консоли занимает 1 колонку при 2 байтах
size_t utf8Length(const string& s) {
    size_t count = 0;
    for (size_t i = 0; i < s.length();) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        int step = 1;
        if ((c & 0xE0) == 0xC0) {
            step = 2;
        } else if ((c & 0xF0) == 0xE0) {
            step = 3;
        } else if ((c & 0xF8) == 0xF0) {
            step = 4;
        }
        i += step;
        count++;
    }
    return count;
}

// отрисовка текста внутри поля ввода при печати
void drawInputContent(int x, int y, int width, string input, bool isPassword, bool isActive) {
    int bgColor = isActive ? (1 * 16) : 0;
    int fgColor = isActive ? 15 : 7;
    setColor(bgColor + fgColor);

    setCursor(x, y);

    // длина в символах (колонках), а не в байтах — иначе кириллица
    // визуально занимала бы вдвое меньше места в поле
    size_t visibleLen = utf8Length(input);
    string displayString = isPassword ? string(visibleLen, '*') : input;

    if (isActive && visibleLen < (size_t)width) {
        displayString += "|";
        visibleLen++;
    }

    while (visibleLen < (size_t)width) {
        displayString += "_";
        visibleLen++;
    }

    cout << displayString;
    setColor(7);
}

// очистка одной строки без полной перерисовки экрана
void clearLine(int x, int y, int width, int color) {
    setCursor(x, y);
    setColor(color);
    cout << string(width, ' ');
    setColor(7);
}

string truncateText(string value, size_t maxLen) {
    size_t bytePos = 0;
    size_t ellipsisBytePos = 0;
    size_t charCount = 0;

    while (bytePos < value.length() && charCount < maxLen) {
        unsigned char c = static_cast<unsigned char>(value[bytePos]);
        int step = 1;
        if ((c & 0xE0) == 0xC0) {
            step = 2;
        } else if ((c & 0xF0) == 0xE0) {
            step = 3;
        } else if ((c & 0xF8) == 0xF0) {
            step = 4;
        }

        if (maxLen > 3 && charCount == maxLen - 3) {
            ellipsisBytePos = bytePos;
        }

        bytePos += step;
        charCount++;
    }

    if (bytePos >= value.length()) {
        return value;
    }
    if (maxLen <= 3) {
        return value.substr(0, bytePos);
    }
    return value.substr(0, ellipsisBytePos) + "...";
}

void drawTableCell(int x, int y, int width, string value, int color) {
    setCursor(x, y);
    setColor(color);
    cout << left << setw(width) << truncateText(value, width);
    setColor(7);
}

void drawTableHeader(int y, const vector<TableColumn>& columns, const vector<int>& separatorXs, int textColor) {
    for (int separatorX : separatorXs) {
        drawTableCell(separatorX, y, 1, "|", textColor);
    }

    for (const auto& column : columns) {
        drawTableCell(column.x, y, column.width, column.title, textColor);
    }
}

void drawTableSeparator(int x, int y, int width, const vector<int>& separatorXs, int color) {
    setCursor(x, y);
    setColor(color);
    for (int i = 0; i < width; ++i) {
        cout << "-";
    }

    for (int separatorX : separatorXs) {
        setCursor(separatorX, y);
        cout << "+";
    }

    setColor(7);
}

// перемещение невидимого курсора в координаты x y
void setCursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsole, coord);
}

// меняет цвет букв в консоли
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// полная очистка всего с экрана
void clearScreen() {
    system("cls");
}

// отрисовка обычной рамки из плюсиков и минусов
void drawBox(int x, int y, int w, int h, int color) {
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

// отрисовка рамки с текстом по центру
void drawTextBox(int x, int y, int w, int h, string text, int textColor, int boxColor, int padding) {
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

// кодирование одного символа (код из _getwch) в UTF-8; кириллица занимает 2 байта
static string utf8Encode(unsigned int cp) {
    string out;
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return out;
}

// удаление одного целого UTF-8 символа с конца (а не одного байта),
// иначе при стирании кириллицы оставался бы битый байт
static void utf8PopBack(string& s) {
    while (!s.empty() && (static_cast<unsigned char>(s.back()) & 0xC0) == 0x80) {
        s.pop_back();  // снимаем продолжающие байты
    }
    if (!s.empty()) {
        s.pop_back();  // снимаем ведущий байт
    }
}

// функция для ввода текста с проверкой раскладки
string processInput(int x, int y, int width, string currentInput, bool isPassword, int& exitKey, int warningY, bool allowUnicode) {
    string input = currentInput;
    int choose;

    drawInputContent(x, y, width, input, isPassword, true);

    auto clearWarning = [&]() {
        if (warningY > 0) {
            setCursor(x - 12, warningY);
            cout << "                                       ";
        }
    };

    while (true) {
        choose = InputHandler::getExtKey();

        if (choose == Key::ENTER || choose == Key::TAB || choose == Key::ESC || choose == Key::UP || choose == Key::DOWN) {
            exitKey = choose;
            break;
        }

        if (choose == Key::BACKSPACE) {
            if (!input.empty()) {
                utf8PopBack(input);
                drawInputContent(x, y, width, input, isPassword, true);
            }
            continue;
        }

        // символы вне ASCII (кириллица и т.п.)
        if (choose > 127) {
            if (allowUnicode) {
                // лимит считаем в символах, а не в байтах
                if (utf8Length(input) < static_cast<size_t>(width - 1)) {
                    clearWarning();
                    input += utf8Encode(static_cast<unsigned int>(choose));
                    drawInputContent(x, y, width, input, isPassword, true);
                }
            } else if (warningY > 0) {
                setCursor(x - 12, warningY);
                setColor(12);
                cout << " Ошибка: Переключитесь на английский! ";
                setColor(7);
            }
            continue;
        }

        if (utf8Length(input) < static_cast<size_t>(width - 1) && isprint(static_cast<unsigned char>(choose))) {
            clearWarning();
            input += static_cast<char>(choose);
            drawInputContent(x, y, width, input, isPassword, true);
        }
    }

    drawInputContent(x, y, width, input, isPassword, false);

    return input;
}
