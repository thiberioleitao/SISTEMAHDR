#pragma once

#include "SecaoTransversalTrapezoidal.h"

class CalculoHidraulicoCanal
{
public:
    static double velocidadeManning(const SecaoTransversalTrapezoidal& secao,
                                    double alturaLamina,
                                    double declividadeFundo,
                                    double coeficienteManning);

    static double vazaoManning(const SecaoTransversalTrapezoidal& secao,
                               double alturaLamina,
                               double declividadeFundo,
                               double coeficienteManning);

    static double vazaoProjeto(const SecaoTransversalTrapezoidal& secao,
                               double alturaLaminaProjeto,
                               double declividadeFundo,
                               double coeficienteManning);
};
