#include "InputHandler.h"
#include <conio.h>

// чтение кода кнопки и распознавание стрелочек
int InputHandler::getExtKey() {
  int key = _getch(); // чтение кода кнопки

  // проверка на спецкод стрелочки
  if (key == 224 || key == 0) {
    int ext = _getch(); // чтение второго кода для уточнения стрелки
    if (ext == 72)
      return Key::UP;
    if (ext == 80)
      return Key::DOWN;
    if (ext == 75)
      return Key::LEFT;
    if (ext == 77)
      return Key::RIGHT;
  }

  return key; // возврат кода обычной кнопки
}

// замирание программы и ожидание нажатия любой клавиши
void InputHandler::waitAnyKey() { _getch(); }
