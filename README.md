# MyTupleCMake

Учебный проект на C++23 с примером `tupleVector` и тестами на GoogleTest. Сборка выполняется через CMake, а зависимости устанавливаются автоматически через vcpkg. Поддерживаются MSVC и GCC через MinGW-w64.

## Архитектура `myTuple::tupleVector`

Контейнер `tupleVector` спроектирован с акцентом на низкоуровневый контроль памяти, безопасность на этапе компиляции и совместимость со стандартной библиотекой C++:

* **Единый непрерывный буфер и нулевой оверхед в рантайме (Zero-Overhead Abstraction).** Все элементы кортежа размещаются в одном общем блоке сырой памяти (`char*`), выделяемом за одну аллокацию. Смещения (`offset`) и итоговый размер буфера вычисляются на этапе компиляции с помощью метапрограммирования (`TupleAligner` и `TupleOffsetCalculator`), что исключает накладные расходы на хранение указателей или метаданных в рантайме.
* **Ручной расчет выравнивания памяти (Memory Alignment).** Метафункция `TupleOffsetCalculator` учитывает `alignof(T)` каждого типа в пакете параметров и автоматически вычисляет корректные отступы (padding) по битовой формуле округления вверх. Это гарантирует безопасное чтение и запись данных без неопределенного поведения (UB) и штрафов производительности на уровне CPU.
* **Двухуровневое управление ресурсами (RAII и Move-семантика).** Выделение и освобождение сырого буфера изолировано в базовом классе `tupleVectorBuff` (идиома RAII), а конструирование и разрушение объектов выполнено через `placement new` (`std::allocator_traits::construct`) и явный вызов деструкторов (`destroy`) в наследнике `tupleVector`. Копирование запрещено для исключения случайного дублирования тяжелых буферов, а перемещение (`move`) реализовано за $O(1)$ с занулением исходного указателя.
* **Детерминированный порядок разрушения (LIFO).** При уничтожении контейнера деструкторы элементов вызываются строго в обратном порядке относительно их создания (через генерацию обратной последовательности индексов `make_index_sequence_reverse`), что соответствует стандарту языка C++ для локальных объектов и членов классов.
* **Поддержка пользовательских и STL-совместимых аллокаторов.** Контейнер работает через `std::allocator_traits`, что позволяет передавать как стандартный `std::allocator`, так и собственные реализации:
  * `AllocatorX::allocatorX` — кастомный аллокатор с поддержкой сверхвыровненных типов (`alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__` через `std::align_val_t`), защитой от целочисленного переполнения при расчете размера и опциональным логированием (`PRINT_ALLOCATE_INFO`).
  * `TrackingAllocator` — диагностический аллокатор для модульных тестов, отслеживающий количество вызовов `allocate`/`deallocate` и позволяющий симулировать сбои выделения памяти (`std::bad_alloc`).
* **Типобезопасный доступ с проверками на этапе компиляции (`static_assert`).** Поддерживается получение элементов как по индексу (`get<0>()`, `operator[](std::integral_constant<size_t, 0>{})`), так и по уникальному типу (`get<std::string>()`). При обращении по несуществующему или дублирующемуся типу, а также при выходе индекса за пределы пакета, сборка прерывается с понятным сообщением `static_assert`.
* **Покрытие модульными тестами (GoogleTest).** Проверены граничные сценарии: корректность выравнивания разнородных типов (`char`, `int`, `double`, `std::string`, `std::map`), безопасность перемещения, жизненный цикл буфера и устойчивость при выбросе `std::bad_alloc`.

## Требования

- Windows и один из компиляторов: Visual Studio с нагрузкой **Разработка классических приложений на C++** (MSVC) или MinGW-w64 (GCC).
- CMake версии 3.21 или новее.
- Ninja.
- Установленный vcpkg.

Для сборки MSVC откройте Developer PowerShell for VS и проверьте `Get-Command cl`. Для GCC проверьте, что команды `g++` и `ninja` доступны в `PATH`, а также задайте `VCPKG_ROOT`.

## Сборка и запуск без IDE

1. Откройте **x64 Native Tools Command Prompt for VS** или **Developer PowerShell for VS** из меню «Пуск». В этих терминалах настроено окружение компилятора MSVC; обычный PowerShell для сборки не подходит, пока это окружение не загружено.
2. Укажите путь к установленному vcpkg и перейдите в каталог проекта. В примере vcpkg находится в `C:\tools\vcpkg`. Для **Command Prompt (`cmd.exe`)** выполните:

   ```cmd
   set VCPKG_ROOT=C:\tools\vcpkg
   cd /d C:\WORK\C++\Projects\MyTupleCMake
   ```

   Если вы используете **PowerShell**, команды будут другими:

   ```powershell
   $env:VCPKG_ROOT = "C:\tools\vcpkg"
   Set-Location "C:\WORK\C++\Projects\MyTupleCMake"
   Get-Command cl
   ```

   `Get-Command cl` должен показать путь к `cl.exe`. Если команда не найдена, откройте Developer PowerShell for VS и повторите команды.

   Если vcpkg установлен в другом месте, замените путь на свой.

3. Сконфигурируйте и соберите Debug-версию:

   ```cmd
   cmake --preset x64-debug -B out\build\fresh-debug
   cmake --build out\build\fresh-debug
   ```

   При первой конфигурации vcpkg прочитает `vcpkg.json` и автоматически установит GoogleTest.

4. Запустите программу:

   ```cmd
   out\build\fresh-debug\MyTupleCMake.exe
   ```

5. Запустите тесты:

   ```cmd
   ctest --test-dir out\build\fresh-debug --output-on-failure
   ```

## Сборка Release

В том же терминале выполните:

```cmd
cmake --preset x64-release -B out\build\fresh-release
cmake --build out\build\fresh-release
out\build\fresh-release\MyTupleCMake.exe
ctest --test-dir out\build\fresh-release --output-on-failure
```

Для Debug и Release используются отдельные каталоги сборки. Если CMake сообщает, что кэш создан в другом каталоге, выберите новое имя для каталога после `-B`.

## Сборка GCC (MinGW-w64)

Установите MinGW-w64 с GCC и убедитесь, что `g++` и Ninja доступны в `PATH`. В PowerShell укажите путь к vcpkg и перейдите в каталог проекта:

```powershell
$env:VCPKG_ROOT = "C:\tools\vcpkg"
Set-Location "C:\WORK\C++\Projects\MyTupleCMake"
g++ --version
```

Сконфигурируйте и соберите Debug-версию:

```powershell
cmake --preset mingw-debug
cmake --build out\build\mingw-debug
ctest --test-dir out\build\mingw-debug --output-on-failure
```

Для Release используйте пресет `mingw-release` и каталог `out\build\mingw-release`. Пресеты MinGW используют отдельный triplet vcpkg `x64-mingw-dynamic`; не переиспользуйте для них каталоги сборки MSVC.
