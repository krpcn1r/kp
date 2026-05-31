#pragma once
#include <string>
#include <vector>

// категории событий для удобной фильтрации в просмотрщике логов
enum class LogCategory {
    AUTH,     // вход/выход/регистрация
    USER,     // изменения учетных записей администратором
    CLIENT,   // изменения абонентов оператором
    BILLING,  // автоматическое списание средств
    SYSTEM    // системные события (бэкап, восстановление и т.п.)
};

// одна разобранная запись лога
struct LogEntry {
    std::string timestamp;  // ISO-подобный формат YYYY-MM-DD HH:MM:SS
    std::string actor;      // логин пользователя или "system"
    LogCategory category;
    std::string action;
    std::string details;
};

// простой логгер с append-записью в data/logs.txt
class Logger {
public:
    // запись с автоопределением актора (берётся текущий залогиненный юзер,
    // если никого нет — "system")
    static void log(LogCategory cat, const std::string& action, const std::string& details = "");
    // запись с явным указанием актора (нужно, например, для логина — там ещё
    // нет залогиненного юзера на момент попытки входа)
    static void logAs(const std::string& actor, LogCategory cat, const std::string& action, const std::string& details = "");

    // чтение всех записей из файла (для просмотрщика)
    static std::vector<LogEntry> loadEntries();

    static std::string categoryToString(LogCategory cat);
    static LogCategory categoryFromString(const std::string& s);

private:
    static const std::string LOG_FILE;
    static std::string nowTimestamp();
    static std::string currentActor();
    static void appendLine(const std::string& line);
};
