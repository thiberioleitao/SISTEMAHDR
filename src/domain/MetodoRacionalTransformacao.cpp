#include "MetodoRacionalTransformacao.h"

#include "BaciaContribuicao.h"

#include <algorithm>

double MetodoRacionalTransformacao::vazaoPicoM3s(const BaciaContribuicao& bacia,
                                                 double intensidadeEfetivaMmH) const
{
    // C_10: coeficiente de escoamento adotado para TR de 10 anos.
    const double c = std::max(0.0, bacia.C_10());
    const double i = std::max(0.0, intensidadeEfetivaMmH);
    const double a = std::max(0.0, bacia.areaKm2());

    return 0.278 * c * i * a;
}
