//
// Created by king on 17-4-11.
//

#ifndef VE490_PROCE_H
#define VE490_PROCE_H

#include <vector>
#include <stack>
#include <boost/shared_ptr.hpp>

#include "node.h"
#include "bool_function.h"
#include "kmap.h"

class wholeProcess {

private:

    DecomposedFuncPtr head;
    std::stack<DecomposedFuncPtr > nowRun;
    //DecomposedFunc& nowRun(); // return the reference of the current divide decomposed function;
                              // 这个当面讲，写不清楚。而且很重要。
    //DecomposedFuncPtr lastDiv; //探测当前的搜索的节点，如果可以则向下继承，否则按照逆先序遍历回朔
    void freeStack();

public:

    bool grow(); // grow if can. return successfully grow or not;
    int diff(); // return (head->combine())^(*(*head));
    bool fullyDecomposed();

    wholeProcess();
    wholeProcess(const DecomposedFuncPtr& initPtr);
    wholeProcess(const DecomposedFunc& initDF);
    wholeProcess(const BoolFunction& initFun);
    ~wholeProcess();

    void operator= (const wholeProcess& initWP); // Deep copy required.
                                                 // only head is required.
    bool operator== (const wholeProcess& initWP);// compare the combined function;


    void display();


};

wholeProcess bestWholeProcess;

#endif //VE490_PROCE_H
