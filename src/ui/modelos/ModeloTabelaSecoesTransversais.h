#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>

struct RegistroSecaoTransversal
{
    QString nomenclatura;
    QString geometria;
    QString baseMenorM;
    QString talude;
    QString diametroM;
};

class ModeloTabelaSecoesTransversais : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Coluna
    {
        ColunaNomenclatura = 0,
        ColunaGeometria,
        ColunaBaseMenor,
        ColunaTalude,
        ColunaDiametro,
        TotalColunas
    };

    explicit ModeloTabelaSecoesTransversais(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void carregarPadrao();
    const QVector<RegistroSecaoTransversal>& registros() const;
    void definirRegistros(const QVector<RegistroSecaoTransversal>& registros);

private:
    QVector<RegistroSecaoTransversal> m_registros;
};
