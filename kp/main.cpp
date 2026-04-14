#include "admin/AdminPanel.h"
#include "admin/ConfigWizard.h"
#include "core/InputHandler.h"
#include <iostream>
#include <string>
#include <windows.h>

#include "auth/AuthManager.h"
#include "auth/AuthMenu.h"


#include "clients/Billing.h"
#include "clients/ClientManager.h"
#include "clients/ClientStruct.h"

#include "core/Database.h"
#include "core/InputHandler.h"
#include "core/Render.h"
#include "core/Utils.h"

using namespace std;

int main() {
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = false;
  SetConsoleCursorInfo(hConsole, &cursorInfo);

  bool isAuthenticated = AuthMenu::show();

  if (isAuthenticated) {
    clearScreen();
    setColor(10);
    cout << "Успешный вход в систему! Пользователь: "
         << AuthManager::getCurrentUser().login << std::endl;
    setColor(8);
    cout << "\nНажмите любую кнопку для завершения работы...";
    InputHandler::waitAnyKey();
  }

  cursorInfo.bVisible = true;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
  setColor(7);
  clearScreen();

  return 0;
}