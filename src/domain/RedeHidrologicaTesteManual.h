#pragma once

#include "RedeHidrologica.h"

#include <QString>

class RedeHidrologicaTesteManual
{
public:
    static bool criarElementosExemplo(RedeHidrologica* rede, QString* erro = nullptr);

    static bool conectarElementosExemplo(RedeHidrologica* rede, QString* erro = nullptr);

    static bool montarRedeExemplo(RedeHidrologica* rede, QString* erro = nullptr);

    static QString gerarRelatorioExemplo(QString* erro = nullptr);
};
