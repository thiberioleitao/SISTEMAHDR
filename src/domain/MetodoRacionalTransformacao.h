#pragma once

#include "ModeloTransformacaoChuvaVazao.h"

#include <algorithm>

class MetodoRacionalTransformacao : public ModeloTransformacaoChuvaVazao
{
public:
    MetodoRacionalTransformacao() = default;
    explicit MetodoRacionalTransformacao(double coeficienteEscoamento)
        : m_coeficienteEscoamento(std::max(0.0, coeficienteEscoamento))
    {
    }

    double coeficienteEscoamento() const { return m_coeficienteEscoamento; }
    void setCoeficienteEscoamento(double valor) { m_coeficienteEscoamento = std::max(0.0, valor); }

    double vazaoPicoM3s(const BaciaContribuicao& bacia,
                        double intensidadeEfetivaMmH) const override;

private:
    // Q = 0.278 * C * I * A (I em mm/h, A em km², Q em m³/s)
    double m_coeficienteEscoamento = 0.0;
};
