#pragma once

#include <QString>

class TipoUsoOcupacaoSolo
{
public:
    TipoUsoOcupacaoSolo() = default;
    explicit TipoUsoOcupacaoSolo(const QString& id);

    const QString& id() const;
    void setId(const QString& id);

    double nManning() const;
    void setNManning(double valor);

    double cRunoff() const;
    void setCRunoff(double valor);

    double curveNumber() const;
    void setCurveNumber(double valor);

    double fatorR() const;
    void setFatorR(double valor);

    double fatorK() const;
    void setFatorK(double valor);

    double fatorC() const;
    void setFatorC(double valor);

    double fatorP() const;
    void setFatorP(double valor);

    double densidadeKgM3() const;
    void setDensidadeKgM3(double valor);

private:
    QString m_id;
    double m_nManning = 0.0;
    double m_cRunoff = 0.0;
    double m_curveNumber = 0.0;
    double m_fatorR = 0.0;
    double m_fatorK = 0.0;
    double m_fatorC = 0.0;
    double m_fatorP = 0.0;
    double m_densidadeKgM3 = 0.0;
};
