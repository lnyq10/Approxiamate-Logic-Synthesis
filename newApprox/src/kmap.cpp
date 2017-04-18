//
// Created by king on 17-4-12.
//

#include <vector>
#include <map>
#include <sstream>

#include "const.h"
#include "ttable.h"
#include "kmap.h"
#include "bool_function.h"
#include "../../circuit_profile/sim_profile.h"

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;

struct tempNodeNameGenertor {
    string base = "tempNodeTempNode";
    int num = 0;
    string gen() {
        std::stringstream xx;
        xx << base << num++ ;
        return xx.str();
    }
};

void num2Dbits(DBitset& db, size_t bits,
               size_t start, size_t num) {
    size_t temp = num;
    for (size_t k = start; k < bits + start; ++k) {
        db[k] = temp % 2;
        temp = temp / 2;
    }
    return;
}

void randTTable(TTable& a, const size_t bits) {
    for (size_t i = 0; i < bits; ++i) {
        a[i] = rand() % 2;
    }
}

tempNodeNameGenertor nodeNameGen;

Kmap::Kmap(
        const BoolFunction &BF,
        const std::vector<NodeName> &heightN,
        const std::vector<NodeName> &widthN) {

    heightName = heightN;
    widthName = widthN;
    height = ( 1 << (heightName.size()));
    width = ( 1 << (widthName.size()));

    auto rowselector = DBitset(BF.getInputSize());
    auto colselector = DBitset(BF.getInputSize());

    for (const auto& elem : heightN)
        colselector.set(BF.getPortNum(elem));

    for (const auto& elem : widthN)
        rowselector.set(BF.getPortNum(elem));

    assert(colselector == ~rowselector);

    kmap = BF.getTTable().breakdown(rowselector, colselector);

}

void Kmap::display() const {
    cout << "size (h*w):  " << height << " * " << width << endl;

    cout << " height name: " ;
    for (int i = 0; i < heightName.size(); ++i)
        cout << heightName[i] << ", ";
    cout << endl;
    cout << " width name: " ;
    for (int i = 0; i < widthName.size(); ++i)
        cout << widthName[i] << ", ";
    cout << endl;

    cout << " map:" << endl;
    for (int i = 0; i < height; ++i) {
        cout << "  ";
        for (int j = 0; j < width; ++j) {
            cout << kmap[i][j] << " ";
        }
        cout << endl;
    }
}

bool Kmap::operator==(const Kmap &initKmap) const {
    return this->kmap == initKmap.kmap;
}

size_t Kmap::operator^(const Kmap &initKmap) const {
    size_t count = 0;
    for (int i = 0; i < height; ++i)
        count += (this->kmap[i] ^ initKmap.kmap[i]).count();
    return count;
}

Kmap::BestApprox Kmap::divide(const SimulationResult &simData) {

    // case 1: 0s and 1s
    //  operation DROP_RIGHT (LEFT_RELA_TABLE)
    //  column = 1 if 1s is selected

    // case 2: MajorRow + 0s
    //  operation AND
    //  column = 1 if RowPattern is selected

    // case 3: 1s + MajorRow
    //  operatrion OR
    //  column = 1 if 1s is selected

    // case 4: MajorRow + ~MajorRow
    //  operation XOR
    //  column = 1 if MajorRow is selected

    vector<NodeName> nodeNameSet = heightName;
    for (int i = 0; i < widthName.size(); ++i) {
        nodeNameSet.push_back(widthName[i]);
    }
    FocusedSimulationResult focusSim(simData, nodeNameSet);

    TTable All0s(0, width);
    TTable All1s( ( 1 << width ) - 1, width);



    // case 1:
    TTable rowPattern(0, width);
    TTable columnPattern(0, height);
    randTTable(columnPattern, height);

    while (true) {
        TTable recordRowPattern(0, width);
        recordRowPattern = rowPattern;
        TTable recordColumnPattern(0, height);
        recordColumnPattern = columnPattern;

        // rowPattern --> columnPatternTemp
        TTable columnPatternTemp(0, height);
        columnPatternTemp = getColumnPattern(
                focusSim, columnPattern, rowPattern, LEFT_RELA_TABLE);
        columnPattern = columnPatternTemp;

        // columnPattern --> rowPatternTemp
        TTable rowPatternTemp(0, width);
        rowPatternTemp = getRowPattern(
                focusSim, columnPattern, rowPattern, LEFT_RELA_TABLE);
        rowPattern = rowPatternTemp;

        // if keep same break out loop;
        if ((recordRowPattern == rowPattern) &&
        (recordColumnPattern == columnPattern)) break;
    }

    BestApprox case1 = errorCountWhole(
            focusSim, columnPattern, rowPattern, LEFT_RELA_TABLE);
    if (case1.leftFunc.isAll1s())
        case1.method = ALL_IRR_TABLE_1;
    else if (case1.leftFunc.isAll0s())
        case1.method = ALL_IRR_TABLE_0;



    // case 2
    randTTable(rowPattern, width);
    randTTable(columnPattern, height);

    while (true) {
        TTable recordRowPattern(0, width);
        recordRowPattern = rowPattern;
        TTable recordColumnPattern(0, height);
        recordColumnPattern = columnPattern;

        // rowPattern --> columnPatternTemp
        TTable columnPatternTemp(0, height);
        columnPatternTemp = getColumnPattern(
                focusSim, columnPattern, rowPattern, AND_TABLE);
        columnPattern = columnPatternTemp;

        // columnPattern --> rowPatternTemp
        TTable rowPatternTemp(0, width);
        rowPatternTemp = getRowPattern(
                focusSim, columnPattern, rowPattern, AND_TABLE);
        rowPattern = rowPatternTemp;

        // if keep same break out loop;
        if ((recordRowPattern == rowPattern) &&
            (recordColumnPattern == columnPattern)) break;
    }

    BestApprox case2 = errorCountWhole(
            focusSim, columnPattern, rowPattern, AND_TABLE);
    if (case2.leftFunc.isAll1s() && case2.rightFunc.isAll1s())
        case2.method = ALL_IRR_TABLE_1;
    else if (case2.leftFunc.isAll0s() || case2.rightFunc.isAll0s())
        case2.method = ALL_IRR_TABLE_0;
    else if (case2.leftFunc.isAll1s())
        case2.method = RIGHT_RELA_TABLE;
    else if (case2.rightFunc.isAll1s())
        case2.method = LEFT_RELA_TABLE;



    // case 3
    randTTable(rowPattern, width);
    randTTable(columnPattern, height);

    while (true) {
        TTable recordRowPattern(0, width);
        recordRowPattern = rowPattern;
        TTable recordColumnPattern(0, height);
        recordColumnPattern = columnPattern;

        // rowPattern --> columnPatternTemp
        TTable columnPatternTemp(0, height);
        columnPatternTemp = getColumnPattern(
                focusSim, columnPattern, rowPattern, OR_TABLE);
        columnPattern = columnPatternTemp;

        // columnPattern --> rowPatternTemp
        TTable rowPatternTemp(0, width);
        rowPatternTemp = getRowPattern(
                focusSim, columnPattern, rowPattern, OR_TABLE);
        rowPattern = rowPatternTemp;

        // if keep same break out loop;
        if ((recordRowPattern == rowPattern) &&
            (recordColumnPattern == columnPattern)) break;
    }

    BestApprox case3 = errorCountWhole(
            focusSim, columnPattern, rowPattern, OR_TABLE);
    if (case3.leftFunc.isAll1s() || case3.rightFunc.isAll1s())
        case3.method = ALL_IRR_TABLE_1;
    else if (case3.leftFunc.isAll0s() && case3.rightFunc.isAll0s())
        case3.method = ALL_IRR_TABLE_0;
    else if (case3.leftFunc.isAll0s())
        case3.method = RIGHT_RELA_TABLE;
    else if (case3.rightFunc.isAll0s())
        case3.method = LEFT_RELA_TABLE;



    // case 4
    randTTable(rowPattern, width);
    randTTable(columnPattern, height);
    while (true) {
        TTable recordRowPattern(0, width);
        recordRowPattern = rowPattern;
        TTable recordColumnPattern(0, height);
        recordColumnPattern = columnPattern;

        // rowPattern --> columnPatternTemp
        TTable columnPatternTemp(0, height);
        columnPatternTemp = getColumnPattern(
                focusSim, columnPattern, rowPattern, XOR_TABLE);
        columnPattern = columnPatternTemp;

        // columnPattern --> rowPatternTemp
        TTable rowPatternTemp(0, width);
        rowPatternTemp = getRowPattern(
                focusSim, columnPattern, rowPattern, XOR_TABLE);
        rowPattern = rowPatternTemp;

        // if keep same break out loop;
        if ((recordRowPattern == rowPattern) &&
            (recordColumnPattern == columnPattern)) break;
    }

    BestApprox case4 = errorCountWhole(
            focusSim, columnPattern, rowPattern, XOR_TABLE);
    if ((case4.leftFunc.isAll0s() && case4.rightFunc.isAll1s()) ||
        (case4.leftFunc.isAll1s() && case4.rightFunc.isAll0s()))
        case4.method = ALL_IRR_TABLE_1;
    else if ((case4.leftFunc.isAll0s() && case4.rightFunc.isAll0s()) ||
        (case4.leftFunc.isAll1s() && case4.rightFunc.isAll1s()))
        case4.method = ALL_IRR_TABLE_0;
    else if (case4.leftFunc.isAll1s())
        case4.method = RIGHT_RELA_NOT_TABLE;
    else if (case4.leftFunc.isAll0s())
        case4.method = RIGHT_RELA_TABLE;
    else if (case4.rightFunc.isAll1s())
        case4.method = LEFT_RELA_NOT_TABLE;
    else if (case4.rightFunc.isAll0s())
        case4.method = LEFT_RELA_TABLE;



    // conclude
    if ((case1.errorCount <= case2.errorCount)
        && (case1.errorCount <= case3.errorCount)
        && (case1.errorCount <= case4.errorCount))
        return case1;
    else if ((case2.errorCount <= case3.errorCount)
             && (case2.errorCount <= case4.errorCount))
        return case2;
    else if (case3.errorCount <= case4.errorCount)
        return case3;
    else return case4;


}

TTable Kmap::getColumnPattern(const FocusedSimulationResult &focusSim,
                              const TTable &columnPattern,
                              const TTable &rowPattern,
                              const TTable &combineMethod) {
// rowPattern --> columnPattern

    TTable colPatternTemp(0, height);
    colPatternTemp = colPatternTemp;
    for (size_t i = 0; i < height; ++i) {
        DBitset line(heightName.size() + widthName.size(), 0);
        num2Dbits(line, heightName.size(), 0, i);
        size_t err_0, err_1;
        err_0 = 0; err_1 = 0;
        for (size_t j = 0; j < width; ++j) {
            num2Dbits(line, widthName.size(), heightName.size(), j);
            err_0 += (errorCountRowColumnPattern(i, j, 0, rowPattern[j], combineMethod)
                      * focusSim.count(line));
            err_1 += (errorCountRowColumnPattern(i, j, 1, rowPattern[j], combineMethod)
                      * focusSim.count(line));
        }
        if (err_0 == err_1) continue;
        colPatternTemp[i] = (err_0 < err_1) ? 0:1;
    }

    return colPatternTemp;
}

TTable Kmap::getRowPattern(const FocusedSimulationResult& focusSim,
                           const TTable& columnPattern,
                           const TTable& rowPattern,
                           const TTable& combineMethed) {
// columnPattern --> rowPattern

    TTable rowPatternTemp(0, width);
    rowPatternTemp = rowPattern;
    for (size_t j = 0; j < width ; ++j) {
        DBitset line(heightName.size()+widthName.size(), 0);
        num2Dbits(line, widthName.size(), heightName.size(), j);
        size_t err_0, err_1;
        err_0 = 0; err_1 = 0;
        for (size_t i = 0; i < height; ++i) {
            num2Dbits(line, heightName.size(), 0, i);
            err_0 += (errorCountRowColumnPattern(i, j, columnPattern[i], 0, combineMethed)
                      * focusSim.count(line));
            err_1 += (errorCountRowColumnPattern(i, j, columnPattern[i], 1, combineMethed)
                      * focusSim.count(line));
        }
        if (err_0 == err_1) continue;
        rowPatternTemp[j] = (err_0 < err_1) ? 0:1;
    }

    return rowPatternTemp;
}


size_t Kmap::errorCountRowColumnPattern(const size_t column,
                                        const size_t row,
                                        const bool columnVal,
                                        const bool rowVal,
                                        const TTable &combineMethod) {
    bool temp;

    if (combineMethod == LEFT_RELA_TABLE) {
        temp = columnVal;
    }
    if (combineMethod == AND_TABLE) {
        temp = (columnVal && rowVal);
    }
    if (combineMethod == OR_TABLE) {
        temp = (columnVal || rowVal);
    }
    if (combineMethod == XOR_TABLE) {
        temp = (columnVal != rowVal)? true : false;
    }
    return ((temp == kmap[column][row])? 0:1);
}

Kmap::BestApprox Kmap::errorCountWhole(const FocusedSimulationResult &focusSim,
                                       const TTable &columnPattern,
                                       const TTable &rowPattern,
                                       const TTable &combineMethod) {

    BestApprox caseT;
    caseT.leftFunc = BoolFunction(heightName.size(), columnPattern, heightName, nodeNameGen.gen());
    caseT.rightFunc = BoolFunction(widthName.size(), rowPattern, widthName, nodeNameGen.gen());
    caseT.errorCount = 0;
    caseT.method = combineMethod;
    for (size_t i = 0; i < height; ++i) {
        DBitset line(heightName.size() + widthName.size(), 0);
        num2Dbits(line, heightName.size(), 0, i);
        for (size_t j = 0; j < width; ++j) {
            num2Dbits(line, widthName.size(), heightName.size(), j);
            caseT.errorCount += (errorCountRowColumnPattern(
                    i, j, columnPattern[i], rowPattern[j], combineMethod)
                                * focusSim.count(line));
        }
    }
    return caseT;
}


/*size_t Kmap::errorCountRowColumnPattern(
        const TTable &rowPattern, 
        const TTable &columnPattern,
        const TTable &combineMethod,
        const FocusedSimulationResult &focusSim
) {
    
// under given rowPattern, columnPattern and combineMethod,
// check the current error. (total bits flip in different cases)
    size_t totalInputs = rowPattern.nInputs() + columnPattern.nInputs();
    DBitset colMask(0, totalInputs);
    DBitset rowMask(0, totalInputs);

    for (size_t i = 0; i < (columnPattern.nInputs()); ++i) {
        colMask[i] = 1;
    }
    for (size_t i = columnPattern.nInputs(); i < totalInputs; ++i) {
        rowMask[i] = 1;
    }

    TTable combineTTable(0, totalInputs);
    combineTTable = combineTruthTable(columnPattern, rowPattern,
                        colMask, rowMask, combineMethod);
    vector<TTable> newMap = combineTTable.breakdown(rowMask, colMask);

    size_t err = 0;
    for (size_t i = 0; i < height; ++i) {
        string base = num2string(heightName.size(), i);
        for (size_t j = 0; j < width; ++j) {
            string exten = num2string(widthName.size(), j);
            err += (newMap[i][j] ^ kmap[i][j]) * focusSim.count(base + exten);
        }
    }

    return err;
}

*/