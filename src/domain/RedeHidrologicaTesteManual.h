#pragma once

#include "RedeHidrologica.h"

#include <QString>

class RedeHidrologicaTesteManual
{
public:
    static bool criarElementosExemplo(RedeHidrologica* rede);

    static bool conectarElementosExemplo(RedeHidrologica* rede);

    static bool montarRedeExemplo(RedeHidrologica* rede);

    static QString gerarRelatorioExemplo();
};
