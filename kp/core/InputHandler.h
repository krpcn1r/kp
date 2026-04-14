#pragma once

// коды кнопок чтобы не запоминать цифры
namespace Key {
const int UP = 256;
const int DOWN = 257;
const int LEFT = 258;
const int RIGHT = 259;
const int ENTER = 13;
const int TAB = 9;
const int ESC = 27;
const int BACKSPACE = 8;
} // namespace Key

// класс для захвата нажатий клавиш
class InputHandler {
public:
  // захват нажатой кнопки с пониманием стрелочек
  static int getExtKey();

  // ожидание нажатия любой клавиши
  static void waitAnyKey();
};
