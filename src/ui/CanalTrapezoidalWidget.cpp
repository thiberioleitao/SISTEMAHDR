#include "CanalTrapezoidalWidget.h"

#include "domain/Canal.h"
#include "domain/SecaoTransversalTrapezoidal.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QWidget>

#include <algorithm>

class CanalTrapezoidalDesenhoWidget : public QWidget
{
public:
    explicit CanalTrapezoidalDesenhoWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(520, 320);
    }

    void setInputs(QDoubleSpinBox* inputB, QDoubleSpinBox* inputH, QDoubleSpinBox* inputZ)
    {
        m_inputB = inputB;
        m_inputH = inputH;
        m_inputZ = inputZ;
        posicionarInputs();
    }

    void setGeometria(double larguraInferior, double alturaMaxima, double taludeLateral)
    {
        m_larguraInferior = std::max(0.1, larguraInferior);
        m_alturaMaxima = std::max(0.1, alturaMaxima);
        m_taludeLateral = std::max(0.0, taludeLateral);
        posicionarInputs();
        update();
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        posicionarInputs();
    }

    void paintEvent(QPaintEvent* event) override
    {
        QWidget::paintEvent(event);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(245, 245, 245));

        const double b = m_larguraInferior;
        const double h = m_alturaMaxima;
        const double z = m_taludeLateral;
        const double t = b + (2.0 * z * h);

        const QRectF area = rect().adjusted(32, 28, -32, -32);
        const double margem = 0.20;
        const double larguraModelo = std::max(t, b) * (1.0 + margem * 2.0);
        const double alturaModelo = h * (1.0 + margem * 2.0);
        const double escala = std::min(area.width() / larguraModelo, area.height() / alturaModelo);

        const QPointF centro(area.center().x(), area.center().y());
        const double yTopo = centro.y() - (h * escala * 0.5);
        const double yFundo = centro.y() + (h * escala * 0.5);

        const QPointF pTopoEsq(centro.x() - (t * escala * 0.5), yTopo);
        const QPointF pFundoEsq(centro.x() - (b * escala * 0.5), yFundo);
        const QPointF pFundoDir(centro.x() + (b * escala * 0.5), yFundo);
        const QPointF pTopoDir(centro.x() + (t * escala * 0.5), yTopo);

        p.setPen(QPen(Qt::black, 3));
        p.drawLine(pTopoEsq, pFundoEsq);
        p.drawLine(pFundoEsq, pFundoDir);
        p.drawLine(pFundoDir, pTopoDir);

        p.setPen(QPen(QColor(180, 0, 0), 2));

        const double yDimB = yFundo + 30.0;
        p.drawLine(QPointF(pFundoEsq.x(), yDimB), QPointF(pFundoDir.x(), yDimB));
        p.drawLine(QPointF(pFundoEsq.x(), yDimB - 7.0), QPointF(pFundoEsq.x(), yDimB + 7.0));
        p.drawLine(QPointF(pFundoDir.x(), yDimB - 7.0), QPointF(pFundoDir.x(), yDimB + 7.0));
        p.drawText(QRectF(((pFundoEsq.x() + pFundoDir.x()) * 0.5) - 45.0, yDimB - 26.0, 40.0, 20.0), Qt::AlignCenter, "b =");

        const double xDimH = pTopoEsq.x() - 30.0;
        p.drawLine(QPointF(xDimH, yTopo), QPointF(xDimH, yFundo));
        p.drawLine(QPointF(xDimH - 7.0, yTopo), QPointF(xDimH + 7.0, yTopo));
        p.drawLine(QPointF(xDimH - 7.0, yFundo), QPointF(xDimH + 7.0, yFundo));
        p.drawText(QRectF(xDimH - 44.0, ((yTopo + yFundo) * 0.5) - 38.0, 40.0, 20.0), Qt::AlignCenter, "h =");

        const QPointF meioTaludeEsq((pTopoEsq.x() + pFundoEsq.x()) * 0.5, (pTopoEsq.y() + pFundoEsq.y()) * 0.5);
        p.drawText(QRectF(meioTaludeEsq.x() + 18.0, meioTaludeEsq.y() - 46.0, 42.0, 20.0), Qt::AlignCenter, "z =");

        const double yAgua = yTopo + ((yFundo - yTopo) * 0.55);
        const double frac = (yFundo - yAgua) / (yFundo - yTopo);
        const double larguraNaLamina = b + (2.0 * z * h * frac);

        p.setPen(QPen(QColor(30, 80, 255), 2));
        p.drawLine(QPointF(centro.x() - (larguraNaLamina * escala * 0.5), yAgua),
                   QPointF(centro.x() + (larguraNaLamina * escala * 0.5), yAgua));
    }

private:
    void posicionarInputs()
    {
        if (!m_inputB || !m_inputH || !m_inputZ) return;

        const double b = m_larguraInferior;
        const double h = m_alturaMaxima;
        const double z = m_taludeLateral;
        const double t = b + (2.0 * z * h);

        const QRectF area = rect().adjusted(32, 28, -32, -32);
        const double margem = 0.20;
        const double larguraModelo = std::max(t, b) * (1.0 + margem * 2.0);
        const double alturaModelo = h * (1.0 + margem * 2.0);
        const double escala = std::min(area.width() / larguraModelo, area.height() / alturaModelo);

        const QPointF centro(area.center().x(), area.center().y());
        const double yTopo = centro.y() - (h * escala * 0.5);
        const double yFundo = centro.y() + (h * escala * 0.5);

        const QPointF pTopoEsq(centro.x() - (t * escala * 0.5), yTopo);
        const QPointF pFundoEsq(centro.x() - (b * escala * 0.5), yFundo);
        const QPointF pFundoDir(centro.x() + (b * escala * 0.5), yFundo);

        const int w = 90;
        const int hInput = 26;
        const int margemTela = 8;

        auto clampX = [&](int x) {
            return std::clamp(x, margemTela, width() - w - margemTela);
        };
        auto clampY = [&](int y) {
            return std::clamp(y, margemTela, height() - hInput - margemTela);
        };

        const int xB = clampX(static_cast<int>(((pFundoEsq.x() + pFundoDir.x()) * 0.5) - (w * 0.5)));
        const int yB = clampY(static_cast<int>(yFundo + 28.0));
        m_inputB->setGeometry(xB, yB, w, hInput);

        const int xH = clampX(static_cast<int>(pTopoEsq.x() - 126.0));
        const int yH = clampY(static_cast<int>(((yTopo + yFundo) * 0.5) - (hInput * 0.5) + 8.0));
        m_inputH->setGeometry(xH, yH, w, hInput);

        const int xZ = clampX(static_cast<int>(pTopoEsq.x() + 58.0));
        const int yZ = clampY(static_cast<int>(yTopo + 18.0));
        m_inputZ->setGeometry(xZ, yZ, w, hInput);
    }

private:
    double m_larguraInferior = 3.0;
    double m_alturaMaxima = 2.0;
    double m_taludeLateral = 1.0;

    QDoubleSpinBox* m_inputB = nullptr;
    QDoubleSpinBox* m_inputH = nullptr;
    QDoubleSpinBox* m_inputZ = nullptr;
};

CanalTrapezoidalWidget::CanalTrapezoidalWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layoutPrincipal = new QHBoxLayout(this);
    layoutPrincipal->setContentsMargins(8, 8, 8, 8);
    layoutPrincipal->setSpacing(12);

    m_desenho = new CanalTrapezoidalDesenhoWidget(this);

    auto* painelDireito = new QWidget(this);
    painelDireito->setMinimumWidth(240);
    auto* form = new QFormLayout(painelDireito);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_inputLarguraInferiorPainel = new QDoubleSpinBox(painelDireito);
    m_inputLarguraInferiorPainel->setRange(0.1, 10000.0);
    m_inputLarguraInferiorPainel->setDecimals(3);
    m_inputLarguraInferiorPainel->setSuffix(" m");

    m_inputAlturaMaximaPainel = new QDoubleSpinBox(painelDireito);
    m_inputAlturaMaximaPainel->setRange(0.1, 10000.0);
    m_inputAlturaMaximaPainel->setDecimals(3);
    m_inputAlturaMaximaPainel->setSuffix(" m");

    m_inputTaludePainel = new QDoubleSpinBox(painelDireito);
    m_inputTaludePainel->setRange(0.0, 100.0);
    m_inputTaludePainel->setDecimals(3);
    m_inputTaludePainel->setToolTip("Proporção do talude (H:V)");

    m_inputDeclividadePainel = new QDoubleSpinBox(painelDireito);
    m_inputDeclividadePainel->setRange(0.000001, 1.0);
    m_inputDeclividadePainel->setDecimals(6);
    m_inputDeclividadePainel->setSingleStep(0.0001);
    m_inputDeclividadePainel->setValue(0.001000);

    m_inputManningPainel = new QDoubleSpinBox(painelDireito);
    m_inputManningPainel->setRange(0.001, 1.0);
    m_inputManningPainel->setDecimals(4);
    m_inputManningPainel->setSingleStep(0.001);
    m_inputManningPainel->setValue(0.0150);

    m_inputVazaoCalculoPainel = new QDoubleSpinBox(painelDireito);
    m_inputVazaoCalculoPainel->setRange(0.0, 1000000000.0);
    m_inputVazaoCalculoPainel->setDecimals(6);
    m_inputVazaoCalculoPainel->setSuffix(" m³/s");

    m_botaoCalcularVazao = new QPushButton("Calcular vazão", painelDireito);

    form->addRow("Largura inferior (b)", m_inputLarguraInferiorPainel);
    form->addRow("Altura máxima (h)", m_inputAlturaMaximaPainel);
    form->addRow("Talude (z)", m_inputTaludePainel);
    form->addRow("Declividade (S)", m_inputDeclividadePainel);
    form->addRow("Manning (n)", m_inputManningPainel);
    form->addRow("Vazão de cálculo", m_inputVazaoCalculoPainel);
    form->addRow(m_botaoCalcularVazao);

    m_inputLarguraInferiorDesenho = new QDoubleSpinBox(m_desenho);
    m_inputLarguraInferiorDesenho->setRange(0.1, 10000.0);
    m_inputLarguraInferiorDesenho->setDecimals(3);
    m_inputLarguraInferiorDesenho->setSuffix(" m");

    m_inputAlturaMaximaDesenho = new QDoubleSpinBox(m_desenho);
    m_inputAlturaMaximaDesenho->setRange(0.1, 10000.0);
    m_inputAlturaMaximaDesenho->setDecimals(3);
    m_inputAlturaMaximaDesenho->setSuffix(" m");

    m_inputTaludeDesenho = new QDoubleSpinBox(m_desenho);
    m_inputTaludeDesenho->setRange(0.0, 100.0);
    m_inputTaludeDesenho->setDecimals(3);

    m_desenho->setInputs(m_inputLarguraInferiorDesenho, m_inputAlturaMaximaDesenho, m_inputTaludeDesenho);

    layoutPrincipal->addWidget(m_desenho, 3);
    layoutPrincipal->addWidget(painelDireito, 1);

    conectarEventosSincronizacao();
    aplicarGeometria(4.0, 2.0, 1.0);
}

void CanalTrapezoidalWidget::setGeometria(double larguraInferior, double alturaMaxima, double taludeLateral)
{
    aplicarGeometria(larguraInferior, alturaMaxima, taludeLateral);
}

void CanalTrapezoidalWidget::aplicarGeometria(double larguraInferior, double alturaMaxima, double taludeLateral)
{
    const double b = std::max(0.1, larguraInferior);
    const double h = std::max(0.1, alturaMaxima);
    const double z = std::max(0.0, taludeLateral);

    {
        QSignalBlocker bl1(m_inputLarguraInferiorDesenho);
        QSignalBlocker bl2(m_inputAlturaMaximaDesenho);
        QSignalBlocker bl3(m_inputTaludeDesenho);
        QSignalBlocker bl4(m_inputLarguraInferiorPainel);
        QSignalBlocker bl5(m_inputAlturaMaximaPainel);
        QSignalBlocker bl6(m_inputTaludePainel);

        m_inputLarguraInferiorDesenho->setValue(b);
        m_inputAlturaMaximaDesenho->setValue(h);
        m_inputTaludeDesenho->setValue(z);

        m_inputLarguraInferiorPainel->setValue(b);
        m_inputAlturaMaximaPainel->setValue(h);
        m_inputTaludePainel->setValue(z);
    }

    atualizarDesenho();
}

void CanalTrapezoidalWidget::atualizarDesenho()
{
    m_desenho->setGeometria(
        m_inputLarguraInferiorDesenho->value(),
        m_inputAlturaMaximaDesenho->value(),
        m_inputTaludeDesenho->value());
}

void CanalTrapezoidalWidget::conectarEventosSincronizacao()
{
    connect(m_inputLarguraInferiorPainel, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
        aplicarGeometria(
            m_inputLarguraInferiorPainel->value(),
            m_inputAlturaMaximaPainel->value(),
            m_inputTaludePainel->value());
    });

    connect(m_inputAlturaMaximaPainel, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
        aplicarGeometria(
            m_inputLarguraInferiorPainel->value(),
            m_inputAlturaMaximaPainel->value(),
            m_inputTaludePainel->value());
    });

    connect(m_inputTaludePainel, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
        aplicarGeometria(
            m_inputLarguraInferiorPainel->value(),
            m_inputAlturaMaximaPainel->value(),
            m_inputTaludePainel->value());
    });

    connect(m_inputLarguraInferiorDesenho, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
        aplicarGeometria(
            m_inputLarguraInferiorDesenho->value(),
            m_inputAlturaMaximaDesenho->value(),
            m_inputTaludeDesenho->value());
    });

    connect(m_inputAlturaMaximaDesenho, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
        aplicarGeometria(
            m_inputLarguraInferiorDesenho->value(),
            m_inputAlturaMaximaDesenho->value(),
            m_inputTaludeDesenho->value());
    });

    connect(m_inputTaludeDesenho, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
        aplicarGeometria(
            m_inputLarguraInferiorDesenho->value(),
            m_inputAlturaMaximaDesenho->value(),
            m_inputTaludeDesenho->value());
    });

    connect(m_botaoCalcularVazao, &QPushButton::clicked, this, [this]() {
        calcularVazaoPainel();
    });
}

void CanalTrapezoidalWidget::calcularVazaoPainel()
{
    const double b = m_inputLarguraInferiorPainel->value();
    const double h = m_inputAlturaMaximaPainel->value();
    const double z = m_inputTaludePainel->value();
    const double s = m_inputDeclividadePainel->value();
    const double n = m_inputManningPainel->value();

    const SecaoTransversalTrapezoidal secao(b, z);
    Canal canal;
    canal.setSecaoTransversal(secao);
    canal.setDeclividadeFinal(s);
    canal.setCoeficienteManning(n);

    const double vazao = canal.vazaoManning(h);
    m_inputVazaoCalculoPainel->setValue(std::max(0.0, vazao));
}
