# MyTupleCMake

Учебный проект на C++23 с примером `tupleVector` и тестами на GoogleTest. Сборка выполняется через CMake, а зависимости устанавливаются автоматически через vcpkg.

## Требования

- Windows и Visual Studio с установленной нагрузкой **Разработка классических приложений на C++** (компилятор MSVC).
- CMake версии 3.21 или новее.
- Ninja.
- Установленный vcpkg.

Проверьте, что команды `cmake`, `ninja` и `cl` доступны в терминале. Пресеты проекта настроены для Windows и MSVC.

## Сборка и запуск без IDE

1. Откройте **x64 Native Tools Command Prompt for VS** из меню «Пуск». Это обычный терминал с настроенным компилятором MSVC; Visual Studio IDE запускать не нужно.
2. Укажите путь к установленному vcpkg и перейдите в каталог проекта. В примере vcpkg находится в `C:\tools\vcpkg`:

   ```cmd
   set VCPKG_ROOT=C:\tools\vcpkg
   cd /d C:\WORK\C++\Projects\MyTupleCMake
   ```

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
