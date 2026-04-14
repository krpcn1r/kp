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
    // меню для правки данных абонента
    static void showEditClient(int clientId);
    
private:
    // отрисовка таблицы с данными абонентов
    static void drawClientTable(const std::vector<Client>& clients, int startIdx = 0, int selectedIdx = -1);
};
