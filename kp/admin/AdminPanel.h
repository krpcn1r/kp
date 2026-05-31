#pragma once
#include <string>
#include <vector>

class AdminPanel {
public:
    static void showAdminPanel();

private:
    static void editUser();
    static void deleteUser();
    static void createBackup();
    static void showBackups();
    static void restoreBackup();
};
