#pragma once
#include <string>
#include "../core/json.hpp"

// структура для хранения информации о тарифном плане
struct Tariff {
    int id;                    // уникальный номер тарифа
    std::string name;          // название тарифа
    double pricePerMonth;      // стоимость в месяц
    int speedMbps;             // скорость в Мбит/с
    std::string description;   // краткое описание
};

// автоматическая JSON-сериализация структуры
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tariff, id, name, pricePerMonth, speedMbps, description)
