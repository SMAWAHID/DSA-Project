#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <map>
#include <string>
#include "../core/CampusGis.h"
#include "../trees/LocationTree.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QTextBrowser;
class QGraphicsView;
class QGraphicsScene;
class QTabWidget;
class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QVariantAnimation;
class QPauseAnimation;
class QSequentialAnimationGroup;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFindPathClicked();
    void updateSourceSubComboBox(const QString& text);
    void updateDestSubComboBox(const QString& text);
    void updateMidSubComboBox(const QString& text);

private:
    void setupUi();
    void setupControlPanel();
    void setupMapTabs();

    void drawFloorSchematic(QGraphicsScene* scene, const std::map<std::string, QPointF>& nodePositions, QString floorName);
    void drawCampusSchematic();
    void drawAllSchematics();

    void populateTopLevelComboBoxes();
    std::string getSelectedNode(QComboBox* top, QComboBox* sub) const;
    void collectLeafNodes(const QString& topName, const std::map<std::string, std::vector<std::pair<std::string, int>>>& graph, QComboBox* comboBox);

    void resetMapStyles();
    void fitViewToScene(QGraphicsView* view, QGraphicsScene* scene);
    void switchToSceneTab(QGraphicsScene* scene);
    int calculateDistance(const QPointF& p1, const QPointF& p2) const;
    QGraphicsScene* getSceneForNode(const std::string& nodeName);
    QPointF getPosForNode(const std::string& nodeName);
    int getTabIndexForScene(QGraphicsScene* scene);

    void buildGraph();
    void addEdge(const std::string& node1, const std::string& node2, int weight);
    std::pair<std::vector<std::string>, int> findShortestPath(const std::string& start, const std::string& end);

    std::map<std::string, QPointF> m_campusNodePositions;

    // EE Maps
    std::map<std::string, QPointF> m_eeFloorANodes;
    std::map<std::string, QPointF> m_eeFloorBNodes;
    std::map<std::string, QPointF> m_eeFloorCNodes;
    std::map<std::string, QPointF> m_eeFloorDNodes;
    std::map<std::string, QPointF> m_eeFloorENodes;

    // CS Maps
    std::map<std::string, QPointF> m_csFloorFNodes;
    std::map<std::string, QPointF> m_csFloorGNodes;
    std::map<std::string, QPointF> m_csFloor1Nodes;

    // Multi Maps
    std::map<std::string, QPointF> m_multiFloorBNodes;
    std::map<std::string, QPointF> m_multiFloorGNodes;
    std::map<std::string, QPointF> m_multiFloor1Nodes;

    std::map<std::string, std::vector<std::pair<std::string, int>>> m_graph;

    void loadDataFromCSV(const QString& filename);
    void assignNodeToFloor(const std::string& id, const QPointF& pos);

    CampusGis m_gis;

    QWidget* m_controlWidget;
    QComboBox *m_sourceTopComboBox, *m_sourceSubComboBox;
    QComboBox *m_midTopComboBox, *m_midSubComboBox;
    QComboBox *m_destTopComboBox, *m_destSubComboBox;
    QPushButton* m_findPathButton;
    QTextBrowser* m_pathResultText;

    QTabWidget* m_mainTabs;
    QGraphicsView* m_campusView; QGraphicsScene* m_campusScene;

    QTabWidget* m_eeFloorTabs;
    QGraphicsView* m_eeFloorA_View; QGraphicsScene* m_eeFloorA_Scene;
    QGraphicsView* m_eeFloorB_View; QGraphicsScene* m_eeFloorB_Scene;
    QGraphicsView* m_eeFloorC_View; QGraphicsScene* m_eeFloorC_Scene;
    QGraphicsView* m_eeFloorD_View; QGraphicsScene* m_eeFloorD_Scene;
    QGraphicsView* m_eeFloorE_View; QGraphicsScene* m_eeFloorE_Scene;

    QTabWidget* m_csFloorTabs;
    QGraphicsView* m_csFloorF_View; QGraphicsScene* m_csFloorF_Scene;
    QGraphicsView* m_csFloorG_View; QGraphicsScene* m_csFloorG_Scene;
    QGraphicsView* m_csFloor1_View; QGraphicsScene* m_csFloor1_Scene;

    QTabWidget* m_multiFloorTabs;
    QGraphicsView* m_multiFloorB_View; QGraphicsScene* m_multiFloorB_Scene;
    QGraphicsView* m_multiFloorG_View; QGraphicsScene* m_multiFloorG_Scene;
    QGraphicsView* m_multiFloor1_View; QGraphicsScene* m_multiFloor1_Scene;

    std::map<std::string, QGraphicsItem*> m_nodeItems;
    std::map<std::string, QGraphicsRectItem*> m_roomItems;
    std::map<std::pair<std::string, std::string>, QGraphicsLineItem*> m_edgeItems;

    QSequentialAnimationGroup* m_animationGroup;
    QGraphicsEllipseItem* m_personIcon;
};

#endif // MAINWINDOW_H
