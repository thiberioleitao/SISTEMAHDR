#pragma once

#include "CanalTrecho.h"
#include "SecaoTransversalTrapezoidal.h"

#include <QString>
#include <QVector>

class RedeHidrologica;

class Canal
{
public:
    Canal() = default;
    explicit Canal(const QString& nome);

    const QString& nome() const;
    void setNome(const QString& nome);

    int quantidadeTrechos() const;
    bool vazio() const;

    const QVector<CanalTrecho>& trechos() const;

    bool adicionarTrecho(const CanalTrecho& trecho, double = 0.0);

    bool removerTrecho(int indice);

    void limpar();

    double declividadeMedia() const;

    double comprimentoTotal() const;


    double areaAcumuladaTotalContribuinte(const RedeHidrologica& rede,
                                          QString* erro = nullptr) const;

    double coeficienteEscoamentoMedioPonderado(const RedeHidrologica& rede,
                                               QString* erro = nullptr) const;

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
    QVector<CanalTrecho> m_trechos;
    double m_vazaoMontante = 0.0;
    double m_vazaoContribuicao = 0.0;
};
