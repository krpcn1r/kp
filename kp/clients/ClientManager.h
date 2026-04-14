#pragma once
#include <string>
#include <vector>
#include "ClientStruct.h"

// класс для управления списком абонентов
class ClientManager {
public:
    // добавление нового человека в базу
    static int addClient(const std::string &name, const std::string &phone);
    // получение полного списка всех абонентов из базы
    static std::vector<Client> getAllClients();
    // поиск людей по имени или телефону
    static std::vector<Client> findClients(const std::string &query);
    // обновление данных при смене тарифа или имени
    static bool updateClient(int id, const Client& updatedData);
    // переключение статуса активности абонента (блокировка)
    static bool toggleClientStatus(int id);
};
