#pragma once

enum class HomeResult { 
    LOGOUT, 
    EXIT_APP 
};

class HomeMenu {
public:
    static HomeResult show();
};
