#include "MainWindow.h"

#include <QAbstractGraphicsShapeItem>
#include <QAbstractItemView>
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
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTableWidget>
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
    popularArvoreRede();
    configurarCenaBase();
    selecionarPrimeiroElemento();
}

void MainWindow::configurarJanela()
{
    setWindowTitle("SISTEMAHDR - Explorador de Rede Hidrologica");
    resize(1520, 860);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);
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
    m_dockRede = new QDockWidget("Explorador da Rede", this);
    m_dockRede->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockRede->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    m_arvoreRede = new QTreeWidget(m_dockRede);
    m_arvoreRede->setColumnCount(1);
    m_arvoreRede->setHeaderLabel("Elementos");
    m_arvoreRede->setAlternatingRowColors(true);
    m_arvoreRede->setUniformRowHeights(true);

    connect(m_arvoreRede, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem* atual, QTreeWidgetItem*) {
                aplicarSelecao(atual);
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

    auto* tituloMapa = m_cenaRede->addSimpleText("Cena base da rede hidrologica");
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

    auto* projetoItem = new QTreeWidgetItem(QStringList() << "Projeto Exemplo - Rede urbana");
    projetoItem->setFlags(projetoItem->flags() & ~Qt::ItemIsSelectable);
    m_arvoreRede->addTopLevelItem(projetoItem);

    auto* grupoBacias = new QTreeWidgetItem(QStringList() << "Bacias de contribuicao");
    auto* grupoCanais = new QTreeWidgetItem(QStringList() << "Canais e trechos");
    auto* grupoEstruturas = new QTreeWidgetItem(QStringList() << "Estruturas de controle");

    projetoItem->addChild(grupoBacias);
    projetoItem->addChild(grupoCanais);
    projetoItem->addChild(grupoEstruturas);

    const QVector<ElementoApresentacao> elementos = {
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
        }
    };

    for (const ElementoApresentacao& elemento : elementos) {
        m_elementos.insert(elemento.id, elemento);
    }

    auto criarItemElemento = [&](QTreeWidgetItem* pai, const QString& chave) {
        const ElementoApresentacao elemento = m_elementos.value(chave);
        auto* item = new QTreeWidgetItem(QStringList() << elemento.nome);
        item->setData(0, kRoleElementoId, chave);
        item->setToolTip(0, elemento.tipo);
        pai->addChild(item);
    };

    criarItemElemento(grupoBacias, "bacia_montante");
    criarItemElemento(grupoBacias, "bacia_leste");
    criarItemElemento(grupoCanais, "canal_principal");
    criarItemElemento(grupoCanais, "trecho_jusante");
    criarItemElemento(grupoEstruturas, "estrutura_saida");

    projetoItem->setExpanded(true);
    grupoBacias->setExpanded(true);
    grupoCanais->setExpanded(true);
    grupoEstruturas->setExpanded(true);
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

    QTreeWidgetItem* grupo = projetoItem->child(0);
    if (!grupo || grupo->childCount() == 0) {
        return;
    }

    m_arvoreRede->setCurrentItem(grupo->child(0));
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
    }
}

QString MainWindow::chaveElemento(QTreeWidgetItem* item) const
{
    if (!item) {
        return {};
    }

    return item->data(0, kRoleElementoId).toString();
}
