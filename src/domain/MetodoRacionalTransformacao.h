#pragma once

class BaciaContribuicao; // Forward declaration para evitar dependência circular

class MetodoRacionalTransformacao
{
public:
    MetodoRacionalTransformacao() = default;

    double vazaoPicoM3s(const BaciaContribuicao& bacia,
                        double intensidadeEfetivaMmH) const;
};
