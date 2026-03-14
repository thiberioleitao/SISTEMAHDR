#pragma once

#include "RedeHidrologica.h"

#include <functional>
#include <QMap>
#include <QString>

enum class MetodologiaAcumulacaoVazao
{
    SomaDasAreas,
    SomaDasVazoes
};

struct ParametrosHidrologicosLocais
{
    double area = 0.0;
    double coeficienteEscoamento = 0.0;
    double comprimentoTalvegue = 0.0;
    double declividadeTalvegue = 0.0;
    double vazaoLocal = 0.0;
};

struct ParametrosAcumuladosBacia
{
    double areaTotal = 0.0;
    double somaCvezesArea = 0.0;
    double comprimentoRepresentativo = 0.0;
    double declividadeRepresentativa = 0.0;
    double tempoConcentracao = 0.0;
    double intensidade = 0.0;
    double vazaoPico = 0.0;
};

class CalculadoraAcumulacaoHidrologica
{
public:
    static QMap<QString, double> contribuicaoBasePorElemento(const RedeHidrologica& rede);

    static double calcularComprimentoTotalTalvegueAteElemento(
        const RedeHidrologica& rede,
        const QString& idElementoDestino,
        const QMap<QString, double>& comprimentoTalvegueLocalPorElemento = QMap<QString, double>(),
        QString* erro = nullptr,
        QMap<QString, double>* comprimentoTotalPorElementoSaida = nullptr);

    static QMap<QString, double> calcularVazaoAcumulada(
        const RedeHidrologica& rede,
        MetodologiaAcumulacaoVazao metodologia,
        const QMap<QString, ParametrosHidrologicosLocais>& parametrosLocais = QMap<QString, ParametrosHidrologicosLocais>(),
        const std::function<double(double tempoConcentracaoMin)>& funcaoIntensidadeIDF = std::function<double(double)>(),
        const std::function<double(double comprimentoTalvegue, double declividadeTalvegue)>& funcaoTempoConcentracao = std::function<double(double, double)>(),
        QString* erro = nullptr,
        QMap<QString, ParametrosAcumuladosBacia>* parametrosAcumuladosSaida = nullptr);

    static QMap<QString, double> calcularVazaoAcumuladaPorElemento(
        const RedeHidrologica& rede,
        const QMap<QString, double>& contribuicaoPorElemento = QMap<QString, double>(),
        QString* erro = nullptr);
};
