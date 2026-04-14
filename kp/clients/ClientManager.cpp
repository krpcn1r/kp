#include "ClientManager.h"
#include "ClientStruct.h"
#include "../core/Database.h"
#include <vector>

int ClientManager::addClient(const std::string &name,
                             const std::string &phone) {
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
  newClient.id = clients.empty() ? 1 : clients.back().id + 1;
  newClient.balance = 0.0;
  newClient.isActive = true;

  clients.push_back(newClient);
  return Database::saveClients(clients) ? 0 : 3;
}

std::vector<Client> ClientManager::getAllClients() {
  return Database::loadClients();
}

std::vector<Client> ClientManager::findClients(const std::string &query) {
  std::vector<Client> all = Database::loadClients();
  std::vector<Client> results;
  
  for (const auto &c : all) {
    bool match = false;
    // Поиск по ID
    if (std::to_string(c.id) == query) match = true;
    // Поиск по телефону
    if (c.phoneNumber.find(query) != std::string::npos) match = true;
    // Поиск по ФИО (грубый поиск без учета регистра пока не делаем для простоты, или можно добавить)
    if (c.fullName.find(query) != std::string::npos) match = true;
    
    if (match) results.push_back(c);
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

bool ClientManager::toggleClientStatus(int id) {
  std::vector<Client> clients = Database::loadClients();
  for (auto &c : clients) {
    if (c.id == id) {
      c.isActive = !c.isActive;
      return Database::saveClients(clients);
    }
  }
  return false;
}