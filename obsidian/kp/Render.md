# Render

> Консольный рендер: рамки, таблицы, поля ввода, диалоги.  
> Файл: `core/Render.h/.cpp`

← [[Ядро системы]]

Использует WinAPI: `SetConsoleCursorPosition`, `SetConsoleTextAttribute`.  
Применяется в: [[AuthMenu]], [[Login]], [[Register]], [[ClientMenu]], [[AdminPanel]], [[HomeMenu]].

Ключевая функция `processInput()` — блокирующий ввод строки с поддержкой редактирования.
