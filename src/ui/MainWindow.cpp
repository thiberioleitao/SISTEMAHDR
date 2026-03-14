#include "MainWindow.h"

#include <QAbstractGraphicsShapeItem>
#include <QAbstractItemView>
#include <QAction>
#include <QDockWidget>
#include <QFrame>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QStatusBar>
#include <QStyle>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
constexpr auto kRoleElementoId = Qt::UserRole + 1;

QPen criarPenPadrao(const QColor& cor, qreal largura)
{
    QPen pen(cor, largura);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    return pen;
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    configurarJanela();
    configurarCentro();
    configurarArvoreRede();
    configurarPainelPropriedades();
    configurarMenuSuperior();
    popularArvoreRede();
    configurarCenaBase();
    selecionarPrimeiroElemento();
}

void MainWindow::configurarJanela()
{
    setWindowTitle("SISTEMAHDR - Explorador de Rede Hidrologica");
    resize(1520, 860);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);
    statusBar()->showMessage("Projeto pronto para edicao.");
}

void MainWindow::configurarMenuSuperior()
{
    auto* menuArquivo = menuBar()->addMenu("Arquivo");
    auto* menuView = menuBar()->addMenu("Exibir");
    auto* menuProject = menuBar()->addMenu("Projeto");
    auto* menuComponents = menuBar()->addMenu("Componentes");
    auto* menuTools = menuBar()->addMenu("Ferramentas");
    auto* menuWindow = menuBar()->addMenu("Janela");
    auto* menuHelp = menuBar()->addMenu("Ajuda");

    auto* acaoNovoProjeto = new QAction(style()->standardIcon(QStyle::SP_FileIcon), "Novo Projeto...", this);
    acaoNovoProjeto->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(acaoNovoProjeto, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Acao Novo Projeto acionada.", 4000);
    });

    auto* acaoAbrirProjeto = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Abrir Projeto...", this);
    acaoAbrirProjeto->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(acaoAbrirProjeto, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Acao Abrir Projeto acionada.", 4000);
    });

    auto* acaoSalvarProjeto = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Salvar Projeto", this);
    acaoSalvarProjeto->setShortcut(QKeySequence::Save);
    connect(acaoSalvarProjeto, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Projeto salvo (acao base da interface).", 4000);
    });

    auto* acaoSalvarComo = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Salvar Projeto Como...", this);
    acaoSalvarComo->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(acaoSalvarComo, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Acao Salvar Como acionada.", 4000);
    });

    auto* acaoSair = new QAction(style()->standardIcon(QStyle::SP_TitleBarCloseButton), "Sair", this);
    acaoSair->setShortcut(QKeySequence::Quit);
    connect(acaoSair, &QAction::triggered, this, &QWidget::close);

    menuArquivo->addAction(acaoNovoProjeto);
    menuArquivo->addAction(acaoAbrirProjeto);
    menuArquivo->addSeparator();
    menuArquivo->addAction(acaoSalvarProjeto);
    menuArquivo->addAction(acaoSalvarComo);
    menuArquivo->addSeparator();
    menuArquivo->addAction(acaoSair);

    auto* acaoAlternarExplorador = m_dockRede ? m_dockRede->toggleViewAction() : new QAction("Explorador do Projeto", this);
    acaoAlternarExplorador->setText("Explorador do Projeto");
    menuView->addAction(acaoAlternarExplorador);

    auto* acaoAlternarPropriedades = m_dockPropriedades ? m_dockPropriedades->toggleViewAction() : new QAction("Propriedades", this);
    acaoAlternarPropriedades->setText("Propriedades");
    menuView->addAction(acaoAlternarPropriedades);

    auto* acaoGerenciarProjeto = new QAction("Configuracoes do Projeto", this);
    connect(acaoGerenciarProjeto, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Configuracoes de projeto em preparacao.", 4000);
    });
    menuProject->addAction(acaoGerenciarProjeto);

    auto* submenuCriarComponente = menuComponents->addMenu("Criar Componente");
    auto adicionarAcaoComponente = [&](const QString& texto) {
        auto* acao = new QAction(texto, this);
        connect(acao, &QAction::triggered, this, [this, texto]() {
            statusBar()->showMessage(QString("Componente '%1' solicitado.").arg(texto), 4000);
        });
        submenuCriarComponente->addAction(acao);
    };

    adicionarAcaoComponente("Dados Pluviometricos");
    adicionarAcaoComponente("Analise e Cadastro de IDFs");
    adicionarAcaoComponente("Hidrologia e Perdas");
    adicionarAcaoComponente("Geometria e Topologia");
    adicionarAcaoComponente("Estruturas Hidraulicas");
    adicionarAcaoComponente("Resumos e Relatorios");

    auto* acaoFerramentas = new QAction("Catalogo de Ferramentas", this);
    connect(acaoFerramentas, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Catalogo de ferramentas em preparacao.", 4000);
    });
    menuTools->addAction(acaoFerramentas);

    auto* acaoOrganizarJanelas = new QAction("Restaurar Layout", this);
    connect(acaoOrganizarJanelas, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Layout restaurado para o padrao.", 4000);
        if (m_dockRede) {
            m_dockRede->show();
        }
        if (m_dockPropriedades) {
            m_dockPropriedades->show();
        }
    });
    menuWindow->addAction(acaoOrganizarJanelas);

    auto* acaoAjuda = new QAction("Sobre a Interface", this);
    connect(acaoAjuda, &QAction::triggered, this, [this]() {
        statusBar()->showMessage("Base de interface inspirada em RMC BestFit e HEC.", 4000);
    });
    menuHelp->addAction(acaoAjuda);

    auto* barraPrincipal = addToolBar("Projeto");
    barraPrincipal->setMovable(false);
    barraPrincipal->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    barraPrincipal->addAction(acaoNovoProjeto);
    barraPrincipal->addAction(acaoAbrirProjeto);
    barraPrincipal->addSeparator();
    barraPrincipal->addAction(acaoSalvarProjeto);
    barraPrincipal->addSeparator();
    barraPrincipal->addAction(acaoSair);
}

void MainWindow::configurarCentro()
{
    auto* paginaCentral = new QWidget(this);
    auto* layoutPrincipal = new QVBoxLayout(paginaCentral);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);
    layoutPrincipal->setSpacing(0);

    auto* cabecalho = new QFrame(paginaCentral);
    cabecalho->setObjectName("painelCabecalho");
    auto* layoutCabecalho = new QVBoxLayout(cabecalho);
    layoutCabecalho->setContentsMargins(18, 14, 18, 14);
    layoutCabecalho->setSpacing(4);

    m_tituloCentro = new QLabel("Mapa da rede", cabecalho);
    m_tituloCentro->setObjectName("tituloCentro");

    m_subtituloCentro = new QLabel("Selecione um elemento na arvore para destacar a geometria e os atributos na cena.", cabecalho);
    m_subtituloCentro->setObjectName("subtituloCentro");
    m_subtituloCentro->setWordWrap(true);

    layoutCabecalho->addWidget(m_tituloCentro);
    layoutCabecalho->addWidget(m_subtituloCentro);

    m_cenaRede = new QGraphicsScene(paginaCentral);
    m_viewRede = new QGraphicsView(m_cenaRede, paginaCentral);
    m_viewRede->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    m_viewRede->setDragMode(QGraphicsView::ScrollHandDrag);
    m_viewRede->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_viewRede->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_viewRede->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    m_viewRede->setBackgroundBrush(QColor("#edf3f8"));
    m_viewRede->setFrameShape(QFrame::NoFrame);

    layoutPrincipal->addWidget(cabecalho);
    layoutPrincipal->addWidget(m_viewRede, 1);

    setCentralWidget(paginaCentral);

    setStyleSheet(
        "QMainWindow { background: #dfe6ee; }"
        "QMenuBar { background: #f3f6fa; border-bottom: 1px solid #c8d3df; }"
        "QMenuBar::item:selected { background: #d7e6f6; }"
        "QToolBar { background: #eef4fa; border-bottom: 1px solid #c8d3df; spacing: 6px; padding: 4px; }"
        "QDockWidget { font-size: 12px; }"
        "QDockWidget::title { background: #1f4e79; color: white; padding: 8px 10px; }"
        "QTreeWidget, QTableWidget { background: #f7f9fc; border: 1px solid #c8d3df; }"
        "QTreeWidget::item:selected { background: #cfe3f6; color: #10253f; }"
        "QHeaderView::section { background: #e8eef5; padding: 6px; border: none; border-bottom: 1px solid #c8d3df; }"
        "QFrame#painelCabecalho { background: #eef4fa; border-bottom: 1px solid #c7d5e3; }"
        "QLabel#tituloCentro { font-size: 20px; font-weight: 600; color: #143a5a; }"
        "QLabel#subtituloCentro { color: #4d647a; }");
}

void MainWindow::configurarArvoreRede()
{
    m_dockRede = new QDockWidget("Explorador do Projeto", this);
    m_dockRede->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockRede->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    m_arvoreRede = new QTreeWidget(m_dockRede);
    m_arvoreRede->setColumnCount(1);
    m_arvoreRede->setHeaderLabel("Itens do projeto");
    m_arvoreRede->setAlternatingRowColors(true);
    m_arvoreRede->setUniformRowHeights(true);
    m_arvoreRede->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_arvoreRede, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem* atual, QTreeWidgetItem*) {
                aplicarSelecao(atual);
            });

    connect(m_arvoreRede, &QWidget::customContextMenuRequested, this,
            [this](const QPoint& posicao) {
                mostrarMenuContextoArvore(posicao);
            });

    m_dockRede->setWidget(m_arvoreRede);
    addDockWidget(Qt::LeftDockWidgetArea, m_dockRede);
}

void MainWindow::configurarPainelPropriedades()
{
    m_dockPropriedades = new QDockWidget("Propriedades", this);
    m_dockPropriedades->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockPropriedades->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    auto* conteudo = new QWidget(m_dockPropriedades);
    auto* layout = new QVBoxLayout(conteudo);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto* titulo = new QLabel("Atributos do elemento selecionado", conteudo);
    titulo->setStyleSheet("font-weight: 600; color: #143a5a;");

    m_tabelaPropriedades = new QTableWidget(conteudo);
    m_tabelaPropriedades->setColumnCount(2);
    m_tabelaPropriedades->setHorizontalHeaderLabels(QStringList() << "Propriedade" << "Valor");
    m_tabelaPropriedades->horizontalHeader()->setStretchLastSection(true);
    m_tabelaPropriedades->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tabelaPropriedades->verticalHeader()->setVisible(false);
    m_tabelaPropriedades->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabelaPropriedades->setSelectionMode(QAbstractItemView::NoSelection);
    m_tabelaPropriedades->setFocusPolicy(Qt::NoFocus);
    m_tabelaPropriedades->setAlternatingRowColors(true);

    layout->addWidget(titulo);
    layout->addWidget(m_tabelaPropriedades, 1);

    m_dockPropriedades->setWidget(conteudo);
    addDockWidget(Qt::RightDockWidgetArea, m_dockPropriedades);
}

void MainWindow::configurarCenaBase()
{
    m_cenaRede->clear();
    m_itensCena.clear();

    const QRectF extensaoCena(0.0, 0.0, 1180.0, 720.0);
    m_cenaRede->setSceneRect(extensaoCena);

    QPen gradePen(QColor("#d7e1ea"));
    gradePen.setWidthF(1.0);

    for (qreal x = extensaoCena.left(); x <= extensaoCena.right(); x += 80.0) {
        m_cenaRede->addLine(x, extensaoCena.top(), x, extensaoCena.bottom(), gradePen);
    }

    for (qreal y = extensaoCena.top(); y <= extensaoCena.bottom(); y += 80.0) {
        m_cenaRede->addLine(extensaoCena.left(), y, extensaoCena.right(), y, gradePen);
    }

    auto* areaTecnica = m_cenaRede->addRect(
        QRectF(80.0, 70.0, 980.0, 560.0),
        criarPenPadrao(QColor("#bfd0df"), 2.0),
        QBrush(QColor("#f9fcff")));
    areaTecnica->setZValue(-10.0);

    auto* tituloMapa = m_cenaRede->addSimpleText("Cena base da geometria");
    tituloMapa->setBrush(QColor("#163e63"));
    tituloMapa->setPos(104.0, 86.0);

    auto* faixaCanal = new QGraphicsPathItem();
    QPainterPath caminhoCanal(QPointF(240.0, 250.0));
    caminhoCanal.cubicTo(QPointF(360.0, 240.0), QPointF(470.0, 260.0), QPointF(570.0, 310.0));
    caminhoCanal.cubicTo(QPointF(650.0, 350.0), QPointF(750.0, 390.0), QPointF(920.0, 430.0));
    faixaCanal->setPath(caminhoCanal);
    faixaCanal->setPen(criarPenPadrao(QColor("#7fa7c9"), 26.0));
    faixaCanal->setZValue(-2.0);
    m_cenaRede->addItem(faixaCanal);

    auto* eixoCanal = new QGraphicsPathItem(caminhoCanal);
    eixoCanal->setPen(criarPenPadrao(QColor("#255d8d"), 5.0));
    eixoCanal->setZValue(-1.0);
    m_cenaRede->addItem(eixoCanal);

    auto adicionarRotulo = [&](const QString& texto, const QPointF& posicao) {
        auto* rotulo = m_cenaRede->addSimpleText(texto);
        rotulo->setBrush(QColor("#274966"));
        rotulo->setPos(posicao);
        return rotulo;
    };

    auto* baciaMontante = new QGraphicsPolygonItem(
        QPolygonF({QPointF(160.0, 140.0), QPointF(320.0, 120.0), QPointF(390.0, 220.0), QPointF(280.0, 300.0), QPointF(140.0, 240.0)}));
    baciaMontante->setBrush(QColor("#d9ead3"));
    baciaMontante->setPen(criarPenPadrao(QColor("#5c8c59"), 3.0));
    m_cenaRede->addItem(baciaMontante);
    m_itensCena.insert("bacia_montante", baciaMontante);
    adicionarRotulo("Bacia Montante", QPointF(190.0, 150.0));

    auto* baciaLeste = new QGraphicsPolygonItem(
        QPolygonF({QPointF(520.0, 170.0), QPointF(680.0, 150.0), QPointF(750.0, 250.0), QPointF(640.0, 320.0), QPointF(500.0, 250.0)}));
    baciaLeste->setBrush(QColor("#e6efcf"));
    baciaLeste->setPen(criarPenPadrao(QColor("#829c58"), 3.0));
    m_cenaRede->addItem(baciaLeste);
    m_itensCena.insert("bacia_leste", baciaLeste);
    adicionarRotulo("Bacia Leste", QPointF(560.0, 180.0));

    auto* canalPrincipal = new QGraphicsRectItem(QRectF(350.0, 275.0, 280.0, 72.0));
    canalPrincipal->setBrush(QColor("#f3c77a"));
    canalPrincipal->setPen(criarPenPadrao(QColor("#b66f15"), 3.0));
    m_cenaRede->addItem(canalPrincipal);
    m_itensCena.insert("canal_principal", canalPrincipal);
    adicionarRotulo("Canal Principal", QPointF(404.0, 293.0));

    auto* trechoJusante = new QGraphicsRectItem(QRectF(650.0, 345.0, 260.0, 68.0));
    trechoJusante->setBrush(QColor("#f6d59b"));
    trechoJusante->setPen(criarPenPadrao(QColor("#bd7f2d"), 3.0));
    m_cenaRede->addItem(trechoJusante);
    m_itensCena.insert("trecho_jusante", trechoJusante);
    adicionarRotulo("Trecho Jusante", QPointF(705.0, 362.0));

    auto* estruturaSaida = new QGraphicsEllipseItem(QRectF(920.0, 390.0, 72.0, 72.0));
    estruturaSaida->setBrush(QColor("#f2b4b4"));
    estruturaSaida->setPen(criarPenPadrao(QColor("#9b3434"), 3.0));
    m_cenaRede->addItem(estruturaSaida);
    m_itensCena.insert("estrutura_saida", estruturaSaida);
    adicionarRotulo("Estrutura de Saida", QPointF(860.0, 472.0));

    m_viewRede->fitInView(areaTecnica->sceneBoundingRect().adjusted(-30.0, -30.0, 30.0, 30.0), Qt::KeepAspectRatio);
}

void MainWindow::popularArvoreRede()
{
    m_arvoreRede->clear();
    m_elementos.clear();

    const QVector<ElementoApresentacao> elementos = {
        {
            "projeto_raiz",
            "Projeto Exemplo - Dimensionamento HDR",
            "Projeto",
            "Macroestrutura inspirada na planilha de dimensionamento, organizada por entradas, hidrologia, geometria, estruturas e resumos.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Projeto"},
                {"Planilha base", "DIMENSIONAMENTO (V2026.03.12).xlsm"},
                {"Estrutura", "Pluviometria, IDFs, hidrologia, geometria, estruturas e resumos"},
                {"Sistema de unidades", "SI"}
            }
        },
        {
            "pluviometria_idf",
            "Pluviometria e IDFs",
            "Macrocomponente",
            "Entradas pluviometricas, isozonas, regressoes e analise de curvas IDF.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Macrocomponente"},
                {"Abas base", "P1dia, isozonas, regressoes, ANALISE IDFS, IDFS"},
                {"Uso", "Definicao das chuvas de projeto"}
            }
        },
        {
            "chuvas_base",
            "Chuvas base",
            "Grupo de planilhas",
            "Fontes basicas de precipitacao e periodos de retorno.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo de planilhas"},
                {"Abas", "P1dia, Taborga_1974, Isozonas_2016 e correlatas"}
            }
        },
        {
            "aba_p1dia",
            "P1dia",
            "Planilha",
            "Tabela base de precipitacao versus periodo de retorno e probabilidade anual de excedencia.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Conteudo", "Periodo de retorno, AEP e precipitacao"},
                {"Funcao", "Entrada primaria de chuva"}
            }
        },
        {
            "aba_taborga_1974",
            "Taborga_1974",
            "Planilha",
            "Referencia historica de apoio para tratamento pluviometrico e regionalizacao.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Categoria", "Referencia pluviometrica historica"}
            }
        },
        {
            "aba_reg_iso_1974",
            "Reg_Isozonas_1974",
            "Planilha",
            "Regressoes associadas ao conjunto de isozonas de 1974.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Categoria", "Regressao de isozonas"}
            }
        },
        {
            "aba_isozonas_2016",
            "Isozonas_2016",
            "Planilha",
            "Base de isozonas atualizada para apoio a intensidade e precipitacao regional.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Categoria", "Isozonas"}
            }
        },
        {
            "aba_reg_iso_2016",
            "Reg_Isozonas_2016",
            "Planilha",
            "Regressoes associadas ao conjunto de isozonas de 2016.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Categoria", "Regressao de isozonas"}
            }
        },
        {
            "aba_regressao_1957",
            "Regressao_,_1957",
            "Planilha",
            "Regressao historica complementar utilizada como referencia na construcao das entradas.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Categoria", "Base historica"}
            }
        },
        {
            "analise_idfs",
            "Analise de IDFs",
            "Grupo de planilhas",
            "Planilhas voltadas para calibracao, analise e cadastro de curvas IDF.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo de planilhas"},
                {"Abas", "ANALISE IDFS e IDFS"}
            }
        },
        {
            "aba_analise_idfs",
            "ANALISE IDFS",
            "Planilha",
            "Comparacao entre metodologias, isozonas e parametros K, m, c e n das IDFs.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "Tempo de retorno, K, m, c, n, I(mm/h), Ptotal(mm)"},
                {"Funcao", "Analise comparativa de IDFs"}
            }
        },
        {
            "aba_idfs",
            "IDFS",
            "Planilha",
            "Cadastro consolidado das curvas IDF disponiveis para uso no dimensionamento.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "ID-IDF, Nome, TR, K, m, c, n"},
                {"Funcao", "Base de curvas para chuva de projeto"}
            }
        },
        {
            "hidrologia_perdas",
            "Hidrologia e perdas",
            "Macrocomponente",
            "Modelos de chuva efetiva, coeficientes equivalentes e perda de solo.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Macrocomponente"},
                {"Abas base", "SCS-CN, C_EQUIVALENTE, RUSLE, CURVAS"}
            }
        },
        {
            "aba_scs_cn_pefetiva",
            "SCS-CN-P_EFETIVA",
            "Planilha",
            "Calculo da precipitacao efetiva pelo metodo SCS-CN.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "Duracao, TR, Ptotal, Ia, CN, S, Pef"},
                {"Funcao", "Chuva efetiva"}
            }
        },
        {
            "aba_c_equivalente",
            "C_EQUIVALENTE",
            "Planilha",
            "Consolidacao de coeficientes de escoamento equivalentes.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Coeficientes equivalentes para metodo racional"}
            }
        },
        {
            "aba_scs_cn_c",
            "SCS-CN-C",
            "Planilha",
            "Parametros hidrologicos auxiliares e coeficientes complementares do metodo SCS-CN.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Parametros complementares de infiltracao"}
            }
        },
        {
            "aba_rusle",
            "RUSLE",
            "Planilha",
            "Estimativa de perda de solo associada as bacias.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Perda de solo e sedimentacao"}
            }
        },
        {
            "aba_curvas",
            "CURVAS",
            "Planilha",
            "Curvas e referencias auxiliares para os metodos hidrologicos.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Apoio grafico e parametrizacao"}
            }
        },
        {
            "geometria_topologia",
            "Geometria e topologia",
            "Macrocomponente",
            "Entrada e consolidacao dos elementos fisicos da rede, dados geometricos e topologia hidraulica.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Macrocomponente"},
                {"Abas base", "NOMENCLATURA, BACIAS, CANAIS, BUEIROS e topologias"},
                {"Visualizacao", "Explorador + cena central"}
            }
        },
        {
            "cadastros_geometria",
            "Cadastros e tabelas de geometria",
            "Grupo de planilhas",
            "Bases auxiliares usadas na montagem e consistencia da geometria.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo de planilhas"},
                {"Abas", "NOMENCLATURA, REVESTIMENTO, SECOES, EDRH_TIPOS"}
            }
        },
        {
            "aba_nomenclatura",
            "NOMENCLATURA",
            "Planilha",
            "Padroes de codificacao e nomenclatura dos elementos.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Padronizacao dos identificadores"}
            }
        },
        {
            "aba_revestimento",
            "REVESTIMENTO",
            "Planilha",
            "Biblioteca de revestimentos e parametros hidraulicos associados.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Materiais e rugosidade"}
            }
        },
        {
            "aba_secoes_preliminar",
            "SECOES_PRELIMINAR",
            "Planilha",
            "Definicao preliminar de secoes para canais e dispositivos.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Pre-dimensionamento geometrico"}
            }
        },
        {
            "aba_secoes_dados",
            "SECOES_DADOS",
            "Planilha",
            "Cadastro detalhado das secoes efetivamente utilizadas no projeto.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Base geometrica consolidada"}
            }
        },
        {
            "aba_edrh_tipos",
            "EDRH_TIPOS",
            "Planilha",
            "Tipos padronizados de elementos de drenagem/hidraulica.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Tipologias de EDRH"}
            }
        },
        {
            "topologia_rede",
            "Topologia da rede",
            "Grupo de planilhas",
            "Relacionamentos montante-jusante e dados principais das bacias e dispositivos.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo de planilhas"},
                {"Abas", "BACIAS-TOPOLOGIA E DADOS, DISPOSITIVOS-TOPOLOGIA E DADOS"}
            }
        },
        {
            "aba_bacias_topologia",
            "BACIAS-TOPOLOGIA E DADOS",
            "Planilha",
            "Topologia das bacias com dados de area, talvegue, declividade e vazao.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "ID-SUBBACIA, ID-JUSANTE, A, L_talvegue, S, n_Manning"},
                {"Funcao", "Base hidrologica das bacias"}
            }
        },
        {
            "aba_dispositivos_topologia",
            "DISPOSITIVOS-TOPOLOGIA E DADOS",
            "Planilha",
            "Topologia dos dispositivos, conexoes e ordens hidrologicas.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "ID-MONTANTE, ID-JUSANTE, IDS-MONTANTE, ORDEM_HIDROLOGICA"},
                {"Funcao", "Base de conexao da rede"}
            }
        },
        {
            "rede_drenagem",
            "Rede de drenagem",
            "Grupo visual",
            "Representacao da rede de drenagem na interface grafica, derivada da macroestrutura da planilha.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo visual"},
                {"Conteudo", "Bacias, canais, bueiros e estruturas"}
            }
        },
        {
            "grupo_bacias",
            "Bacias de contribuicao",
            "Grupo geometrico",
            "Subgrupo da geometria dedicado as areas de contribuicao hidrologica.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo geometrico"},
                {"Planilha origem", "BACIAS e BACIAS-TOPOLOGIA E DADOS"},
                {"Representacao", "Poligonos na cena"}
            }
        },
        {
            "bacia_montante",
            "Bacia Montante",
            "Bacia de contribuicao",
            "Area de drenagem principal que alimenta o canal trapezoidal em estudo.",
            4.0,
            2.0,
            1.0,
            {
                {"Tipo", "Bacia de contribuicao"},
                {"Planilha origem", "BACIAS"},
                {"Area", "2,84 km2"},
                {"Tempo de concentracao", "34 min"},
                {"Elemento jusante", "Canal Principal"}
            }
        },
        {
            "bacia_leste",
            "Bacia Leste",
            "Sub-bacia",
            "Contribuicao lateral conectada ao trecho intermediario da rede.",
            3.2,
            1.8,
            1.2,
            {
                {"Tipo", "Sub-bacia"},
                {"Planilha origem", "BACIAS"},
                {"Area", "1,37 km2"},
                {"Declividade media", "4,1 %"},
                {"Elemento jusante", "Juncao J-02"}
            }
        },
        {
            "grupo_canais",
            "Canais e trechos",
            "Grupo geometrico",
            "Subgrupo com os condutos e trechos abertos principais da rede.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo geometrico"},
                {"Planilha origem", "CANAIS"},
                {"Representacao", "Eixo e blocos na cena"}
            }
        },
        {
            "canal_principal",
            "Canal Principal",
            "Canal trapezoidal",
            "Trecho principal representado na cena grafica para futuras edicoes.",
            4.0,
            2.0,
            1.0,
            {
                {"Tipo", "Canal trapezoidal"},
                {"Planilha origem", "CANAIS"},
                {"Comprimento", "185 m"},
                {"Largura de fundo", "4,00 m"},
                {"Altura de projeto", "2,00 m"},
                {"Declividade", "0,0010 m/m"}
            }
        },
        {
            "trecho_jusante",
            "Trecho Jusante",
            "Trecho de canal",
            "Continuacao do canal apos a juncao principal, conduzindo a vazao acumulada.",
            5.5,
            2.4,
            1.4,
            {
                {"Tipo", "Trecho de canal"},
                {"Planilha origem", "CANAIS"},
                {"Comprimento", "240 m"},
                {"Largura de fundo", "5,50 m"},
                {"Altura de projeto", "2,40 m"}
            }
        },
        {
            "grupo_bueiros",
            "Bueiros",
            "Grupo geometrico",
            "Subgrupo com os dispositivos tubulares associados a planilha de bueiros.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo geometrico"},
                {"Planilha origem", "BUEIROS"},
                {"Representacao", "Dispositivos lineares e especiais"}
            }
        },
        {
            "grupo_estruturas",
            "Estruturas especiais",
            "Grupo geometrico",
            "Subgrupo de dispositivos especiais conectados a rede principal.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo geometrico"},
                {"Planilhas origem", "DISPOSITIVOS-TOPOLOGIA E DADOS e estruturas"},
                {"Representacao", "Elemento especial na descarga"}
            }
        },
        {
            "estrutura_saida",
            "Estrutura de Saida",
            "Dissipador / descarga",
            "Estrutura terminal para controle da energia especifica na descarga da rede.",
            2.8,
            1.5,
            0.8,
            {
                {"Tipo", "Estrutura de controle"},
                {"Planilha origem", "DISS_RESSALTO / DISPOSITIVOS-TOPOLOGIA E DADOS"},
                {"Cota de soleira", "102,35 m"},
                {"Largura util", "2,80 m"},
                {"Condicao", "Descarga livre"}
            }
        },
        {
            "estruturas_hidraulicas",
            "Estruturas hidraulicas",
            "Macrocomponente",
            "Dimensionamento de bueiros, dissipadores, reservatorios e dispositivos especiais.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Macrocomponente"},
                {"Abas base", "BUEIROS, DISSIPADORES, RESERVATORIOS e auxiliares"}
            }
        },
        {
            "aba_bueiros",
            "BUEIROS",
            "Planilha",
            "Dimensionamento e conferencia dos bueiros a partir da topologia e da vazao total.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "ID-BUEIRO, Qp_total, A_total, L_talvegue_total, Cmed, Tc"},
                {"Funcao", "Dimensionamento de bueiros"}
            }
        },
        {
            "aba_tubulares",
            "TUBULAR_ADIMENSIONAIS",
            "Planilha",
            "Tabelas auxiliares adimensionais para dispositivos tubulares.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Consulta de parametros tubulares"}
            }
        },
        {
            "aba_espacamento",
            "ESPACAMENTO_DISPOSITIVOS",
            "Planilha",
            "Apoio a distribuicao e espacamento de dispositivos na rede.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Regras de espacamento"}
            }
        },
        {
            "aba_res_det",
            "RES_DET.",
            "Planilha",
            "Dimensionamento de reservatorios de detencao.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Reservatorio de detencao"}
            }
        },
        {
            "aba_res_sed",
            "RES_SED.",
            "Planilha",
            "Dimensionamento de reservatorios de retencao/sedimentacao.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Retencao e sedimentacao"}
            }
        },
        {
            "aba_diss_ressalto",
            "DISS_RESSALTO",
            "Planilha",
            "Dimensionamento de dissipadores por ressalto hidraulico.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Dissipacao por ressalto"}
            }
        },
        {
            "aba_diss_tapete",
            "DISS_TAPETE_ENROCAMENTO",
            "Planilha",
            "Dimensionamento de dissipadores com tapete de enrocamento.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Dissipacao com enrocamento"}
            }
        },
        {
            "aba_diss_bacia",
            "DISS_BACIA_ENROCAMENTO",
            "Planilha",
            "Dimensionamento de bacia dissipadora com enrocamento.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Bacia dissipadora"}
            }
        },
        {
            "resumos_relatorios",
            "Resumos e relatorios",
            "Macrocomponente",
            "Consolidacao dos resultados globais e resumos por tipo de elemento.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Macrocomponente"},
                {"Abas base", "RESUMO_GLOBAL e resumos especificos"}
            }
        },
        {
            "aba_resumo_global",
            "RESUMO_GLOBAL",
            "Planilha",
            "Quadro consolidado dos elementos com vazoes, areas e ordem hidrologica.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "ID, Tipo, Qp, Atotal, Tc_Atotal, ID-JUSANTE"},
                {"Funcao", "Painel global de resultados"}
            }
        },
        {
            "aba_bacias_resumo",
            "BACIAS_RESUMO",
            "Planilha",
            "Resumo sintetico das bacias para conferencia rapida.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Resumo de bacias"}
            }
        },
        {
            "aba_rusle_resumo",
            "RUSLE_RESUMO",
            "Planilha",
            "Resumo da perda de solo estimada.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Resumo de RUSLE"}
            }
        },
        {
            "aba_canais_resumo",
            "CANAIS_RESUMO",
            "Planilha",
            "Resumo dos canais com geometria, vazao e folga hidraulica.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "Geometry, Qp, Hmax water, Freeboard"},
                {"Funcao", "Resumo de canais"}
            }
        },
        {
            "aba_bueiros_resumo",
            "BUEIROS_RESUMO",
            "Planilha",
            "Resumo dos bueiros com vazoes parciais e totais por TR.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Resumo de bueiros"}
            }
        },
        {
            "aba_diss_ressalto_resumo",
            "DISS_RESSALTO_RESUMO",
            "Planilha",
            "Resumo dos dissipadores por ressalto.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Funcao", "Resumo de dissipadores"}
            }
        },
        {
            "auxiliares",
            "Tabelas auxiliares",
            "Macrocomponente",
            "Bibliotecas complementares e tabelas de consulta geral do dimensionamento.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Macrocomponente"},
                {"Aba base", "TABELAS_AUXILIARES_DIVERSAS"}
            }
        },
        {
            "aba_tabelas_aux",
            "TABELAS_AUXILIARES_DIVERSAS",
            "Planilha",
            "Biblioteca de combinacoes, pecas e parametros auxiliares para dispositivos.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Planilha"},
                {"Campos-chave", "No bueiros, D, dispositivo, combinacoes usuais"},
                {"Funcao", "Consulta auxiliar de dimensionamento"}
            }
        }
    };

    for (const ElementoApresentacao& elemento : elementos) {
        m_elementos.insert(elemento.id, elemento);
    }

    const QIcon iconeProjeto = style()->standardIcon(QStyle::SP_DirHomeIcon);
    const QIcon iconePastaAberta = style()->standardIcon(QStyle::SP_DirOpenIcon);
    const QIcon iconeArquivo = style()->standardIcon(QStyle::SP_FileIcon);
    const QIcon iconeSalvar = style()->standardIcon(QStyle::SP_DialogSaveButton);

    auto criarItem = [&](QTreeWidgetItem* pai, const QString& chave, const QIcon& icone) {
        const ElementoApresentacao elemento = m_elementos.value(chave);
        auto* item = new QTreeWidgetItem(QStringList() << elemento.nome);
        item->setData(0, kRoleElementoId, chave);
        item->setToolTip(0, elemento.tipo);
        item->setIcon(0, icone);
        pai->addChild(item);
        return item;
    };

    auto* projetoItem = new QTreeWidgetItem(QStringList() << m_elementos.value("projeto_raiz").nome);
    projetoItem->setData(0, kRoleElementoId, "projeto_raiz");
    projetoItem->setToolTip(0, m_elementos.value("projeto_raiz").tipo);
    projetoItem->setIcon(0, iconeProjeto);
    m_arvoreRede->addTopLevelItem(projetoItem);

    auto* pluviometriaItem = criarItem(projetoItem, "pluviometria_idf", iconePastaAberta);
    auto* chuvasBaseItem = criarItem(pluviometriaItem, "chuvas_base", iconePastaAberta);
    criarItem(chuvasBaseItem, "aba_p1dia", iconeArquivo);
    criarItem(chuvasBaseItem, "aba_taborga_1974", iconeArquivo);
    criarItem(chuvasBaseItem, "aba_reg_iso_1974", iconeArquivo);
    criarItem(chuvasBaseItem, "aba_isozonas_2016", iconeArquivo);
    criarItem(chuvasBaseItem, "aba_reg_iso_2016", iconeArquivo);
    criarItem(chuvasBaseItem, "aba_regressao_1957", iconeArquivo);

    auto* analiseIdfItem = criarItem(pluviometriaItem, "analise_idfs", iconePastaAberta);
    criarItem(analiseIdfItem, "aba_analise_idfs", iconeArquivo);
    criarItem(analiseIdfItem, "aba_idfs", iconeArquivo);

    auto* hidrologiaItem = criarItem(projetoItem, "hidrologia_perdas", iconePastaAberta);
    criarItem(hidrologiaItem, "aba_scs_cn_pefetiva", iconeArquivo);
    criarItem(hidrologiaItem, "aba_c_equivalente", iconeArquivo);
    criarItem(hidrologiaItem, "aba_scs_cn_c", iconeArquivo);
    criarItem(hidrologiaItem, "aba_rusle", iconeArquivo);
    criarItem(hidrologiaItem, "aba_curvas", iconeArquivo);

    auto* geometriaItem = criarItem(projetoItem, "geometria_topologia", iconePastaAberta);
    auto* cadastrosGeometriaItem = criarItem(geometriaItem, "cadastros_geometria", iconePastaAberta);
    criarItem(cadastrosGeometriaItem, "aba_nomenclatura", iconeArquivo);
    criarItem(cadastrosGeometriaItem, "aba_revestimento", iconeArquivo);
    criarItem(cadastrosGeometriaItem, "aba_secoes_preliminar", iconeArquivo);
    criarItem(cadastrosGeometriaItem, "aba_secoes_dados", iconeArquivo);
    criarItem(cadastrosGeometriaItem, "aba_edrh_tipos", iconeArquivo);

    auto* topologiaRedeItem = criarItem(geometriaItem, "topologia_rede", iconePastaAberta);
    criarItem(topologiaRedeItem, "aba_bacias_topologia", iconeArquivo);
    criarItem(topologiaRedeItem, "aba_dispositivos_topologia", iconeArquivo);

    auto* redeDrenagemItem = criarItem(geometriaItem, "rede_drenagem", iconePastaAberta);
    auto* grupoBacias = criarItem(redeDrenagemItem, "grupo_bacias", iconePastaAberta);
    criarItem(grupoBacias, "bacia_montante", iconeArquivo);
    criarItem(grupoBacias, "bacia_leste", iconeArquivo);

    auto* grupoCanais = criarItem(redeDrenagemItem, "grupo_canais", iconePastaAberta);
    criarItem(grupoCanais, "canal_principal", iconeArquivo);
    criarItem(grupoCanais, "trecho_jusante", iconeArquivo);

    auto* grupoBueiros = criarItem(redeDrenagemItem, "grupo_bueiros", iconePastaAberta);
    auto* grupoEstruturas = criarItem(redeDrenagemItem, "grupo_estruturas", iconePastaAberta);
    criarItem(grupoEstruturas, "estrutura_saida", iconeArquivo);

    auto* estruturasItem = criarItem(projetoItem, "estruturas_hidraulicas", iconePastaAberta);
    criarItem(estruturasItem, "aba_bueiros", iconeArquivo);
    criarItem(estruturasItem, "aba_tubulares", iconeArquivo);
    criarItem(estruturasItem, "aba_espacamento", iconeArquivo);
    criarItem(estruturasItem, "aba_res_det", iconeArquivo);
    criarItem(estruturasItem, "aba_res_sed", iconeArquivo);
    criarItem(estruturasItem, "aba_diss_ressalto", iconeArquivo);
    criarItem(estruturasItem, "aba_diss_tapete", iconeArquivo);
    criarItem(estruturasItem, "aba_diss_bacia", iconeArquivo);

    auto* resumosItem = criarItem(projetoItem, "resumos_relatorios", iconePastaAberta);
    criarItem(resumosItem, "aba_resumo_global", iconeSalvar);
    criarItem(resumosItem, "aba_bacias_resumo", iconeSalvar);
    criarItem(resumosItem, "aba_rusle_resumo", iconeSalvar);
    criarItem(resumosItem, "aba_canais_resumo", iconeSalvar);
    criarItem(resumosItem, "aba_bueiros_resumo", iconeSalvar);
    criarItem(resumosItem, "aba_diss_ressalto_resumo", iconeSalvar);

    auto* auxiliaresItem = criarItem(projetoItem, "auxiliares", iconePastaAberta);
    criarItem(auxiliaresItem, "aba_tabelas_aux", iconeArquivo);

    projetoItem->setExpanded(true);
    pluviometriaItem->setExpanded(true);
    chuvasBaseItem->setExpanded(true);
    analiseIdfItem->setExpanded(true);
    hidrologiaItem->setExpanded(true);
    geometriaItem->setExpanded(true);
    cadastrosGeometriaItem->setExpanded(true);
    topologiaRedeItem->setExpanded(true);
    redeDrenagemItem->setExpanded(true);
    grupoBacias->setExpanded(true);
    grupoCanais->setExpanded(true);
    grupoBueiros->setExpanded(true);
    grupoEstruturas->setExpanded(true);
    estruturasItem->setExpanded(true);
    resumosItem->setExpanded(true);
    auxiliaresItem->setExpanded(true);
}

void MainWindow::selecionarPrimeiroElemento()
{
    if (!m_arvoreRede) {
        return;
    }

    QTreeWidgetItem* projetoItem = m_arvoreRede->topLevelItem(0);
    if (!projetoItem || projetoItem->childCount() == 0) {
        return;
    }

    QTreeWidgetItem* estudoItem = projetoItem->child(0);
    if (!estudoItem || estudoItem->childCount() == 0) {
        return;
    }

    QTreeWidgetItem* macroItem = estudoItem->child(0);
    if (!macroItem || macroItem->childCount() == 0) {
        return;
    }

    m_arvoreRede->setCurrentItem(macroItem->child(0));
}

void MainWindow::aplicarSelecao(QTreeWidgetItem* item)
{
    const QString chave = chaveElemento(item);
    if (chave.isEmpty() || !m_elementos.contains(chave)) {
        return;
    }

    const ElementoApresentacao elemento = m_elementos.value(chave);

    m_tituloCentro->setText(QString("%1  |  %2").arg(elemento.nome, elemento.tipo));
    m_subtituloCentro->setText(elemento.resumo);

    atualizarPainelPropriedades(elemento);
    destacarElementoCena(chave);
}

void MainWindow::atualizarPainelPropriedades(const ElementoApresentacao& elemento)
{
    m_tabelaPropriedades->setRowCount(elemento.propriedades.size());

    for (int i = 0; i < elemento.propriedades.size(); ++i) {
        const PropriedadeElemento& propriedade = elemento.propriedades.at(i);
        auto* itemNome = new QTableWidgetItem(propriedade.nome);
        auto* itemValor = new QTableWidgetItem(propriedade.valor);
        itemNome->setFlags(itemNome->flags() & ~Qt::ItemIsEditable);
        itemValor->setFlags(itemValor->flags() & ~Qt::ItemIsEditable);
        m_tabelaPropriedades->setItem(i, 0, itemNome);
        m_tabelaPropriedades->setItem(i, 1, itemValor);
    }

    m_tabelaPropriedades->resizeRowsToContents();
}

void MainWindow::destacarElementoCena(const QString& chaveElemento)
{
    const QColor corDestaqueContorno("#0f6cbd");
    const QColor corDestaquePreenchimento("#fde8a6");

    for (auto it = m_itensCena.begin(); it != m_itensCena.end(); ++it) {
        if (auto* item = dynamic_cast<QAbstractGraphicsShapeItem*>(it.value())) {
            QBrush brush = item->brush();
            QPen pen = item->pen();

            if (it.key() == "bacia_montante") {
                brush.setColor(QColor("#d9ead3"));
                pen = criarPenPadrao(QColor("#5c8c59"), 3.0);
            } else if (it.key() == "bacia_leste") {
                brush.setColor(QColor("#e6efcf"));
                pen = criarPenPadrao(QColor("#829c58"), 3.0);
            } else if (it.key() == "canal_principal") {
                brush.setColor(QColor("#f3c77a"));
                pen = criarPenPadrao(QColor("#b66f15"), 3.0);
            } else if (it.key() == "trecho_jusante") {
                brush.setColor(QColor("#f6d59b"));
                pen = criarPenPadrao(QColor("#bd7f2d"), 3.0);
            } else if (it.key() == "estrutura_saida") {
                brush.setColor(QColor("#f2b4b4"));
                pen = criarPenPadrao(QColor("#9b3434"), 3.0);
            }

            item->setBrush(brush);
            item->setPen(pen);
            item->setZValue(1.0);
        }
    }

    QGraphicsItem* itemSelecionado = m_itensCena.value(chaveElemento, nullptr);
    if (auto* shape = dynamic_cast<QAbstractGraphicsShapeItem*>(itemSelecionado)) {
        QBrush brush = shape->brush();
        brush.setColor(corDestaquePreenchimento);
        shape->setBrush(brush);
        shape->setPen(criarPenPadrao(corDestaqueContorno, 4.0));
        shape->setZValue(5.0);
        m_viewRede->fitInView(shape->sceneBoundingRect().adjusted(-150.0, -110.0, 150.0, 110.0), Qt::KeepAspectRatio);
    } else {
        m_viewRede->fitInView(m_cenaRede->sceneRect().adjusted(60.0, 40.0, -60.0, -40.0), Qt::KeepAspectRatio);
    }
}

void MainWindow::mostrarMenuContextoArvore(const QPoint& posicao)
{
    QTreeWidgetItem* itemSelecionado = m_arvoreRede->itemAt(posicao);
    if (!itemSelecionado) {
        itemSelecionado = m_arvoreRede->currentItem();
    }
    if (!itemSelecionado) {
        return;
    }

    m_arvoreRede->setCurrentItem(itemSelecionado);
    const bool podeReceberFilhos = (itemSelecionado->childCount() > 0) || (itemSelecionado == m_arvoreRede->topLevelItem(0));

    QMenu menuContexto(this);
    auto* acaoNovoItem = menuContexto.addAction(style()->standardIcon(QStyle::SP_FileIcon), "Novo Item...");
    auto* acaoNovoGrupo = menuContexto.addAction(style()->standardIcon(QStyle::SP_DirClosedIcon), "Adicionar Novo Grupo");
    menuContexto.addSeparator();
    auto* acaoOrdenarAsc = menuContexto.addAction("Ordenar Crescente");
    auto* acaoOrdenarDesc = menuContexto.addAction("Ordenar Decrescente");

    acaoNovoItem->setEnabled(podeReceberFilhos);
    acaoNovoGrupo->setEnabled(podeReceberFilhos);

    QAction* acaoSelecionada = menuContexto.exec(m_arvoreRede->viewport()->mapToGlobal(posicao));
    if (!acaoSelecionada) {
        return;
    }

    if (acaoSelecionada == acaoNovoItem) {
        QTreeWidgetItem* novoItem = adicionarElementoArvore(
            itemSelecionado,
            "item_usuario",
            "Novo item",
            "Item do projeto",
            "Item criado a partir do menu de contexto do explorador.",
            {
                {"Tipo", "Item do projeto"},
                {"Origem", "Criado manualmente"},
                {"Pai", itemSelecionado->text(0)}
            });
        itemSelecionado->setExpanded(true);
        m_arvoreRede->setCurrentItem(novoItem);
        statusBar()->showMessage("Novo item adicionado ao explorador.", 4000);
    } else if (acaoSelecionada == acaoNovoGrupo) {
        QTreeWidgetItem* novoGrupo = adicionarElementoArvore(
            itemSelecionado,
            "grupo_usuario",
            "Novo grupo",
            "Grupo do projeto",
            "Grupo criado para organizar novos subitens do projeto.",
            {
                {"Tipo", "Grupo do projeto"},
                {"Origem", "Criado manualmente"},
                {"Pai", itemSelecionado->text(0)}
            },
            true);
        itemSelecionado->setExpanded(true);
        m_arvoreRede->setCurrentItem(novoGrupo);
        statusBar()->showMessage("Novo grupo adicionado ao explorador.", 4000);
    } else if (acaoSelecionada == acaoOrdenarAsc) {
        itemSelecionado->sortChildren(0, Qt::AscendingOrder);
        statusBar()->showMessage("Itens ordenados em ordem crescente.", 3000);
    } else if (acaoSelecionada == acaoOrdenarDesc) {
        itemSelecionado->sortChildren(0, Qt::DescendingOrder);
        statusBar()->showMessage("Itens ordenados em ordem decrescente.", 3000);
    }
}

QString MainWindow::gerarIdElemento(const QString& prefixo) const
{
    int indice = m_elementos.size() + 1;
    QString id;
    do {
        id = QString("%1_%2").arg(prefixo).arg(indice++);
    } while (m_elementos.contains(id));
    return id;
}

QTreeWidgetItem* MainWindow::adicionarElementoArvore(QTreeWidgetItem* pai,
                                                     const QString& prefixo,
                                                     const QString& nome,
                                                     const QString& tipo,
                                                     const QString& resumo,
                                                     const QVector<PropriedadeElemento>& propriedades,
                                                     bool comoGrupo)
{
    const QString id = gerarIdElemento(prefixo);

    ElementoApresentacao elemento;
    elemento.id = id;
    elemento.nome = QString("%1 %2").arg(nome).arg(m_elementos.size());
    elemento.tipo = tipo;
    elemento.resumo = resumo;
    elemento.propriedades = propriedades;
    m_elementos.insert(id, elemento);

    auto* item = new QTreeWidgetItem(QStringList() << elemento.nome);
    item->setData(0, kRoleElementoId, id);
    item->setToolTip(0, tipo);
    item->setIcon(0, style()->standardIcon(comoGrupo ? QStyle::SP_DirClosedIcon : QStyle::SP_FileIcon));
    pai->addChild(item);
    return item;
}

QString MainWindow::chaveElemento(QTreeWidgetItem* item) const
{
    if (!item) {
        return {};
    }

    return item->data(0, kRoleElementoId).toString();
}



