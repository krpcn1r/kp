#pragma once
#include <string>
#include <vector>

#include "../clients/ClientStruct.h"
#include "../clients/TariffStruct.h"
#include "../user/UserStruct.h"

// класс для работы с файлами базы данных
class Database {
public:
    // сохранение всех юзеров в файл
    static bool saveUsers(const std::vector<User>& users);
    // чтение юзеров из файла в программу
    static std::vector<User> loadUsers();

    // сохранение всех абонентов в базу
    static bool saveClients(const std::vector<Client>& clients);
    // загрузка списка абонентов из файла
    static std::vector<Client> loadClients();

    // загрузка тарифов из файла
    static std::vector<Tariff> loadTariffs();

private:
    // пути к файлам с данными
    static const std::string USERS_FILE;
    static const std::string CLIENTS_FILE;
    static const std::string TARIFFS_FILE;
};