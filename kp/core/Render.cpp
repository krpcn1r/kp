#include "Render.h"
#include "InputHandler.h"
#include <iostream>
#include <windows.h>
using namespace std;

// показ экрана типа раздел еще не сделан
void showPlaceholder(const string& title) {
  clearScreen();
  drawDoubleBox(10, 8, 60, 10, 14); // Желтая рамка
  
  setCursor(26, 10);
  setColor(14);
  cout << "РАЗДЕЛ В РАЗРАБОТКЕ";
  
  setCursor(15, 12);
  setColor(15);
  cout << "Раздел: " << title;
  
  setCursor(15, 14);
  setColor(8);
  cout << "Данный функционал будет доступен в будущих версиях.";
  
  setCursor(15, 16);
  cout << "Нажмите любую клавишу для возврата...";
  
  InputHandler::waitAnyKey();
}

// маленькое окошко для подтверждения действия да или нет
bool showConfirmation(const string& message) {
  // отрисовка небольшого окна в центре
  int w = 46;
  int h = 7;
  int x = (80 - w) / 2;
  int y = 10;
  
  drawDoubleBox(x, y, w, h, 14); // Желтая рамка
  
  // закрашивание внутренней части черным чтобы окно не просвечивало
  setColor(0);
  for (int i = 1; i < h - 1; i++) {
    setCursor(x + 1, y + i);
    for (int j = 0; j < w - 2; j++) cout << " ";
  }
  
  setCursor(x + (w - message.length())/2, y + 2);
  setColor(15);
  cout << message;
  
  setCursor(x + 10, y + 4);
  setColor(10); cout << "[Enter] Да";
  
  setCursor(x + w - 18, y + 4);
  setColor(12); cout << "[Esc] Нет";
  
  while (true) {
    int key = InputHandler::getExtKey();
    if (key == Key::ENTER || key == 'y' || key == 'Y') return true;
    if (key == Key::ESC || key == 'n' || key == 'N') return false;
  }
}

// отрисовка подсказок по кнопкам в нижней части экрана
void drawFooter(int y, bool hasBack) {
    setColor(8);
    setCursor(2, y);
    std::cout << "[Tab] Навигация  | [Enter] Выбрать";
    if (hasBack) std::cout << "   |  [Esc] Назад";
    else std::cout << "   |  [Esc] Выход";
}

// отрисовка текста внутри поля ввода при печати
void drawInputContent(int x, int y, int width, string input, bool isPassword, bool isActive) {
    int bgColor = isActive ? (1 * 16) : 0; 
    int fgColor = isActive ? 15 : 7;       
    setColor(bgColor + fgColor);

    setCursor(x, y);
    
    string displayString = "";
    if (isPassword) {
        displayString = string(input.length(), '*');
    } else {
        displayString = input;
    }

    if (isActive && displayString.length() < (size_t)width) {
        displayString += "|";
    }

    while (displayString.length() < (size_t)width) {
        displayString += "_";
    }

    cout << displayString;
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

// отрисовка красивой рамки из двойных линий
void drawDoubleBox(int x, int y, int w, int h, int color) {
    setColor(color);
    setCursor(x, y);

    cout << "╔";
    for (int i = 0; i < w - 2; i++) cout << "═";
    cout << "╗";

    for (int i = 1; i < h - 1; i++) {
        setCursor(x, y + i);
        cout << "║";
        setCursor(x + w - 1, y + i);
        cout << "║";
    }

    setCursor(x, y + h - 1);
    cout << "╚";
    for (int i = 0; i < w - 2; i++) cout << "═";
    cout << "╝";

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

// функция для ввода текста юзером (старая версия)
string inputField(int x, int y, int width, bool isPassword, int warningY) {
    string input = "";
    int choose;

    while (true) {
        setCursor(x + input.length(), y);
        choose = InputHandler::getExtKey();

        if (choose == Key::ENTER) break;

        if (choose > 127) {
            if (warningY > 0) {
                setCursor(x - 12, warningY);
                setColor(12);
                cout << " Ошибка: Переключитесь на английский! ";
                setColor(7);
            }
            continue;
        }

        if (choose == Key::BACKSPACE) {
            if (input.length() > 0) {
                input.pop_back();
                setCursor(x, y);
                for(int i=0; i< width; i++) cout << " ";
                setCursor(x, y);
                if (isPassword) {
                    for(size_t i=0; i<input.length(); i++) cout << "*";
                } else {
                    cout << input;
                }
            }
        }
        else if (input.length() < width - 2 && isprint(static_cast<unsigned char>(choose))) {
            if (warningY > 0) {
                setCursor(x - 12, warningY);
                cout << "                                       ";
            }
            input += choose;
            setCursor(x, y);
            if (isPassword) {
                for(size_t i=0; i<input.length(); i++) cout << "*";
            } else {
                cout << input;
            }
        }
    }
    return input;
}

// основная функция для ввода текста с проверкой раскладки
string processInput(int x, int y, int width, string currentInput, bool isPassword, int& exitKey, int warningY) {
    string input = currentInput;
    int choose;

    drawInputContent(x, y, width, input, isPassword, true);

    while (true) {
        choose = InputHandler::getExtKey();

        if (choose == Key::ENTER || choose == Key::TAB || choose == Key::ESC || choose == Key::UP || choose == Key::DOWN) {
            exitKey = choose;
            break;
        }

        if (choose > 127) {
            if (warningY > 0) {
                setCursor(x - 12, warningY);
                setColor(12);
                cout << " Ошибка: Переключитесь на английский! ";
                setColor(7);
            }
            continue;
        }

        if (choose == Key::BACKSPACE) {
            if (input.length() > 0) {
                input.pop_back();
                drawInputContent(x, y, width, input, isPassword, true);
            }
        }
        else if (input.length() < width - 1 && isprint(static_cast<unsigned char>(choose))) {
            if (warningY > 0) {
                setCursor(x - 12, warningY);
                cout << "                                       ";
            }
            input += choose;
            drawInputContent(x, y, width, input, isPassword, true);
        }
    }
    
    drawInputContent(x, y, width, input, isPassword, false);
    
    return input;
}