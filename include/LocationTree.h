#ifndef LOCATIONTREE_H
#define LOCATIONTREE_H

#include <string>
#include <vector>
#include <map>
#include <memory>


struct TreeNode {
    std::string name;
    std::string fullPath;
    std::map<std::string, std::unique_ptr<TreeNode>> children;

    TreeNode(const std::string& n) : name(n) {}
};

class LocationTree {
public:
    LocationTree();
    void addLocation(const std::string& fullNodeName);
    const TreeNode* getRoot() const;


private:
    std::unique_ptr<TreeNode> root;
};

#endif // LOCATIONTREE_H
