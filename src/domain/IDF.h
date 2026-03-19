#pragma once

class IDF
{
public:
    IDF() = default;

    IDF(double K,
        double m,
        double n,
        double c);

    double K() const;
    void setK(double valor);

    double m() const;
    void setM(double valor);

    double n() const;
    void setN(double valor);

    double c() const;
    void setC(double valor);

    bool valida() const;

    double intensidadeMmH(double tempoRetornoAnos,
                          double duracaoMin) const;

private:
    double m_K = 0.0;
    double m_m = 0.0;
    double m_n = 0.0;
    double m_c = 0.0;
};
