#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @brief Representa uma secao transversal cadastrada na tabela de configuracao.
 */
struct RegistroSecaoTransversal
{
    QString nomenclatura;
    QString geometria;
    QString baseMenorM;
    QString talude;
    QString diametroM;
    QString alturaMaximaM;
    QString folgaMinimaM;
    QString revestimento;
};

/**
 * @brief Modelo Qt que expoe as secoes transversais reutilizaveis da aplicacao.
 */
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
        ColunaAlturaMaxima,
        ColunaFolgaMinima,
        ColunaRevestimento,
        TotalColunas
    };

    /**
     * @brief Cria o modelo com as secoes padrao do sistema.
     * @param parent Objeto pai do modelo.
     */
    explicit ModeloTabelaSecoesTransversais(QObject* parent = nullptr);

    /**
     * @brief Retorna o total de linhas exibidas.
     * @param parent Indice pai do Qt.
     * @return Quantidade de secoes cadastradas.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * @brief Retorna o total de colunas da tabela.
     * @param parent Indice pai do Qt.
     * @return Quantidade de colunas configuradas.
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * @brief Fornece os dados de exibicao, edicao e tooltip da tabela.
     * @param index Indice solicitado pela view.
     * @param role Papel Qt do dado solicitado.
     * @return Valor correspondente ao papel informado.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Informa os cabecalhos e tooltips das colunas.
     * @param section Indice da linha ou coluna.
     * @param orientation Orientacao do cabecalho.
     * @param role Papel Qt solicitado.
     * @return Texto do cabecalho correspondente.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Define os flags de edicao de cada celula conforme a geometria.
     * @param index Indice consultado pela view.
     * @return Conjunto de flags Qt aplicavel a celula.
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * @brief Atualiza um campo editavel do registro selecionado.
     * @param index Celula alterada pela view.
     * @param value Novo valor informado pelo usuario.
     * @param role Papel Qt da alteracao.
     * @return true quando o valor foi aplicado.
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /**
     * @brief Restaura as secoes padrao utilizadas como base no sistema.
     */
    void carregarPadrao();

    /**
     * @brief Retorna os registros atualmente armazenados.
     * @return Vetor de secoes transversais.
     */
    const QVector<RegistroSecaoTransversal>& registros() const;

    /**
     * @brief Substitui todos os registros do modelo.
     * @param registros Novo conjunto de secoes transversais.
     */
    void definirRegistros(const QVector<RegistroSecaoTransversal>& registros);

    /**
     * @brief Procura um registro de secao pelo identificador textual.
     * @param nomenclatura ID da secao procurada.
     * @param registro Saida opcional com o registro encontrado.
     * @return true quando a secao foi localizada.
     */
    bool obterRegistroPorNomenclatura(const QString& nomenclatura, RegistroSecaoTransversal* registro = nullptr) const;

    /**
     * @brief Lista os revestimentos padrao exibidos no cadastro.
     * @return Nomes padronizados de revestimento.
     */
    QStringList nomesRevestimentosDisponiveis() const;

    /**
     * @brief Atualiza a lista de revestimentos disponíveis para edição das seções.
     * @param nomes Lista atual de nomes válidos.
     */
    void definirNomesRevestimentosDisponiveis(const QStringList& nomes);

private:
    QVector<RegistroSecaoTransversal> m_registros;
    QStringList m_nomesRevestimentosDisponiveis;
};
