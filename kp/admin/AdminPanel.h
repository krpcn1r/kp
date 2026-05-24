#pragma once
#include <vector>
#include <string>

class AdminPanel {
public:
	static void showAdminPanel();

private:
	static void showUsersList();
	static void editUser();
	static void deleteUser();
	static void createBackup();
	static void showBackups();
	static void restoreBackup();
	static void runManualBilling();
};
