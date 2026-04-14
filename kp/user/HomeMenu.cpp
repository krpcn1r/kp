#include "HomeMenu.h"
#include "../auth/AuthManager.h"
#include "../core/Database.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include <iostream>
#include <string>
#include <vector>

#include "../clients/ClientMenu.h"

using namespace std;

HomeResult HomeMenu::show() {
  int selectedOption = 0;
  const int numOptions = 8;
  bool needFullRedraw = true;

  while (true) {
    if (needFullRedraw) {
      clearScreen();

      drawDoubleBox(2, 1, 76, 28, 15);

      // 1) Блок 1: Логотип и информация
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

      setColor(15);
      setCursor(2, 14);
      cout << "╠═══════════════════════════════════════════════════════════════"
              "═══════════╣";

      setColor(7);
      setCursor(4, 15);
      User u = AuthManager::getCurrentUser();
      string roleStr = (u.role == Role::ADMIN) ? "Администратор" : "Оператор";
      cout << "Пользователь: " << u.login << "   |   Статус: " << roleStr;

      setColor(15);
      setCursor(2, 16);
      cout << "╠═══════════════════════════════════════════════════════════════"
              "═══════════╣";

      needFullRedraw = false;
    }

    drawFooter(29, false);

    // Рендерим опции
    string options[numOptions] = {
        "1. Просмотр базы клиентов ", "2. Поиск по базе          ",
        "3. Просмотр текущих тарифов", "4. Оформление нового клиента",
        "5. Редактирование записей  ", "6. Сменить пароль          ",
        "7. Выход в меню авторизации", "8. Закрыть программу       "};

    for (int i = 0; i < numOptions; i++) {
      int yPos = 17 + i;
      if (i >= 3)
        yPos += 1; // Сепаратор после 3-й опции
      if (i >= 5)
        yPos += 1; // Сепаратор после 5-й опции

      // Рисуем сепараторы
      if (i == 3) {
        setCursor(2, 17 + 3);
        setColor(15);
        cout << "╠═════════════════════════════════════════════════════════════"
                "═════════════╣";
      }
      if (i == 5) {
        setCursor(2, 17 + 5 + 1);
        setColor(15);
        cout << "╠═════════════════════════════════════════════════════════════"
                "═════════════╣";
      }
      setCursor(6, yPos);
      if (i == selectedOption) {
        if (i == 7)
          setColor(12); // Ярко-красный только для выхода из программы
        else
          setColor(10); // Зеленый для всех остальных
        cout << "> " << options[i] << "         ";
      } else {
        if (i == 7)
          setColor(4); // Темно-красный только для выхода из программы
        else
          setColor(8); // Серый для остальных
        cout << "  " << options[i] << "         ";
      }
    }

    int key = InputHandler::getExtKey();

    if (key == Key::TAB || key == Key::DOWN) {
      selectedOption = (selectedOption + 1) % numOptions;
    } else if (key == Key::UP) {
      selectedOption = (selectedOption - 1 + numOptions) % numOptions;
    } else if ((key >= '1' && key <= '8') || key == Key::ENTER) {
      if (key >= '1' && key <= '8') {
          selectedOption = key - '1';
      }
      
      // Выполняем действие
      if (selectedOption == 0) ClientMenu::showList();
      else if (selectedOption == 1) ClientMenu::showSearch();
      else if (selectedOption == 3) ClientMenu::showAddClient();
      else if (selectedOption == 4) ClientMenu::showEditClient(0);
      else if (selectedOption == 6) return HomeResult::LOGOUT;
      else if (selectedOption == 7) return HomeResult::EXIT_APP;
      else {
        showPlaceholder(options[selectedOption]);
      }
      
      needFullRedraw = true;
    } else if (key == Key::ESC) {
      if (showConfirmation("Выйти из учетной записи?")) {
          return HomeResult::LOGOUT;
      }
      needFullRedraw = true;
    }
  }
}
