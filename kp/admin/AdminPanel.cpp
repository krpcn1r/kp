#include "AdminPanel.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "../core/Render.h"
#include "../core/InputHandler.h"
#include "../core/Database.h"
#include "../auth/AuthManager.h"
#include "../user/HomeMenu.h"

using namespace std;
namespace fs = std::filesystem;

void AdminPanel::showAdminPanel()
{
	int selectedOption = 0;
	const int numOptions = 10;
	bool needFullRedraw = true;

	while (true) {
		if (needFullRedraw) {
			clearScreen();
			drawDoubleBox(2, 1, 76, 28, 11);

			setCursor(30, 3);
			setColor(14);
			cout << "ПАНЕЛЬ АДМИНИСТРАТОРА";

			setColor(11);
			setCursor(2, 5);
			cout << "╠══════════════════════════════════════════════════════════════════════════╣";

			needFullRedraw = false;
		}

		drawFooter(28, true);

		string options[numOptions] = {
			"1. Просмотр текущих пользователей",
			"2. Изменение данных пользователя ",
			"3. Удаление пользователей        ",
			"4. Создать бэкап БД              ",
			"5. Просмотреть бэкапы БД         ",
			"6. Загрузка бэкапов              ",
			"7. Просмотр логов                ",
			"8. Изменение пароля админа       ",
			"9. Настройки                     ",
			"10. Выход                        "
		};

		for (int i = 0; i < numOptions; i++) {
			int yPos = 7 + i;
			
			if (i >= 3) yPos += 1;
			if (i >= 6) yPos += 1;
			
			if (i == 3) {
				setCursor(2, 7 + 3);
				setColor(11);
				cout << "╠══════════════════════════════════════════════════════════════════════════╣";
			}
			if (i == 6) {
				setCursor(2, 7 + 6 + 1);
				setColor(11);
				cout << "╠══════════════════════════════════════════════════════════════════════════╣";
			}

			setCursor(6, yPos);
			if (i == selectedOption) {
				if (i == 9) setColor(12);
				else setColor(10);
				cout << "> " << options[i] << "         ";
			} else {
				if (i == 9) setColor(4);
				else setColor(8);
				cout << "  " << options[i] << "         ";
			}
		}

		int key = InputHandler::getExtKey();

		if (key == Key::TAB || key == Key::DOWN) {
			selectedOption = (selectedOption + 1) % numOptions;
		} else if (key == Key::UP) {
			selectedOption = (selectedOption - 1 + numOptions) % numOptions;
		} else if (key == Key::ENTER) {
			if (selectedOption == 0) showUsersList();
			else if (selectedOption == 1) editUser();
			else if (selectedOption == 2) deleteUser();
			else if (selectedOption == 3) createBackup();
			else if (selectedOption == 4) showBackups();
			else if (selectedOption == 5) restoreBackup();
			else if (selectedOption == 6) showPlaceholder("Просмотр логов");
			else if (selectedOption == 7) HomeMenu::showChangePassword();
			else if (selectedOption == 8) showPlaceholder("Настройки");
			else if (selectedOption == 9) return;
			
			needFullRedraw = true;
		} else if (key == Key::ESC) {
			return;
		}
	}
}

void AdminPanel::showUsersList() {
	clearScreen();
	drawDoubleBox(2, 1, 76, 25, 14);
	setCursor(28, 3);
	setColor(15);
	cout << "СПИСОК ПОЛЬЗОВАТЕЛЕЙ";
	
	setColor(14);
	setCursor(2, 5);
	cout << "╠══════════════════════════════════════════════════════════════════════════╣";
	
	vector<User> users = Database::loadUsers();
	
	setCursor(6, 6);
	setColor(11);
	cout << left << setw(20) << "Логин" << setw(20) << "Роль";
	
	setColor(7);
	int y = 8;
	for (const auto& u : users) {
		setCursor(6, y++);
		string roleStr = (u.role == Role::ADMIN) ? "ADMIN" : "OPERATOR";
		cout << left << setw(20) << u.login << setw(20) << roleStr;
		if (y > 23) break; // ограничение вывода
	}
	
	drawFooter(27, true);
	InputHandler::waitAnyKey();
}

void AdminPanel::editUser() {
	showPlaceholder("Изменение данных пользователя");
}

void AdminPanel::deleteUser() {
	showPlaceholder("Удаление пользователей");
}

void AdminPanel::createBackup() {
	clearScreen();
	drawDoubleBox(10, 8, 60, 10, 10);
	
	try {
		if (!fs::exists("data/backups")) {
			fs::create_directories("data/backups");
		}
		
		auto now = chrono::system_clock::now();
		time_t now_time = chrono::system_clock::to_time_t(now);
		tm local_tm;
		localtime_s(&local_tm, &now_time);
		
		stringstream ss;
		ss << put_time(&local_tm, "%Y-%m-%d_%H-%M-%S");
		string backupFolderName = ss.str();
		
		string backupPath = "data/backups/" + backupFolderName;
		fs::create_directories(backupPath);
		
		if (fs::exists("data/users.json")) fs::copy("data/users.json", backupPath + "/users.json");
		if (fs::exists("data/clients.json")) fs::copy("data/clients.json", backupPath + "/clients.json");
		
		setCursor(26, 12);
		setColor(10);
		cout << "Бэкап успешно создан!";
		setCursor(22, 14);
		setColor(7);
		cout << "Папка: " << backupFolderName;
	} catch (const exception& e) {
		setCursor(26, 12);
		setColor(12);
		cout << "Ошибка создания бэкапа!";
	}
	
	InputHandler::waitAnyKey();
}

void AdminPanel::showBackups() {
	clearScreen();
	drawDoubleBox(2, 1, 76, 25, 14);
	setCursor(32, 3);
	setColor(15);
	cout << "БЭКАПЫ БД";
	
	setColor(14);
	setCursor(2, 5);
	cout << "╠══════════════════════════════════════════════════════════════════════════╣";
	
	int y = 7;
	setColor(7);
	if (fs::exists("data/backups")) {
		for (const auto& entry : fs::directory_iterator("data/backups")) {
			if (entry.is_directory()) {
				setCursor(6, y++);
				cout << entry.path().filename().string();
			}
			if (y > 23) break;
		}
	}
	if (y == 7) {
		setCursor(6, y);
		setColor(8);
		cout << "Бэкапы не найдены.";
	}
	
	drawFooter(27, true);
	InputHandler::waitAnyKey();
}

void AdminPanel::restoreBackup() {
	showPlaceholder("Загрузка бэкапов");
}
