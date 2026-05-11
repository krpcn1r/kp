#include "ClientMenu.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "ClientManager.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

// обрезает длинные имена чтобы они не вылазили за края таблицы
string safeTruncate(string str, size_t maxLen) {
  if (str.length() <= maxLen)
    return str;

  size_t bytes = 0;
  size_t chars = 0;
  // идем по байтам чтобы не сломать русские буквы
  while (bytes < str.length() && chars < maxLen - 3) {
    unsigned char c = (unsigned char)str[bytes];
    if (c >= 0 && c <= 127)
      bytes += 1;
    else if ((c & 0xE0) == 0xC0)
      bytes += 2;
    else if ((c & 0xF0) == 0xE0)
      bytes += 3;
    else if ((c & 0xF8) == 0xF0)
      bytes += 4;
    else
      bytes += 1; // Ошибка кодировки, но идем дальше
    chars++;
  }
  return str.substr(0, bytes) + "...";
}

// сколько строк влезает на страницу
static const int CLIENT_PAGE_SIZE = 8;

// x-позиции колонок и их ширины (box 90x28, внутренняя область x=2..89)
static const int COL_ID_X = 3,      COL_ID_W = 4;
static const int COL_NAME_X = 10,   COL_NAME_W = 24;
static const int COL_PHONE_X = 37,  COL_PHONE_W = 14;
static const int COL_TARIFF_X = 54, COL_TARIFF_W = 12;
static const int COL_BAL_X = 69,    COL_BAL_W = 8;
static const int COL_STAT_X = 80,   COL_STAT_W = 9;

// форматирует баланс в строку с двумя знаками после запятой
static string balanceToStr(double balance) {
    ostringstream oss;
    oss << fixed << setprecision(2) << balance;
    return oss.str();
}

// рисует саму таблицу с абонентами на экране
void ClientMenu::drawClientTable(const vector<Client> &clients, int startIdx,
                                 int selectedIdx) {
  clearScreen();
  drawDoubleBox(1, 1, 90, 26, 15); // большая рамка на весь экран

  setCursor(38, 2);
  setColor(11);
  cout << "БАЗА АБОНЕНТОВ";

  // пишем названия колонок
  setColor(15);
  setCursor(2, 4);
  cout << "ID";
  setCursor(7, 4);
  cout << "│";
  setCursor(9, 4);
  cout << "ФИО";
  setCursor(48, 4);
  cout << "│";
  setCursor(50, 4);
  cout << "Телефон";
  setCursor(64, 4);
  cout << "│";
  setCursor(66, 4);
  cout << "Тариф";
  setCursor(78, 4);
  cout << "│";
  setCursor(80, 4);
  cout << "Статус";

  // рисуем линию разделитель
  setCursor(1, 5);
  setColor(15);
  cout << "╠═════╪═══════════════════════════════════════╪═══════════════╪═════"
          "════════╪══════════╣";

  int firstRow = 6; // первая строка данных
  int pageSize = 18; // сколько человек влезет на экран

  // выводим список абонентов построчно
  for (int i = 0; i < pageSize; i++) {
    int curIdx = startIdx + i;
    int y = firstRow + i;
    bool isSelected = (curIdx == selectedIdx && curIdx < (int)clients.size());

    setCursor(2, y);
    if (isSelected) {
      setColor(240); // белый фон для выбранного
      cout << "                                                                "
              "                        ";
    } else {
      setColor(7);
      cout << "     │                                       │               │  "
              "           │          ";
    }

    if (curIdx < (int)clients.size()) {
      const auto &c = clients[curIdx];

      int focusColor = isSelected ? 240 : (c.isActive ? 7 : 8);

      // пишем ID
      setCursor(2, y);
      if (isSelected)
        setColor(240);
      else
        setColor(15);
      cout << left << setw(5) << c.id;

      // пишем ФИО и обрезаем если длинное
      setCursor(7, y);
      if (isSelected)
        setColor(240);
      else
        setColor(15);
      cout << "│";
      setCursor(9, y);
      setColor(focusColor);
      string name = safeTruncate(c.fullName, 37);
      cout << left << setw(38) << name;

      // пишем номер телефона
      setCursor(48, y);
      if (isSelected)
        setColor(240);
      else
        setColor(15);
      cout << "│";
      setCursor(50, y);
      setColor(focusColor);
      cout << left << setw(13) << c.phoneNumber;

      // пишем какой тариф у чела
      setCursor(64, y);
      if (isSelected)
        setColor(240);
      else
        setColor(15);
      cout << "│";
      setCursor(66, y);
      setColor(focusColor);
      string trfStr = c.tariffName.empty() ? "Базовый" : c.tariffName;
      string trf = safeTruncate(trfStr, 11);
      cout << left << setw(11) << trf;

      // пишем активен он или заблочен
      setCursor(78, y);
      if (isSelected)
        setColor(240);
      else
        setColor(15);
      cout << "│";
      setCursor(80, y);
      if (isSelected) {
        setColor(240);
      } else {
        c.isActive ? setColor(10) : setColor(4);
      }
      cout << (c.isActive ? "Активен" : "Заблокир.");
    }
  }

  // рисуем низ таблицы
  setColor(15);
  setCursor(1, 24);
  cout << "╠═══════════════════════════════════════════════════════════════════"
          "═════════════════════╣";
  setCursor(3, 25);
  setColor(8);
  int total = (int)clients.size();
  cout << "Абонент: " << (total > 0 ? selectedIdx + 1 : 0) << " / " << total
       << "  |  [↑][↓] Навигация  |  [Enter] Редактировать  |  [Esc] Назад";

  drawFooter(29, true); // рисуем подсказки в подвале
}

// отрисовка таблицы с поддержкой выделения и inline-редактирования
static void drawClientsTable(const string& title, const vector<Client>& clients,
                              int startIdx, int selectedIdx,
                              int editingIdx, int activeField,
                              const Client& draft, const string& draftBalance,
                              const string& message, int messageColor,
                              bool fullRedraw, const string& statusText) {
    vector<TableColumn> cols = {
        {COL_ID_X,     COL_ID_W,     "ID"},
        {COL_NAME_X,   COL_NAME_W,   "ФИО"},
        {COL_PHONE_X,  COL_PHONE_W,  "Телефон"},
        {COL_TARIFF_X, COL_TARIFF_W, "Тариф"},
        {COL_BAL_X,    COL_BAL_W,    "Баланс"},
        {COL_STAT_X,   COL_STAT_W,   "Статус"}
    };
    vector<int> seps = {8, 35, 52, 67, 78};

    if (fullRedraw) {
        clearScreen();
        drawDoubleBox(1, 1, 90, 28, 14);

        setCursor(34 - (int)title.length() / 4, 2);
        setColor(11);
        cout << title;

        drawTableHeader(4, cols, seps, 15);
        drawTableSeparator(2, 5, 87, seps, 15);
    }

    for (int i = 0; i < CLIENT_PAGE_SIZE; i++) {
        int idx = startIdx + i;
        int y = 7 + i * 2;
        bool hasClient = idx < (int)clients.size();
        bool isSelected = hasClient && idx == selectedIdx;
        bool isEditing = hasClient && idx == editingIdx;
        int rowColor = isSelected ? 240 : 7;

        clearLine(2, y, 87, rowColor);

        if (hasClient) {
            const Client& c = isEditing ? draft : clients[idx];

            // ID (только читаемый)
            drawTableCell(COL_ID_X, y, COL_ID_W, to_string(clients[idx].id), rowColor);
            drawTableCell(8, y, 1, "│", rowColor);

            // ФИО
            if (isEditing && activeField == 0)
                drawInputContent(COL_NAME_X, y, COL_NAME_W, draft.fullName, false, true);
            else
                drawTableCell(COL_NAME_X, y, COL_NAME_W, c.fullName, rowColor);

            drawTableCell(35, y, 1, "│", rowColor);

            // Телефон
            if (isEditing && activeField == 1)
                drawInputContent(COL_PHONE_X, y, COL_PHONE_W, draft.phoneNumber, false, true);
            else
                drawTableCell(COL_PHONE_X, y, COL_PHONE_W, c.phoneNumber, rowColor);

            drawTableCell(52, y, 1, "│", rowColor);

            // Тариф
            string tariffStr = c.tariffName.empty() ? "Базовый" : c.tariffName;
            if (isEditing && activeField == 2)
                drawInputContent(COL_TARIFF_X, y, COL_TARIFF_W, draft.tariffName, false, true);
            else
                drawTableCell(COL_TARIFF_X, y, COL_TARIFF_W, tariffStr, rowColor);

            drawTableCell(67, y, 1, "│", rowColor);

            // Баланс
            if (isEditing && activeField == 3)
                drawInputContent(COL_BAL_X, y, COL_BAL_W, draftBalance, false, true);
            else
                drawTableCell(COL_BAL_X, y, COL_BAL_W, balanceToStr(c.balance), rowColor);

            drawTableCell(78, y, 1, "│", rowColor);

            // Статус
            if (isEditing) {
                string draftStat = draft.isActive ? "Активен" : "Заблок.";
                drawTableCell(COL_STAT_X, y, COL_STAT_W, draftStat,
                              activeField == 4 ? 31 : rowColor);
            } else {
                string statStr = c.isActive ? "Активен" : "Заблок.";
                int statColor = isSelected ? 240 : (c.isActive ? 10 : 4);
                drawTableCell(COL_STAT_X, y, COL_STAT_W, statStr, statColor);
            }
        }

        drawTableSeparator(2, y + 1, 87, seps, 8);
    }

    clearLine(3, 24, 84);
    setCursor(3, 24);
    setColor(messageColor);
    if (!message.empty())
        cout << message;
    else if (clients.empty())
        cout << "Абоненты не найдены.";

    clearLine(3, 27, 84);
    setCursor(3, 27);
    setColor(8);
    cout << statusText;

    clearLine(2, 29, 86);
    drawFooter(29, true);
}

// проверка данных черновика перед сохранением
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

// открывает список всех абонентов с возможностью inline-редактирования
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
        // корректируем выделение и прокрутку
        if (clients.empty()) {
            selectedIdx = -1;
            startIdx = 0;
        } else {
            if (selectedIdx < 0) selectedIdx = 0;
            if (selectedIdx >= (int)clients.size()) selectedIdx = (int)clients.size() - 1;
            if (selectedIdx < startIdx) startIdx = selectedIdx;
            if (selectedIdx >= startIdx + CLIENT_PAGE_SIZE)
                startIdx = selectedIdx - CLIENT_PAGE_SIZE + 1;
        }

        string statusText;
        if (editingIdx >= 0) {
            statusText = "Всего абонентов: " + to_string(clients.size()) +
                         "  |  Редактирование строки " + to_string(editingIdx + 1);
        } else {
            statusText = "Всего абонентов: " + to_string(clients.size()) +
                         "  |  Выбран: " + to_string(clients.empty() ? 0 : selectedIdx + 1);
        }

        drawClientsTable("БАЗА АБОНЕНТОВ", clients, startIdx, selectedIdx,
                         editingIdx, activeField, draftClient, draftBalance,
                         message, messageColor, needFullRedraw, statusText);
        needFullRedraw = false;

        // режим выбора строки
        if (editingIdx == -1) {
            int key = InputHandler::getExtKey();
            if (key == Key::ESC) return;
            if (clients.empty()) continue;

            if (key == Key::DOWN || key == Key::TAB) {
                if (selectedIdx < (int)clients.size() - 1) selectedIdx++;
            } else if (key == Key::UP) {
                if (selectedIdx > 0) selectedIdx--;
            } else if (key == Key::ENTER) {
                editingIdx = selectedIdx;
                draftClient = clients[editingIdx];
                draftBalance = balanceToStr(draftClient.balance);
                activeField = 0;
                message = "";
                messageColor = 8;
            }
            continue;
        }

        // редактирование — текстовые поля (ФИО, Телефон, Тариф, Баланс)
        if (activeField >= 0 && activeField <= 3) {
            int rowY = 7 + (editingIdx - startIdx) * 2;
            int exitKey = 0;

            if (activeField == 0) {
                draftClient.fullName = processInput(COL_NAME_X, rowY, COL_NAME_W,
                                                    draftClient.fullName, false, exitKey, 0);
            } else if (activeField == 1) {
                draftClient.phoneNumber = processInput(COL_PHONE_X, rowY, COL_PHONE_W,
                                                       draftClient.phoneNumber, false, exitKey, 0);
            } else if (activeField == 2) {
                draftClient.tariffName = processInput(COL_TARIFF_X, rowY, COL_TARIFF_W,
                                                      draftClient.tariffName, false, exitKey, 0);
            } else {
                draftBalance = processInput(COL_BAL_X, rowY, COL_BAL_W,
                                            draftBalance, false, exitKey, 0);
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
            continue;
        }

        // редактирование — поле статуса (Space/стрелки — переключить, Enter — сохранить)
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

// меню поиска абонентов
void ClientMenu::showSearch() {
  string query = ""; // тут будет то что мы ищем
  int activeField = 0;

  clearScreen();
  drawDoubleBox(2, 1, 76, 22, 8); // средняя рамка

  setCursor(30, 2);
  setColor(11);
  cout << "ПОИСК ПО БАЗЕ";

  setColor(8);
  setCursor(2, 4);
  cout << "╠═══════════════════════════════════════════════════════════════════"
          "═══════╣";

  setCursor(6, 6);
  setColor(7);
  cout << "Введите ФИО, номер телефона или ID абонента:";

  // рисуем поле для ввода текста
  drawInputContent(15, 8, 44, query, false, true);

  while (true) {
    setCursor(4, 8);
    setColor(10);
    cout << "> Запрос:";

    drawFooter(26);

    int exitKey = 0;
    // ждем пока юзер напишет запрос
    query = processInput(15, 8, 44, query, false, exitKey, 24);

    if (exitKey == Key::ESC) {
      return; // отмена поиска
    } else if (exitKey == Key::ENTER) {
      break; // подтверждение поиска
    }
  }

  if (query.empty())
    return;

  auto results = ClientManager::findClients(query);
  if (results.empty()) {
    clearScreen();
    drawDoubleBox(15, 9, 50, 7, 8);
    setCursor(25, 11);
    setColor(12);
    cout << "Ничего не найдено";
    setCursor(20, 13);
    setColor(8);
    cout << "По запросу \"" << query << "\" абоненты не найдены.";
    setCursor(20, 15);
    cout << "Нажмите любую клавишу для возврата...";
    InputHandler::waitAnyKey();
  } else {
    int start = 0;
    const int pageSize = 19;
    while (true) {
      drawClientTable(results, start);
      setCursor(25, 2);
      setColor(14);
      cout << "РЕЗУЛЬТАТЫ ПОИСКА (" << results.size() << ")";

      int key = InputHandler::getExtKey();
      if (key == Key::ESC)
        break;
      if (key == Key::DOWN && (size_t)(start + pageSize) < results.size())
        start++;
      if (key == Key::UP && start > 0)
        start--;
    }
  }
}

// форма для добавления нового клиента
void ClientMenu::showAddClient() {
  string fullName = "";
  string phone = "";
  string tariff = "";
  int activeField = 0; // на каком поле сейчас стоим
  const int numFields = 3;

  clearScreen();
  drawDoubleBox(2, 2, 76, 22, 8);

  setCursor(30, 3);
  setColor(11);
  cout << "НОВЫЙ КЛИЕНТ";

  setColor(8);
  setCursor(2, 5);
  cout << "╠═══════════════════════════════════════════════════════════════════"
          "═══════╣";

  // рисуем пустые поля
  drawInputContent(25, 9, 36, fullName, false, false);
  drawInputContent(25, 13, 36, phone, false, false);
  drawInputContent(25, 17, 36, tariff, false, false);

  setCursor(6, 21);
  setColor(8);
  cout << "Баланс и ID назначаются автоматически. Статус: Активен.";

  while (true) {
    // подсвечиваем поле ФИО
    setCursor(6, 9);
    setColor(activeField == 0 ? 10 : 7);
    cout << (activeField == 0 ? "> ФИО / Имя:" : "  ФИО / Имя:");

    setColor(8);
    setCursor(2, 11);
    cout << "╠═════════════════════════════════════════════════════════════════"
            "═════════╣";

    // подсвечиваем телефон
    setCursor(6, 13);
    setColor(activeField == 1 ? 10 : 7);
    cout << (activeField == 1 ? "> Номер телефона:" : "  Номер телефона:");

    setColor(8);
    setCursor(2, 15);
    cout << "╠═════════════════════════════════════════════════════════════════"
            "═════════╣";

    // подсвечиваем поле тарифа
    setCursor(6, 17);
    setColor(activeField == 2 ? 10 : 7);
    cout << (activeField == 2 ? "> Название тарифа:" : "  Название тарифа:");

    drawFooter(26);

    int exitKey = 0;
    // ввод данных в текущее поле
    if (activeField == 0) {
      fullName = processInput(25, 9, 36, fullName, false, exitKey, 22);
    } else if (activeField == 1) {
      phone = processInput(25, 13, 36, phone, false, exitKey, 22);
    } else {
      tariff = processInput(25, 17, 36, tariff, false, exitKey, 22);
    }

    // управление прыгаем по полям
    if (exitKey == Key::TAB || exitKey == Key::DOWN) {
      activeField = (activeField + 1) % numFields;
    } else if (exitKey == Key::UP) {
      activeField = (activeField - 1 + numFields) % numFields;
    } else if (exitKey == Key::ENTER) {
      if (activeField < numFields - 1) {
        activeField++; // идем на следующее поле
      } else {
        break; // все ввели сохраняем
      }
    } else if (exitKey == Key::ESC) {
      return; // отмена ввода
    }
  }

  int res = ClientManager::addClient(fullName, phone);

  clearScreen();
  drawDoubleBox(15, 9, 50, 8, 8);

  if (res == 0) {
    setCursor(28, 11);
    setColor(10);
    cout << "Клиент добавлен!";
    setCursor(20, 13);
    setColor(7);
    cout << "ФИО: " << fullName;
    setCursor(20, 14);
    cout << "Тел: " << phone;
  } else if (res == 2) {
    setCursor(22, 11);
    setColor(12);
    cout << "Ошибка: такой номер уже есть!";
  } else if (res == 1) {
    setCursor(22, 11);
    setColor(12);
    cout << "Ошибка: заполните все поля!";
  } else {
    setCursor(22, 11);
    setColor(12);
    cout << "Ошибка сохранения.";
  }

  setCursor(20, 16);
  setColor(8);
  cout << "Нажмите любую клавишу...";
  InputHandler::waitAnyKey();
}

void ClientMenu::showEditClient(int clientId) {
  showPlaceholder("Редактирование клиента ID: " + to_string(clientId));
}
