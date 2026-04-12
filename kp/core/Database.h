#pragma once
#include "../auth/UserStruct.h"
#include "../clients/ClientStruct.h"
#include <string>
#include <vector>

class Database {
public:
	static bool saveUsers(const std::vector<User>& users);
	static std::vector<User> loadUsers();

	static bool saveClients(const std::vector<Client>& clients);
	static std::vector<Client> loadClients();

private:
	static const std::string USERS_FILE;
	static const std::string CLIENTS_FILE;
};