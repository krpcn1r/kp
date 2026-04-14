#pragma once

// варианты того что юзер может выбрать при выходе из главного меню
enum class HomeResult { 
    LOGOUT, // просто разлогиниться
    EXIT_APP // совсем закрыть программу
};

// класс для главного экрана программы
class HomeMenu {
public:
    // показывает главное меню с кучей кнопок
    static HomeResult show();
};
