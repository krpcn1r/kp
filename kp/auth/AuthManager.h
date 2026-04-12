#pragma once
#include <string>
#include "UserStruct.h"

class AuthManager {
public:
    // Возвращает код статуса (0 - успех, 1 - пустой ввод, 2 - слабый пароль, 3 - логин занят, 5 - не англ. символы)
    static int registerUser(const std::string& login, const std::string& password, Role role = Role::OPERATOR);
    static bool loginUser(const std::string& login, const std::string& password);
    static void logout();
    
    static bool isUserLoggedIn();
    static User getCurrentUser();

private:
    static bool isLoggedIn;
    static User currentUser;
};
