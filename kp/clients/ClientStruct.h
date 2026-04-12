#pragma once
#include <string>
#include "../core/json.hpp"

struct Client {
    int id;                 
    std::string phoneNumber;  
    std::string fullName;     
    std::string tariffName;    
    double balance;         
    bool isActive;         
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Client, id, phoneNumber, fullName, tariffName, balance, isActive)

