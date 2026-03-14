#pragma once

#include "ModeloTransformacaoChuvaVazao.h"

class BaciaContribuicao; // Forward declaration para evitar dependência circular

class MetodoRacionalTransformacao : public ModeloTransformacaoChuvaVazao
{
public:
    MetodoRacionalTransformacao() = default;

    double vazaoPicoM3s(const BaciaContribuicao& bacia,
                        double intensidadeEfetivaMmH) const override;
};
