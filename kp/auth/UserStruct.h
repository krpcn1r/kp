#pragma once
#include <string>
#include "../core/json.hpp"

enum class Role{
	ADMIN,
	OPERATOR
};

// Макрос для автоматической конвертации Role в строку JSON
NLOHMANN_JSON_SERIALIZE_ENUM(Role, {
    {Role::ADMIN, "ADMIN"},
    {Role::OPERATOR, "OPERATOR"}
})

struct User {
	std::string login;
	std::string password;
	Role role;
};

// Макрос для автоматического сохранения/чтения User в/из JSON
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, login, password, role)