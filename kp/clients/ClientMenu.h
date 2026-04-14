#pragma once
#include <string>
#include <vector>
#include "ClientStruct.h"

class ClientMenu {
public:
    static void showList();
    static void showSearch();
    static void showAddClient();
    static void showEditClient(int clientId);
    
private:
    static void drawClientTable(const std::vector<Client>& clients, int startIdx = 0, int selectedIdx = -1);
};
