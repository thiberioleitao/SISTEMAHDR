#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QPushButton;
class CanalTrapezoidalDesenhoWidget;

class CanalTrapezoidalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanalTrapezoidalWidget(QWidget* parent = nullptr);

    void setGeometria(double larguraInferior, double alturaMaxima, double taludeLateral);

private:
    void aplicarGeometria(double larguraInferior, double alturaMaxima, double taludeLateral);
    void atualizarDesenho();
    void conectarEventosSincronizacao();
    void calcularVazaoPainel();

private:
    QDoubleSpinBox* m_inputLarguraInferiorDesenho = nullptr;
    QDoubleSpinBox* m_inputAlturaMaximaDesenho = nullptr;
    QDoubleSpinBox* m_inputTaludeDesenho = nullptr;

    QDoubleSpinBox* m_inputLarguraInferiorPainel = nullptr;
    QDoubleSpinBox* m_inputAlturaMaximaPainel = nullptr;
    QDoubleSpinBox* m_inputTaludePainel = nullptr;
    QDoubleSpinBox* m_inputDeclividadePainel = nullptr;
    QDoubleSpinBox* m_inputManningPainel = nullptr;
    QDoubleSpinBox* m_inputVazaoCalculoPainel = nullptr;
    QPushButton* m_botaoCalcularVazao = nullptr;

    CanalTrapezoidalDesenhoWidget* m_desenho = nullptr;
};
