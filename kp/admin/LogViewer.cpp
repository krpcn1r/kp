#include "LogViewer.h"
#include "../core/InputHandler.h"
#include "../core/Logger.h"
#include "../core/Render.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static const int LOG_PAGE_SIZE = 16;

// цвет строки в зависимости от категории
static int colorForCategory(LogCategory cat) {
  switch (cat) {
    case LogCategory::AUTH:
        return 11; // светло-голубой
    case LogCategory::USER:
        return 14; // желтый
    case LogCategory::CLIENT:
        return 10; // зеленый
    case LogCategory::BILLING:
        return 13; // фиолетовый
    case LogCategory::SYSTEM:
        return 8;  // серый
  }
  return 7;
}

void LogViewer::show() {
  vector<LogEntry> all = Logger::loadEntries();

  // показываем самые свежие сверху
  reverse(all.begin(), all.end());

  // активный фильтр: -1 — все, иначе конкретная категория
  int filter = -1;

  auto applyFilter = [&]() {
    vector<LogEntry> out;
    for (const auto& e : all) {
      if (filter == -1 || (int)e.category == filter) out.push_back(e);
    }
    return out;
  };

  vector<LogEntry> view = applyFilter();
  int startIdx = 0;
  int selectedIdx = view.empty() ? -1 : 0;
  bool needFullRedraw = true;

  while (true) {
    if (!view.empty()) {
      if (selectedIdx < 0) selectedIdx = 0;
      if (selectedIdx >= (int)view.size()) selectedIdx = (int)view.size() - 1;
      if (selectedIdx < startIdx) startIdx = selectedIdx;
      if (selectedIdx >= startIdx + LOG_PAGE_SIZE)
        startIdx = selectedIdx - LOG_PAGE_SIZE + 1;
    } else {
      startIdx = 0;
      selectedIdx = -1;
    }

    if (needFullRedraw) {
      clearScreen();
      drawBox(1, 1, 110, 28, 14);

      setCursor(45, 2);
      setColor(11);
      cout << "ЖУРНАЛ СОБЫТИЙ";

      setColor(14);
      setCursor(1, 3);
      cout << "+";
      for (int i = 0; i < 108; i++) cout << "-";
      cout << "+";

      // шапка
      setColor(15);
      setCursor(3, 4);
      cout << "Время";
      setCursor(24, 4);
      cout << "|";
      setCursor(26, 4);
      cout << "Кто";
      setCursor(44, 4);
      cout << "|";
      setCursor(46, 4);
      cout << "Категория";
      setCursor(58, 4);
      cout << "|";
      setCursor(60, 4);
      cout << "Событие";

      setColor(14);
      setCursor(1, 5);
      cout << "+";
      for (int i = 0; i < 108; i++) cout << "-";
      cout << "+";

      needFullRedraw = false;
    }

    // отрисовка строк
    for (int i = 0; i < LOG_PAGE_SIZE; i++) {
      int idx = startIdx + i;
      int y = 6 + i;
      bool has = idx < (int)view.size();
      bool sel = has && idx == selectedIdx;
      int rowColor = sel ? 240 : 7;

      clearLine(2, y, 107, rowColor);
      if (!has) continue;

      const LogEntry& e = view[idx];
      drawTableCell(3,  y, 20, e.timestamp, rowColor);
      drawTableCell(24, y, 1,  "|", 7);
      drawTableCell(26, y, 17, e.actor, rowColor);
      drawTableCell(44, y, 1,  "|", 7);
      drawTableCell(46, y, 11, Logger::categoryToString(e.category), sel ? 240 : colorForCategory(e.category));
      drawTableCell(58, y, 1,  "|", 7);

      string text = e.action;
      if (!e.details.empty()) text += " :: " + e.details;
      drawTableCell(60, y, 47, text, rowColor);
    }

    // нижняя полоса
    setColor(14);
    setCursor(1, 23);
    cout << "+";
    for (int i = 0; i < 108; i++) cout << "-";
    cout << "+";

    clearLine(3, 24, 105);
    setCursor(3, 24);
    setColor(8);
    string filterName;
    switch (filter) {
      case -1:
          filterName = "Все";
          break;
      case (int)LogCategory::AUTH:
          filterName = "AUTH";
          break;
      case (int)LogCategory::USER:
          filterName = "USER";
          break;
      case (int)LogCategory::CLIENT:
          filterName = "CLIENT";
          break;
      case (int)LogCategory::BILLING:
          filterName = "BILLING";
          break;
      case (int)LogCategory::SYSTEM:
          filterName = "SYSTEM";
          break;
    }
    cout << "Всего записей: " << all.size()
         << "  |  Показано: " << view.size()
         << "  |  Фильтр: " << filterName;

    clearLine(3, 25, 105);
    setCursor(3, 25);
    setColor(8);
    cout << "Фильтры: [A]uth [U]ser [C]lient [B]illing [S]ystem [R] сброс";

    if (view.empty()) {
      setCursor(3, 14);
      setColor(12);
      cout << "Записей нет";
    }

    drawFooter(29, true);

    int key = InputHandler::getExtKey();
    if (key == Key::ESC) return;

    if (key == Key::DOWN || key == Key::TAB) {
      if (selectedIdx < (int)view.size() - 1) selectedIdx++;
    } else if (key == Key::UP) {
      if (selectedIdx > 0) selectedIdx--;
    } else {
      int newFilter = filter;
      if (key == 'A' || key == 'a') newFilter = (int)LogCategory::AUTH;
      else if (key == 'U' || key == 'u') newFilter = (int)LogCategory::USER;
      else if (key == 'C' || key == 'c') newFilter = (int)LogCategory::CLIENT;
      else if (key == 'B' || key == 'b') newFilter = (int)LogCategory::BILLING;
      else if (key == 'S' || key == 's') newFilter = (int)LogCategory::SYSTEM;
      else if (key == 'R' || key == 'r') newFilter = -1;

      if (newFilter != filter) {
        filter = newFilter;
        view = applyFilter();
        startIdx = 0;
        selectedIdx = view.empty() ? -1 : 0;
        needFullRedraw = true;
      }
    }
  }
}
