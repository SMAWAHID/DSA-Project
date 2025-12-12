#include "../../include/gui/MainWindow.h"
#include <QtWidgets>
#include <QDebug>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <cmath>
#include <queue>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
using NodeDistance = pair<int, string>;

// ====================================================================
// == HELPER FUNCTIONS (Useful little tools)
// ====================================================================

// Takes two points on the map and calculates the distance between them
// (like measuring with a ruler on graph paper)
int calculateDistanceHelper(const QPointF& p1, const QPointF& p2) {
    qreal dist = sqrt(pow(p1.x() - p2.x(), 2) + pow(p1.y() - p2.y(), 2));
    return static_cast<int>(round(dist / 5.0));
}

// Looks at a room name like "EE-Lab-5" and tells you which building it belongs to
// (Returns "EE", "CS", "Multi", or "Outdoor" based on the name)
QString getTopLevelName(const string& fullNodeName) {
    if (fullNodeName.find("EE-") == 0) return "EE";
    if (fullNodeName.find("CS-") == 0) return "CS";
    if (fullNodeName.find("Multi") == 0) return "Multi";

    if (fullNodeName.find("EE-Building") != string::npos) return "EE";
    if (fullNodeName.find("CS-Building") != string::npos) return "CS";
    if (fullNodeName.find("Multipurpose-Building") != string::npos) return "Multi";

    return "Outdoor";
}

// ====================================================================
// == MAINWINDOW IMPLEMENTATION (The main app window)
// ====================================================================

// Constructor: This runs when the app first starts
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // Create all the buttons, maps, dropdowns, etc.
    setupUi();

    // Load the map data from the CSV file (reads all rooms and paths)
    // The file is stored as a resource in the app (:/data/campus_map_detailed.csv)
    loadDataFromCSV(":/data/campus_map_detailed.csv");

    // Draw all the floor maps and campus map
    drawAllSchematics();

    // Fill the dropdown menus with building names ("EE", "CS", "Multi", etc.)
    populateTopLevelComboBoxes();

    // Create a group to manage all the animation (the walking person)
    m_animationGroup = new QSequentialAnimationGroup(this);

    // When user clicks "Search" button, run onFindPathClicked()
    connect(m_findPathButton, &QPushButton::clicked, this, &MainWindow::onFindPathClicked);

    // When user changes the top-level area (like "EE"), update the sub-location dropdown
    connect(m_sourceTopComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateSourceSubComboBox);
    connect(m_midTopComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateMidSubComboBox);
    connect(m_destTopComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateDestSubComboBox);

    // Initialize the sub-location dropdowns with their first values
    updateSourceSubComboBox(m_sourceTopComboBox->currentText());
    updateMidSubComboBox(m_midTopComboBox->currentText());
    updateDestSubComboBox(m_destTopComboBox->currentText());

    // Set the window title and minimum size
    setWindowTitle("FAST NUCES - Navigation System");
    setMinimumSize(1200, 800);

    // After a tiny delay, zoom the campus map to fit the whole campus in view
    QTimer::singleShot(100, this, [this](){
        if(m_campusScene && m_campusView) {
            fitViewToScene(m_campusView, m_campusScene);
        }
    });
}

// Destructor: This runs when the app closes (cleanup)
MainWindow::~MainWindow() {}

// ====================================================================
// == DATA LOADING (Reading the CSV file and organizing rooms)
// ====================================================================

// This function reads the CSV file line by line and loads all room data and paths
void MainWindow::loadDataFromCSV(const QString& filename) {
    qDebug() << "Attempting to load:" << filename;

    // Clear all the old data (reset everything)
    m_graph.clear();
    m_campusNodePositions.clear();
    m_eeFloorANodes.clear(); m_eeFloorBNodes.clear(); m_eeFloorCNodes.clear(); m_eeFloorDNodes.clear(); m_eeFloorENodes.clear();
    m_csFloorGNodes.clear(); m_csFloor1Nodes.clear();
    m_multiFloorBNodes.clear(); m_multiFloorGNodes.clear(); m_multiFloor1Nodes.clear();

    // Open the CSV file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // If file doesn't exist, show an error message
        QMessageBox::critical(this, "Error", "Could not open file: " + filename);
        return;
    }

    // Read the file line by line
    QTextStream in(&file);
    int mode = 0;  // 0 = looking for section, 1 = reading nodes, 2 = reading edges
    int nodeCount = 0;
    int edgeCount = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty lines and comment lines
        if (line.isEmpty()) continue;

        // Check which section we're in
        if (line.contains("SECTION 1: NODES", Qt::CaseInsensitive)) { mode = 1; continue; }
        if (line.contains("SECTION 2: EDGES", Qt::CaseInsensitive)) { mode = 2; continue; }
        if (line.startsWith("#")) continue;  // Skip comments

        // Split the line by commas
        QStringList parts = line.split(',');

        // SECTION 1: Read rooms and their coordinates
        if (mode == 1 && parts.size() >= 3) {
            string id = parts[0].trimmed().toStdString();  // Room name like "EE-Lab-1"
            int x = parts[1].trimmed().toInt();             // X position on map
            int y = parts[2].trimmed().toInt();             // Y position on map
            assignNodeToFloor(id, QPointF(x, y));           // Put room on the right floor
            nodeCount++;
        }
        // SECTION 2: Read the connections between rooms
        else if (mode == 2 && parts.size() >= 3) {
            string u = parts[0].trimmed().toStdString();   // Room 1
            string v = parts[1].trimmed().toStdString();   // Room 2
            int w = parts[2].trimmed().toInt();             // Distance between them
            addEdge(u, v, w);                               // Connect the rooms
            edgeCount++;
        }
    }
    file.close();

    // Print how many rooms and paths we loaded
    qDebug() << "SUCCESS: Loaded" << nodeCount << "nodes and" << edgeCount << "edges.";

    // Now draw all the maps with the new data
    drawAllSchematics();
}

// This function decides which floor each room belongs to
// (Like sorting mail into the right mailbox)
void MainWindow::assignNodeToFloor(const string& id, const QPointF& pos) {
    // 1. OUTDOOR ROOMS
    // Check if the room name ends with "-O" (outdoor marker) or contains "Building"
    if (id.find("-O") != string::npos ||
        id.find("Outdoor") != string::npos ||
        id == "EE-Building" ||
        id == "CS-Building" ||
        id == "Multipurpose-Building") {
        m_campusNodePositions[id] = pos;  // Put it on the outdoor campus map
        return;
    }

    // 2. EE BUILDING - Different floors
    // If room name contains "EE-A" or "EE-Stairs-A", it's on Floor A
    if (id.find("EE-A-") != string::npos || id.find("EE-Stairs-A") != string::npos || id == "EE-Lab-DLD" || id == "EE-Lab-Eng" || id == "EE-Lab-6" || id == "EE-Hall-A" || id == "EE-Entrance-Mid-Internal") { m_eeFloorANodes[id] = pos; }
    else if (id.find("EE-B-") != string::npos || id.find("EE-Stairs-B") != string::npos || id == "EE-Hall-B") { m_eeFloorBNodes[id] = pos; }
    else if (id.find("EE-C-") != string::npos || id.find("EE-Stairs-C") != string::npos || id.find("EE-Lab-7") != string::npos || id.find("EE-Lab-8") != string::npos || id == "EE-Hall-C") { m_eeFloorCNodes[id] = pos; }
    else if (id.find("EE-D-") != string::npos || id.find("EE-Stairs-D") != string::npos || id.find("EE-Lab-9") != string::npos || id.find("EE-Lab-10") != string::npos || id.find("EE-Lab-11") != string::npos || id == "EE-Hall-D" || id == "EE-Library-Hall" || id == "EE-Sitting-Area-D") { m_eeFloorDNodes[id] = pos; }
    else if (id.find("EE-E-") != string::npos || id.find("EE-Stairs-E") != string::npos || id.find("EE-Lab-") != string::npos || id == "EE-BCR" || id == "EE-Hall-E" || id == "EE-Sitting-Area-E") { m_eeFloorENodes[id] = pos; }

    // 3. CS BUILDING - Different floors
    else if (id.find("CS-Hall-1") != string::npos || id.find("CS-Stairs-1") != string::npos || id.find("CS-Lab-") != string::npos || id.find("CS-E-") != string::npos) {
        m_csFloor1Nodes[id] = pos;  // 1st floor
    }
    else if (id.find("CS-") != string::npos && id.find("-O") == string::npos) {
        m_csFloorGNodes[id] = pos;  // Ground floor
    }

    // 4. MULTIPURPOSE BUILDING - Different floors
    else if (id == "Multi-Library" || id == "Multi-Stairs-B") { m_multiFloorBNodes[id] = pos; }
    else if (id.find("Multi-Cafeteria") != string::npos || id.find("Multi-Stairs-1") != string::npos) { m_multiFloor1Nodes[id] = pos; }
    else if (id.find("Multi-") != string::npos && id.find("-O") == string::npos) {
        m_multiFloorGNodes[id] = pos;
    }

    // 5. DEFAULT: If we don't know where it goes, put it on the outdoor map
    else {
        m_campusNodePositions[id] = pos;
    }
}

// Add a connection between two rooms in the graph
// (This is like drawing a hallway between two rooms)
void MainWindow::addEdge(const string& node1, const string& node2, int weight) {
    m_graph[node1].push_back({node2, weight});  // Room1 connects to Room2
    m_graph[node2].push_back({node1, weight});  // Room2 connects back to Room1 (can go both ways)
}

// ====================================================================
// == UI SETUP (Creating buttons, dropdowns, and maps)
// ====================================================================

// Create all the visual elements (buttons, text boxes, maps)
void MainWindow::setupUi() {
    // Create the main container widget
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // Create the main layout (horizontal: left side for controls, right side for maps)
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // Create and add the control panel (left side with dropdowns and button)
    setupControlPanel();
    mainLayout->addWidget(m_controlWidget);

    // Create and add the map tabs (right side with all the floor maps)
    setupMapTabs();
    mainLayout->addWidget(m_mainTabs);

    // Create the red person icon (shows where the user is walking)
    m_personIcon = new QGraphicsEllipseItem(0, 0, 20, 20);
    m_personIcon->setBrush(QBrush(QColor(231, 76, 60))); // Red color
    m_personIcon->setPen(QPen(Qt::white, 3));            // White outline
    m_personIcon->setZValue(100);                        // Put it on top of everything
    m_personIcon->hide();                                // Hide it initially
}

// Create the left control panel with dropdowns and the search button
void MainWindow::setupControlPanel() {
    m_controlWidget = new QWidget();
    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlWidget);

    // Create the dropdown menus for selecting source, via, and destination
    m_sourceTopComboBox = new QComboBox();
    m_sourceSubComboBox = new QComboBox();
    m_midTopComboBox = new QComboBox();
    m_midSubComboBox = new QComboBox();
    m_destTopComboBox = new QComboBox();
    m_destSubComboBox = new QComboBox();

    // Create the "Search" button with fancy styling
    m_findPathButton = new QPushButton("Search");
    m_findPathButton->setCursor(Qt::PointingHandCursor);
    m_findPathButton->setStyleSheet(
        "QPushButton { background-color: #2c3e50; color: white; font-weight: bold; font-size: 14px; padding: 12px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #34495e; }"
        "QPushButton:pressed { background-color: #1abc9c; }"
        );

    // Create the text area to show the path results
    m_pathResultText = new QTextBrowser();
    m_pathResultText->setStyleSheet("font-family: Arial; font-size: 13px; background-color: #ecf0f1; border: 1px solid #bdc3c7; border-radius: 4px;");

    // Arrange the controls in a form layout (label on left, control on right)
    QFormLayout* f = new QFormLayout();
    f->setLabelAlignment(Qt::AlignRight);
    f->addRow("<b>Source Area:</b>", m_sourceTopComboBox);
    f->addRow("Location:", m_sourceSubComboBox);
    f->addRow("<b>Via Area:</b>", m_midTopComboBox);
    f->addRow("Location:", m_midSubComboBox);
    f->addRow("<b>Dest Area:</b>", m_destTopComboBox);
    f->addRow("Location:", m_destSubComboBox);

    // Add everything to the control layout
    controlLayout->addLayout(f);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(m_findPathButton);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(m_pathResultText);

    // Set the width of the control panel
    m_controlWidget->setFixedWidth(320);
}

// Create all the map tabs (outdoor map, building maps, floor maps)
void MainWindow::setupMapTabs() {
    // Create the main tab widget that holds all the maps
    m_mainTabs = new QTabWidget();
    m_mainTabs->setStyleSheet("QTabBar::tab { height: 35px; width: 120px; font-weight: bold; }");

    // Create the OUTDOOR campus map
    m_campusScene = new QGraphicsScene(this);
    m_campusView = new QGraphicsView(m_campusScene);

    // Create EE BUILDING maps (5 floors)
    m_eeFloorA_Scene = new QGraphicsScene(this); m_eeFloorA_View = new QGraphicsView(m_eeFloorA_Scene);
    m_eeFloorB_Scene = new QGraphicsScene(this); m_eeFloorB_View = new QGraphicsView(m_eeFloorB_Scene);
    m_eeFloorC_Scene = new QGraphicsScene(this); m_eeFloorC_View = new QGraphicsView(m_eeFloorC_Scene);
    m_eeFloorD_Scene = new QGraphicsScene(this); m_eeFloorD_View = new QGraphicsView(m_eeFloorD_Scene);
    m_eeFloorE_Scene = new QGraphicsScene(this); m_eeFloorE_View = new QGraphicsView(m_eeFloorE_Scene);

    // Create CS BUILDING maps (2 floors)
    m_csFloorG_Scene = new QGraphicsScene(this); m_csFloorG_View = new QGraphicsView(m_csFloorG_Scene);
    m_csFloor1_Scene = new QGraphicsScene(this); m_csFloor1_View = new QGraphicsView(m_csFloor1_Scene);

    // Create MULTIPURPOSE BUILDING maps (3 floors)
    m_multiFloorB_Scene = new QGraphicsScene(this); m_multiFloorB_View = new QGraphicsView(m_multiFloorB_Scene);
    m_multiFloorG_Scene = new QGraphicsScene(this); m_multiFloorG_View = new QGraphicsView(m_multiFloorG_Scene);
    m_multiFloor1_Scene = new QGraphicsScene(this); m_multiFloor1_View = new QGraphicsView(m_multiFloor1_Scene);

    // Add the main outdoor tab
    m_mainTabs->addTab(m_campusView, "Outdoor Map");

    // Add tabs for EE Building with 5 floor tabs inside
    m_eeFloorTabs = new QTabWidget();
    m_eeFloorTabs->addTab(m_eeFloorA_View, "EE-A (Base)");
    m_eeFloorTabs->addTab(m_eeFloorB_View, "EE-B (Gnd)");
    m_eeFloorTabs->addTab(m_eeFloorC_View, "EE-C (1st)");
    m_eeFloorTabs->addTab(m_eeFloorD_View, "EE-D (2nd)");
    m_eeFloorTabs->addTab(m_eeFloorE_View, "EE-E (3rd)");
    m_mainTabs->addTab(m_eeFloorTabs, "EE Building");

    // Add tabs for CS Building with 2 floor tabs inside
    m_csFloorTabs = new QTabWidget();
    m_csFloorTabs->addTab(m_csFloorG_View, "CS-G (Gnd)");
    m_csFloorTabs->addTab(m_csFloor1_View, "CS-1 (1st)");
    m_mainTabs->addTab(m_csFloorTabs, "CS Building");

    // Add tabs for Multipurpose Building with 3 floor tabs inside
    m_multiFloorTabs = new QTabWidget();
    m_multiFloorTabs->addTab(m_multiFloorB_View, "Multi-B (Lib)");
    m_multiFloorTabs->addTab(m_multiFloorG_View, "Multi-G (Aud)");
    m_multiFloorTabs->addTab(m_multiFloor1_View, "Multi-1 (Caf)");
    m_mainTabs->addTab(m_multiFloorTabs, "Multipurpose");

    // Enable smooth drawing for all maps
    QList<QGraphicsView*> views = {
        m_campusView, m_eeFloorA_View, m_eeFloorB_View, m_eeFloorC_View, m_eeFloorD_View, m_eeFloorE_View,
        m_csFloorG_View, m_csFloor1_View, m_multiFloorB_View, m_multiFloorG_View, m_multiFloor1_View
    };
    for(auto v : views) v->setRenderHint(QPainter::Antialiasing);
}

// ====================================================================
// == VISUALS & DRAWING (Actually drawing the maps)
// ====================================================================

// Clear all maps and redraw them with the new data
void MainWindow::drawAllSchematics() {
    // First, clear all existing graphics
    QList<QGraphicsScene*> scenes = {
        m_campusScene, m_eeFloorA_Scene, m_eeFloorB_Scene, m_eeFloorC_Scene, m_eeFloorD_Scene, m_eeFloorE_Scene,
        m_csFloorG_Scene, m_csFloor1_Scene, m_multiFloorB_Scene, m_multiFloorG_Scene, m_multiFloor1_Scene
    };
    for(auto s : scenes) if(s) s->clear();

    // Clear the item caches
    m_nodeItems.clear();
    m_edgeItems.clear();

    // Draw each floor map
    if (m_campusScene) drawCampusSchematic();

    if (m_eeFloorA_Scene) drawFloorSchematic(m_eeFloorA_Scene, m_eeFloorANodes, "EE Floor A");
    if (m_eeFloorB_Scene) drawFloorSchematic(m_eeFloorB_Scene, m_eeFloorBNodes, "EE Floor B");
    if (m_eeFloorC_Scene) drawFloorSchematic(m_eeFloorC_Scene, m_eeFloorCNodes, "EE Floor C");
    if (m_eeFloorD_Scene) drawFloorSchematic(m_eeFloorD_Scene, m_eeFloorDNodes, "EE Floor D");
    if (m_eeFloorE_Scene) drawFloorSchematic(m_eeFloorE_Scene, m_eeFloorENodes, "EE Floor E");

    if (m_csFloorG_Scene) drawFloorSchematic(m_csFloorG_Scene, m_csFloorGNodes, "CS Floor G");
    if (m_csFloor1_Scene) drawFloorSchematic(m_csFloor1_Scene, m_csFloor1Nodes, "CS Floor 1");

    if (m_multiFloorB_Scene) drawFloorSchematic(m_multiFloorB_Scene, m_multiFloorBNodes, "Multi Floor B");
    if (m_multiFloorG_Scene) drawFloorSchematic(m_multiFloorG_Scene, m_multiFloorGNodes, "Multi Floor G");
    if (m_multiFloor1_Scene) drawFloorSchematic(m_multiFloor1_Scene, m_multiFloor1Nodes, "Multi Floor 1");
}

// Draw a single floor map with all its rooms and hallways
void MainWindow::drawFloorSchematic(QGraphicsScene* scene, const map<string, QPointF>& positions, QString floorName) {
    // Choose colors based on which building this floor belongs to
    QColor bgCol, roomCol, labCol, stairCol, hallCol;

    if (floorName.contains("EE")) {
        bgCol = QColor(255, 228, 225);      // Light pink background
        roomCol = QColor(255, 255, 255);    // White for regular rooms
        labCol = QColor(244, 143, 177);     // Pink for labs
        stairCol = QColor(155, 89, 182);    // Purple for stairs
        hallCol = QColor(210, 180, 222);    // Light purple for halls
    }
    else if (floorName.contains("CS")) {
        bgCol = QColor(214, 234, 248);      // Light blue background
        roomCol = QColor(255, 255, 255);    // White for regular rooms
        labCol = QColor(118, 215, 196);     // Teal for labs
        stairCol = QColor(41, 128, 185);    // Dark blue for stairs
        hallCol = QColor(169, 204, 227);    // Light blue for halls
    }
    else { // Multi building
        bgCol = QColor(252, 243, 207);      // Light yellow background
        roomCol = QColor(255, 255, 255);    // White for regular rooms
        labCol = QColor(241, 148, 138);     // Salmon for labs
        stairCol = QColor(211, 84, 0);      // Orange for stairs
        hallCol = QColor(248, 196, 113);    // Light orange for halls
    }

    // Figure out the size of the map (find the minimum and maximum coordinates)
    qreal minX = 10000, minY = 10000, maxX = -10000, maxY = -10000;
    if (positions.empty()) {
        // If no rooms, use a default size
        minX = 0; minY = 0; maxX = 800; maxY = 600;
    } else {
        // Find the boundaries of all rooms
        for (const auto& pair : positions) {
            QPointF p = pair.second;
            if (p.x() < minX) minX = p.x(); if (p.y() < minY) minY = p.y();
            if (p.x() > maxX) maxX = p.x(); if (p.y() > maxY) maxY = p.y();
        }
    }

    // Calculate the map size and center
    qreal width = maxX - minX;
    qreal height = maxY - minY;
    qreal centerX = minX + width/2;
    qreal centerY = minY + height/2;

    // Make sure the map is at least 1200x800
    qreal targetW = max(width + 200, 1200.0);
    qreal targetH = max(height + 200, 800.0);

    // Create the scene rectangle (the viewable area)
    QRectF sceneRect(centerX - targetW/2, centerY - targetH/2, targetW, targetH);
    scene->setSceneRect(sceneRect);

    // Draw the background
    scene->addRect(sceneRect, Qt::NoPen, QBrush(bgCol))->setZValue(-100);

    // Draw HALLWAYS (connections between rooms)
    QPen corridorPen(QColor(100, 100, 100), 4);  // Grey lines
    corridorPen.setCapStyle(Qt::RoundCap);
    QPen stairsPathPen(stairCol, 4);             // Colored lines for stairs
    stairsPathPen.setCapStyle(Qt::RoundCap);

    // Keep track of which edges we've already drawn (don't draw twice)
    set<pair<string, string>> drawnEdges;

    // Draw all the hallway connections on this floor
    for (const auto& pair : m_graph) {
        const string& u = pair.first;
        for (const auto& edge : pair.second) {
            const string& v = edge.first;

            // Only draw if both rooms are on this floor
            if (positions.count(u) && positions.count(v)) {
                // Create a unique key for this edge (u-v or v-u are the same)
                string key1 = u, key2 = v;
                if (key1 > key2) swap(key1, key2);

                // Skip if we've already drawn this edge
                if (drawnEdges.find({key1, key2}) == drawnEdges.end()) {
                    QPointF p1 = positions.at(u);
                    QPointF p2 = positions.at(v);

                    // Use different colored line for stairs
                    QPen pen = corridorPen;
                    if (u.find("Stairs") != string::npos || v.find("Stairs") != string::npos) pen = stairsPathPen;

                    // Draw the hallway line
                    QGraphicsLineItem* line = scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), pen);
                    line->setZValue(5);
                    m_edgeItems[{key1, key2}] = line;
                    drawnEdges.insert({key1, key2});
                }
            }
        }
    }

    // Font for room labels
    QFont labelFont("Arial", 10, QFont::Bold);

    // Draw ROOMS (nodes)
    for (const auto& pair : positions) {
        string name = pair.first;
        QPointF center = pair.second;

        // Draw different shapes for different room types
        QGraphicsItem* shape = nullptr;
        if (name.find("Stairs") != string::npos) {
            shape = scene->addRect(center.x()-15, center.y()-15, 30, 30, QPen(Qt::black, 1), QBrush(stairCol));
        } else if (name.find("Hall") != string::npos) {
            shape = scene->addEllipse(center.x()-4, center.y()-4, 8, 8, Qt::NoPen, QBrush(hallCol));
        } else if (name.find("Lab") != string::npos || name.find("BCR") != string::npos) {
            shape = scene->addRect(center.x()-25, center.y()-20, 50, 40, QPen(Qt::black, 1), QBrush(labCol));
        } else if (name.find("Entrance") != string::npos) {
            shape = scene->addRect(center.x()-20, center.y()-15, 40, 30, QPen(Qt::black, 1), QBrush(QColor(230, 126, 34)));
        } else {
            shape = scene->addRect(center.x()-20, center.y()-15, 40, 30, QPen(Qt::black, 1), QBrush(roomCol));
        }
        if(shape) shape->setZValue(10);

    // Draw the room label (text)
        // Remove the building prefix from the label to make it shorter
        QString label = QString::fromStdString(name);
        if (label.startsWith("EE-")) label = label.mid(3);
        if (label.startsWith("CS-")) label = label.mid(3);
        if (label.startsWith("Multi-")) label = label.mid(6);
        // Replace dashes with newlines for multi-line labels
        label = label.replace("-", "\n");

        // Draw the text label
        QGraphicsTextItem* text = scene->addText(label, labelFont);
        text->setDefaultTextColor(Qt::black);

        // Center the text over the room
        QRectF br = text->boundingRect();
        text->setPos(center.x() - br.width()/2, center.y() - br.height()/2);
        text->setZValue(20);

        // Draw a white background behind the text so it's readable
        QGraphicsRectItem* txtBg = scene->addRect(br, Qt::NoPen, QBrush(QColor(255, 255, 255, 255)));
        txtBg->setPos(text->pos());
        txtBg->setZValue(19);

        // Store this room shape for later (if we need to highlight it)
        m_nodeItems[name] = shape;
    }
}

// Draw the outdoor campus map with all the buildings
void MainWindow::drawCampusSchematic() {
    m_campusScene->clear();

    // Step 1: Figure out the size of the campus
    qreal minX = 10000, minY = 10000, maxX = -10000, maxY = -10000;
    for (const auto& pair : m_campusNodePositions) {
        QPointF p = pair.second;
        if (p.x() < minX) minX = p.x(); if (p.y() < minY) minY = p.y();
        if (p.x() > maxX) maxX = p.x(); if (p.y() > maxY) maxY = p.y();
    }

    // Add some padding around the campus
    qreal padding = 50.0;
    QRectF sceneRect(minX - padding, minY - padding,
                     (maxX - minX) + (padding * 2), (maxY - minY) + (padding * 2));

    m_campusScene->setSceneRect(sceneRect);

    // Draw a light green background
    m_campusScene->addRect(sceneRect, Qt::NoPen, QBrush(QColor(235, 240, 235)))->setZValue(-100);

    // Step 2: Try to load and display the campus image
    QPixmap campusImage(":/images/campus_map.png");
    if (!campusImage.isNull()) {
        // Scale the image to fit the scene
        QPixmap scaledImg = campusImage.scaled(
            sceneRect.size().toSize(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
            );
        QGraphicsPixmapItem* imgItem = m_campusScene->addPixmap(scaledImg);
        imgItem->setPos(sceneRect.topLeft());
        imgItem->setZValue(-50);
    }

    // Step 3: Draw the paths (hallways) between outdoor locations
    QPen connectionPen(QColor(46, 204, 113), 3);  // Green dotted lines
    connectionPen.setStyle(Qt::DotLine);

    // Draw edges (connections between outdoor nodes)
    set<pair<string, string>> drawnEdges;
    for (const auto& pair : m_graph) {
        const string& u = pair.first;
        for (const auto& edge : pair.second) {
            const string& v = edge.first;

            // Only draw if both locations are outdoors
            if (m_campusNodePositions.count(u) && m_campusNodePositions.count(v)) {
                string key1 = u, key2 = v;
                if (key1 > key2) swap(key1, key2);

                // Skip if we've already drawn this edge
                if (drawnEdges.find({key1, key2}) == drawnEdges.end()) {
                    QPointF p1 = m_campusNodePositions.at(u);
                    QPointF p2 = m_campusNodePositions.at(v);
                    QGraphicsLineItem* line = m_campusScene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), connectionPen);
                    line->setZValue(5);
                    m_edgeItems[{key1, key2}] = line;
                    drawnEdges.insert({key1, key2});
                }
            }
        }
    }

    // Step 4: Draw the outdoor nodes (buildings and important locations)
    for (const auto& pair : m_campusNodePositions) {
        string name = pair.first;
        QPointF center = pair.second;

        // Decide if this is a "landmark" (important) or just a walkway point
        bool isLandmark = (name.find("Entrance") != string::npos ||
                           name.find("Gate") != string::npos ||
                           name.find("Building") != string::npos);

        // Landmarks get big red dots, walkways get small yellow dots
        qreal size = isLandmark ? 16.0 : 10.0;
        QColor color = isLandmark ? QColor(231, 76, 60) : QColor(241, 196, 15); // Red vs Yellow

        // Draw the dot
        QGraphicsEllipseItem* pin = m_campusScene->addEllipse(center.x() - size/2, center.y() - size/2, size, size,
                                                              QPen(Qt::white, 2), QBrush(color));
        pin->setZValue(20);

        m_nodeItems[name] = pin;

        // Only label important landmarks (to avoid clutter)
        if (!isLandmark) continue;

        // We could add text labels here, but it's commented out to keep the map clean
    }
}

// ====================================================================
// == PATHFINDING (Finding the shortest path between two rooms)
// ====================================================================

// This uses Dijkstra's algorithm to find the shortest path
// (Same idea as GPS finding the shortest route)
pair<vector<string>, int> MainWindow::findShortestPath(const string& start, const string& end) {
    // Check if both rooms exist in our graph
    if (m_graph.find(start) == m_graph.end() || m_graph.find(end) == m_graph.end())
        return {{}, -1};

    // Priority queue: Always gives us the closest room to explore next
    priority_queue<NodeDistance, vector<NodeDistance>, greater<NodeDistance>> pq;

    // Map to store the shortest distance to each room
    map<string, int> distances;

    // Map to store which room we came from (for reconstructing the path)
    map<string, string> parent;

    // Initialize all distances to infinity
    for (const auto& pair : m_graph)
        distances[pair.first] = numeric_limits<int>::max();

    // Starting room has distance 0
    distances[start] = 0;
    pq.push({0, start});

    // Main algorithm loop
    while (!pq.empty()) {
        // Get the closest unexplored room
        int dist = pq.top().first;
        string u = pq.top().second;
        pq.pop();

        // If we've already found a better path to this room, skip it
        if (dist > distances[u]) continue;

        // If we reached the destination, we can stop
        if (u == end) break;

        // Check all neighboring rooms (connected by hallways)
        for (const auto& edge : m_graph[u]) {
            string v = edge.first;     // Neighbor room
            int weight = edge.second;  // Distance to neighbor

            // If going through room 'u' is faster than what we knew before...
            if (distances[u] != numeric_limits<int>::max() &&
                distances[u] + weight < distances[v]) {
                // Update the shortest distance to neighbor room
                distances[v] = distances[u] + weight;
                // Remember we came from room 'u'
                parent[v] = u;
                // Add to priority queue for exploration
                pq.push({distances[v], v});
            }
        }
    }

    // Check if we found a path (destination should not be at infinity distance)
    vector<string> path;
    int totalDistance = distances[end];
    if (totalDistance == numeric_limits<int>::max())
        return {{}, -1};  // No path exists

    // Reconstruct the path by walking backwards from end to start
    string current = end;
    while (current != start && parent.count(current)) {
        path.push_back(current);
        current = parent[current];
    }

    // Add the starting room and reverse to get start->end order
    if (current == start) {
        path.push_back(start);
        reverse(path.begin(), path.end());
    } else
        return {{}, -1};  // Error: couldn't reconstruct path

    return {path, totalDistance};
}

// ====================================================================
// == COMBO BOX & FILTERED SELECTION LOGIC
// ====================================================================

// Fill the top-level dropdown menus with building names
void MainWindow::populateTopLevelComboBoxes() {
    QStringList areas = {"Select Area...", "Outdoor", "EE", "CS", "Multi"};

    m_sourceTopComboBox->clear();
    m_midTopComboBox->clear();
    m_destTopComboBox->clear();

    m_sourceTopComboBox->addItems(areas);
    m_midTopComboBox->addItems(areas);
    m_destTopComboBox->addItems(areas);
}

// Fill the sub-location dropdown based on which area was selected
// (If user picks "EE", show only rooms in EE building)
void MainWindow::collectLeafNodes(const QString& topName, const map<string, vector<pair<string, int>>>& graph, QComboBox* comboBox) {
    comboBox->clear();
    if (topName == "Select Area...") return;

    // Go through all rooms in the graph
    for (const auto& pair : graph) {
        string n = pair.first;

        // FILTER: Skip system/internal rooms that shouldn't be selectable
        if (n == "North" || n == "South" || n == "East" || n == "West" || n.find("Mid-") != string::npos) continue;
        if (n.find("Stairs") != string::npos) continue;  // Skip stairs
        if (n.find("Hall") != string::npos && n.find("Library") == string::npos) continue;  // Skip halls (except library)
        if (n.find("Internal") != string::npos) continue;  // Skip internal connections

        // Special handling for entrances (only keep important ones)
        if (n.find("Entrance") != string::npos) {
            bool keep = false;
            if (n.find("Auditorium") != string::npos) keep = true;
            if (n.find("Cafeteria") != string::npos) keep = true;
            if (n.find("Gate") != string::npos) keep = true;
            if (!keep) continue;
        }

        bool add = false;

        // Check which building/area this room belongs to
        if (topName == "Outdoor") {
            // Show main buildings
            if (n == "EE-Building" || n == "CS-Building" || n == "Multipurpose-Building") {
                add = true;
            }
            // Also show outdoor rooms (that don't start with building prefixes)
            else if (n.find("EE-") != 0 && n.find("CS-") != 0 && n.find("Multi") != 0) {
                add = true;
            }
        }
        // Show rooms in EE building
        else if (topName == "EE") {
            if (n.find("EE-") == 0 || n == "EE-Building") add = true;
        }
        // Show rooms in CS building
        else if (topName == "CS") {
            if (n.find("CS-") == 0 || n == "CS-Building") add = true;
        }
        // Show rooms in Multipurpose building
        else if (topName == "Multi") {
            if (n.find("Multi") == 0 || n == "Multipurpose-Building") add = true;
        }

        // If we decided to add this room, format it nicely and add it
        if (add) {
            QString displayName = QString::fromStdString(n).replace("-", " ");
            displayName.replace(" O", "");  // Remove " O" suffix if visible
            comboBox->addItem(displayName, QVariant(QString::fromStdString(n)));
        }
    }

    // Sort the items alphabetically
    comboBox->model()->sort(0);
}

// These functions are called when the user changes a top-level dropdown
// They update the sub-location dropdown to show only rooms in that area
void MainWindow::updateSourceSubComboBox(const QString& text) {
    collectLeafNodes(text, m_graph, m_sourceSubComboBox);
}
void MainWindow::updateMidSubComboBox(const QString& text) {
    collectLeafNodes(text, m_graph, m_midSubComboBox);
}
void MainWindow::updateDestSubComboBox(const QString& text) {
    collectLeafNodes(text, m_graph, m_destSubComboBox);
}

// Get the actual room name from a top-level and sub-location dropdown pair
// Returns empty string if nothing is selected
string MainWindow::getSelectedNode(QComboBox* t, QComboBox* s) const {
    if(t->currentIndex()<=0 || s->currentIndex()<0) return "";
    return s->currentData().toString().toStdString();
}

// ====================================================================
// == LOOKUP HELPERS (Finding information about rooms)
// ====================================================================

// Find which floor map a room is on and return its position
QPointF MainWindow::getPosForNode(const string& nodeName) {
    // Check outdoor map first
    if (m_campusNodePositions.count(nodeName)) return m_campusNodePositions.at(nodeName);

    // Check each EE floor
    if (m_eeFloorANodes.count(nodeName)) return m_eeFloorANodes.at(nodeName);
    if (m_eeFloorBNodes.count(nodeName)) return m_eeFloorBNodes.at(nodeName);
    if (m_eeFloorCNodes.count(nodeName)) return m_eeFloorCNodes.at(nodeName);
    if (m_eeFloorDNodes.count(nodeName)) return m_eeFloorDNodes.at(nodeName);
    if (m_eeFloorENodes.count(nodeName)) return m_eeFloorENodes.at(nodeName);

    // Check each CS floor
    if (m_csFloorGNodes.count(nodeName)) return m_csFloorGNodes.at(nodeName);
    if (m_csFloor1Nodes.count(nodeName)) return m_csFloor1Nodes.at(nodeName);

    // Check each Multi floor
    if (m_multiFloorBNodes.count(nodeName)) return m_multiFloorBNodes.at(nodeName);
    if (m_multiFloorGNodes.count(nodeName)) return m_multiFloorGNodes.at(nodeName);
    if (m_multiFloor1Nodes.count(nodeName)) return m_multiFloor1Nodes.at(nodeName);

    // Room not found, return (0,0)
    return QPointF(0,0);
}

// Find which graphics scene (map) contains a room
QGraphicsScene* MainWindow::getSceneForNode(const string& nodeName) {
    if (m_campusNodePositions.count(nodeName)) return m_campusScene;

    // Check EE building floors
    if (m_eeFloorANodes.count(nodeName)) return m_eeFloorA_Scene;
    if (m_eeFloorBNodes.count(nodeName)) return m_eeFloorB_Scene;
    if (m_eeFloorCNodes.count(nodeName)) return m_eeFloorC_Scene;
    if (m_eeFloorDNodes.count(nodeName)) return m_eeFloorD_Scene;
    if (m_eeFloorENodes.count(nodeName)) return m_eeFloorE_Scene;

    // Check CS building floors
    if (m_csFloorGNodes.count(nodeName)) return m_csFloorG_Scene;
    if (m_csFloor1Nodes.count(nodeName)) return m_csFloor1_Scene;

    // Check Multi building floors
    if (m_multiFloorBNodes.count(nodeName)) return m_multiFloorB_Scene;
    if (m_multiFloorGNodes.count(nodeName)) return m_multiFloorG_Scene;
    if (m_multiFloor1Nodes.count(nodeName)) return m_multiFloor1_Scene;

    return nullptr;
}

// Find which main tab (0=Outdoor, 1=EE, 2=CS, 3=Multi) a scene belongs to
int MainWindow::getTabIndexForScene(QGraphicsScene* scene) {
    if (!scene) return 0;
    if (scene == m_campusScene) return 0;

    // EE building tab
    if (scene == m_eeFloorA_Scene || scene == m_eeFloorB_Scene || scene == m_eeFloorC_Scene ||
        scene == m_eeFloorD_Scene || scene == m_eeFloorE_Scene) return 1;

    // CS building tab
    if (scene == m_csFloorG_Scene || scene == m_csFloor1_Scene) return 2;

    // Multipurpose building tab
    if (scene == m_multiFloorB_Scene || scene == m_multiFloorG_Scene || scene == m_multiFloor1_Scene) return 3;

    return 0;
}

// Zoom the map view to fit the entire scene
void MainWindow::fitViewToScene(QGraphicsView* view, QGraphicsScene* scene) {
    if (!view || !scene) return;
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

// Switch to the tab that contains a specific scene (map)
void MainWindow::switchToSceneTab(QGraphicsScene* scene) {
    if (!scene) return;
    QGraphicsView* targetView = nullptr;

    // Get which main tab this scene is on
    int mainTabIndex = getTabIndexForScene(scene);

    // Switch to the correct main tab
    if (m_mainTabs->currentIndex() != mainTabIndex)
        m_mainTabs->setCurrentIndex(mainTabIndex);

    // Switch to the correct view (and sub-tab if needed)
    if (mainTabIndex == 0)
        targetView = m_campusView;
    else if (mainTabIndex == 1) {
        if (scene == m_eeFloorA_Scene) { m_eeFloorTabs->setCurrentIndex(0); targetView = m_eeFloorA_View; }
        else if (scene == m_eeFloorB_Scene) { m_eeFloorTabs->setCurrentIndex(1); targetView = m_eeFloorB_View; }
        else if (scene == m_eeFloorC_Scene) { m_eeFloorTabs->setCurrentIndex(2); targetView = m_eeFloorC_View; }
        else if (scene == m_eeFloorD_Scene) { m_eeFloorTabs->setCurrentIndex(3); targetView = m_eeFloorD_View; }
        else if (scene == m_eeFloorE_Scene) { m_eeFloorTabs->setCurrentIndex(4); targetView = m_eeFloorE_View; }
    }
    else if (mainTabIndex == 2) {
        if (scene == m_csFloorG_Scene) { m_csFloorTabs->setCurrentIndex(0); targetView = m_csFloorG_View; }
        else if (scene == m_csFloor1_Scene) { m_csFloorTabs->setCurrentIndex(1); targetView = m_csFloor1_View; }
    }
    else if (mainTabIndex == 3) {
        if (scene == m_multiFloorB_Scene) { m_multiFloorTabs->setCurrentIndex(0); targetView = m_multiFloorB_View; }
        else if (scene == m_multiFloorG_Scene) { m_multiFloorTabs->setCurrentIndex(1); targetView = m_multiFloorG_View; }
        else if (scene == m_multiFloor1_Scene) { m_multiFloorTabs->setCurrentIndex(2); targetView = m_multiFloor1_View; }
    }

    // Zoom the view to fit the scene
    if (targetView) fitViewToScene(targetView, scene);
}

// Reset all map styles (unhighlight edges, hide person icon)
void MainWindow::resetMapStyles() {
    m_animationGroup->stop();  // Stop any current animation
    m_animationGroup->clear();  // Clear animation queue

    // Remove the person icon from the current scene
    if(m_personIcon->scene()) m_personIcon->scene()->removeItem(m_personIcon);
    m_personIcon->hide();

    // Set all edges back to their normal color (grey)
    for (auto const& [key, item] : m_edgeItems)
        if(item) item->setPen(QPen(QColor(149, 165, 166), 4));
}

// ====================================================================
// == MAIN EVENT LOGIC (What happens when user clicks Search)
// ====================================================================

void MainWindow::onFindPathClicked() {
    // Reset all highlighting from previous search
    resetMapStyles();

    // Get which rooms the user selected
    string source = getSelectedNode(m_sourceTopComboBox, m_sourceSubComboBox);
    string mid = getSelectedNode(m_midTopComboBox, m_midSubComboBox);
    string dest = getSelectedNode(m_destTopComboBox, m_destSubComboBox);

    // Validate input
    if (source.empty() || dest.empty()) {
        QMessageBox::warning(this, "Selection Incomplete", "Please select a source and destination.");
        return;
    }
    if (source == dest) {
        QMessageBox::information(this, "Info", "Source and destination are the same.");
        return;
    }

    // Find the path
    vector<string> finalPath;
    int totalDistance = 0;

    if (mid.empty() || mid == source || mid == dest) {
        // Direct path from source to destination
        auto result = findShortestPath(source, dest);
        finalPath = result.first;
        totalDistance = result.second;
    } else {
        // Path with intermediate stop: source -> mid -> dest
        auto r1 = findShortestPath(source, mid);
        auto r2 = findShortestPath(mid, dest);

        if (r1.second != -1 && r2.second != -1) {
            totalDistance = r1.second + r2.second;
            finalPath = r1.first;
            if (!finalPath.empty()) finalPath.pop_back();  // Remove duplicate middle node
            finalPath.insert(finalPath.end(), r2.first.begin(), r2.first.end());
        } else {
            totalDistance = -1;
        }
    }

    // Check if path was found
    if (totalDistance == -1) {
        m_pathResultText->setText("No Path Found");
        QMessageBox::warning(this, "No Path", "No path exists between these locations.");
        return;
    }

    // Format and display the path
    QString pathStr = QString("<div style='color:#2980b9; font-size:14px; font-weight:bold; margin-bottom:5px;'>📍 Route (%1m):</div>").arg(totalDistance);

    for (size_t i = 0; i < finalPath.size(); ++i) {
        // Format the room name nicely (remove prefixes and underscores)
        QString nodeName = QString::fromStdString(finalPath[i]).replace("-", " ");
        nodeName = nodeName.replace(" O", "");

        // Add the room name to the display
        pathStr += QString("<span style='color:#2c3e50'>%1</span>").arg(nodeName);

        // Add arrow between rooms
        if (i < finalPath.size() - 1) {
            pathStr += " <span style='color:#e67e22; font-weight:bold;'>→</span> ";
        }

        // Highlight the hallway on the map (red)
        if (i < finalPath.size() - 1) {
            string u = finalPath[i], v = finalPath[i+1];
            string key1 = u, key2 = v;
            if (key1 > key2) swap(key1, key2);
            if (m_edgeItems.count({key1, key2})) {
                QPen highlightPen(QColor(231, 76, 60), 6);  // Red thick line
                highlightPen.setCapStyle(Qt::RoundCap);
                m_edgeItems.at({key1, key2})->setPen(highlightPen);
            }
        }
    }
    m_pathResultText->setHtml(pathStr);

    // If no path, stop here
    if (finalPath.empty()) return;

    // Place the person icon at the starting location
    QGraphicsScene* startScene = getSceneForNode(finalPath[0]);
    if (startScene) {
        if (m_personIcon->scene()) m_personIcon->scene()->removeItem(m_personIcon);
        startScene->addItem(m_personIcon);
        m_personIcon->setPos(getPosForNode(finalPath[0]) - QPointF(10, 10));
        m_personIcon->show();
        switchToSceneTab(startScene);
    }

    // Clear previous animations
    m_animationGroup->clear();

    // Create animations for each step of the path
    for (size_t i = 0; i < finalPath.size() - 1; ++i) {
        string u = finalPath[i], v = finalPath[i+1];
        QGraphicsScene* currScene = getSceneForNode(u);
        QGraphicsScene* nextScene = getSceneForNode(v);

        if (!currScene || !nextScene) continue;

        // If moving between different floors, add a pause and scene switch
        if (currScene != nextScene) {
            QPauseAnimation* pause = new QPauseAnimation(1000);
            m_animationGroup->addAnimation(pause);

            QVariantAnimation* switchAnim = new QVariantAnimation();
            switchAnim->setDuration(100);
            switchAnim->setStartValue(0);
            switchAnim->setEndValue(1);
            QObject::connect(switchAnim, &QVariantAnimation::finished, [this, nextScene, v]() {
                if (m_personIcon->scene()) m_personIcon->scene()->removeItem(m_personIcon);
                nextScene->addItem(m_personIcon);
                m_personIcon->setPos(getPosForNode(v) - QPointF(10, 10));
                switchToSceneTab(nextScene);
            });
            m_animationGroup->addAnimation(switchAnim);
        } else {
            // Animate walking between two rooms on the same floor
            QPointF start = getPosForNode(u) - QPointF(10, 10);
            QPointF end = getPosForNode(v) - QPointF(10, 10);

            QVariantAnimation* moveAnim = new QVariantAnimation();
            // Duration based on distance (slower animation for longer distances)
            moveAnim->setDuration(calculateDistanceHelper(start, end) * 80);
            moveAnim->setStartValue(start);
            moveAnim->setEndValue(end);
            moveAnim->setEasingCurve(QEasingCurve::InOutQuad);

            // Update person position as animation progresses
            QObject::connect(moveAnim, &QVariantAnimation::valueChanged, [this](const QVariant& val){
                m_personIcon->setPos(val.toPointF());
            });
            m_animationGroup->addAnimation(moveAnim);
        }
    }

    // Start the animation sequence
    m_animationGroup->start();
}
