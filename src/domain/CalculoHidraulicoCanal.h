#pragma once

#include <QString>

struct EntradaVerificacaoCanal
{
    double qpHidrologiaM3s = 0.0;
    double qpHidraulicaSMinM3s = 0.0;
    double qpHidraulicaSMaxM3s = 0.0;

    double alturaSMinM = 0.0;
    double alturaMaximaSecaoM = 0.0;
    double folgaMinimaM = 0.0;

    double velocidadeSMaxMps = 0.0;
    double velocidadeMaximaAdmissivelMps = 0.0;

    double toleranciaRelativaQp = 0.02;
};

struct ResultadoVerificacaoCanal
{
    bool criterioQpSMin = false;
    bool criterioQpSMax = false;
    bool criterioAlturaSMin = false;
    bool criterioVelocidadeSMax = false;

    bool todosAtendidos() const
    {
        return criterioQpSMin
            && criterioQpSMax
            && criterioAlturaSMin
            && criterioVelocidadeSMax;
    }

    QString textoStatus(bool valor) const
    {
        return valor ? QString("OK") : QString("NOK");
    }
};

class CalculoHidraulicoCanal
{
public:
    static ResultadoVerificacaoCanal verificarCriterios(const EntradaVerificacaoCanal& entrada);
};