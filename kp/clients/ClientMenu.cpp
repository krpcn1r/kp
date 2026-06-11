#include "ClientMenu.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "../core/Database.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "ClientManager.h"
#include "TariffStruct.h"

using namespace std;

static void drawHLine(int y) {
    drawHLineAt(2, y, 74, 8);
}

static void drawClientsTable(const string& title, const vector<Client>& clients, int startIdx, int selectedIdx, int editingIdx, int activeField, const Client& draft, const string& draftBalance, const string& message, int messageColor, bool fullRedraw, const string& statusText) {
    vector<TableColumn> cols = {
        {3, 4, "ID"},
        {10, 24, "ФИО"},
        {37, 14, "Телефон"},
        {54, 12, "Тариф"},
        {69, 8, "Баланс"},
        {80, 9, "Статус"}};
    vector<int> seps = {8, 35, 52, 67, 78};

    if (fullRedraw) {
        clearScreen();
        drawBox(1, 1, 90, 28, 14);

        setCursor(34 - (int)title.length() / 4, 2);
        setColor(11);
        cout << title;

        drawTableHeader(4, cols, seps, 15);
        drawTableSeparator(2, 5, 87, seps, 15);
    }

    for (int i = 0; i < 8; i++) {
        int idx = startIdx + i;
        int y = 7 + i * 2;
        bool hasClient = idx < (int)clients.size();
        bool isSelected = hasClient && idx == selectedIdx;
        bool isEditing = hasClient && idx == editingIdx;
        int rowColor = isSelected ? 240 : 7;

        clearLine(2, y, 87, rowColor);

        if (hasClient) {
            const Client& c = isEditing ? draft : clients[idx];

            drawTableCell(3, y, 4, to_string(clients[idx].id), rowColor);
            drawTableCell(8, y, 1, "|", rowColor);

            if (isEditing && activeField == 0) {
                drawInputContent(10, y, 24, draft.fullName, false, true);
            } else {
                drawTableCell(10, y, 24, c.fullName, rowColor);
            }

            drawTableCell(35, y, 1, "|", rowColor);

            if (isEditing && activeField == 1) {
                drawInputContent(37, y, 14, draft.phoneNumber, false, true);
            } else {
                drawTableCell(37, y, 14, c.phoneNumber, rowColor);
            }

            drawTableCell(52, y, 1, "|", rowColor);

            string tariffStr = c.tariffName.empty() ? "Базовый" : c.tariffName;
            if (isEditing && activeField == 2) {
                drawInputContent(54, y, 12, draft.tariffName, false, true);
            } else {
                drawTableCell(54, y, 12, tariffStr, rowColor);
            }

            drawTableCell(67, y, 1, "|", rowColor);

            if (isEditing && activeField == 3) {
                drawInputContent(69, y, 8, draftBalance, false, true);
            } else {
                drawTableCell(69, y, 8, formatMoney(c.balance), rowColor);
            }

            drawTableCell(78, y, 1, "|", rowColor);

            if (isEditing) {
                string draftStat = draft.isActive ? "Активен" : "Заблок.";
                drawTableCell(80, y, 9, draftStat, activeField == 4 ? 31 : rowColor);
            } else {
                string statStr = c.isActive ? "Активен" : "Заблок.";
                int statColor = isSelected ? 240 : (c.isActive ? 10 : 4);
                drawTableCell(80, y, 9, statStr, statColor);
            }
        }

        drawTableSeparator(2, y + 1, 87, seps, 8);
    }

    clearLine(3, 24, 84);
    setCursor(3, 24);
    setColor(messageColor);
    if (!message.empty()) {
        cout << message;
    } else if (clients.empty()) {
        cout << "Абоненты не найдены.";
    }

    clearLine(3, 27, 84);
    setCursor(3, 27);
    setColor(8);
    cout << statusText;

    clearLine(2, 29, 86);
    drawFooter(29, true);
}

static bool validateClientEdit(const vector<Client>& clients, int editIdx, const Client& draft, const string& draftBalance, string& message, int& activeField) {
    if (draft.fullName.empty()) {
        message = "Ошибка: ФИО не может быть пустым.";
        activeField = 0;
        return false;
    }
    if (draft.phoneNumber.empty()) {
        message = "Ошибка: номер телефона не может быть пустым.";
        activeField = 1;
        return false;
    }
    for (int i = 0; i < (int)clients.size(); i++) {
        if (i != editIdx && clients[i].phoneNumber == draft.phoneNumber) {
            message = "Ошибка: такой номер телефона уже занят.";
            activeField = 1;
            return false;
        }
    }
    try {
        (void)stod(draftBalance);
    } catch (...) {
        message = "Ошибка: введите корректный баланс (например 100.50).";
        activeField = 3;
        return false;
    }
    return true;
}

void ClientMenu::showList() {
    vector<Client> clients = ClientManager::getAllClients();
    int selectedIdx = clients.empty() ? -1 : 0;
    int startIdx = 0;
    int editingIdx = -1;
    int activeField = 0;
    Client draftClient = {};
    string draftBalance = "";
    string message = "";
    int messageColor = 8;
    bool needFullRedraw = true;

    while (true) {
        if (clients.empty()) {
            selectedIdx = -1;
            startIdx = 0;
        } else {
            if (selectedIdx < 0) {
                selectedIdx = 0;
            }
            if (selectedIdx >= (int)clients.size()) {
                selectedIdx = (int)clients.size() - 1;
            }
            if (selectedIdx < startIdx) {
                startIdx = selectedIdx;
            }
            if (selectedIdx >= startIdx + 8) {
                startIdx = selectedIdx - 8 + 1;
            }
        }

        string statusText = editingIdx >= 0
                                ? "Редактирование #" + to_string(editingIdx + 1) + "  |  Tab - поле  Enter - сохранить  Esc - отменить"
                                : "Всего абонентов: " + to_string(clients.size()) + "  |  Enter - изменить  Del - удалить  Esc - назад";

        drawClientsTable("БАЗА АБОНЕНТОВ", clients, startIdx, selectedIdx, editingIdx, activeField, draftClient, draftBalance, message, messageColor, needFullRedraw, statusText);
        needFullRedraw = false;

        if (editingIdx == -1) {
            int key = InputHandler::getExtKey();
            if (key == Key::ESC) {
                return;
            }
            if (clients.empty()) {
                continue;
            }

            if (key == Key::DOWN || key == Key::TAB) {
                if (selectedIdx < (int)clients.size() - 1) {
                    selectedIdx++;
                }
            } else if (key == Key::UP) {
                if (selectedIdx > 0) {
                    selectedIdx--;
                }
            } else if (key == Key::ENTER) {
                editingIdx = selectedIdx;
                draftClient = clients[editingIdx];
                draftBalance = formatMoney(draftClient.balance);
                activeField = 0;
                message = "";
                messageColor = 8;
            } else if (key == Key::DEL) {
                string name = clients[selectedIdx].fullName;
                if (!showConfirmation("Удалить абонента " + name + "?")) {
                    needFullRedraw = true;
                    message = "Удаление отменено.";
                    messageColor = 8;
                    continue;
                }
                if (!ClientManager::deleteClient(clients[selectedIdx].id)) {
                    needFullRedraw = true;
                    message = "Ошибка: не удалось удалить абонента.";
                    messageColor = 12;
                    continue;
                }
                clients = ClientManager::getAllClients();
                if (selectedIdx >= (int)clients.size()) {
                    selectedIdx = (int)clients.size() - 1;
                }
                needFullRedraw = true;
                message = "Абонент " + name + " удален.";
                messageColor = 10;
            }
        } else if (activeField <= 3) {
            int rowY = 7 + (editingIdx - startIdx) * 2;
            int exitKey = 0;

            if (activeField == 0) {
                draftClient.fullName = processInput(10, rowY, 24, draftClient.fullName, false, exitKey, 0, true);
            } else if (activeField == 1) {
                // телефон — только цифры и знак плюс
                draftClient.phoneNumber = processInput(37, rowY, 14, draftClient.phoneNumber, false, exitKey, 0, false, "0123456789+");
            } else if (activeField == 2) {
                draftClient.tariffName = processInput(54, rowY, 12, draftClient.tariffName, false, exitKey, 0, true);
            } else {
                draftBalance = processInput(69, rowY, 8, draftBalance, false, exitKey, 0);
            }

            message = "";
            messageColor = 8;

            if (exitKey == Key::TAB || exitKey == Key::DOWN) {
                activeField = (activeField + 1) % 5;
            } else if (exitKey == Key::UP) {
                activeField = (activeField - 1 + 5) % 5;
            } else if (exitKey == Key::ENTER) {
                activeField++;
            } else if (exitKey == Key::ESC) {
                editingIdx = -1;
                activeField = 0;
                message = "Редактирование отменено.";
                messageColor = 8;
            }
        } else {
            int key = InputHandler::getExtKey();
            message = "";
            messageColor = 8;

            if (key == Key::TAB || key == Key::DOWN) {
                activeField = (activeField + 1) % 5;
            } else if (key == Key::UP) {
                activeField = (activeField - 1 + 5) % 5;
            } else if (key == Key::LEFT || key == Key::RIGHT || key == ' ') {
                draftClient.isActive = !draftClient.isActive;
            } else if (key == Key::ENTER) {
                string validMsg;
                int invalidField = activeField;
                if (!validateClientEdit(clients, editingIdx, draftClient, draftBalance, validMsg, invalidField)) {
                    message = validMsg;
                    messageColor = 12;
                    activeField = invalidField;
                } else {
                    draftClient.balance = stod(draftBalance);
                    if (!ClientManager::updateClient(draftClient.id, draftClient)) {
                        message = "Ошибка: не удалось сохранить изменения.";
                        messageColor = 12;
                    } else {
                        clients = ClientManager::getAllClients();
                        selectedIdx = editingIdx;
                        editingIdx = -1;
                        activeField = 0;
                        message = "Абонент успешно изменен.";
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
}

void ClientMenu::showSearch() {
    string queryId = "";
    string queryName = "";
    string queryPhone = "";
    string queryTariff = "";
    int activeField = 0;

    auto drawSearchForm = [&](bool full) {
        if (full) {
            clearScreen();
            drawBox(2, 1, 76, 24, 8);

            setCursor(30, 2);
            setColor(11);
            cout << "ПОИСК ПО БАЗЕ";

            drawHLine(3);
        }

        setCursor(6, 5);
        setColor(activeField == 0 ? 10 : 7);
        cout << (activeField == 0 ? "> ID:             " : "  ID:             ");
        drawInputContent(25, 5, 36, queryId, false, activeField == 0);

        drawHLine(7);

        setCursor(6, 9);
        setColor(activeField == 1 ? 10 : 7);
        cout << (activeField == 1 ? "> ФИО:            " : "  ФИО:            ");
        drawInputContent(25, 9, 36, queryName, false, activeField == 1);

        drawHLine(11);

        setCursor(6, 13);
        setColor(activeField == 2 ? 10 : 7);
        cout << (activeField == 2 ? "> Телефон:        " : "  Телефон:        ");
        drawInputContent(25, 13, 36, queryPhone, false, activeField == 2);

        drawHLine(15);

        setCursor(6, 17);
        setColor(activeField == 3 ? 10 : 7);
        cout << (activeField == 3 ? "> Тариф:          " : "  Тариф:          ");
        drawInputContent(25, 17, 36, queryTariff, false, activeField == 3);

        drawHLine(19);

        setCursor(6, 21);
        setColor(8);
        cout << "Незаполненные поля не учитываются. Enter на последнем поле = поиск.";

        drawFooter(27);
    };

    drawSearchForm(true);

    while (true) {
        drawSearchForm(false);

        int exitKey = 0;
        string* vals[] = {&queryId, &queryName, &queryPhone, &queryTariff};
        int ys[] = {5, 9, 13, 17};
        // ФИО и тариф разрешаем вводить кириллицей, ID/телефон — нет
        bool allowUnicode = (activeField == 1 || activeField == 3);
        *vals[activeField] = processInput(25, ys[activeField], 36, *vals[activeField], false, exitKey, 0, allowUnicode);

        if (exitKey == Key::ESC) {
            return;
        } else if (exitKey == Key::TAB || exitKey == Key::DOWN) {
            activeField = (activeField + 1) % 4;
        } else if (exitKey == Key::UP) {
            activeField = (activeField - 1 + 4) % 4;
        } else if (exitKey == Key::ENTER) {
            if (activeField < 3) {
                activeField++;
            } else {
                break;  // Enter на последнем поле — начинаем поиск
            }
        }
    }

    auto results = ClientManager::findClientsByFields(queryId, queryName, queryPhone, queryTariff);

    if (results.empty()) {
        clearScreen();
        drawBox(15, 9, 50, 7, 8);
        setCursor(25, 11);
        setColor(12);
        cout << "Ничего не найдено";
        setCursor(20, 13);
        setColor(8);
        cout << "По заданным критериям абоненты не найдены.";
        setCursor(20, 15);
        cout << "Нажмите любую клавишу для возврата...";
        InputHandler::waitAnyKey();
    } else {
        int idx = 0;
        int total = (int)results.size();

        while (true) {
            const Client& c = results[idx];

            clearScreen();
            drawBox(8, 2, 64, 22, 15);

            // заголовок по центру: box x=8, w=64 -> центр x=40, строка 17 симв -> старт x=32
            setCursor(32, 3);
            setColor(11);
            cout << "РЕЗУЛЬТАТЫ ПОИСКА";

            drawHLineAt(8, 4, 62, 15);

            // счётчик
            setCursor(10, 5);
            setColor(8);
            cout << "Найдено: " << total
                 << "  |  Запись " << (idx + 1) << " из " << total;

            drawHLineAt(8, 6, 62, 15);

            // значения выровнены по x=26 (после самого длинного "Телефон:" + отступ)
            const int VAL_X = 26;
            auto field = [&](int y, const string& label, const string& value, int col) {
                setCursor(11, y);
                setColor(8);
                cout << label;
                setCursor(VAL_X, y);
                setColor(col);
                cout << value;
            };

            field(8, "ID:", to_string(c.id), 15);
            field(10, "ФИО:", c.fullName, 7);
            field(12, "Телефон:", c.phoneNumber, 7);
            field(14, "Тариф:", c.tariffName.empty() ? "-" : c.tariffName, 14);
            field(16, "Баланс:", formatMoney(c.balance) + " руб.", 10);
            field(18, "Статус:", c.isActive ? "Активен" : "Заблокирован", c.isActive ? 10 : 12);

            drawHLineAt(8, 20, 62, 15);

            setCursor(10, 21);
            setColor(8);
            if (total > 1) {
                cout << "[Up] Пред.  [Down] След.  |  [Esc] Назад";
            } else {
                cout << "[Esc] Назад";
            }

            drawFooter(27, true);

            int key = InputHandler::getExtKey();
            if (key == Key::ESC) {
                break;
            }
            if (key == Key::DOWN && idx < total - 1) {
                idx++;
            }
            if (key == Key::UP && idx > 0) {
                idx--;
            }
        }
    }
}

void ClientMenu::showAddClient() {
    vector<Tariff> tariffs = Database::loadTariffs();
    int tariffIdx = 0;

    string fullName = "";
    string phone = "";
    string balance = "0.00";
    int activeField = 0;

    auto drawForm = [&](bool full) {
        if (full) {
            clearScreen();
            drawBox(2, 1, 76, 24, 8);

            setCursor(28, 2);
            setColor(11);
            cout << "ОФОРМЛЕНИЕ НОВОГО КЛИЕНТА";

            drawHLine(3);
        }

        setCursor(6, 5);
        setColor(activeField == 0 ? 10 : 7);
        cout << (activeField == 0 ? "> ФИО:      " : "  ФИО:      ");
        drawInputContent(25, 5, 36, fullName, false, activeField == 0);

        drawHLine(7);

        setCursor(6, 9);
        setColor(activeField == 1 ? 10 : 7);
        cout << (activeField == 1 ? "> Номер телефона: " : "  Номер телефона: ");
        drawInputContent(25, 9, 36, phone, false, activeField == 1);

        drawHLine(11);

        setCursor(6, 13);
        setColor(activeField == 2 ? 10 : 7);
        cout << (activeField == 2 ? "> Тариф:          " : "  Тариф:          ");

        string tariffDisplay;
        if (tariffs.empty()) {
            tariffDisplay = "(нет тарифов)";
        } else {
            tariffDisplay = activeField == 2
                                ? "[ " + tariffs[tariffIdx].name + " ]"
                                : tariffs[tariffIdx].name;
        }
        setColor(activeField == 2 ? 14 : 7);
        setCursor(25, 13);
        cout << left << setw(36) << tariffDisplay;

        drawHLine(15);

        setCursor(6, 17);
        setColor(activeField == 3 ? 10 : 7);
        cout << (activeField == 3 ? "> Баланс:         " : "  Баланс:         ");
        drawInputContent(25, 17, 36, balance, false, activeField == 3);

        drawHLine(19);

        setCursor(6, 21);
        setColor(8);
        cout << "ID назначается автоматически. Статус: Активен.           ";

        if (activeField == 2 && !tariffs.empty()) {
            setCursor(6, 22);
            setColor(8);
            cout << "[" << tariffIdx + 1 << "/" << (int)tariffs.size() << "] "
                 << tariffs[tariffIdx].pricePerMonth << " руб/мес  "
                 << tariffs[tariffIdx].speedMbps << " Мбит/с          ";
        } else {
            setCursor(6, 22);
            cout << "                                                       ";
        }

        drawFooter(27);
    };

    drawForm(true);

    while (true) {
        drawForm(false);

        if (activeField <= 1) {
            string* val = activeField == 0 ? &fullName : &phone;
            int y = activeField == 0 ? 5 : 9;
            int exitKey = 0;
            // ФИО разрешаем кириллицей; телефон — только цифры и знак плюс
            // warningY=23 (внутри рамки), строка 24 — нижняя граница рамки
            string allowed = activeField == 1 ? "0123456789+" : "";
            *val = processInput(25, y, 36, *val, false, exitKey, 23, activeField == 0, allowed);
            if (exitKey == Key::TAB || exitKey == Key::DOWN || exitKey == Key::ENTER) {
                activeField++;
            } else if (exitKey == Key::UP) {
                activeField = activeField == 0 ? 3 : 0;
            } else if (exitKey == Key::ESC) {
                return;
            }

        } else if (activeField == 2) {
            int key = InputHandler::getExtKey();
            if (key == Key::LEFT && !tariffs.empty()) {
                tariffIdx = (tariffIdx - 1 + (int)tariffs.size()) % (int)tariffs.size();
            } else if (key == Key::RIGHT && !tariffs.empty()) {
                tariffIdx = (tariffIdx + 1) % (int)tariffs.size();
            } else if (key == Key::DOWN || key == Key::TAB || key == Key::ENTER) {
                activeField = 3;
            } else if (key == Key::UP) {
                activeField = 1;
            } else if (key == Key::ESC) {
                return;
            }

        } else {  // activeField == 3
            int exitKey = 0;
            balance = processInput(25, 17, 36, balance, false, exitKey, 23);
            if (exitKey == Key::TAB || exitKey == Key::DOWN) {
                activeField = 0;
            } else if (exitKey == Key::UP) {
                activeField = 2;
            } else if (exitKey == Key::ENTER) {
                break;
            } else if (exitKey == Key::ESC) {
                return;
            }
        }
    }

    double balVal = 0.0;
    try {
        balVal = stod(balance);
    } catch (...) {
    }

    string tariffName = tariffs.empty() ? "" : tariffs[tariffIdx].name;
    int res = ClientManager::addClient(fullName, phone, tariffName, balVal);

    clearScreen();
    drawBox(15, 9, 50, 9, 8);

    if (res == 0) {
        setCursor(28, 11);
        setColor(10);
        cout << "Клиент добавлен!";
        setCursor(20, 12);
        setColor(7);
        cout << "ФИО: " << fullName;
        setCursor(20, 13);
        cout << "Тел: " << phone;
        setCursor(20, 14);
        cout << "Тариф: " << (tariffName.empty() ? "-" : tariffName);
        setCursor(20, 15);
        cout << "Баланс: " << balance;
    } else if (res == 2) {
        setCursor(22, 12);
        setColor(12);
        cout << "Ошибка: такой номер уже есть!";
    } else if (res == 1) {
        setCursor(22, 12);
        setColor(12);
        cout << "Ошибка: заполните ФИО и телефон!";
    } else {
        setCursor(22, 12);
        setColor(12);
        cout << "Ошибка сохранения.";
    }

    setCursor(20, 17);
    setColor(8);
    cout << "Нажмите любую клавишу...";
    InputHandler::waitAnyKey();
}
