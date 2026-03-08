#include "CalculoHidraulicoCanal.h"

#include <algorithm>
#include <cmath>

double CalculoHidraulicoCanal::velocidadeManning(const SecaoTransversalTrapezoidal& secao,
                                                 double alturaLamina,
                                                 double declividadeFundo,
                                                 double coeficienteManning)
{
    const double y = std::max(0.0, alturaLamina);
    const double s = std::max(0.0, declividadeFundo);
    const double n = std::max(0.0, coeficienteManning);

    if (n <= 0.0 || s <= 0.0) return 0.0;

    const double r = secao.raioHidraulico(y);
    if (r <= 0.0) return 0.0;

    return (1.0 / n) * std::pow(r, 2.0 / 3.0) * std::sqrt(s);
}

double CalculoHidraulicoCanal::vazaoManning(const SecaoTransversalTrapezoidal& secao,
                                            double alturaLamina,
                                            double declividadeFundo,
                                            double coeficienteManning)
{
    const double y = std::max(0.0, alturaLamina);
    const double area = secao.areaMolhada(y);
    const double velocidade = velocidadeManning(secao, y, declividadeFundo, coeficienteManning);
    return area * velocidade;
}

double CalculoHidraulicoCanal::vazaoProjeto(const SecaoTransversalTrapezoidal& secao,
                                            double alturaLaminaProjeto,
                                            double declividadeFundo,
                                            double coeficienteManning)
{
    return vazaoManning(secao,
                        alturaLaminaProjeto,
                        declividadeFundo,
                        coeficienteManning);
}
