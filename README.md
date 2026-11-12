# MyTupleCMake

Учебный проект на C++23 с примером `tupleVector` и тестами на GoogleTest. Сборка выполняется через CMake, а зависимости устанавливаются автоматически через vcpkg. Поддерживаются MSVC и GCC через MinGW-w64.

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
