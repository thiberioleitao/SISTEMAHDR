#pragma once

#include "domain/BaciaContribuicao.h"

#include <QAbstractTableModel>
#include <QPointer>
#include <QVector>

class ModeloTabelaUsoOcupacaoSolo;

class ModeloTabelaBacias : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Coluna
    {
        ColunaBacia = 0,
        ColunaSubbacia,
        ColunaUsoOcupacaoSolo,
        ColunaIdJusante,
        ColunaAreaKm2,
        ColunaAreaM2,
        ColunaDeclividadeMedia,
        ColunaTalveguePrincipal,
        TotalColunas
    };

    explicit ModeloTabelaBacias(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void definirBacias(const QVector<BaciaContribuicao>& bacias);
    const QVector<BaciaContribuicao>& bacias() const;

    bool adicionarBacia(const QString& nomeBacia,
                        const QString& nomeSubbacia,
                        QString* mensagemErro = nullptr);
    bool removerBacia(int linha, QString* mensagemErro = nullptr);
    bool existeCombinacao(const QString& nomeBacia,
                          const QString& nomeSubbacia,
                          int linhaIgnorada = -1) const;

    QString nomeBaciaPorLinha(int linha) const;
    QString nomeSubbaciaPorLinha(int linha) const;
    QString idUsoOcupacaoPorLinha(int linha) const;

    void definirModeloUsoOcupacaoSolo(ModeloTabelaUsoOcupacaoSolo* modeloUsoOcupacaoSolo);
    void sincronizarCoeficienteRunoffPeloUsoOcupacao();

signals:
    void dadosAlterados();

private:
    QString criarIdComposto(const QString& nomeBacia,
                            const QString& nomeSubbacia) const;

    void decomporIdComposto(const QString& idComposto,
                            QString* nomeBacia,
                            QString* nomeSubbacia) const;

    QVector<BaciaContribuicao> m_bacias;
    QVector<QString> m_nomesBaciaPorLinha;
    QVector<QString> m_nomesSubbaciaPorLinha;
    QVector<QString> m_idsUsoOcupacaoPorLinha;
    QPointer<ModeloTabelaUsoOcupacaoSolo> m_modeloUsoOcupacaoSolo;
};
