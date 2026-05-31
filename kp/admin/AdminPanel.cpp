#include "AdminPanel.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../auth/AuthManager.h"
#include "../core/Database.h"
#include "../core/InputHandler.h"
#include "../core/Logger.h"
#include "../core/Render.h"
#include "../user/HomeMenu.h"
#include "LogViewer.h"

using namespace std;

// вариант 8: общая корректировка скролла для таблиц пользователей
static void clampScroll(int& selectedIdx, int& startIdx, int size) {
    if (size == 0) {
        selectedIdx = -1;
        startIdx = 0;
        return;
    }
    if (selectedIdx < 0) {
        selectedIdx = 0;
    }
    if (selectedIdx >= size) {
        selectedIdx = size - 1;
    }
    if (selectedIdx < startIdx) {
        startIdx = selectedIdx;
    }
    if (selectedIdx >= startIdx + 8) {
        startIdx = selectedIdx - 8 + 1;
    }
}

static void drawUsersTable(const string& title, const vector<User>& users, int startIdx, int selectedIdx, int editingIdx, int activeField, const User& draftUser, const string& message, int messageColor, bool fullRedraw, const string& defaultHint, const string& editHint, const string& statusText) {
    vector<TableColumn> columns = {
        {3, 4, "N"},
        {10, 20, "Логин"},
        {34, 20, "Пароль"},
        {58, 14, "Роль"}};
    vector<int> separators = {8, 32, 56};

    if (fullRedraw) {
        clearScreen();
        drawBox(1, 1, 90, 28, 14);

        setCursor(34 - (int)title.length() / 4, 2);
        setColor(11);
        cout << title;

        drawTableHeader(4, columns, separators, 15);
        drawTableSeparator(2, 5, 70, separators, 15);
    }

    for (int i = 0; i < 8; ++i) {
        int curIdx = startIdx + i;
        int y = 7 + i * 2;
        bool hasUser = curIdx < (int)users.size();
        bool isSelected = hasUser && curIdx == selectedIdx;
        bool isEditing = hasUser && curIdx == editingIdx;
        int rowColor = isSelected ? 240 : 7;

        clearLine(2, y, 70, rowColor);

        if (hasUser) {
            const User& rowUser = isEditing ? draftUser : users[curIdx];

            drawTableCell(3, y, 4, to_string(curIdx + 1), rowColor);
            drawTableCell(8, y, 1, "|", rowColor);

            if (isEditing && activeField == 0) {
                drawInputContent(10, y, 20, draftUser.login, false, true);
            } else {
                drawTableCell(10, y, 20, rowUser.login, rowColor);
            }

            drawTableCell(32, y, 1, "|", rowColor);

            if (isEditing && activeField == 1) {
                drawInputContent(34, y, 20, draftUser.password, true, true);
            } else {
                size_t maskLen = rowUser.password.length();
                if (maskLen > 18) {
                    maskLen = 18;
                }
                drawTableCell(34, y, 20, string(maskLen, '*'), rowColor);
            }

            drawTableCell(56, y, 1, "|", rowColor);

            if (isEditing) {
                string roleStr = (draftUser.role == Role::ADMIN) ? "ADMIN" : "OPERATOR";
                int roleColor = (activeField == 2) ? 31 : rowColor;
                drawTableCell(58, y, 14, roleStr, roleColor);
            } else {
                string roleStr = (rowUser.role == Role::ADMIN) ? "ADMIN" : "OPERATOR";
                drawTableCell(58, y, 14, roleStr, rowColor);
            }
        }

        drawTableSeparator(2, y + 1, 70, separators, 8);
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

static bool validateUserEdit(const vector<User>& users, int editIdx, const User& draft, string& message, int& activeField) {
    if (draft.login.empty() || draft.password.empty()) {
        message = "Ошибка: логин и пароль не должны быть пустыми.";
        activeField = draft.login.empty() ? 0 : 1;
        return false;
    }

    if (!AuthManager::isAsciiOnly(draft.login) || !AuthManager::isAsciiOnly(draft.password)) {
        message = "Ошибка: используйте только английские символы.";
        activeField = !AuthManager::isAsciiOnly(draft.login) ? 0 : 1;
        return false;
    }

    if (!AuthManager::isStrongPassword(draft.password)) {
        message = "Ошибка: пароль минимум 8 символов, 1 цифра и 1 спецсимвол.";
        activeField = 1;
        return false;
    }

    string newLoginLower = draft.login;
    for (char& c : newLoginLower) {
        c = (char)tolower((unsigned char)c);
    }
    for (int i = 0; i < (int)users.size(); ++i) {
        if (i == editIdx) {
            continue;
        }
        string existingLower = users[i].login;
        for (char& c : existingLower) {
            c = (char)tolower((unsigned char)c);
        }
        if (existingLower == newLoginLower) {
            message = "Ошибка: такой логин уже занят.";
            activeField = 0;
            return false;
        }
    }

    if (draft.role != Role::ADMIN) {
        bool otherAdminExists = false;
        for (int i = 0; i < (int)users.size(); ++i) {
            if (i != editIdx && users[i].role == Role::ADMIN) {
                otherAdminExists = true;
                break;
            }
        }
        if (!otherAdminExists) {
            message = "Ошибка: в системе должен остаться хотя бы один администратор.";
            activeField = 2;
            return false;
        }
    }

    return true;
}

// вариант 3: сортировка в обратном порядке за один вызов
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

    sort(backups.rbegin(), backups.rend());
    return backups;
}

static void drawBackupsList(const string& title, const vector<string>& backups, int startIdx, int selectedIdx, const string& message, int messageColor) {
    vector<TableColumn> cols = {
        {5, 4, "N"},
        {14, 50, "Папка бэкапа"}};
    vector<int> seps = {11};

    clearScreen();
    drawBox(1, 1, 90, 28, 14);

    setCursor(34 - (int)title.length() / 4, 2);
    setColor(11);
    cout << title;

    drawTableHeader(4, cols, seps, 15);
    drawTableSeparator(2, 5, 70, seps, 15);

    for (int i = 0; i < 8; i++) {
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

void AdminPanel::showAdminPanel() {
    int selectedOption = 0;
    const int numOptions = 9;
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
            for (int i = 0; i < 74; i++) {
                cout << "-";
            }
            cout << "+";

            needFullRedraw = false;
        }

        drawFooter(28, true);

        string options[numOptions] = {
            "1. Просмотр/изменение пользователей",
            "2. Удаление пользователей           ",
            "3. Создать бэкап БД                 ",
            "4. Просмотреть бэкапы БД            ",
            "5. Загрузка бэкапов                 ",
            "6. Просмотр логов                   ",
            "7. Изменение пароля админа          ",
            "8. Выход                            "};

        // вариант 4: разделители рисуем до цикла, не внутри него
        setColor(11);
        setCursor(2, 10);
        cout << "+";
        for (int i = 0; i < 74; i++) {
            cout << "-";
        }
        cout << "+";

        // вариант 5: цвет и префикс вычисляем один раз, выводим одним блоком
        for (int i = 0; i < numOptions; i++) {
            int yPos = 7 + i;
            if (i >= 3) {
                yPos++;
            }

            bool selected = (i == selectedOption);
            bool isExit = (i == numOptions - 1);

            int color;
            if (selected) {
                color = isExit ? 12 : 10;
            } else {
                color = isExit ? 4 : 8;
            }

            setCursor(6, yPos);
            setColor(color);
            cout << (selected ? "> " : "  ") << options[i] << "   ";
        }

        int key = InputHandler::getExtKey();

        if (key == Key::TAB || key == Key::DOWN) {
            selectedOption = (selectedOption + 1) % numOptions;
        } else if (key == Key::UP) {
            selectedOption = (selectedOption - 1 + numOptions) % numOptions;
        } else if (key == Key::ENTER) {
            // вариант 7: showUsersList убран, пункт 1 ведёт в editUser
            if (selectedOption == 0) {
                editUser();
            } else if (selectedOption == 1) {
                deleteUser();
            } else if (selectedOption == 2) {
                createBackup();
            } else if (selectedOption == 3) {
                showBackups();
            } else if (selectedOption == 4) {
                restoreBackup();
            } else if (selectedOption == 5) {
                LogViewer::show();
            } else if (selectedOption == 6) {
                HomeMenu::showChangePassword();
            } else if (selectedOption == 7) {
                return;
            }

            needFullRedraw = true;
        } else if (key == Key::ESC) {
            return;
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
        // вариант 8: корректировка скролла через общую функцию
        clampScroll(selectedIdx, startIdx, (int)users.size());

        string statusText;
        if (editingIdx >= 0) {
            statusText = "Всего пользователей: " + to_string(users.size()) + "  |  Режим: редактирование строки " + to_string(editingIdx + 1);
        } else {
            statusText = "Всего пользователей: " + to_string(users.size()) + "  |  Выбран: " + to_string(users.empty() ? 0 : selectedIdx + 1);
        }

        drawUsersTable("ИЗМЕНЕНИЕ ПОЛЬЗОВАТЕЛЕЙ", users, startIdx, selectedIdx, editingIdx, activeField, draftUser, message, messageColor, needFullRedraw, "Enter - редактировать выбранную строку. Esc - назад.", "Tab - поле. Space/стрелки/A/O - роль. Enter - сохранить. Esc - отменить.", statusText);
        needFullRedraw = false;

        if (editingIdx == -1) {
            int key = InputHandler::getExtKey();
            if (key == Key::ESC) {
                return;
            }
            if (users.empty()) {
                continue;
            }

            if (key == Key::DOWN || key == Key::TAB) {
                if (selectedIdx < (int)users.size() - 1) {
                    selectedIdx++;
                }
            } else if (key == Key::UP) {
                if (selectedIdx > 0) {
                    selectedIdx--;
                }
            } else if (key == Key::ENTER) {
                editingIdx = selectedIdx;
                draftUser = users[editingIdx];
                activeField = 0;
                message = "";
                messageColor = 8;
            }
            continue;
        }

        if (activeField == 0 || activeField == 1) {
            int rowY = 7 + (editingIdx - startIdx) * 2;
            int exitKey = 0;

            if (activeField == 0) {
                draftUser.login = processInput(10, rowY, 20, draftUser.login, false, exitKey, 0);
            } else {
                draftUser.password = processInput(34, rowY, 20, draftUser.password, true, exitKey, 0);
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

        int key = InputHandler::getExtKey();
        message = "";
        messageColor = 8;

        if (key == Key::TAB || key == Key::DOWN) {
            activeField = (activeField + 1) % 3;
        } else if (key == Key::UP) {
            activeField = (activeField + 2) % 3;
        } else if (key == Key::LEFT || key == Key::RIGHT || key == ' ') {
            if (draftUser.role == Role::ADMIN) {
                draftUser.role = Role::OPERATOR;
            } else {
                draftUser.role = Role::ADMIN;
            }
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
                    if (before.login != draftUser.login) {
                        det += "; логин: " + before.login + " -> " + draftUser.login;
                    }
                    if (before.password != draftUser.password) {
                        det += "; пароль изменён";
                    }
                    if (before.role != draftUser.role) {
                        string beforeRole = (before.role == Role::ADMIN) ? "ADMIN" : "OPERATOR";
                        string newRole = (draftUser.role == Role::ADMIN) ? "ADMIN" : "OPERATOR";
                        det += "; роль: " + beforeRole + " -> " + newRole;
                    }
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
        // вариант 8: корректировка скролла через общую функцию
        clampScroll(selectedIdx, startIdx, (int)users.size());

        string statusText = "Всего пользователей: " + to_string(users.size()) + "  |  Выбран: " + to_string(users.empty() ? 0 : selectedIdx + 1);

        drawUsersTable("УДАЛЕНИЕ ПОЛЬЗОВАТЕЛЕЙ", users, startIdx, selectedIdx, -1, 0, emptyUser, message, messageColor, needFullRedraw, "", "", statusText);
        needFullRedraw = false;

        int key = InputHandler::getExtKey();
        message = "";
        messageColor = 8;

        if (key == Key::ESC) {
            return;
        }
        if (users.empty()) {
            continue;
        }

        if (key == Key::DOWN || key == Key::TAB) {
            if (selectedIdx < (int)users.size() - 1) {
                selectedIdx++;
                if (selectedIdx >= startIdx + 8) {
                    startIdx++;
                }
            }
        } else if (key == Key::UP) {
            if (selectedIdx > 0) {
                selectedIdx--;
                if (selectedIdx < startIdx) {
                    startIdx--;
                }
            }
        } else if (key == Key::ENTER) {
            if (users[selectedIdx].role == Role::ADMIN) {
                bool otherAdminExists = false;
                for (int i = 0; i < (int)users.size(); ++i) {
                    if (i != selectedIdx && users[i].role == Role::ADMIN) {
                        otherAdminExists = true;
                        break;
                    }
                }
                if (!otherAdminExists) {
                    message = "Ошибка: нельзя удалить последнего администратора.";
                    messageColor = 12;
                    continue;
                }
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

            string snapshotRole = (snapshot.role == Role::ADMIN) ? "ADMIN" : "OPERATOR";
            Logger::log(LogCategory::USER, "Удалён пользователь", "login=" + snapshot.login + ", role=" + snapshotRole);

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
        {28, 45, "Значение"}};
    vector<int> separators = {25};

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

        // вариант 2: цикл по файлам вместо трёх одинаковых if-блоков
        string files[] = {"users.json", "clients.json", "tariffs.json"};
        for (const string& file : files) {
            if (std::filesystem::exists("data/" + file)) {
                std::filesystem::copy("data/" + file, backupPath + "/" + file, std::filesystem::copy_options::overwrite_existing);
            }
        }

        // вариант 1: логируем успех прямо внутри try
        Logger::log(LogCategory::SYSTEM, "Создан бэкап БД", "папка=" + backupFolderName);
    } catch (const exception&) {
        resultText = "Ошибка создания бэкапа";
        resultColor = 12;
        // вариант 1: логируем ошибку прямо внутри catch
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
}

void AdminPanel::showBackups() {
    vector<string> backups = loadBackupNames();
    int startIdx = 0;

    while (true) {
        drawBackupsList("БЭКАПЫ БД", backups, startIdx, -1, "", 8);

        int key = InputHandler::getExtKey();
        if (key == Key::ESC) {
            return;
        }
        if (backups.empty()) {
            continue;
        }

        if (key == Key::DOWN || key == Key::TAB) {
            if (startIdx + 8 < (int)backups.size()) {
                startIdx++;
            }
        } else if (key == Key::UP) {
            if (startIdx > 0) {
                startIdx--;
            }
        }
    }
}

void AdminPanel::restoreBackup() {
    vector<string> backups = loadBackupNames();
    int selectedIdx = backups.empty() ? -1 : 0;
    int startIdx = 0;
    string message = "";
    int messageColor = 8;

    while (true) {
        drawBackupsList("ВОССТАНОВЛЕНИЕ БЭКАПА", backups, startIdx, selectedIdx, message, messageColor);

        int key = InputHandler::getExtKey();
        message = "";
        messageColor = 8;

        if (key == Key::ESC) {
            return;
        }
        if (backups.empty()) {
            continue;
        }

        if (key == Key::DOWN || key == Key::TAB) {
            if (selectedIdx < (int)backups.size() - 1) {
                selectedIdx++;
                if (selectedIdx >= startIdx + 8) {
                    startIdx++;
                }
            }
        } else if (key == Key::UP) {
            if (selectedIdx > 0) {
                selectedIdx--;
                if (selectedIdx < startIdx) {
                    startIdx--;
                }
            }
        } else if (key == Key::ENTER) {
            string name = backups[selectedIdx];
            if (!showConfirmation("Восстановить бэкап " + name + "?")) {
                message = "Восстановление отменено.";
                messageColor = 8;
                continue;
            }

            try {
                string path = "data/backups/" + name;
                // вариант 2: цикл по файлам вместо трёх одинаковых if-блоков
                string files[] = {"users.json", "clients.json", "tariffs.json"};
                for (const string& file : files) {
                    if (filesystem::exists(path + "/" + file)) {
                        filesystem::copy(path + "/" + file, "data/" + file, filesystem::copy_options::overwrite_existing);
                    }
                }
                message = "Бэкап " + name + " успешно восстановлен.";
                messageColor = 10;
                Logger::log(LogCategory::SYSTEM, "Восстановлен бэкап БД", "папка=" + name);
            } catch (const exception&) {
                message = "Ошибка: не удалось восстановить бэкап.";
                messageColor = 12;
                Logger::log(LogCategory::SYSTEM, "Ошибка восстановления бэкапа", "папка=" + name);
            }
        }
    }
}
