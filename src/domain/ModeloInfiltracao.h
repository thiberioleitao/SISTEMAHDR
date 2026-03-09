#pragma once

class BaciaContribuicao;

class ModeloInfiltracao
{
public:
    virtual ~ModeloInfiltracao() = default;

    virtual double intensidadeEfetivaMmH(const BaciaContribuicao& bacia,
                                         double intensidadeChuvaBrutaMmH) const = 0;
};
