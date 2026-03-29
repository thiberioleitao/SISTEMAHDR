#include "RevestimentoCanal.h"

#include <algorithm>

/**
 * @brief Monta o catalogo padrao de revestimentos reutilizado em cadastro e verificacoes.
 * @return Vetor com os revestimentos conhecidos do dominio.
 */
static QVector<RevestimentoCanal> criarRevestimentosPadrao()
{
    return {
        RevestimentoCanal("Grama", 0.030, 1.50, 60.0),
        RevestimentoCanal("Concreto", 0.015, 5.00, 300.0),
        RevestimentoCanal("Enrocamento", 0.035, 4.00, 250.0),
        RevestimentoCanal("Solo exposto", 0.025, 1.20, 25.0),
        RevestimentoCanal("Gabião", 0.028, 3.50, 180.0)
    };
}

RevestimentoCanal::RevestimentoCanal(const QString& nome,
                                     double coeficienteManning,
                                     double velocidadeMaximaAdmissivelMps,
                                     double tensaoCisalhamentoMaximaAdmissivelPa)
    : m_nome(nome.trimmed())
    , m_coeficienteManning(std::max(0.0, coeficienteManning))
    , m_velocidadeMaximaAdmissivelMps(std::max(0.0, velocidadeMaximaAdmissivelMps))
    , m_tensaoCisalhamentoMaximaAdmissivelPa(std::max(0.0, tensaoCisalhamentoMaximaAdmissivelPa))
{
}

RevestimentoCanal::RevestimentoCanal(const QString& material,
                                     double coeficienteManning,
                                     double espessura)
    : m_nome(material.trimmed())
    , m_coeficienteManning(std::max(0.0, coeficienteManning))
    , m_espessura(std::max(0.0, espessura))
{
}

const QString& RevestimentoCanal::nome() const
{
    return m_nome;
}

void RevestimentoCanal::setNome(const QString& valor)
{
    m_nome = valor.trimmed();
}

const QString& RevestimentoCanal::material() const
{
    return nome();
}

void RevestimentoCanal::setMaterial(const QString& valor)
{
    setNome(valor);
}

double RevestimentoCanal::coeficienteManning() const
{
    return m_coeficienteManning;
}

void RevestimentoCanal::setCoeficienteManning(double valor)
{
    m_coeficienteManning = std::max(0.0, valor);
}

double RevestimentoCanal::velocidadeMaximaAdmissivelMps() const
{
    return m_velocidadeMaximaAdmissivelMps;
}

void RevestimentoCanal::setVelocidadeMaximaAdmissivelMps(double valor)
{
    m_velocidadeMaximaAdmissivelMps = std::max(0.0, valor);
}

double RevestimentoCanal::tensaoCisalhamentoMaximaAdmissivelPa() const
{
    return m_tensaoCisalhamentoMaximaAdmissivelPa;
}

void RevestimentoCanal::setTensaoCisalhamentoMaximaAdmissivelPa(double valor)
{
    m_tensaoCisalhamentoMaximaAdmissivelPa = std::max(0.0, valor);
}

double RevestimentoCanal::espessura() const
{
    return m_espessura;
}

void RevestimentoCanal::setEspessura(double valor)
{
    m_espessura = std::max(0.0, valor);
}

bool RevestimentoCanal::ehValido() const
{
    return !m_nome.trimmed().isEmpty();
}

QVector<RevestimentoCanal> RevestimentoCanal::revestimentosPadrao()
{
    return criarRevestimentosPadrao();
}

QStringList RevestimentoCanal::nomesPadrao()
{
    QStringList nomes;
    const QVector<RevestimentoCanal> revestimentos = revestimentosPadrao();
    nomes.reserve(revestimentos.size());
    for (const RevestimentoCanal& revestimento : revestimentos) {
        nomes.append(revestimento.nome());
    }
    return nomes;
}

RevestimentoCanal RevestimentoCanal::revestimentoPorNome(const QString& nome)
{
    const QString nomeNormalizado = nome.trimmed();
    for (const RevestimentoCanal& revestimento : revestimentosPadrao()) {
        if (revestimento.nome().compare(nomeNormalizado, Qt::CaseInsensitive) == 0) {
            return revestimento;
        }
    }

    if (!nomeNormalizado.isEmpty()) {
        RevestimentoCanal personalizado = revestimentoPadraoSeguro();
        personalizado.setNome(nomeNormalizado);
        return personalizado;
    }

    return revestimentoPadraoSeguro();
}

RevestimentoCanal RevestimentoCanal::revestimentoPadraoSeguro()
{
    return RevestimentoCanal("Concreto", 0.015, 5.00, 300.0);
}
