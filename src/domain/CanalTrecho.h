#pragma once

#include "RevestimentoCanal.h"
#include "SecaoTransversalTrapezoidal.h"

#include <QString>

class CanalTrecho
{
public:
    CanalTrecho() = default;

    CanalTrecho(const QString& nome,
                double comprimento,
                const SecaoTransversalTrapezoidal& secao,
                double declividadeFundo,
                const RevestimentoCanal& revestimento);

    const QString& nome() const;
    void setNome(const QString& nome);

    double comprimento() const;
    void setComprimento(double valor);

    double larguraFundo() const;
    void setLarguraFundo(double valor);

    double taludeLateral() const;
    void setTaludeLateral(double valor);

    double declividadeFundo() const;
    void setDeclividadeFundo(double valor);

    double coeficienteManning() const;
    void setCoeficienteManning(double valor);

    const SecaoTransversalTrapezoidal& secaoTransversal() const;
    void setSecaoTransversal(const SecaoTransversalTrapezoidal& secao);

    const RevestimentoCanal& revestimento() const;
    void setRevestimento(const RevestimentoCanal& revestimento);

    double larguraSuperficial(double alturaLamina) const;

    double areaMolhada(double alturaLamina) const;

    double perimetroMolhado(double alturaLamina) const;

    double raioHidraulico(double alturaLamina) const;

    double velocidadeManning(double alturaLamina) const;

    double vazaoManning(double alturaLamina) const;

    double alturaLaminaParaVazaoProjeto(double vazaoProjeto,
                                        double alturaMaximaBusca = 10.0,
                                        double tolerancia = 1e-6,
                                        int maxIteracoes = 100) const;

private:
    QString m_nome;
    double m_comprimento = 0.0;
    SecaoTransversalTrapezoidal m_secao;
    double m_declividadeFundo = 0.0;
    RevestimentoCanal m_revestimento;
};
