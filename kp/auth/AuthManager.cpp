#include "AuthManager.h"
#include "../core/Database.h"
#include <vector>

using namespace std;

bool AuthManager::isLoggedIn = false;
User AuthManager::currentUser = {};

// регистрация нового пользователя
int AuthManager::registerUser(const string &login, const string &password,
                              Role role) {
  if (login.empty() || password.empty()) {
    return 1; // пустой текст
  }

  // проверка на английские буквы
  for (unsigned char c : login)
    if (c > 127)
      return 5; // если ввели русскую букву
  for (unsigned char c : password)
    if (c > 127)
      return 5;


  if (password.length() < 8)
    return 2; 

  // проверка чтобы в пароле были цифры и знаки
  bool hasDigit = false;
  bool hasSpecial = false;
  for (char c : password) {
    if (isdigit((unsigned char)c))
      hasDigit = true;
    if (ispunct((unsigned char)c))
      hasSpecial = true;
  }

  // если нет цифр или знаков пароль не подходит
  if (!hasDigit || !hasSpecial)
    return 2;

  // загрузка списка всех юзеров из базы
  vector<User> users = Database::loadUsers();

  // проверка занятости логина
  for (const auto &u : users) {
    if (u.login == login) {
      return 3; // логин уже есть в базе
    }
  }

  // создание нового юзера
  User newUser;
  newUser.login = login;
  newUser.password = password;

  // если база пустая то первый будет админом
  if (users.empty()) {
    newUser.role = Role::ADMIN;
  } else {
    newUser.role = role;
  }

  users.push_back(newUser); // добавление в общий список
  return Database::saveUsers(users) ? 0 : 4; // сохранение всё в файл
}

// функция входа в систему по логину
bool AuthManager::loginUser(const string &login, const string &password) {
  vector<User> users = Database::loadUsers(); // загрузка юзеров

  // поиск совпадения логина и пароля
  for (const auto &u : users) {
    if (u.login == login && u.password == password) {
      currentUser = u; // сохранение данных того кто зашел
      isLoggedIn = true; // установка флага входа
      return true;
    }
  }

  return false; // не нашли такого юзера
}

void AuthManager::logout() {
  isLoggedIn = false;
  currentUser = {};
}

bool AuthManager::isUserLoggedIn() { return isLoggedIn; }

User AuthManager::getCurrentUser() { return currentUser; }
