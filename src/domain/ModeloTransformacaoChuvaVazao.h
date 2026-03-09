#pragma once

class BaciaContribuicao;

class ModeloTransformacaoChuvaVazao
{
public:
    virtual ~ModeloTransformacaoChuvaVazao() = default;

    virtual double vazaoPicoM3s(const BaciaContribuicao& bacia,
                                double intensidadeEfetivaMmH) const = 0;
};
