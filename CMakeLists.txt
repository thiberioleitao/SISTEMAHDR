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
    src/ui/CanalTrapezoidalWidget.cpp
    src/ui/modelos/ModeloTabelaCanais.cpp
    src/ui/modelos/ModeloTabelaBacias.cpp
    src/ui/modelos/ModeloTabelaSecoesTransversais.cpp
    src/ui/modelos/ModeloTabelaUsoOcupacaoSolo.cpp
    src/ui/delegates/DelegadoGeometriaSecaoComboBox.cpp
    src/ui/delegates/DelegadoSecaoTransversalComboBox.cpp
    src/ui/delegates/DelegadoUsoOcupacaoSoloComboBox.cpp
    src/domain/Canal.cpp
    src/domain/Bueiro.cpp
    src/domain/BaciaContribuicao.cpp
    src/domain/CalculoHidraulicoCanal.cpp
    src/domain/TipoUsoOcupacaoSolo.cpp
    src/domain/IDF.cpp
    src/domain/MetodoRacionalTransformacao.cpp
    src/domain/HidrologiaUtils.cpp
    src/domain/RegistroElementosRede.cpp
    src/domain/RedeHidrologicaTesteManual.cpp
    src/domain/RedeHidrologica.cpp
    
    src/domain/RevestimentoCanal.cpp
    src/domain/SeriaPrecipitacoesDiariaHidrowebANA.h
    src/domain/SeriaPrecipitacoesDiariaHidrowebANA.cpp
    src/domain/Canal.h
    src/domain/Bueiro.h
    src/domain/BaciaContribuicao.h
    src/domain/CalculoHidraulicoCanal.h
    src/domain/TipoUsoOcupacaoSolo.h
    
    src/domain/IDF.h
    src/domain/MetodoRacionalTransformacao.h
    src/domain/HidrologiaUtils.h
    src/domain/RegistroElementosRede.h
    src/domain/RedeHidrologicaTesteManual.h
    src/domain/RedeHidrologica.h
    
    src/domain/SecaoTransversalTrapezoidal.h
    src/domain/SecaoTransversalSemicircular.h
    src/domain/RevestimentoCanal.h
    src/ui/CanalTrapezoidalWidget.h
    src/ui/modelos/ModeloTabelaCanais.h
    src/ui/modelos/ModeloTabelaBacias.h
    src/ui/modelos/ModeloTabelaSecoesTransversais.h
    src/ui/modelos/ModeloTabelaUsoOcupacaoSolo.h
    src/ui/delegates/DelegadoGeometriaSecaoComboBox.h
    src/ui/delegates/DelegadoSecaoTransversalComboBox.h
    src/ui/delegates/DelegadoUsoOcupacaoSoloComboBox.h
)

target_include_directories(SISTEMAHDR PRIVATE
    src
)

target_link_libraries(SISTEMAHDR
    Qt6::Widgets
)

if (MSVC)
    # Garante leitura de fonte e literais em UTF-8 no compilador da Microsoft.
    # Qt 6 em MSVC exige conformidade da linguagem e valor correto para __cplusplus.
    target_compile_options(SISTEMAHDR PRIVATE /utf-8 /permissive- /Zc:__cplusplus)
endif()
