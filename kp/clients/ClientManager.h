#pragma once
#include <string>
#include <vector>

#include "ClientStruct.h"

// класс для управления списком абонентов
class ClientManager {
public:
    // добавление нового человека в базу
    static int addClient(const std::string &name, const std::string &phone, const std::string &tariff = "", double balance = 0.0);
    // получение полного списка всех абонентов из базы
    static std::vector<Client> getAllClients();
    // поиск по отдельным полям (пустое поле — не учитывается)
    static std::vector<Client> findClientsByFields(const std::string &id,
                                                   const std::string &name,
                                                   const std::string &phone,
                                                   const std::string &tariff);
    // обновление данных при смене тарифа или имени
    static bool updateClient(int id, const Client &updatedData);
    // удаление абонента по ID
    static bool deleteClient(int id);
};
