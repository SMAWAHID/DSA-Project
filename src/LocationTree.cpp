#include "../../include/trees/LocationTree.h"
#include <sstream>
#include <vector>
#include <iterator>
using namespace std;

// ====================================================================
// == HELPER FUNCTION: Split a string into parts
// ====================================================================

// This function takes a string like "EE-Lab-1" and splits it by the delimiter '-'
// Result: ["EE", "Lab", "1"]
// Example: split("EE-Floor-A-Lab-5", '-') returns {"EE", "Floor", "A", "Lab", "5"}
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;  // This will hold all the parts
    string token;           // Current part being built
    istringstream tokenStream(s);  // Create a stream from the string

    // Read from the stream until we hit the delimiter
    // (getline with a delimiter works like split in other languages)
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);  // Add this part to our list
    }
    return tokens;  // Return all the parts
}

// ====================================================================
// == LOCATIONTREE IMPLEMENTATION
// ====================================================================

// Constructor: Create the root node of the tree (like the root folder)
LocationTree::LocationTree() {
    // Create the root with the name "Campus"
    // This is like having a main folder called "Campus" that contains everything else
    root = make_unique<TreeNode>("Campus");
}

// Add a location to the tree
// This organizes room names into a folder structure
// Example: addLocation("EE-Lab-1") creates folders: Campus/EE/Lab/1
void LocationTree::addLocation(const string& fullNodeName) {
    // Step 1: Split the room name by dashes
    // "EE-Lab-1" becomes ["EE", "Lab", "1"]
    vector<string> parts = split(fullNodeName, '-');

    // Start at the root (top-level folder)
    TreeNode* current = root.get();

    // If the name is empty, stop here
    if (parts.empty()) return;

    // Special case: If the room name has no dashes (just one part like "Outdoor")
    // Just add it directly under the root
    if (parts.size() == 1) {
        // Check if this part already exists as a child
        if (current->children.find(parts[0]) == current->children.end()) {
            // Doesn't exist, so create a new folder
            current->children[parts[0]] = make_unique<TreeNode>(parts[0]);
            // Store the full room name in this leaf node
            current->children[parts[0]]->fullPath = fullNodeName;
        }
        return;
    }

    // Step 2: Walk through each part and create folders as needed
    // For "EE-Lab-1", we create: Campus/EE/Lab/1
    for (size_t i = 0; i < parts.size(); ++i) {
        string& part = parts[i];

        // Check if this folder already exists under the current location
        if (current->children.find(part) == current->children.end()) {
            // Doesn't exist, so create a new folder with this name
            current->children[part] = make_unique<TreeNode>(part);
        }

        // Move into this folder
        current = current->children[part].get();
    }

    // Step 3: Store the full room name in the final leaf node
    // So if we're at Campus/EE/Lab/1, we store "EE-Lab-1" there
    current->fullPath = fullNodeName;
}

// This function returns the root node of the tree
// (Currently commented out in the original code, but here's what it would do)
// It lets other parts of the code access the tree structure
// const TreeNode* LocationTree::getRoot() const {
//     return root.get();
// }

// ====================================================================
// == HOW THE TREE WORKS (Example)
// ====================================================================
//
// Let's say we add these room names:
//     addLocation("EE-A-Lab-1");
//     addLocation("EE-A-Hall");
//     addLocation("EE-B-Lab-2");
//     addLocation("CS-Lab-5");
//     addLocation("Outdoor");
//
// The tree structure would look like this:
//
//     Campus (root)
//     ├── EE
//     │   ├── A
//     │   │   ├── Lab
//     │   │   │   └── 1 (fullPath = "EE-A-Lab-1")
//     │   │   └── Hall (fullPath = "EE-A-Hall")
//     │   └── B
//     │       └── Lab
//     │           └── 2 (fullPath = "EE-B-Lab-2")
//     ├── CS
//     │   └── Lab
//     │       └── 5 (fullPath = "CS-Lab-5")
//     └── Outdoor (fullPath = "Outdoor")
//
// This tree makes it easy to organize and navigate through all the rooms!
//
// ====================================================================
