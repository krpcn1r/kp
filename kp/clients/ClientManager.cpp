#include "ClientManager.h"
#include "ClientStruct.h"
#include "../core/Database.h"
#include <vector>

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
  newClient.fullName    = name;
  newClient.phoneNumber = phone;
  newClient.tariffName  = tariff;
  newClient.balance     = balance;
  newClient.id          = clients.empty() ? 1 : clients.back().id + 1;
  newClient.isActive    = true;

  clients.push_back(newClient);
  return Database::saveClients(clients) ? 0 : 3;
}

// получение списка всех абонентов из базы
std::vector<Client> ClientManager::getAllClients() {
  return Database::loadClients();
}

// поиск абонентов по тексту или номеру id
std::vector<Client> ClientManager::findClients(const std::string &query) {
  std::vector<Client> all = Database::loadClients(); // получение всех
  std::vector<Client> results; // те кто подошел

  for (const auto &c : all) {
    bool match = false;
    // поиск по id если введено число
    if (std::to_string(c.id) == query) match = true;
    // поиск в номере телефона
    if (c.phoneNumber.find(query) != std::string::npos) match = true;
    // поиск в ФИО
    if (c.fullName.find(query) != std::string::npos) match = true;

    if (match) results.push_back(c); // добавление совпадения в результат
  }

  return results;
}

std::vector<Client> ClientManager::findClientsByFields(
    const std::string &idQuery,  const std::string &nameQuery,
    const std::string &phoneQuery, const std::string &tariffQuery) {
  std::vector<Client> all = Database::loadClients();

  bool allEmpty = idQuery.empty() && nameQuery.empty()
               && phoneQuery.empty() && tariffQuery.empty();
  if (allEmpty) return all;

  std::vector<Client> results;
  for (const auto &c : all) {
    if (!idQuery.empty()     && std::to_string(c.id).find(idQuery)       == std::string::npos) continue;
    if (!nameQuery.empty()   && c.fullName.find(nameQuery)                == std::string::npos) continue;
    if (!phoneQuery.empty()  && c.phoneNumber.find(phoneQuery)            == std::string::npos) continue;
    if (!tariffQuery.empty() && c.tariffName.find(tariffQuery)            == std::string::npos) continue;
    results.push_back(c);
  }
  return results;
}

bool ClientManager::updateClient(int id, const Client& updatedData) {
  std::vector<Client> clients = Database::loadClients();
  for (auto &c : clients) {
    if (c.id == id) {
      c = updatedData;
      c.id = id; // На всякий случай сохраняем ID
      return Database::saveClients(clients);
    }
  }
  return false;
}

// смена статуса активности абонента
bool ClientManager::toggleClientStatus(int id) {
  std::vector<Client> clients = Database::loadClients();
  for (auto &c : clients) {
    if (c.id == id) {
      c.isActive = !c.isActive; // инвертирование статуса
      return Database::saveClients(clients); // сохранение обратно
    }
  }
  return false;
}