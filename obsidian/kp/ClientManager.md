# ClientManager

> CRUD-операции с базой абонентов.  
> Файл: `clients/ClientManager.h/.cpp`

← [[Управление клиентами]]

Все операции проходят через [[Database]].  
Поиск `findClientsByFields()` фильтрует по непустым полям.  
Удаление `deleteClient()` физически убирает запись из `clients.json`.
