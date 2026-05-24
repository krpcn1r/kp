#include "admin/AdminPanel.h"
#include "admin/ConfigWizard.h"
#include "core/InputHandler.h"
#include <iostream>
#include <string>
#include <windows.h>

#include "auth/AuthManager.h"
#include "auth/AuthMenu.h"
#include "user/HomeMenu.h"

#include "clients/Billing.h"
#include "clients/ClientManager.h"
#include "clients/ClientStruct.h"

#include "core/Database.h"
#include "core/InputHandler.h"
#include "core/Logger.h"
#include "core/Render.h"
#include "core/Utils.h"

using namespace std;

// главная точка входа всей проги
int main() {

  // установка кодировки чтобы русский текст показывался нормально
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = false;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
  // скрытие курсора в консоли чтобы не мешал

  // лог запуска и автоматическая тарификация за пропущенные дни
  Logger::logAs("system", LogCategory::SYSTEM, "Запуск программы");
  Billing::runDailyChargeIfDue();

  // запуск бесконечного цикла работы
  while (true) {
    // показ меню входа если юзер не залогинился
    if (!AuthManager::isUserLoggedIn()) {
      if (!AuthMenu::show()) {
        break;
      }
    }

    // открытие главного меню если зашел
    if (AuthManager::isUserLoggedIn()) {
      HomeResult result = HomeMenu::show();

     // сброс логина при нажатии выхода
      if (result == HomeResult::LOGOUT) {
        AuthManager::logout();
        continue;
      } else if (result == HomeResult::EXIT_APP) {
        // выход из программы
        break;
      }
    }
  }

  // возврат курсора и очистка экрана перед выходом
  Logger::logAs("system", LogCategory::SYSTEM, "Завершение программы");
  cursorInfo.bVisible = true;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
  setColor(7);
  clearScreen();

  return 0;
}