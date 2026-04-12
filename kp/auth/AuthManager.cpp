#include "AuthManager.h"
#include "../core/Database.h"
#include <vector>

using namespace std;

bool AuthManager::isLoggedIn = false;
User AuthManager::currentUser = {};

int AuthManager::registerUser(const string &login, const string &password,
                              Role role) {
  if (login.empty() || password.empty()) {
    return 1; // 1 - Пустой ввод
  }

  // Проверка на только английские символы (ASCII)
  for (unsigned char c : login)
    if (c > 127)
      return 5;
  for (unsigned char c : password)
    if (c > 127)
      return 5;

  if (password.length() < 8)
    return 2; // 2 - Слабый пароль

  bool hasDigit = false;
  bool hasSpecial = false;
  for (char c : password) {
    if (isdigit((unsigned char)c))
      hasDigit = true;
    if (ispunct((unsigned char)c))
      hasSpecial = true;
  }

  if (!hasDigit || !hasSpecial)
    return 2;

  vector<User> users = Database::loadUsers();

  // Проверка занятли логин
  for (const auto &u : users) {
    if (u.login == login) {
      return 3; // 3 - логин занят
    }
  }

  User newUser;
  newUser.login = login;
  newUser.password = password;

  // Первый пользователь автоматически становится администратором
  if (users.empty()) {
    newUser.role = Role::ADMIN;
  } else {
    newUser.role = role;
  }

  users.push_back(newUser);
  return Database::saveUsers(users) ? 0 : 4; // 0 - успех
}

bool AuthManager::loginUser(const string &login, const string &password) {
  vector<User> users = Database::loadUsers();

  for (const auto &u : users) {
    if (u.login == login && u.password == password) {
      currentUser = u;
      isLoggedIn = true;
      return true;
    }
  }

  return false;
}

void AuthManager::logout() {
  isLoggedIn = false;
  currentUser = {};
}

bool AuthManager::isUserLoggedIn() { return isLoggedIn; }

User AuthManager::getCurrentUser() { return currentUser; }
