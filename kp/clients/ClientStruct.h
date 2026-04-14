#pragma once
#include <string>
#include "../core/json.hpp"

// структура для хранения всей информации про абонента
struct Client {
    int id; // уникальный номер             
    std::string phoneNumber; // номер мобилы
    std::string fullName; // Фамилия Имя Отчество    
    std::string tariffName; // название тарифа
    double balance; // количество денег на счету        
    bool isActive; // статус активности симки        
};

// макрос для автоматического заполнения структуры из джейсона
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Client, id, phoneNumber, fullName, tariffName, balance, isActive)

