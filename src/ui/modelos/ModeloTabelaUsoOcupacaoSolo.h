#pragma once

#include "domain/TipoUsoOcupacaoSolo.h"

#include <QAbstractTableModel>
#include <QVector>

class ModeloTabelaUsoOcupacaoSolo : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Coluna
    {
        ColunaId = 0,
        ColunaNManning,
        ColunaCRunoff,
        ColunaCurveNumber,
        ColunaFatorR,
        ColunaFatorK,
        ColunaFatorC,
        ColunaFatorP,
        ColunaDensidade,
        TotalColunas
    };

    explicit ModeloTabelaUsoOcupacaoSolo(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    const QVector<TipoUsoOcupacaoSolo>& tiposUsoOcupacao() const;
    void definirTiposUsoOcupacao(const QVector<TipoUsoOcupacaoSolo>& tipos);
    void carregarPadrao();

    bool existeId(const QString& id, int linhaIgnorada = -1) const;
    const TipoUsoOcupacaoSolo* tipoPorId(const QString& id) const;
    QStringList idsDisponiveis() const;

signals:
    void dadosAlterados();

private:
    QVector<TipoUsoOcupacaoSolo> m_tiposUsoOcupacao;
};
