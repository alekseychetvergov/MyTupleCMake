# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.21)
# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(MyTupleCMake VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Описываем исполняемый файл и все исходники (.cpp)
add_executable(MyTupleCMake
    src/main.cpp
    src/Example.cpp
)

# Указываем путь к заголовочным файлам (.hpp) через таргет-ориентированный подход
target_include_directories(MyTupleCMake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Настройка флагов компилятора для строгости (полезно для коммерческих проектов)
if(MSVC)
    target_compile_options(MyTupleCMake PRIVATE /W4 /WX)
else()
    target_compile_options(MyTupleCMake PRIVATE -Wall -Wextra -Werror)
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(tests)
endif()
# Включите подпроекты.
#add_subdirectory ("MyTupleCMake")
