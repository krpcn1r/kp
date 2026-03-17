#include <iostream>
#include <conio.h>
#include <string>

void showAdminMenu() {
    bool running = true;
    while (running) {
        system("cls");
        std::cout << "=== ПАНЕЛЬ АДМИНИСТРАТОРА ===\n";
        std::cout << "1. Управление клиентами\n";
        std::cout << "2. Статистика сети\n";
        std::cout << "3. Настройки системы\n";
        std::cout << "0. Выход\n";
        std::cout << "\nВыберите пункт: ";

        char choice = _getch(); // Ждем нажатия цифры

        switch (choice) {
        case '1':
            // Вызов функции из папки clients
            std::cout << "\nОткрываем список клиентов...";
            _getch(); // Пауза
            break;
        case '2':
            std::cout << "\nЗагрузка статистики...";
            _getch();
            break;
        case '3':
            // Настройки
            break;
        case '0':
            running = false; // Выход из цикла меню
            break;
        default:
            // Если нажали что-то другое, ничего не делаем или пишем ошибку
            break;
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    showAdminMenu();
}