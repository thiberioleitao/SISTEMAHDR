#include "RegistroElementosRede.h"

#include "RedeHidrologica.h"

ElementoTopologicoInterno::ElementoTopologicoInterno()
    : tipo(TipoElementoRede::Outro)
{
}

ElementoTopologicoInterno::ElementoTopologicoInterno(const QString& idElemento,
                                                   const QString& idJusanteElemento,
                                                   TipoElementoRede tipoElemento)
    : id(idElemento)
    , idJusante(idJusanteElemento)
    , tipo(tipoElemento)
{
}

void EstruturaTopologica::limpar()
{
    indicePorId.clear();
    grauEntrada.clear();
    ordemTopologica.clear();
}

QVector<ElementoTopologicoInterno> criarElementosTopologicos(const RedeHidrologica& rede)
{
    QVector<ElementoTopologicoInterno> elementos;
    const QVector<QString> ids = rede.idsElementos();
    elementos.reserve(ids.size());

    for (const QString& id : ids) {
        elementos.append(ElementoTopologicoInterno(id, rede.idJusanteDoElemento(id), rede.tipoDoElemento(id)));
    }

    return elementos;
}
