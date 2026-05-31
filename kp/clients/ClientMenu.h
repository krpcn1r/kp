#pragma once
#include <string>
#include <vector>

#include "ClientStruct.h"

// класс для отрисовки меню при работе с абонентами
class ClientMenu {
public:
    // показ полного списка людей
    static void showList();
    // показ экрана поиска
    static void showSearch();
    // показ формы добавления нового клиента
    static void showAddClient();
};
