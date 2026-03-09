#include "MetodoRacionalTransformacao.h"

#include "BaciaContribuicao.h"

#include <algorithm>

double MetodoRacionalTransformacao::vazaoPicoM3s(const BaciaContribuicao& bacia,
                                                 double intensidadeEfetivaMmH) const
{
    const double c = std::max(0.0, m_coeficienteEscoamento);
    const double i = std::max(0.0, intensidadeEfetivaMmH);
    const double a = std::max(0.0, bacia.areaKm2());

    return 0.278 * c * i * a;
}
