#include "AdminPanel.h"
#include <iostream>
#include "../core/Render.h"
#include "../core/InputHandler.h"
#include "../core/Database.h"
#include "../auth/AuthManager.h"
#include <vector>

void AdminPanel::showAdminPanel()
{
	clearScreen();
	setColor(4);
	setCursor(5, 5);
	std::cout << "Test admin panel";
	InputHandler::waitAnyKey();
}
