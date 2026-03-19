#pragma once

#include "RevestimentoCanal.h"
#include "SecaoTransversalSemicircular.h"
#include "SecaoTransversalTrapezoidal.h"

#include <QMap>
#include <QString>
#include <QVector>

enum class TipoSecaoCanal
{
    Trapezoidal,
    Semicircular
};

class Canal
{
public:

    Canal() = default;

    explicit Canal(const QString& id);

    const QString& idProprio() const;
    void setIdProprio(const QString& id);

    const QString& idJusante() const;
    void setIdJusante(const QString& idJusante);

    const QString& id() const;
    void setId(const QString& id);

    const QString& nome() const { return id(); }
    void setNome(const QString& nome) { setId(nome); }

    double comprimentoTotal() const;

    double comprimento() const;
    void setComprimento(double valor);

    double larguraFundo() const;
    void setLarguraFundo(double valor);

    double taludeLateral() const;
    void setTaludeLateral(double valor);

    double declividadeFinal() const;
    void setDeclividadeFinal(double valor);

    double declividadeMinima() const;
    void setDeclividadeMinima(double valor);

    double declividadeMaxima() const;
    void setDeclividadeMaxima(double valor);

    double coeficienteManning() const;
    void setCoeficienteManning(double valor);

    const SecaoTransversalTrapezoidal& secaoTransversal() const;
    void setSecaoTransversal(const SecaoTransversalTrapezoidal& secao);

    const SecaoTransversalSemicircular& secaoSemicircular() const;
    void setSecaoSemicircular(const SecaoTransversalSemicircular& secao);

    TipoSecaoCanal tipoSecao() const;
    void setTipoSecao(TipoSecaoCanal tipo);

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
    QString m_id;
    QString m_idJusante;
    
    double m_comprimento = 0.0;
    SecaoTransversalTrapezoidal m_secao;
    SecaoTransversalSemicircular m_secaoSemicircular;
    TipoSecaoCanal m_tipoSecao = TipoSecaoCanal::Trapezoidal;

    RevestimentoCanal m_revestimento;

    double m_declividadeFinal = 0.0;
    double m_declividadeMinima = 0.0;
    double m_declividadeMaxima = 0.0;

};
