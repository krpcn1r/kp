#include "admin/AdminPanel.h"
#include "admin/ConfigWizard.h"
#include <conio.h>
#include <iostream>
#include <string>
#include <windows.h>

#include "auth/AuthManager.h"
#include "auth/Login.h"
#include "auth/Register.h"

#include "clients/Billing.h"
#include "clients/ClientManager.h"
#include "clients/ClientStruct.h"

#include "core/Database.h"
#include "core/InputHandler.h"
#include "core/Render.h"
#include "core/Utils.h"

using namespace std;

int main() {
  SetConsoleOutputCP(1251);
  SetConsoleCP(1251);
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = false;
  SetConsoleCursorInfo(hConsole, &cursorInfo);

  int selectedOption = 0;
  const int numOptions = 3;
  bool isRunning = true;

  while (isRunning) {
    clearScreen();

    setColor(11);
    setCursor(30, 2);
    cout << "===============================";
    setCursor(30, 3);
    cout << "         MobileHelper          ";
    setCursor(30, 4);
    cout << "===============================";

    string options[numOptions] = {"[L] Войти в систему", "[R] Регистрация",
                                  "[Q] Выход"};

    for (int i = 0; i < numOptions; i++) {
      int textColor = 7;
      int boxColor = 8;

      if (i == selectedOption) {
        textColor = 15;
        boxColor = 10;
      }

      drawTextBox(30, 7 + i * 4, 31, 3, options[i], textColor, boxColor, 2);
    }

    int key = _getch();

    if (key == 9) {
      selectedOption++;
      if (selectedOption >= numOptions)
        selectedOption = 0;
    } else if (key == 224) {
      int arrow = _getch();
      if (arrow == 72) {
        selectedOption--;
        if (selectedOption < 0)
          selectedOption = numOptions - 1;
      } else if (arrow == 80) {
        selectedOption++;
        if (selectedOption >= numOptions)
          selectedOption = 0;
      }
    } else if (key == 13) {
      if (selectedOption == 0)
        key = 'l';
      else if (selectedOption == 1)
        key = 'r';
      else if (selectedOption == 2)
        key = 'q';
    }

    if (key == 'l' || key == 'L') {
      selectedOption = 0;
      clearScreen();
      drawBox(28, 9, 36, 5, 11);
      setCursor(30, 11);
      setColor(15);
      cout << "Запуск функции Login()...";
      _getch();
    } else if (key == 'r' || key == 'R') {
      selectedOption = 1;
      clearScreen();
      drawBox(28, 9, 36, 5, 11);
      setCursor(30, 11);
      setColor(15);
      cout << "Запуск функции Register()...";
      _getch();
    } else if (key == 'q' || key == 'Q') {
      isRunning = false;
    }
  }

  cursorInfo.bVisible = true;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
  setColor(7);
  clearScreen();

  return 0;
}