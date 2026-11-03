#pragma once

#include <cstddef>
#include <utility>

template <size_t Index, typename... Ts>
struct MyTypeAt;

// Это сердце алгоритма. Когда Index больше нуля, компилятор выбирает эту специализацию.
//   • typename Head, typename... Tail — это ключевой трюк.Компилятор берет ваш пакет типов и насильно разделяет его на две части :
//       Head(самый первый, «головной» тип) и Tail... (пакет из всех оставшихся «хвостов»).
//   • using type = typename MyTypeAt<Index - 1, Tail...>::type; — здесь запускается рекурсия.Раз первый тип(Head) нам не подошел
//       (ведь индекс еще не равен 0), мы его просто выбрасываем!Мы говорим компилятору : «Перейди на следующий шаг поиска : уменьши индекс
//       на единицу(Index - 1) и ищи дальше, но уже только внутри оставшегося хвоста(Tail...)».
template <size_t Index, typename Head, typename... Tail>
struct MyTypeAt<Index, Head, Tail...>
{
    using type = typename MyTypeAt<Index - 1, Tail...>::type;
};

template <typename Head, typename... Tail>
struct MyTypeAt<0, Head, Tail...>
{
    using type = Head;
};

template <size_t Index, typename... Ts>
using MyTypeAt_t = typename MyTypeAt<Index, Ts...>::type;
//****************************************************************************************************************
//
//  Это Primary Template (основной шаблон)
template <size_t CurrentIdx, typename... Types>
struct TupleOffsetCalculator;
//  Это Частичная специализация (Partial Specialization) для случая, когда CurrentIdx равен 0
//  • static_assert здесь не нужен, потому что для первого элемента всё очевидно:
//  • offset = 0: Самый первый элемент всегда гарантированно кладется в начало буфера(оффсет 0).Адрес начала буфера, который выдает
//      аллокатор, уже выровнен под максимальный тип, поэтому дополнительное выравнивание здесь не требуется.
//  • next_offset = sizeof(T) : Координата, где первый элемент заканчивается.Она равна его размеру.Если T — это int, то первый элемент
//      занимает байты с 0 по 3, а закончится на отметке 4.
template <typename T, typename... Tail>
struct TupleOffsetCalculator<0, T, Tail...>
{
    static constexpr size_t offset = 0;
    static constexpr size_t next_offset = sizeof(T);
};
//  Компилятор видит, что нам нужен, например, индекс 2. Он берет эту структуру и начинает вычислять её внутренние переменные одну за другой.
template <size_t CurrentIdx, typename T, typename... Tail>
struct TupleOffsetCalculator<CurrentIdx, T, Tail...>
{
    //  • Что делает : Запускает рекурсию «назад».Она просит компилятор : «Сгенерируй такую же структуру, но для предыдущего индекса
    //      (CurrentIdx - 1), и дай мне её константу next_offset».
    //  • Суть : Мы узнаем, на каком байте в памяти завершилась запись предыдущего объекта, еще до учета требований выравнивания текущего
    //      объекта.Поэтому переменная называется raw(«сырая»).
    static constexpr size_t prev_raw = TupleOffsetCalculator<CurrentIdx - 1, T, Tail...>::next_offset;
    // • Что делает: С помощью вашего кастомного метахелпера определяет, какой именно тип данных скрывается под индексом CurrentIdx в
    //      общем пакете.
    //  • Пример : Если мы считаем смещение для индекса 2 в пакете <int, double, std::string>, то CurrentType станет равен std::string.
    //      Нам жизненно необходимо знать этот тип, чтобы вытащить его alignof и sizeof.
    using CurrentType = MyTypeAt_t<CurrentIdx, T, Tail...>;
    //  • Что делает: Берет сырой конец предыдущего элемента (prev_raw), берет требование выравнивания текущего типа (alignof(CurrentType)) и
    //      пропускает их через ту самую битовую формулу округления вверх, которую мы детально разобрали.
    //  • Результат : Мы получаем идеальное, безопасное для процессора смещение в байтах, с которого начнется наш текущий элемент.Все
    //      «дыры»(padding) между элементами автоматически учтены.
    static constexpr size_t offset = (prev_raw + alignof(CurrentType) - 1) & ~(alignof(CurrentType) - 1);
    //  • Что делает: Прибавляет к только что найденному идеальному началу элемента offset его физический размер в байтах (sizeof(CurrentType)).
    //  • Суть : Мы фиксируем точку финиша текущего элемента в буфере.Именно это число будет передано дальше «вверх» по цепочке
    //      рекурсии для следующего элемента(в его переменную prev_raw), либо вернется в метод get_total_size(), если этот элемент был последним.
    static constexpr size_t next_offset = offset + sizeof(CurrentType);
};
//****************************************************************************************************************
//
//  TupleAligner сам по себе ничего не считает. Он является красивым фасадом (интерфейсом) для рекурсивной структуры TupleOffsetCalculator.
template <typename... Types>
struct TupleAligner
{
    static constexpr size_t Count = sizeof...(Types);
    //  1. template <size_t TargetIdx>: Индекс передается как параметр шаблона (в угловых скобках), а не как обычный аргумент функции.
    //      Это значит, что индекс должен быть известен еще при написании кода (например, get_offset<2>()).
    //  2. static_assert(...) : Это проверка на этапе компиляции.Если в вашем векторе 3 элемента, а вы попытаетесь вызвать get_offset<5>(),
    //      программа даже не скомпилируется.Вы получите понятную ошибку прямо в IDE, что защищает от выхода за границы массива в рантайме.
    //  3. TupleOffsetCalculator<TargetIdx, Types...>::offset : Компилятор инстанцирует(создает в памяти шаблонов) структуру
    //      TupleOffsetCalculator конкретно для шага TargetIdx.Из этой структуры вытаскивается константа offset, которая уже была высчитана по
    //      битовой формуле выравнивания.
    template <size_t TargetIdx>
    static constexpr size_t get_offset()
    {
        static_assert(TargetIdx < Count, "Index out of bounds in get_offset");
        return TupleOffsetCalculator<TargetIdx, Types...>::offset;
    }
    //  1. if constexpr (Count == 0): Ключевое слово constexpr здесь критически важно. Это ветвление во время компиляции. Если ваш tupleVector
    //      пустой (в него не передали типов), компилятор полностью выбросит ветку else и сгенерирует функцию, которая просто возвращает 0.
    //  2. Count - 1: Последний элемент в пакете параметров всегда имеет индекс на единицу меньше, чем общее количество типов(так как
    //      индексация начинается с 0).
    //  3. TupleOffsetCalculator<Count - 1, Types...>::next_offset : Метод обращается к калькулятору для самого последнего элемента.Константа
    //      next_offset внутри калькулятора хранит координату, где этот последний элемент заканчивается.
    static constexpr size_t get_total_size()
    {
        if constexpr (Count == 0)
            return 0;
        else
            return TupleOffsetCalculator<Count - 1, Types...>::next_offset;
    }
};
//***********************************************************************************************************************
template <typename Target, size_t CurrentIdx, typename... Ts>
struct MyIndexOfImpl;

// Рекурсивный случай: тип совпал с Head!
//  • С помощью метафункции std::is_same_v<Target, Head> компилятор проверяет: совпадает ли искомый тип с текущим «головным» типом.
//  • Если совпадает(true) : переменная value на данном шаге инициализируется значением CurrentIdx.Поиск окончен.
//  • Если не совпадает(false) : компилятор идет в ветку else и запрашивает MyIndexOfImpl<Target, CurrentIdx + 1, Tail...>::value.
//      То есть запускается следующий шаг рекурсии : индекс увеличивается на единицу, а первый элемент отбрасывается.
template <typename Target, size_t CurrentIdx, typename Head, typename... Tail>
struct MyIndexOfImpl<Target, CurrentIdx, Head, Tail...>
{
    static constexpr size_t value = std::is_same_v<Target, Head>
                                        ? CurrentIdx
                                        : MyIndexOfImpl<Target, CurrentIdx + 1, Tail...>::value;
};

// Терминальный случай: пакет пуст, тип не найден. Возвращаем специальный маркер (size_t)-1
// Специализация для пустого пакета параметров (когда после Tail... ничего не осталось). Если компилятор дошел до этого шаблона, значит,
// тип Target не совпал ни с одним элементом в пакете. Метафункция возвращает специальный маркер ошибки: максимальное значение для size_t
// (в C++ запись static_cast<size_t>(-1) из-за особенностей знакового представления превращается в 18446744073709551615 на x64).
template <typename Target, size_t CurrentIdx>
struct MyIndexOfImpl<Target, CurrentIdx>
{
    static constexpr size_t value = static_cast<size_t>(-1);
};

// Удобный интерфейс для внешнего использования
template <typename Target, typename... Ts>
inline constexpr size_t MyIndexOf_v = MyIndexOfImpl<Target, 0, Ts...>::value;

// Метафункция для проверки уникальности типа (чтобы защититься от дубликатов типов)
template <typename Target, typename... Ts>
inline constexpr size_t CountType_v = (static_cast<size_t>(std::is_same_v<Target, Ts>) + ...);