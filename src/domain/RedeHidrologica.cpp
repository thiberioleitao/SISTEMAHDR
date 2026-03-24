#include "RedeHidrologica.h"
#include "HidrologiaUtils.h"

#include <QQueue>
#include <QSet>
#include <QDebug>

#include <algorithm>
#include <cmath>

bool RedeHidrologica::existeId(const QString& id) const
{
    return m_tipoPorId.contains(id.trimmed());
}

bool RedeHidrologica::registrarNo(const QString& id,
                                  const QString& idJusante,
                                  TipoElementoRede tipo)
{
    const QString idElemento = id.trimmed();
    const QString idJusanteNormalizado = idJusante.trimmed();

    if (idElemento.isEmpty()) return false;
    if (!idJusanteNormalizado.isEmpty() && idElemento == idJusanteNormalizado) return false;
    if (existeId(idElemento)) return false;
    if (!idJusanteNormalizado.isEmpty() && !existeId(idJusanteNormalizado)) return false;

    m_idsElementos.append(idElemento);
    m_idJusantePorId.insert(idElemento, idJusanteNormalizado);
    m_idsMontanteDiretoPorId.insert(idElemento, {});

    if (!idJusanteNormalizado.isEmpty()) {
        if (!m_idsMontanteDiretoPorId.contains(idJusanteNormalizado)) {
            m_idsMontanteDiretoPorId.insert(idJusanteNormalizado, {});
        }
        m_idsMontanteDiretoPorId[idJusanteNormalizado].append(idElemento);
    }

    m_tipoPorId.insert(idElemento, tipo);
    return true;
}


bool RedeHidrologica::adicionarCanal(const Canal& canal)
{
    const QString idElemento = canal.nome().trimmed();
    if (idElemento.isEmpty()) return false;

    if (!registrarNo(idElemento, canal.idJusante(), TipoElementoRede::Canal)) return false;

    Canal canalVinculado = canal;
    canalVinculado.setIdProprio(idElemento);
    canalVinculado.setIdJusante(canal.idJusante());
    m_canaisPorId.insert(idElemento, canalVinculado);
    return true;
}

bool RedeHidrologica::adicionarBacia(const QString& idElemento,
                                     const BaciaContribuicao& bacia,
                                     const QString& idJusante)
{
    const QString id = idElemento.trimmed();
    if (id.isEmpty()) return false;

    if (!registrarNo(id, idJusante, TipoElementoRede::BaciaContribuicao)) return false;

    BaciaContribuicao baciaVinculada = bacia;
    baciaVinculada.setIdProprio(id);
    baciaVinculada.setIdJusante(idJusante);
    m_baciasPorId.insert(id, baciaVinculada);
    return true;
}

bool RedeHidrologica::adicionarBacia(const BaciaContribuicao& bacia)
{
    const QString idElemento = bacia.nome().trimmed();
    if (idElemento.isEmpty()) return false;

    if (!registrarNo(idElemento, bacia.idJusante(), TipoElementoRede::BaciaContribuicao)) return false;

    BaciaContribuicao baciaVinculada = bacia;
    baciaVinculada.setIdProprio(idElemento);
    baciaVinculada.setIdJusante(bacia.idJusante());
    m_baciasPorId.insert(idElemento, baciaVinculada);
    return true;
}

bool RedeHidrologica::adicionarBueiro(const Bueiro& bueiro)
{
    const QString idElemento = bueiro.nome().trimmed();
    if (idElemento.isEmpty()) return false;

    if (!registrarNo(idElemento, bueiro.idJusante(), TipoElementoRede::Bueiro)) return false;

    Bueiro bueiroVinculado = bueiro;
    bueiroVinculado.setIdProprio(idElemento);
    bueiroVinculado.setIdJusante(bueiro.idJusante());
    m_bueirosPorId.insert(idElemento, bueiroVinculado);
    return true;
}

bool RedeHidrologica::adicionarExutorio(const QString& id,
                                        const QString& idJusante)
{
    return registrarNo(id, idJusante, TipoElementoRede::Exutorio);
}

QVector<QString> RedeHidrologica::ordemTopologica() const
{
    const QVector<QString> ids = idsElementos();
    if (ids.isEmpty()) return {};

    QMap<QString, QString> jusantePorId;
    QMap<QString, int> grauEntrada;

    for (const QString& id : ids) {
        jusantePorId[id] = idJusanteDoElemento(id);
        grauEntrada[id] = 0;
    }

    for (const QString& id : ids) {
        const QString idJusante = jusantePorId.value(id);
        if (idJusante.isEmpty()) continue;
        if (!grauEntrada.contains(idJusante)) return {};
        grauEntrada[idJusante] += 1;
    }

    QQueue<QString> fila;
    for (auto it = grauEntrada.cbegin(); it != grauEntrada.cend(); ++it) {
        if (it.value() == 0) fila.enqueue(it.key());
    }

    QVector<QString> ordem;
    while (!fila.isEmpty()) {
        const QString idAtual = fila.dequeue();
        ordem.append(idAtual);

        const QString idJusante = jusantePorId.value(idAtual);
        if (idJusante.isEmpty()) continue;

        grauEntrada[idJusante] -= 1;
        if (grauEntrada.value(idJusante) == 0) fila.enqueue(idJusante);
    }

    if (ordem.size() != ids.size()) return {};
    return ordem;
}

bool RedeHidrologica::definirJusanteElemento(const QString& idElemento,
                                             const QString& idJusante)
{
    const QString idMontante = idElemento.trimmed();
    const QString idJusanteNormalizado = idJusante.trimmed();

    if (idMontante.isEmpty()) return false;
    if (!existeId(idMontante)) return false;

    if (!idJusanteNormalizado.isEmpty() && idMontante == idJusanteNormalizado) return false;
    if (!idJusanteNormalizado.isEmpty() && !existeId(idJusanteNormalizado)) return false;

    // Evita ciclo indireto: o novo jusante não pode alcançar o próprio montante.
    if (!idJusanteNormalizado.isEmpty()) {
        QSet<QString> idsVisitados;
        QQueue<QString> fila;
        fila.enqueue(idJusanteNormalizado);
        idsVisitados.insert(idJusanteNormalizado);

        while (!fila.isEmpty()) {
            const QString idAtual = fila.dequeue();
            if (idAtual == idMontante) return false;

            const QString proximoJusante = m_idJusantePorId.value(idAtual);
            if (proximoJusante.isEmpty()) continue;

            if (!idsVisitados.contains(proximoJusante)) {
                idsVisitados.insert(proximoJusante);
                fila.enqueue(proximoJusante);
            }
        }
    }

    const QString idJusanteAnterior = m_idJusantePorId.value(idMontante);

    if (idJusanteAnterior == idJusanteNormalizado) return true;

    if (!idJusanteAnterior.isEmpty() && m_idsMontanteDiretoPorId.contains(idJusanteAnterior)) {
        m_idsMontanteDiretoPorId[idJusanteAnterior].removeAll(idMontante);
    }

    m_idJusantePorId[idMontante] = idJusanteNormalizado;

    if (!idJusanteNormalizado.isEmpty()) {
        if (!m_idsMontanteDiretoPorId.contains(idJusanteNormalizado)) {
            m_idsMontanteDiretoPorId.insert(idJusanteNormalizado, {});
        }

        QVector<QString>& idsMontante = m_idsMontanteDiretoPorId[idJusanteNormalizado];
        if (!idsMontante.contains(idMontante)) {
            idsMontante.append(idMontante);
        }
    }

    if (Canal* canal = canalPorId(idMontante)) {
        canal->setIdJusante(idJusanteNormalizado);
    }
    else if (BaciaContribuicao* bacia = baciaPorId(idMontante)) {
        bacia->setIdJusante(idJusanteNormalizado);
    }
    else if (Bueiro* bueiro = bueiroPorId(idMontante)) {
        bueiro->setIdJusante(idJusanteNormalizado);
    }

    return true;
}

double RedeHidrologica::intensidadeChuvaProjetoMmH() const
{
    return std::max(0.0, m_intensidadeChuvaProjetoMmH);
}

void RedeHidrologica::setIntensidadeChuvaProjetoMmH(double intensidadeChuvaProjetoMmH)
{
    m_intensidadeChuvaProjetoMmH = std::max(0.0, intensidadeChuvaProjetoMmH);
}

double RedeHidrologica::tempoConcentracaoMinimoMin() const
{
    return std::max(0.0, m_tempoConcentracaoMinimoMin);
}

void RedeHidrologica::setTempoConcentracaoMinimoMin(double tempoConcentracaoMinimoMin)
{
    m_tempoConcentracaoMinimoMin = std::max(0.0, tempoConcentracaoMinimoMin);
}

const QMap<QString, BaciaContribuicao>& RedeHidrologica::baciasPorId() const
{
    return m_baciasPorId;
}

const QMap<QString, Bueiro>& RedeHidrologica::bueirosPorId() const
{
    return m_bueirosPorId;
}

const QMap<QString, Canal>& RedeHidrologica::canaisPorId() const
{
    return m_canaisPorId;
}

const BaciaContribuicao* RedeHidrologica::baciaPorId(const QString& idElemento) const
{
    const auto it = m_baciasPorId.constFind(idElemento.trimmed());
    if (it == m_baciasPorId.cend()) return nullptr;
    return &it.value();
}

BaciaContribuicao* RedeHidrologica::baciaPorId(const QString& idElemento)
{
    const auto it = m_baciasPorId.find(idElemento.trimmed());
    if (it == m_baciasPorId.end()) return nullptr;
    return &it.value();
}

const Bueiro* RedeHidrologica::bueiroPorId(const QString& idElemento) const
{
    const auto it = m_bueirosPorId.constFind(idElemento.trimmed());
    if (it == m_bueirosPorId.cend()) return nullptr;
    return &it.value();
}

Bueiro* RedeHidrologica::bueiroPorId(const QString& idElemento)
{
    const auto it = m_bueirosPorId.find(idElemento.trimmed());
    if (it == m_bueirosPorId.end()) return nullptr;
    return &it.value();
}

const Canal* RedeHidrologica::canalPorId(const QString& idElemento) const
{
    const auto it = m_canaisPorId.constFind(idElemento.trimmed());
    if (it == m_canaisPorId.cend()) return nullptr;
    return &it.value();
}

Canal* RedeHidrologica::canalPorId(const QString& idElemento)
{
    const auto it = m_canaisPorId.find(idElemento.trimmed());
    if (it == m_canaisPorId.end()) return nullptr;
    return &it.value();
}

double RedeHidrologica::contribuicaoBaciasParaElemento(const QString& idElemento,
                                                       double intensidadeChuvaBrutaMmH) const
{
    const QString idAlvo = idElemento.trimmed();
    if (idAlvo.isEmpty()) return 0.0;
    if (intensidadeChuvaBrutaMmH < 0.0) return 0.0;

    double soma = 0.0;
    for (const QString& id : m_idsElementos) {
        if (tipoDoElemento(id) == TipoElementoRede::BaciaContribuicao && idJusanteDoElemento(id) == idAlvo) {
            if (const BaciaContribuicao* bacia = baciaPorId(id)) {
                soma += std::max(0.0, bacia->calcularContribuicaoRacional(intensidadeChuvaBrutaMmH));
            }
        }
    }

    return std::max(0.0, soma);
}

double RedeHidrologica::calcularComprimentoTotalTalvegueAteElemento(const QString& idElemento) const
{
    double comprimentoTalvegueKm = 0.0;
    double declividadeEquivalente = 0.0;
    if (!calcularTalvegueCriticoAteElemento(idElemento, &comprimentoTalvegueKm, &declividadeEquivalente)) {
        return 0.0;
    }

    return std::max(0.0, comprimentoTalvegueKm);
}

bool RedeHidrologica::calcularTalvegueCriticoAteElemento(const QString& idElemento,
                                                         double* comprimentoTalvegueKm,
                                                         double* declividadeEquivalente) const
{
    if (!comprimentoTalvegueKm || !declividadeEquivalente) return false;

    const QString idAlvo = idElemento.trimmed();
    if (idAlvo.isEmpty()) return false;
    if (!existeId(idAlvo)) return false;

    const QVector<QString> ordem = ordemTopologica();
    if (ordem.isEmpty()) return false;

    QMap<QString, double> comprimentoTalvegueCriticoPorId;
    QMap<QString, double> declividadeEquivalenteCriticaPorId;

    for (const QString& idAtual : ordem) {
        const QVector<QString> idsMontanteDireto = m_idsMontanteDiretoPorId.value(idAtual);

        double maiorComprimentoMontante = 0.0;
        double declividadeDoMaiorMontante = 0.0;
        for (const QString& idMontante : idsMontanteDireto) {
            const double comprimentoMontante = comprimentoTalvegueCriticoPorId.value(idMontante, 0.0);
            if (comprimentoMontante >= maiorComprimentoMontante) {
                maiorComprimentoMontante = comprimentoMontante;
                declividadeDoMaiorMontante = declividadeEquivalenteCriticaPorId.value(idMontante, 0.0);
            }
        }

        double maiorComprimentoMontanteNaoBacia = 0.0;
        double declividadeDoMaiorMontanteNaoBacia = 0.0;
        for (const QString& idMontante : idsMontanteDireto) {
            if (tipoDoElemento(idMontante) == TipoElementoRede::BaciaContribuicao) continue;

            const double comprimentoMontante = comprimentoTalvegueCriticoPorId.value(idMontante, 0.0);
            if (comprimentoMontante >= maiorComprimentoMontanteNaoBacia) {
                maiorComprimentoMontanteNaoBacia = comprimentoMontante;
                declividadeDoMaiorMontanteNaoBacia = declividadeEquivalenteCriticaPorId.value(idMontante, 0.0);
            }
        }

        double comprimentoCriticoKm = maiorComprimentoMontante;
        double declividadeEquivalenteCritica = declividadeDoMaiorMontante;

        if (tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
            const BaciaContribuicao* bacia = baciaPorId(idAtual);
            if (!bacia) return false;

            const double comprimentoBaciaKm = std::max(0.0, bacia->comprimentoTalveguePrincipalKm());
            const double declividadeBacia = std::max(0.0, bacia->declividadeMedia());

            if (comprimentoBaciaKm >= comprimentoCriticoKm) {
                comprimentoCriticoKm = comprimentoBaciaKm;
                declividadeEquivalenteCritica = declividadeBacia;
            }
        }
        else if (tipoDoElemento(idAtual) == TipoElementoRede::Canal) {
            const Canal* canal = canalPorId(idAtual);
            if (!canal) return false;

            const double comprimentoCanalKm = std::max(0.0, canal->comprimento()) / 1000.0;
            const double declividadeCanal = std::max(0.0, (canal->declividadeMinima() + canal->declividadeMaxima()) * 0.5);

            const double comprimentoViaMontante = maiorComprimentoMontanteNaoBacia + comprimentoCanalKm;
            const double declividadeViaMontante = (comprimentoViaMontante > 0.0)
                                                    ? ((declividadeDoMaiorMontanteNaoBacia * maiorComprimentoMontanteNaoBacia)
                                                       + (declividadeCanal * comprimentoCanalKm))
                                                        / comprimentoViaMontante
                                                    : 0.0;

            double maiorComprimentoBaciaLocal = 0.0;
            double declividadeBaciaLocal = 0.0;
            for (const QString& idMontante : idsMontanteDireto) {
                if (tipoDoElemento(idMontante) != TipoElementoRede::BaciaContribuicao) continue;

                const BaciaContribuicao* baciaLocal = baciaPorId(idMontante);
                if (!baciaLocal) continue;

                const double comprimentoBaciaKm = std::max(0.0, baciaLocal->comprimentoTalveguePrincipalKm());
                if (comprimentoBaciaKm >= maiorComprimentoBaciaLocal) {
                    maiorComprimentoBaciaLocal = comprimentoBaciaKm;
                    declividadeBaciaLocal = std::max(0.0, baciaLocal->declividadeMedia());
                }
            }

            const double comprimentoViaBaciaLocal = maiorComprimentoBaciaLocal + (comprimentoCanalKm * 0.5);

            const double comprimentoBaseSeqViaBaciaLocal = maiorComprimentoBaciaLocal + comprimentoCanalKm;
            const double declividadeViaBaciaLocal = (comprimentoBaseSeqViaBaciaLocal > 0.0)
                                                      ? ((declividadeBaciaLocal * maiorComprimentoBaciaLocal)
                                                         + (declividadeCanal * comprimentoCanalKm))
                                                          / comprimentoBaseSeqViaBaciaLocal
                                                      : 0.0;

            if (comprimentoViaBaciaLocal > comprimentoViaMontante) {
                comprimentoCriticoKm = comprimentoViaBaciaLocal;
                declividadeEquivalenteCritica = declividadeViaBaciaLocal;
            }
            else {
                comprimentoCriticoKm = comprimentoViaMontante;
                declividadeEquivalenteCritica = declividadeViaMontante;
            }
        }

        comprimentoTalvegueCriticoPorId[idAtual] = std::max(0.0, comprimentoCriticoKm);
        declividadeEquivalenteCriticaPorId[idAtual] = std::max(0.0, declividadeEquivalenteCritica);
    }

    *comprimentoTalvegueKm = comprimentoTalvegueCriticoPorId.value(idAlvo, 0.0);
    *declividadeEquivalente = declividadeEquivalenteCriticaPorId.value(idAlvo, 0.0);
    return true;
}

QMap<QString, double> RedeHidrologica::calcularVazaoAcumuladaPorElemento(
    const QMap<QString, double>& contribuicaoPorElemento) const
{
    QMap<QString, double> vazaoAcumuladaPorElemento;

    const QVector<QString> ids = idsElementos();
    if (ids.isEmpty()) return vazaoAcumuladaPorElemento;

    const QVector<QString> ordem = ordemTopologica();
    if (ordem.isEmpty()) return QMap<QString, double>();

    QMap<QString, QString> jusantePorId;

    for (const QString& id : ids) {
        jusantePorId[id] = idJusanteDoElemento(id);
    }

    QMap<QString, double> vazaoEntradaAcumulada;
    for (const QString& id : ids) {
        vazaoEntradaAcumulada.insert(id, 0.0);
        vazaoAcumuladaPorElemento.insert(id, 0.0);
    }

    for (const QString& idAtual : ordem) {
        const double vazaoLocal = std::max(0.0, contribuicaoPorElemento.value(idAtual, 0.0));
        const double vazaoSaida = vazaoEntradaAcumulada.value(idAtual) + vazaoLocal;
        vazaoAcumuladaPorElemento[idAtual] = vazaoSaida;

        const QString idJusante = jusantePorId.value(idAtual);
        if (!idJusante.isEmpty()) {
            vazaoEntradaAcumulada[idJusante] = vazaoEntradaAcumulada.value(idJusante) + vazaoSaida;
        }
    }

    return vazaoAcumuladaPorElemento;
}

double RedeHidrologica::areaAcumuladaTotalContribuinte(const QString& idElemento) const
{
    const QString idAtual = idElemento.trimmed();
    if (idAtual.isEmpty()) return 0.0;
    if (!existeId(idAtual)) return 0.0;

    const QVector<QString> idsMontanteLista = idsMontanteDoElemento(idAtual);
    QSet<QString> idsMontante;
    for (const QString& id : idsMontanteLista) {
        idsMontante.insert(id);
    }

    // Para ponto de controle em bacia, inclui a própria bacia local.
    if (tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
        idsMontante.insert(idAtual);
    }

    double areaTotal = 0.0;
    for (const QString& id : idsElementos()) {
        if (tipoDoElemento(id) == TipoElementoRede::BaciaContribuicao && idsMontante.contains(id)) {
            if (const BaciaContribuicao* b = baciaPorId(id)) {
                areaTotal += std::max(0.0, b->areaKm2());
            }
        }
    }

    return areaTotal;
}

double RedeHidrologica::coeficienteEscoamentoMedioPonderado(const QString& idElemento) const
{
    const QString idAtual = idElemento.trimmed();
    if (idAtual.isEmpty()) return 0.0;
    if (!existeId(idAtual)) return 0.0;

    const QVector<QString> idsMontanteLista = idsMontanteDoElemento(idAtual);
    QSet<QString> idsMontante;
    for (const QString& id : idsMontanteLista) {
        idsMontante.insert(id);
    }

    // Para ponto de controle em bacia, inclui a própria bacia local.
    if (tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
        idsMontante.insert(idAtual);
    }

    double somaArea = 0.0;
    double somaCvezesArea = 0.0;

    for (const QString& id : idsElementos()) {
        if (tipoDoElemento(id) != TipoElementoRede::BaciaContribuicao || !idsMontante.contains(id)) continue;

        const BaciaContribuicao* bacia = baciaPorId(id);
        if (!bacia) continue;

        const double area = std::max(0.0, bacia->areaKm2());
        if (area <= 0.0) continue;

        const double c = std::max(0.0, bacia->C_10());
        somaArea += area;
        somaCvezesArea += c * area;
    }

    if (somaArea <= 0.0) return 0.0;
    return somaCvezesArea / somaArea;
}

double RedeHidrologica::tempoConcentracaoKirpichModificadoAreaTotal(const QString& idElemento) const
{
    const QString idAlvo = idElemento.trimmed();
    if (idAlvo.isEmpty()) return 0.0;
    if (!existeId(idAlvo)) return 0.0;

    const QVector<QString> ordem = ordemTopologica();
    if (ordem.isEmpty()) return 0.0;

    // Acumula comprimento crítico e declividade equivalente do caminho crítico.
    QMap<QString, double> comprimentoTalvegueCriticoPorId;
    QMap<QString, double> declividadeEquivalenteCriticaPorId;
    QMap<QString, double> tempoConcentracaoPorId;

    const auto textoTipo = [](TipoElementoRede tipo) {
        switch (tipo) {
        case TipoElementoRede::Canal: return QString("Canal");
        case TipoElementoRede::BaciaContribuicao: return QString("BaciaContribuicao");
        case TipoElementoRede::Bueiro: return QString("Bueiro");
        case TipoElementoRede::Exutorio: return QString("Exutorio");
        default: return QString("Outro");
        }
    };

    qDebug() << "[TC][INICIO]" << "idAlvo=" << idAlvo;

    for (const QString& idAtual : ordem) {
        const QVector<QString> idsMontanteDireto = m_idsMontanteDiretoPorId.value(idAtual);

        double maiorComprimentoMontante = 0.0;
        double declividadeDoMaiorMontante = 0.0;
        for (const QString& idMontante : idsMontanteDireto) {
            const double comprimentoMontante = comprimentoTalvegueCriticoPorId.value(idMontante, 0.0);
            if (comprimentoMontante >= maiorComprimentoMontante) {
                maiorComprimentoMontante = comprimentoMontante;
                declividadeDoMaiorMontante = declividadeEquivalenteCriticaPorId.value(idMontante, 0.0);
            }
        }

        // Para o caminho Lmontante+Lcanal, desconsidera bacias locais ligadas diretamente ao canal.
        double maiorComprimentoMontanteNaoBacia = 0.0;
        double declividadeDoMaiorMontanteNaoBacia = 0.0;
        for (const QString& idMontante : idsMontanteDireto) {
            if (tipoDoElemento(idMontante) == TipoElementoRede::BaciaContribuicao) continue;

            const double comprimentoMontante = comprimentoTalvegueCriticoPorId.value(idMontante, 0.0);
            if (comprimentoMontante >= maiorComprimentoMontanteNaoBacia) {
                maiorComprimentoMontanteNaoBacia = comprimentoMontante;
                declividadeDoMaiorMontanteNaoBacia = declividadeEquivalenteCriticaPorId.value(idMontante, 0.0);
            }
        }

        qDebug() << "[TC][MONTANTE]"
                 << "id=" << idAtual
                 << "tipo=" << textoTipo(tipoDoElemento(idAtual))
                 << "maiorComprimentoMontante_km=" << maiorComprimentoMontante
                 << "declividadeMaiorMontante=" << declividadeDoMaiorMontante
                 << "maiorComprimentoMontanteNaoBacia_km=" << maiorComprimentoMontanteNaoBacia
                 << "declividadeMaiorMontanteNaoBacia=" << declividadeDoMaiorMontanteNaoBacia;

        double comprimentoCriticoKm = maiorComprimentoMontante;
        double declividadeEquivalenteCritica = declividadeDoMaiorMontante;

        if (tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
            const BaciaContribuicao* bacia = baciaPorId(idAtual);
            if (!bacia) return 0.0;

            const double comprimentoBaciaKm = std::max(0.0, bacia->comprimentoTalveguePrincipalKm());
            const double declividadeBacia = std::max(0.0, bacia->declividadeMedia());

            if (comprimentoBaciaKm >= comprimentoCriticoKm) {
                comprimentoCriticoKm = comprimentoBaciaKm;
                declividadeEquivalenteCritica = declividadeBacia;
            }

            qDebug() << "[TC][BACIA]"
                     << "id=" << idAtual
                     << "comprimentoBacia_km=" << comprimentoBaciaKm
                     << "declividadeBacia=" << declividadeBacia
                     << "comprimentoCritico_km=" << comprimentoCriticoKm
                     << "declividadeCritica=" << declividadeEquivalenteCritica;
        }
        else if (tipoDoElemento(idAtual) == TipoElementoRede::Canal) {
            const Canal* canal = canalPorId(idAtual);
            if (!canal) return 0.0;

            const double comprimentoCanalKm = std::max(0.0, canal->comprimento()) / 1000.0;
            const double declividadeCanal = std::max(0.0, (canal->declividadeMinima() + canal->declividadeMaxima()) * 0.5);

            // Caminho 1: maior caminho de montante + canal completo.
            const double comprimentoViaMontante = maiorComprimentoMontanteNaoBacia + comprimentoCanalKm;
            const double declividadeViaMontante = (comprimentoViaMontante > 0.0)
                                                    ? ((declividadeDoMaiorMontanteNaoBacia * maiorComprimentoMontanteNaoBacia) + (declividadeCanal * comprimentoCanalKm))
                                                        / comprimentoViaMontante
                                                    : 0.0;

            // Caminho 2 (comprimento crítico): sub-bacia local + metade do canal.
            double maiorComprimentoBaciaLocal = 0.0;
            double declividadeBaciaLocal = 0.0;
            for (const QString& idMontante : idsMontanteDireto) {
                if (tipoDoElemento(idMontante) != TipoElementoRede::BaciaContribuicao) continue;

                const BaciaContribuicao* baciaLocal = baciaPorId(idMontante);
                if (!baciaLocal) continue;

                const double comprimentoBaciaKm = std::max(0.0, baciaLocal->comprimentoTalveguePrincipalKm());
                if (comprimentoBaciaKm >= maiorComprimentoBaciaLocal) {
                    maiorComprimentoBaciaLocal = comprimentoBaciaKm;
                    declividadeBaciaLocal = std::max(0.0, baciaLocal->declividadeMedia());
                }
            }

            const double comprimentoViaBaciaLocal = maiorComprimentoBaciaLocal + (comprimentoCanalKm * 0.5);

            // Para Seq no caminho de sub-bacia, usa o comprimento total do canal.
            const double comprimentoBaseSeqViaBaciaLocal = maiorComprimentoBaciaLocal + comprimentoCanalKm;
            const double declividadeViaBaciaLocal = (comprimentoBaseSeqViaBaciaLocal > 0.0)
                                                      ? ((declividadeBaciaLocal * maiorComprimentoBaciaLocal)
                                                         + (declividadeCanal * comprimentoCanalKm))
                                                          / comprimentoBaseSeqViaBaciaLocal
                                                      : 0.0;

            if (comprimentoViaBaciaLocal > comprimentoViaMontante) {
                comprimentoCriticoKm = comprimentoViaBaciaLocal;
                declividadeEquivalenteCritica = declividadeViaBaciaLocal;

                qDebug() << "[TC][CANAL][ESCOLHA]"
                         << "id=" << idAtual
                         << "caminho=viaBaciaLocal"
                         << "comprimentoViaBaciaLocal_km=" << comprimentoViaBaciaLocal
                         << "declividadeViaBaciaLocal=" << declividadeViaBaciaLocal
                         << "comprimentoViaMontante_km=" << comprimentoViaMontante
                         << "declividadeViaMontante=" << declividadeViaMontante;
            }
            else {
                comprimentoCriticoKm = comprimentoViaMontante;
                declividadeEquivalenteCritica = declividadeViaMontante;

                qDebug() << "[TC][CANAL][ESCOLHA]"
                         << "id=" << idAtual
                         << "caminho=viaMontante"
                         << "comprimentoViaMontante_km=" << comprimentoViaMontante
                         << "declividadeViaMontante=" << declividadeViaMontante
                         << "comprimentoViaBaciaLocal_km=" << comprimentoViaBaciaLocal
                         << "declividadeViaBaciaLocal=" << declividadeViaBaciaLocal;
            }

            qDebug() << "[TC][CANAL]"
                     << "id=" << idAtual
                     << "comprimentoCanal_km=" << comprimentoCanalKm
                     << "declividadeCanal=" << declividadeCanal
                     << "comprimentoCritico_km=" << comprimentoCriticoKm
                     << "declividadeCritica=" << declividadeEquivalenteCritica;
        }

        const double declividadeEquivalente = std::max(0.0, declividadeEquivalenteCritica);

        const double tempoConcentracaoMin = calcularTempoKirpichModificado(
            comprimentoCriticoKm,
            declividadeEquivalente,
            0.0);

        comprimentoTalvegueCriticoPorId[idAtual] = comprimentoCriticoKm;
        declividadeEquivalenteCriticaPorId[idAtual] = declividadeEquivalente;
        // Aplica Tc mínimo configurável na rede (ex.: 5 min).
        tempoConcentracaoPorId[idAtual] = std::max(tempoConcentracaoMinimoMin(), std::max(0.0, tempoConcentracaoMin));

        qDebug() << "[TC][RESULTADO_ID]"
                 << "id=" << idAtual
                 << "comprimentoCritico_km=" << comprimentoCriticoKm
                 << "declividadeEquivalente=" << declividadeEquivalente
                 << "tc_min=" << tempoConcentracaoPorId.value(idAtual);
    }

    const double tcFinal = tempoConcentracaoPorId.value(idAlvo, 0.0);
    qDebug() << "[TC][FIM]" << "idAlvo=" << idAlvo << "tcFinal_min=" << tcFinal;
    return tcFinal;
}

double RedeHidrologica::calcularTempoKirpichModificado(double comprimentoTalvegueKm,
                                                       double declividadeTalvegue,
                                                       double maiorTempoConcentracaoMontanteMin) const
{
    return Hidrologia::calcularTcKirpichModificadoMin(
        comprimentoTalvegueKm,
        declividadeTalvegue,
        maiorTempoConcentracaoMontanteMin);
}

QVector<QString> RedeHidrologica::idsMontanteDoElemento(const QString& idElemento) const
{
    const QString idDestino = idElemento.trimmed();
    if (idDestino.isEmpty()) return {};
    if (!existeId(idDestino)) return {};

    QSet<QString> visitados;
    QQueue<QString> fila;
    QVector<QString> idsMontante;

    visitados.insert(idDestino);
    fila.enqueue(idDestino);

    while (!fila.isEmpty()) {
        const QString idAlvo = fila.dequeue();

        const QVector<QString> idsMontanteDireto = m_idsMontanteDiretoPorId.value(idAlvo);
        for (const QString& idMontante : idsMontanteDireto) {
            if (!visitados.contains(idMontante)) {
                visitados.insert(idMontante);
                fila.enqueue(idMontante);
                idsMontante.append(idMontante);
            }
        }
    }

    return idsMontante;
}

QVector<QString> RedeHidrologica::idsElementos() const
{
    return m_idsElementos;
}

QString RedeHidrologica::idJusanteDoElemento(const QString& idElemento) const
{
    return m_idJusantePorId.value(idElemento.trimmed());
}

TipoElementoRede RedeHidrologica::tipoDoElemento(const QString& idElemento) const
{
    return m_tipoPorId.value(idElemento.trimmed(), TipoElementoRede::Outro);
}

void RedeHidrologica::limpar()
{
    m_idsElementos.clear();
    m_idJusantePorId.clear();
    m_idsMontanteDiretoPorId.clear();
    m_tipoPorId.clear();
    m_baciasPorId.clear();
    m_bueirosPorId.clear();
    m_canaisPorId.clear();
}
