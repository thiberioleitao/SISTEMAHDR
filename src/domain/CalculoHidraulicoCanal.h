#pragma once

#include <QString>
#include <QStringList>

/**
 * @brief Reune os valores usados na verificacao hidraulica e visual de um canal.
 */
struct EntradaVerificacaoCanal
{
    double qpHidrologiaM3s = 0.0;
    double qpHidraulicaSMaxM3s = 0.0;

    double alturaSMinM = 0.0;
    double alturaMaximaSecaoM = 0.0;
    double folgaMinimaM = 0.0;
    double folgaDisponivelSMinM = 0.0;

    double velocidadeSMaxMps = 0.0;
    double velocidadeMaximaAdmissivelMps = 0.0;

    double margemProximidadeRelativa = 0.10;
    double toleranciaRelativaQp = 0.02;
};

/**
 * @brief Representa a severidade visual consolidada para uma linha de resultados.
 */
enum class SituacaoVisualResultadoCanal
{
    Atende,
    AtendeNoLimite,
    NaoAtende
};

/**
 * @brief Guarda o resultado da verificacao hidraulica consolidada.
 */
struct ResultadoVerificacaoCanal
{
    bool criterioCapacidadeVazao = false;
    bool criterioFolga = false;
    bool criterioVelocidade = false;
    SituacaoVisualResultadoCanal situacaoVisual = SituacaoVisualResultadoCanal::Atende;
    QString detalheVisual;

    /**
     * @brief Indica se todos os criterios obrigatorios foram atendidos.
     * @return true quando o canal atende vazao, folga e velocidade.
     */
    bool todosAtendidos() const
    {
        return criterioCapacidadeVazao
            && criterioFolga
            && criterioVelocidade;
    }

    /**
     * @brief Traduz um criterio booleano para a notacao curta usada na tabela.
     * @param valor Resultado booleano do criterio.
     * @return Texto "OK" ou "NOK".
     */
    QString textoStatus(bool valor) const
    {
        return valor ? QString("OK") : QString("NOK");
    }
};

/**
 * @brief Centraliza as regras de verificacao hidraulica e de destaque visual.
 */
class CalculoHidraulicoCanal
{
public:
    /**
     * @brief Avalia os criterios hidraulicos e o status visual de uma linha de resultados.
     * @param entrada Valores calculados para o canal avaliado.
     * @return Estrutura com criterios individuais e situacao visual consolidada.
     */
    static ResultadoVerificacaoCanal verificarCriterios(const EntradaVerificacaoCanal& entrada);
};
