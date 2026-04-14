#include "Login.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "AuthManager.h"
#include <iostream>

using namespace std;

// вывод экрана для авторизации пользователя
void showLoginScreen() {
  string login = "";
  string password = "";
  int activeField = 0; // 0 это логин а 1 это пароль

  clearScreen(); // очистка экрана перед отрисовкой

  // отрисовка самой рамки окна входа
  drawDoubleBox(4, 5, 42, 11, 8);

  // вывод заголовка покрасивее
  setCursor(19, 6);
  setColor(11);
  cout << "АВТОРИЗАЦИЯ";

  // отрисовка черты под заголовком
  setColor(8);
  setCursor(4, 7);
  cout << "╠";
  for (int i = 0; i < 40; i++)
    cout << "═";
  cout << "╣";

  // отрисовка пустых полей для ввода
  drawInputContent(18, 9, 20, login, false, false);
  drawInputContent(18, 12, 20, password, true, false);

  // запуск цикла до момента нажатия кнопки входа
  while (true) {
    // вывод слова логин с покраской если оно выбрано
    setCursor(6, 9);
    if (activeField == 0)
      setColor(10);
    else
      setColor(7);
    cout << (activeField == 0 ? "> Логин:" : "  Логин:");

    // тоже самое для пароля
    setCursor(6, 12);
    if (activeField == 1)
      setColor(10);
    else
      setColor(7);
    cout << (activeField == 1 ? "> Пароль:" : "  Пароль:");

    // отрисовка разделителя между строками ввода
    setCursor(4, 11);
    setColor(8);
    cout << "╠";
    for (int i = 0; i < 40; i++)
      cout << "═";
    cout << "╣";

    // отрисовка подсказок по кнопкам в самом низу
    drawFooter(18);

    int exitKey = 0;
    // запуск ввода текста в зависимости от активного поля
    if (activeField == 0) {
      login = processInput(18, 9, 20, login, false, exitKey, 16);
    } else {
      password = processInput(18, 12, 20, password, true, exitKey, 16);
    }

    // обработка нажатия спецкнопок
    if (exitKey == Key::TAB || exitKey == Key::UP || exitKey == Key::DOWN) {
      activeField = (activeField == 0) ? 1 : 0; // переключение между полями
    } else if (exitKey == Key::ENTER) {
      if (activeField == 0)
        activeField = 1; // переход на пароль после ввода логина
      else
        break; // попытка входа после ввода пароля
    } else if (exitKey == Key::ESC) {
      return; // выход без авторизации
    }
  }

  setCursor(8, 14);
  // отправка данных в менеджер для проверки
  if (AuthManager::loginUser(login, password)) {
    setColor(10);
    cout << "Вход выполнен успешно!    ";
  } else {
    // вывод ошибки красным цветом
    setColor(12);
    cout << "Ошибка: неверные данные   ";
  }

  setCursor(8, 15);
  setColor(8);
  cout << "Нажмите любую кнопку...";
  InputHandler::waitAnyKey();
}