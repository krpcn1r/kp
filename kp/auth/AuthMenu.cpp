#include "../auth/AuthMenu.h"
#include "../core/Database.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "AuthManager.h"
#include "Login.h"
#include "Register.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

bool AuthMenu::show() {
  int selectedOption = 0;
  const int numOptions = 3;
  bool isRunning = true;
  bool needFullRedraw = true;

  while (isRunning) {
    if (AuthManager::isUserLoggedIn()) {
      return true;
    }

    if (needFullRedraw) {
      clearScreen();

      setColor(15);
      setCursor(2, 1);
      cout << "╔═══════════════════════════════════════════════════════════════"
              "═══════════╗";
      for (int i = 2; i <= 23; i++) {
        setCursor(2, i);
        cout << "║";
        setCursor(77, i);
        cout << "║";
      }
      setCursor(2, 24);
      cout << "╚═══════════════════════════════════════════════════════════════"
              "═══════════╝";

      setCursor(2, 14);
      cout << "╠═══════════════════════════════════════════════════════════════"
              "═══════════╣";
      setCursor(2, 16);
      cout << "╠═══════════════════════════════════════════════════════════════"
              "═══════════╣";

      setColor(7);
      setCursor(4, 15);

      std::vector<User> users = Database::loadUsers();

      cout << " Статус БД: ОК   |   Пользователей: " << users.size()
           << "   |   Сессия: [Гость]         ";

      drawFooter(26, false);

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

      needFullRedraw = false;
    }

    string options[numOptions] = {"1. Войти в существующий аккаунт",
                                  "2. Создать новый профиль       ",
                                  "0. Завершить работу            "};

    for (int i = 0; i < numOptions; i++) {
      int yPos = 18 + i + (i == 2 ? 1 : 0);
      setCursor(6, yPos);
      if (i == selectedOption) {
        if (i == 2)
          setColor(12);
        else
          setColor(10);
        cout << "> " << options[i];
      } else {
        if (i == 2)
          setColor(4);
        else
          setColor(8);
        cout << "  " << options[i];
      }
    }

    int key = InputHandler::getExtKey();

    if (key == Key::TAB) {
      selectedOption++;
      if (selectedOption >= numOptions)
        selectedOption = 0;
    } else if (key == Key::UP) {
      selectedOption--;
      if (selectedOption < 0)
        selectedOption = numOptions - 1;
    } else if (key == Key::DOWN) {
      selectedOption++;
      if (selectedOption >= numOptions)
        selectedOption = 0;
    } else if (key == Key::ENTER || key == '1' || key == '2' || key == '0') {
      if (key == '1')
        selectedOption = 0;
      if (key == '2')
        selectedOption = 1;
      if (key == '0')
        selectedOption = 2;

      if (selectedOption == 0) {
        showLoginScreen();
        needFullRedraw = true;
      } else if (selectedOption == 1) {
        showRegisterScreen();
        needFullRedraw = true;
      } else if (selectedOption == 2) {
        isRunning = false;
      }
    } else if (key == Key::ESC) {
      isRunning = false;
    }
  }

  return false;
}
