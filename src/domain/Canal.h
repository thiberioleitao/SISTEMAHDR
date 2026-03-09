#pragma once

#include "CanalTrecho.h"

#include <QMap>
#include <QString>
#include <QVector>

class RedeHidrologica;

class Canal
{
public:
    Canal() = default;
    explicit Canal(const QString& nome);
    Canal(const QString& id, const QString& nome, const QString& idJusante = QString());

    const QString& id() const;
    void setId(const QString& id);

    const QString& idJusante() const;
    void setIdJusante(const QString& idJusante);

    const QString& nome() const;
    void setNome(const QString& nome);

    int quantidadeTrechos() const;
    bool vazio() const;

    const QVector<CanalTrecho>& trechos() const;

    bool adicionarTrecho(const CanalTrecho& trecho, double = 0.0);

    bool removerTrecho(int indice);

    void limpar();

    double declividadeFundo() const;

    double declividadeMedia() const;

    double comprimentoTotal() const;

    double vazaoMontante() const;
    bool atualizarVazaoMontanteDaRede(const RedeHidrologica& rede,
                                      const QMap<QString, double>& contribuicaoPorElemento,
                                      QString* erro = nullptr);

    double vazaoContribuicao() const;
    void setVazaoContribuicao(double valor);

    double vazaoAssociada() const;

private:
    QString m_id;
    QString m_idJusante;
    QString m_nome;
    QVector<CanalTrecho> m_trechos;
    double m_vazaoMontante = 0.0;
    double m_vazaoContribuicao = 0.0;
};
