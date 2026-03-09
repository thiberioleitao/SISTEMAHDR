#include "Canal.h"

#include "RedeHidrologica.h"

#include <algorithm>

Canal::Canal(const QString& nome)
    : m_nome(nome)
{
}

Canal::Canal(const QString& id, const QString& nome, const QString& idJusante)
    : m_id(id)
    , m_idJusante(idJusante)
    , m_nome(nome)
{
}

const QString& Canal::id() const
{
    return m_id;
}

void Canal::setId(const QString& id)
{
    m_id = id;
}

const QString& Canal::idJusante() const
{
    return m_idJusante;
}

void Canal::setIdJusante(const QString& idJusante)
{
    m_idJusante = idJusante;
}

const QString& Canal::nome() const
{
    return m_nome;
}

void Canal::setNome(const QString& nome)
{
    m_nome = nome;
}

int Canal::quantidadeTrechos() const
{
    return m_trechos.size();
}

bool Canal::vazio() const
{
    return m_trechos.isEmpty();
}

const QVector<CanalTrecho>& Canal::trechos() const
{
    return m_trechos;
}

bool Canal::adicionarTrecho(const CanalTrecho& trecho, double)
{
    m_trechos.append(trecho);
    return true;
}

bool Canal::removerTrecho(int indice)
{
    if (indice < 0 || indice >= m_trechos.size()) return false;
    m_trechos.removeAt(indice);
    return true;
}

void Canal::limpar()
{
    m_trechos.clear();
}

double Canal::declividadeFundo() const
{
    return declividadeMedia();
}

double Canal::declividadeMedia() const
{
    if (m_trechos.isEmpty()) return 0.0;

    double soma = 0.0;
    for (const CanalTrecho& trecho : m_trechos) {
        soma += trecho.declividadeFundo();
    }

    return soma / static_cast<double>(m_trechos.size());
}

double Canal::comprimentoTotal() const
{
    double soma = 0.0;
    for (const CanalTrecho& trecho : m_trechos) {
        soma += trecho.comprimento();
    }
    return soma;
}

double Canal::vazaoMontante() const
{
    return m_vazaoMontante;
}

bool Canal::atualizarVazaoMontanteDaRede(const RedeHidrologica& rede,
                                         const QMap<QString, double>& contribuicaoPorElemento,
                                         QString* erro)
{
    if (erro) erro->clear();

    const QString idAtual = m_id.trimmed();
    if (idAtual.isEmpty()) {
        if (erro) *erro = "Canal sem ID: não é possível calcular vazão de montante.";
        return false;
    }

    QString erroRede;
    const QMap<QString, double> vazaoAcumulada = rede.calcularVazaoAcumuladaPorElemento(contribuicaoPorElemento, &erroRede);
    if (!erroRede.isEmpty()) {
        if (erro) *erro = erroRede;
        return false;
    }

    if (!vazaoAcumulada.contains(idAtual)) {
        if (erro) *erro = "ID do canal não encontrado na rede: " + idAtual;
        return false;
    }

    const double contribuicaoLocal = std::max(0.0, contribuicaoPorElemento.value(idAtual, 0.0));
    const double vazaoTotalNoCanal = std::max(0.0, vazaoAcumulada.value(idAtual, 0.0));

    m_vazaoMontante = std::max(0.0, vazaoTotalNoCanal - contribuicaoLocal);
    return true;
}

double Canal::vazaoContribuicao() const
{
    return m_vazaoContribuicao;
}

void Canal::setVazaoContribuicao(double valor)
{
    m_vazaoContribuicao = std::max(0.0, valor);
}

double Canal::vazaoAssociada() const
{
    return m_vazaoMontante + m_vazaoContribuicao;
}
