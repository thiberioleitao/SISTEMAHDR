#pragma once

#include "domain/BaciaContribuicao.h"
#include "domain/Canal.h"

#include <QMainWindow>
#include <QMap>
#include <QVector>

class QDockWidget;
class QIcon;
class QStandardItem;
class QStandardItemModel;
class QTabWidget;
class QTableView;
class QTreeView;
class QModelIndex;
class QPoint;
class QWidget;
class QAction;

class DelegadoSecaoTransversalComboBox;
class DelegadoGeometriaSecaoComboBox;
class DelegadoUsoOcupacaoSoloComboBox;
class ModeloTabelaCanais;
class ModeloTabelaBacias;
class ModeloTabelaSecoesTransversais;
class ModeloTabelaUsoOcupacaoSolo;
class QJsonObject;

/**
 * @brief Janela principal da aplicacao para navegacao do modelo hidrologico.
 *
 * A interface apresenta uma arvore lateral com os grupos principais do modelo
 * e uma area central com abas que exibem tabelas de canais e bacias.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Cria a janela principal e inicializa todos os componentes visuais.
     * @param parent Widget pai da janela principal.
     */
    explicit MainWindow(QWidget* parent = nullptr);

private:
    /**
     * @brief Configura propriedades gerais da janela, como titulo e tamanho.
     */
    void configurarJanela();

    /**
     * @brief Monta a area central com abas e uma pagina inicial orientativa.
     */
    void configurarAreaCentral();

    /**
     * @brief Cria o painel lateral com a arvore de navegacao do modelo.
     */
    void configurarArvoreModelo();

    /**
     * @brief Insere na arvore os itens do modelo hidrologico.
     */
    void popularArvoreModelo();

    /**
     * @brief Processa o clique em um item da arvore e abre a aba correspondente.
     * @param indice Item selecionado na arvore lateral.
     */
    void abrirAbaParaIndice(const QModelIndex& indice);

    /**
     * @brief Garante que a aba de seções transversais exista e a torna ativa.
     */
    void garantirAbaSecoesTransversais();

    /**
     * @brief Garante que a aba de canais exista e a torna ativa.
     */
    void garantirAbaCanais();

    /**
     * @brief Garante que a aba de bacias exista e a torna ativa.
     */
    void garantirAbaBacias();

    /**
     * @brief Garante que a aba de uso e ocupação do solo exista e a torna ativa.
     */
    void garantirAbaUsoOcupacaoSolo();

    /**
     * @brief Garante que a aba de resultados de canais exista e a torna ativa.
     */
    void garantirAbaResultadosCanais();

    /**
     * @brief Garante que a aba de resultados de canais (declividade mínima) exista e a torna ativa.
     */
    void garantirAbaResultadosCanaisDeclividadeMinima();

    /**
     * @brief Garante que a aba de resultados de canais (declividade máxima) exista e a torna ativa.
     */
    void garantirAbaResultadosCanaisDeclividadeMaxima();

    /**
     * @brief Garante que a aba de resultados de canais (declividade final) exista e a torna ativa.
     */
    void garantirAbaResultadosCanaisDeclividadeFinal();

    /**
     * @brief Garante que a aba de resultados de bacias exista e a torna ativa.
     */
    void garantirAbaResultadosBacias();

    /**
     * @brief Procura o indice de uma aba pelo identificador armazenado.
     * @param chave Identificador logico da aba.
     * @return Indice da aba, ou -1 quando ela ainda nao existe.
     */
    int indiceAbaPorChave(const QString& chave) const;

    /**
     * @brief Cria uma tabela base com configuracao visual padrao.
     * @return Ponteiro para a tabela criada.
     */
    QTableView* criarTabelaBase();

    /**
     * @brief Preenche a tabela de canais usando os principais parametros do objeto Canal.
     * @param tabela Tabela onde os canais serao exibidos.
     */
    void prepararModeloCanais();

    /**
     * @brief Preenche a tabela de bacias usando os principais parametros do objeto BaciaContribuicao.
     * @param tabela Tabela onde as bacias serao exibidas.
     */
    void prepararModeloBacias();

    /**
     * @brief Atualiza o modelo de apresentação dos resultados hidráulicos dos canais.
     */
    void atualizarModeloResultadosCanais();

    /**
     * @brief Atualiza o modelo de apresentação dos resultados das bacias.
     */
    void atualizarModeloResultadosBacias();

    /**
     * @brief Recalcula os modelos de resultados de canais e bacias.
     */
    void calcularResultadosModelo();

    /**
     * @brief Configura ações da interface para cálculo manual de resultados.
     */
    void configurarAcoesCalculo();

    /**
     * @brief Configura ações para persistência do projeto (salvar/abrir).
     */
    void configurarAcoesProjeto();

    /**
     * @brief Salva os dados atuais em arquivo JSON de projeto.
     */
    void salvarProjetoEmArquivo();

    /**
     * @brief Abre um arquivo JSON de projeto e aplica os dados na interface.
     */
    void abrirProjetoDeArquivo();

    /**
     * @brief Importa bacias/subbacias e parâmetros a partir de arquivo CSV.
     */
    void importarBaciasDeCsv();

    /**
     * @brief Importa tipos de uso e ocupação do solo e parâmetros a partir de arquivo CSV.
     */
    void importarUsoOcupacaoSoloDeCsv();

    /**
     * @brief Importa canais e parâmetros hidráulicos básicos a partir de arquivo CSV.
     */
    void importarCanaisDeCsv();

    /**
     * @brief Cria objeto JSON com os dados atuais do projeto.
     */
    QJsonObject criarObjetoJsonProjeto() const;

    /**
     * @brief Aplica objeto JSON de projeto nos modelos de dados.
     * @param objetoProjeto Conteúdo do projeto carregado.
     * @return true quando o conteúdo for válido e aplicado.
     */
    bool aplicarObjetoJsonProjeto(const QJsonObject& objetoProjeto);

    /**
     * @brief Exibe menu de contexto da árvore com ação de cálculo.
     * @param posicao Posição local do clique.
     */
    void mostrarMenuContextoArvore(const QPoint& posicao);

    /**
     * @brief Configura a acao de adicionar um novo canal na tabela.
     * @param tabela Tabela onde os canais serao exibidos.
     */
    void configurarAcaoAdicionarCanal(QTableView* tabela);

    /**
     * @brief Configura a acao de adicionar uma nova bacia na tabela.
     * @param tabela Tabela onde as bacias serao exibidas.
     */
    void configurarAcaoAdicionarBacia(QTableView* tabela);

    /**
     * @brief Configura atalhos de copiar/colar para uma tabela.
     * @param tabela Tabela alvo para integração com dados tabulados (Excel).
     */
    void configurarAcoesCopiarColarTabela(QTableView* tabela);

    /**
     * @brief Copia o bloco selecionado da tabela para a área de transferência.
     * @param tabela Tabela origem.
     */
    void copiarSelecaoTabela(QTableView* tabela) const;

    /**
     * @brief Cola dados tabulados (Excel) a partir da célula atual da tabela.
     * @param tabela Tabela destino.
     */
    void colarDadosTabela(QTableView* tabela);

    /**
     * @brief Ajusta as larguras de coluna conforme conteúdo/header e mantém redimensionamento manual.
     * @param tabela Tabela alvo.
     */
    void ajustarLarguraColunasTabela(QTableView* tabela) const;

    /**
     * @brief Cria um item na arvore do modelo hidrologico.
     * @param pai Item pai na arvore.
     * @param texto Texto a ser exibido no item.
     * @param tipo Tipo do item (para identificacao posterior).
     * @param icone Icone a ser exibido no item.
     * @return Ponteiro para o item criado.
     */
    QStandardItem* criarItemArvore(QStandardItem* pai,
                                   const QString& texto,
                                   const QString& tipo,
                                   const QIcon& icone) const;

    /**
     * @brief Cria dados de exemplo para demonstrar a aba de canais.
     * @return Vetor com canais basicos para exibicao inicial.
     */
    QVector<Canal> criarCanaisExemplo() const;

    /**
     * @brief Cria dados de exemplo para demonstrar a aba de bacias.
     * @return Vetor com bacias basicas para exibicao inicial.
     */
    QVector<BaciaContribuicao> criarBaciasExemplo() const;

private:
    /** @brief Painel acoplavel que contem a arvore lateral do modelo. */
    QDockWidget* m_dockModelo = nullptr;

    /** @brief Arvore usada para navegar entre canais e bacias do modelo. */
    QTreeView* m_arvoreModelo = nullptr;
    QStandardItemModel* m_modeloArvore = nullptr;

    /** @brief Controle central responsavel por exibir as abas abertas. */
    QTabWidget* m_abasCentrais = nullptr;

    /** @brief Pagina inicial exibida antes da abertura de qualquer aba de dados. */
    QWidget* m_paginaInicial = nullptr;

    /** @brief Modelos de tabela para exibir canais e bacias. */
    ModeloTabelaSecoesTransversais* m_modeloSecoes = nullptr;
    ModeloTabelaUsoOcupacaoSolo* m_modeloUsoOcupacaoSolo = nullptr;
    ModeloTabelaCanais* m_modeloCanais = nullptr;
    ModeloTabelaBacias* m_modeloBacias = nullptr;
    QStandardItemModel* m_modeloResultadosCanais = nullptr;
    QStandardItemModel* m_modeloResultadosBacias = nullptr;

    DelegadoSecaoTransversalComboBox* m_delegadoSecaoCanais = nullptr;
    DelegadoGeometriaSecaoComboBox* m_delegadoGeometriaSecoes = nullptr;
    DelegadoUsoOcupacaoSoloComboBox* m_delegadoUsoOcupacaoBacias = nullptr;

    /** @brief Mapeamento das tabelas por chave para acesso rapido. */
    QMap<QString, QTableView*> m_tabelaPorChave;

    QAction* m_acaoCalcularResultados = nullptr;
    QAction* m_acaoSalvarProjeto = nullptr;
    QAction* m_acaoAbrirProjeto = nullptr;
    QAction* m_acaoImportarBaciasCsv = nullptr;
    QAction* m_acaoImportarUsoOcupacaoCsv = nullptr;
    QAction* m_acaoImportarCanaisCsv = nullptr;
};
