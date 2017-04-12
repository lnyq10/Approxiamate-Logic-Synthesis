//
// Created by king on 17-4-11.
//

#ifndef VE490_DECOMPOSITION_INFO_H
#define VE490_DECOMPOSITION_INFO_H

#include<map>
#include<set>

#include "boolean_function.h"
#include "kmap.h"
#include "const.h"

class BlifBuilder
{
    struct Connection
    {
        NodeName out;
        NodeName in1;
        NodeName in2;
        TruthTable method;
    };

    std::vector<NodeName, Connection> data;
    NodeName outputNodeName;

public:

    BlifBuilder(const NodeName& node, bool flip=false);

    friend BlifBuilder
    combineDecompositionInfo(const BlifBuilder & d1,
                             const BlifBuilder & d2,
                             const TruthTable& table,
                             const NodeName& newOutput);
    // for real program convenience
    // return combineDecompositionInfo(combineDecompositionInfo(d1, d2) , d3)

    NodeName outputNode();

    void exportBlif(const std::string& filename);

    std::set<NodeName> inputName();
};

BlifBuilder
combineDecompositionInfo(const BlifBuilder & d1,
                         const BlifBuilder & d2,
                         const TruthTable& table,
                         const NodeName& newOutput);

#endif //VE490_DECOMPOSITION_INFO_H