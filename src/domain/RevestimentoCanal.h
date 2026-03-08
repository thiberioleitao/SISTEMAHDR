#pragma once

#include <QString>

class RevestimentoCanal
{
public:
    RevestimentoCanal() = default;

    RevestimentoCanal(const QString& material,
                      double coeficienteManning,
                      double espessura);

    const QString& material() const;
    void setMaterial(const QString& valor);

    double coeficienteManning() const;
    void setCoeficienteManning(double valor);

    double espessura() const;
    void setEspessura(double valor);

private:
    QString m_material;
    double m_coeficienteManning = 0.0;
    double m_espessura = 0.0;
};
