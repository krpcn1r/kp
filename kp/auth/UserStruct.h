#pragma once

enum class Role{
	ADMIN,
	OPERATOR
};

struct User {
	char login[32];
	char password[64];
	Role role;
};