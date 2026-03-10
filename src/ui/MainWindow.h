#pragma once

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QVector>

class QDockWidget;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    struct PropriedadeElemento
    {
        QString nome;
        QString valor;
    };

    struct ElementoApresentacao
    {
        QString id;
        QString nome;
        QString tipo;
        QString resumo;
        double larguraInferior = 0.0;
        double alturaMaxima = 0.0;
        double taludeLateral = 0.0;
        QVector<PropriedadeElemento> propriedades;
    };

    void configurarJanela();
    void configurarCentro();
    void configurarArvoreRede();
    void configurarPainelPropriedades();
    void configurarCenaBase();
    void popularArvoreRede();
    void selecionarPrimeiroElemento();
    void aplicarSelecao(QTreeWidgetItem* item);
    void atualizarPainelPropriedades(const ElementoApresentacao& elemento);
    void destacarElementoCena(const QString& chaveElemento);
    QString chaveElemento(QTreeWidgetItem* item) const;

private:
    QDockWidget* m_dockRede = nullptr;
    QDockWidget* m_dockPropriedades = nullptr;
    QTreeWidget* m_arvoreRede = nullptr;
    QTableWidget* m_tabelaPropriedades = nullptr;
    QLabel* m_tituloCentro = nullptr;
    QLabel* m_subtituloCentro = nullptr;
    QGraphicsView* m_viewRede = nullptr;
    QGraphicsScene* m_cenaRede = nullptr;
    QMap<QString, QGraphicsItem*> m_itensCena;
    QMap<QString, ElementoApresentacao> m_elementos;
};
