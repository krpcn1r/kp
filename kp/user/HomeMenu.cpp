#include "HomeMenu.h"
#include "../auth/AuthManager.h"
#include "../core/Render.h"
#include "../core/InputHandler.h"
#include "../core/Database.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

HomeResult HomeMenu::show() {
    int selectedOption = 0;
    const int numOptions = 8;
    bool needFullRedraw = true;

    while (true) {
        if (needFullRedraw) {
            clearScreen();
            
            // 1) Блок 1: Логотип и информация
            setColor(9);
            setCursor(16, 1);
            cout << "███╗   ███╗ ██████╗ ██████╗ ██╗██╗     ███████╗";
            setCursor(16, 2);
            cout << "████╗ ████║██╔═══██╗██╔══██╗██║██║     ██╔════╝";
            setCursor(16, 3);
            cout << "██╔████╔██║██║   ██║██████╔╝██║██║     █████╗  ";
            setCursor(16, 4);
            cout << "██║╚██╔╝██║██║   ██║██╔══██╗██║██║     ██╔══╝  ";
            setCursor(16, 5);
            cout << "██║ ╚═╝ ██║╚██████╔╝██████╔╝██║███████╗███████╗";
            setCursor(16, 6);
            cout << "╚═╝     ╚═╝ ╚═════╝ ╚═════╝ ╚═╝╚══════╝╚══════╝";

            setCursor(16, 7);
            cout << "██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ ";
            setCursor(16, 8);
            cout << "██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗";
            setCursor(16, 9);
            cout << "███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝";
            setCursor(16, 10);
            cout << "██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗";
            setCursor(16, 11);
            cout << "██║  ██║███████╗███████╗██║     ███████╗██║  ██║";
            setCursor(16, 12);
            cout << "╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝";

            setColor(15);
            setCursor(2, 13);
            cout << "╠══════════════════════════════════════════════════════════════════════════╣";
            
            setColor(7);
            setCursor(4, 14);
            User u = AuthManager::getCurrentUser();
            string roleStr = (u.role == Role::ADMIN) ? "Администратор" : "Оператор";
            cout << "Пользователь: " << u.login << "   |   Статус: " << roleStr;

            setColor(15);
            setCursor(2, 15);
            cout << "╠══════════════════════════════════════════════════════════════════════════╣";

            setColor(8);
            setCursor(2, 25);
            cout << "[Tab] Навигация  | [Enter] Выбрать   |  [Esc] Выход";

            needFullRedraw = false;
        }

        // Рендерим опции
        string options[numOptions] = {
            "Просмотр базы клиентов", 
            "Поиск по базе", 
            "Просмотр текущих тарифов",
            "Оформление нового клиента",
            "Редактирование записей",
            "Сменить пароль",
            "Выход в меню авторизации",
            "Закрыть программу"
        };

        for (int i = 0; i < numOptions; i++) {
            // Расчет y-позиции с учетом сепараторов
            int yPos = 16 + i;
            if (i >= 3) yPos += 1; // Сепаратор после 3-й опции
            if (i >= 5) yPos += 1; // Сепаратор после 5-й опции

            // Рисуем сепараторы
            if (i == 3) {
                setCursor(2, 16 + 3);
                setColor(15);
                cout << "╠══════════════════════════════════════════════════════════════════════════╣";
            }
            if (i == 5) {
                setCursor(2, 16 + 5 + 1);
                setColor(15);
                cout << "╠══════════════════════════════════════════════════════════════════════════╣";
            }

            setCursor(6, yPos);
            if (i == selectedOption) {
                setColor(10);
                cout << "> " << options[i] << "         ";
            } else {
                setColor(8);
                cout << "  " << options[i] << "         ";
            }
        }

        int key = InputHandler::getExtKey();

        if (key == Key::TAB || key == Key::DOWN) {
            selectedOption = (selectedOption + 1) % numOptions;
        } else if (key == Key::UP) {
            selectedOption = (selectedOption - 1 + numOptions) % numOptions;
        } else if (key == Key::ENTER) {
            if (selectedOption == 6) return HomeResult::LOGOUT;
            if (selectedOption == 7) return HomeResult::EXIT_APP;
            
            // Для остальных пока просто заглушка
            setCursor(30, 26);
            setColor(14);
            cout << "Раздел \"" << options[selectedOption] << "\" в разработке...";
            InputHandler::waitAnyKey();
            needFullRedraw = true;
        } else if (key == Key::ESC) {
            return HomeResult::LOGOUT;
        }
    }
}
