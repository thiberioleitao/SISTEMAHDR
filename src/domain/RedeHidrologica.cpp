#include "RedeHidrologica.h"

#include <QQueue>
#include <QSet>

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
    const QString idDestino = idElemento.trimmed();
    if (idDestino.isEmpty()) return 0.0;
    if (!existeId(idDestino)) return 0.0;

    const QVector<QString> ids = idsElementos();
    if (ids.isEmpty()) return 0.0;

    const QVector<QString> ordem = ordemTopologica();
    if (ordem.isEmpty()) return 0.0;

    QMap<QString, QString> jusantePorId;

    for (const QString& id : ids) {
        jusantePorId[id] = idJusanteDoElemento(id);
    }

    QMap<QString, double> comprimentoAcumulado;
    for (const QString& id : ids) {
        comprimentoAcumulado[id] = 0.0;
    }

    for (const QString& idAtual : ordem) {
        double comprimentoLocal = 0.0;
        if (tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
            const BaciaContribuicao* bacia = baciaPorId(idAtual);
            if (!bacia) return 0.0;

            const double valor = bacia->comprimentoTalveguePrincipalKm();
            if (valor < 0.0) return 0.0;
            comprimentoLocal = valor;
        }

        double maiorMontante = 0.0;
        for (const QString& idMontante : ids) {
            if (jusantePorId.value(idMontante) == idAtual) {
                maiorMontante = std::max(maiorMontante, comprimentoAcumulado.value(idMontante, 0.0));
            }
        }

        comprimentoAcumulado[idAtual] = maiorMontante + comprimentoLocal;
    }

    return std::max(0.0, comprimentoAcumulado.value(idDestino, 0.0));
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

    // Acumula comprimento crítico e desnível crítico para obter declividade equivalente.
    QMap<QString, double> comprimentoTalvegueCriticoPorId;
    QMap<QString, double> desnivelCriticoPorId;
    QMap<QString, double> tempoConcentracaoPorId;

    for (const QString& idAtual : ordem) {
        const QVector<QString> idsMontanteDireto = m_idsMontanteDiretoPorId.value(idAtual);

        double maiorComprimentoMontante = 0.0;
        double desnivelDoMaiorMontante = 0.0;
        for (const QString& idMontante : idsMontanteDireto) {
            const double comprimentoMontante = comprimentoTalvegueCriticoPorId.value(idMontante, 0.0);
            if (comprimentoMontante >= maiorComprimentoMontante) {
                maiorComprimentoMontante = comprimentoMontante;
                desnivelDoMaiorMontante = desnivelCriticoPorId.value(idMontante, 0.0);
            }
        }

        double comprimentoCriticoKm = maiorComprimentoMontante;
        double desnivelCritico = desnivelDoMaiorMontante;

        if (tipoDoElemento(idAtual) == TipoElementoRede::BaciaContribuicao) {
            const BaciaContribuicao* bacia = baciaPorId(idAtual);
            if (!bacia) return 0.0;

            const double comprimentoBaciaKm = std::max(0.0, bacia->comprimentoTalveguePrincipalKm());
            const double declividadeBacia = std::max(0.0, bacia->declividadeMedia());
            const double desnivelBacia = comprimentoBaciaKm * declividadeBacia;

            if (comprimentoBaciaKm >= comprimentoCriticoKm) {
                comprimentoCriticoKm = comprimentoBaciaKm;
                desnivelCritico = desnivelBacia;
            }
        }
        else if (tipoDoElemento(idAtual) == TipoElementoRede::Canal) {
            const Canal* canal = canalPorId(idAtual);
            if (!canal) return 0.0;

            const double comprimentoCanalKm = std::max(0.0, canal->comprimento()) / 1000.0;
            const double declividadeCanal = std::max(0.0, canal->declividadeFinal());

            // Caminho 1: maior caminho de montante + canal completo.
            const double comprimentoViaMontante = maiorComprimentoMontante + comprimentoCanalKm;
            const double desnivelViaMontante = desnivelDoMaiorMontante + (declividadeCanal * comprimentoCanalKm);

            // Caminho 2: bacia local ligada ao canal + metade do canal.
            double maiorComprimentoBaciaLocal = 0.0;
            double desnivelBaciaLocal = 0.0;
            for (const QString& idMontante : idsMontanteDireto) {
                if (tipoDoElemento(idMontante) != TipoElementoRede::BaciaContribuicao) continue;

                const BaciaContribuicao* baciaLocal = baciaPorId(idMontante);
                if (!baciaLocal) continue;

                const double comprimentoBaciaKm = std::max(0.0, baciaLocal->comprimentoTalveguePrincipalKm());
                if (comprimentoBaciaKm >= maiorComprimentoBaciaLocal) {
                    maiorComprimentoBaciaLocal = comprimentoBaciaKm;
                    desnivelBaciaLocal = comprimentoBaciaKm * std::max(0.0, baciaLocal->declividadeMedia());
                }
            }

            const double comprimentoViaBaciaLocal = maiorComprimentoBaciaLocal + (comprimentoCanalKm * 0.5);
            const double desnivelViaBaciaLocal = desnivelBaciaLocal + (declividadeCanal * comprimentoCanalKm * 0.5);

            if (comprimentoViaBaciaLocal > comprimentoViaMontante) {
                comprimentoCriticoKm = comprimentoViaBaciaLocal;
                desnivelCritico = desnivelViaBaciaLocal;
            }
            else {
                comprimentoCriticoKm = comprimentoViaMontante;
                desnivelCritico = desnivelViaMontante;
            }
        }

        const double declividadeEquivalente = (comprimentoCriticoKm > 0.0)
                                                ? std::max(0.0, desnivelCritico / comprimentoCriticoKm)
                                                : 0.0;

        const double tempoConcentracaoMin = calcularTempoKirpichModificado(
            comprimentoCriticoKm,
            declividadeEquivalente,
            0.0);

        comprimentoTalvegueCriticoPorId[idAtual] = comprimentoCriticoKm;
        desnivelCriticoPorId[idAtual] = desnivelCritico;
        tempoConcentracaoPorId[idAtual] = std::max(0.0, tempoConcentracaoMin);
    }

    return tempoConcentracaoPorId.value(idAlvo, 0.0);
}

double RedeHidrologica::calcularTempoKirpichModificado(double comprimentoTalvegueKm,
                                                       double declividadeTalvegue,
                                                       double maiorTempoConcentracaoMontanteMin) const
{
    const double comprimentoKm = std::max(1e-6, comprimentoTalvegueKm);
    const double declividade = std::max(1e-6, declividadeTalvegue);
    const double tcLocalMin = 57.0 * std::pow(comprimentoKm, 0.77) * std::pow(declividade, -0.385);
    return std::max(std::max(0.0, maiorTempoConcentracaoMontanteMin), std::max(0.0, tcLocalMin));
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
