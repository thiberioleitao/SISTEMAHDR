#include "HidrologiaUtils.h"

#include <algorithm>
#include <cmath>

namespace Hidrologia
{
double calcularTcKirpichModificadoMin(double comprimentoTalvegueKm,
                                      double declividadeTalvegue,
                                      double maiorTcMontanteMin)
{
    const double comprimentoKm = std::max(1e-6, comprimentoTalvegueKm);
    const double comprimentoM = comprimentoKm * 1000.0;
    const double declividade = std::max(1e-6, declividadeTalvegue);

    // tc(min) = 60 * 1.42 * [ (L_km^3) / (L_m * S) ]^0.385
    const double tcMin = 60.0 * 1.42
        * std::pow((std::pow(comprimentoKm, 3.0)) / (comprimentoM * declividade), 0.385);

    return std::max(std::max(0.0, maiorTcMontanteMin), std::max(0.0, tcMin));
}
}
