#pragma once

#include "CalculoHidraulicoCanal.h"
#include "RevestimentoCanal.h"
#include "SecaoTransversalTrapezoidal.h"

#include <QString>

class CanalTrecho
{
public:
    CanalTrecho() = default;

    CanalTrecho(const QString& nome,
                double comprimento,
                double larguraFundo,
                double alturaLaminaProjeto,
                double taludeLateral,
                double declividadeFundo,
                double coeficienteManning);

    CanalTrecho(const QString& nome,
                double comprimento,
                const SecaoTransversalTrapezoidal& secao,
                double declividadeFundo,
                const RevestimentoCanal& revestimento,
                double alturaLaminaProjeto = 0.0);

    const QString& nome() const;
    void setNome(const QString& nome);

    double comprimento() const;
    void setComprimento(double valor);

    double larguraFundo() const;
    void setLarguraFundo(double valor);

    double alturaLaminaProjeto() const;
    void setAlturaLaminaProjeto(double valor);

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

    double vazaoProjeto() const;

private:
    QString m_nome;
    double m_comprimento = 0.0;
    SecaoTransversalTrapezoidal m_secao;
    double m_alturaLaminaProjeto = 0.0;
    double m_declividadeFundo = 0.0;
    RevestimentoCanal m_revestimento;
};
