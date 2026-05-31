#include "ClientManager.h"

#include <sstream>
#include <vector>

#include "../core/Database.h"
#include "../core/Logger.h"
#include "ClientStruct.h"

// добавление нового абонента в список
int ClientManager::addClient(const std::string &name,
                             const std::string &phone,
                             const std::string &tariff,
                             double balance) {
    if (name.empty() || phone.empty()) {
        return 1;
    }

    std::vector<Client> clients = Database::loadClients();

    for (const auto &c : clients) {
        if (c.phoneNumber == phone) {
            return 2;
        }
    }

    Client newClient;
    newClient.fullName = name;
    newClient.phoneNumber = phone;
    newClient.tariffName = tariff;
    newClient.balance = balance;
    newClient.id = clients.empty() ? 1 : clients.back().id + 1;
    newClient.isActive = true;

    clients.push_back(newClient);
    if (!Database::saveClients(clients)) return 3;

    std::ostringstream det;
    det << "id=" << newClient.id << ", ФИО=" << name
        << ", тел=" << phone << ", тариф=" << (tariff.empty() ? "-" : tariff)
        << ", баланс=" << balance;
    Logger::log(LogCategory::CLIENT, "Добавлен клиент", det.str());
    return 0;
}

// получение списка всех абонентов из базы
std::vector<Client> ClientManager::getAllClients() {
    return Database::loadClients();
}

// поиск абонентов по тексту или номеру id
std::vector<Client> ClientManager::findClients(const std::string &query) {
    std::vector<Client> all = Database::loadClients();  // получение всех
    std::vector<Client> results;                        // те кто подошел

    for (const auto &c : all) {
        bool match = false;
        // поиск по id если введено число
        if (std::to_string(c.id) == query) match = true;
        // поиск в номере телефона
        if (c.phoneNumber.find(query) != std::string::npos) match = true;
        // поиск в ФИО
        if (c.fullName.find(query) != std::string::npos) match = true;

        if (match) results.push_back(c);  // добавление совпадения в результат
    }

    return results;
}

std::vector<Client> ClientManager::findClientsByFields(
    const std::string &idQuery, const std::string &nameQuery,
    const std::string &phoneQuery, const std::string &tariffQuery) {
    std::vector<Client> all = Database::loadClients();

    bool allEmpty = idQuery.empty() && nameQuery.empty() && phoneQuery.empty() && tariffQuery.empty();
    if (allEmpty) return all;

    std::vector<Client> results;
    for (const auto &c : all) {
        if (!idQuery.empty() && std::to_string(c.id).find(idQuery) == std::string::npos) continue;
        if (!nameQuery.empty() && c.fullName.find(nameQuery) == std::string::npos) continue;
        if (!phoneQuery.empty() && c.phoneNumber.find(phoneQuery) == std::string::npos) continue;
        if (!tariffQuery.empty() && c.tariffName.find(tariffQuery) == std::string::npos) continue;
        results.push_back(c);
    }
    return results;
}

bool ClientManager::updateClient(int id, const Client &updatedData) {
    std::vector<Client> clients = Database::loadClients();
    for (auto &c : clients) {
        if (c.id == id) {
            Client before = c;
            c = updatedData;
            c.id = id;  // На всякий случай сохраняем ID
            if (!Database::saveClients(clients)) return false;

            std::ostringstream det;
            det << "id=" << id;
            if (before.fullName != c.fullName)
                det << "; ФИО: '" << before.fullName << "' -> '" << c.fullName << "'";
            if (before.phoneNumber != c.phoneNumber)
                det << "; тел: '" << before.phoneNumber << "' -> '" << c.phoneNumber << "'";
            if (before.tariffName != c.tariffName)
                det << "; тариф: '" << before.tariffName << "' -> '" << c.tariffName << "'";
            if (before.balance != c.balance)
                det << "; баланс: " << before.balance << " -> " << c.balance;
            if (before.isActive != c.isActive)
                det << "; статус: " << (before.isActive ? "Активен" : "Заблок.")
                    << " -> " << (c.isActive ? "Активен" : "Заблок.");
            Logger::log(LogCategory::CLIENT, "Изменён клиент", det.str());
            return true;
        }
    }
    return false;
}

bool ClientManager::deleteClient(int id) {
    std::vector<Client> clients = Database::loadClients();
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it->id == id) {
            std::string snapshot = "id=" + std::to_string(it->id) +
                                   ", ФИО=" + it->fullName +
                                   ", тел=" + it->phoneNumber;
            clients.erase(it);
            if (!Database::saveClients(clients)) return false;
            Logger::log(LogCategory::CLIENT, "Удалён клиент", snapshot);
            return true;
        }
    }
    return false;
}

// смена статуса активности абонента
bool ClientManager::toggleClientStatus(int id) {
    std::vector<Client> clients = Database::loadClients();
    for (auto &c : clients) {
        if (c.id == id) {
            c.isActive = !c.isActive;  // инвертирование статуса
            if (!Database::saveClients(clients)) return false;
            Logger::log(LogCategory::CLIENT, "Изменён статус клиента",
                        "id=" + std::to_string(id) + ", новый статус=" +
                            (c.isActive ? "Активен" : "Заблокирован"));
            return true;
        }
    }
    return false;
}