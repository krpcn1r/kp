#include "InputHandler.h"
#include <conio.h>

int InputHandler::getExtKey() {
  int key = _getch();

  if (key == 224 || key == 0) {
    int ext = _getch();
    if (ext == 72)
      return Key::UP;
    if (ext == 80)
      return Key::DOWN;
    if (ext == 75)
      return Key::LEFT;
    if (ext == 77)
      return Key::RIGHT;
  }

  return key;
}

void InputHandler::waitAnyKey() { _getch(); }
