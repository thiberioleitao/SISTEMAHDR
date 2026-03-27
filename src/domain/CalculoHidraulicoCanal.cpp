#include "CalculoHidraulicoCanal.h"

#include <algorithm>
#include <cmath>

namespace
{
/**
 * @brief Calcula a razao de uso entre um valor atual e um limite positivo.
 * @param valorAtual Valor efetivamente utilizado.
 * @param limite Valor maximo ou minimo de referencia.
 * @return Razao normalizada usada para detectar proximidade do limite.
 */
double calcularRazaoUso(double valorAtual, double limite)
{
    const double limiteSeguro = std::max(0.0, limite);
    if (limiteSeguro <= 0.0) return 0.0;
    return std::max(0.0, valorAtual) / limiteSeguro;
}
}

ResultadoVerificacaoCanal CalculoHidraulicoCanal::verificarCriterios(const EntradaVerificacaoCanal& entrada)
{
    ResultadoVerificacaoCanal resultado;

    // A capacidade deve suportar a vazao de projeto considerando a tolerancia adotada.
    const double qProjeto = std::max(0.0, entrada.qpHidrologiaM3s);
    const double qCapacidade = std::max(0.0, entrada.qpHidraulicaSMaxM3s);
    const double toleranciaCapacidade = qProjeto * std::max(0.0, entrada.toleranciaRelativaQp);
    resultado.criterioCapacidadeVazao = qCapacidade + toleranciaCapacidade >= qProjeto;

    // A folga disponivel em Smin deve ser maior ou igual ao criterio da secao.
    const double folgaMinima = std::max(0.0, entrada.folgaMinimaM);
    const double folgaDisponivel = std::max(0.0, entrada.folgaDisponivelSMinM);
    resultado.criterioFolga = folgaDisponivel >= folgaMinima;

    // A velocidade em Smax deve respeitar o limite do revestimento escolhido.
    const double vMaxAdmissivel = std::max(0.0, entrada.velocidadeMaximaAdmissivelMps);
    const double velocidadeCalculada = std::max(0.0, entrada.velocidadeSMaxMps);
    resultado.criterioVelocidade = velocidadeCalculada <= vMaxAdmissivel;

    if (!resultado.todosAtendidos()) {
        resultado.situacaoVisual = SituacaoVisualResultadoCanal::NaoAtende;

        QStringList motivos;
        if (!resultado.criterioCapacidadeVazao) {
            motivos << "capacidade hidraulica insuficiente para a vazao de projeto";
        }
        if (!resultado.criterioFolga) {
            motivos << "folga minima da secao nao atendida";
        }
        if (!resultado.criterioVelocidade) {
            motivos << "velocidade acima do limite do revestimento";
        }
        resultado.detalheVisual = motivos.join("; ");
        return resultado;
    }

    // Azul significa margem de seguranca menor ou igual a 10% em qualquer criterio avaliado.
    const double margemProximidade = std::clamp(entrada.margemProximidadeRelativa, 0.0, 1.0);
    const double limiteSuperiorUso = 1.0 - margemProximidade;

    const double usoFolga = (folgaMinima > 0.0)
                                ? calcularRazaoUso(folgaMinima, std::max(folgaMinima, folgaDisponivel))
                                : 0.0;
    const double usoVelocidade = calcularRazaoUso(velocidadeCalculada, vMaxAdmissivel);
    const double usoCapacidade = (qCapacidade > 0.0)
                                     ? calcularRazaoUso(qProjeto, qCapacidade)
                                     : 0.0;

    const bool atendeNoLimite = usoFolga >= limiteSuperiorUso
        || usoVelocidade >= limiteSuperiorUso
        || usoCapacidade >= limiteSuperiorUso;

    if (atendeNoLimite) {
        resultado.situacaoVisual = SituacaoVisualResultadoCanal::AtendeNoLimite;

        QStringList motivos;
        if (usoFolga >= limiteSuperiorUso) {
            motivos << "folga disponivel esta proxima do minimo exigido";
        }
        if (usoVelocidade >= limiteSuperiorUso) {
            motivos << "velocidade calculada esta proxima do limite do revestimento";
        }
        if (usoCapacidade >= limiteSuperiorUso) {
            motivos << "capacidade hidraulica esta proxima da vazao de projeto";
        }
        resultado.detalheVisual = motivos.join("; ");
    }
    else {
        resultado.situacaoVisual = SituacaoVisualResultadoCanal::Atende;
        resultado.detalheVisual = "Criterios atendidos com margem de seguranca superior a 10%.";
    }

    return resultado;
}
