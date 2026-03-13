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

    adicionarAcaoComponente("Gerenciador de Dados Pluviometricos");
    adicionarAcaoComponente("Gerenciador de IDF");
    adicionarAcaoComponente("Gerenciador de Geometria");
    adicionarAcaoComponente("Gerenciador de Cenarios");
    adicionarAcaoComponente("Gerenciador de Estudos Hidrologicos");

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
            "Projeto Exemplo - Rede urbana",
            "Projeto",
            "Visao geral do projeto, organizada por estudos, dados, geometria e cenarios.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Projeto"},
                {"Estrutura", "Hidrologia, Geometria, Cenarios"},
                {"Sistema de unidades", "SI"},
                {"Status", "Interface base pronta para integracao"}
            }
        },
        {
            "hidrologia",
            "Hidrologia",
            "Grupo de estudo",
            "Agrupa os dados e modelos hidrologicos do projeto.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo de estudo"},
                {"Subgrupos", "Dados pluviometricos, IDF"},
                {"Uso", "Entrada para chuva-vazao"}
            }
        },
        {
            "dados_pluviometricos",
            "Dados pluviometricos",
            "Macroitem",
            "Agrupa series observadas, tormentas sinteticas e fontes de precipitacao.",
            0.0,
            0.0,
            0.0,
            {
                {"Categoria", "Dados pluviometricos"},
                {"Subitens", "Series observadas, tormentas de projeto"},
                {"Origem", "ANA / estudos locais"}
            }
        },
        {
            "serie_ana_3252005",
            "Serie ANA 3252005",
            "Serie pluviometrica",
            "Serie historica principal utilizada para consistencia e analise de eventos observados.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Serie pluviometrica"},
                {"Estacao", "3252005"},
                {"Periodo", "2002-2024"},
                {"Resolucao", "Diaria"},
                {"Fonte", "Hidroweb ANA"}
            }
        },
        {
            "chuva_projeto_10a",
            "Chuva de projeto 10 anos",
            "Evento sintetico",
            "Evento de referencia para verificacoes preliminares de microdrenagem.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Evento sintetico"},
                {"TR", "10 anos"},
                {"Duracao critica", "30 min"},
                {"Metodo", "Chicago"}
            }
        },
        {
            "idf",
            "IDF",
            "Macroitem",
            "Agrupa curvas intensidade-duracao-frequencia disponiveis para o projeto.",
            0.0,
            0.0,
            0.0,
            {
                {"Categoria", "IDF"},
                {"Subitens", "Curvas locais e ajustadas"},
                {"Uso", "Geracao de chuvas de projeto"}
            }
        },
        {
            "idf_fortaleza",
            "IDF Fortaleza",
            "Curva IDF",
            "Curva base para duracoes curtas aplicada aos cenarios urbanos do projeto.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Curva IDF"},
                {"Localidade", "Fortaleza"},
                {"Faixa de duracao", "5 a 1440 min"},
                {"Fonte", "Bibliografia local"}
            }
        },
        {
            "idf_regional",
            "IDF Regional Ajustada",
            "Curva IDF",
            "Alternativa regional para comparacao de sensibilidade dos resultados.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Curva IDF"},
                {"Abrangencia", "Regional"},
                {"Ajuste", "Gumbel"},
                {"Observacao", "Usar para comparacao"}
            }
        },
        {
            "geometria",
            "Geometria",
            "Macroitem",
            "Concentra a rede de drenagem e os elementos fisicos representados na cena central.",
            0.0,
            0.0,
            0.0,
            {
                {"Categoria", "Geometria"},
                {"Subgrupos", "Bacias, canais, estruturas"},
                {"Visualizacao", "QGraphicsScene"}
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
                {"Elementos", "2"},
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
                {"Area", "2,84 km2"},
                {"CN equivalente", "78"},
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
                {"Area", "1,37 km2"},
                {"Declividade media", "4,1 %"},
                {"Tempo de concentracao", "22 min"},
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
                {"Elementos", "2"},
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
                {"Comprimento", "185 m"},
                {"Largura de fundo", "4,00 m"},
                {"Altura de projeto", "2,00 m"},
                {"Talude lateral", "1,0 H:1,0 V"},
                {"Revestimento", "Concreto"},
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
                {"Comprimento", "240 m"},
                {"Largura de fundo", "5,50 m"},
                {"Altura de projeto", "2,40 m"},
                {"Talude lateral", "1,4 H:1,0 V"},
                {"Revestimento", "Solo compactado"},
                {"Declividade", "0,0008 m/m"}
            }
        },
        {
            "grupo_estruturas",
            "Estruturas de controle",
            "Grupo geometrico",
            "Subgrupo de dispositivos especiais conectados a rede principal.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Grupo geometrico"},
                {"Elementos", "1"},
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
                {"Cota de soleira", "102,35 m"},
                {"Largura util", "2,80 m"},
                {"Altura", "1,50 m"},
                {"Condicao", "Descarga livre"},
                {"Observacao", "Verificar dissipacao a jusante"}
            }
        },
        {
            "cenarios",
            "Cenarios de projeto",
            "Macroitem",
            "Area reservada para agrupar alternativas de simulacao e comparacao de premissas.",
            0.0,
            0.0,
            0.0,
            {
                {"Categoria", "Cenarios"},
                {"Subitens", "Preliminar, executivo"},
                {"Status", "Estrutura inicial"}
            }
        },
        {
            "cenario_preliminar",
            "Cenario preliminar",
            "Cenario",
            "Configuracao base usada para organizar os dados atuais da interface.",
            0.0,
            0.0,
            0.0,
            {
                {"Tipo", "Cenario"},
                {"Descricao", "Base de trabalho inicial"},
                {"Rede ativa", "Geometria principal"}
            }
        }
    };

    for (const ElementoApresentacao& elemento : elementos) {
        m_elementos.insert(elemento.id, elemento);
    }

    const QIcon iconeProjeto = style()->standardIcon(QStyle::SP_DirHomeIcon);
    const QIcon iconePasta = style()->standardIcon(QStyle::SP_DirClosedIcon);
    const QIcon iconePastaAberta = style()->standardIcon(QStyle::SP_DirOpenIcon);
    const QIcon iconeArquivo = style()->standardIcon(QStyle::SP_FileIcon);
    const QIcon iconeSalvar = style()->standardIcon(QStyle::SP_DialogSaveButton);

    auto definirIconePasta = [&](QTreeWidgetItem* item) {
        item->setIcon(0, iconePasta);
        item->setData(0, Qt::DecorationRole, iconePasta);
    };

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

    auto* hidrologiaItem = criarItem(projetoItem, "hidrologia", iconePastaAberta);
    auto* dadosItem = criarItem(hidrologiaItem, "dados_pluviometricos", iconePastaAberta);
    criarItem(dadosItem, "serie_ana_3252005", iconeArquivo);
    criarItem(dadosItem, "chuva_projeto_10a", iconeArquivo);

    auto* idfItem = criarItem(hidrologiaItem, "idf", iconePastaAberta);
    criarItem(idfItem, "idf_fortaleza", iconeArquivo);
    criarItem(idfItem, "idf_regional", iconeArquivo);

    auto* geometriaItem = criarItem(projetoItem, "geometria", iconePastaAberta);
    auto* grupoBacias = criarItem(geometriaItem, "grupo_bacias", iconePastaAberta);
    criarItem(grupoBacias, "bacia_montante", iconeArquivo);
    criarItem(grupoBacias, "bacia_leste", iconeArquivo);

    auto* grupoCanais = criarItem(geometriaItem, "grupo_canais", iconePastaAberta);
    criarItem(grupoCanais, "canal_principal", iconeArquivo);
    criarItem(grupoCanais, "trecho_jusante", iconeArquivo);

    auto* grupoEstruturas = criarItem(geometriaItem, "grupo_estruturas", iconePastaAberta);
    criarItem(grupoEstruturas, "estrutura_saida", iconeArquivo);

    auto* cenariosItem = criarItem(projetoItem, "cenarios", iconePastaAberta);
    criarItem(cenariosItem, "cenario_preliminar", iconeSalvar);

    projetoItem->setExpanded(true);
    hidrologiaItem->setExpanded(true);
    dadosItem->setExpanded(true);
    idfItem->setExpanded(true);
    geometriaItem->setExpanded(true);
    grupoBacias->setExpanded(true);
    grupoCanais->setExpanded(true);
    grupoEstruturas->setExpanded(true);
    cenariosItem->setExpanded(true);
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

