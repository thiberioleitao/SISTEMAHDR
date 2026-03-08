#include "RevestimentoCanal.h"

#include <algorithm>

RevestimentoCanal::RevestimentoCanal(const QString& material,
                                     double coeficienteManning,
                                     double espessura)
    : m_material(material)
    , m_coeficienteManning(std::max(0.0, coeficienteManning))
    , m_espessura(std::max(0.0, espessura))
{
}

const QString& RevestimentoCanal::material() const
{
    return m_material;
}

void RevestimentoCanal::setMaterial(const QString& valor)
{
    m_material = valor;
}

double RevestimentoCanal::coeficienteManning() const
{
    return m_coeficienteManning;
}

void RevestimentoCanal::setCoeficienteManning(double valor)
{
    m_coeficienteManning = std::max(0.0, valor);
}

double RevestimentoCanal::espessura() const
{
    return m_espessura;
}

void RevestimentoCanal::setEspessura(double valor)
{
    m_espessura = std::max(0.0, valor);
}
