#include "RegistroElementosRede.h"

#include "RedeHidrologica.h"

#include <algorithm>

bool RegistroElementosRede::associarBaciaAoElemento(const RedeHidrologica& rede,
                                                    const QString& idElemento,
                                                    const BaciaContribuicao& bacia,
                                                    QString* erro)
{
    if (erro) erro->clear();

    const QString id = idElemento.trimmed();
    if (id.isEmpty()) {
        if (erro) *erro = "ID do elemento não informado para associação de bacia.";
        return false;
    }

    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.id == id) {
            if (e.tipo != TipoElementoRede::BaciaContribuicao) {
                if (erro) *erro = "Elemento não é do tipo BaciaContribuicao: " + id;
                return false;
            }

            m_baciasPorId.insert(id, bacia);
            return true;
        }
    }

    if (erro) *erro = "Elemento inexistente na rede para associação de bacia: " + id;
    return false;
}

const QMap<QString, BaciaContribuicao>& RegistroElementosRede::baciasPorId() const
{
    return m_baciasPorId;
}

const BaciaContribuicao* RegistroElementosRede::baciaPorId(const QString& idElemento) const
{
    const auto it = m_baciasPorId.constFind(idElemento);
    if (it == m_baciasPorId.cend()) return nullptr;
    return &it.value();
}

double RegistroElementosRede::contribuicaoBaciasParaElemento(const RedeHidrologica& rede,
                                                             const QString& idElemento,
                                                             double intensidadeChuvaBrutaMmH,
                                                             QString* erro) const
{
    if (erro) erro->clear();

    const QString idAlvo = idElemento.trimmed();
    if (idAlvo.isEmpty()) {
        if (erro) *erro = "ID do elemento não informado para cálculo de contribuição de bacias.";
        return 0.0;
    }

    double soma = 0.0;
    for (const ElementoRedeHidrologica& e : rede.elementos()) {
        if (e.tipo == TipoElementoRede::BaciaContribuicao && e.idJusante == idAlvo) {
            if (const BaciaContribuicao* b = baciaPorId(e.id)) {
                soma += std::max(0.0, b->calcularVazaoProjetoMetodoRacional(intensidadeChuvaBrutaMmH));
            }
        }
    }

    return std::max(0.0, soma);
}

void RegistroElementosRede::limpar()
{
    m_baciasPorId.clear();
}
