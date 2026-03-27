#include "MainWindow.h"

#include "ui/delegates/DelegadoGeometriaSecaoComboBox.h"
#include "ui/delegates/DelegadoSecaoTransversalComboBox.h"
#include "ui/delegates/DelegadoUsoOcupacaoSoloComboBox.h"

#include "domain/IDF.h"
#include "domain/CalculoHidraulicoCanal.h"
#include "domain/RedeHidrologica.h"

#include "ui/modelos/ModeloTabelaBacias.h"
#include "ui/modelos/ModeloTabelaCanais.h"
#include "ui/modelos/ModeloTabelaSecoesTransversais.h"
#include "ui/modelos/ModeloTabelaUsoOcupacaoSolo.h"

#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSet>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QStyle>
#include <QShortcut>
#include <QTabWidget>
#include <QTableView>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <cmath>

namespace
{
constexpr auto kRoleTipoItem = Qt::UserRole + 1;
constexpr auto kAbaInicial = "aba_inicial";
constexpr auto kAbaSecoes = "aba_secoes";
constexpr auto kAbaCanais = "aba_canais";
constexpr auto kAbaBacias = "aba_bacias";
constexpr auto kAbaUsoOcupacaoSolo = "aba_uso_ocupacao_solo";
constexpr auto kAbaResultadosCanais = "aba_resultados_canais";
constexpr auto kAbaResultadosCanaisDeclividadeMinima = "aba_resultados_canais_declividade_minima";
constexpr auto kAbaResultadosCanaisDeclividadeMaxima = "aba_resultados_canais_declividade_maxima";
constexpr auto kAbaResultadosCanaisDeclividadeFinal = "aba_resultados_canais_declividade_final";
constexpr auto kAbaResultadosBacias = "aba_resultados_bacias";

void configurarVisibilidadeColunasResultadosCanais(QTableView* tabela, const QString& modoDeclividade)
{
    if (!tabela || !tabela->model()) return;

    // Colunas base comuns para todas as visões de resultados dos canais.
    const QSet<int> colunasBase = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
    QSet<int> colunasPermitidas = colunasBase;

    if (modoDeclividade == "min") {
        for (int coluna = 9; coluna <= 19; ++coluna) colunasPermitidas.insert(coluna);
    }
    else if (modoDeclividade == "max") {
        for (int coluna = 20; coluna <= 30; ++coluna) colunasPermitidas.insert(coluna);
    }
    else if (modoDeclividade == "final") {
        for (int coluna = 31; coluna <= 33; ++coluna) colunasPermitidas.insert(coluna);
    }
    else {
        const int totalColunas = tabela->model()->columnCount();
        for (int coluna = 0; coluna < totalColunas; ++coluna) {
            tabela->setColumnHidden(coluna, false);
        }
        return;
    }

    const int totalColunas = tabela->model()->columnCount();
    for (int coluna = 0; coluna < totalColunas; ++coluna) {
        tabela->setColumnHidden(coluna, !colunasPermitidas.contains(coluna));
    }
}

double alturaMaximaSecaoPorId(const QString& idSecao,
                              const ModeloTabelaSecoesTransversais* modeloSecoes)
{
    if (!modeloSecoes) return 1.0;

    const QString idAlvo = idSecao.trimmed();
    const int totalLinhas = modeloSecoes->rowCount();
    for (int linha = 0; linha < totalLinhas; ++linha) {
        const QString idLinha = modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaNomenclatura)
                                    .data(Qt::DisplayRole)
                                    .toString()
                                    .trimmed();
        if (idLinha.compare(idAlvo, Qt::CaseInsensitive) != 0) continue;

        const QString geometria = modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaGeometria)
                                      .data(Qt::DisplayRole)
                                      .toString();
        if (geometria.contains("Semicircular", Qt::CaseInsensitive)) {
            const double diametro = modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaDiametro)
                                         .data(Qt::DisplayRole)
                                         .toString()
                                         .toDouble();
            return std::max(0.0, diametro);
        }

        return 1.0;
    }

    return 1.0;
}

double calcularFroude(double velocidade, double areaMolhada, double larguraSuperficial)
{
    if (velocidade <= 0.0 || areaMolhada <= 0.0 || larguraSuperficial <= 0.0) return 0.0;
    const double profundidadeHidraulica = areaMolhada / larguraSuperficial;
    if (profundidadeHidraulica <= 0.0) return 0.0;
    constexpr double gravidade = 9.81;
    return velocidade / std::sqrt(gravidade * profundidadeHidraulica);
}

double calcularTensaoCisalhantePa(double raioHidraulico, double declividade)
{
    if (raioHidraulico <= 0.0 || declividade <= 0.0) return 0.0;
    constexpr double gammaAgua = 9810.0;
    return gammaAgua * raioHidraulico * declividade;
}

bool aplicarSecaoTransversalNoCanal(Canal* canal,
                                    const QString& idSecao,
                                    const ModeloTabelaSecoesTransversais* modeloSecoes)
{
    if (!canal || !modeloSecoes) return false;

    const QString idSecaoNormalizado = idSecao.trimmed();
    if (idSecaoNormalizado.isEmpty()) return false;

    const int totalLinhas = modeloSecoes->rowCount();
    for (int linha = 0; linha < totalLinhas; ++linha) {
        const QString idLinha = modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaNomenclatura)
                                    .data(Qt::DisplayRole)
                                    .toString()
                                    .trimmed();
        if (idLinha.compare(idSecaoNormalizado, Qt::CaseInsensitive) != 0) continue;

        const QString geometria = modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaGeometria)
                                      .data(Qt::DisplayRole)
                                      .toString();

        if (geometria.compare("Semicircular", Qt::CaseInsensitive) == 0) {
            const double diametro = std::max(0.0,
                modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaDiametro)
                    .data(Qt::DisplayRole)
                    .toString()
                    .toDouble());

            canal->setSecaoSemicircular(SecaoTransversalSemicircular(diametro));
            if (canal->coeficienteManning() <= 0.0) {
                canal->setCoeficienteManning(0.015);
            }
            return true;
        }

        const double larguraFundo = std::max(0.0,
            modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaBaseMenor)
                .data(Qt::DisplayRole)
                .toString()
                .toDouble());
        const double taludeLateral = std::max(0.0,
            modeloSecoes->index(linha, ModeloTabelaSecoesTransversais::ColunaTalude)
                .data(Qt::DisplayRole)
                .toString()
                .toDouble());

        canal->setSecaoTransversal(SecaoTransversalTrapezoidal(larguraFundo, taludeLateral));
        if (canal->coeficienteManning() <= 0.0) {
            canal->setCoeficienteManning(0.025);
        }
        return true;
    }

    return false;
}

double converterTextoParaNumeroCsv(QString texto)
{
    texto = texto.trimmed();
    texto.remove('%');
    texto.remove('"');
    texto.replace(',', '.');
    return texto.toDouble();
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    configurarJanela();
    configurarAreaCentral();
    configurarArvoreModelo();

    m_modeloSecoes = new ModeloTabelaSecoesTransversais(this);
    m_modeloUsoOcupacaoSolo = new ModeloTabelaUsoOcupacaoSolo(this);
    m_modeloCanais = new ModeloTabelaCanais(this);
    m_modeloBacias = new ModeloTabelaBacias(this);
    m_modeloResultadosCanais = new QStandardItemModel(this);
    m_modeloResultadosBacias = new QStandardItemModel(this);
    m_delegadoSecaoCanais = new DelegadoSecaoTransversalComboBox(m_modeloSecoes, this);
    m_delegadoGeometriaSecoes = new DelegadoGeometriaSecaoComboBox(this);
    m_delegadoUsoOcupacaoBacias = new DelegadoUsoOcupacaoSoloComboBox(m_modeloUsoOcupacaoSolo, this);

    m_modeloBacias->definirModeloUsoOcupacaoSolo(m_modeloUsoOcupacaoSolo);

    prepararModeloCanais();
    prepararModeloBacias();
    calcularResultadosModelo();

    connect(m_modeloCanais, &ModeloTabelaCanais::dadosAlterados,
            this,
            [this]() {
                calcularResultadosModelo();
            });

    connect(m_modeloBacias, &ModeloTabelaBacias::dadosAlterados,
            this,
            [this]() {
                calcularResultadosModelo();
            });

    connect(m_modeloSecoes, &QAbstractItemModel::dataChanged,
            this,
            [this](const QModelIndex&, const QModelIndex&, const QList<int>&) {
                calcularResultadosModelo();
            });

    connect(m_modeloSecoes, &QAbstractItemModel::modelReset,
            this,
            [this]() {
                calcularResultadosModelo();
            });

    connect(m_modeloUsoOcupacaoSolo, &ModeloTabelaUsoOcupacaoSolo::dadosAlterados,
            this,
            [this]() {
                m_modeloBacias->sincronizarCoeficienteRunoffPeloUsoOcupacao();
                calcularResultadosModelo();
            });

    configurarAcoesProjeto();
    configurarAcoesCalculo();

    popularArvoreModelo();
}

void MainWindow::configurarJanela()
{
    setWindowTitle("SISTEMAHDR - Modelo Hidrol\u00F3gico");
    resize(1280, 780);
    statusBar()->showMessage("Selecione um item em Dados ou Resultados na \u00E1rvore lateral.");

    setStyleSheet(
        "QMainWindow { background: #eef3f7; }"
        "QDockWidget::title { background: #1f4e79; color: white; padding: 8px 10px; }"
        "QTreeView, QTableView { background: white; border: 1px solid #c9d4df; }"
        "QTreeView::item:selected { background: #d7e8f7; color: #0f2f4a; }"
        "QTabWidget::pane { border: 1px solid #c9d4df; background: white; }"
        "QTabBar::tab { background: #dfe8f1; padding: 8px 14px; margin-right: 2px; }"
        "QTabBar::tab:selected { background: white; color: #143a5a; font-weight: 600; }"
        "QHeaderView::section { background: #ebf1f6; padding: 6px; border: none; border-bottom: 1px solid #c9d4df; }");
}

void MainWindow::configurarAreaCentral()
{
    m_abasCentrais = new QTabWidget(this);
    m_abasCentrais->setTabsClosable(true);
    m_abasCentrais->setDocumentMode(true);

    m_paginaInicial = new QWidget(m_abasCentrais);
    auto* layoutInicial = new QVBoxLayout(m_paginaInicial);
    layoutInicial->setContentsMargins(32, 32, 32, 32);
    layoutInicial->setSpacing(10);

    auto* titulo = new QLabel("Modelo 1", m_paginaInicial);
    titulo->setStyleSheet("font-size: 24px; font-weight: 600; color: #143a5a;");

    auto* descricao = new QLabel(
        "Use a \u00E1rvore lateral para abrir os grupos Dados e Resultados. "
        "A edi\u00E7\u00E3o e apresenta\u00E7\u00E3o seguem a arquitetura model/view.",
        m_paginaInicial);
    descricao->setWordWrap(true);
    descricao->setStyleSheet("font-size: 14px; color: #4f6375;");

    layoutInicial->addWidget(titulo);
    layoutInicial->addWidget(descricao);
    layoutInicial->addStretch();

    m_abasCentrais->addTab(m_paginaInicial, "In\u00EDcio");
    m_abasCentrais->widget(0)->setProperty("tabKey", kAbaInicial);

    connect(m_abasCentrais, &QTabWidget::tabCloseRequested, this, [this](int indice) {
        QWidget* pagina = m_abasCentrais->widget(indice);
        if (pagina == m_paginaInicial) return;
        if (!pagina) return;

        const QString chave = pagina->property("tabKey").toString();
        m_tabelaPorChave.remove(chave);

        m_abasCentrais->removeTab(indice);
        pagina->deleteLater();
    });

    setCentralWidget(m_abasCentrais);
}

void MainWindow::configurarArvoreModelo()
{
    m_dockModelo = new QDockWidget("Modelo", this);
    m_dockModelo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockModelo->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    m_arvoreModelo = new QTreeView(m_dockModelo);
    m_arvoreModelo->setAlternatingRowColors(true);
    m_arvoreModelo->setUniformRowHeights(true);

    m_modeloArvore = new QStandardItemModel(this);
    m_modeloArvore->setHorizontalHeaderLabels({ "Estrutura do modelo" });
    m_arvoreModelo->setModel(m_modeloArvore);
    m_arvoreModelo->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_arvoreModelo->selectionModel(), &QItemSelectionModel::currentChanged,
            this,
            [this](const QModelIndex& atual, const QModelIndex&) {
                abrirAbaParaIndice(atual);
            });

    connect(m_arvoreModelo, &QWidget::customContextMenuRequested,
            this,
            [this](const QPoint& posicao) {
                mostrarMenuContextoArvore(posicao);
            });

    m_dockModelo->setWidget(m_arvoreModelo);
    addDockWidget(Qt::LeftDockWidgetArea, m_dockModelo);
}

void MainWindow::popularArvoreModelo()
{
    m_modeloArvore->removeRows(0, m_modeloArvore->rowCount());

    QStandardItem* raiz = criarItemArvore(
        m_modeloArvore->invisibleRootItem(),
        "Modelo 1",
        "modelo",
        style()->standardIcon(QStyle::SP_DirHomeIcon));

    QStandardItem* itemDados = criarItemArvore(
        raiz,
        "Dados",
        "dados",
        style()->standardIcon(QStyle::SP_DirIcon));

    criarItemArvore(itemDados, "Se\u00E7\u00F5es transversais", "secoes", style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    criarItemArvore(itemDados, "Canais", "canais", style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    criarItemArvore(itemDados, "Bacias", "bacias", style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    criarItemArvore(itemDados, "Uso e ocupa\u00E7\u00E3o do solo", "uso_ocupacao_solo", style()->standardIcon(QStyle::SP_FileDialogDetailedView));

    QStandardItem* itemResultados = criarItemArvore(
        raiz,
        "Resultados",
        "resultados",
        style()->standardIcon(QStyle::SP_DirIcon));

    QStandardItem* itemResultadosCanais = criarItemArvore(itemResultados, "Canais", "resultados_canais", style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    criarItemArvore(itemResultados, "Bacias", "resultados_bacias", style()->standardIcon(QStyle::SP_FileDialogDetailedView));

    if (itemResultadosCanais) {
        criarItemArvore(itemResultadosCanais, "Declividade m\u00EDnima", "resultados_canais_declividade_minima", style()->standardIcon(QStyle::SP_FileDialogInfoView));
        criarItemArvore(itemResultadosCanais, "Declividade m\u00E1xima", "resultados_canais_declividade_maxima", style()->standardIcon(QStyle::SP_FileDialogInfoView));
        criarItemArvore(itemResultadosCanais, "Declividade final", "resultados_canais_declividade_final", style()->standardIcon(QStyle::SP_FileDialogInfoView));
        m_arvoreModelo->expand(itemResultadosCanais->index());
    }

    m_arvoreModelo->expand(raiz->index());
    m_arvoreModelo->expand(itemDados->index());
    m_arvoreModelo->expand(itemResultados->index());
}

void MainWindow::abrirAbaParaIndice(const QModelIndex& indice)
{
    if (!indice.isValid()) return;

    const QString tipo = indice.data(kRoleTipoItem).toString();

    if (tipo == "secoes") {
        garantirAbaSecoesTransversais();
        statusBar()->showMessage("Aba de se\u00E7\u00F5es transversais aberta.", 3000);
    }
    else if (tipo == "canais") {
        garantirAbaCanais();
        statusBar()->showMessage("Aba de canais aberta.", 3000);
    }
    else if (tipo == "bacias") {
        garantirAbaBacias();
        statusBar()->showMessage("Aba de bacias aberta.", 3000);
    }
    else if (tipo == "uso_ocupacao_solo") {
        garantirAbaUsoOcupacaoSolo();
        statusBar()->showMessage("Aba de uso e ocupa\u00E7\u00E3o do solo aberta.", 3000);
    }
    else if (tipo == "resultados_canais") {
        garantirAbaResultadosCanais();
        statusBar()->showMessage("Aba de resultados de canais aberta.", 3000);
    }
    else if (tipo == "resultados_canais_declividade_minima") {
        garantirAbaResultadosCanaisDeclividadeMinima();
        statusBar()->showMessage("Resultados de canais - declividade m\u00EDnima.", 3000);
    }
    else if (tipo == "resultados_canais_declividade_maxima") {
        garantirAbaResultadosCanaisDeclividadeMaxima();
        statusBar()->showMessage("Resultados de canais - declividade m\u00E1xima.", 3000);
    }
    else if (tipo == "resultados_canais_declividade_final") {
        garantirAbaResultadosCanaisDeclividadeFinal();
        statusBar()->showMessage("Resultados de canais - declividade final.", 3000);
    }
    else if (tipo == "resultados_bacias") {
        garantirAbaResultadosBacias();
        statusBar()->showMessage("Aba de resultados de bacias aberta.", 3000);
    }
}

void MainWindow::garantirAbaSecoesTransversais()
{
    const int indiceExistente = indiceAbaPorChave(kAbaSecoes);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloSecoes);
    tabela->setItemDelegateForColumn(ModeloTabelaSecoesTransversais::ColunaGeometria, m_delegadoGeometriaSecoes);
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Se\u00E7\u00F5es transversais");
    tabela->setProperty("tabKey", kAbaSecoes);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaSecoes, tabela);
}

void MainWindow::garantirAbaCanais()
{
    const int indiceExistente = indiceAbaPorChave(kAbaCanais);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloCanais);
    tabela->setItemDelegateForColumn(ModeloTabelaCanais::ColunaSecaoTransversal, m_delegadoSecaoCanais);
    configurarAcaoAdicionarCanal(tabela);
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Canais");
    tabela->setProperty("tabKey", kAbaCanais);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaCanais, tabela);
}

void MainWindow::garantirAbaBacias()
{
    const int indiceExistente = indiceAbaPorChave(kAbaBacias);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloBacias);
    tabela->setItemDelegateForColumn(ModeloTabelaBacias::ColunaUsoOcupacaoSolo, m_delegadoUsoOcupacaoBacias);
    configurarAcaoAdicionarBacia(tabela);
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Bacias");
    tabela->setProperty("tabKey", kAbaBacias);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaBacias, tabela);
}

void MainWindow::garantirAbaUsoOcupacaoSolo()
{
    const int indiceExistente = indiceAbaPorChave(kAbaUsoOcupacaoSolo);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloUsoOcupacaoSolo);
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Uso e ocupa\u00E7\u00E3o do solo");
    tabela->setProperty("tabKey", kAbaUsoOcupacaoSolo);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaUsoOcupacaoSolo, tabela);
}

void MainWindow::garantirAbaResultadosCanais()
{
    const int indiceExistente = indiceAbaPorChave(kAbaResultadosCanais);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloResultadosCanais);
    configurarVisibilidadeColunasResultadosCanais(tabela, "geral");
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Resultados - Canais");
    tabela->setProperty("tabKey", kAbaResultadosCanais);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaResultadosCanais, tabela);
}

void MainWindow::garantirAbaResultadosCanaisDeclividadeMinima()
{
    const int indiceExistente = indiceAbaPorChave(kAbaResultadosCanaisDeclividadeMinima);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloResultadosCanais);
    configurarVisibilidadeColunasResultadosCanais(tabela, "min");
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Resultados - Canais (Sp min)");
    tabela->setProperty("tabKey", kAbaResultadosCanaisDeclividadeMinima);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaResultadosCanaisDeclividadeMinima, tabela);
}

void MainWindow::garantirAbaResultadosCanaisDeclividadeMaxima()
{
    const int indiceExistente = indiceAbaPorChave(kAbaResultadosCanaisDeclividadeMaxima);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloResultadosCanais);
    configurarVisibilidadeColunasResultadosCanais(tabela, "max");
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Resultados - Canais (Sp max)");
    tabela->setProperty("tabKey", kAbaResultadosCanaisDeclividadeMaxima);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaResultadosCanaisDeclividadeMaxima, tabela);
}

void MainWindow::garantirAbaResultadosCanaisDeclividadeFinal()
{
    const int indiceExistente = indiceAbaPorChave(kAbaResultadosCanaisDeclividadeFinal);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloResultadosCanais);
    configurarVisibilidadeColunasResultadosCanais(tabela, "final");
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Resultados - Canais (Sp final)");
    tabela->setProperty("tabKey", kAbaResultadosCanaisDeclividadeFinal);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaResultadosCanaisDeclividadeFinal, tabela);
}

void MainWindow::garantirAbaResultadosBacias()
{
    const int indiceExistente = indiceAbaPorChave(kAbaResultadosBacias);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloResultadosBacias);
    ajustarLarguraColunasTabela(tabela);

    const int indice = m_abasCentrais->addTab(tabela, "Resultados - Bacias");
    tabela->setProperty("tabKey", kAbaResultadosBacias);
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaResultadosBacias, tabela);
}

int MainWindow::indiceAbaPorChave(const QString& chave) const
{
    for (int indice = 0; indice < m_abasCentrais->count(); ++indice) {
        QWidget* pagina = m_abasCentrais->widget(indice);
        if (pagina && pagina->property("tabKey").toString() == chave) {
            return indice;
        }
    }
    return -1;
}

QTableView* MainWindow::criarTabelaBase()
{
    auto* tabela = new QTableView(m_abasCentrais);
    tabela->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    tabela->setSelectionBehavior(QAbstractItemView::SelectItems);
    tabela->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabela->setAlternatingRowColors(true);
    tabela->setSortingEnabled(false);
    tabela->verticalHeader()->setVisible(true);
    tabela->verticalHeader()->setSectionsClickable(true);
    tabela->horizontalHeader()->setSectionsClickable(true);
    tabela->horizontalHeader()->setHighlightSections(true);
    tabela->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tabela->horizontalHeader()->setStretchLastSection(false);
    tabela->setContextMenuPolicy(Qt::CustomContextMenu);

    // Permite selecionar linha inteira ao clicar no cabeçalho vertical.
    connect(tabela->verticalHeader(), &QHeaderView::sectionClicked, tabela, [tabela](int secao) {
        if (!tabela || !tabela->model()) return;
        if (secao < 0 || secao >= tabela->model()->rowCount()) return;
        tabela->selectRow(secao);
    });

    // Permite selecionar coluna inteira ao clicar no cabeçalho horizontal.
    connect(tabela->horizontalHeader(), &QHeaderView::sectionClicked, tabela, [tabela](int secao) {
        if (!tabela || !tabela->model()) return;
        if (secao < 0 || secao >= tabela->model()->columnCount()) return;
        tabela->selectColumn(secao);
    });

    configurarAcoesCopiarColarTabela(tabela);
    return tabela;
}

void MainWindow::ajustarLarguraColunasTabela(QTableView* tabela) const
{
    if (!tabela || !tabela->model()) return;

    // Primeiro ajusta com base em header + conteúdo.
    tabela->resizeColumnsToContents();

    // Mantém ajuste manual pelo usuário após o dimensionamento inicial.
    tabela->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

void MainWindow::configurarAcoesCopiarColarTabela(QTableView* tabela)
{
    if (!tabela) return;

    auto* atalhoCopiar = new QShortcut(QKeySequence::Copy, tabela);
    connect(atalhoCopiar, &QShortcut::activated,
            this,
            [this, tabela]() {
                copiarSelecaoTabela(tabela);
            });

    auto* atalhoColar = new QShortcut(QKeySequence::Paste, tabela);
    connect(atalhoColar, &QShortcut::activated,
            this,
            [this, tabela]() {
                colarDadosTabela(tabela);
            });
}

void MainWindow::copiarSelecaoTabela(QTableView* tabela) const
{
    if (!tabela || !tabela->model() || !tabela->selectionModel()) return;

    const QModelIndexList indices = tabela->selectionModel()->selectedIndexes();
    if (indices.isEmpty()) return;

    int menorLinha = indices.first().row();
    int maiorLinha = indices.first().row();
    int menorColuna = indices.first().column();
    int maiorColuna = indices.first().column();

    for (const QModelIndex& indice : indices) {
        menorLinha = std::min(menorLinha, indice.row());
        maiorLinha = std::max(maiorLinha, indice.row());
        menorColuna = std::min(menorColuna, indice.column());
        maiorColuna = std::max(maiorColuna, indice.column());
    }

    QStringList linhasTexto;
    for (int linha = menorLinha; linha <= maiorLinha; ++linha) {
        QStringList colunasTexto;
        for (int coluna = menorColuna; coluna <= maiorColuna; ++coluna) {
            const QModelIndex indice = tabela->model()->index(linha, coluna);
            if (tabela->selectionModel()->isSelected(indice)) {
                colunasTexto << indice.data(Qt::DisplayRole).toString();
            }
            else {
                colunasTexto << QString();
            }
        }
        linhasTexto << colunasTexto.join('\t');
    }

    QApplication::clipboard()->setText(linhasTexto.join('\n'));
}

void MainWindow::colarDadosTabela(QTableView* tabela)
{
    if (!tabela || !tabela->model()) return;

    QString texto = QApplication::clipboard()->text();
    if (texto.trimmed().isEmpty()) return;

    // Normaliza quebra de linha vinda do Excel.
    texto.replace("\r\n", "\n");
    texto.replace('\r', '\n');

    QStringList linhas = texto.split('\n', Qt::KeepEmptyParts);
    if (!linhas.isEmpty() && linhas.last().isEmpty()) {
        linhas.removeLast();
    }
    if (linhas.isEmpty()) return;

    QModelIndex indiceInicial = tabela->currentIndex();
    if (!indiceInicial.isValid()) {
        indiceInicial = tabela->model()->index(0, 0);
    }

    const int linhaBase = indiceInicial.row();
    const int colunaBase = indiceInicial.column();

    for (int deslocLinha = 0; deslocLinha < linhas.size(); ++deslocLinha) {
        const QStringList colunas = linhas.at(deslocLinha).split('\t', Qt::KeepEmptyParts);

        for (int deslocColuna = 0; deslocColuna < colunas.size(); ++deslocColuna) {
            const int linhaDestino = linhaBase + deslocLinha;
            const int colunaDestino = colunaBase + deslocColuna;

            if (linhaDestino < 0 || linhaDestino >= tabela->model()->rowCount()) continue;
            if (colunaDestino < 0 || colunaDestino >= tabela->model()->columnCount()) continue;

            const QModelIndex indiceDestino = tabela->model()->index(linhaDestino, colunaDestino);
            tabela->model()->setData(indiceDestino, colunas.at(deslocColuna), Qt::EditRole);
        }
    }
}

void MainWindow::prepararModeloCanais()
{
    m_modeloCanais->definirCanais(criarCanaisExemplo());

    // Replica as seções do cenário manual de referência.
    const QVector<QString> secoes = {
        "CSD-1", "CSD-1", "CSD-3", "CSD-4", "CSD-1",
        "CSD-2", "CSD-4", "CSD-4", "VTD-1", "CSD-3"
    };

    const int total = std::min(m_modeloCanais->rowCount(), static_cast<int>(secoes.size()));
    for (int linha = 0; linha < total; ++linha) {
        m_modeloCanais->setData(
            m_modeloCanais->index(linha, ModeloTabelaCanais::ColunaSecaoTransversal),
            secoes.at(linha),
            Qt::EditRole);
    }
}

void MainWindow::prepararModeloBacias()
{
    m_modeloBacias->definirBacias(criarBaciasExemplo());
}

void MainWindow::calcularResultadosModelo()
{
    atualizarModeloResultadosCanais();
    atualizarModeloResultadosBacias();
}

void MainWindow::configurarAcoesProjeto()
{
    m_acaoSalvarProjeto = new QAction("Salvar projeto", this);
    m_acaoSalvarProjeto->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));

    m_acaoAbrirProjeto = new QAction("Abrir projeto", this);
    m_acaoAbrirProjeto->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));

    m_acaoImportarBaciasCsv = new QAction("Importar bacias (.csv)", this);
    m_acaoImportarBaciasCsv->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    m_acaoImportarUsoOcupacaoCsv = new QAction("Importar uso/ocupação (.csv)", this);
    m_acaoImportarUsoOcupacaoCsv->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    m_acaoImportarCanaisCsv = new QAction("Importar canais (.csv)", this);
    m_acaoImportarCanaisCsv->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    connect(m_acaoSalvarProjeto, &QAction::triggered,
            this,
            [this]() {
                salvarProjetoEmArquivo();
            });

    connect(m_acaoAbrirProjeto, &QAction::triggered,
            this,
            [this]() {
                abrirProjetoDeArquivo();
            });

    connect(m_acaoImportarBaciasCsv, &QAction::triggered,
            this,
            [this]() {
                importarBaciasDeCsv();
            });

    connect(m_acaoImportarUsoOcupacaoCsv, &QAction::triggered,
            this,
            [this]() {
                importarUsoOcupacaoSoloDeCsv();
            });

    connect(m_acaoImportarCanaisCsv, &QAction::triggered,
            this,
            [this]() {
                importarCanaisDeCsv();
            });

    QToolBar* barraProjeto = addToolBar("Projeto");
    barraProjeto->setMovable(false);
    barraProjeto->addAction(m_acaoSalvarProjeto);
    barraProjeto->addAction(m_acaoAbrirProjeto);
    barraProjeto->addAction(m_acaoImportarBaciasCsv);
    barraProjeto->addAction(m_acaoImportarUsoOcupacaoCsv);
    barraProjeto->addAction(m_acaoImportarCanaisCsv);
}

QJsonObject MainWindow::criarObjetoJsonProjeto() const
{
    QJsonObject projeto;
    projeto["versao"] = 1;

    QJsonArray secoesJson;
    for (const RegistroSecaoTransversal& registro : m_modeloSecoes->registros()) {
        QJsonObject item;
        item["nomenclatura"] = registro.nomenclatura;
        item["geometria"] = registro.geometria;
        item["b_menor_m"] = registro.baseMenorM;
        item["talude"] = registro.talude;
        item["diametro_m"] = registro.diametroM;
        secoesJson.append(item);
    }
    projeto["secoes"] = secoesJson;

    QJsonArray usosSoloJson;
    for (const TipoUsoOcupacaoSolo& tipo : m_modeloUsoOcupacaoSolo->tiposUsoOcupacao()) {
        QJsonObject item;
        item["id"] = tipo.id();
        item["n_manning"] = tipo.nManning();
        item["c_runoff"] = tipo.cRunoff();
        item["curve_number"] = tipo.curveNumber();
        item["fator_r"] = tipo.fatorR();
        item["fator_k"] = tipo.fatorK();
        item["fator_c"] = tipo.fatorC();
        item["fator_p"] = tipo.fatorP();
        item["densidade_kg_m3"] = tipo.densidadeKgM3();
        usosSoloJson.append(item);
    }
    projeto["usos_ocupacao_solo"] = usosSoloJson;

    QJsonArray canaisJson;
    const QVector<Canal>& canais = m_modeloCanais->canais();
    for (int linha = 0; linha < canais.size(); ++linha) {
        const Canal& canal = canais.at(linha);

        QJsonObject item;
        item["id"] = canal.id();
        item["id_jusante"] = canal.idJusante();
        item["secao"] = m_modeloCanais->idSecaoPorLinha(linha);
        item["comprimento_m"] = canal.comprimento();
        item["largura_fundo_m"] = canal.larguraFundo();
        item["talude_lateral"] = canal.taludeLateral();
        item["declividade_final_m_m"] = canal.declividadeFinal();
        item["declividade_min_m_m"] = canal.declividadeMinima();
        item["declividade_max_m_m"] = canal.declividadeMaxima();
        item["coeficiente_manning"] = canal.coeficienteManning();
        canaisJson.append(item);
    }
    projeto["canais"] = canaisJson;

    QJsonArray baciasJson;
    const QVector<BaciaContribuicao>& baciasModelo = m_modeloBacias->bacias();
    for (int linha = 0; linha < baciasModelo.size(); ++linha) {
        const BaciaContribuicao& bacia = baciasModelo.at(linha);
        QJsonObject item;
        item["id"] = bacia.id();
        item["bacia"] = m_modeloBacias->nomeBaciaPorLinha(linha);
        item["subbacia"] = m_modeloBacias->nomeSubbaciaPorLinha(linha);
        item["uso_ocupacao_solo"] = m_modeloBacias->idUsoOcupacaoPorLinha(linha);
        item["id_jusante"] = bacia.idJusante();
        item["area_km2"] = bacia.areaKm2();
        item["declividade_media"] = bacia.declividadeMedia();
        item["comprimento_talvegue_km"] = bacia.comprimentoTalveguePrincipalKm();
        item["c10"] = bacia.C_10();
        baciasJson.append(item);
    }
    projeto["bacias"] = baciasJson;

    return projeto;
}

bool MainWindow::aplicarObjetoJsonProjeto(const QJsonObject& objetoProjeto)
{
    if (!objetoProjeto.contains("secoes")
        || !objetoProjeto.contains("canais")
        || !objetoProjeto.contains("bacias")) {
        return false;
    }

    const QJsonArray secoesJson = objetoProjeto.value("secoes").toArray();
    const QJsonArray usosSoloJson = objetoProjeto.value("usos_ocupacao_solo").toArray();
    const QJsonArray canaisJson = objetoProjeto.value("canais").toArray();
    const QJsonArray baciasJson = objetoProjeto.value("bacias").toArray();

    QVector<RegistroSecaoTransversal> secoes;
    secoes.reserve(secoesJson.size());
    for (const QJsonValue& valor : secoesJson) {
        const QJsonObject item = valor.toObject();
        RegistroSecaoTransversal registro;
        registro.nomenclatura = item.value("nomenclatura").toString().trimmed();
        registro.geometria = item.value("geometria").toString().trimmed();
        registro.baseMenorM = item.value("b_menor_m").toString().trimmed();
        registro.talude = item.value("talude").toString().trimmed();
        registro.diametroM = item.value("diametro_m").toString().trimmed();
        if (registro.nomenclatura.isEmpty()) continue;
        secoes.append(registro);
    }

    QVector<TipoUsoOcupacaoSolo> tiposUsoOcupacao;
    tiposUsoOcupacao.reserve(usosSoloJson.size());
    for (const QJsonValue& valor : usosSoloJson) {
        const QJsonObject item = valor.toObject();
        const QString id = item.value("id").toString().trimmed();
        if (id.isEmpty()) continue;

        TipoUsoOcupacaoSolo tipo(id);
        tipo.setNManning(item.value("n_manning").toDouble());
        tipo.setCRunoff(item.value("c_runoff").toDouble());
        tipo.setCurveNumber(item.value("curve_number").toDouble());
        tipo.setFatorR(item.value("fator_r").toDouble());
        tipo.setFatorK(item.value("fator_k").toDouble());
        tipo.setFatorC(item.value("fator_c").toDouble());
        tipo.setFatorP(item.value("fator_p").toDouble());
        tipo.setDensidadeKgM3(item.value("densidade_kg_m3").toDouble());
        tiposUsoOcupacao.append(tipo);
    }

    QVector<Canal> canais;
    QVector<QString> secoesCanal;
    canais.reserve(canaisJson.size());
    secoesCanal.reserve(canaisJson.size());
    for (const QJsonValue& valor : canaisJson) {
        const QJsonObject item = valor.toObject();
        const QString id = item.value("id").toString().trimmed();
        if (id.isEmpty()) continue;

        Canal canal(id);
        canal.setIdJusante(item.value("id_jusante").toString());
        canal.setComprimento(item.value("comprimento_m").toDouble());
        canal.setLarguraFundo(item.value("largura_fundo_m").toDouble());
        canal.setTaludeLateral(item.value("talude_lateral").toDouble());
        canal.setDeclividadeFinal(item.value("declividade_final_m_m").toDouble());
        canal.setDeclividadeMinima(item.value("declividade_min_m_m").toDouble());
        canal.setDeclividadeMaxima(item.value("declividade_max_m_m").toDouble());
        canal.setCoeficienteManning(item.value("coeficiente_manning").toDouble());

        canais.append(canal);
        secoesCanal.append(item.value("secao").toString().trimmed());
    }

    QVector<BaciaContribuicao> bacias;
    QVector<QString> idsUsoOcupacaoPorLinhaBacia;
    bacias.reserve(baciasJson.size());
    idsUsoOcupacaoPorLinhaBacia.reserve(baciasJson.size());
    for (const QJsonValue& valor : baciasJson) {
        const QJsonObject item = valor.toObject();
        const QString nomeBacia = item.value("bacia").toString().trimmed();
        const QString nomeSubbacia = item.value("subbacia").toString().trimmed();
        QString id = item.value("id").toString().trimmed();
        if (!nomeBacia.isEmpty() && !nomeSubbacia.isEmpty()) {
            id = QString("%1-%2").arg(nomeBacia, nomeSubbacia);
        }
        if (id.isEmpty()) continue;

        BaciaContribuicao bacia(id);
        bacia.setIdJusante(item.value("id_jusante").toString());
        bacia.setAreaKm2(item.value("area_km2").toDouble());
        bacia.setDeclividadeMedia(item.value("declividade_media").toDouble());
        bacia.setComprimentoTalveguePrincipalKm(item.value("comprimento_talvegue_km").toDouble());
        bacia.setC_10(item.value("c10").toDouble());

        bacias.append(bacia);
        idsUsoOcupacaoPorLinhaBacia.append(item.value("uso_ocupacao_solo").toString().trimmed());
    }

    m_modeloSecoes->definirRegistros(secoes);
    if (!tiposUsoOcupacao.isEmpty()) {
        m_modeloUsoOcupacaoSolo->definirTiposUsoOcupacao(tiposUsoOcupacao);
    }
    m_modeloCanais->definirCanais(canais);
    m_modeloBacias->definirBacias(bacias);

    const int totalSecoesCanal = std::min(m_modeloCanais->rowCount(), static_cast<int>(secoesCanal.size()));
    for (int linha = 0; linha < totalSecoesCanal; ++linha) {
        m_modeloCanais->setData(
            m_modeloCanais->index(linha, ModeloTabelaCanais::ColunaSecaoTransversal),
            secoesCanal.at(linha),
            Qt::EditRole);
    }

    const int totalUsosBacia = std::min(m_modeloBacias->rowCount(), static_cast<int>(idsUsoOcupacaoPorLinhaBacia.size()));
    for (int linha = 0; linha < totalUsosBacia; ++linha) {
        m_modeloBacias->setData(
            m_modeloBacias->index(linha, ModeloTabelaBacias::ColunaUsoOcupacaoSolo),
            idsUsoOcupacaoPorLinhaBacia.at(linha),
            Qt::EditRole);
    }

    for (QTableView* tabela : m_tabelaPorChave) {
        ajustarLarguraColunasTabela(tabela);
    }

    calcularResultadosModelo();
    return true;
}

void MainWindow::salvarProjetoEmArquivo()
{
    const QString caminhoArquivo = QFileDialog::getSaveFileName(
        this,
        "Salvar projeto",
        QString(),
        "Projeto SISTEMAHDR (*.sishdr.json);;JSON (*.json)");
    if (caminhoArquivo.isEmpty()) return;

    QFile arquivo(caminhoArquivo);
    if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Salvar projeto", "Não foi possível salvar o arquivo informado.");
        return;
    }

    const QJsonDocument documento(criarObjetoJsonProjeto());
    arquivo.write(documento.toJson(QJsonDocument::Indented));
    arquivo.close();

    statusBar()->showMessage("Projeto salvo com sucesso.", 2500);
}

void MainWindow::abrirProjetoDeArquivo()
{
    const QString caminhoArquivo = QFileDialog::getOpenFileName(
        this,
        "Abrir projeto",
        QString(),
        "Projeto SISTEMAHDR (*.sishdr.json *.json);;JSON (*.json)");
    if (caminhoArquivo.isEmpty()) return;

    QFile arquivo(caminhoArquivo);
    if (!arquivo.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Abrir projeto", "Não foi possível abrir o arquivo informado.");
        return;
    }

    const QByteArray conteudo = arquivo.readAll();
    arquivo.close();

    QJsonParseError erroParse;
    const QJsonDocument documento = QJsonDocument::fromJson(conteudo, &erroParse);
    if (erroParse.error != QJsonParseError::NoError || !documento.isObject()) {
        QMessageBox::warning(this, "Abrir projeto", "Arquivo inválido para projeto SISTEMAHDR.");
        return;
    }

    if (!aplicarObjetoJsonProjeto(documento.object())) {
        QMessageBox::warning(this, "Abrir projeto", "Os dados do arquivo estão incompletos ou inválidos.");
        return;
    }

    statusBar()->showMessage("Projeto carregado com sucesso.", 2500);
}

void MainWindow::importarBaciasDeCsv()
{
    const QString caminhoArquivo = QFileDialog::getOpenFileName(
        this,
        "Importar bacias por CSV",
        QString(),
        "Arquivo CSV (*.csv)");
    if (caminhoArquivo.isEmpty()) return;

    QFile arquivo(caminhoArquivo);
    if (!arquivo.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Importar CSV", "Não foi possível abrir o arquivo CSV informado.");
        return;
    }

    const QByteArray conteudoBytes = arquivo.readAll();
    arquivo.close();

    QString conteudo = QString::fromUtf8(conteudoBytes);
    if (conteudo.contains(QChar::ReplacementCharacter)) {
        conteudo = QString::fromLocal8Bit(conteudoBytes);
    }

    conteudo.replace("\r\n", "\n");
    conteudo.replace('\r', '\n');
    const QStringList linhas = conteudo.split('\n', Qt::SkipEmptyParts);
    if (linhas.size() <= 1) {
        QMessageBox::warning(this, "Importar CSV", "Arquivo CSV vazio ou sem dados válidos.");
        return;
    }

    QVector<BaciaContribuicao> baciasImportadas;
    baciasImportadas.reserve(linhas.size() - 1);

    QSet<QString> combinacoesUnicas;
    const QString separadorCombinacao = "||";

    QStringList erros;
    for (int i = 1; i < linhas.size(); ++i) {
        const QString linhaTexto = linhas.at(i).trimmed();
        if (linhaTexto.isEmpty()) continue;

        const QStringList colunas = linhaTexto.split(',', Qt::KeepEmptyParts);
        if (colunas.size() < 6) {
            erros << QString("Linha %1 inválida: quantidade de colunas insuficiente.").arg(i + 1);
            continue;
        }

        const QString nomeBacia = colunas.at(0).trimmed();
        const QString nomeSubbacia = colunas.at(1).trimmed();
        const QString idJusante = colunas.at(2).trimmed();

        const double areaM2 = colunas.at(3).trimmed().toDouble();
        const double lTalvegueM = colunas.at(4).trimmed().toDouble();

        QString declividadeTexto = colunas.at(5).trimmed();
        declividadeTexto.remove('%');
        const double declividadePercentual = declividadeTexto.toDouble();

        if (nomeBacia.isEmpty() || nomeSubbacia.isEmpty()) {
            erros << QString("Linha %1 inválida: Bacia/Subbacia vazias.").arg(i + 1);
            continue;
        }

        const QString chaveCombinacao = QString("%1%2%3")
            .arg(nomeBacia.toUpper(), separadorCombinacao, nomeSubbacia.toUpper());
        if (combinacoesUnicas.contains(chaveCombinacao)) {
            erros << QString("Linha %1 inválida: combinação Bacia-Subbacia duplicada no CSV.").arg(i + 1);
            continue;
        }
        combinacoesUnicas.insert(chaveCombinacao);

        const QString idComposto = QString("%1-%2").arg(nomeBacia, nomeSubbacia);
        BaciaContribuicao bacia(idComposto);
        bacia.setIdJusante(idJusante);
        bacia.setAreaM2(areaM2);
        bacia.setComprimentoTalveguePrincipalKm(std::max(0.0, lTalvegueM) / 1000.0);
        bacia.setDeclividadeMedia(std::max(0.0, declividadePercentual) / 100.0);
        bacia.setC_10(0.6);

        baciasImportadas.append(bacia);
    }

    if (!erros.isEmpty()) {
        QMessageBox::warning(this,
                             "Importar CSV",
                             QString("Foram encontrados erros durante a importação:\n- %1")
                                 .arg(erros.join("\n- ")));
        return;
    }

    if (baciasImportadas.isEmpty()) {
        QMessageBox::warning(this, "Importar CSV", "Nenhuma bacia válida encontrada no arquivo CSV.");
        return;
    }

    m_modeloBacias->definirBacias(baciasImportadas);

    if (m_tabelaPorChave.contains(kAbaBacias)) {
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaBacias));
    }

    calcularResultadosModelo();
    statusBar()->showMessage("Bacias importadas com sucesso do CSV.", 3000);
}

void MainWindow::importarUsoOcupacaoSoloDeCsv()
{
    const QString caminhoArquivo = QFileDialog::getOpenFileName(
        this,
        "Importar uso e ocupação do solo por CSV",
        QString(),
        "Arquivo CSV (*.csv)");
    if (caminhoArquivo.isEmpty()) return;

    QFile arquivo(caminhoArquivo);
    if (!arquivo.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Importar CSV", "Não foi possível abrir o arquivo CSV informado.");
        return;
    }

    const QByteArray conteudoBytes = arquivo.readAll();
    arquivo.close();

    QString conteudo = QString::fromUtf8(conteudoBytes);
    if (conteudo.contains(QChar::ReplacementCharacter)) {
        conteudo = QString::fromLocal8Bit(conteudoBytes);
    }

    conteudo.replace("\r\n", "\n");
    conteudo.replace('\r', '\n');
    const QStringList linhas = conteudo.split('\n', Qt::SkipEmptyParts);
    if (linhas.size() <= 1) {
        QMessageBox::warning(this, "Importar CSV", "Arquivo CSV vazio ou sem dados válidos.");
        return;
    }

    QVector<TipoUsoOcupacaoSolo> tiposImportados;
    tiposImportados.reserve(linhas.size() - 1);

    QSet<QString> idsUnicos;
    QStringList erros;

    for (int i = 1; i < linhas.size(); ++i) {
        const QString linhaTexto = linhas.at(i).trimmed();
        if (linhaTexto.isEmpty()) continue;

        const QStringList colunas = linhaTexto.split(',', Qt::KeepEmptyParts);
        if (colunas.size() < 9) {
            erros << QString("Linha %1 inválida: quantidade de colunas insuficiente.").arg(i + 1);
            continue;
        }

        const QString id = colunas.at(0).trimmed();
        if (id.isEmpty()) {
            erros << QString("Linha %1 inválida: ID vazio.").arg(i + 1);
            continue;
        }

        const QString idNormalizado = id.toUpper();
        if (idsUnicos.contains(idNormalizado)) {
            erros << QString("Linha %1 inválida: ID duplicado no CSV.").arg(i + 1);
            continue;
        }
        idsUnicos.insert(idNormalizado);

        TipoUsoOcupacaoSolo tipo(id);
        tipo.setNManning(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(1))));
        tipo.setCRunoff(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(2))));
        tipo.setCurveNumber(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(3))));
        tipo.setFatorR(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(4))));
        tipo.setFatorK(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(5))));
        tipo.setFatorC(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(6))));
        tipo.setFatorP(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(7))));
        tipo.setDensidadeKgM3(std::max(0.0, converterTextoParaNumeroCsv(colunas.at(8))));

        tiposImportados.append(tipo);
    }

    if (!erros.isEmpty()) {
        QMessageBox::warning(this,
                             "Importar CSV",
                             QString("Foram encontrados erros durante a importação:\n- %1")
                                 .arg(erros.join("\n- ")));
        return;
    }

    if (tiposImportados.isEmpty()) {
        QMessageBox::warning(this, "Importar CSV", "Nenhum tipo de uso/ocupação válido encontrado no CSV.");
        return;
    }

    m_modeloUsoOcupacaoSolo->definirTiposUsoOcupacao(tiposImportados);
    m_modeloBacias->sincronizarCoeficienteRunoffPeloUsoOcupacao();

    if (m_tabelaPorChave.contains(kAbaBacias)) {
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaBacias));
    }

    if (m_tabelaPorChave.contains(kAbaUsoOcupacaoSolo)) {
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaUsoOcupacaoSolo));
    }

    calcularResultadosModelo();
    statusBar()->showMessage("Uso e ocupação do solo importados com sucesso do CSV.", 3000);
}

void MainWindow::importarCanaisDeCsv()
{
    const QString caminhoArquivo = QFileDialog::getOpenFileName(
        this,
        "Importar canais por CSV",
        QString(),
        "Arquivo CSV (*.csv)");
    if (caminhoArquivo.isEmpty()) return;

    QFile arquivo(caminhoArquivo);
    if (!arquivo.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Importar CSV", "Não foi possível abrir o arquivo CSV informado.");
        return;
    }

    const QByteArray conteudoBytes = arquivo.readAll();
    arquivo.close();

    QString conteudo = QString::fromUtf8(conteudoBytes);
    if (conteudo.contains(QChar::ReplacementCharacter)) {
        conteudo = QString::fromLocal8Bit(conteudoBytes);
    }

    conteudo.replace("\r\n", "\n");
    conteudo.replace('\r', '\n');
    const QStringList linhas = conteudo.split('\n', Qt::SkipEmptyParts);
    if (linhas.size() <= 1) {
        QMessageBox::warning(this, "Importar CSV", "Arquivo CSV vazio ou sem dados válidos.");
        return;
    }

    QVector<Canal> canaisImportados;
    QVector<QString> secoesPorLinha;
    canaisImportados.reserve(linhas.size() - 1);
    secoesPorLinha.reserve(linhas.size() - 1);

    QSet<QString> idsUnicos;
    QStringList erros;

    for (int i = 1; i < linhas.size(); ++i) {
        const QString linhaTexto = linhas.at(i).trimmed();
        if (linhaTexto.isEmpty()) continue;

        const QStringList colunas = linhaTexto.split(',', Qt::KeepEmptyParts);
        if (colunas.size() < 6) {
            erros << QString("Linha %1 inválida: quantidade de colunas insuficiente.").arg(i + 1);
            continue;
        }

        const QString idCanal = colunas.at(0).trimmed();
        const QString idJusante = colunas.at(1).trimmed();
        const QString idSecao = (colunas.size() >= 7)
                                    ? colunas.at(6).trimmed()
                                    : QString("VTD-1");

        if (idCanal.isEmpty()) {
            erros << QString("Linha %1 inválida: ID do canal vazio.").arg(i + 1);
            continue;
        }

        const QString idNormalizado = idCanal.toUpper();
        if (idsUnicos.contains(idNormalizado)) {
            erros << QString("Linha %1 inválida: ID de canal duplicado no CSV.").arg(i + 1);
            continue;
        }
        idsUnicos.insert(idNormalizado);

        const double comprimentoM = std::max(0.0, converterTextoParaNumeroCsv(colunas.at(2)));
        const double spMinPercentual = std::max(0.0, converterTextoParaNumeroCsv(colunas.at(3)));
        const double spMaxPercentual = std::max(0.0, converterTextoParaNumeroCsv(colunas.at(4)));
        const double soFinalPercentual = std::max(0.0, converterTextoParaNumeroCsv(colunas.at(5)));

        Canal canal(idCanal);
        canal.setIdJusante(idJusante);
        canal.setComprimento(comprimentoM);
        canal.setDeclividadeMinima(spMinPercentual / 100.0);
        canal.setDeclividadeMaxima(spMaxPercentual / 100.0);
        canal.setDeclividadeFinal(soFinalPercentual / 100.0);

        canaisImportados.append(canal);
        secoesPorLinha.append(idSecao.isEmpty() ? QString("VTD-1") : idSecao);
    }

    if (!erros.isEmpty()) {
        QMessageBox::warning(this,
                             "Importar CSV",
                             QString("Foram encontrados erros durante a importação:\n- %1")
                                 .arg(erros.join("\n- ")));
        return;
    }

    if (canaisImportados.isEmpty()) {
        QMessageBox::warning(this, "Importar CSV", "Nenhum canal válido encontrado no CSV.");
        return;
    }

    m_modeloCanais->definirCanais(canaisImportados);

    const int totalSecoes = std::min(m_modeloCanais->rowCount(), static_cast<int>(secoesPorLinha.size()));
    for (int linha = 0; linha < totalSecoes; ++linha) {
        m_modeloCanais->setData(
            m_modeloCanais->index(linha, ModeloTabelaCanais::ColunaSecaoTransversal),
            secoesPorLinha.at(linha),
            Qt::EditRole);
    }

    if (m_tabelaPorChave.contains(kAbaCanais)) {
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaCanais));
    }

    calcularResultadosModelo();
    statusBar()->showMessage("Canais importados com sucesso do CSV.", 3000);
}

void MainWindow::configurarAcoesCalculo()
{
    m_acaoCalcularResultados = new QAction("Calcular resultados", this);
    m_acaoCalcularResultados->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(m_acaoCalcularResultados, &QAction::triggered,
            this,
            [this]() {
                calcularResultadosModelo();
                statusBar()->showMessage("Resultados recalculados com sucesso.", 2500);
            });

    QToolBar* barraCalculo = addToolBar("Resultados");
    barraCalculo->setMovable(false);
    barraCalculo->addAction(m_acaoCalcularResultados);
}

void MainWindow::mostrarMenuContextoArvore(const QPoint& posicao)
{
    if (!m_arvoreModelo) return;

    QModelIndex indice = m_arvoreModelo->indexAt(posicao);
    if (!indice.isValid()) {
        indice = m_arvoreModelo->currentIndex();
    }

    const QString tipo = indice.data(kRoleTipoItem).toString();
    const bool emResultados = (tipo == "resultados"
                               || tipo == "resultados_canais"
                               || tipo == "resultados_bacias");
    if (!emResultados) return;

    QMenu menu(this);
    QAction* acaoCalcular = menu.addAction("Calcular resultados");

    const QAction* acaoSelecionada = menu.exec(m_arvoreModelo->viewport()->mapToGlobal(posicao));
    if (acaoSelecionada == acaoCalcular) {
        calcularResultadosModelo();
        statusBar()->showMessage("Resultados recalculados com sucesso.", 2500);
    }
}

void MainWindow::atualizarModeloResultadosCanais()
{
    if (!m_modeloResultadosCanais || !m_modeloCanais) return;

    RedeHidrologica rede;
    // Registra elementos primeiro sem conexão jusante e conecta em seguida.
    QMap<QString, QString> jusantePorElemento;

    for (const Canal& canal : m_modeloCanais->canais()) {
        Canal canalSemConexao = canal;
        jusantePorElemento.insert(canalSemConexao.id(), canal.idJusante());
        canalSemConexao.setIdJusante(QString());
        rede.adicionarCanal(canalSemConexao);
    }

    for (const BaciaContribuicao& bacia : m_modeloBacias->bacias()) {
        BaciaContribuicao baciaSemConexao = bacia;
        jusantePorElemento.insert(baciaSemConexao.id(), bacia.idJusante());
        baciaSemConexao.setIdJusante(QString());
        rede.adicionarBacia(baciaSemConexao);
    }

    for (auto it = jusantePorElemento.cbegin(); it != jusantePorElemento.cend(); ++it) {
        if (it.value().trimmed().isEmpty()) continue;
        rede.definirJusanteElemento(it.key(), it.value());
    }

    const IDF idf(279.63, 0.00, 0.524, 0.00);
    const double tempoRetornoAnos = 50.0;

    m_modeloResultadosCanais->clear();
    m_modeloResultadosCanais->setHorizontalHeaderLabels(
        { "ID", "A_total (m\u00B2)", "L_talvegue_total (m)", "S_talvegue_total (%)", "Cmed", "Tc_Atotal (Kirpich-modificado)",
          "I_Atotal_TR (mm/h)", "Qp (dimensionamento)", "Se\u00E7\u00E3o",
          "Hn_Sp(min)", "T_Sp(min)", "Pm_Sp(min)", "Am_Sp(min)", "Rh_Sp(min)", "Ph_Sp(min)", "Folga_Sp(min)", "V_Sp(min)", "A(%)_Sp(min)", "Fr_V_Sp(min)", "tau_o_min",
          "Hn_Sp(max)", "T_Sp(max)", "Pm_Sp(max)", "Am_Sp(max)", "Rh_Sp(max)", "Ph_Sp(max)", "Folga_Sp(max)", "V_Sp(max)", "A(%)_Sp(max)", "Fr_V_Sp(max)", "tau_o_max",
          "Hn_Sp(final)", "V_Sp(final)", "tau_o_final",
          "Crit_Qp_Smin", "Crit_Qp_Smax", "Crit_Altura_Smin", "Crit_V_Smax", "Crit_Geral" });

    const QVector<Canal>& canais = m_modeloCanais->canais();
    for (int linha = 0; linha < canais.size(); ++linha) {
        const Canal& canal = canais.at(linha);
        const QString idSecao = m_modeloCanais->idSecaoPorLinha(linha);
        const QString idCanal = canal.id();

        double lTalvegueCriticoKm = 0.0;
        double declividadeTalvegueCritica = 0.0;
        rede.calcularTalvegueCriticoAteElemento(idCanal, &lTalvegueCriticoKm, &declividadeTalvegueCritica);

        const double areaTotalKm2 = std::max(0.0, rede.areaAcumuladaTotalContribuinte(idCanal));
        const double areaTotalM2 = areaTotalKm2 * 1e6;
        const double lTalvegueTotalM = std::max(0.0, lTalvegueCriticoKm) * 1000.0;
        const double cMedio = std::max(0.0, rede.coeficienteEscoamentoMedioPonderado(idCanal));
        const double tcAtotalMin = std::max(0.0, rede.tempoConcentracaoKirpichModificadoAreaTotal(idCanal));
        const double intensidadeMmH = std::max(0.0, idf.intensidadeMmH(tempoRetornoAnos, tcAtotalMin));
        const double qDimensionamento = std::max(0.0, 0.278 * cMedio * intensidadeMmH * areaTotalKm2);

        const double sTalvegueTotalPercentual = std::max(0.0, declividadeTalvegueCritica) * 100.0;

        const double alturaMaxSecaoM = std::max(1e-6, alturaMaximaSecaoPorId(idSecao, m_modeloSecoes));

        Canal canalMin = canal;
        aplicarSecaoTransversalNoCanal(&canalMin, idSecao, m_modeloSecoes);
        canalMin.setDeclividadeFinal(canal.declividadeMinima());
        const double hMin = canalMin.alturaLaminaParaVazaoProjeto(qDimensionamento);
        const double tMin = canalMin.larguraSuperficial(hMin);
        const double pMin = canalMin.perimetroMolhado(hMin);
        const double aMin = canalMin.areaMolhada(hMin);
        const double rhMin = canalMin.raioHidraulico(hMin);
        const double folgaMin = std::max(0.0, alturaMaxSecaoM - std::max(0.0, hMin));
        const double vMin = canalMin.velocidadeManning(hMin);
        const double ocupacaoMin = std::max(0.0, hMin) / alturaMaxSecaoM * 100.0;
        const double frMin = calcularFroude(vMin, aMin, tMin);
        const double tauMin = calcularTensaoCisalhantePa(rhMin, std::max(0.0, canal.declividadeMinima()));

        Canal canalMax = canal;
        aplicarSecaoTransversalNoCanal(&canalMax, idSecao, m_modeloSecoes);
        canalMax.setDeclividadeFinal(canal.declividadeMaxima());
        const double hMax = canalMax.alturaLaminaParaVazaoProjeto(qDimensionamento);
        const double tMax = canalMax.larguraSuperficial(hMax);
        const double pMax = canalMax.perimetroMolhado(hMax);
        const double aMax = canalMax.areaMolhada(hMax);
        const double rhMax = canalMax.raioHidraulico(hMax);
        const double folgaMax = std::max(0.0, alturaMaxSecaoM - std::max(0.0, hMax));
        const double vMax = canalMax.velocidadeManning(hMax);
        const double ocupacaoMax = std::max(0.0, hMax) / alturaMaxSecaoM * 100.0;
        const double frMax = calcularFroude(vMax, aMax, tMax);
        const double tauMax = calcularTensaoCisalhantePa(rhMax, std::max(0.0, canal.declividadeMaxima()));

        Canal canalFinal = canal;
        aplicarSecaoTransversalNoCanal(&canalFinal, idSecao, m_modeloSecoes);
        canalFinal.setDeclividadeFinal(canal.declividadeFinal());
        const double hFinal = canalFinal.alturaLaminaParaVazaoProjeto(qDimensionamento);
        const double vFinal = canalFinal.velocidadeManning(hFinal);
        const double rhFinal = canalFinal.raioHidraulico(hFinal);
        const double tauFinal = calcularTensaoCisalhantePa(rhFinal, std::max(0.0, canal.declividadeFinal()));

        // Critérios de verificação hidráulica do canal.
        constexpr double kVelocidadeMaximaAdmissivelMps = 5.0;
        EntradaVerificacaoCanal entradaVerificacao;
        entradaVerificacao.qpHidrologiaM3s = qDimensionamento;
        entradaVerificacao.qpHidraulicaSMinM3s = canalMin.vazaoManning(std::max(0.0, hMin));
        entradaVerificacao.qpHidraulicaSMaxM3s = canalMax.vazaoManning(std::max(0.0, hMax));
        entradaVerificacao.alturaSMinM = std::max(0.0, hMin);
        entradaVerificacao.alturaMaximaSecaoM = alturaMaxSecaoM;
        entradaVerificacao.folgaMinimaM = folgaMin;
        entradaVerificacao.velocidadeSMaxMps = std::max(0.0, vMax);
        entradaVerificacao.velocidadeMaximaAdmissivelMps = kVelocidadeMaximaAdmissivelMps;

        const ResultadoVerificacaoCanal resultadoVerificacao = CalculoHidraulicoCanal::verificarCriterios(entradaVerificacao);

        auto textoNumero = [](double valor, int casas = 3) {
            return std::isfinite(valor)
                       ? QString::number(valor, 'f', casas)
                       : QString("n/d");
        };

        QList<QStandardItem*> itensLinha;
        itensLinha << new QStandardItem(canal.id())
                   << new QStandardItem(textoNumero(areaTotalM2, 2))
                   << new QStandardItem(textoNumero(lTalvegueTotalM, 2))
                   << new QStandardItem(textoNumero(sTalvegueTotalPercentual, 3))
                   << new QStandardItem(textoNumero(cMedio, 3))
                   << new QStandardItem(textoNumero(tcAtotalMin, 2))
                   << new QStandardItem(textoNumero(intensidadeMmH, 2))
                   << new QStandardItem(textoNumero(qDimensionamento, 3))
                   << new QStandardItem(idSecao)
                   << new QStandardItem(textoNumero(hMin, 3))
                   << new QStandardItem(textoNumero(tMin, 3))
                   << new QStandardItem(textoNumero(pMin, 3))
                   << new QStandardItem(textoNumero(aMin, 3))
                   << new QStandardItem(textoNumero(rhMin, 3))
                   << new QStandardItem(textoNumero(pMin, 3))
                   << new QStandardItem(textoNumero(folgaMin, 3))
                   << new QStandardItem(textoNumero(vMin, 3))
                   << new QStandardItem(textoNumero(ocupacaoMin, 2))
                   << new QStandardItem(textoNumero(frMin, 3))
                   << new QStandardItem(textoNumero(tauMin, 2))
                   << new QStandardItem(textoNumero(hMax, 3))
                   << new QStandardItem(textoNumero(tMax, 3))
                   << new QStandardItem(textoNumero(pMax, 3))
                   << new QStandardItem(textoNumero(aMax, 3))
                   << new QStandardItem(textoNumero(rhMax, 3))
                   << new QStandardItem(textoNumero(pMax, 3))
                   << new QStandardItem(textoNumero(folgaMax, 3))
                   << new QStandardItem(textoNumero(vMax, 3))
                   << new QStandardItem(textoNumero(ocupacaoMax, 2))
                   << new QStandardItem(textoNumero(frMax, 3))
                   << new QStandardItem(textoNumero(tauMax, 2))
                   << new QStandardItem(textoNumero(hFinal, 3))
                   << new QStandardItem(textoNumero(vFinal, 3))
                   << new QStandardItem(textoNumero(tauFinal, 2))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioQpSMin))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioQpSMax))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioAlturaSMin))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioVelocidadeSMax))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.todosAtendidos()));

        for (QStandardItem* item : itensLinha) {
            item->setEditable(false);
        }

        m_modeloResultadosCanais->appendRow(itensLinha);
    }

    if (m_tabelaPorChave.contains(kAbaResultadosCanais)) {
        configurarVisibilidadeColunasResultadosCanais(m_tabelaPorChave.value(kAbaResultadosCanais), "geral");
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaResultadosCanais));
    }
    if (m_tabelaPorChave.contains(kAbaResultadosCanaisDeclividadeMinima)) {
        configurarVisibilidadeColunasResultadosCanais(m_tabelaPorChave.value(kAbaResultadosCanaisDeclividadeMinima), "min");
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaResultadosCanaisDeclividadeMinima));
    }
    if (m_tabelaPorChave.contains(kAbaResultadosCanaisDeclividadeMaxima)) {
        configurarVisibilidadeColunasResultadosCanais(m_tabelaPorChave.value(kAbaResultadosCanaisDeclividadeMaxima), "max");
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaResultadosCanaisDeclividadeMaxima));
    }
    if (m_tabelaPorChave.contains(kAbaResultadosCanaisDeclividadeFinal)) {
        configurarVisibilidadeColunasResultadosCanais(m_tabelaPorChave.value(kAbaResultadosCanaisDeclividadeFinal), "final");
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaResultadosCanaisDeclividadeFinal));
    }
}

void MainWindow::atualizarModeloResultadosBacias()
{
    if (!m_modeloResultadosBacias || !m_modeloBacias) return;

    m_modeloResultadosBacias->clear();
    m_modeloResultadosBacias->setHorizontalHeaderLabels(
        { "ID", "ID Jusante", "\u00C1rea (km\u00B2)", "Declividade m\u00E9dia", "L_talvegue (km)", "Tc local (min)", "Q racional (i=100 mm/h)" });

    const QVector<BaciaContribuicao>& bacias = m_modeloBacias->bacias();
    for (const BaciaContribuicao& bacia : bacias) {
        const double tcLocalMin = bacia.tempoConcentracaoKirpichModificado(0.0);
        const double qRacional = bacia.calcularContribuicaoRacional(100.0);

        QList<QStandardItem*> itensLinha;
        itensLinha << new QStandardItem(bacia.id())
                   << new QStandardItem(bacia.idJusante())
                   << new QStandardItem(QString::number(bacia.areaKm2(), 'f', 3))
                   << new QStandardItem(QString::number(bacia.declividadeMedia(), 'f', 4))
                   << new QStandardItem(QString::number(bacia.comprimentoTalveguePrincipalKm(), 'f', 3))
                   << new QStandardItem(QString::number(tcLocalMin, 'f', 2))
                   << new QStandardItem(QString::number(qRacional, 'f', 3));

        for (QStandardItem* item : itensLinha) {
            item->setEditable(false);
        }

        m_modeloResultadosBacias->appendRow(itensLinha);
    }

    if (m_tabelaPorChave.contains(kAbaResultadosBacias)) {
        ajustarLarguraColunasTabela(m_tabelaPorChave.value(kAbaResultadosBacias));
    }
}

void MainWindow::configurarAcaoAdicionarCanal(QTableView* tabela)
{
    if (!tabela || !m_modeloCanais) return;

    connect(tabela, &QWidget::customContextMenuRequested, this, [this, tabela](const QPoint& posicao) {
        const QModelIndex indiceClicado = tabela->indexAt(posicao);
        if (indiceClicado.isValid()) {
            tabela->setCurrentIndex(indiceClicado);
        }

        QMenu menu(this);
        QAction* acaoAdicionar = menu.addAction("Adicionar novo canal");
        QAction* acaoRemover = menu.addAction("Remover canal da linha atual");

        const bool linhaValidaSelecionada = tabela->currentIndex().isValid();
        acaoRemover->setEnabled(linhaValidaSelecionada);

        const QAction* acaoSelecionada = menu.exec(tabela->viewport()->mapToGlobal(posicao));
        if (acaoSelecionada == acaoRemover) {
            const QModelIndex indiceAtual = tabela->currentIndex();
            if (!indiceAtual.isValid()) return;

            const int linhaRemocao = indiceAtual.row();
            QString mensagemErro;
            if (!m_modeloCanais->removerCanal(linhaRemocao, &mensagemErro)) {
                QMessageBox::warning(this, "Remover canal", mensagemErro);
                return;
            }

            statusBar()->showMessage("Canal removido da tabela.", 2000);
            return;
        }

        if (acaoSelecionada != acaoAdicionar) return;

        while (true) {
            bool confirmou = false;
            const QString idCanal = QInputDialog::getText(
                this,
                "Novo canal",
                "Informe o ID do novo canal:",
                QLineEdit::Normal,
                QString(),
                &confirmou).trimmed();

            if (!confirmou) return;

            QString mensagemErro;
            if (m_modeloCanais->adicionarCanal(idCanal, &mensagemErro)) {
                const int linhaNova = m_modeloCanais->rowCount() - 1;
                tabela->setCurrentIndex(m_modeloCanais->index(linhaNova, ModeloTabelaCanais::ColunaSecaoTransversal));
                tabela->edit(m_modeloCanais->index(linhaNova, ModeloTabelaCanais::ColunaSecaoTransversal));
                return;
            }

            QMessageBox::warning(this, "Novo canal", mensagemErro);
        }
    });
}

void MainWindow::configurarAcaoAdicionarBacia(QTableView* tabela)
{
    if (!tabela || !m_modeloBacias) return;

    connect(tabela, &QWidget::customContextMenuRequested, this, [this, tabela](const QPoint& posicao) {
        const QModelIndex indiceClicado = tabela->indexAt(posicao);
        if (indiceClicado.isValid()) {
            tabela->setCurrentIndex(indiceClicado);
        }

        QMenu menu(this);
        QAction* acaoAdicionar = menu.addAction("Adicionar nova bacia");
        QAction* acaoRemover = menu.addAction("Remover bacia da linha atual");

        const bool linhaValidaSelecionada = tabela->currentIndex().isValid();
        acaoRemover->setEnabled(linhaValidaSelecionada);

        const QAction* acaoSelecionada = menu.exec(tabela->viewport()->mapToGlobal(posicao));
        if (acaoSelecionada == acaoRemover) {
            const QModelIndex indiceAtual = tabela->currentIndex();
            if (!indiceAtual.isValid()) return;

            const int linhaRemocao = indiceAtual.row();
            QString mensagemErro;
            if (!m_modeloBacias->removerBacia(linhaRemocao, &mensagemErro)) {
                QMessageBox::warning(this, "Remover bacia", mensagemErro);
                return;
            }

            statusBar()->showMessage("Bacia removida da tabela.", 2000);
            return;
        }

        if (acaoSelecionada != acaoAdicionar) return;

        while (true) {
            bool confirmouBacia = false;
            const QString nomeBacia = QInputDialog::getText(
                this,
                "Nova bacia",
                "Informe o nome da bacia:",
                QLineEdit::Normal,
                QString(),
                &confirmouBacia).trimmed();

            if (!confirmouBacia) return;

            bool confirmouSubbacia = false;
            const QString nomeSubbacia = QInputDialog::getText(
                this,
                "Nova bacia",
                "Informe o nome da subbacia:",
                QLineEdit::Normal,
                QString(),
                &confirmouSubbacia).trimmed();

            if (!confirmouSubbacia) return;

            QString mensagemErro;
            if (m_modeloBacias->adicionarBacia(nomeBacia, nomeSubbacia, &mensagemErro)) {
                const int linhaNova = m_modeloBacias->rowCount() - 1;
                tabela->setCurrentIndex(m_modeloBacias->index(linhaNova, ModeloTabelaBacias::ColunaIdJusante));
                return;
            }

            QMessageBox::warning(this, "Nova bacia", mensagemErro);
        }
    });
}

QStandardItem* MainWindow::criarItemArvore(QStandardItem* pai,
                                           const QString& texto,
                                           const QString& tipo,
                                           const QIcon& icone) const
{
    if (!pai) return nullptr;

    auto* item = new QStandardItem(icone, texto);
    item->setData(tipo, kRoleTipoItem);
    item->setEditable(false);
    pai->appendRow(item);
    return item;
}

QVector<Canal> MainWindow::criarCanaisExemplo() const
{
    // Baseado no cenário manual de `RedeHidrologicaTesteManual`.
    Canal d041("D04.1-ACO");
    d041.setIdJusante("D04.2-ACO");
    d041.setComprimento(110.00);
    d041.setDeclividadeMinima(2.85 / 100.0);
    d041.setDeclividadeMaxima(9.10 / 100.0);
    d041.setDeclividadeFinal(9.10 / 100.0);
    d041.setCoeficienteManning(0.015);

    Canal d042("D04.2-ACO");
    d042.setIdJusante("D04.3-ACO");
    d042.setComprimento(17.00);
    d042.setDeclividadeMinima(9.10 / 100.0);
    d042.setDeclividadeMaxima(11.60 / 100.0);
    d042.setDeclividadeFinal(11.60 / 100.0);
    d042.setCoeficienteManning(0.015);

    Canal d043("D04.3-ACO");
    d043.setIdJusante("D04.4-ACO");
    d043.setComprimento(89.32);
    d043.setDeclividadeMinima(3.50 / 100.0);
    d043.setDeclividadeMaxima(11.60 / 100.0);
    d043.setDeclividadeFinal(11.60 / 100.0);
    d043.setCoeficienteManning(0.015);

    Canal d044("D04.4-ACO");
    d044.setIdJusante("C04-ACO");
    d044.setComprimento(64.00);
    d044.setDeclividadeMinima(4.00 / 100.0);
    d044.setDeclividadeMaxima(8.50 / 100.0);
    d044.setDeclividadeFinal(8.50 / 100.0);
    d044.setCoeficienteManning(0.015);

    Canal d051("D05.1-ACO");
    d051.setIdJusante("D05.2-ACO");
    d051.setComprimento(47.00);
    d051.setDeclividadeMinima(3.00 / 100.0);
    d051.setDeclividadeMaxima(13.50 / 100.0);
    d051.setDeclividadeFinal(13.50 / 100.0);
    d051.setCoeficienteManning(0.015);

    Canal d052("D05.2-ACO");
    d052.setIdJusante("C03-ACO");
    d052.setComprimento(86.00);
    d052.setDeclividadeMinima(9.60 / 100.0);
    d052.setDeclividadeMaxima(15.80 / 100.0);
    d052.setDeclividadeFinal(15.80 / 100.0);
    d052.setCoeficienteManning(0.015);

    Canal d061("D06.1-ACO");
    d061.setIdJusante("EX-B1");
    d061.setComprimento(115.00);
    d061.setDeclividadeMinima(5.40 / 100.0);
    d061.setDeclividadeMaxima(14.50 / 100.0);
    d061.setDeclividadeFinal(14.50 / 100.0);
    d061.setCoeficienteManning(0.015);

    Canal d031("D03.1-ACO");
    d031.setIdJusante("C01-ACO");
    d031.setComprimento(50.00);
    d031.setDeclividadeMinima(3.00 / 100.0);
    d031.setDeclividadeMaxima(6.50 / 100.0);
    d031.setDeclividadeFinal(6.50 / 100.0);
    d031.setCoeficienteManning(0.015);

    Canal d021("D02.1-ACO");
    d021.setIdJusante("C01-ACO");
    d021.setComprimento(30.00);
    d021.setDeclividadeMinima(3.50 / 100.0);
    d021.setDeclividadeMaxima(5.00 / 100.0);
    d021.setDeclividadeFinal(5.00 / 100.0);
    d021.setCoeficienteManning(0.025);

    Canal d011("D01.1-ACO");
    d011.setIdJusante("C02-ACO");
    d011.setComprimento(40.00);
    d011.setDeclividadeMinima(1.00 / 100.0);
    d011.setDeclividadeMaxima(4.90 / 100.0);
    d011.setDeclividadeFinal(4.90 / 100.0);
    d011.setCoeficienteManning(0.015);

    return { d041, d042, d043, d044, d051, d052, d061, d031, d021, d011 };
}

QVector<BaciaContribuicao> MainWindow::criarBaciasExemplo() const
{
    // Baseado no cenário manual de `RedeHidrologicaTesteManual`.
    BaciaContribuicao sb01("BACIA01-SB01");
    sb01.setIdJusante("D04.1-ACO");
    sb01.setAreaKm2(5445.88 / 1e6);
    sb01.setDeclividadeMedia(16.00 / 100.0);
    sb01.setComprimentoTalveguePrincipalKm(74.69 / 1000.0);
    sb01.setC_10(0.6);

    BaciaContribuicao sb02("BACIA01-SB02");
    sb02.setIdJusante("D04.2-ACO");
    sb02.setAreaKm2(1244.39 / 1e6);
    sb02.setDeclividadeMedia(12.18 / 100.0);
    sb02.setComprimentoTalveguePrincipalKm(53.99 / 1000.0);
    sb02.setC_10(0.6);

    BaciaContribuicao sb03("BACIA01-SB03");
    sb03.setIdJusante("D04.3-ACO");
    sb03.setAreaKm2(10347.64 / 1e6);
    sb03.setDeclividadeMedia(12.18 / 100.0);
    sb03.setComprimentoTalveguePrincipalKm(140.97 / 1000.0);
    sb03.setC_10(0.6);

    BaciaContribuicao sb04("BACIA01-SB04");
    sb04.setIdJusante("D04.4-ACO");
    sb04.setAreaKm2(12175.37 / 1e6);
    sb04.setDeclividadeMedia(12.18 / 100.0);
    sb04.setComprimentoTalveguePrincipalKm(134.71 / 1000.0);
    sb04.setC_10(0.6);

    BaciaContribuicao sb032("BACIA03-SB02");
    sb032.setIdJusante("D02.1-ACO");
    sb032.setAreaKm2(1338.67 / 1e6);
    sb032.setDeclividadeMedia(11.78 / 100.0);
    sb032.setComprimentoTalveguePrincipalKm(38.78 / 1000.0);
    sb032.setC_10(0.6);

    BaciaContribuicao sb031("BACIA03-SB01");
    sb031.setIdJusante("D03.1-ACO");
    sb031.setAreaKm2(24281.00 / 1e6);
    sb031.setDeclividadeMedia(11.20 / 100.0);
    sb031.setComprimentoTalveguePrincipalKm(244.00 / 1000.0);
    sb031.setC_10(0.6);

    BaciaContribuicao sb021("BACIA02-SB01");
    sb021.setIdJusante("D01.1-ACO");
    sb021.setAreaKm2(7855.00 / 1e6);
    sb021.setDeclividadeMedia(9.80 / 100.0);
    sb021.setComprimentoTalveguePrincipalKm(168.89 / 1000.0);
    sb021.setC_10(0.6);

    BaciaContribuicao sb05("BACIA01-SB05");
    sb05.setIdJusante("D05.1-ACO");
    sb05.setAreaKm2(2043.70 / 1e6);
    sb05.setDeclividadeMedia(19.20 / 100.0);
    sb05.setComprimentoTalveguePrincipalKm(54.45 / 1000.0);
    sb05.setC_10(0.6);

    BaciaContribuicao sb06("BACIA01-SB06");
    sb06.setIdJusante("D05.2-ACO");
    sb06.setAreaKm2(11729.73 / 1e6);
    sb06.setDeclividadeMedia(18.00 / 100.0);
    sb06.setComprimentoTalveguePrincipalKm(117.00 / 1000.0);
    sb06.setC_10(0.6);

    BaciaContribuicao sb07("BACIA01-SB07");
    sb07.setIdJusante("D06.1-ACO");
    sb07.setAreaKm2(2854.25 / 1e6);
    sb07.setDeclividadeMedia(8.00 / 100.0);
    sb07.setComprimentoTalveguePrincipalKm(59.97 / 1000.0);
    sb07.setC_10(0.6);

    return { sb01, sb02, sb03, sb04, sb032, sb031, sb021, sb05, sb06, sb07 };
}



