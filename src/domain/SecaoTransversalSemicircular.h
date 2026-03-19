#pragma once

#include <algorithm>
#include <cmath>

class SecaoTransversalSemicircular
{
public:
    SecaoTransversalSemicircular() = default;

    explicit SecaoTransversalSemicircular(double diametro)
        : m_diametro(std::max(0.0, diametro))
    {
    }

    double diametro() const { return m_diametro; }
    void setDiametro(double valor) { m_diametro = std::max(0.0, valor); }

    double raio() const { return 0.5 * m_diametro; }

    double larguraSuperficial(double alturaLamina) const
    {
        const double d = std::max(0.0, m_diametro);
        if (d <= 0.0) return 0.0;

        const double r = 0.5 * d;
        const double y = std::clamp(alturaLamina, 0.0, d);
        return 2.0 * std::sqrt(std::max(0.0, y * (2.0 * r - y)));
    }

    double areaMolhada(double alturaLamina) const
    {
        const double d = std::max(0.0, m_diametro);
        if (d <= 0.0) return 0.0;

        const double r = 0.5 * d;
        const double y = std::clamp(alturaLamina, 0.0, d);
        if (y <= 0.0) return 0.0;

        const double argumento = std::clamp((r - y) / r, -1.0, 1.0);
        const double theta = 2.0 * std::acos(argumento);
        return 0.5 * r * r * (theta - std::sin(theta));
    }

    double perimetroMolhado(double alturaLamina) const
    {
        const double d = std::max(0.0, m_diametro);
        if (d <= 0.0) return 0.0;

        const double r = 0.5 * d;
        const double y = std::clamp(alturaLamina, 0.0, d);
        if (y <= 0.0) return 0.0;

        const double argumento = std::clamp((r - y) / r, -1.0, 1.0);
        const double theta = 2.0 * std::acos(argumento);
        return r * theta;
    }

    double raioHidraulico(double alturaLamina) const
    {
        const double p = perimetroMolhado(alturaLamina);
        if (p <= 0.0) return 0.0;
        return areaMolhada(alturaLamina) / p;
    }

private:
    double m_diametro = 0.0;
};
