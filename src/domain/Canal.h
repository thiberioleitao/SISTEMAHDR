#pragma once

#include "CanalTrecho.h"

#include <QString>
#include <QVector>

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

    double declividadeFundo() const;

    double declividadeMedia() const;

    double comprimentoTotal() const;

private:
    QString m_nome;
    QVector<CanalTrecho> m_trechos;
};
