#ifndef CAMPUSGIS_H
#define CAMPUSGIS_H

#include "../graph/Graph.h"
#include "../trees/LocationTree.h"
#include <string>

// This class is the "Boss" of the non-visual part of the app.
class CampusGis {
public:
    CampusGis();

    // Loads the connections (edges) from the text file into our brain.
    bool loadMapData(const std::string& filePath);

    // Getters: Let other parts of the app (like the Window) look at the data.
    const Graph& getGraph() const;
    const LocationTree& getLocationTree() const;

private:
    // Helper to organize room names after loading them.
    void buildLocationTree();

    // The actual "Brain" holding nodes and edges.
    Graph campusGraph;

    // The "Filing Cabinet" holding room names in categories.
    LocationTree locationTree;
};

#endif // CAMPUSGIS_H
