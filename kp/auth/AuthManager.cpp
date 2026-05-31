#include "AuthManager.h"
#include "../core/Database.h"
#include "../core/Logger.h"
#include <vector>

using namespace std;

bool AuthManager::isLoggedIn = false;
User AuthManager::currentUser = {};

bool AuthManager::isAsciiOnly(const string& str) {
  for (unsigned char c : str)
    if (c > 127) return false;
  return true;
}

bool AuthManager::isStrongPassword(const string& password) {
  if (password.length() < 8) return false;
  bool hasDigit = false, hasSpecial = false;
  for (char c : password) {
    if (isdigit((unsigned char)c))  hasDigit = true;
    if (ispunct((unsigned char)c))  hasSpecial = true;
  }
  return hasDigit && hasSpecial;
}

// регистрация нового пользователя
int AuthManager::registerUser(const string &login, const string &password,
                              Role role) {
  if (login.empty() || password.empty())
    return 1;
  if (!isAsciiOnly(login) || !isAsciiOnly(password))
    return 5;
  if (!isStrongPassword(password))
    return 2;

  string lowerLogin = login;
  for (char& c : lowerLogin) {
      c = (unsigned char)tolower(c);
  }

  // загрузка списка всех юзеров из базы
  vector<User> users = Database::loadUsers();

  for (const auto& u : users) {
      // Копируем логин пользователя из базы тоже в нижний регистр
      string existingUserLower = u.login;
      for (char& c : existingUserLower) {
          c = (unsigned char)tolower(c);
      }

      if (existingUserLower == lowerLogin) {
          return 3; // Логин занят (даже если регистр разный)
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
  if (!Database::saveUsers(users)) return 4;
  Logger::logAs("system", LogCategory::AUTH, "Регистрация нового пользователя",
                "login=" + login +
                ", role=" + (newUser.role == Role::ADMIN ? "ADMIN" : "OPERATOR"));
  return 0;
}

// функция входа в систему по логину
bool AuthManager::loginUser(const string &login, const string &password) {
  vector<User> users = Database::loadUsers(); // загрузка юзеров

  // поиск совпадения логина и пароля
  for (const auto &u : users) {
    if (u.login == login && u.password == password) {
      currentUser = u; // сохранение данных того кто зашел
      isLoggedIn = true; // установка флага входа
      Logger::log(LogCategory::AUTH, "Успешный вход в систему",
                  "role=" + string(u.role == Role::ADMIN ? "ADMIN" : "OPERATOR"));
      return true;
    }
  }

  Logger::logAs(login.empty() ? "(пусто)" : login, LogCategory::AUTH,
                "Неудачная попытка входа", "неверный логин или пароль");
  return false; // не нашли такого юзера
}

void AuthManager::logout() {
  if (isLoggedIn) {
    Logger::log(LogCategory::AUTH, "Выход из системы");
  }
  isLoggedIn = false;
  currentUser = {};
}

// смена пароля для главного меню
int AuthManager::changePassword(std::string& currentPassword, const std::string& newPassword) {
    if (newPassword.empty() || currentPassword.empty())
        return 3;
    if (!isAsciiOnly(currentPassword) || !isAsciiOnly(newPassword))
        return 6;
    if (newPassword.length() < 8)
        return 4;
    if (!isStrongPassword(newPassword))
        return 5;

    vector<User> users = Database::loadUsers();
    User currentUser = getCurrentUser();
    bool found = false;
    for (auto& u : users) {
        if (u.login == currentUser.login) {
            if (currentPassword != currentUser.password) {
                return 2;
            }
            u.password = newPassword;
            found = true;
        }
    }

    if (found) {
        Database::saveUsers(users);
        Logger::log(LogCategory::AUTH, "Смена пароля",
                    "login=" + currentUser.login);
        return 0;
    }

    return 1;
}

bool AuthManager::isUserLoggedIn() { return isLoggedIn; }

User AuthManager::getCurrentUser() { return currentUser; }
