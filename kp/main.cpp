#include <iostream>
#include <string>
#include <windows.h>
#include <conio.h>
#include "admin/AdminPanel.h"
#include "admin/ConfigWizard.h"

#include "auth/AuthManager.h"
#include "auth/Login.h"
#include "auth/Register.h"

#include "clients/Billing.h"
#include "clients/ClientManager.h"
#include "clients/ClientStruct.h"

#include "core/Database.h"
#include "core/InputHandler.h"
#include "core/Render.h"
#include "core/Utils.h"

using namespace std;

int main() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	drawTextBox(1, 1, 30, 10, "Привет", 7, 10, 2);
	_getch();
	return 0;
}
