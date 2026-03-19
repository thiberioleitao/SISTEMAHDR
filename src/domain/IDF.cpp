#include "IDF.h"

#include <algorithm>
#include <cmath>

IDF::IDF(double K,
         double m,
         double n,
         double c)
    : m_K(std::max(0.0, K))
    , m_m(std::max(0.0, m))
    , m_n(std::max(0.0, n))
    , m_c(std::max(0.0, c))
{
}

double IDF::K() const
{
    return m_K;
}

void IDF::setK(double valor)
{
    m_K = std::max(0.0, valor);
}

double IDF::m() const
{
    return m_m;
}

void IDF::setM(double valor)
{
    m_m = std::max(0.0, valor);
}

double IDF::n() const
{
    return m_n;
}

void IDF::setN(double valor)
{
    m_n = std::max(0.0, valor);
}

double IDF::c() const
{
    return m_c;
}

void IDF::setC(double valor)
{
    m_c = std::max(0.0, valor);
}

bool IDF::valida() const
{
    return m_K > 0.0 && m_n > 0.0;
}

double IDF::intensidadeMmH(double tempoRetornoAnos,
                           double duracaoMin) const
{
    if (!valida()) return 0.0;

    // i (mm/h) = K * TR^m / (t + c)^n
    const double tr = std::max(1e-6, tempoRetornoAnos);
    const double t = std::max(1e-6, duracaoMin);
    const double denominador = std::pow(t + m_c, m_n);
    if (denominador <= 0.0) return 0.0;

    const double numerador = m_K * std::pow(tr, m_m);
    return std::max(0.0, numerador / denominador);
}
