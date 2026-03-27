#include "CalculoHidraulicoCanal.h"

#include <algorithm>
#include <cmath>

namespace
{
bool compararQpComTolerancia(double qpHidrologia,
                             double qpHidraulica,
                             double toleranciaRelativa)
{
    const double qh = std::max(0.0, qpHidrologia);
    const double qman = std::max(0.0, qpHidraulica);

    if (qh <= 0.0 && qman <= 0.0) return true;

    const double tolerancia = std::max(0.0, toleranciaRelativa);
    const double referencia = std::max(1e-9, qh);
    const double erroRelativo = std::abs(qman - qh) / referencia;
    return erroRelativo <= tolerancia;
}
}

ResultadoVerificacaoCanal CalculoHidraulicoCanal::verificarCriterios(const EntradaVerificacaoCanal& entrada)
{
    ResultadoVerificacaoCanal resultado;

    // Qp hidrologia deve ser compatível com Qp hidráulica para Smin e Smax.
    resultado.criterioQpSMin = compararQpComTolerancia(
        entrada.qpHidrologiaM3s,
        entrada.qpHidraulicaSMinM3s,
        entrada.toleranciaRelativaQp);

    resultado.criterioQpSMax = compararQpComTolerancia(
        entrada.qpHidrologiaM3s,
        entrada.qpHidraulicaSMaxM3s,
        entrada.toleranciaRelativaQp);

    // Altura para Smin: h <= hmax - folga.
    const double alturaLimiteSMinM = std::max(0.0, entrada.alturaMaximaSecaoM - entrada.folgaMinimaM);
    resultado.criterioAlturaSMin = std::max(0.0, entrada.alturaSMinM) <= alturaLimiteSMinM;

    // Velocidade para Smax: V <= Vmax admissível.
    const double vMaxAdmissivel = std::max(0.0, entrada.velocidadeMaximaAdmissivelMps);
    resultado.criterioVelocidadeSMax = std::max(0.0, entrada.velocidadeSMaxMps) <= vMaxAdmissivel;

    return resultado;
}