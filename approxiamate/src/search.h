//
// Created by tripack on 16-10-26.
//

#ifndef VE490_SEARCH_H
#define VE490_SEARCH_H

#include "boolean_function.h"
#include "conts.h"
#include <truth_table.h>
#include <memory.h>
#include <graph_common.h>
#include <binary_tree_impl.h>
#include <tree_impl.h>


class SearchNode;
class SearchNodeOp;
class SearchSpace;

typedef std::shared_ptr<SearchNode> SearchNodePtr;
typedef std::shared_ptr<SearchNodeOp> SearchNodeOpPtr;
typedef std::shared_ptr<SearchSpace> SearchSpacePtr;


class SearchNode {//: public Object {
public:
    std::tuple<SearchNodePtr, SearchNodePtr, int, int>
            divide(int divideMethod);  // return two boolean function.
    bool divideAble(int divideMethod); // chech whether this node is divideable.

    SearchNode();
    SearchNode(BooleanFunctionPtr ptr);
    ~SearchNode();

    BooleanFunctionPtr getBooleanFunction();
        // create!!! notice this function create a new booleanfunction
        // that has exactly same information as itself.
    BooleanFunctionPtr combineBooleanFunction(
                BooleanFunctionPtr p1,
                BooleanFunctionPtr p2, int oper);
        // combine p1 and p2 together based on the bestOper.
        // note that this node can't be a leaf node.
    int getDivideRange(); // return 2^(getInputNum)
    int getInputNum();

    bool isDividable();

private:

    BooleanFunctionPtr booleanFunction;
    // record the best divide.
    int bestLocalErr;
    int bestOper;   // OPERATION_AND, OR, XOR, DROP
    int bestDivide;
};



class SearchNodeOp {
public:
    SearchNodeOp();
    ~SearchNodeOp();
    SearchNodeOp(SearchNodePtr nd);

    BooleanFunctionPtr combineBooleanFunction(
            BooleanFunctionPtr p1,
            BooleanFunctionPtr p2);

    std::tuple<SearchNodeOpPtr, SearchNodeOpPtr, SearchNodeOpPtr>
        divide(int method);
    // a new opnode, its left opnode and its right opnode

    bool isDiviable();

    SearchNodePtr node;
    int oper;
    int currentDivide;
    int localErr;
};


class SearchSpace {//: public Object {

public:

    SearchSpace(BinaryTree<SearchNodeOpPtr> &oldTree);
    SearchSpace(std::unique_ptr<BinaryTree<SearchNodeOpPtr> > oldTreePtr);
    SearchSpace();
    ~SearchSpace();

    BinaryTree<SearchNodeOpPtr>::VertexID_t findDivideNode();
    // return the vertexID of divide node
    bool searchSpaceGrow();  // whether this search space could be divided or not
    SearchSpacePtr searchSpaceGenerate(); // return a search space that is generated from the current one.
    std::vector<SearchSpacePtr > searchSpaceSeriesGenerate(int leftNum); // return a vector contains several seachspaceptrs
                                                                         // with least totalerrors
    SearchSpacePtr searchSpaceGenerate(int divideMethod);
    int getTotalError(); // return the total error.
    void printSearchSpace();
    BooleanFunctionPtr getFinalBooleanFuntion();
    void generateBlifFile(std::string BlifFileName, TruthTable &TruthTab);
    bool isAtLowestLevel();
    bool isCalculatedInAdvanced();


private:

    void clearData(BinaryTree<SearchNodeOpPtr>::VertexID_t node);

    std::unique_ptr<BinaryTree<SearchNodeOpPtr > > btree;
    SearchNodeOpPtr divideNode;   // which node is to divide.
    int currentDivide;        // how is the current node is divided.
    int currentDivideRange;   // the range for current divide, should be a 2^n number,
                              // the range should be 1~(currentDivideRange-2). The left tree
                              // or right tree can't be null.
    bool growAble;
    int calculTotalError();
    BinaryTree<SearchNodeOpPtr>::VertexID_t
        findDivideNodeHelper(BinaryTree<SearchNodeOpPtr>::VertexID_t node);
    BooleanFunctionPtr
        calculTotalErrorHelper(BinaryTree<SearchNodeOpPtr>::VertexID_t node);
    void printSearchSpaceHelper(BinaryTree<SearchNodeOpPtr>::VertexID_t node);
    std::tuple<std::string, BooleanFunctionPtr, int > generateBlifFileHelper(
            BinaryTree<SearchNodeOpPtr >::VertexID_t node, std::ofstream &BlifFile, TruthTable &TruthTab
    );

    //error
    int totalError; // the total error for this search space
                    // should be calculated after this SearchSpace is created.
    bool isPreviouslyCalculated = false; // in BFS, if this point if this searchspace is fully previously decomposed,
                                         // new field added, used in algorithm like A*.

};

class SearchSpaceComparator {
public:
    SearchSpacePtr ssPtr = nullptr;

    SearchSpaceComparator() {
        ssPtr = nullptr;
    }
    SearchSpaceComparator(const SearchSpacePtr& samplePtr) {
        ssPtr = samplePtr;
    }
    SearchSpaceComparator(const SearchSpaceComparator& a) {
        this->ssPtr = a.ssPtr;
    }
    ~SearchSpaceComparator() {
        ssPtr.reset();
    }
    friend bool operator< (const SearchSpaceComparator& a, const SearchSpaceComparator& b) {
        return (a.ssPtr->getTotalError() < b.ssPtr->getTotalError());
    }
    friend bool operator> (const SearchSpaceComparator& a, const SearchSpaceComparator& b) {
        return (a.ssPtr->getTotalError() > b.ssPtr->getTotalError());
    }
    /*friend SearchSpacePtr& operator* (const SearchSpaceComparator& a) {
        return a.ssPtr;
    }*/
    SearchSpaceComparator& operator= (SearchSpaceComparator& b) {
        ssPtr = b.ssPtr;
        return *this;
    }

};


class SearchTree {

public:

    SearchTree();
    SearchTree(BooleanFunction &initBoolFunc);
    ~SearchTree();

    SearchSpacePtr getNextSearchSpace(); // asking current search space could generate a new one or nor,
                                        // if could, finish current generating. Back to his parent.
    SearchSpacePtr getNextSearchSpace_BFS();
    SearchSpacePtr getNextSearchSpace_Full();
    SearchSpacePtr getNextSearchSpace(int method);
    SearchSpacePtr getCurrentSearchSpace(); //
    SearchSpacePtr getRootSpace();
    SearchSpacePtr getBestSpace();
    SearchSpace &getMinTotalError();

private:

    void clearData(Tree<SearchSpacePtr>::VertexID_t node);

    SearchSpacePtr getMinTotalErrorHelper(Tree<SearchSpacePtr >::VertexID_t node);
    SearchSpacePtr getBestSpaceHelper(Tree<SearchSpacePtr>::VertexID_t node);

    std::unique_ptr<Tree<SearchSpacePtr>> mtree;
    //SearchSpacePtr currentSearchSpace;    // the current working on search space.
    Tree<SearchSpacePtr>::VertexID_t currentVertexID;
    SearchSpacePtr recordBestSearchSpacePtr;

};



#endif //VE490_SEARCH_H
