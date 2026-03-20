#pragma once

struct Client {
    int id;                 
    char phoneNumber[12];  
    char fullName[100];     
    char tariffName[32];    
    double balance;         
    bool isActive;         
};

