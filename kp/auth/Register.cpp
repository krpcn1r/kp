#include "Register.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "AuthManager.h"
#include <iostream>

using namespace std;

// показ экрана регистрации нового пользователя
void showRegisterScreen() {
  string login = "";
  string password = "";
  int activeField = 0; // 0 это ввод логина а 1 пароля

  clearScreen(); // очистка экрана перед отрисовкой нового окна

  // отрисовка большой рамки для регистрации
  drawDoubleBox(4, 4, 42, 12, 8);

  setCursor(19, 5);
  setColor(11);
  cout << "РЕГИСТРАЦИЯ";

  // полоска под заголовком
  setColor(8);
  setCursor(4, 6);
  cout << "╠";
  for (int i = 0; i < 40; i++)
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
  for (int i = 0; i < 40; i++)
    cout << "═";
  cout << "╣";

  drawInputContent(18, 12, 20, login, false, false);
  drawInputContent(18, 14, 20, password, true, false);

  // запуск цикла для заполнения полей
  while (true) {
    // подсветка поля логин если фокус на нем
    setCursor(6, 12);
    if (activeField == 0)
      setColor(10);
    else
      setColor(7);
    cout << (activeField == 0 ? "> Логин:" : "  Логин:");

    // тоже самое для пароля
    setCursor(6, 14);
    if (activeField == 1)
      setColor(10);
    else
      setColor(7);
    cout << (activeField == 1 ? "> Пароль:" : "  Пароль:");

    // отрисовка кнопок в нижней части
    drawFooter(20);

    int exitKey = 0;
    // вызов функции ввода текста
    if (activeField == 0) {
      login = processInput(18, 12, 20, login, false, exitKey, 16);
    } else {
      password = processInput(18, 14, 20, password, true, exitKey, 16);
    }

    // управление переключением полей или сохранение
    if (exitKey == Key::TAB || exitKey == Key::UP || exitKey == Key::DOWN) {
      activeField = (activeField == 0) ? 1 : 0;
    } else if (exitKey == Key::ENTER) {
      if (activeField == 0)
        activeField = 1; // переход к полю пароля
      else
        break; // попытка создания юзера
    } else if (exitKey == Key::ESC) {
      return; // выход без регистрации
    }
  }

  setCursor(8, 17);
  // попытка добавления нового пользователя в базу
  int result = AuthManager::registerUser(login, password);
  if (result == 0) {
    setColor(10);
    cout << "Успешная регистрация!         ";
  } else {
    // проверка кода ошибки при неудаче
    setColor(12);
    if (result == 1)
      cout << "Ошибка: поля не заполнены     ";
    else if (result == 2)
      cout << "Ошибка: слабый пароль         ";
    else if (result == 3)
      cout << "Ошибка: логин уже занят       ";
    else if (result == 5)
      cout << "Ошибка: только английский     ";
    else
      cout << "Ошибка записи в базу данных   ";
  }

  setCursor(8, 18);
  setColor(8);
  cout << "Нажмите любую кнопку...";
  InputHandler::waitAnyKey();
}
