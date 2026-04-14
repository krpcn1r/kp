#include "Login.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "AuthManager.h"
#include <iostream>

using namespace std;

void showLoginScreen() {
  string login = "";
  string password = "";
  int activeField = 0; // 0 - Логин, 1 - Пароль

  clearScreen();

  drawDoubleBox(4, 5, 42, 11, 8);

  // Заголовок
  setCursor(19, 6);
  setColor(11);
  cout << "АВТОРИЗАЦИЯ";

  setColor(8);
  setCursor(4, 7);
  cout << "╠";
  for (int i = 0; i < 40; i++)
    cout << "═";
  cout << "╣";

  // Исходная отрисовка полей
  drawInputContent(18, 9, 20, login, false, false);
  drawInputContent(18, 12, 20, password, true, false);

  while (true) {
    setCursor(6, 9);
    if (activeField == 0)
      setColor(10);
    else
      setColor(7);
    cout << (activeField == 0 ? "> Логин:" : "  Логин:");

    setCursor(6, 12);
    if (activeField == 1)
      setColor(10);
    else
      setColor(7);
    cout << (activeField == 1 ? "> Пароль:" : "  Пароль:");

    // Сепаратор между полями
    setCursor(4, 11);
    setColor(8);
    cout << "╠";
    for (int i = 0; i < 40; i++)
      cout << "═";
    cout << "╣";

    // Кнопки управления внизу
    drawFooter(18);

    int exitKey = 0;
    if (activeField == 0) {
      login = processInput(18, 9, 20, login, false, exitKey, 16);
    } else {
      password = processInput(18, 12, 20, password, true, exitKey, 16);
    }

    if (exitKey == Key::TAB || exitKey == Key::UP || exitKey == Key::DOWN) {
      activeField = (activeField == 0) ? 1 : 0;
    } else if (exitKey == Key::ENTER) {
      if (activeField == 0)
        activeField = 1;
      else
        break;
    } else if (exitKey == Key::ESC) {
      return;
    }
  }

  setCursor(8, 14);
  if (AuthManager::loginUser(login, password)) {
    setColor(10);
    cout << "Вход выполнен успешно!    ";
  } else {
    setColor(12);
    cout << "Ошибка: неверные данные   ";
  }

  setCursor(8, 15);
  setColor(8);
  cout << "Нажмите любую кнопку...";
  InputHandler::waitAnyKey();
}