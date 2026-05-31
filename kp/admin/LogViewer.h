#pragma once

// просмотрщик журнала событий: показывает последние записи из data/logs.txt
// с прокруткой и фильтром по категории (A=AUTH, U=USER, C=CLIENT, B=BILLING,
// S=SYSTEM, R=сброс фильтра)
class LogViewer {
public:
    static void show();
};
