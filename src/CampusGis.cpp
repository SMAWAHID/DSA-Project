#include "../../include/core/CampusGis.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

// Added: Use standard namespace to remove std:: prefixes
using namespace std;

CampusGis::CampusGis() {}

// This opens the CSV text file and reads it line by line.
bool CampusGis::loadMapData(const string& filePath) {
    QString qFilePath = QString::fromStdString(filePath);
    QFile file(qFilePath);

    // Check if the file actually exists
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Error: Could not open file " << qFilePath;
        return false;
    }

    QTextStream in(&file);
    // Skip the header line if it exists
    if (!in.atEnd()) {
        QString firstLine = in.readLine();
        if (!firstLine.startsWith("#")) { in.seek(0); }
    }

    // Read the file until the end
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');

        // If we found a line with 3 parts (From, To, Weight), it's a path!
        if (parts.size() == 3) {
            string from = parts[0].toStdString();
            string to = parts[1].toStdString();
            bool ok;
            int weight = parts[2].toInt(&ok);
            if (ok) {
                // Tell the brain (Graph) about this connection
                campusGraph.addEdge(from, to, weight);
            }
        }
    }

    file.close();
    qInfo() << "Successfully loaded detailed map data from" << qFilePath;

    // Now organize the names for the tree
    buildLocationTree();
    return true;
}

const Graph& CampusGis::getGraph() const {
    return campusGraph;
}

const LocationTree& CampusGis::getLocationTree() const {
    return locationTree;
}

// Takes all the messy node names and files them neatly into the tree
void CampusGis::buildLocationTree() {
    vector<string> nodes = campusGraph.getNodes();
    for (const string& nodeName : nodes) {
        locationTree.addLocation(nodeName);
    }
}
