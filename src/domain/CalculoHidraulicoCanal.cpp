#include "CalculoHidraulicoCanal.h"

#include <boost/math/tools/roots.hpp>

#include <algorithm>
#include <cstdint>
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

    double yMin = 0.0;
    double yMax = std::max(1e-6, alturaMaximaBusca);

    const auto f = [&](double y) {
        return vazaoManning(secao, y, declividadeFundo, coeficienteManning) - vazaoDesejada;
    };

    double fMin = f(yMin);
    double fMax = f(yMax);

    int expansoes = 0;
    while (fMin * fMax > 0.0 && expansoes < maxIt) {
        yMax *= 2.0;
        fMax = f(yMax);
        ++expansoes;
    }

    if (fMin * fMax > 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    auto criterioParada = [tol](double a, double b) {
        return std::abs(b - a) <= tol;
    };

    std::uintmax_t it = static_cast<std::uintmax_t>(maxIt);
    const auto raiz = boost::math::tools::toms748_solve(
        f,
        yMin,
        yMax,
        fMin,
        fMax,
        criterioParada,
        it);

    return 0.5 * (raiz.first + raiz.second);
}
