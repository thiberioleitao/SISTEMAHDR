#include "Canal.h"

Canal::Canal(const QString& nome)
    : m_nome(nome)
{
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
