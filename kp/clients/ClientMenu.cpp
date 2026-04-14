#include "ClientMenu.h"
#include "../core/InputHandler.h"
#include "../core/Render.h"
#include "ClientManager.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

using namespace std;

// Функция для безопасного обрезания UTF-8 строки
string safeTruncate(string str, size_t maxLen) {
    if (str.length() <= maxLen) return str;
    
    size_t bytes = 0;
    size_t chars = 0;
    while (bytes < str.length() && chars < maxLen - 3) {
        unsigned char c = (unsigned char)str[bytes];
        if (c >= 0 && c <= 127) bytes += 1;
        else if ((c & 0xE0) == 0xC0) bytes += 2;
        else if ((c & 0xF0) == 0xE0) bytes += 3;
        else if ((c & 0xF8) == 0xF0) bytes += 4;
        else bytes += 1; // Ошибка кодировки, но идем дальше
        chars++;
    }
    return str.substr(0, bytes) + "...";
}

// ============================================================
// 1) ПРОСМОТР БАЗЫ КЛИЕНТОВ (С ТАБЛИЦЕЙ И ПОДСВЕТКОЙ)
// ============================================================
void ClientMenu::drawClientTable(const vector<Client> &clients, int startIdx, int selectedIdx) {
  clearScreen();
  drawDoubleBox(1, 1, 90, 26, 15); // Ширина 90, начало с 1

  setCursor(38, 2);
  setColor(11);
  cout << "БАЗА АБОНЕНТОВ";

  // --- ЗАГОЛОВОК --- (координаты под ширину 90)
  setColor(15);
  setCursor(2, 4);  cout << "ID";
  setCursor(7, 4);  cout << "│";
  setCursor(9, 4);  cout << "ФИО";
  setCursor(48, 4); cout << "│";
  setCursor(50, 4); cout << "Телефон";
  setCursor(64, 4); cout << "│";
  setCursor(66, 4); cout << "Тариф";
  setCursor(78, 4); cout << "│";
  setCursor(80, 4); cout << "Статус";

  // --- СЕТКА (Разделитель) --- (длина 90)
  setCursor(1, 5);
  setColor(15);
  // ╠ (1) + ═ (2-6) + ╪ (7) + ═ (8-47) + ╪ (48) + ═ (49-63) + ╪ (64) + ═ (65-77) + ╪ (78) + ═ (79-88) + ╣ (89)
  cout << "╠═════╪═══════════════════════════════════════╪═══════════════╪═════════════╪══════════╣";

  int firstRow = 6;
  int pageSize = 18;
  
  for (int i = 0; i < pageSize; i++) {
    int curIdx = startIdx + i;
    int y = firstRow + i;
    bool isSelected = (curIdx == selectedIdx && curIdx < (int)clients.size());

    setCursor(2, y);
    if (isSelected) {
        setColor(240); // Белый фон, черный текст
        // Заполняем всю строку фоном (88 символов)
        cout << "                                                                                        "; 
    } else {
        setColor(7);
        cout << "     │                                       │               │             │          ";
    }

    if (curIdx < (int)clients.size()) {
        const auto &c = clients[curIdx];
        
        // Цвет: если выбрано, то везде 240 (черный на белом), иначе стандарт
        int focusColor = isSelected ? 240 : (c.isActive ? 7 : 8);

        // ID
        setCursor(2, y);
        if (isSelected) setColor(240); else setColor(15);
        cout << left << setw(5) << c.id;

        // Разделители и Данные 
        setCursor(7, y);  if (isSelected) setColor(240); else setColor(15); cout << "│";
        setCursor(9, y);  setColor(focusColor);
        string name = safeTruncate(c.fullName, 37);
        cout << left << setw(38) << name;

        setCursor(48, y); if (isSelected) setColor(240); else setColor(15); cout << "│";
        setCursor(50, y); setColor(focusColor);
        cout << left << setw(13) << c.phoneNumber;

        setCursor(64, y); if (isSelected) setColor(240); else setColor(15); cout << "│";
        setCursor(66, y); setColor(focusColor);
        string trfStr = c.tariffName.empty() ? "Базовый" : c.tariffName; 
        string trf = safeTruncate(trfStr, 11);
        cout << left << setw(11) << trf;

        setCursor(78, y); if (isSelected) setColor(240); else setColor(15); cout << "│";
        setCursor(80, y); 
        if (isSelected) {
            setColor(240);
        } else {
            c.isActive ? setColor(10) : setColor(4);
        }
        cout << (c.isActive ? "Активен" : "Заблокир.");
    }
  }

  // --- НИЖНЯЯ ПАНЕЛЬ ---
  setColor(15);
  setCursor(1, 24);
  cout << "╠════════════════════════════════════════════════════════════════════════════════════════╣";
  setCursor(3, 25);
  setColor(8);
  int total = (int)clients.size();
  cout << "Абонент: " << (total > 0 ? selectedIdx + 1 : 0) << " / " << total
       << "  |  [↑][↓] Навигация  |  [Enter] Редактировать  |  [Esc] Назад";

  drawFooter(29, true);
}

void ClientMenu::showList() {
  auto all = ClientManager::getAllClients();
  int startIdx = 0;
  int selectedIdx = 0;
  const int pageSize = 18; // Количество строк данных в таблице

  while (true) {
    drawClientTable(all, startIdx, (all.empty() ? -1 : selectedIdx));

    int key = InputHandler::getExtKey();
    if (key == Key::ESC) break;

    if (key == Key::DOWN && !all.empty() && selectedIdx < (int)all.size() - 1) {
      selectedIdx++;
      if (selectedIdx >= startIdx + pageSize) startIdx++;
    }
    if (key == Key::UP && !all.empty() && selectedIdx > 0) {
      selectedIdx--;
      if (selectedIdx < startIdx) startIdx--;
    }
    if (key == Key::ENTER && !all.empty()) {
        // В будущем здесь будет открытие меню редактирования конкретного клиента
        showEditClient(all[selectedIdx].id);
    }
  }
}

// ============================================================
// 2) ПОИСК ПО БАЗЕ
// ============================================================
void ClientMenu::showSearch() {
  string query = "";
  int activeField = 0;

  clearScreen();
  drawDoubleBox(2, 1, 76, 22, 8); // Уменьшил высоту с 24 до 22

  setCursor(30, 2);
  setColor(11);
  cout << "ПОИСК ПО БАЗЕ";

  // Сепаратор
  setColor(8);
  setCursor(2, 4);
  cout << "╠══════════════════════════════════════════════════════════════════════════╣";

  setCursor(6, 6);
  setColor(7);
  cout << "Введите ФИО, номер телефона или ID абонента:";

  // Исходная отрисовка поля (x=15 чтобы не наезжать на "> Запрос:")
  drawInputContent(15, 8, 44, query, false, true);

  while (true) {
    // Подсветка метки
    setCursor(4, 8);
    setColor(10);
    cout << "> Запрос:";

    drawFooter(26);

    int exitKey = 0;
    query = processInput(15, 8, 44, query, false, exitKey, 24); // warningY на 24 (под рамкой)

    if (exitKey == Key::ESC) {
      return; // выходим без поиска
    } else if (exitKey == Key::ENTER) {
      break; // начинаем поиск
    }
  }

  if (query.empty()) return;

  auto results = ClientManager::findClients(query);
  if (results.empty()) {
    // Экран «ничего не найдено»
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
      // Перекрашиваем заголовок
      setCursor(25, 2);
      setColor(14);
      cout << "РЕЗУЛЬТАТЫ ПОИСКА (" << results.size() << ")";

      int key = InputHandler::getExtKey();
      if (key == Key::ESC) break;
      if (key == Key::DOWN && (size_t)(start + pageSize) < results.size()) start++;
      if (key == Key::UP && start > 0) start--;
    }
  }
}

// ============================================================
// 3) ДОБАВЛЕНИЕ НОВОГО КЛИЕНТА
// ============================================================
void ClientMenu::showAddClient() {
  string fullName = "";
  string phone = "";
  string tariff = "";
  int activeField = 0;
  const int numFields = 3;

  clearScreen();
  drawDoubleBox(2, 2, 76, 22, 8); // Ширина 76, высота 22

  setCursor(30, 3);
  setColor(11);
  cout << "НОВЫЙ КЛИЕНТ";

  // Разделитель после заголовка
  setColor(8);
  setCursor(2, 5);
  cout << "╠══════════════════════════════════════════════════════════════════════════╣";

  // Исходная отрисовка полей
  drawInputContent(25, 9,  36, fullName, false, false);
  drawInputContent(25, 13, 36, phone,    false, false);
  drawInputContent(25, 17, 36, tariff,   false, false);

  // Пояснение по балансу
  setCursor(6, 21);
  setColor(8);
  cout << "Баланс и ID назначаются автоматически. Статус: Активен.";

  while (true) {
    // Метки полей
    setCursor(6, 9);
    setColor(activeField == 0 ? 10 : 7);
    cout << (activeField == 0 ? "> ФИО / Имя:" : "  ФИО / Имя:");

    setColor(8);
    setCursor(2, 11);
    cout << "╠══════════════════════════════════════════════════════════════════════════╣";

    setCursor(6, 13);
    setColor(activeField == 1 ? 10 : 7);
    cout << (activeField == 1 ? "> Номер телефона:" : "  Номер телефона:");

    setColor(8);
    setCursor(2, 15);
    cout << "╠══════════════════════════════════════════════════════════════════════════╣";

    setCursor(6, 17);
    setColor(activeField == 2 ? 10 : 7);
    cout << (activeField == 2 ? "> Название тарифа:" : "  Название тарифа:");

    drawFooter(26);

    int exitKey = 0;
    if (activeField == 0) {
      fullName = processInput(25, 9,  36, fullName, false, exitKey, 22);
    } else if (activeField == 1) {
      phone    = processInput(25, 13, 36, phone,    false, exitKey, 22);
    } else {
      tariff   = processInput(25, 17, 36, tariff,   false, exitKey, 22);
    }

    if (exitKey == Key::TAB || exitKey == Key::DOWN) {
      activeField = (activeField + 1) % numFields;
    } else if (exitKey == Key::UP) {
      activeField = (activeField - 1 + numFields) % numFields;
    } else if (exitKey == Key::ENTER) {
      if (activeField < numFields - 1) {
        activeField++;
      } else {
        // Последнее поле — попытка сохранить
        break;
      }
    } else if (exitKey == Key::ESC) {
      return;
    }
  }

  // Сохраняем
  // Используем расширенный addClient, дополняя тариф
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
