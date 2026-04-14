#pragma once
#include "../user/UserStruct.h"
#include <string>

// класс для управления входом и регистрацией юзеров
class AuthManager {
public:
  // функция для регистрации нового пользователя
  // возврат 0 если всё ок или другой код ошибки
  static int registerUser(const std::string &login, const std::string &password,
                          Role role = Role::OPERATOR);
  
  // проверка логина и пароля для входа в прогу
  static bool loginUser(const std::string &login, const std::string &password);
  
  // сброс данных текущего юзера
  static void logout();

  // проверка статуса входа
  static bool isUserLoggedIn();
  // получение данных того кто сейчас в системе
  static User getCurrentUser();

private:
  static bool isLoggedIn; // статус входа
  static User currentUser; // данные текущего юзера
};
