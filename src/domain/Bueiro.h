#pragma once

#include "Canal.h"
#include "SecaoTransversalTrapezoidal.h"

#include <QMap>
#include <QString>
#include <QVector>

class RedeHidrologica;

class Bueiro
{
public:
    Bueiro() = default;
    explicit Bueiro(const QString& nome);

    RedeHidrologica* rede();
    const RedeHidrologica* rede() const;
    void setRede(RedeHidrologica* rede);

    const QString& idProprio() const;
    void setIdProprio(const QString& id);

    const QString& idJusante() const;
    void setIdJusante(const QString& idJusante);

    const QString& nome() const;
    void setNome(const QString& nome);

    int quantidadeTrechos() const;
    bool vazio() const;

    const QVector<Canal>& trechos() const;

    bool adicionarTrecho(const Canal& trecho, double = 0.0);

    bool removerTrecho(int indice);

    void limpar();

    double declividadeMedia() const;

    double comprimentoTotal() const;

    double calcularComprimentoTotalTalvegueAteElemento() const;

    QMap<QString, double> calcularVazaoAcumuladaPorElemento(
        const QMap<QString, double>& contribuicaoPorElemento = QMap<QString, double>()) const;

    double areaAcumuladaTotalContribuinte() const;

    double coeficienteEscoamentoMedioPonderado() const;

    double vazaoAcumuladaTotal() const;

    bool calcularVazaoAcumuladaTotal();

    static double velocidadeManning(const SecaoTransversalTrapezoidal& secao,
        double alturaLamina,
        double declividadeFundo,
        double coeficienteManning);

    static double vazaoManning(const SecaoTransversalTrapezoidal& secao,
        double alturaLamina,
        double declividadeFundo,
        double coeficienteManning);

    static double laminaParaVazao(const SecaoTransversalTrapezoidal& secao,
        double vazaoDesejada,
        double declividadeFundo,
        double coeficienteManning,
        double tolerancia = 1e-6,
        int maxIteracoes = 100,
        double alturaMaximaBusca = 10.0);

private:
    QString m_nome;
    QString m_idJusante;
    QVector<Canal> m_trechos;

    double m_vazaoMontante = 0.0;
    double m_vazaoContribuicao = 0.0;
    double m_vazaoAcumuladaTotal = 0.0;
};
