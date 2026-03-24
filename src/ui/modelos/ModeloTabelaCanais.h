#pragma once

#include "domain/Canal.h"

#include <QAbstractTableModel>
#include <QVector>

class ModeloTabelaCanais : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Coluna
    {
        ColunaId = 0,
        ColunaIdJusante,
        ColunaSecaoTransversal,
        ColunaComprimento,
        ColunaDeclividadeFinal,
        ColunaDeclividadeFinalPercentual,
        ColunaDeclividadeMinima,
        ColunaDeclividadeMinimaPercentual,
        ColunaDeclividadeMaxima,
        ColunaDeclividadeMaximaPercentual,
        TotalColunas
    };

    explicit ModeloTabelaCanais(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void definirCanais(const QVector<Canal>& canais);
    const QVector<Canal>& canais() const;

    bool adicionarCanal(const QString& idCanal, QString* mensagemErro = nullptr);
    bool removerCanal(int linha, QString* mensagemErro = nullptr);
    bool existeId(const QString& idCanal, int linhaIgnorada = -1) const;

    QString idSecaoPorLinha(int linha) const;

signals:
    void dadosAlterados();

private:
    QVector<Canal> m_canais;
    QVector<QString> m_idsSecaoPorLinha;
};
