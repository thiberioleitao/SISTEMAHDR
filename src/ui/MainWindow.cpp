#include "MainWindow.h"

#include "ui/delegates/DelegadoGeometriaSecaoComboBox.h"
#include "ui/delegates/DelegadoRevestimentoCanalComboBox.h"
#include "ui/delegates/DelegadoSecaoTransversalComboBox.h"
#include "ui/delegates/DelegadoUsoOcupacaoSoloComboBox.h"

#include "domain/IDF.h"
#include "domain/CalculoHidraulicoCanal.h"
#include "domain/RedeHidrologica.h"

#include "ui/modelos/ModeloTabelaBacias.h"
#include "ui/modelos/ModeloTabelaCanais.h"
#include "ui/modelos/ModeloTabelaRevestimentos.h"
#include "ui/modelos/ModeloTabelaSecoesTransversais.h"
#include "ui/modelos/ModeloTabelaUsoOcupacaoSolo.h"

#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QAction>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QDockWidget>
#include <QGridLayout>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QShortcut>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
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
constexpr auto kAbaRevestimentos = "aba_revestimentos";
constexpr auto kAbaUsoOcupacaoSolo = "aba_uso_ocupacao_solo";
constexpr auto kAbaResultadosCanais = "aba_resultados_canais";
constexpr auto kAbaResultadosCanaisDeclividadeMinima = "aba_resultados_canais_declividade_minima";
constexpr auto kAbaResultadosCanaisDeclividadeMaxima = "aba_resultados_canais_declividade_maxima";
constexpr auto kAbaResultadosCanaisDeclividadeFinal = "aba_resultados_canais_declividade_final";
constexpr auto kAbaResultadosBacias = "aba_resultados_bacias";

/**
 * @brief Aplica sombra suave em widgets destacados da interface.
 * @param widget Widget que recebera a elevacao visual.
 */
void aplicarSombraSuave(QWidget* widget)
{
    if (!widget) return;

    auto* sombra = new QGraphicsDropShadowEffect(widget);
    sombra->setBlurRadius(24);
    sombra->setOffset(0, 8);
    sombra->setColor(QColor(31, 78, 121, 28));
    widget->setGraphicsEffect(sombra);
}

/**
 * @brief Retorna o icone padronizado utilizado nas acoes e cards da interface.
 * @param tipo Identificador semantico da acao.
 * @return Icone configurado para o tipo solicitado.
 */
QIcon iconePadrao(const QString& tipo)
{
    const QSize tamanhoIcone(20, 20);
    QPixmap pixmap(tamanhoIcone);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QColor corPrimaria("#536b7c");
    const QColor corSecundaria("#8aa7bf");
    const QPen caneta(corPrimaria, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(caneta);
    painter.setBrush(Qt::NoBrush);

    auto desenharPonto = [&](const QPointF& centro, qreal raio, const QColor& cor) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(cor);
        painter.drawEllipse(centro, raio, raio);
        painter.restore();
        painter.setPen(caneta);
    };

    if (tipo == "novo") {
        painter.drawRoundedRect(QRectF(4.2, 2.8, 10.8, 14.2), 2.0, 2.0);
        painter.drawLine(QPointF(15.0, 7.0), QPointF(11.8, 3.8));
        painter.drawLine(QPointF(10.2, 10.0), QPointF(10.2, 15.0));
        painter.drawLine(QPointF(7.7, 12.5), QPointF(12.7, 12.5));
    } else if (tipo == "abrir") {
        QPainterPath pasta;
        pasta.moveTo(3.0, 7.0);
        pasta.lineTo(7.4, 7.0);
        pasta.lineTo(9.2, 5.2);
        pasta.lineTo(17.0, 5.2);
        pasta.lineTo(17.0, 15.7);
        pasta.lineTo(3.0, 15.7);
        pasta.closeSubpath();
        painter.drawPath(pasta);
        painter.drawLine(QPointF(5.0, 9.2), QPointF(15.2, 9.2));
    } else if (tipo == "salvar") {
        painter.drawRoundedRect(QRectF(4.0, 3.0, 12.0, 14.0), 2.0, 2.0);
        painter.drawRect(QRectF(6.5, 4.7, 7.0, 3.4));
        painter.drawLine(QPointF(7.0, 12.5), QPointF(13.0, 12.5));
    } else if (tipo == "importar") {
        painter.drawLine(QPointF(10.0, 3.5), QPointF(10.0, 12.5));
        painter.drawLine(QPointF(6.8, 9.4), QPointF(10.0, 12.7));
        painter.drawLine(QPointF(13.2, 9.4), QPointF(10.0, 12.7));
        painter.drawRoundedRect(QRectF(4.5, 13.4, 11.0, 2.4), 1.1, 1.1);
    } else if (tipo == "dados") {
        painter.drawRoundedRect(QRectF(4.0, 4.0, 12.0, 12.0), 3.0, 3.0);
        painter.drawLine(QPointF(10.0, 5.5), QPointF(10.0, 14.5));
        painter.drawLine(QPointF(5.5, 10.0), QPointF(14.5, 10.0));
    } else if (tipo == "modelo") {
        desenharPonto(QPointF(5.0, 10.0), 1.7, corSecundaria);
        desenharPonto(QPointF(10.0, 5.0), 1.7, corSecundaria);
        desenharPonto(QPointF(15.0, 10.0), 1.7, corSecundaria);
        desenharPonto(QPointF(10.0, 15.0), 1.7, corSecundaria);
        painter.drawLine(QPointF(6.5, 9.0), QPointF(8.7, 6.5));
        painter.drawLine(QPointF(11.3, 6.5), QPointF(13.5, 9.0));
        painter.drawLine(QPointF(13.5, 11.0), QPointF(11.3, 13.5));
        painter.drawLine(QPointF(8.7, 13.5), QPointF(6.5, 11.0));
    } else if (tipo == "resultados") {
        painter.drawLine(QPointF(4.0, 15.5), QPointF(16.0, 15.5));
        painter.drawRoundedRect(QRectF(5.2, 10.2, 2.2, 5.3), 0.8, 0.8);
        painter.drawRoundedRect(QRectF(8.8, 7.8, 2.2, 7.7), 0.8, 0.8);
        painter.drawRoundedRect(QRectF(12.4, 5.2, 2.2, 10.3), 0.8, 0.8);
    } else if (tipo == "canais") {
        QPainterPath onda;
        onda.moveTo(3.0, 12.0);
        onda.cubicTo(5.0, 7.0, 7.0, 17.0, 9.0, 12.0);
        onda.cubicTo(11.0, 7.0, 13.0, 17.0, 15.0, 12.0);
        onda.cubicTo(15.6, 10.8, 16.2, 10.0, 17.0, 9.4);
        painter.drawPath(onda);
        painter.drawLine(QPointF(4.0, 6.3), QPointF(16.0, 6.3));
    } else if (tipo == "bacias") {
        QPainterPath poligono;
        poligono.moveTo(5.0, 5.5);
        poligono.lineTo(12.8, 4.2);
        poligono.lineTo(15.8, 9.0);
        poligono.lineTo(13.2, 15.5);
        poligono.lineTo(6.2, 15.0);
        poligono.lineTo(3.8, 10.2);
        poligono.closeSubpath();
        painter.drawPath(poligono);
        painter.drawLine(QPointF(7.0, 13.0), QPointF(12.5, 6.5));
    } else if (tipo == "secoes") {
        QPainterPath secao;
        secao.moveTo(4.0, 14.5);
        secao.lineTo(7.3, 6.0);
        secao.lineTo(12.7, 6.0);
        secao.lineTo(16.0, 14.5);
        painter.drawPath(secao);
        painter.drawLine(QPointF(6.0, 10.8), QPointF(14.0, 10.8));
    } else if (tipo == "solo") {
        painter.drawRoundedRect(QRectF(4.5, 5.0, 11.0, 2.4), 1.0, 1.0);
        painter.drawRoundedRect(QRectF(4.5, 8.8, 11.0, 2.4), 1.0, 1.0);
        painter.drawRoundedRect(QRectF(4.5, 12.6, 11.0, 2.4), 1.0, 1.0);
    } else if (tipo == "calcular") {
        painter.drawEllipse(QRectF(4.2, 4.2, 11.6, 11.6));
        painter.drawLine(QPointF(10.0, 6.5), QPointF(10.0, 10.2));
        painter.drawLine(QPointF(10.0, 10.2), QPointF(12.8, 11.8));
    } else if (tipo == "min") {
        painter.drawLine(QPointF(4.0, 15.5), QPointF(16.0, 15.5));
        painter.drawLine(QPointF(5.0, 7.0), QPointF(15.0, 12.8));
        painter.drawLine(QPointF(5.0, 12.8), QPointF(8.2, 12.8));
    } else if (tipo == "max") {
        painter.drawLine(QPointF(4.0, 15.5), QPointF(16.0, 15.5));
        painter.drawLine(QPointF(5.0, 12.8), QPointF(15.0, 7.0));
        painter.drawLine(QPointF(11.8, 7.0), QPointF(15.0, 7.0));
    } else if (tipo == "final") {
        painter.drawLine(QPointF(4.5, 15.0), QPointF(8.0, 10.5));
        painter.drawLine(QPointF(8.0, 10.5), QPointF(11.2, 13.0));
        painter.drawLine(QPointF(11.2, 13.0), QPointF(15.5, 6.0));
        desenharPonto(QPointF(15.5, 6.0), 1.5, corSecundaria);
    } else {
        painter.drawEllipse(QRectF(5.0, 5.0, 10.0, 10.0));
        painter.drawLine(QPointF(10.0, 8.0), QPointF(10.0, 10.6));
        desenharPonto(QPointF(10.0, 13.1), 0.9, corPrimaria);
    }

    return QIcon(pixmap);
}

void configurarVisibilidadeColunasResultadosCanais(QTableView* tabela, const QString& modoDeclividade)
{
    if (!tabela || !tabela->model()) return;

    // Colunas base comuns para todas as visões de resultados dos canais.
    const QSet<int> colunasBase = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    QSet<int> colunasPermitidas = colunasBase;

    if (modoDeclividade == "min") {
        for (int coluna = 12; coluna <= 22; ++coluna) colunasPermitidas.insert(coluna);
    }
    else if (modoDeclividade == "max") {
        for (int coluna = 23; coluna <= 33; ++coluna) colunasPermitidas.insert(coluna);
    }
    else if (modoDeclividade == "final") {
        for (int coluna = 34; coluna <= 36; ++coluna) colunasPermitidas.insert(coluna);
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

/**
 * @brief Converte um texto vindo do cadastro de secoes para numero.
 * @param texto Valor textual armazenado no modelo.
 * @return Numero convertido com protecao para vazios e simbolos.
 */
double converterTextoSecaoParaNumero(QString texto)
{
    texto = texto.trimmed();
    texto.remove('%');
    texto.remove('"');
    texto.replace(',', '.');
    return texto.toDouble();
}

/**
 * @brief Procura um registro de secao e devolve um fallback seguro quando ele nao existe.
 * @param idSecao Identificador da secao procurada.
 * @param modeloSecoes Modelo de secoes cadastrado na interface.
 * @return Registro da secao localizada ou um registro padrao seguro.
 */
RegistroSecaoTransversal obterSecaoPorId(const QString& idSecao,
                                         const ModeloTabelaSecoesTransversais* modeloSecoes)
{
    RegistroSecaoTransversal registro;
    if (modeloSecoes && modeloSecoes->obterRegistroPorNomenclatura(idSecao, &registro)) {
        return registro;
    }

    registro.nomenclatura = idSecao.trimmed();
    registro.geometria = "Trapezoidal";
    registro.baseMenorM = "0.50";
    registro.talude = "1.00";
    registro.diametroM = "-";
    registro.alturaMaximaM = "1.00";
    registro.folgaMinimaM = "0.10";
    registro.revestimento = RevestimentoCanal::revestimentoPadraoSeguro().nome();
    return registro;
}

/**
 * @brief Retorna a altura maxima fisica da secao cadastrada.
 * @param registro Registro textual da secao.
 * @return Altura maxima utilizavel da secao em metros.
 */
double alturaMaximaSecao(const RegistroSecaoTransversal& registro)
{
    const double alturaMaxima = std::max(0.0, converterTextoSecaoParaNumero(registro.alturaMaximaM));
    if (alturaMaxima > 0.0) {
        return alturaMaxima;
    }

    if (registro.geometria.contains("Semicircular", Qt::CaseInsensitive)) {
        return std::max(0.0, converterTextoSecaoParaNumero(registro.diametroM));
    }

    return 1.0;
}

/**
 * @brief Retorna a altura maxima fisica da secao associada ao ID informado.
 * @param idSecao Identificador da secao procurada.
 * @param modeloSecoes Modelo com o cadastro de secoes.
 * @return Altura maxima utilizavel da secao em metros.
 */
double alturaMaximaSecaoPorId(const QString& idSecao,
                              const ModeloTabelaSecoesTransversais* modeloSecoes)
{
    return alturaMaximaSecao(obterSecaoPorId(idSecao, modeloSecoes));
}

/**
 * @brief Retorna o criterio de folga minimo associado a uma secao.
 * @param idSecao Identificador da secao procurada.
 * @param modeloSecoes Modelo com o cadastro de secoes.
 * @return Folga minima exigida em metros.
 */
double folgaMinimaSecaoPorId(const QString& idSecao,
                             const ModeloTabelaSecoesTransversais* modeloSecoes)
{
    const RegistroSecaoTransversal registro = obterSecaoPorId(idSecao, modeloSecoes);
    return std::max(0.0, converterTextoSecaoParaNumero(registro.folgaMinimaM));
}

/**
 * @brief Retorna o revestimento associado a uma secao transversal.
 * @param idSecao Identificador da secao procurada.
 * @param modeloSecoes Modelo com o cadastro de secoes.
 * @return Revestimento padronizado usado no canal.
 */
RevestimentoCanal revestimentoSecaoPorId(const QString& idSecao,
                                         const ModeloTabelaSecoesTransversais* modeloSecoes,
                                         const ModeloTabelaRevestimentos* modeloRevestimentos)
{
    const RegistroSecaoTransversal registro = obterSecaoPorId(idSecao, modeloSecoes);
    if (modeloRevestimentos) {
        if (const RevestimentoCanal* revestimento = modeloRevestimentos->revestimentoPorNome(registro.revestimento)) {
            return *revestimento;
        }
    }

    return RevestimentoCanal::revestimentoPorNome(registro.revestimento);
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
                                    const ModeloTabelaSecoesTransversais* modeloSecoes,
                                    const ModeloTabelaRevestimentos* modeloRevestimentos)
{
    if (!canal || !modeloSecoes) return false;

    const QString idSecaoNormalizado = idSecao.trimmed();
    if (idSecaoNormalizado.isEmpty()) return false;

    const RegistroSecaoTransversal registro = obterSecaoPorId(idSecaoNormalizado, modeloSecoes);
    canal->setRevestimento(revestimentoSecaoPorId(idSecaoNormalizado, modeloSecoes, modeloRevestimentos));

    if (registro.geometria.compare("Semicircular", Qt::CaseInsensitive) == 0) {
        const double diametro = std::max(0.0, converterTextoSecaoParaNumero(registro.diametroM));
        canal->setSecaoSemicircular(SecaoTransversalSemicircular(diametro));
        return true;
    }

    const double larguraFundo = std::max(0.0, converterTextoSecaoParaNumero(registro.baseMenorM));
    const double taludeLateral = std::max(0.0, converterTextoSecaoParaNumero(registro.talude));
    canal->setSecaoTransversal(SecaoTransversalTrapezoidal(larguraFundo, taludeLateral));
    return true;
}

double converterTextoParaNumeroCsv(QString texto)
{
    texto = texto.trimmed();
    texto.remove('%');
    texto.remove('"');
    texto.replace(',', '.');
    return texto.toDouble();
}

/**
 * @brief Aplica o estilo visual consolidado em todos os itens de uma linha de resultados.
 * @param itensLinha Lista de itens que compoem a linha.
 * @param resultado Resultado consolidado da verificacao hidraulica.
 */
void aplicarEstiloLinhaResultado(QList<QStandardItem*>& itensLinha,
                                 const ResultadoVerificacaoCanal& resultado)
{
    QColor corFundo;
    QColor corTexto("#111111");
    QString descricaoStatus;

    if (resultado.situacaoVisual == SituacaoVisualResultadoCanal::NaoAtende) {
        corFundo = QColor("#fde7e7");
        descricaoStatus = "Nao atende";
    }
    else if (resultado.situacaoVisual == SituacaoVisualResultadoCanal::AtendeNoLimite) {
        corFundo = QColor("#dbeafe");
        descricaoStatus = "Atende no limite";
    }
    else {
        descricaoStatus = "Atende";
    }

    const QString tooltip = QString("Status visual: %1. %2")
                                .arg(descricaoStatus, resultado.detalheVisual);

    for (QStandardItem* item : itensLinha) {
        if (!item) continue;
        item->setEditable(false);
        item->setToolTip(tooltip);

        if (corFundo.isValid()) {
            item->setData(corFundo, Qt::BackgroundRole);
            item->setData(corTexto, Qt::ForegroundRole);
        }
    }
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    configurarJanela();
    configurarAreaCentral();
    configurarArvoreModelo();
    configurarPainelContextual();

    m_modeloSecoes = new ModeloTabelaSecoesTransversais(this);
    m_modeloRevestimentos = new ModeloTabelaRevestimentos(this);
    m_modeloUsoOcupacaoSolo = new ModeloTabelaUsoOcupacaoSolo(this);
    m_modeloCanais = new ModeloTabelaCanais(this);
    m_modeloBacias = new ModeloTabelaBacias(this);
    m_modeloResultadosCanais = new QStandardItemModel(this);
    m_modeloResultadosBacias = new QStandardItemModel(this);
    m_delegadoSecaoCanais = new DelegadoSecaoTransversalComboBox(m_modeloSecoes, this);
    m_delegadoGeometriaSecoes = new DelegadoGeometriaSecaoComboBox(this);
    m_delegadoRevestimentoSecoes = new DelegadoRevestimentoCanalComboBox(m_modeloSecoes, this);
    m_delegadoUsoOcupacaoBacias = new DelegadoUsoOcupacaoSoloComboBox(m_modeloUsoOcupacaoSolo, this);

    m_modeloSecoes->definirNomesRevestimentosDisponiveis(m_modeloRevestimentos->nomesDisponiveis());
    m_modeloBacias->definirModeloUsoOcupacaoSolo(m_modeloUsoOcupacaoSolo);

    prepararModeloCanais();
    prepararModeloBacias();
    calcularResultadosModelo();

    connect(m_modeloCanais, &ModeloTabelaCanais::dadosAlterados,
            this,
            [this]() {
                calcularResultadosModelo();
                atualizarDashboard();
                atualizarPainelContextual();
            });

    connect(m_modeloBacias, &ModeloTabelaBacias::dadosAlterados,
            this,
            [this]() {
                calcularResultadosModelo();
                atualizarDashboard();
                atualizarPainelContextual();
            });

    connect(m_modeloSecoes, &QAbstractItemModel::dataChanged,
            this,
            [this](const QModelIndex&, const QModelIndex&, const QList<int>&) {
                calcularResultadosModelo();
                atualizarDashboard();
                atualizarPainelContextual();
            });

    connect(m_modeloSecoes, &QAbstractItemModel::modelReset,
            this,
            [this]() {
                calcularResultadosModelo();
                atualizarDashboard();
                atualizarPainelContextual();
            });

    connect(m_modeloRevestimentos, &ModeloTabelaRevestimentos::dadosAlterados,
            this,
            [this]() {
                m_modeloSecoes->definirNomesRevestimentosDisponiveis(m_modeloRevestimentos->nomesDisponiveis());
                calcularResultadosModelo();
                atualizarPainelContextual();
            });

    connect(m_modeloUsoOcupacaoSolo, &ModeloTabelaUsoOcupacaoSolo::dadosAlterados,
            this,
            [this]() {
                m_modeloBacias->sincronizarCoeficienteRunoffPeloUsoOcupacao();
                calcularResultadosModelo();
                atualizarPainelContextual();
            });

    configurarAcoesProjeto();
    configurarAcoesCalculo();

    popularArvoreModelo();
    atualizarDashboard();
    atualizarPainelContextual();
}

void MainWindow::configurarJanela()
{
    setWindowTitle("SISTEMAHDR - Plataforma de Modelagem Hidrol\u00F3gica");
    resize(1480, 880);
    setMinimumSize(1240, 760);
    statusBar()->showMessage("Selecione um m\u00F3dulo na navega\u00E7\u00E3o lateral para editar dados e visualizar resultados.");

    setStyleSheet(
        "QMainWindow { background: #edf2f6; }"
        "QWidget { color: #213847; font-size: 11px; }"
        "QStatusBar { background: #f8fafc; color: #5f7483; border-top: 1px solid #d7e0e8; }"
        "QToolBar { background: #f8fafc; border: none; spacing: 6px; padding: 8px 14px; }"
        "QToolBar::separator { width: 10px; }"
        "QToolButton { background: transparent; color: #334c5e; border: 1px solid transparent; border-radius: 10px; padding: 6px 10px; font-size: 11px; }"
        "QToolButton:hover { background: #e7eff6; border-color: #cedae5; }"
        "QToolButton:pressed { background: #dbe7f1; }"
        "QToolButton:focus { border-color: #78a7cf; background: #eef5fb; }"
        "QDockWidget { color: #274355; font-weight: 600; }"
        "QDockWidget::title { background: #f8fafc; color: #274355; padding: 11px 14px; border-bottom: 1px solid #d7e0e8; }"
        "QTreeView, QTableView, QScrollArea, QTextEdit { background: white; border: 1px solid #d8e1e8; border-radius: 14px; outline: none; }"
        "QTreeView::item { padding: 9px 8px; margin: 3px 8px; border-radius: 10px; }"
        "QTreeView::item:hover { background: #eef4f9; }"
        "QTreeView::item:selected { background: #d9e8f5; color: #153a56; }"
        "QTreeView::branch:selected { background: transparent; }"
        "QTreeView:focus, QTableView:focus, QLineEdit:focus, QDoubleSpinBox:focus, QComboBox:focus { border: 1px solid #78a7cf; }"
        "QTabWidget::pane { border: 1px solid #d8e1e8; border-radius: 16px; background: #fcfdfe; top: -1px; }"
        "QTabBar::tab { background: #e9eff4; color: #5d7382; padding: 9px 14px; margin-right: 4px; border-top-left-radius: 12px; border-top-right-radius: 12px; font-size: 11px; }"
        "QTabBar::tab:hover { background: #dfe8f1; color: #304a5d; }"
        "QTabBar::tab:selected { background: white; color: #17394f; font-weight: 600; }"
        "QHeaderView::section { background: #eef3f7; color: #3e586b; padding: 9px 10px; border: none; border-bottom: 1px solid #d7e0e8; font-size: 11px; font-weight: 600; }"
        "QPushButton { border-radius: 11px; padding: 8px 14px; border: 1px solid #ccd8e3; background: white; color: #26465a; font-size: 11px; }"
        "QPushButton:hover { background: #eef4f9; border-color: #b4c9da; }"
        "QPushButton:pressed { background: #dfeaf3; }"
        "QPushButton:focus { border-color: #78a7cf; background: #f2f8fd; }"
        "QLineEdit, QDoubleSpinBox, QComboBox { min-height: 30px; border: 1px solid #d1dbe4; border-radius: 9px; padding: 0 9px; background: white; font-size: 11px; }");
}

void MainWindow::configurarAreaCentral()
{
    m_abasCentrais = new QTabWidget(this);
    m_abasCentrais->setTabsClosable(true);
    m_abasCentrais->setDocumentMode(true);

    m_paginaInicial = new QWidget(m_abasCentrais);
    auto* layoutInicial = new QVBoxLayout(m_paginaInicial);
    layoutInicial->setContentsMargins(28, 28, 28, 28);
    layoutInicial->setSpacing(18);

    auto* painelHeroi = new QFrame(m_paginaInicial);
    painelHeroi->setObjectName("painelHeroiDashboard");
    painelHeroi->setStyleSheet(
        "#painelHeroiDashboard {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #e8f2fb);"
        "border: 1px solid #d2e0ec;"
        "border-radius: 24px;"
        "}");
    aplicarSombraSuave(painelHeroi);

    auto* layoutHeroi = new QVBoxLayout(painelHeroi);
    layoutHeroi->setContentsMargins(24, 24, 24, 24);
    layoutHeroi->setSpacing(14);

    auto* etiquetaTopo = new QLabel("Painel de controle", painelHeroi);
    etiquetaTopo->setStyleSheet("color: #6289aa; font-size: 11px; font-weight: 700; letter-spacing: 0.08em;");

    m_labelTituloDashboard = new QLabel("Projeto hidrol\u00F3gico ativo", painelHeroi);
    m_labelTituloDashboard->setStyleSheet("color: #163a59; font-size: 18px; font-weight: 700;");

    m_labelDescricaoDashboard = new QLabel(
        "Organize dados de entrada, confira consist\u00EAncia dos modelos e navegue entre resultados hidr\u00E1ulicos com uma estrutura mais clara e profissional.",
        painelHeroi);
    m_labelDescricaoDashboard->setWordWrap(true);
    m_labelDescricaoDashboard->setStyleSheet("color: #5e7787; font-size: 11px;");

    auto* layoutAcoes = new QHBoxLayout();
    layoutAcoes->setSpacing(10);
    QPushButton* botaoNovo = criarBotaoAcao("Novo projeto", iconePadrao("novo"), true);
    QPushButton* botaoAbrir = criarBotaoAcao("Abrir projeto", iconePadrao("abrir"), false);
    QPushButton* botaoCalcular = criarBotaoAcao("Calcular resultados", iconePadrao("calcular"), false);
    layoutAcoes->addWidget(botaoNovo);
    layoutAcoes->addWidget(botaoAbrir);
    layoutAcoes->addWidget(botaoCalcular);
    layoutAcoes->addStretch();

    connect(botaoNovo, &QPushButton::clicked, this, [this]() {
        popularArvoreModelo();
        m_abasCentrais->setCurrentWidget(m_paginaInicial);
        statusBar()->showMessage("Dashboard reposicionada para um novo fluxo de trabalho.", 2500);
        atualizarPainelContextual();
    });
    connect(botaoAbrir, &QPushButton::clicked, this, [this]() {
        abrirProjetoDeArquivo();
    });
    connect(botaoCalcular, &QPushButton::clicked, this, [this]() {
        calcularResultadosModelo();
        atualizarPainelContextual();
        statusBar()->showMessage("Resultados recalculados com sucesso.", 2500);
    });

    layoutHeroi->addWidget(etiquetaTopo);
    layoutHeroi->addWidget(m_labelTituloDashboard);
    layoutHeroi->addWidget(m_labelDescricaoDashboard);
    layoutHeroi->addLayout(layoutAcoes);

    auto* layoutCards = new QHBoxLayout();
    layoutCards->setSpacing(14);

    QWidget* cardCanais = criarCardResumo("Canais modelados", "0", "Trechos cadastrados para dimensionamento.");
    QWidget* cardBacias = criarCardResumo("Bacias ativas", "0", "Contribui\u00E7\u00F5es consideradas no modelo.");
    QWidget* cardSecoes = criarCardResumo("Se\u00E7\u00F5es dispon\u00EDveis", "0", "Perfis reutiliz\u00E1veis para os canais.");

    m_labelResumoCanais = cardCanais->findChild<QLabel*>("valorResumo");
    m_labelResumoBacias = cardBacias->findChild<QLabel*>("valorResumo");
    m_labelResumoSecoes = cardSecoes->findChild<QLabel*>("valorResumo");

    layoutCards->addWidget(cardCanais);
    layoutCards->addWidget(cardBacias);
    layoutCards->addWidget(cardSecoes);

    auto* painelRecentes = new QFrame(m_paginaInicial);
    painelRecentes->setObjectName("painelRecentesDashboard");
    painelRecentes->setStyleSheet(
        "#painelRecentesDashboard { background: white; border: 1px solid #d7e1e8; border-radius: 22px; }"
        "#painelRecentesDashboard QPushButton { text-align: left; padding: 16px 18px; }");
    aplicarSombraSuave(painelRecentes);

    auto* layoutRecentes = new QVBoxLayout(painelRecentes);
    layoutRecentes->setContentsMargins(20, 20, 20, 20);
    layoutRecentes->setSpacing(10);

    auto* tituloRecentes = new QLabel("Projetos recentes e atalhos", painelRecentes);
    tituloRecentes->setStyleSheet("color: #173d5d; font-size: 15px; font-weight: 700;");

    auto* subtituloRecentes = new QLabel(
        "Acesse rapidamente os grupos de dados mais usados para edi\u00E7\u00E3o e confer\u00EAncia.",
        painelRecentes);
    subtituloRecentes->setWordWrap(true);
    subtituloRecentes->setStyleSheet("color: #5f7a8d; font-size: 11px;");

    QPushButton* botaoSecoes = criarBotaoAcao("Abrir se\u00E7\u00F5es transversais", iconePadrao("secoes"), false);
    QPushButton* botaoCanais = criarBotaoAcao("Abrir canais", iconePadrao("canais"), false);
    QPushButton* botaoBacias = criarBotaoAcao("Abrir bacias", iconePadrao("bacias"), false);
    QPushButton* botaoResultados = criarBotaoAcao("Abrir resultados de canais", iconePadrao("resultados"), false);

    connect(botaoSecoes, &QPushButton::clicked, this, [this]() { garantirAbaSecoesTransversais(); });
    connect(botaoCanais, &QPushButton::clicked, this, [this]() { garantirAbaCanais(); });
    connect(botaoBacias, &QPushButton::clicked, this, [this]() { garantirAbaBacias(); });
    connect(botaoResultados, &QPushButton::clicked, this, [this]() { garantirAbaResultadosCanais(); });

    layoutRecentes->addWidget(tituloRecentes);
    layoutRecentes->addWidget(subtituloRecentes);
    layoutRecentes->addWidget(botaoSecoes);
    layoutRecentes->addWidget(botaoCanais);
    layoutRecentes->addWidget(botaoBacias);
    layoutRecentes->addWidget(botaoResultados);

    layoutInicial->addWidget(painelHeroi);
    layoutInicial->addLayout(layoutCards);
    layoutInicial->addWidget(painelRecentes);
    layoutInicial->addStretch();

    m_abasCentrais->addTab(m_paginaInicial, iconePadrao("dados"), "In\u00EDcio");
    m_abasCentrais->widget(0)->setProperty("tabKey", kAbaInicial);

    connect(m_abasCentrais, &QTabWidget::tabCloseRequested, this, [this](int indice) {
        QWidget* pagina = m_abasCentrais->widget(indice);
        if (pagina == m_paginaInicial) return;
        if (!pagina) return;

        const QString chave = pagina->property("tabKey").toString();
        m_tabelaPorChave.remove(chave);

        m_abasCentrais->removeTab(indice);
        pagina->deleteLater();
        atualizarPainelContextual();
    });

    connect(m_abasCentrais, &QTabWidget::currentChanged, this, [this](int) {
        atualizarPainelContextual();
    });

    setCentralWidget(m_abasCentrais);
}

void MainWindow::configurarPainelContextual()
{
    m_dockContexto = new QDockWidget("Painel contextual", this);
    m_dockContexto->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockContexto->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_dockContexto->setMinimumWidth(300);

    auto* conteudo = new QWidget(m_dockContexto);
    auto* layout = new QVBoxLayout(conteudo);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(12);

    auto* cartao = new QFrame(conteudo);
    cartao->setObjectName("cartaoContexto");
    cartao->setStyleSheet("#cartaoContexto { background: white; border: 1px solid #d7e1e8; border-radius: 20px; }");
    aplicarSombraSuave(cartao);

    auto* layoutCartao = new QVBoxLayout(cartao);
    layoutCartao->setContentsMargins(18, 18, 18, 18);
    layoutCartao->setSpacing(8);

    auto* etiqueta = new QLabel("Contexto ativo", cartao);
    etiqueta->setStyleSheet("color: #6a8ba8; font-size: 10px; font-weight: 700; letter-spacing: 0.08em;");

    m_labelTituloContexto = new QLabel(cartao);
    m_labelTituloContexto->setWordWrap(true);
    m_labelTituloContexto->setStyleSheet("color: #163a59; font-size: 15px; font-weight: 700;");

    m_labelDescricaoContexto = new QLabel(cartao);
    m_labelDescricaoContexto->setWordWrap(true);
    m_labelDescricaoContexto->setStyleSheet("color: #597386; font-size: 11px;");

    m_labelDetalhesContexto = new QLabel(cartao);
    m_labelDetalhesContexto->setWordWrap(true);
    m_labelDetalhesContexto->setStyleSheet(
        "background: #f4f8fb; color: #365266; border: 1px solid #dbe7f0; border-radius: 14px; padding: 10px; font-size: 11px;");

    layoutCartao->addWidget(etiqueta);
    layoutCartao->addWidget(m_labelTituloContexto);
    layoutCartao->addWidget(m_labelDescricaoContexto);
    layoutCartao->addWidget(m_labelDetalhesContexto);

    layout->addWidget(cartao);
    layout->addStretch();

    m_dockContexto->setWidget(conteudo);
    addDockWidget(Qt::RightDockWidgetArea, m_dockContexto);
}

void MainWindow::configurarArvoreModelo()
{
    m_dockModelo = new QDockWidget("Navega\u00E7\u00E3o do projeto", this);
    m_dockModelo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockModelo->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_dockModelo->setMinimumWidth(280);

    m_arvoreModelo = new QTreeView(m_dockModelo);
    m_arvoreModelo->setAlternatingRowColors(true);
    m_arvoreModelo->setUniformRowHeights(true);
    m_arvoreModelo->setAnimated(true);
    m_arvoreModelo->setHeaderHidden(true);
    m_arvoreModelo->setIndentation(14);
    m_arvoreModelo->setExpandsOnDoubleClick(false);
    m_arvoreModelo->setIconSize(QSize(18, 18));
    m_arvoreModelo->setStyleSheet(
        "QTreeView { background: #fbfcfe; padding: 8px 0; }"
        "QTreeView::item { min-height: 24px; }");

    m_modeloArvore = new QStandardItemModel(this);
    m_modeloArvore->setHorizontalHeaderLabels({ "Estrutura hidr\u00E1ulica" });
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
        iconePadrao("modelo"));

    QStandardItem* itemDados = criarItemArvore(
        raiz,
        "Dados",
        "dados",
        iconePadrao("dados"));

    criarItemArvore(itemDados, "Se\u00E7\u00F5es transversais", "secoes", iconePadrao("secoes"));
    criarItemArvore(itemDados, "Revestimentos", "revestimentos", iconePadrao("dados"));
    criarItemArvore(itemDados, "Canais", "canais", iconePadrao("canais"));
    criarItemArvore(itemDados, "Bacias", "bacias", iconePadrao("bacias"));
    criarItemArvore(itemDados, "Uso e ocupa\u00E7\u00E3o do solo", "uso_ocupacao_solo", iconePadrao("solo"));

    QStandardItem* itemResultados = criarItemArvore(
        raiz,
        "Resultados",
        "resultados",
        iconePadrao("resultados"));

    QStandardItem* itemResultadosCanais = criarItemArvore(itemResultados, "Canais", "resultados_canais", iconePadrao("resultados"));
    criarItemArvore(itemResultados, "Bacias", "resultados_bacias", iconePadrao("resultados"));

    if (itemResultadosCanais) {
        criarItemArvore(itemResultadosCanais, "Declividade m\u00EDnima", "resultados_canais_declividade_minima", iconePadrao("min"));
        criarItemArvore(itemResultadosCanais, "Declividade m\u00E1xima", "resultados_canais_declividade_maxima", iconePadrao("max"));
        criarItemArvore(itemResultadosCanais, "Declividade final", "resultados_canais_declividade_final", iconePadrao("final"));
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
    else if (tipo == "revestimentos") {
        garantirAbaRevestimentos();
        statusBar()->showMessage("Aba de revestimentos aberta.", 3000);
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
    tabela->setItemDelegateForColumn(ModeloTabelaSecoesTransversais::ColunaRevestimento, m_delegadoRevestimentoSecoes);
    ajustarLarguraColunasTabela(tabela);

    QWidget* pagina = criarPaginaTabela(
        kAbaSecoes,
        "Se\u00E7\u00F5es transversais",
        "Cadastre geometrias reutiliz\u00E1veis para perfis trapezoidais e semicirculares.",
        "Se\u00E7\u00F5es",
        tabela,
        iconePadrao("secoes"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("secoes"), "Se\u00E7\u00F5es");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaCanais,
        "Canais",
        "Edite trechos, vincule se\u00E7\u00F5es e mantenha a estrutura jusante organizada.",
        "Canais",
        tabela,
        iconePadrao("canais"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("canais"), "Canais");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaBacias,
        "Bacias contribuintes",
        "Centralize \u00E1rea, talvegue, declividade e v\u00EDnculo de uso e ocupa\u00E7\u00E3o do solo.",
        "Bacias",
        tabela,
        iconePadrao("bacias"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("bacias"), "Bacias");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaUsoOcupacaoSolo,
        "Uso e ocupa\u00E7\u00E3o do solo",
        "Padronize coeficientes hidrol\u00F3gicos e par\u00E2metros complementares por tipo de cobertura.",
        "Uso do solo",
        tabela,
        iconePadrao("solo"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("solo"), "Uso do solo");
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaUsoOcupacaoSolo, tabela);
}

void MainWindow::garantirAbaRevestimentos()
{
    const int indiceExistente = indiceAbaPorChave(kAbaRevestimentos);
    if (indiceExistente >= 0) {
        m_abasCentrais->setCurrentIndex(indiceExistente);
        return;
    }

    QTableView* tabela = criarTabelaBase();
    tabela->setModel(m_modeloRevestimentos);
    ajustarLarguraColunasTabela(tabela);

    QWidget* pagina = criarPaginaTabela(
        kAbaRevestimentos,
        "Revestimentos hidraulicos",
        "Ajuste Manning, limites admissiveis e parametros complementares do catalogo de revestimentos.",
        "Revestimentos",
        tabela,
        iconePadrao("dados"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("dados"), "Revestimentos");
    m_abasCentrais->setCurrentIndex(indice);
    m_tabelaPorChave.insert(kAbaRevestimentos, tabela);
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

    QWidget* pagina = criarPaginaTabela(
        kAbaResultadosCanais,
        "Resultados hidr\u00E1ulicos dos canais",
        "Compare indicadores consolidados de dimensionamento, folga, velocidade e crit\u00E9rios de verifica\u00E7\u00E3o.",
        "Resultados canais",
        tabela,
        iconePadrao("resultados"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("resultados"), "Resultados canais");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaResultadosCanaisDeclividadeMinima,
        "Resultados de canais com declividade m\u00EDnima",
        "Revise a resposta hidr\u00E1ulica considerando o limite inferior de declividade admiss\u00EDvel.",
        "Sp min",
        tabela,
        iconePadrao("resultados"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("resultados"), "Sp min");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaResultadosCanaisDeclividadeMaxima,
        "Resultados de canais com declividade m\u00E1xima",
        "Avalie a condi\u00E7\u00E3o mais cr\u00EDtica para velocidades e tens\u00E3o cisalhante.",
        "Sp max",
        tabela,
        iconePadrao("resultados"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("resultados"), "Sp max");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaResultadosCanaisDeclividadeFinal,
        "Resultados de canais com declividade final",
        "Confer\u00EAncia da solu\u00E7\u00E3o adotada para execu\u00E7\u00E3o e compatibiliza\u00E7\u00E3o final.",
        "Sp final",
        tabela,
        iconePadrao("resultados"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("resultados"), "Sp final");
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

    QWidget* pagina = criarPaginaTabela(
        kAbaResultadosBacias,
        "Resultados hidrol\u00F3gicos das bacias",
        "Consulte contribui\u00E7\u00F5es racionais e par\u00E2metros sintetizados para confer\u00EAncia do escoamento.",
        "Resultados bacias",
        tabela,
        iconePadrao("resultados"));
    const int indice = m_abasCentrais->addTab(pagina, iconePadrao("resultados"), "Resultados bacias");
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
    tabela->setShowGrid(false);
    tabela->setWordWrap(false);
    tabela->setCornerButtonEnabled(false);
    tabela->verticalHeader()->setVisible(true);
    tabela->verticalHeader()->setSectionsClickable(true);
    tabela->verticalHeader()->setDefaultSectionSize(32);
    tabela->horizontalHeader()->setSectionsClickable(true);
    tabela->horizontalHeader()->setHighlightSections(true);
    tabela->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tabela->horizontalHeader()->setStretchLastSection(false);
    tabela->horizontalHeader()->setMinimumSectionSize(90);
    tabela->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tabela->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tabela->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tabela->setStyleSheet(
        "QTableView { background: white; alternate-background-color: #f7f9fb; selection-background-color: #dce9f6; selection-color: #17394f; border: 1px solid #d7e0e8; }"
        "QTableView::item { padding: 7px 10px; border-bottom: 1px solid #edf2f6; }"
        "QTableCornerButton::section { background: #eef3f7; border: none; border-bottom: 1px solid #d7e0e8; }"
        "QHeaderView::section { background: #eef3f7; color: #425a6b; border-right: none; border-left: none; }");
    tabela->verticalHeader()->setStyleSheet(
        "QHeaderView::section { background: #f6f9fb; color: #6d8190; padding: 0 8px; border: none; border-right: 1px solid #e5edf3; }");
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

QWidget* MainWindow::criarPaginaTabela(const QString& chave,
                                       const QString& titulo,
                                       const QString& descricao,
                                       const QString& tituloAba,
                                       QTableView* tabela,
                                       const QIcon& icone)
{
    auto* pagina = new QWidget(m_abasCentrais);
    pagina->setProperty("tabKey", chave);

    auto* layoutPrincipal = new QVBoxLayout(pagina);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    layoutPrincipal->setSpacing(14);

    auto* cabecalho = new QFrame(pagina);
    cabecalho->setStyleSheet("background: white; border: 1px solid #d7e1e8; border-radius: 18px;");
    aplicarSombraSuave(cabecalho);

    auto* layoutCabecalho = new QHBoxLayout(cabecalho);
    layoutCabecalho->setContentsMargins(16, 14, 16, 14);
    layoutCabecalho->setSpacing(12);

    auto* iconeTitulo = new QLabel(cabecalho);
    iconeTitulo->setPixmap(icone.pixmap(18, 18));
    iconeTitulo->setFixedSize(26, 26);
    iconeTitulo->setAlignment(Qt::AlignCenter);
    iconeTitulo->setStyleSheet("background: #eff4f8; border: 1px solid #dbe6ef; border-radius: 13px;");

    auto* layoutTextos = new QVBoxLayout();
    layoutTextos->setSpacing(4);

    auto* labelTitulo = new QLabel(titulo, cabecalho);
    labelTitulo->setStyleSheet("color: #163a59; font-size: 15px; font-weight: 700;");

    auto* labelDescricao = new QLabel(descricao, cabecalho);
    labelDescricao->setWordWrap(true);
    labelDescricao->setStyleSheet("color: #60798d; font-size: 11px;");

    auto* etiquetaAba = new QLabel(tituloAba, cabecalho);
    etiquetaAba->setStyleSheet(
        "background: #eff4f8; color: #58788f; border: 1px solid #dbe6ef; border-radius: 10px; padding: 5px 9px; font-size: 10px; font-weight: 700;");

    layoutTextos->addWidget(labelTitulo);
    layoutTextos->addWidget(labelDescricao);

    layoutCabecalho->addWidget(iconeTitulo, 0, Qt::AlignTop);
    layoutCabecalho->addLayout(layoutTextos, 1);
    layoutCabecalho->addWidget(etiquetaAba, 0, Qt::AlignTop);

    layoutPrincipal->addWidget(cabecalho);
    layoutPrincipal->addWidget(tabela, 1);
    return pagina;
}

QPushButton* MainWindow::criarBotaoAcao(const QString& texto,
                                        const QIcon& icone,
                                        bool destaque) const
{
    auto* botao = new QPushButton(icone, texto);
    botao->setCursor(Qt::PointingHandCursor);
    botao->setMinimumHeight(36);
    botao->setIconSize(QSize(16, 16));
    botao->setStyleSheet(
        "QPushButton { background: #ffffff; color: #294658; border: 1px solid #d4dee7; border-radius: 10px; padding: 7px 12px; text-align: left; }"
        "QPushButton:hover { background: #f0f5f9; border-color: #b9ccdc; }"
        "QPushButton:pressed { background: #e2edf5; }"
        "QPushButton:focus { border-color: #78a7cf; background: #f4f9fd; }");
    if (destaque) {
        botao->setStyleSheet(
            "QPushButton { background: #2f6b9b; color: white; border: 1px solid #2f6b9b; border-radius: 10px; padding: 7px 12px; text-align: left; font-weight: 600; }"
            "QPushButton:hover { background: #285b84; border-color: #285b84; }"
            "QPushButton:pressed { background: #1f4d6f; }"
            "QPushButton:focus { border-color: #8db8dc; }");
    }
    return botao;
}

QWidget* MainWindow::criarCardResumo(const QString& titulo,
                                     const QString& valor,
                                     const QString& descricao) const
{
    auto* card = new QFrame(m_paginaInicial);
    card->setStyleSheet("background: white; border: 1px solid #d7e1e8; border-radius: 20px;");
    aplicarSombraSuave(card);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(6);

    auto* labelTitulo = new QLabel(titulo, card);
    labelTitulo->setStyleSheet("color: #657d8f; font-size: 11px; font-weight: 700;");

    auto* labelValor = new QLabel(valor, card);
    labelValor->setObjectName("valorResumo");
    labelValor->setStyleSheet("color: #163a59; font-size: 20px; font-weight: 700;");

    auto* labelDescricao = new QLabel(descricao, card);
    labelDescricao->setWordWrap(true);
    labelDescricao->setStyleSheet("color: #6c8597; font-size: 11px;");

    layout->addWidget(labelTitulo);
    layout->addWidget(labelValor);
    layout->addWidget(labelDescricao);
    layout->addStretch();
    return card;
}

void MainWindow::atualizarDashboard()
{
    if (m_labelResumoCanais) {
        m_labelResumoCanais->setText(QString::number(m_modeloCanais ? m_modeloCanais->rowCount() : 0));
    }
    if (m_labelResumoBacias) {
        m_labelResumoBacias->setText(QString::number(m_modeloBacias ? m_modeloBacias->rowCount() : 0));
    }
    if (m_labelResumoSecoes) {
        m_labelResumoSecoes->setText(QString::number(m_modeloSecoes ? m_modeloSecoes->rowCount() : 0));
    }
    if (m_labelTituloDashboard) {
        m_labelTituloDashboard->setText("Projeto hidrol\u00F3gico ativo");
    }
    if (m_labelDescricaoDashboard) {
        const int canais = m_modeloCanais ? m_modeloCanais->rowCount() : 0;
        const int bacias = m_modeloBacias ? m_modeloBacias->rowCount() : 0;
        const int secoes = m_modeloSecoes ? m_modeloSecoes->rowCount() : 0;
        m_labelDescricaoDashboard->setText(
            QString("O ambiente est\u00E1 preparado para an\u00E1lise com %1 canal(is), %2 bacia(s) e %3 se\u00E7\u00E3o(\u00F5es) cadastrada(s).")
                .arg(canais)
                .arg(bacias)
                .arg(secoes));
    }
}

void MainWindow::definirResumoContextual(const QString& titulo,
                                         const QString& descricao,
                                         const QStringList& detalhes)
{
    if (m_labelTituloContexto) {
        m_labelTituloContexto->setText(titulo);
    }
    if (m_labelDescricaoContexto) {
        m_labelDescricaoContexto->setText(descricao);
    }
    if (m_labelDetalhesContexto) {
        m_labelDetalhesContexto->setText(detalhes.join("\n"));
    }
}

void MainWindow::atualizarPainelContextual()
{
    QWidget* paginaAtual = m_abasCentrais ? m_abasCentrais->currentWidget() : nullptr;
    const QString chave = paginaAtual ? paginaAtual->property("tabKey").toString() : QString();

    if (chave == kAbaSecoes) {
        definirResumoContextual(
            "Se\u00E7\u00F5es transversais",
            "Biblioteca geom\u00E9trica para o dimensionamento dos canais.",
            {
                QString("Registros: %1").arg(m_modeloSecoes ? m_modeloSecoes->rowCount() : 0),
                "Use perfis consistentes para reduzir retrabalho de edi\u00E7\u00E3o.",
                "Clique nos cabe\u00E7alhos para selecionar linhas ou colunas inteiras."
            });
        return;
    }

    if (chave == kAbaCanais) {
        definirResumoContextual(
            "Canais",
            "Painel principal para edi\u00E7\u00E3o dos trechos e par\u00E2metros hidr\u00E1ulicos.",
            {
                QString("Trechos cadastrados: %1").arg(m_modeloCanais ? m_modeloCanais->rowCount() : 0),
                "O menu de contexto permite adicionar ou remover linhas com seguran\u00E7a.",
                "Atribua uma se\u00E7\u00E3o transversal para manter os resultados coerentes."
            });
        return;
    }

    if (chave == kAbaBacias) {
        definirResumoContextual(
            "Bacias contribuintes",
            "Organiza \u00E1reas, talvegues e conex\u00F5es hidrol\u00F3gicas para propaga\u00E7\u00E3o de vaz\u00F5es.",
            {
                QString("Bacias cadastradas: %1").arg(m_modeloBacias ? m_modeloBacias->rowCount() : 0),
                "Mantenha o campo jusante preenchido para garantir o encadeamento do modelo.",
                "Os coeficientes podem ser sincronizados pelo uso e ocupa\u00E7\u00E3o do solo."
            });
        return;
    }

    if (chave == kAbaUsoOcupacaoSolo) {
        definirResumoContextual(
            "Uso e ocupa\u00E7\u00E3o do solo",
            "Tabela de suporte para os par\u00E2metros de escoamento e eros\u00E3o.",
            {
                QString("Tipos cadastrados: %1").arg(m_modeloUsoOcupacaoSolo ? m_modeloUsoOcupacaoSolo->rowCount() : 0),
                "Importe planilhas CSV para acelerar a padroniza\u00E7\u00E3o dos coeficientes.",
                "As altera\u00E7\u00F5es repercutem nas bacias vinculadas."
            });
        return;
    }

    if (chave == kAbaRevestimentos) {
        definirResumoContextual(
            "Revestimentos",
            "Cadastro dos parametros hidraulicos usados por seções e verificacoes de resultados.",
            {
                QString("Revestimentos cadastrados: %1").arg(m_modeloRevestimentos ? m_modeloRevestimentos->rowCount() : 0),
                "Ajuste Manning, velocidade maxima admissivel e tensao de cisalhamento maxima.",
                "As secoes vinculadas passam a usar os novos parametros imediatamente."
            });
        return;
    }

    if (chave == kAbaResultadosCanais
        || chave == kAbaResultadosCanaisDeclividadeMinima
        || chave == kAbaResultadosCanaisDeclividadeMaxima
        || chave == kAbaResultadosCanaisDeclividadeFinal) {
        definirResumoContextual(
            "Resultados de canais",
            "Consolida indicadores de desempenho hidr\u00E1ulico, folga e verifica\u00E7\u00E3o normativa.",
            {
                QString("Linhas calculadas: %1").arg(m_modeloResultadosCanais ? m_modeloResultadosCanais->rowCount() : 0),
                "Abra as variantes Sp min, Sp max e Sp final para leitura focalizada.",
                "Recalcule os resultados sempre que dados de entrada forem alterados."
            });
        return;
    }

    if (chave == kAbaResultadosBacias) {
        definirResumoContextual(
            "Resultados de bacias",
            "Resumo hidrol\u00F3gico para confer\u00EAncia r\u00E1pida de contribui\u00E7\u00F5es.",
            {
                QString("Linhas calculadas: %1").arg(m_modeloResultadosBacias ? m_modeloResultadosBacias->rowCount() : 0),
                "Use esta vis\u00E3o para auditoria de \u00E1rea, declividade e Q racional.",
                "Os resultados refletem imediatamente as edi\u00E7\u00F5es dos dados de entrada."
            });
        return;
    }

    definirResumoContextual(
        "Dashboard do projeto",
        "Vis\u00E3o inicial do ambiente de trabalho com atalhos e indicadores do modelo.",
        {
            QString("Canais: %1").arg(m_modeloCanais ? m_modeloCanais->rowCount() : 0),
            QString("Bacias: %1").arg(m_modeloBacias ? m_modeloBacias->rowCount() : 0),
            "Use os cards e a navega\u00E7\u00E3o lateral para iniciar a edi\u00E7\u00E3o."
        });
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
    m_acaoSalvarProjeto->setIcon(iconePadrao("salvar"));

    m_acaoAbrirProjeto = new QAction("Abrir projeto", this);
    m_acaoAbrirProjeto->setIcon(iconePadrao("abrir"));

    m_acaoImportarBaciasCsv = new QAction("Importar bacias (.csv)", this);
    m_acaoImportarBaciasCsv->setIcon(iconePadrao("importar"));

    m_acaoImportarUsoOcupacaoCsv = new QAction("Importar uso/ocupação (.csv)", this);
    m_acaoImportarUsoOcupacaoCsv->setIcon(iconePadrao("importar"));

    m_acaoImportarCanaisCsv = new QAction("Importar canais (.csv)", this);
    m_acaoImportarCanaisCsv->setIcon(iconePadrao("importar"));

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
    barraProjeto->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    barraProjeto->setIconSize(QSize(16, 16));
    barraProjeto->addAction(m_acaoSalvarProjeto);
    barraProjeto->addAction(m_acaoAbrirProjeto);
    barraProjeto->addSeparator();
    barraProjeto->addAction(m_acaoImportarBaciasCsv);
    barraProjeto->addAction(m_acaoImportarUsoOcupacaoCsv);
    barraProjeto->addAction(m_acaoImportarCanaisCsv);
}

QJsonObject MainWindow::criarObjetoJsonProjeto() const
{
    QJsonObject projeto;
    projeto["versao"] = 3;

    QJsonArray secoesJson;
    for (const RegistroSecaoTransversal& registro : m_modeloSecoes->registros()) {
        QJsonObject item;
        item["nomenclatura"] = registro.nomenclatura;
        item["geometria"] = registro.geometria;
        item["b_menor_m"] = registro.baseMenorM;
        item["talude"] = registro.talude;
        item["diametro_m"] = registro.diametroM;
        item["altura_maxima_m"] = registro.alturaMaximaM;
        item["folga_minima_m"] = registro.folgaMinimaM;
        item["revestimento_nome"] = registro.revestimento;
        secoesJson.append(item);
    }
    projeto["secoes"] = secoesJson;

    QJsonArray revestimentosJson;
    for (const RevestimentoCanal& revestimento : m_modeloRevestimentos->revestimentos()) {
        QJsonObject item;
        item["nome"] = revestimento.nome();
        item["coeficiente_manning"] = revestimento.coeficienteManning();
        item["velocidade_maxima_admissivel_mps"] = revestimento.velocidadeMaximaAdmissivelMps();
        item["tensao_cisalhamento_maxima_admissivel_pa"] = revestimento.tensaoCisalhamentoMaximaAdmissivelPa();
        item["espessura_m"] = revestimento.espessura();
        revestimentosJson.append(item);
    }
    projeto["revestimentos"] = revestimentosJson;

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
    const QJsonArray revestimentosJson = objetoProjeto.value("revestimentos").toArray();
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
        registro.alturaMaximaM = item.contains("altura_maxima_m")
                                     ? item.value("altura_maxima_m").toVariant().toString().trimmed()
                                     : QString();
        registro.folgaMinimaM = item.contains("folga_minima_m")
                                    ? item.value("folga_minima_m").toVariant().toString().trimmed()
                                    : QString("0.10");
        registro.revestimento = item.value("revestimento_nome").toString().trimmed();
        if (registro.revestimento.isEmpty()) {
            registro.revestimento = registro.geometria.compare("Semicircular", Qt::CaseInsensitive) == 0
                                        ? QString("Concreto")
                                        : QString("Grama");
        }
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

    QVector<RevestimentoCanal> revestimentos;
    revestimentos.reserve(revestimentosJson.size());
    for (const QJsonValue& valor : revestimentosJson) {
        const QJsonObject item = valor.toObject();
        const QString nome = item.value("nome").toString().trimmed();
        if (nome.isEmpty()) continue;

        RevestimentoCanal revestimento(
            nome,
            item.value("coeficiente_manning").toDouble(),
            item.value("velocidade_maxima_admissivel_mps").toDouble(),
            item.value("tensao_cisalhamento_maxima_admissivel_pa").toDouble());
        revestimento.setEspessura(item.value("espessura_m").toDouble());
        revestimentos.append(revestimento);
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
    if (!revestimentos.isEmpty()) {
        m_modeloRevestimentos->definirRevestimentos(revestimentos);
    }
    m_modeloSecoes->definirNomesRevestimentosDisponiveis(m_modeloRevestimentos->nomesDisponiveis());
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
    atualizarDashboard();
    atualizarPainelContextual();
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

    atualizarPainelContextual();
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

    atualizarDashboard();
    atualizarPainelContextual();
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
    atualizarDashboard();
    atualizarPainelContextual();
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
    atualizarPainelContextual();
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
    atualizarDashboard();
    atualizarPainelContextual();
    statusBar()->showMessage("Canais importados com sucesso do CSV.", 3000);
}

void MainWindow::configurarAcoesCalculo()
{
    m_acaoCalcularResultados = new QAction("Calcular resultados", this);
    m_acaoCalcularResultados->setIcon(iconePadrao("calcular"));

    connect(m_acaoCalcularResultados, &QAction::triggered,
            this,
            [this]() {
                calcularResultadosModelo();
                statusBar()->showMessage("Resultados recalculados com sucesso.", 2500);
            });

    QToolBar* barraCalculo = addToolBar("Resultados");
    barraCalculo->setMovable(false);
    barraCalculo->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    barraCalculo->setIconSize(QSize(16, 16));
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
          "I_Atotal_TR (mm/h)", "Qp (dimensionamento)", "Se\u00E7\u00E3o", "Folga crit. (m)", "Revestimento", "Vmax rev. (m/s)",
          "Hn_Sp(min)", "T_Sp(min)", "Pm_Sp(min)", "Am_Sp(min)", "Rh_Sp(min)", "Ph_Sp(min)", "Folga_Sp(min)", "V_Sp(min)", "A(%)_Sp(min)", "Fr_V_Sp(min)", "tau_o_min",
          "Hn_Sp(max)", "T_Sp(max)", "Pm_Sp(max)", "Am_Sp(max)", "Rh_Sp(max)", "Ph_Sp(max)", "Folga_Sp(max)", "V_Sp(max)", "A(%)_Sp(max)", "Fr_V_Sp(max)", "tau_o_max",
          "Hn_Sp(final)", "V_Sp(final)", "tau_o_final",
          "Crit_Capacidade", "Crit_Folga", "Crit_Velocidade", "Crit_Geral" });

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

        const double folgaMinimaSecaoM = folgaMinimaSecaoPorId(idSecao, m_modeloSecoes);
        const RevestimentoCanal revestimentoSecao = revestimentoSecaoPorId(idSecao, m_modeloSecoes, m_modeloRevestimentos);
        const double alturaMaxSecaoM = std::max(1e-6, alturaMaximaSecaoPorId(idSecao, m_modeloSecoes));

        Canal canalMin = canal;
        aplicarSecaoTransversalNoCanal(&canalMin, idSecao, m_modeloSecoes, m_modeloRevestimentos);
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
        aplicarSecaoTransversalNoCanal(&canalMax, idSecao, m_modeloSecoes, m_modeloRevestimentos);
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
        aplicarSecaoTransversalNoCanal(&canalFinal, idSecao, m_modeloSecoes, m_modeloRevestimentos);
        canalFinal.setDeclividadeFinal(canal.declividadeFinal());
        const double hFinal = canalFinal.alturaLaminaParaVazaoProjeto(qDimensionamento);
        const double vFinal = canalFinal.velocidadeManning(hFinal);
        const double rhFinal = canalFinal.raioHidraulico(hFinal);
        const double tauFinal = calcularTensaoCisalhantePa(rhFinal, std::max(0.0, canal.declividadeFinal()));

        // Critérios de verificação hidráulica do canal.
        EntradaVerificacaoCanal entradaVerificacao;
        entradaVerificacao.qpHidrologiaM3s = qDimensionamento;
        entradaVerificacao.qpHidraulicaSMaxM3s = canalMax.vazaoManning(std::max(0.0, hMax));
        entradaVerificacao.alturaSMinM = std::max(0.0, hMin);
        entradaVerificacao.alturaMaximaSecaoM = alturaMaxSecaoM;
        entradaVerificacao.folgaMinimaM = folgaMinimaSecaoM;
        entradaVerificacao.folgaDisponivelSMinM = folgaMin;
        entradaVerificacao.velocidadeSMaxMps = std::max(0.0, vMax);
        entradaVerificacao.velocidadeMaximaAdmissivelMps = revestimentoSecao.velocidadeMaximaAdmissivelMps();

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
                   << new QStandardItem(textoNumero(folgaMinimaSecaoM, 3))
                   << new QStandardItem(revestimentoSecao.nome())
                   << new QStandardItem(textoNumero(revestimentoSecao.velocidadeMaximaAdmissivelMps(), 3))
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
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioCapacidadeVazao))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioFolga))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.criterioVelocidade))
                   << new QStandardItem(resultadoVerificacao.textoStatus(resultadoVerificacao.todosAtendidos()));

        aplicarEstiloLinhaResultado(itensLinha, resultadoVerificacao);

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



