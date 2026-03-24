#include "TipoUsoOcupacaoSolo.h"

#include <algorithm>

TipoUsoOcupacaoSolo::TipoUsoOcupacaoSolo(const QString& id)
{
    setId(id);
}

const QString& TipoUsoOcupacaoSolo::id() const
{
    return m_id;
}

void TipoUsoOcupacaoSolo::setId(const QString& id)
{
    m_id = id.trimmed();
}

double TipoUsoOcupacaoSolo::nManning() const
{
    return std::max(0.0, m_nManning);
}

void TipoUsoOcupacaoSolo::setNManning(double valor)
{
    m_nManning = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::cRunoff() const
{
    return std::max(0.0, m_cRunoff);
}

void TipoUsoOcupacaoSolo::setCRunoff(double valor)
{
    m_cRunoff = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::curveNumber() const
{
    return std::max(0.0, m_curveNumber);
}

void TipoUsoOcupacaoSolo::setCurveNumber(double valor)
{
    m_curveNumber = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::fatorR() const
{
    return std::max(0.0, m_fatorR);
}

void TipoUsoOcupacaoSolo::setFatorR(double valor)
{
    m_fatorR = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::fatorK() const
{
    return std::max(0.0, m_fatorK);
}

void TipoUsoOcupacaoSolo::setFatorK(double valor)
{
    m_fatorK = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::fatorC() const
{
    return std::max(0.0, m_fatorC);
}

void TipoUsoOcupacaoSolo::setFatorC(double valor)
{
    m_fatorC = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::fatorP() const
{
    return std::max(0.0, m_fatorP);
}

void TipoUsoOcupacaoSolo::setFatorP(double valor)
{
    m_fatorP = std::max(0.0, valor);
}

double TipoUsoOcupacaoSolo::densidadeKgM3() const
{
    return std::max(0.0, m_densidadeKgM3);
}

void TipoUsoOcupacaoSolo::setDensidadeKgM3(double valor)
{
    m_densidadeKgM3 = std::max(0.0, valor);
}
