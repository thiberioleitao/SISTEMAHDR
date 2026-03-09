#include "CalculoHidraulicoCanal.h"

#include <algorithm>
#include <cmath>
#include <limits>

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

double CalculoHidraulicoCanal::laminaParaVazao(const SecaoTransversalTrapezoidal& secao,
                                               double vazaoDesejada,
                                               double declividadeFundo,
                                               double coeficienteManning,
                                               double tolerancia,
                                               int maxIteracoes,
                                               double alturaMaximaBusca)
{
    if (vazaoDesejada <= 0.0) return 0.0;
    if (declividadeFundo <= 0.0 || coeficienteManning <= 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double tol = std::max(1e-12, tolerancia);
    const int maxIt = std::max(1, maxIteracoes);
    const double yMaxFisico = std::max(1e-6, alturaMaximaBusca);

    const auto f = [&](double y) {
        return vazaoManning(secao, y, declividadeFundo, coeficienteManning) - vazaoDesejada;
    };

    // 1) Newton-Raphson (derivada numérica central)
    double y = 0.5 * yMaxFisico; // chute inicial interno
    for (int i = 0; i < maxIt; ++i) {
        const double fy = f(y);
        if (std::abs(fy) <= tol) return y;

        const double h = std::max(1e-8, 1e-6 * std::max(1.0, y));
        const double yMenos = std::max(0.0, y - h);
        const double yMais = std::min(yMaxFisico, y + h);
        const double intervalo = yMais - yMenos;
        if (intervalo <= 0.0) break;

        const double dFy = (f(yMais) - f(yMenos)) / intervalo;
        if (!std::isfinite(dFy) || std::abs(dFy) < 1e-14) break;

        const double yNovo = y - (fy / dFy);
        if (!std::isfinite(yNovo)) break;

        y = std::clamp(yNovo, 0.0, yMaxFisico);
    }

    // 2) Fallback: bisseção no intervalo físico [0, yMaxFisico]
    double yMin = 0.0;
    double yMax = yMaxFisico;

    double fMin = f(yMin);
    double fMax = f(yMax);

    if (fMin * fMax > 0.0) {
        // Sem raiz no domínio físico da seção: vazão exige lâmina acima da altura máxima.
        return std::numeric_limits<double>::quiet_NaN();
    }

    for (int i = 0; i < maxIt; ++i) {
        const double yMid = 0.5 * (yMin + yMax);
        const double fMid = f(yMid);

        if (std::abs(fMid) <= tol || std::abs(yMax - yMin) <= tol) {
            return yMid;
        }

        if (fMin * fMid <= 0.0) {
            yMax = yMid;
            fMax = fMid;
        }
        else {
            yMin = yMid;
            fMin = fMid;
        }
    }

    return 0.5 * (yMin + yMax);
}
