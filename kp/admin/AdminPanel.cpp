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

	drawDoubleBox(1,1,61,25,7);
	setCursor(24, 2);
	setColor(6);
	std::cout << "Admin panel";
	setColor(7);
	setCursor(1, 3);
	std::cout << "╠═══════════════════════════════════════════════════════════╣";




	drawFooter(27);
	InputHandler::waitAnyKey();
}
