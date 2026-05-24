#pragma once
#include <string>

// Простая модель тарификации.
// Считаем, что плата клиента в день = pricePerMonth его тарифа / 30.
// При каждом запуске программы (или ручной команде из админ-панели) проверяем,
// сколько суток прошло со дня последнего списания, и снимаем плату за каждые
// пропущенные сутки. Баланс может уйти в минус — клиент остаётся "должен".

struct BillingResult {
  int daysCharged    = 0; // за сколько суток списали
  int clientsTouched = 0; // у скольких клиентов сняли (хотя бы за день)
  double totalCharged = 0.0; // общая сумма списания
};

class Billing {
public:
  // выполнить автоматическое списание за все пропущенные дни с момента
  // последнего запуска (если их 0 — ничего не делает)
  static BillingResult runDailyChargeIfDue();

  // принудительно списать ровно за один день (для админ-кнопки)
  static BillingResult chargeOneDay();

  // дата последнего списания в формате YYYY-MM-DD ("" если ни разу не было)
  static std::string getLastChargeDate();

private:
  static const std::string STATE_FILE;
  static std::string todayDate();
  static int daysBetween(const std::string& fromDate, const std::string& toDate);
  static bool saveState(const std::string& lastChargeDate);
  static std::string loadLastDate();
  // фактическое списание за N дней; обновляет state и пишет лог
  static BillingResult chargeForDays(int days, const std::string& newLastDate);
};
