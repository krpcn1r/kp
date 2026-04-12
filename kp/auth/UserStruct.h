#pragma once
#include "../core/json.hpp"
#include <string>


enum class Role { ADMIN, OPERATOR };

NLOHMANN_JSON_SERIALIZE_ENUM(Role, {{Role::ADMIN, "ADMIN"},
                                    {Role::OPERATOR, "OPERATOR"}})

struct User {
  std::string login;
  std::string password;
  Role role;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, login, password, role)