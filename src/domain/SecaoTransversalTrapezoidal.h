#pragma once

#include <algorithm>
#include <cmath>

class SecaoTransversalTrapezoidal
{
public:
    SecaoTransversalTrapezoidal() = default;

    SecaoTransversalTrapezoidal(double larguraFundo,
                                double taludeLateral)
        : m_larguraFundo(std::max(0.0, larguraFundo))
        , m_taludeLateral(std::max(0.0, taludeLateral))
    {
    }

    double larguraFundo() const { return m_larguraFundo; }
    void setLarguraFundo(double valor) { m_larguraFundo = std::max(0.0, valor); }

    double taludeLateral() const { return m_taludeLateral; }
    void setTaludeLateral(double valor) { m_taludeLateral = std::max(0.0, valor); }

    double larguraSuperficial(double alturaLamina) const
    {
        const double y = std::max(0.0, alturaLamina);
        return m_larguraFundo + (2.0 * m_taludeLateral * y);
    }

    double areaMolhada(double alturaLamina) const
    {
        const double y = std::max(0.0, alturaLamina);
        return y * (m_larguraFundo + (m_taludeLateral * y));
    }

    double perimetroMolhado(double alturaLamina) const
    {
        const double y = std::max(0.0, alturaLamina);
        return m_larguraFundo + (2.0 * y * std::sqrt(1.0 + (m_taludeLateral * m_taludeLateral)));
    }

    double raioHidraulico(double alturaLamina) const
    {
        const double p = perimetroMolhado(alturaLamina);
        if (p <= 0.0) return 0.0;
        return areaMolhada(alturaLamina) / p;
    }

private:
    double m_larguraFundo = 0.0;
    double m_taludeLateral = 0.0;
};
