#include "Billing.h"
#include "ClientStruct.h"
#include "TariffStruct.h"
#include "../core/Database.h"
#include "../core/Logger.h"
#include "../core/json.hpp"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>

using namespace std;
using json = nlohmann::json;

const string Billing::STATE_FILE = "data/billing_state.json";

string Billing::todayDate() {
  auto now = chrono::system_clock::now();
  time_t t = chrono::system_clock::to_time_t(now);
  tm local_tm;
  localtime_s(&local_tm, &t);
  ostringstream oss;
  oss << put_time(&local_tm, "%Y-%m-%d");
  return oss.str();
}

// возвращает количество полных суток (toDate - fromDate); если что-то не парсится,
// возвращает 0
int Billing::daysBetween(const string& fromDate, const string& toDate) {
  if (fromDate.empty() || toDate.empty()) return 0;

  auto parse = [](const string& s, time_t& out) -> bool {
    tm t = {};
    istringstream iss(s);
    iss >> get_time(&t, "%Y-%m-%d");
    if (iss.fail()) return false;
    t.tm_hour = 12; // полдень, чтобы избежать сюрпризов из-за летнего времени
    out = mktime(&t);
    return out != -1;
  };

  time_t a = 0, b = 0;
  if (!parse(fromDate, a) || !parse(toDate, b)) return 0;
  double diff = difftime(b, a) / (60 * 60 * 24);
  if (diff <= 0) return 0;
  return (int)(diff + 0.5);
}

string Billing::loadLastDate() {
  ifstream f(STATE_FILE);
  if (!f.is_open()) return "";
  try {
    json j;
    f >> j;
    if (j.contains("lastChargeDate") && j["lastChargeDate"].is_string()) {
      return j["lastChargeDate"].get<string>();
    }
  } catch (...) {}
  return "";
}

bool Billing::saveState(const string& lastChargeDate) {
  try {
    if (!filesystem::exists("data")) filesystem::create_directories("data");
  } catch (...) {}
  json j;
  j["lastChargeDate"] = lastChargeDate;
  ofstream f(STATE_FILE);
  if (!f.is_open()) return false;
  f << j.dump(4);
  return true;
}

string Billing::getLastChargeDate() {
  return loadLastDate();
}

BillingResult Billing::chargeForDays(int days, const string& newLastDate) {
  BillingResult res;
  if (days <= 0) return res;

  vector<Client> clients = Database::loadClients();
  vector<Tariff> tariffs = Database::loadTariffs();

  unordered_map<string, double> priceByName;
  for (const auto& t : tariffs) priceByName[t.name] = t.pricePerMonth;

  for (auto& c : clients) {
    if (!c.isActive) continue;
    auto it = priceByName.find(c.tariffName);
    if (it == priceByName.end() || it->second <= 0.0) continue;

    double daily = it->second / 30.0;
    double charge = daily * days;
    c.balance -= charge;
    res.clientsTouched++;
    res.totalCharged += charge;
  }

  res.daysCharged = days;
  Database::saveClients(clients);
  saveState(newLastDate);

  ostringstream det;
  det << "дней=" << days
      << ", клиентов=" << res.clientsTouched
      << ", всего списано=" << fixed << setprecision(2) << res.totalCharged
      << " руб., новая опорная дата=" << newLastDate;
  Logger::log(LogCategory::BILLING, "Тарификация", det.str());

  return res;
}

BillingResult Billing::runDailyChargeIfDue() {
  string today = todayDate();
  string last = loadLastDate();

  if (last.empty()) {
    // первый запуск — фиксируем сегодня как опорную дату, без списания
    saveState(today);
    Logger::log(LogCategory::BILLING, "Инициализация тарификации",
                "опорная дата=" + today);
    return {};
  }

  int days = daysBetween(last, today);
  if (days <= 0) return {};

  return chargeForDays(days, today);
}

