## Лабораторная работа №2 "Сортировки"
Задача: Реализовать сортировку Timsort. В реализации должны быть выполнены все основные элементы алгоритма: сортировка вставками, поиск последовательностей run, подсчёт minrun, слияние последовательностей run, режим галопа при слиянии.

Функция сортировки timSort() может сортировать любые контейнеры, для которых существует сущность, ведущая себя как итератор произвольного доступа (https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator). Функция timSort() принимает два параметра - итератор на первый элемент, и итератор на элемент после последнего.

Компиляция: 
```bat
g++ --std c++17 main.cpp
``` 
из папки проекта.
