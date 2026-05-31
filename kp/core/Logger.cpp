#include "Logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../auth/AuthManager.h"

using namespace std;

const string Logger::LOG_FILE = "data/logs.txt";

string Logger::categoryToString(LogCategory cat) {
    switch (cat) {
        case LogCategory::AUTH:
            return "AUTH";
        case LogCategory::USER:
            return "USER";
        case LogCategory::CLIENT:
            return "CLIENT";
        case LogCategory::BILLING:
            return "BILLING";
        case LogCategory::SYSTEM:
            return "SYSTEM";
    }
    return "SYSTEM";
}

LogCategory Logger::categoryFromString(const string& s) {
    if (s == "AUTH") {
        return LogCategory::AUTH;
    }
    if (s == "USER") {
        return LogCategory::USER;
    }
    if (s == "CLIENT") {
        return LogCategory::CLIENT;
    }
    if (s == "BILLING") {
        return LogCategory::BILLING;
    }
    return LogCategory::SYSTEM;
}

string Logger::nowTimestamp() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm local_tm;
    localtime_s(&local_tm, &t);
    ostringstream oss;
    oss << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

string Logger::currentActor() {
    if (AuthManager::isUserLoggedIn()) {
        return AuthManager::getCurrentUser().login;
    }
    return "system";
}

void Logger::appendLine(const string& line) {
    try {
        if (!filesystem::exists("data")) {
            filesystem::create_directories("data");
        }
    } catch (...) {
    }
    ofstream file(LOG_FILE, ios::app);
    if (!file.is_open()) {
        return;
    }
    file << line << '\n';
}

void Logger::log(LogCategory cat, const string& action, const string& details) {
    logAs(currentActor(), cat, action, details);
}

void Logger::logAs(const string& actor, LogCategory cat, const string& action, const string& details) {
    ostringstream oss;
    oss << "[" << nowTimestamp() << "] "
        << "[" << actor << "] "
        << "[" << categoryToString(cat) << "] "
        << action;
    if (!details.empty()) {
        oss << " :: " << details;
    }
    appendLine(oss.str());
}

vector<LogEntry> Logger::loadEntries() {
    vector<LogEntry> entries;
    ifstream file(LOG_FILE);
    if (!file.is_open()) {
        return entries;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        LogEntry e;
        size_t p = 0;

        auto readBracket = [&](string& out) -> bool {
            if (p >= line.size() || line[p] != '[') {
                return false;
            }
            size_t end = line.find(']', p);
            if (end == string::npos) {
                return false;
            }
            out = line.substr(p + 1, end - p - 1);
            p = end + 1;
            while (p < line.size() && line[p] == ' ') {
                p++;
            }
            return true;
        };

        string catStr;
        if (!readBracket(e.timestamp) || !readBracket(e.actor) ||
            !readBracket(catStr)) {
            e.timestamp = "";
            e.actor = "";
            e.category = LogCategory::SYSTEM;
            e.action = line;
            entries.push_back(e);
            continue;
        }
        e.category = categoryFromString(catStr);

        // оставшаяся часть: action [:: details]
        string rest = line.substr(p);
        size_t sep = rest.find(" :: ");
        if (sep != string::npos) {
            e.action = rest.substr(0, sep);
            e.details = rest.substr(sep + 4);
        } else {
            e.action = rest;
        }
        entries.push_back(e);
    }
    return entries;
}
