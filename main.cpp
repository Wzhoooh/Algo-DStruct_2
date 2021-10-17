#include <iostream>
#include <algorithm>
#include <type_traits>
#include <iterator>

#include "stack.hpp"

template < typename T1, typename T2 >
struct Pair
{
    Pair() {}
    Pair(const T1& x, const T2& y): first(x), second(y) {}
    template< typename U1, typename U2 >
    Pair(U1&& x, U2&& y): first(std::move(x)), second(std::move(y)) {}
    Pair(const Pair& p) = default;
    Pair(Pair&& p) = default;

    T1 first;
    T2 second;
};

inline long long getMinrun(size_t n)
{
    int r = 0; // станет 1 если среди сдвинутых битов будет хотя бы 1 ненулевой
    while (n >= 64) {
        r |= n & 1;
        n >>= 1;
    }
    return n + r;
}

template < typename BidirectionalIterator >
void advance(BidirectionalIterator p, int dist)
{
    if (dist < 0)
        for (; dist != 0; dist++)
            p--;
    else if (dist > 0)
        for (; dist != 0; dist--)
            p++;

    std::cout << "pointer: " << p << "\n";
}

// https://ru.wikipedia.org/wiki/Сортировка_вставками
template < typename BidirectionalIterator >
void InsertionSort(const BidirectionalIterator begin, const BidirectionalIterator end)
{
    if (begin == end)
        return;

    BidirectionalIterator curP = begin;
    curP++;
    for (; curP != end; curP++)
    {
        bool isBeg = false;
        auto val = *curP;
        auto beforeP = curP;
        beforeP--;
        for (; *beforeP > val;)
        {
            auto k = beforeP;
            k++;
            *k = *beforeP;
            if (beforeP != begin)
                beforeP--;
            else
            {
                isBeg = true;
                break;
            }
        }
        auto insertP = beforeP;
        if (!isBeg)
            insertP++;

        *insertP = val;
    }
}

// https://ru.wikipedia.org/wiki/Timsort
template < typename BidirectionalIterator >
void TimSort(BidirectionalIterator begin, BidirectionalIterator end);
template < typename BidirectionalIterator >
auto splitAndSort(BidirectionalIterator begin, BidirectionalIterator end);
template < typename BidirectionalIterator >
void merge(BidirectionalIterator begin, BidirectionalIterator end);

template < typename BidirectionalIterator >
void TimSort(BidirectionalIterator begin, BidirectionalIterator end)
{
    splitAndSort(begin, end);
    merge(begin, end);
}

template < typename BidirectionalIterator >
auto splitAndSort(BidirectionalIterator begin, BidirectionalIterator end)
{
    Stack<Pair<T, size_t>> subs;
    auto minRun = getMinrun(std::distance(begin, end));
    auto size = std::distance(begin, end);
std::cout << "minrun: " << minRun << "\n";

    // Проверяем на размер 0 или 1
    if (size == 0 || size == 1)
        return;

    /*
        Указатель текущего элемента ставится в начало входного массива.
    */
    auto subBeginP = begin;
    for (; subBeginP != end;)
    {
        /*
            Начиная с текущего элемента, в этом массиве идёт поиск упорядоченного 
            подмассива run. По определению, в run однозна88888888чно войдет текущий элемент 
            и следующий за ним. Если получившийся подмассив упорядочен по убыванию — 
            элементы переставляются так, чтобы они шли по возрастанию.
        */
        // Найдем первый элемент, не равный начальному эл-ту подмассива
        auto p = subBeginP;
        p++;
        for (; p != end; p++)
            if (*p != *subBeginP)
                break;

        if (p == end)
            return;

        if (*p < *subBeginP)
        {
            // Это убывающий подмассив, идем пока массив убывает
            for (; p != end; p++)
            {
                auto bef = p;
                bef--;
                if (*bef < *p)
                    break;
            }
            std::reverse(subBeginP, p);
        }
        else
        {
            // Это возрастающий подмассив, идем пока массив возрастает
            for (; p != end; p++)
            {
                auto bef = p;
                bef--;
                if (*bef > *p)
                    break;
            }
        }

        /*
            Если размер текущего run’а меньше, чем minrun — выбираются следующие 
            за найденным run-ом элементы в количестве minrun-size(run). Таким образом, 
            на выходе будет получен подмассив размером minrun или больше, часть которого 
            (а в идеале — он весь) упорядочена.
        */
        auto subEndP = p;
        auto subSize = std::distance(subBeginP, subEndP);
        if (subSize < minRun)
        {
            // Длина подмассива меньше minrun
            auto maxSubSize = size - std::distance(begin, subBeginP);
            auto dist = std::min(maxSubSize, minRun);
std::cout << "dist: " << dist << "\n";            
            subEndP = subBeginP;
            std::advance(subEndP, dist);
            subSize = dist;
        }
std::cout << "subEndP: " << std::distance(begin, subEndP) << "\n";

        /*
            К данному подмассиву применяется сортировка вставками. Так как размер 
            подмассива невелик и часть его уже упорядочена — сортировка работает 
            быстро и эффективно.
        */
        InsertionSort(subBeginP, subEndP);

        /*
            Указатель текущего элемента ставится на следующий за подмассивом элемент.
        */
        subEndP;

        /*
            Если конец входного массива не достигнут — переход к пункту 2, иначе — конец 
            данного шага.
        */
        // поместим характеристики подмассива в стек
        subs.push( {subBeginP, (size_t)subSize});
        subBeginP = subEndP;
    }
std::cout << "[ ";
for (auto i = begin; i != end; i++)
    std::cout << *i << " ";
std::cout << "]\n";

std::cout << "(\n";
for (; !subs.empty();)
{
    std::cout << *subs.top().first << " " << subs.top().second << "\n";
    subs.pop();
}
std::cout << ")";

    return subs;
}

template < typename BidirectionalIterator >
void merge(BidirectionalIterator begin, BidirectionalIterator end)
{}

int main()
{
    int arr[100] = { 0 };
    for (int i = 0; i < 65; i++)
        arr[i] = -i;
    for (int i = 65; i < 97; i++)
        arr[i] = 100 - i;
    for (int i = 97; i < 100; i++)
        arr[i] = 200 - i;

    std::cout << "[ ";
    for (auto&& i : arr)
        std::cout << i << " ";
    std::cout << "]\n";

    std::cout << "arr size: " << sizeof(arr) / sizeof(arr[0]) << "\n";

    splitAndSort(arr, arr + sizeof(arr) / sizeof(arr[0]));

    std::cout << "finish";
}