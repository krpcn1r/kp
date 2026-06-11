#include "Render.h"
#include <windows.h>
#include <iomanip>
#include <iostream>
#include <sstream>
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

static wstring toWide(const string& s) {
    if (s.empty()) {
        return L"";
    }
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    wstring result(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &result[0], len);
    return result;
}

// широкая строка -> UTF-8
static string toUtf8(const wstring& w) {
    if (w.empty()) {
        return "";
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    string result(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &result[0], len, nullptr, nullptr);
    return result;
}

// число символов (а не байт) в UTF-8 строке
size_t utf8Length(const string& s) {
    return toWide(s).size();
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

// обрезка строки до maxLen символов; если не влезает — добавляется "..."
string truncateText(string value, size_t maxLen) {
    wstring w = toWide(value);
    if (w.size() <= maxLen) {
        return value;
    }
    if (maxLen <= 3) {
        return toUtf8(w.substr(0, maxLen));
    }
    return toUtf8(w.substr(0, maxLen - 3)) + "...";
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

// горизонтальная линия вида +-----+ : x,y — левый угол, innerWidth — число дефисов
void drawHLineAt(int x, int y, int innerWidth, int color) {
    setColor(color);
    setCursor(x, y);
    cout << "+";
    for (int i = 0; i < innerWidth; i++) {
        cout << "-";
    }
    cout << "+";
    setColor(7);
}

// форматирование суммы с двумя знаками после запятой ("123.45")
string formatMoney(double value) {
    ostringstream oss;
    oss << fixed << setprecision(2) << value;
    return oss.str();
}

// ASCII-логотип "MOBILE HELPER" в фиксированной позиции (x=16, строки 2..13)
void drawLogo() {
    setColor(9);
    setCursor(16, 2);
    cout << "███╗   ███╗ ██████╗ ██████╗ ██╗██╗     ███████╗";
    setCursor(16, 3);
    cout << "████╗ ████║██╔═══██╗██╔══██╗██║██║     ██╔════╝";
    setCursor(16, 4);
    cout << "██╔████╔██║██║   ██║██████╔╝██║██║     █████╗  ";
    setCursor(16, 5);
    cout << "██║╚██╔╝██║██║   ██║██╔══██╗██║██║     ██╔══╝  ";
    setCursor(16, 6);
    cout << "██║ ╚═╝ ██║╚██████╔╝██████╔╝██║███████╗███████╗";
    setCursor(16, 7);
    cout << "╚═╝     ╚═╝ ╚═════╝ ╚═════╝ ╚═╝╚══════╝╚══════╝";
    setCursor(16, 8);
    cout << "██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ ";
    setCursor(16, 9);
    cout << "██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗";
    setCursor(16, 10);
    cout << "███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝";
    setCursor(16, 11);
    cout << "██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗";
    setCursor(16, 12);
    cout << "██║  ██║███████╗███████╗██║     ███████╗██║  ██║";
    setCursor(16, 13);
    cout << "╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝";
    setColor(7);
}

// один символ (код из _getwch) -> UTF-8
static string utf8Encode(unsigned int cp) {
    return toUtf8(wstring(1, static_cast<wchar_t>(cp)));
}

// удаление последнего символа из UTF-8 строки
static void utf8PopBack(string& s) {
    wstring w = toWide(s);
    if (!w.empty()) {
        w.pop_back();
    }
    s = toUtf8(w);
}

// функция для ввода текста с проверкой раскладки
string processInput(int x, int y, int width, string currentInput, bool isPassword, int& exitKey, int warningY, bool allowUnicode, const string& allowedChars) {
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

        if (isprint(static_cast<unsigned char>(choose))) {
            // фильтр по разрешённым символам (например, только цифры и + для телефона)
            if (!allowedChars.empty() && allowedChars.find(static_cast<char>(choose)) == string::npos) {
                continue;
            }
            if (utf8Length(input) < static_cast<size_t>(width - 1)) {
                clearWarning();
                input += static_cast<char>(choose);
                drawInputContent(x, y, width, input, isPassword, true);
            }
        }
    }

    drawInputContent(x, y, width, input, isPassword, false);

    return input;
}
