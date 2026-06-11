#include "InputHandler.h"

#include <conio.h>

// чтение кода кнопки и распознавание стрелочек
int InputHandler::getExtKey() {
    // _getwch вместо _getch — чтобы корректно ловить юникод (кириллицу),
    // а не отдельные байты UTF-8
    wint_t key = _getwch();

    // проверка на спецкод стрелочки
    if (key == 224 || key == 0) {
        wint_t ext = _getwch();  // чтение второго кода для уточнения стрелки
        if (ext == 72) {
            return Key::UP;
        }
        if (ext == 80) {
            return Key::DOWN;
        }
        if (ext == 75) {
            return Key::LEFT;
        }
        if (ext == 77) {
            return Key::RIGHT;
        }
        if (ext == 83) {
            return Key::DEL;
        }
    }

    return (int)key;  // возврат кода обычной кнопки
}

// замирание программы и ожидание нажатия любой клавиши
void InputHandler::waitAnyKey() {
    _getch();
}
