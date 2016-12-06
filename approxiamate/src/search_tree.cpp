//
// Created by king on 16-12-5.
//

#include "search.h"
#include "conts.h"
#include "boolean_function.h"

#include <graph_common.h>
#include <binary_tree_impl.h>
#include <tree_impl.h>


#include <iostream>
#include <sstream>
#include <tuple>
#include <memory>

using namespace std;

//=======search tree=========================

SearchTree::SearchTree() {
    mtree = NULL;
    currentVertexID = 0;
}

SearchTree::SearchTree(BooleanFunction &initBoolFunc) {
    unique_ptr<BooleanFunction> boolFunc(new BooleanFunction(initBoolFunc));
    SearchNodePtr initSearchNode(new SearchNode(std::move(boolFunc)));
    SearchNodeOpPtr initSearchNodeOp(new SearchNodeOp(initSearchNode));

   /* SearchSpacePtr initSearchSpace(
            new MapBasedBinaryTree<SearchNodeOpPtr>(initSearchNodeOp)
    );*/
    //mtree = new MapBasedTree<SearchSpacePtr >(initSearchSpace);
    // TODO finish the left part of MapBasedTree
    //currentVertexID = mtree -> root();
}

SearchTree::~SearchTree() {
    return ;
}

SearchSpacePtr SearchTree::getRootSpace() {
    return (mtree->valueOf(mtree->root()));
}

SearchSpacePtr SearchTree::getNextSearchSpace() {
    SearchSpace& currentSearchSpace = *(mtree->valueOf(currentVertexID));
    SearchSpacePtr newSpace = currentSearchSpace.searchSpaceGenerate();
    if (newSpace!=NULL) mtree->addAsChildren(currentVertexID, newSpace);
    return newSpace;
}

SearchSpacePtr SearchTree::getNextSearchSpace(int method) {
    SearchSpace& currentSearchSpace = *(mtree->valueOf(currentVertexID));
    SearchSpacePtr newSpace = currentSearchSpace.searchSpaceGenerate(method);
    if (newSpace!=NULL) mtree->addAsChildren(currentVertexID, newSpace);
    return newSpace;
}

SearchSpacePtr SearchTree::getCurrentSearchSpace() {
    return (mtree->valueOf(currentVertexID));
}

SearchSpace& SearchTree::getMinTotalError() {

    SearchSpacePtr res = getMinTotalErrorHelper(mtree->root());
    return *res;

}

SearchSpacePtr
SearchTree::getMinTotalErrorHelper(Tree<SearchSpacePtr >::VertexID_t node) {
    vector<Tree<SearchSpacePtr>::VertexID_t >
            res = mtree->getChild(node);
    if (res.empty()) return mtree->valueOf(node);

    SearchSpacePtr record= nullptr;
    for (auto iter : res) {
        SearchSpacePtr tmp=getMinTotalErrorHelper(iter);
        if ((record == nullptr) || (tmp->getTotalError() < record->getTotalError()))
            record = tmp;
    }
    return record;
}
