#include "Register.h"
#include "AuthManager.h"
#include "../core/Render.h"
#include "../core/InputHandler.h"
#include <iostream>

using namespace std;

void showRegisterScreen() {
    string login = "";
    string password = "";
    int activeField = 0; // 0 - Логин, 1 - Пароль

    clearScreen();
    
    // Рамка (прижата к левому краю, x=4)
    drawDoubleBox(4, 4, 42, 12, 8);
    
    // Заголовок
    setCursor(19, 5);
    setColor(11);
    cout << "РЕГИСТРАЦИЯ";

    // Сепаратор после заголовка
    setColor(8);
    setCursor(4, 6);
    cout << "╠"; for(int i=0; i<40; i++) cout << "═"; cout << "╣";

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

    // Сепаратор после условий
    setCursor(4, 11);
    cout << "╠"; for(int i=0; i<40; i++) cout << "═"; cout << "╣";

    // Исходная отрисовка полей
    drawInputContent(18, 12, 20, login, false, false);
    drawInputContent(18, 14, 20, password, true, false);

    while (true) {
        setCursor(6, 12);
        if (activeField == 0) setColor(10); else setColor(7);
        cout << (activeField == 0 ? "> Логин:" : "  Логин:");
        
        setCursor(6, 14);
        if (activeField == 1) setColor(10); else setColor(7);
        cout << (activeField == 1 ? "> Пароль:" : "  Пароль:");

        // Сепаратор между полями
        setCursor(4, 13);
        setColor(8);
        cout << "╠"; for(int i=0; i<40; i++) cout << "═"; cout << "╣";

        // Кнопки управления внизу
        setColor(8);
        setCursor(2, 25);
        cout << "[Tab] Навигация  | [Enter] Выбрать   |  [Esc] Назад";

        int exitKey = 0;
        if (activeField == 0) {
            login = processInput(18, 12, 20, login, false, exitKey);
        } else {
            password = processInput(18, 14, 20, password, true, exitKey);
        }

        if (exitKey == Key::TAB || exitKey == Key::UP || exitKey == Key::DOWN) {
            activeField = (activeField == 0) ? 1 : 0;
        } else if (exitKey == Key::ENTER) {
            if (activeField == 0) activeField = 1;
            else break;
        } else if (exitKey == Key::ESC) {
            return;
        }
    }

    setCursor(8, 17);
    int result = AuthManager::registerUser(login, password);
    if (result == 0) {
        setColor(10);
        cout << "Успешная регистрация!         "; 
    } else {
        setColor(12);
        if (result == 1) cout << "Ошибка: поля не заполнены     ";
        else if (result == 2) cout << "Ошибка: слабый пароль         ";
        else if (result == 3) cout << "Ошибка: логин уже занят       ";
        else if (result == 5) cout << "Ошибка: только английский     ";
        else cout << "Ошибка записи в базу данных   ";
    }

    setCursor(8, 18);
    setColor(8);
    cout << "Нажмите любую кнопку...";
    InputHandler::waitAnyKey();
}
