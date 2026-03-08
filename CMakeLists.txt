cmake_minimum_required(VERSION 3.16)

project(SISTEMAHDR LANGUAGES CXX)

# ===============================
# C++ padrão
# ===============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===============================
# Qt
# ===============================
set(CMAKE_PREFIX_PATH "C:/Qt/6.7.0/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ===============================
# Executável
# ===============================
add_executable(SISTEMAHDR
    src/main.cpp
    src/ui/MainWindow.cpp
    src/domain/Canal.cpp
    src/domain/CanalTrecho.cpp
    src/domain/RevestimentoCanal.cpp
    src/domain/CalculoHidraulicoCanal.cpp
    src/domain/SeriaPrecipitacoesDiariaHidrowebANA.h
    src/domain/SeriaPrecipitacoesDiariaHidrowebANA.cpp
    src/domain/Canal.h
    src/domain/CanalTrecho.h
    src/domain/SecaoTransversalTrapezoidal.h
    src/domain/RevestimentoCanal.h
    src/domain/CalculoHidraulicoCanal.h
)

target_include_directories(SISTEMAHDR PRIVATE
    src
)

target_link_libraries(SISTEMAHDR
    Qt6::Widgets
)
