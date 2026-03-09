#include "RedeHidrologica.h"

#include <QQueue>

#include <algorithm>

bool RedeHidrologica::existeId(const QString& id) const
{
    const QString alvo = id.trimmed();
    if (alvo.isEmpty()) return false;

    for (const ElementoRedeHidrologica& e : m_elementos) {
        if (e.id == alvo) return true;
    }

    return false;
}

QString RedeHidrologica::gerarProximoId(const QString& prefixo) const
{
    QString base = prefixo.trimmed();
    if (base.isEmpty()) base = "E";

    int numero = 1;
    while (true) {
        const QString candidato = base + QString::number(numero);
        if (!existeId(candidato)) {
            return candidato;
        }
        ++numero;
    }
}

bool RedeHidrologica::adicionarElemento(const QString& id,
                                        const QString& idJusante,
                                        QString* erro)
{
    const QString elemId = id.trimmed();
    const QString jus = idJusante.trimmed();

    if (elemId.isEmpty()) {
        if (erro) *erro = "ID do elemento deve ser informado.";
        return false;
    }

    if (!jus.isEmpty() && elemId == jus) {
        if (erro) *erro = "ID do elemento e ID jusante não podem ser iguais.";
        return false;
    }

    for (const ElementoRedeHidrologica& e : m_elementos) {
        if (e.id == elemId) {
            if (erro) *erro = "Já existe elemento com o mesmo ID: " + elemId;
            return false;
        }
    }

    m_elementos.append({ elemId, jus });
    return true;
}

QString RedeHidrologica::adicionarElementoComIdAutomatico(const QString& idJusante,
                                                          const QString& prefixo,
                                                          QString* erro)
{
    const QString idGerado = gerarProximoId(prefixo);

    if (!adicionarElemento(idGerado, idJusante, erro)) {
        return QString();
    }

    return idGerado;
}

const QVector<ElementoRedeHidrologica>& RedeHidrologica::elementos() const
{
    return m_elementos;
}

QMap<QString, double> RedeHidrologica::calcularVazaoAcumuladaPorElemento(
    const QMap<QString, double>& contribuicaoPorElemento,
    QString* erro) const
{
    QMap<QString, double> vazaoAcumuladaPorElemento;
    if (erro) erro->clear();

    if (m_elementos.isEmpty()) {
        return vazaoAcumuladaPorElemento;
    }

    QMap<QString, int> indicePorId;
    QMap<QString, int> grauEntrada;
    QMap<QString, double> vazaoEntradaAcumulada;

    for (int i = 0; i < m_elementos.size(); ++i) {
        const ElementoRedeHidrologica& e = m_elementos.at(i);
        indicePorId.insert(e.id, i);
        grauEntrada.insert(e.id, 0);
        vazaoEntradaAcumulada.insert(e.id, 0.0);
        vazaoAcumuladaPorElemento.insert(e.id, 0.0);
    }

    for (const ElementoRedeHidrologica& e : m_elementos) {
        if (e.idJusante.isEmpty()) continue; // exutório da rede

        if (!indicePorId.contains(e.idJusante)) {
            if (erro) *erro = "ID jusante inexistente na rede: " + e.idJusante;
            return QMap<QString, double>();
        }

        grauEntrada[e.idJusante] += 1;
    }

    QQueue<QString> fila;
    for (auto it = grauEntrada.cbegin(); it != grauEntrada.cend(); ++it) {
        if (it.value() == 0) fila.enqueue(it.key());
    }

    int elementosProcessados = 0;

    while (!fila.isEmpty()) {
        const QString idAtual = fila.dequeue();
        const ElementoRedeHidrologica& eAtual = m_elementos.at(indicePorId.value(idAtual));

        const double contribuicaoLocal = std::max(0.0, contribuicaoPorElemento.value(idAtual, 0.0));
        const double vazaoSaida = vazaoEntradaAcumulada.value(idAtual) + contribuicaoLocal;
        vazaoAcumuladaPorElemento[idAtual] = vazaoSaida;

        if (!eAtual.idJusante.isEmpty()) {
            vazaoEntradaAcumulada[eAtual.idJusante] = vazaoEntradaAcumulada.value(eAtual.idJusante) + vazaoSaida;

            grauEntrada[eAtual.idJusante] -= 1;
            if (grauEntrada.value(eAtual.idJusante) == 0) {
                fila.enqueue(eAtual.idJusante);
            }
        }

        elementosProcessados += 1;
    }

    if (elementosProcessados != m_elementos.size()) {
        if (erro) *erro = "A rede possui ciclo ou estrutura inválida para acumulação de vazão.";
        return QMap<QString, double>();
    }

    return vazaoAcumuladaPorElemento;
}

void RedeHidrologica::limpar()
{
    m_elementos.clear();
}
