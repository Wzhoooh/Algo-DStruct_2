#include <iostream>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <stdexcept>

#include "stack.hpp"
#include "dynamic_array.hpp"

template < typename T1, typename T2 >
struct Pair
{
    Pair() {}
    Pair(const T1& x, const T2& y): first(x), second(y) {}
    template< typename U1, typename U2 >
    Pair(U1&& x, U2&& y): first(std::move(x)), second(std::move(y)) {}
    Pair(const Pair& p) = default;
    Pair(Pair&& p) = default;

    Pair& operator=(const Pair& other) = default;

    T1 first;
    T2 second;
};

// several pairs
template < typename Arg = Pair<int, int>> 
DynArr<int> generateArr(Arg arg)
{
    int from = arg.first;
    int to = arg.second;

    DynArr<int> arr(std::abs(to - from) + 1);
    for (size_t i = 0; i <= std::abs(to - from); i++)
        arr[i] = from + i * (int(to - from > 0) - int(to - from < 0));

    return arr;
}
template < typename Arg = Pair<int, int>, typename... Args > 
DynArr<int> generateArr(Arg arg, Args... args)
{
    int from = arg.first;
    int to = arg.second;
    DynArr<int> oldArr = generateArr(args...);
    DynArr<int> newArr(std::abs(to - from) + 1 + oldArr.size());
    for (size_t i = 0; i <= std::abs(to - from); i++)
        newArr[i] = from + i * (int(to - from > 0) - int(to - from < 0));

    for (size_t i = std::abs(to - from) + 1; i < newArr.size(); i++)
        newArr[i] = oldArr[i-std::abs(to - from)-1];

    return newArr;
}

// inline long long getMinrun(size_t n)
// {
//     int r = 0; // станет 1 если среди сдвинутых битов будет хотя бы 1 ненулевой
//     while (n >= 64) {
//         r |= n & 1;
//         n >>= 1;
//     }
//     return n + r;
// }
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
void timSort(const BidirectionalIterator begin, const BidirectionalIterator end);
template < typename BidirectionalIterator >
Stack<Pair<BidirectionalIterator, size_t>> splitAndSort(const BidirectionalIterator begin, 
    const BidirectionalIterator end);
template < typename BidirectionalIterator >
void mergeAll(Stack<Pair<BidirectionalIterator, size_t>>, const BidirectionalIterator begin, 
    const BidirectionalIterator end);
template < typename BidirectionalIterator >
Pair<BidirectionalIterator, size_t> merge(Pair<BidirectionalIterator, size_t> fir, 
    Pair<BidirectionalIterator, size_t> secEnd);

template < typename BidirectionalIterator >
void timSort(const BidirectionalIterator begin, const BidirectionalIterator end)
{
    mergeAll(splitAndSort(begin, end), begin, end);
}

template < typename BidirectionalIterator >
Stack<Pair<BidirectionalIterator, size_t>> splitAndSort(const BidirectionalIterator begin, 
    const BidirectionalIterator end)
{
    Stack<Pair<BidirectionalIterator, size_t>> subs;
    auto minRun = getMinrun(std::distance(begin, end));
    auto size = std::distance(begin, end);
std::cout << "minrun: " << minRun << "\n";

    // Проверяем на размер 0 или 1
    if (size == 0 || size == 1)
        return {};

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
            return {};

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

    return subs;
}

template < typename BidirectionalIterator >
Pair<BidirectionalIterator, size_t> merge(Pair<BidirectionalIterator, size_t> left, 
    Pair<BidirectionalIterator, size_t> right)
{
    auto _val = *left.first; // I do not know how to do this other way
    DynArr<decltype(_val)> temp(left.second);
    // move values from left subarray to temp array
    for (auto i = left.first, j = temp.begin(); j != temp.end(); i++, j++)
        *j = std::move(*i);

    auto tempIt = temp.begin();
    auto rightIt = right.first;
    auto resultIt = left.first;
    size_t tempIndex = 0, biggerIndex = 0;
    for (; !(tempIndex == temp.size() && biggerIndex == right.second);)
    {
        if (tempIndex == temp.size())
        {
            // reached end of temp array
            *resultIt = *rightIt;
            resultIt++;
            rightIt++;
            biggerIndex++;
        }
        else if (biggerIndex == right.second)
        {
            // reached end of right array
            *resultIt = *tempIt;
            resultIt++;
            tempIt++;
            tempIndex++;
        }
        else
        {
            // has not reached end of any array
            *resultIt = std::min(*tempIt, *rightIt);
            resultIt++;
            if (*tempIt <= *rightIt)
            {
                tempIt++;
                tempIndex++;
            }
            else
            {
                rightIt++;
                biggerIndex++;
            }
        }
// for (auto i = smaller.first; i != resultIt; i++)
//     std::cout << *i << " ";
// std::cout << "\n";
    }
    return {left.first, left.second + right.second};
}

template < typename BidirectionalIterator >
void mergeAll(Stack<Pair<BidirectionalIterator, size_t>> subs, BidirectionalIterator begin, BidirectionalIterator end)
{
    if (subs.size() == 0)
        return;

    // Подмассивы subs расположены в обратном порядке!
    // first = z, second = y, third = x

    decltype(subs) stack;
    stack.push(subs.top()); // first
    subs.pop();
    for (; subs.size() != 0;) // Добавляем новые эл-ты в стек, пока не останется новых элементов
    {
        // Добавляем новый элемент в стек
        stack.push(subs.top()); // third
        subs.pop();
std::cout << "0---\n";
std::cout << "stack.size(): " << stack.size()  << " " << "\n";
        for (;;)
        {
            auto third = stack.top();
            stack.pop();
            auto second = stack.top();
            stack.pop();
std::cout << "third: " << std::distance(begin, third.first) << " " << third.second << "\n";
std::cout << "second: " << std::distance(begin, second.first) << " " << second.second << "\n";            
std::cout << "1---\n";
            // stack size >= 2 && y <= x -> merge(x, y)
            if (second.second <= third.second)
            {
std::cout << "merging(x, y)\n";
                stack.push(merge(third, second));
            }
            // stack size >= 3 && z <= x + y -> merge(y, min(x, z))
            else if (!stack.empty() && stack.top().second <= third.second + second.second)
            {
std::cout << "merging(x, y, z)\n";                
std::cout << "stack.top(): " << std::distance(begin, stack.top().first) << " " << stack.top().second << "\n";
                if (third.second <= stack.top().second)
                    stack.push(merge(third, second));
                else
                {
                    stack.push(third);
                    auto first = stack.top();
                    stack.pop();
                    stack.push(merge(second, first));
                }   
            }
            else
            {
                stack.push(second);
                stack.push(third);
                break;
            }
std::cout << "2---\n";
            if (stack.size() == 1)
                break;

std::cout << "3---\n";
        }
    }
std::cout << "range finish---\n";    
    // Производим слияние всех оставшихся элементов в стеке
    for (; stack.size() != 1;)
    {
        auto x = stack.top();
        stack.pop();
        auto y = stack.top();
        stack.pop();
        stack.push(merge(x, y));
    }
std::cout << "last el: " << std::distance(begin, stack.top().first) << " " << stack.top().second << "\n";
}

int main()
{
    DynArr<int> arr = generateArr(Pair(53, -30), Pair(30, 55));
    std::cout << "[ ";
    for (auto&& i : arr)
        std::cout << i << " ";
    std::cout << "]\n";

    std::cout << "arr size: " << arr.size() << "\n";
    timSort(arr.begin(), arr.end());

    std::cout << "[ ";
    for (auto&& i : arr)
        std::cout << i << " ";
    std::cout << "]\n";

    std::cout << "finish";
}