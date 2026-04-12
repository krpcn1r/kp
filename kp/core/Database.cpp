#include "Database.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using json = nlohmann::json;

const string Database::USERS_FILE = "data/users.json";
const string Database::CLIENTS_FILE = "data/clients.json";

bool Database::saveUsers(const vector<User> &users) {
  json j = users;
  ofstream file(USERS_FILE);
  if (!file.is_open())
    return false;

  file << j.dump(4);
  file.close();
  return true;
}

vector<User> Database::loadUsers() {
  vector<User> users;
  ifstream file(USERS_FILE);
  if (!file.is_open())
    return users;

  try {
    json j;
    file >> j;
    users = j.get<vector<User>>();
  } catch (...) {
  }
  file.close();
  return users;
}

bool Database::saveClients(const vector<Client> &clients) {
  json j = clients;
  ofstream file(CLIENTS_FILE);
  if (!file.is_open())
    return false;

  file << j.dump(4);
  file.close();
  return true;
}

vector<Client> Database::loadClients() {
  vector<Client> clients;
  ifstream file(CLIENTS_FILE);
  if (!file.is_open())
    return clients;

  try {
    json j;
    file >> j;
    clients = j.get<vector<Client>>();
  } catch (...) {
  }
  file.close();
  return clients;
}