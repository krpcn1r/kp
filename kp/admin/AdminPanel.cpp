#include "AdminPanel.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "../core/Render.h"
#include "../core/InputHandler.h"
#include "../core/Database.h"
#include "../core/Logger.h"
#include "../auth/AuthManager.h"
#include "../user/HomeMenu.h"
#include "../clients/Billing.h"
#include "LogViewer.h"

using namespace std;

static const int USER_TABLE_PAGE_SIZE = 8;
static const int USER_ROW_START_Y = 7;
static const int USER_ROW_STEP = 2;
static const int USER_ROW_CLEAR_WIDTH = 70;
static const int USER_NUMBER_X = 3;
static const int USER_NUMBER_WIDTH = 4;
static const int USER_LOGIN_X = 10;
static const int USER_PASSWORD_X = 34;
static const int USER_ROLE_X = 58;
static const int USER_LOGIN_WIDTH = 20;
static const int USER_PASSWORD_WIDTH = 20;
static const int USER_ROLE_WIDTH = 14;
static const int USER_LOGIN_SEPARATOR_X = 8;
static const int USER_PASSWORD_SEPARATOR_X = 32;
static const int USER_ROLE_SEPARATOR_X = 56;
static const int BACKUP_PAGE_SIZE = 8;

static string toLowerAscii(string value) {
	for (char& c : value) {
		c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
	}
	return value;
}

static bool isAsciiText(const string& value) {
	for (unsigned char c : value) {
		if (c > 127) return false;
	}
	return true;
}

static bool isStrongPassword(const string& password) {
	if (password.length() < 8) return false;

	bool hasDigit = false;
	bool hasSpecial = false;
	for (char c : password) {
		if (isdigit(static_cast<unsigned char>(c))) hasDigit = true;
		if (ispunct(static_cast<unsigned char>(c))) hasSpecial = true;
	}
	return hasDigit && hasSpecial;
}

static string roleToText(Role role) {
	return role == Role::ADMIN ? "ADMIN" : "OPERATOR";
}

static void toggleUserRole(User& user) {
	user.role = user.role == Role::ADMIN ? Role::OPERATOR : Role::ADMIN;
}

static string maskedPassword(const string& password) {
	return string(std::min<size_t>(password.length(), USER_PASSWORD_WIDTH - 2), '*');
}

static int visibleUserRowY(int index, int startIdx) {
	return USER_ROW_START_Y + (index - startIdx) * USER_ROW_STEP;
}

static bool hasAnotherAdmin(const vector<User>& users, int ignoredIdx) {
	for (int i = 0; i < (int)users.size(); ++i) {
		if (i != ignoredIdx && users[i].role == Role::ADMIN) {
			return true;
		}
	}
	return false;
}

static void drawUsersTable(const string& title, const vector<User>& users,
						int startIdx, int selectedIdx,
						int editingIdx, int activeField, const User& draftUser,
						const string& message, int messageColor, bool fullRedraw,
						const string& defaultHint, const string& editHint,
						const string& statusText) {
	vector<TableColumn> columns = {
		{USER_NUMBER_X, USER_NUMBER_WIDTH, "N"},
		{USER_LOGIN_X, USER_LOGIN_WIDTH, "Логин"},
		{USER_PASSWORD_X, USER_PASSWORD_WIDTH, "Пароль"},
		{USER_ROLE_X, USER_ROLE_WIDTH, "Роль"}
	};
	vector<int> separators = {
		USER_LOGIN_SEPARATOR_X,
		USER_PASSWORD_SEPARATOR_X,
		USER_ROLE_SEPARATOR_X
	};

	if (fullRedraw) {
		clearScreen();
		drawBox(1, 1, 90, 28, 14);

		setCursor(34 - (int)title.length() / 4, 2);
		setColor(11);
		cout << title;

		drawTableHeader(4, columns, separators, 15);
		drawTableSeparator(2, 5, USER_ROW_CLEAR_WIDTH, separators, 15);
	}

	for (int i = 0; i < USER_TABLE_PAGE_SIZE; ++i) {
		int curIdx = startIdx + i;
		int y = USER_ROW_START_Y + i * USER_ROW_STEP;
		bool hasUser = curIdx < (int)users.size();
		bool isSelected = hasUser && curIdx == selectedIdx;
		bool isEditing = hasUser && curIdx == editingIdx;
		int rowColor = isSelected ? 240 : 7;

		clearLine(2, y, USER_ROW_CLEAR_WIDTH, rowColor);

		if (hasUser) {
			const User& rowUser = isEditing ? draftUser : users[curIdx];

			drawTableCell(USER_NUMBER_X, y, USER_NUMBER_WIDTH, to_string(curIdx + 1), rowColor);
			drawTableCell(USER_LOGIN_SEPARATOR_X, y, 1, "|", rowColor);

			if (isEditing && activeField == 0) {
				drawInputContent(USER_LOGIN_X, y, USER_LOGIN_WIDTH, draftUser.login, false, true);
			} else {
				drawTableCell(USER_LOGIN_X, y, USER_LOGIN_WIDTH, rowUser.login, rowColor);
			}

			drawTableCell(USER_PASSWORD_SEPARATOR_X, y, 1, "|", rowColor);
			if (isEditing && activeField == 1) {
				drawInputContent(USER_PASSWORD_X, y, USER_PASSWORD_WIDTH, draftUser.password, true, true);
			} else {
				drawTableCell(USER_PASSWORD_X, y, USER_PASSWORD_WIDTH, maskedPassword(rowUser.password), rowColor);
			}

			drawTableCell(USER_ROLE_SEPARATOR_X, y, 1, "|", rowColor);
			if (isEditing) {
				drawTableCell(USER_ROLE_X, y, USER_ROLE_WIDTH, roleToText(draftUser.role),
							  activeField == 2 ? 31 : rowColor);
			} else {
				drawTableCell(USER_ROLE_X, y, USER_ROLE_WIDTH, roleToText(rowUser.role), rowColor);
			}
		}

		drawTableSeparator(2, y + 1, USER_ROW_CLEAR_WIDTH, separators, 8);
	}

	clearLine(3, 24, 84);
	setCursor(3, 24);
	setColor(messageColor);
	if (!message.empty()) {
		cout << left << setw(78) << message;
	} else if (users.empty()) {
		cout << "Пользователи не найдены.";
	} else if (editingIdx >= 0) {
		cout << editHint;
	} else {
		cout << defaultHint;
	}

	clearLine(3, 27, 84);
	setCursor(3, 27);
	setColor(8);
	cout << statusText;

	clearLine(2, 29, 86);
	drawFooter(29, true);
}

static bool validateUserEdit(const vector<User>& users, int editIdx, const User& draft,
					  string& message, int& activeField) {
	if (draft.login.empty() || draft.password.empty()) {
		message = "Ошибка: логин и пароль не должны быть пустыми.";
		activeField = draft.login.empty() ? 0 : 1;
		return false;
	}

	if (!isAsciiText(draft.login) || !isAsciiText(draft.password)) {
		message = "Ошибка: используйте только английские символы.";
		activeField = !isAsciiText(draft.login) ? 0 : 1;
		return false;
	}

	if (!isStrongPassword(draft.password)) {
		message = "Ошибка: пароль минимум 8 символов, 1 цифра и 1 спецсимвол.";
		activeField = 1;
		return false;
	}

	string originalLogin = toLowerAscii(users[editIdx].login);
	string newLogin = toLowerAscii(draft.login);
	if (newLogin != originalLogin) {
		for (int i = 0; i < (int)users.size(); ++i) {
			if (i != editIdx && toLowerAscii(users[i].login) == newLogin) {
				message = "Ошибка: такой логин уже занят.";
				activeField = 0;
				return false;
			}
		}
	}

	if (draft.role != Role::ADMIN) {
		if (!hasAnotherAdmin(users, editIdx)) {
			message = "Ошибка: в системе должен остаться хотя бы один администратор.";
			activeField = 2;
			return false;
		}
	}

	return true;
}

static vector<string> loadBackupNames() {
	vector<string> backups;
	if (!std::filesystem::exists("data/backups")) {
		return backups;
	}

	for (const auto& entry : std::filesystem::directory_iterator("data/backups")) {
		if (entry.is_directory()) {
			backups.push_back(entry.path().filename().string());
		}
	}

	sort(backups.begin(), backups.end());
	reverse(backups.begin(), backups.end());
	return backups;
}

static void drawBackupsList(const string& title, const vector<string>& backups,
							int startIdx, int selectedIdx,
							const string& message, int messageColor, bool fullRedraw) {
	vector<TableColumn> cols = {
		{5, 4, "N"},
		{14, 50, "Папка бэкапа"}
	};
	vector<int> seps = { 11 };

	if (fullRedraw) {
		clearScreen();
		drawBox(1, 1, 90, 28, 14);

		setCursor(34 - (int)title.length() / 4, 2);
		setColor(11);
		cout << title;

		drawTableHeader(4, cols, seps, 15);
		drawTableSeparator(2, 5, 70, seps, 15);
	}

	for (int i = 0; i < BACKUP_PAGE_SIZE; i++) {
		int idx = startIdx + i;
		int y = 7 + i * 2;
		bool exists = idx < (int)backups.size();
		bool selected = exists && idx == selectedIdx;
		int color = selected ? 240 : 7;

		clearLine(2, y, 70, color);
		if (exists) {
			drawTableCell(5, y, 4, to_string(idx + 1), color);
			drawTableCell(11, y, 1, "|", 15);
			drawTableCell(14, y, 50, backups[idx], color);
		}
		drawTableSeparator(2, y + 1, 70, seps, 8);
	}

	clearLine(3, 24, 84);
	setCursor(3, 24);
	setColor(messageColor);
	if (!message.empty()) {
		cout << message;
	} else if (backups.empty()) {
		cout << "Бэкапы не найдены.";
	}

	clearLine(3, 27, 84);
	setCursor(3, 27);
	setColor(8);
	cout << "Всего бэкапов: " << backups.size();

	clearLine(2, 29, 86);
	drawFooter(29, true);
}

void AdminPanel::showAdminPanel()
{
	int selectedOption = 0;
	const int numOptions = 10;
	bool needFullRedraw = true;

	while (true) {
		if (needFullRedraw) {
			clearScreen();
			drawBox(2, 1, 76, 20, 11);

			setCursor(30, 3);
			setColor(14);
			cout << "ПАНЕЛЬ АДМИНИСТРАТОРА";

			setColor(11);
			setCursor(2, 5);
			cout << "+";
			for (int i = 0; i < 74; i++) cout << "-";
			cout << "+";

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
			"9. Списать оплату за день вручную",
			"10. Выход                        "
		};

		for (int i = 0; i < numOptions; i++) {
			int yPos = 7 + i;
			
			if (i >= 3) yPos += 1;
			if (i >= 6) yPos += 1;
			
			if (i == 3) {
				setCursor(2, 7 + 3);
				setColor(11);
				{ cout << "+"; for (int j = 0; j < 74; j++) cout << "-"; cout << "+"; }
			}
			if (i == 6) {
				setCursor(2, 7 + 6 + 1);
				setColor(11);
				{ cout << "+"; for (int j = 0; j < 74; j++) cout << "-"; cout << "+"; }
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
			else if (selectedOption == 6) LogViewer::show();
			else if (selectedOption == 7) HomeMenu::showChangePassword();
			else if (selectedOption == 8) AdminPanel::runManualBilling();
			else if (selectedOption == 9) return;
			
			needFullRedraw = true;
		} else if (key == Key::ESC) {
			return;
		}
	}
}

void AdminPanel::showUsersList() {
	vector<User> users = Database::loadUsers();
	int selectedIdx = users.empty() ? -1 : 0;
	int startIdx = 0;
	User emptyUser = {};
	bool needFullRedraw = true;

	while (true) {
		string statusText = "Всего пользователей: " + to_string(users.size()) +
							"  |  Просмотр списка";

		drawUsersTable("СПИСОК ПОЛЬЗОВАТЕЛЕЙ", users, startIdx, selectedIdx,
					   -1, 0, emptyUser, "", 8, needFullRedraw,
					   "", "", statusText);
		needFullRedraw = false;

		int key = InputHandler::getExtKey();
		if (key == Key::ESC) return;
		if (users.empty()) continue;

		if (key == Key::DOWN || key == Key::TAB) {
			if (selectedIdx < (int)users.size() - 1) {
				selectedIdx++;
				if (selectedIdx >= startIdx + USER_TABLE_PAGE_SIZE) startIdx++;
			}
		} else if (key == Key::UP) {
			if (selectedIdx > 0) {
				selectedIdx--;
				if (selectedIdx < startIdx) startIdx--;
			}
		}
	}
}

void AdminPanel::editUser() {
	vector<User> users = Database::loadUsers();
	int selectedIdx = users.empty() ? -1 : 0;
	int startIdx = 0;
	int editingIdx = -1;
	int activeField = 0;
	User draftUser = {};
	string message = "";
	int messageColor = 8;
	bool needFullRedraw = true;

	while (true) {
		// выравниваем selectedIdx и startIdx по границам
		if (users.empty()) {
			selectedIdx = -1;
			startIdx = 0;
		} else {
			if (selectedIdx < 0) selectedIdx = 0;
			if (selectedIdx >= (int)users.size()) selectedIdx = (int)users.size() - 1;
			if (selectedIdx < startIdx) startIdx = selectedIdx;
			if (selectedIdx >= startIdx + USER_TABLE_PAGE_SIZE)
				startIdx = selectedIdx - USER_TABLE_PAGE_SIZE + 1;
		}

		string statusText;
		if (editingIdx >= 0) {
			statusText = "Всего пользователей: " + to_string(users.size()) +
						 "  |  Режим: редактирование строки " + to_string(editingIdx + 1);
		} else {
			statusText = "Всего пользователей: " + to_string(users.size()) +
						 "  |  Выбран: " + to_string(users.empty() ? 0 : selectedIdx + 1);
		}

		drawUsersTable("ИЗМЕНЕНИЕ ПОЛЬЗОВАТЕЛЕЙ", users, startIdx, selectedIdx,
					   editingIdx, activeField, draftUser, message, messageColor,
					   needFullRedraw,
					   "Enter - редактировать выбранную строку. Esc - назад.",
					   "Tab - поле. Space/стрелки/A/O - роль. Enter - сохранить. Esc - отменить.",
					   statusText);
		needFullRedraw = false;

		// режим выбора пользователя
		if (editingIdx == -1) {
			int key = InputHandler::getExtKey();
			if (key == Key::ESC) return;
			if (users.empty()) continue;

			if (key == Key::DOWN || key == Key::TAB) {
				if (selectedIdx < (int)users.size() - 1) selectedIdx++;
			} else if (key == Key::UP) {
				if (selectedIdx > 0) selectedIdx--;
			} else if (key == Key::ENTER) {
				editingIdx = selectedIdx;
				draftUser = users[editingIdx];
				activeField = 0;
				message = "";
				messageColor = 8;
			}
			continue;
		}

		// режим редактирования — поля логин и пароль
		if (activeField == 0 || activeField == 1) {
			int rowY = visibleUserRowY(editingIdx, startIdx);
			int exitKey = 0;

			if (activeField == 0) {
				draftUser.login = processInput(USER_LOGIN_X, rowY, USER_LOGIN_WIDTH,
											   draftUser.login, false, exitKey, 0);
			} else {
				draftUser.password = processInput(USER_PASSWORD_X, rowY, USER_PASSWORD_WIDTH,
												  draftUser.password, true, exitKey, 0);
			}

			message = "";
			messageColor = 8;

			if (exitKey == Key::TAB || exitKey == Key::DOWN) {
				activeField = (activeField + 1) % 3;
			} else if (exitKey == Key::UP) {
				activeField = (activeField + 2) % 3;
			} else if (exitKey == Key::ENTER) {
				activeField++;
			} else if (exitKey == Key::ESC) {
				editingIdx = -1;
				activeField = 0;
				message = "Редактирование отменено.";
				messageColor = 8;
			}
			continue;
		}

		// режим редактирования — поле роли
		int key = InputHandler::getExtKey();
		message = "";
		messageColor = 8;

		if (key == Key::TAB || key == Key::DOWN) {
			activeField = (activeField + 1) % 3;
		} else if (key == Key::UP) {
			activeField = (activeField + 2) % 3;
		} else if (key == Key::LEFT || key == Key::RIGHT || key == ' ') {
			toggleUserRole(draftUser);
		} else if (key == 'a' || key == 'A') {
			draftUser.role = Role::ADMIN;
		} else if (key == 'o' || key == 'O') {
			draftUser.role = Role::OPERATOR;
		} else if (key == Key::ENTER) {
			string validationMsg;
			int invalidField = activeField;
			if (!validateUserEdit(users, editingIdx, draftUser, validationMsg, invalidField)) {
				message = validationMsg;
				messageColor = 12;
				activeField = invalidField;
			} else {
				User before = users[editingIdx];
				vector<User> updatedUsers = users;
				updatedUsers[editingIdx] = draftUser;
				if (!Database::saveUsers(updatedUsers)) {
					message = "Ошибка: не удалось сохранить пользователей.";
					messageColor = 12;
				} else {
					string det = "login=" + before.login;
					if (before.login != draftUser.login)
						det += "; логин: " + before.login + " -> " + draftUser.login;
					if (before.password != draftUser.password)
						det += "; пароль изменён";
					if (before.role != draftUser.role)
						det += string("; роль: ") + roleToText(before.role) + " -> " + roleToText(draftUser.role);
					Logger::log(LogCategory::USER, "Изменён пользователь", det);
					users = Database::loadUsers();
					selectedIdx = editingIdx;
					editingIdx = -1;
					activeField = 0;
					message = "Пользователь успешно изменен.";
					messageColor = 10;
				}
			}
		} else if (key == Key::ESC) {
			editingIdx = -1;
			activeField = 0;
			message = "Редактирование отменено.";
			messageColor = 8;
		}
	}
}

void AdminPanel::deleteUser() {
	vector<User> users = Database::loadUsers();
	int selectedIdx = users.empty() ? -1 : 0;
	int startIdx = 0;
	User emptyUser = {};
	string message = "";
	int messageColor = 8;
	bool needFullRedraw = true;

	while (true) {
		if (users.empty()) {
			selectedIdx = -1;
			startIdx = 0;
		} else {
			if (selectedIdx < 0) selectedIdx = 0;
			if (selectedIdx >= (int)users.size()) selectedIdx = (int)users.size() - 1;
			if (selectedIdx < startIdx) startIdx = selectedIdx;
			if (selectedIdx >= startIdx + USER_TABLE_PAGE_SIZE) {
				startIdx = selectedIdx - USER_TABLE_PAGE_SIZE + 1;
			}
		}

		string statusText = "Всего пользователей: " + to_string(users.size()) +
							"  |  Выбран: " + to_string(users.empty() ? 0 : selectedIdx + 1);

		drawUsersTable("УДАЛЕНИЕ ПОЛЬЗОВАТЕЛЕЙ", users, startIdx, selectedIdx,
					   -1, 0, emptyUser, message, messageColor, needFullRedraw,
					   "", "", statusText);
		needFullRedraw = false;

		int key = InputHandler::getExtKey();
		message = "";
		messageColor = 8;

		if (key == Key::ESC) return;
		if (users.empty()) continue;

		if (key == Key::DOWN || key == Key::TAB) {
			if (selectedIdx < (int)users.size() - 1) {
				selectedIdx++;
				if (selectedIdx >= startIdx + USER_TABLE_PAGE_SIZE) startIdx++;
			}
		} else if (key == Key::UP) {
			if (selectedIdx > 0) {
				selectedIdx--;
				if (selectedIdx < startIdx) startIdx--;
			}
		} else if (key == Key::ENTER) {
			if (users[selectedIdx].role == Role::ADMIN && !hasAnotherAdmin(users, selectedIdx)) {
				message = "Ошибка: нельзя удалить последнего администратора.";
				messageColor = 12;
				continue;
			}

			string login = users[selectedIdx].login;
			if (!showConfirmation("Удалить пользователя " + login + "?")) {
				needFullRedraw = true;
				message = "Удаление отменено.";
				messageColor = 8;
				continue;
			}

			User snapshot = users[selectedIdx];
			vector<User> updatedUsers = users;
			updatedUsers.erase(updatedUsers.begin() + selectedIdx);

			if (!Database::saveUsers(updatedUsers)) {
				needFullRedraw = true;
				message = "Ошибка: не удалось сохранить изменения.";
				messageColor = 12;
				continue;
			}

			Logger::log(LogCategory::USER, "Удалён пользователь",
			            "login=" + snapshot.login + ", role=" + roleToText(snapshot.role));

			users = Database::loadUsers();
			if (users.empty()) {
				selectedIdx = -1;
				startIdx = 0;
			} else if (selectedIdx >= (int)users.size()) {
				selectedIdx = (int)users.size() - 1;
			}

			needFullRedraw = true;
			message = "Пользователь " + login + " удален.";
			messageColor = 10;
		}
	}
}

void AdminPanel::createBackup() {
	clearScreen();
	drawBox(1, 1, 90, 20, 10);

	setCursor(34, 2);
	setColor(11);
	cout << "СОЗДАНИЕ БЭКАПА";

	vector<TableColumn> columns = {
		{6, 18, "Параметр"},
		{28, 45, "Значение"}
	};
	vector<int> separators = { 25 };

	drawTableHeader(5, columns, separators, 15);
	drawTableSeparator(4, 6, 72, separators, 15);

	string backupFolderName = "-";
	string resultText = "Бэкап успешно создан";
	int resultColor = 10;

	try {
		if (!std::filesystem::exists("data/backups")) {
			std::filesystem::create_directories("data/backups");
		}

		auto now = chrono::system_clock::now();
		time_t now_time = chrono::system_clock::to_time_t(now);
		tm local_tm;
		localtime_s(&local_tm, &now_time);

		stringstream ss;
		ss << put_time(&local_tm, "%Y-%m-%d_%H-%M-%S");
		backupFolderName = ss.str();

		string backupPath = "data/backups/" + backupFolderName;
		std::filesystem::create_directories(backupPath);

		if (std::filesystem::exists("data/users.json")) {
			std::filesystem::copy("data/users.json", backupPath + "/users.json",
								  std::filesystem::copy_options::overwrite_existing);
		}
		if (std::filesystem::exists("data/clients.json")) {
			std::filesystem::copy("data/clients.json", backupPath + "/clients.json",
								  std::filesystem::copy_options::overwrite_existing);
		}
		if (std::filesystem::exists("data/tariffs.json")) {
			std::filesystem::copy("data/tariffs.json", backupPath + "/tariffs.json",
								  std::filesystem::copy_options::overwrite_existing);
		}
	} catch (const exception&) {
		resultText = "Ошибка создания бэкапа";
		resultColor = 12;
	}

	if (resultColor == 10) {
		Logger::log(LogCategory::SYSTEM, "Создан бэкап БД", "папка=" + backupFolderName);
	} else {
		Logger::log(LogCategory::SYSTEM, "Ошибка создания бэкапа БД");
	}

	clearLine(4, 7, 72, 7);
	drawTableCell(6, 7, 18, "Статус", resultColor);
	drawTableCell(25, 7, 1, "|", 15);
	drawTableCell(28, 7, 45, resultText, resultColor);
	drawTableSeparator(4, 8, 72, separators, 8);

	clearLine(4, 9, 72, 7);
	drawTableCell(6, 9, 18, "Папка", 7);
	drawTableCell(25, 9, 1, "|", 15);
	drawTableCell(28, 9, 45, backupFolderName, 7);
	drawTableSeparator(4, 10, 72, separators, 8);

	setCursor(6, 14);
	setColor(8);
	cout << "Нажмите любую клавишу для возврата...";

	InputHandler::waitAnyKey();
	return;
}

void AdminPanel::showBackups() {
	vector<string> backups = loadBackupNames();
	int startIdx = 0;
	bool needFullRedraw = true;

	while (true) {
		drawBackupsList("БЭКАПЫ БД", backups, startIdx, -1, "", 8, needFullRedraw);
		needFullRedraw = false;

		int key = InputHandler::getExtKey();
		if (key == Key::ESC) return;
		if (backups.empty()) continue;

		if (key == Key::DOWN || key == Key::TAB) {
			if (startIdx + BACKUP_PAGE_SIZE < (int)backups.size()) startIdx++;
		} else if (key == Key::UP) {
			if (startIdx > 0) startIdx--;
		}
	}
}

void AdminPanel::restoreBackup() {
	vector<string> backups = loadBackupNames();
	int selectedIdx = backups.empty() ? -1 : 0;
	int startIdx = 0;
	string message = "";
	int messageColor = 8;
	bool needFullRedraw = true;

	while (true) {
		drawBackupsList("ВОССТАНОВЛЕНИЕ БЭКАПА", backups, startIdx, selectedIdx,
						message, messageColor, needFullRedraw);
		needFullRedraw = false;

		int key = InputHandler::getExtKey();
		message = "";
		messageColor = 8;

		if (key == Key::ESC) return;
		if (backups.empty()) continue;

		if (key == Key::DOWN || key == Key::TAB) {
			if (selectedIdx < (int)backups.size() - 1) {
				selectedIdx++;
				if (selectedIdx >= startIdx + BACKUP_PAGE_SIZE) startIdx++;
			}
		} else if (key == Key::UP) {
			if (selectedIdx > 0) {
				selectedIdx--;
				if (selectedIdx < startIdx) startIdx--;
			}
		} else if (key == Key::ENTER) {
			string name = backups[selectedIdx];
			if (!showConfirmation("Восстановить бэкап " + name + "?")) {
				needFullRedraw = true;
				message = "Восстановление отменено.";
				messageColor = 8;
				continue;
			}

			try {
				string path = "data/backups/" + name;
				if (filesystem::exists(path + "/users.json"))
					filesystem::copy(path + "/users.json", "data/users.json",
									 filesystem::copy_options::overwrite_existing);
				if (filesystem::exists(path + "/clients.json"))
					filesystem::copy(path + "/clients.json", "data/clients.json",
									 filesystem::copy_options::overwrite_existing);
				if (filesystem::exists(path + "/tariffs.json"))
					filesystem::copy(path + "/tariffs.json", "data/tariffs.json",
									 filesystem::copy_options::overwrite_existing);
				message = "Бэкап " + name + " успешно восстановлен.";
				messageColor = 10;
				Logger::log(LogCategory::SYSTEM, "Восстановлен бэкап БД", "папка=" + name);
			} catch (const exception&) {
				message = "Ошибка: не удалось восстановить бэкап.";
				messageColor = 12;
				Logger::log(LogCategory::SYSTEM, "Ошибка восстановления бэкапа", "папка=" + name);
			}
			needFullRedraw = true;
		}
	}
}


void AdminPanel::runManualBilling() {
	BillingResult res = Billing::chargeOneDay();

	clearScreen();
	drawBox(15, 8, 50, 11, 14);

	setCursor(28, 9);
	setColor(11);
	cout << "РУЧНАЯ ТАРИФИКАЦИЯ";

	setColor(8);
	setCursor(16, 10);
	cout << "+";
	for (int i = 0; i < 48; i++) cout << "-";
	cout << "+";

	setColor(7);
	setCursor(18, 12);
	cout << "Списано за: 1 день";
	setCursor(18, 13);
	cout << "Затронуто клиентов: " << res.clientsTouched;
	setCursor(18, 14);
	cout << fixed << setprecision(2);
	cout << "Сумма списания:     " << res.totalCharged << " руб.";

	setCursor(18, 16);
	setColor(8);
	cout << "Нажмите любую клавишу для возврата...";

	InputHandler::waitAnyKey();
}
