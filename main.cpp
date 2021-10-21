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


const static size_t gallopCoef = 7;

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
Pair<BidirectionalIterator, size_t> mergeWithoutGallop(Pair<BidirectionalIterator, size_t> left, 
    Pair<BidirectionalIterator, size_t> right);
template < typename RandomAccessIterator >
Pair<RandomAccessIterator, size_t> merge(Pair<RandomAccessIterator, size_t> left, 
    Pair<RandomAccessIterator, size_t> right);

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
            subEndP = subBeginP;
            std::advance(subEndP, dist);
            subSize = dist;
        }

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
    return subs;
}

template < typename BidirectionalIterator >
Pair<BidirectionalIterator, size_t> mergeWithoutGallop(Pair<BidirectionalIterator, size_t> left, 
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
    size_t tempIndex = 0, rightIndex = 0;
    for (; !(tempIndex == temp.size() && rightIndex == right.second);)
    {
        if (tempIndex == temp.size())
        {
            // reached end of temp array
            *resultIt = *rightIt;
            resultIt++;
            rightIt++;
            rightIndex++;
        }
        else if (rightIndex == right.second)
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
                rightIndex++;
            }
        }
    }
    return {left.first, left.second + right.second};
}

template < typename RandomAccessIterator >
Pair<RandomAccessIterator, size_t> merge(Pair<RandomAccessIterator, size_t> left, 
    Pair<RandomAccessIterator, size_t> right)
{
    auto _val = *left.first; // I do not know how to do this other way
    DynArr<decltype(_val)> temp(left.second);
    // move values from left subarray to temp array
    for (auto i = left.first, j = temp.begin(); j != temp.end(); i++, j++)
        *j = std::move(*i);

    auto tempIt = temp.begin();
    auto rightIt = right.first;
    auto resultIt = left.first;
    size_t numElemsFromThisSub = 0;
    int fromSub = 0; // 0 - initial state, 1 - temp, 2 - right
    for (; !(tempIt == temp.end() && rightIt - right.first == right.second);)
    {
        if (tempIt == temp.end())
            // reached end of temp array
            break;

        else if (rightIt - right.first == right.second)
        {
            // reached end of right array
            for (; tempIt != temp.end(); resultIt++, tempIt++)
                *resultIt = *tempIt;

            break;
        }
        else
        {
            // has not reached end of any array
            if (numElemsFromThisSub == gallopCoef)
            {
                // start galloping mode
                if (fromSub == 1)
                {
                    auto newIt = tempIt;
                    for(;;)
                    {
                        std::ptrdiff_t diff = temp.end() - newIt;
                        auto it = newIt + diff / 2;
                        if (it != newIt && *it < *rightIt)
                            newIt = it;
                        else
                            break;
                    }
                    for (; newIt - tempIt >= 0; resultIt++, tempIt++)
                        *resultIt = *tempIt;
                }
                else if (fromSub == 2)
                {
                    auto newIt = rightIt;
                    for(;;)
                    {
                        std::ptrdiff_t diff = right.first + right.second - newIt;
                        auto it = newIt + diff / 2;
                        if (it != newIt && *it < *tempIt)
                            newIt = it;
                        else
                            break;
                    }
                    for (; newIt - rightIt >= 0; resultIt++, rightIt++)
                        *resultIt = *rightIt;
                }
                else
                    throw std::logic_error("error in galloping mode");

                fromSub = 0;
                numElemsFromThisSub = 0;
            }
            else
            {
                *resultIt = std::min(*tempIt, *rightIt);
                resultIt++;
                if (*tempIt <= *rightIt)
                {
                    tempIt++;
                    if (fromSub == 1)
                        numElemsFromThisSub++;
                    else
                    {
                        fromSub = 1;
                        numElemsFromThisSub = 1;
                    }
                }
                else
                {
                    rightIt++;
                    if (fromSub == 2)
                        numElemsFromThisSub++;
                    else
                    {
                        fromSub = 2;
                        numElemsFromThisSub = 1;
                    }
                }
            }
        }
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
        for (;;)
        {
            auto third = stack.top();
            stack.pop();
            auto second = stack.top();
            stack.pop();
            // stack size >= 2 && y <= x -> merge(x, y)
            if (second.second <= third.second)
            {
                stack.push(merge(third, second));
            }
            // stack size >= 3 && z <= x + y -> merge(y, min(x, z))
            else if (!stack.empty() && stack.top().second <= third.second + second.second)
            {
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
            if (stack.size() == 1)
                break;

        }
    }
    // Производим слияние всех оставшихся элементов в стеке
    for (; stack.size() != 1;)
    {
        auto x = stack.top();
        stack.pop();
        auto y = stack.top();
        stack.pop();
        stack.push(merge(x, y));
    }
}

int main()
{
    DynArr<int> arr1 = generateArr(Pair(99, 40), Pair(10, 20));
    DynArr<int> arr = generateArr(Pair(99, 10), Pair(20, 70), Pair(60, 15));
    std::cout << "Input: " << arr << "\n\n";
    timSort(arr.begin(), arr.end());
    std::cout << "Result: " << arr << "\n\n";
    std::cout << "finished";
}