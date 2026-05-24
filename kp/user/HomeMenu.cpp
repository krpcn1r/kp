#include "HomeMenu.h"
#include "../auth/AuthManager.h"
#include "../core/Database.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "../admin/AdminPanel.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../clients/ClientMenu.h"
#include "../clients/ClientManager.h"
#include "../clients/TariffStruct.h"
using namespace std;

// рисует главное меню после того как юзер зашел в систему
HomeResult HomeMenu::show() {
  int selectedOption = 0; // какая кнопка сейчас выбрана
  const int numOptions = 8; // всего 8 пунктов меню
  bool needFullRedraw = true; // флаг полной перерисовки окна
  
  User u = AuthManager::getCurrentUser();
  string roleStr = (u.role == Role::ADMIN) ? "Администратор" : "Оператор";

  while (true) {
    if (needFullRedraw) {
      clearScreen(); // чистим всё перед рисованием

      drawBox(2, 1, 76, 28, 15); // простая ASCII-рамка

      auto drawHLine = [](int y) {
        setColor(15);
        setCursor(2, y);
        cout << "+";
        for (int i = 0; i < 74; i++) cout << "-";
        cout << "+";
      };

      // ASCII-арт логотип
      setColor(9);
      setCursor(16, 2);
      cout << "███╗   ███╗ ██████╗ ██████╗ ██╗██╗     ███████╗";
      setCursor(16, 3);
      cout << "████╗ ████║██╔═══██╗██╔══██╗██║██║     ██╔════╝";
      setCursor(16, 4);
      cout << "██╔████╔██║██║   ██║██████╔╝██║██║     █████╗  ";
      setCursor(16, 5);
      cout << "██║╚██╔╝██║██║   ██║██╔══██╗██║██║     ██╔══╝  ";
      setCursor(16, 6);
      cout << "██║ ╚═╝ ██║╚██████╔╝██████╔╝██║███████╗███████╗";
      setCursor(16, 7);
      cout << "╚═╝     ╚═╝ ╚═════╝ ╚═════╝ ╚═╝╚══════╝╚══════╝";

      setCursor(16, 8);
      cout << "██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ ";
      setCursor(16, 9);
      cout << "██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗";
      setCursor(16, 10);
      cout << "███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝";
      setCursor(16, 11);
      cout << "██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗";
      setCursor(16, 12);
      cout << "██║  ██║███████╗███████╗██║     ███████╗██║  ██║";
      setCursor(16, 13);
      cout << "╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝";

      drawHLine(14);

      // пишем кто сейчас зашел и какая у него роль
      setColor(7);
      setCursor(4, 15);
      cout << "Пользователь: " << u.login << "   |   Статус: " << roleStr;

      // рисуем разделительную линию
      drawHLine(16);

      needFullRedraw = false;
    }

    drawFooter(29, false);

    // Рендерим опции
    // список всех доступных кнопок в главном меню
    string options[numOptions] = {
        "1. Просмотр базы клиентов ", "2. Поиск по базе          ",
        "3. Просмотр текущих тарифов", "4. Оформление нового клиента",
        "5. Редактирование записей  ", "6. Сменить пароль          ",
        "7. Выход в меню авторизации", "8. Закрыть программу       "};
    if (roleStr == "Администратор") {
        options[5] = "6. Админ-панель";
    }
    // выводим пункты меню и красиво их красим
    for (int i = 0; i < numOptions; i++) {
      int yPos = 17 + i;
      if (i >= 3)
        yPos += 1; // делаем отступ после 3-й кнопки
      if (i >= 5)
        yPos += 1; // делаем отступ после 5-й кнопки

      // рисуем линии между группами кнопок: + ---- +
      auto drawMenuHLine = [](int y) {
        setCursor(2, y);
        setColor(15);
        cout << "+";
        for (int j = 0; j < 74; j++) cout << "-";
        cout << "+";
      };
      if (i == 3) drawMenuHLine(17 + 3);
      if (i == 5) drawMenuHLine(17 + 5 + 1);
      setCursor(6, yPos);
      if (i == selectedOption) {
        // подсвечиваем ту кнопку на которой стоит курсор
        if (i == 7)
          setColor(12); // красная если это выход
        else if (i == 5 && roleStr == "Администратор")
          setColor(14); // желтая для админ-панели
        else
          setColor(10); // зеленая для работы
        cout << "> " << options[i] << "         ";
      } else {
        // серые если не выбраны
        if (i == 7)
          setColor(4);
        else if (i == 5 && roleStr == "Администратор")
          setColor(6); // тёмно-желтая для админ-панели
        else
          setColor(8);
        cout << "  " << options[i] << "         ";
      }
    }

    int key = InputHandler::getExtKey(); // слушаем что нажал юзер

    // листаем меню вниз или по табу
    if (key == Key::TAB || key == Key::DOWN) {
      selectedOption = (selectedOption + 1) % numOptions;
    } else if (key == Key::UP) {
      selectedOption = (selectedOption - 1 + numOptions) % numOptions;
    } else if ((key >= '1' && key <= '8') || key == Key::ENTER) {
      // если нажали цифру то сразу выбираем нужный пункт
      if (key >= '1' && key <= '8') {
          selectedOption = key - '1';
      }
      
      // тут логика что делать для каждой кнопки
      if (selectedOption == 0) ClientMenu::showList(); // заходим в список
      else if (selectedOption == 1) ClientMenu::showSearch(); // идем искать
      else if (selectedOption == 2) showTariffs(); // просмотр тарифов
      else if (selectedOption == 3) ClientMenu::showAddClient(); // создаем нового чела
      else if (selectedOption == 4) editClients(); // редактирование клиентов
      else if (selectedOption == 5 && roleStr == "Оператор") {
          showChangePassword();
      }
      else if (selectedOption == 5 && roleStr == "Администратор") {
          AdminPanel::showAdminPanel();
      }
      else if (selectedOption == 6) return HomeResult::LOGOUT; // разлогиниться
      else if (selectedOption == 7) return HomeResult::EXIT_APP; // закрыть всё
      else {
        showPlaceholder(options[selectedOption]); // если раздел еще не готов
      }
      
      needFullRedraw = true; // перерисовываем всё после возврата из меню
    } else if (key == Key::ESC) {
      // если нажали эскейп спрашиваем реально ли выйти
      if (showConfirmation("Выйти из учетной записи?")) {
          return HomeResult::LOGOUT;
      }
      needFullRedraw = true;
    }
  }
}

// колонки таблицы тарифов
static const int TCOL_ID_X    = 3,  TCOL_ID_W    = 4;
static const int TCOL_NAME_X  = 10, TCOL_NAME_W  = 16;
static const int TCOL_PRICE_X = 29, TCOL_PRICE_W = 12;
static const int TCOL_SPEED_X = 44, TCOL_SPEED_W = 12;
static const int TCOL_DESC_X  = 59, TCOL_DESC_W  = 28;

static string priceToStr(double price) {
    ostringstream oss;
    oss << fixed << setprecision(2) << price;
    return oss.str() + " руб.";
}

void HomeMenu::showTariffs() {
    vector<Tariff> tariffs = Database::loadTariffs();
    int selectedIdx = tariffs.empty() ? -1 : 0;
    int startIdx = 0;
    const int PAGE_SIZE = 10;
    bool needFullRedraw = true;

    vector<TableColumn> cols = {
        {TCOL_ID_X,    TCOL_ID_W,    "ID"},
        {TCOL_NAME_X,  TCOL_NAME_W,  "Название"},
        {TCOL_PRICE_X, TCOL_PRICE_W, "Цена/мес"},
        {TCOL_SPEED_X, TCOL_SPEED_W, "Скорость"},
        {TCOL_DESC_X,  TCOL_DESC_W,  "Описание"},
    };
    vector<int> seps = {8, 27, 42, 57};

    while (true) {
        if (!tariffs.empty()) {
            if (selectedIdx < 0) selectedIdx = 0;
            if (selectedIdx >= (int)tariffs.size()) selectedIdx = (int)tariffs.size() - 1;
            if (selectedIdx < startIdx) startIdx = selectedIdx;
            if (selectedIdx >= startIdx + PAGE_SIZE) startIdx = selectedIdx - PAGE_SIZE + 1;
        }

        if (needFullRedraw) {
            clearScreen();
            drawBox(1, 3, 90, 20, 15);

            setCursor(36, 4);
            setColor(11);
            cout << "ТАРИФНЫЕ ПЛАНЫ";

            setColor(15);
            setCursor(1, 5);
            cout << "+";
            for (int i = 0; i < 88; i++) cout << "-";
            cout << "+";

            drawTableHeader(7, cols, seps, 15);
            drawTableSeparator(2, 8, 87, seps, 15);

            needFullRedraw = false;
        }

        for (int i = 0; i < PAGE_SIZE; i++) {
            int idx = startIdx + i;
            int y = 6 + i;
            bool hasRow = idx < (int)tariffs.size();
            bool isSelected = hasRow && idx == selectedIdx;
            int rowColor = isSelected ? 240 : 7;

            clearLine(2, y, 87, rowColor);

            if (hasRow) {
                const Tariff& t = tariffs[idx];

                drawTableCell(TCOL_ID_X,    y, TCOL_ID_W,    to_string(t.id),          rowColor);
                drawTableCell(8,            y, 1,             "|",                       rowColor);
                drawTableCell(TCOL_NAME_X,  y, TCOL_NAME_W,  t.name,                    rowColor);
                drawTableCell(27,           y, 1,             "|",                       rowColor);

                // цена — желтая если не выделено
                int priceColor = isSelected ? 240 : 14;
                drawTableCell(TCOL_PRICE_X, y, TCOL_PRICE_W, priceToStr(t.pricePerMonth), priceColor);
                drawTableCell(42,           y, 1,             "|",                       rowColor);

                // скорость — голубая если не выделено
                int speedColor = isSelected ? 240 : 11;
                string speedStr = to_string(t.speedMbps) + " Мбит/с";
                drawTableCell(TCOL_SPEED_X, y, TCOL_SPEED_W, speedStr,                   speedColor);
                drawTableCell(57,           y, 1,             "|",                       rowColor);

                drawTableCell(TCOL_DESC_X,  y, TCOL_DESC_W,  t.description,             rowColor);
            }
        }

        // статус-строка
        setColor(15);
        setCursor(1, 16);
        cout << "+";
        for (int i = 0; i < 88; i++) cout << "-";
        cout << "+";
        setCursor(3, 17);
        setColor(8);
        cout << "Всего тарифов: " << tariffs.size();

        drawFooter(29, true);

        int key = InputHandler::getExtKey();
        if (key == Key::ESC) return;
        if (key == Key::DOWN || key == Key::TAB) {
            if (selectedIdx < (int)tariffs.size() - 1) selectedIdx++;
        } else if (key == Key::UP) {
            if (selectedIdx > 0) selectedIdx--;
        }
    }
}

static string balanceToStr(double balance) {
    ostringstream oss;
    oss << fixed << setprecision(2) << balance;
    return oss.str();
}



static const int CCOL_PAGE_SIZE = 8;
static const int CCOL_ROW_Y    = 7;
static const int CCOL_ROW_STEP = 2;
static const int CCOL_ROW_W    = 87;

static const int CCOL_ID_X     = 3,  CCOL_ID_W     = 4;
static const int CCOL_NAME_X   = 10, CCOL_NAME_W   = 18;
static const int CCOL_PHONE_X  = 31, CCOL_PHONE_W  = 14;
static const int CCOL_TARIFF_X = 48, CCOL_TARIFF_W = 12;
static const int CCOL_BAL_X    = 63, CCOL_BAL_W    = 9;
static const int CCOL_STAT_X   = 75, CCOL_STAT_W   = 13;

static void drawEditClientsTable(
    const string& title,
    const vector<Client>& clients,
    int startIdx, int selectedIdx,
    int editingIdx, int activeField,
    const Client& draft, const string& draftBalance,
    const string& message, int messageColor,
    bool fullRedraw, const string& statusText)
{
    vector<TableColumn> cols = {
        {CCOL_ID_X,     CCOL_ID_W,     "ID"},
        {CCOL_NAME_X,   CCOL_NAME_W,   "ФИО"},
        {CCOL_PHONE_X,  CCOL_PHONE_W,  "Телефон"},
        {CCOL_TARIFF_X, CCOL_TARIFF_W, "Тариф"},
        {CCOL_BAL_X,    CCOL_BAL_W,    "Баланс"},
        {CCOL_STAT_X,   CCOL_STAT_W,   "Статус"}
    };
    vector<int> seps = {8, 29, 46, 61, 73};

    if (fullRedraw) {
        clearScreen();
        drawBox(1, 1, 90, 28, 14);
        setCursor(36 - (int)title.length() / 4, 2);
        setColor(11);
        cout << title;
        drawTableHeader(4, cols, seps, 15);
        drawTableSeparator(2, 5, CCOL_ROW_W, seps, 15);
    }

    for (int i = 0; i < CCOL_PAGE_SIZE; i++) {
        int idx = startIdx + i;
        int y   = CCOL_ROW_Y + i * CCOL_ROW_STEP;
        bool hasRow    = idx < (int)clients.size();
        bool isSelected = hasRow && idx == selectedIdx;
        bool isEditing  = hasRow && idx == editingIdx;
        int rowColor = isSelected ? 240 : 7;

        clearLine(2, y, CCOL_ROW_W, rowColor);

        if (hasRow) {
            const Client& c = isEditing ? draft : clients[idx];

            drawTableCell(CCOL_ID_X, y, CCOL_ID_W, to_string(clients[idx].id), rowColor);
            drawTableCell(8,  y, 1, "|", rowColor);

            if (isEditing && activeField == 0)
                drawInputContent(CCOL_NAME_X, y, CCOL_NAME_W, draft.fullName, false, true);
            else
                drawTableCell(CCOL_NAME_X, y, CCOL_NAME_W, c.fullName, rowColor);

            drawTableCell(29, y, 1, "|", rowColor);

            if (isEditing && activeField == 1)
                drawInputContent(CCOL_PHONE_X, y, CCOL_PHONE_W, draft.phoneNumber, false, true);
            else
                drawTableCell(CCOL_PHONE_X, y, CCOL_PHONE_W, c.phoneNumber, rowColor);

            drawTableCell(46, y, 1, "|", rowColor);

            string tariffStr = c.tariffName.empty() ? "-" : c.tariffName;
            drawTableCell(CCOL_TARIFF_X, y, CCOL_TARIFF_W, tariffStr,
                          (isEditing && activeField == 2) ? 31 : rowColor);

            drawTableCell(61, y, 1, "|", rowColor);

            if (isEditing && activeField == 3)
                drawInputContent(CCOL_BAL_X, y, CCOL_BAL_W, draftBalance, false, true);
            else
                drawTableCell(CCOL_BAL_X, y, CCOL_BAL_W, balanceToStr(c.balance), rowColor);

            drawTableCell(73, y, 1, "|", rowColor);

            bool activeStat = isEditing ? draft.isActive : c.isActive;
            string statStr  = activeStat ? "Активен" : "Заблок.";
            int statColor   = (isEditing && activeField == 4) ? 31
                            : isSelected ? 240
                            : (c.isActive ? 10 : 4);
            drawTableCell(CCOL_STAT_X, y, CCOL_STAT_W, statStr, statColor);
        }

        drawTableSeparator(2, y + 1, CCOL_ROW_W, seps, 8);
    }

    clearLine(3, 24, 84);
    setCursor(3, 24);
    setColor(messageColor);
    if (!message.empty())
        cout << message;
    else if (clients.empty())
        cout << "Клиенты не найдены.";

    clearLine(3, 27, 84);
    setCursor(3, 27);
    setColor(8);
    cout << statusText;

    clearLine(2, 29, 86);
    drawFooter(29, true);
}

static bool validateClientEdit(const vector<Client>& clients, int editIdx,
                               const Client& draft, const string& draftBalance,
                               string& message, int& activeField) {
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
        stod(draftBalance);
    } catch (...) {
        message = "Ошибка: введите корректный баланс (например 100.50).";
        activeField = 3;
        return false;
    }
    return true;
}

void HomeMenu::editClients() {
    vector<Client> clients = ClientManager::getAllClients();
    vector<Tariff> tariffs = Database::loadTariffs();
    int selectedIdx   = clients.empty() ? -1 : 0;
    int startIdx      = 0;
    int editingIdx    = -1;
    int activeField   = 0;
    int draftTariffIdx = 0;
    Client draftClient = {};
    string draftBalance = "";
    string message = "";
    int messageColor = 8;
    bool needFullRedraw = true;

    auto findTariffIdx = [&](const string& name) -> int {
        for (int i = 0; i < (int)tariffs.size(); i++)
            if (tariffs[i].name == name) return i;
        return 0;
    };

    while (true) {
        if (clients.empty()) {
            selectedIdx = -1;
            startIdx = 0;
        } else {
            if (selectedIdx < 0) selectedIdx = 0;
            if (selectedIdx >= (int)clients.size()) selectedIdx = (int)clients.size() - 1;
            if (selectedIdx < startIdx) startIdx = selectedIdx;
            if (selectedIdx >= startIdx + CCOL_PAGE_SIZE)
                startIdx = selectedIdx - CCOL_PAGE_SIZE + 1;
        }

        string statusText;
        if (editingIdx >= 0) {
            statusText = "Редактирование #" + to_string(editingIdx + 1) +
                         "  |  Tab - поле  ←→ тариф/статус  Enter - сохранить  Esc - отменить";
        } else {
            statusText = "Всего клиентов: " + to_string(clients.size()) +
                         "  |  Выбран: " + to_string(clients.empty() ? 0 : selectedIdx + 1) +
                         "  |  Enter - редактировать  Del - удалить  Esc - назад";
        }

        drawEditClientsTable("ИЗМЕНЕНИЕ КЛИЕНТОВ", clients, startIdx, selectedIdx,
                             editingIdx, activeField, draftClient, draftBalance,
                             message, messageColor, needFullRedraw, statusText);
        needFullRedraw = false;

        // --- режим выбора строки ---
        if (editingIdx == -1) {
            int key = InputHandler::getExtKey();
            if (key == Key::ESC) return;
            if (clients.empty()) continue;

            if (key == Key::DOWN || key == Key::TAB) {
                if (selectedIdx < (int)clients.size() - 1) selectedIdx++;
            } else if (key == Key::UP) {
                if (selectedIdx > 0) selectedIdx--;
            } else if (key == Key::ENTER) {
                editingIdx    = selectedIdx;
                draftClient   = clients[editingIdx];
                draftBalance  = balanceToStr(draftClient.balance);
                draftTariffIdx = findTariffIdx(draftClient.tariffName);
                activeField   = 0;
                message       = "";
                messageColor  = 8;
            } else if (key == Key::DEL) {
                string name = clients[selectedIdx].fullName;
                if (!showConfirmation("Удалить клиента " + name + "?")) {
                    needFullRedraw = true;
                    message = "Удаление отменено.";
                    messageColor = 8;
                    continue;
                }
                int deletedId = clients[selectedIdx].id;
                if (!ClientManager::deleteClient(deletedId)) {
                    needFullRedraw = true;
                    message = "Ошибка: не удалось удалить клиента.";
                    messageColor = 12;
                    continue;
                }
                clients = ClientManager::getAllClients();
                if (clients.empty()) {
                    selectedIdx = -1;
                    startIdx = 0;
                } else if (selectedIdx >= (int)clients.size()) {
                    selectedIdx = (int)clients.size() - 1;
                }
                needFullRedraw = true;
                message = "Клиент " + name + " удален.";
                messageColor = 10;
            }
            continue;
        }

        // --- ФИО (0) и Телефон (1): текстовый ввод ---
        if (activeField == 0 || activeField == 1) {
            int rowY = CCOL_ROW_Y + (editingIdx - startIdx) * CCOL_ROW_STEP;
            int exitKey = 0;

            if (activeField == 0)
                draftClient.fullName = processInput(CCOL_NAME_X, rowY, CCOL_NAME_W,
                                                   draftClient.fullName, false, exitKey, 0);
            else
                draftClient.phoneNumber = processInput(CCOL_PHONE_X, rowY, CCOL_PHONE_W,
                                                      draftClient.phoneNumber, false, exitKey, 0);
            message = "";
            messageColor = 8;

            if (exitKey == Key::TAB || exitKey == Key::DOWN || exitKey == Key::ENTER)
                activeField++;
            else if (exitKey == Key::UP)
                activeField = activeField > 0 ? activeField - 1 : 0;
            else if (exitKey == Key::ESC) {
                editingIdx = -1;
                message = "Редактирование отменено.";
                messageColor = 8;
            }
            continue;
        }

        // --- Баланс (3): текстовый ввод ---
        if (activeField == 3) {
            int rowY = CCOL_ROW_Y + (editingIdx - startIdx) * CCOL_ROW_STEP;
            int exitKey = 0;
            draftBalance = processInput(CCOL_BAL_X, rowY, CCOL_BAL_W,
                                       draftBalance, false, exitKey, 0);
            message = "";
            messageColor = 8;

            if (exitKey == Key::TAB || exitKey == Key::DOWN || exitKey == Key::ENTER)
                activeField++;
            else if (exitKey == Key::UP)
                activeField--;
            else if (exitKey == Key::ESC) {
                editingIdx = -1;
                message = "Редактирование отменено.";
                messageColor = 8;
            }
            continue;
        }

        // --- Тариф (2) и Статус (4): навигация стрелками ---
        int key = InputHandler::getExtKey();
        message = "";
        messageColor = 8;

        if (key == Key::TAB || key == Key::DOWN) {
            activeField = (activeField + 1) % 5;
        } else if (key == Key::UP) {
            activeField = (activeField - 1 + 5) % 5;
        } else if (key == Key::LEFT || key == Key::RIGHT) {
            if (activeField == 2 && !tariffs.empty()) {
                draftTariffIdx = key == Key::RIGHT
                    ? (draftTariffIdx + 1) % (int)tariffs.size()
                    : (draftTariffIdx - 1 + (int)tariffs.size()) % (int)tariffs.size();
                draftClient.tariffName = tariffs[draftTariffIdx].name;
            } else if (activeField == 4) {
                draftClient.isActive = !draftClient.isActive;
            }
        } else if (key == ' ') {
            if (activeField == 4)
                draftClient.isActive = !draftClient.isActive;
        } else if (key == Key::ENTER) {
            if (activeField == 2) {
                activeField++;
            } else { // activeField == 4
                string validMsg;
                int invalidField = activeField;
                if (!validateClientEdit(clients, editingIdx, draftClient, draftBalance,
                                       validMsg, invalidField)) {
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
                        editingIdx  = -1;
                        activeField = 0;
                        message     = "Клиент успешно изменен.";
                        messageColor = 10;
                        needFullRedraw = true;
                    }
                }
            }
        } else if (key == Key::ESC) {
            editingIdx  = -1;
            activeField = 0;
            message = "Редактирование отменено.";
            messageColor = 8;
        }
    }
}

void HomeMenu::showChangePassword() {
    string oldPassword = "";
    string newPassword = "";
    int activeField = 0; // 0 это ввод логина а 1 пароля

    clearScreen(); // очистка экрана перед отрисовкой нового окна

    // отрисовка простой ASCII-рамки
    drawBox(4, 4, 45, 12, 8);

    setCursor(19, 5);
    setColor(11);
    cout << "СМЕНА ПАРОЛЯ";

    // полоска под заголовком
    setColor(8);
    setCursor(4, 6);
    cout << "+";
    for (int i = 0; i < 43; i++)
        cout << "-";
    cout << "+";

    // вывод условий безопасности пароля
    setCursor(6, 7);
    setColor(7);
    cout << "Условия безопасности:";
    setCursor(7, 8);
    setColor(8);
    cout << "- Минимум 8 символов";
    setCursor(7, 9);
    cout << "- 1 цифра + 1 спецсимвол";
    setCursor(7, 10);
    cout << "- Только английские буквы";

    setCursor(4, 11);
    cout << "+";
    for (int i = 0; i < 43; i++)
        cout << "-";
    cout << "+";

    drawInputContent(24, 12, 20, oldPassword, false, false);
    drawInputContent(24, 14, 20, newPassword, false, false);

    // запуск цикла для заполнения полей
    while (true) {
        // подсветка поля логин если фокус на нем
        setCursor(6, 12);
        if (activeField == 0)
            setColor(10);
        else
            setColor(7);
        cout << (activeField == 0 ? "> Старый пароль:" : "  Старый пароль:");

        // тоже самое для пароля
        setCursor(6, 14);
        if (activeField == 1)
            setColor(10);
        else
            setColor(7);
        cout << (activeField == 1 ? "> Новый пароль:" : "  Новый пароль:");

        // отрисовка кнопок в нижней части
        drawFooter(20);

        int exitKey = 0;
        // вызов функции ввода текста
        if (activeField == 0) {
            oldPassword = processInput(24, 12, 20, oldPassword, false, exitKey, 16);
        }
        else {
            newPassword = processInput(24, 14, 20, newPassword, false, exitKey, 16);
        }

        // управление переключением полей или сохранение
        if (exitKey == Key::TAB || exitKey == Key::UP || exitKey == Key::DOWN) {
            activeField = (activeField == 0) ? 1 : 0;
        }
        else if (exitKey == Key::ENTER) {
            if (activeField == 0)
                activeField = 1; // переход к полю пароля
            else
                break; // попытка создания юзера
        }
        else if (exitKey == Key::ESC) {
            return; // выход без регистрации
        }
    }

    setCursor(8, 17);
    // попытка добавления нового пользователя в базу
    int result = AuthManager::changePassword(oldPassword, newPassword);
    if (result == 0) {
        setColor(10);
        cout << "Пароль успешно сменился!    ";
    }
    else {
        // вывод ошибки красным цветом
        setColor(12);
        if (result == 1)
            cout << "Ошибка: неверные данные                                   ";
        else if (result == 2)
            cout << "Ошибка: старый пароль не совпадает                        ";
        else if (result == 3)
            cout << "Ошибка: одно из полей пустое                              ";
        else if (result == 4)
            cout << "Ошибка: длина пароля меньше 8 символов                    ";
        else if (result == 5)
            cout << "Ошибка: в пароле отсутствуют специальные символы или цифры";
        else if (result == 6)
            cout << "Ошибка: только английские буквы                           ";
        else
            cout << "Ошибка записи в базу данных                               ";
    }

    setCursor(8, 18);
    setColor(8);
    cout << "Нажмите любую кнопку...";
    InputHandler::waitAnyKey();
}
