#include "Database.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using json = nlohmann::json;

const string Database::USERS_FILE = "data/users.json";
const string Database::CLIENTS_FILE = "data/clients.json";

// сохранение списка юзеров в файл
bool Database::saveUsers(const vector<User> &users) {
  json j = users; // перевод вектора в формат джейсон
  ofstream file(USERS_FILE); // открытие файла для записи
  if (!file.is_open()) // выход с ошибкой если файл не открылся
    return false;

  file << j.dump(4); // запись данных с отступами 4 пробела
  file.close();
  return true;
}

// чтение юзеров из файла для работы в программе
vector<User> Database::loadUsers() {
  vector<User> users;
  ifstream file(USERS_FILE); // открытие файла на чтение
  if (!file.is_open()) // возврат пустого списка если файла нет
    return users;

  try {
    json j;
    file >> j; // чтение данных из файла
    users = j.get<vector<User>>(); // преобразование в вектор юзеров
  } catch (...) {
    // пропуск ошибок при повреждении файла
  }
  file.close(); // закрытие файла
  return users;
}

// сохранение всех абонентов в файл
bool Database::saveClients(const vector<Client> &clients) {
  json j = clients;
  ofstream file(CLIENTS_FILE); // открытие файла клиентов
  if (!file.is_open())
    return false;

  file << j.dump(4); // запись данных
  file.close();
  return true;
}

// загрузка абонентов из базы в память
vector<Client> Database::loadClients() {
  vector<Client> clients;
  ifstream file(CLIENTS_FILE); // чтение файла базы
  if (!file.is_open())
    return clients;

  try {
    json j;
    file >> j; // извлечение джейсон из файла
    clients = j.get<vector<Client>>(); // парсинг данных в вектор объектов
  } catch (...) {
    // игнорирование ошибок кривой базы
  }
  file.close();
  return clients;
}