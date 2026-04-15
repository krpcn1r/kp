#include "HomeMenu.h"
#include "../auth/AuthManager.h"
#include "../core/Database.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include <iostream>
#include <string>
#include <vector>

#include "../clients/ClientMenu.h"
#include "../clients/ClientManager.h"
using namespace std;

// рисует главное меню после того как юзер зашел в систему
HomeResult HomeMenu::show() {
  int selectedOption = 0; // какая кнопка сейчас выбрана
  const int numOptions = 8; // всего 8 пунктов меню
  bool needFullRedraw = true; // флаг полной перерисовки окна

  while (true) {
    if (needFullRedraw) {
      clearScreen(); // чистим всё перед рисованием

      drawDoubleBox(2, 1, 76, 28, 15); // большая рамка

      // рисуем большой логотип программы (ASCII арт)
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

      // пишем кто сейчас зашел и какая у него роль
      setColor(7);
      setCursor(4, 15);
      User u = AuthManager::getCurrentUser();
      string roleStr = (u.role == Role::ADMIN) ? "Администратор" : "Оператор";
      cout << "Пользователь: " << u.login << "   |   Статус: " << roleStr;

      // рисуем разделительную линию
      setColor(15);
      setCursor(2, 16);
      cout << "╠═══════════════════════════════════════════════════════════════"
              "═══════════╣";

      needFullRedraw = false;
    }

    drawFooter(29, false);

    // Рендерим опции
    // список всех доступных кнопок в главном меню
    string options[numOptions] = {
        "1. Просмотр базы клиентов ", "2. Поиск по базе          ",
        "3. Просмотр текущих тарифов", "4. Оформление нового клиента",
        "5. Редактирование записей  ", "6. Сменить пароль          ",
        "7. Выход в меню авторизации", "8. Закрыть программу       "};

    // выводим пункты меню и красиво их красим
    for (int i = 0; i < numOptions; i++) {
      int yPos = 17 + i;
      if (i >= 3)
        yPos += 1; // делаем отступ после 3-й кнопки
      if (i >= 5)
        yPos += 1; // делаем отступ после 5-й кнопки

      // рисуем линии между группами кнопок
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
        // подсвечиваем ту кнопку на которой стоит курсор
        if (i == 7)
          setColor(12); // красная если это выход
        else
          setColor(10); // зеленая для работы
        cout << "> " << options[i] << "         ";
      } else {
        // серые если не выбраны
        if (i == 7)
          setColor(4);
        else
          setColor(8);
        cout << "  " << options[i] << "         ";
      }
    }

    int key = InputHandler::getExtKey(); // слушаем что нажал юзер

    // листаем меню вниз или по табу
    if (key == Key::TAB || key == Key::DOWN) {
      selectedOption = (selectedOption + 1) % numOptions;
    } else if (key == Key::UP) {
      selectedOption = (selectedOption - 1 + numOptions) % numOptions;
    } else if ((key >= '1' && key <= '8') || key == Key::ENTER) {
      // если нажали цифру то сразу выбираем нужный пункт
      if (key >= '1' && key <= '8') {
          selectedOption = key - '1';
      }
      
      // тут логика что делать для каждой кнопки
      if (selectedOption == 0) ClientManager::getAllClients(), ClientMenu::showList(); // заходим в список
      else if (selectedOption == 1) ClientMenu::showSearch(); // идем искать
      else if (selectedOption == 3) ClientMenu::showAddClient(); // создаем нового чела
      else if (selectedOption == 4) ClientMenu::showEditClient(0); // пока просто заглушка
      else if (selectedOption == 5) showChangePassword();
      else if (selectedOption == 6) return HomeResult::LOGOUT; // разлогиниться
      else if (selectedOption == 7) return HomeResult::EXIT_APP; // закрыть всё
      else {
        showPlaceholder(options[selectedOption]); // если раздел еще не готов
      }
      
      needFullRedraw = true; // перерисовываем всё после возврата из меню
    } else if (key == Key::ESC) {
      // если нажали эскейп спрашиваем реально ли выйти
      if (showConfirmation("Выйти из учетной записи?")) {
          return HomeResult::LOGOUT;
      }
      needFullRedraw = true;
    }
  }
}

void HomeMenu::showChangePassword() {
    string oldPassword = "";
    string newPassword = "";
    int activeField = 0; // 0 это ввод логина а 1 пароля

    clearScreen(); // очистка экрана перед отрисовкой нового окна

    // отрисовка большой рамки для регистрации
    drawDoubleBox(4, 4, 45, 12, 8);

    setCursor(19, 5);
    setColor(11);
    cout << "СМЕНА ПАРОЛЯ";

    // полоска под заголовком
    setColor(8);
    setCursor(4, 6);
    cout << "╠";
    for (int i = 0; i < 43; i++)
        cout << "═";
    cout << "╣";

    // вывод условий безопасности пароля
    setCursor(6, 7);
    setColor(7);
    cout << "Условия безопасности:";
    setCursor(7, 8);
    setColor(8);
    cout << "• Минимум 8 символов";
    setCursor(7, 9);
    cout << "• 1 цифра + 1 спецсимвол";
    setCursor(7, 10);
    cout << "• Только английские буквы";

    setCursor(4, 11);
    cout << "╠";
    for (int i = 0; i < 43; i++)
        cout << "═";
    cout << "╣";

    drawInputContent(24, 12, 20, oldPassword, false, false);
    drawInputContent(24, 14, 20, newPassword, false, false);

    // запуск цикла для заполнения полей
    while (true) {
        // подсветка поля логин если фокус на нем
        setCursor(6, 12);
        if (activeField == 0)
            setColor(10);
        else
            setColor(7);
        cout << (activeField == 0 ? "> Старый пароль:" : "  Старый пароль:");

        // тоже самое для пароля
        setCursor(6, 14);
        if (activeField == 1)
            setColor(10);
        else
            setColor(7);
        cout << (activeField == 1 ? "> Новый пароль:" : "  Новый пароль:");

        // отрисовка кнопок в нижней части
        drawFooter(20);

        int exitKey = 0;
        // вызов функции ввода текста
        if (activeField == 0) {
            oldPassword = processInput(24, 12, 20, oldPassword, false, exitKey, 16);
        }
        else {
            newPassword = processInput(24, 14, 20, newPassword, false, exitKey, 16);
        }

        // управление переключением полей или сохранение
        if (exitKey == Key::TAB || exitKey == Key::UP || exitKey == Key::DOWN) {
            activeField = (activeField == 0) ? 1 : 0;
        }
        else if (exitKey == Key::ENTER) {
            if (activeField == 0)
                activeField = 1; // переход к полю пароля
            else
                break; // попытка создания юзера
        }
        else if (exitKey == Key::ESC) {
            return; // выход без регистрации
        }
    }

    setCursor(8, 17);
    // попытка добавления нового пользователя в базу
    int result = AuthManager::changePassword(oldPassword, newPassword);
    if (result == 0) {
        setColor(10);
        cout << "Пароль успешно сменился!    ";
    }
    else {
        // вывод ошибки красным цветом
        setColor(12);
        if (result == 1)
            cout << "Ошибка: неверные данные                                   ";
        else if (result == 2)
            cout << "Ошибка: старый пароль не совпадает                        ";
        else if (result == 3)
            cout << "Ошибка: одно из полей пустое                              ";
        else if (result == 4)
            cout << "Ошибка: длина пароля меньше 8 символов                    ";
        else if (result == 5)
            cout << "Ошибка: в пароле отсутствуют специальные символы или цифры";
        else if (result == 6)
            cout << "Ошибка: только английские буквы                           ";
        else
            cout << "Ошибка записи в базу данных                               ";
    }

    setCursor(8, 18);
    setColor(8);
    cout << "Нажмите любую кнопку...";
    InputHandler::waitAnyKey();
}
