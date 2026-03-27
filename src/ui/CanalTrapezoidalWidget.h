#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QPushButton;
class CanalTrapezoidalDesenhoWidget;

/**
 * @brief Widget de apoio para visualizar a geometria trapezoidal de um canal.
 *
 * O componente sincroniza entradas numéricas com um desenho esquemático e
 * permite estimar a vazão pela fórmula de Manning.
 */
class CanalTrapezoidalWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Cria o widget com painel de parâmetros e área de desenho.
     * @param parent Widget pai opcional na hierarquia Qt.
     */
    explicit CanalTrapezoidalWidget(QWidget* parent = nullptr);

    /**
     * @brief Define a geometria exibida e sincroniza os controles da interface.
     * @param larguraInferior Largura de fundo do canal em metros.
     * @param alturaMaxima Altura total considerada para o perfil em metros.
     * @param taludeLateral Inclinação lateral do talude na relação H:V.
     */
    void setGeometria(double larguraInferior, double alturaMaxima, double taludeLateral);

private:
    /**
     * @brief Aplica uma geometria nos campos de edição e no desenho.
     * @param larguraInferior Largura de fundo do canal em metros.
     * @param alturaMaxima Altura total considerada para o perfil em metros.
     * @param taludeLateral Inclinação lateral do talude na relação H:V.
     */
    void aplicarGeometria(double larguraInferior, double alturaMaxima, double taludeLateral);

    /**
     * @brief Redesenha o perfil geométrico com os valores atuais.
     */
    void atualizarDesenho();

    /**
     * @brief Conecta os sinais dos campos para manter desenho e painel sincronizados.
     */
    void conectarEventosSincronizacao();

    /**
     * @brief Calcula a vazão de Manning a partir dos parâmetros informados no painel.
     */
    void calcularVazaoPainel();

private:
    /** @brief Entrada de largura inferior posicionada diretamente sobre o desenho. */
    QDoubleSpinBox* m_inputLarguraInferiorDesenho = nullptr;

    /** @brief Entrada de altura máxima posicionada diretamente sobre o desenho. */
    QDoubleSpinBox* m_inputAlturaMaximaDesenho = nullptr;

    /** @brief Entrada de talude lateral posicionada diretamente sobre o desenho. */
    QDoubleSpinBox* m_inputTaludeDesenho = nullptr;

    /** @brief Campo de largura inferior exibido no painel lateral de propriedades. */
    QDoubleSpinBox* m_inputLarguraInferiorPainel = nullptr;

    /** @brief Campo de altura máxima exibido no painel lateral de propriedades. */
    QDoubleSpinBox* m_inputAlturaMaximaPainel = nullptr;

    /** @brief Campo de talude exibido no painel lateral de propriedades. */
    QDoubleSpinBox* m_inputTaludePainel = nullptr;

    /** @brief Campo de declividade usado no cálculo hidráulico simplificado. */
    QDoubleSpinBox* m_inputDeclividadePainel = nullptr;

    /** @brief Campo do coeficiente de Manning usado no cálculo hidráulico simplificado. */
    QDoubleSpinBox* m_inputManningPainel = nullptr;

    /** @brief Campo de saída com a vazão calculada pelo painel auxiliar. */
    QDoubleSpinBox* m_inputVazaoCalculoPainel = nullptr;

    /** @brief Botão que dispara o cálculo de vazão com os valores atuais. */
    QPushButton* m_botaoCalcularVazao = nullptr;

    /** @brief Área gráfica que representa o perfil trapezoidal do canal. */
    CanalTrapezoidalDesenhoWidget* m_desenho = nullptr;
};
