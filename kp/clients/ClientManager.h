#pragma once
#include <string>
#include <vector>
#include "ClientStruct.h"

class ClientManager {
public:
    static int addClient(const std::string &name, const std::string &phone);
    static std::vector<Client> getAllClients();
    static std::vector<Client> findClients(const std::string &query);
    static bool updateClient(int id, const Client& updatedData);
    static bool toggleClientStatus(int id);
};
