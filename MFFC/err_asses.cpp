//
// Created by tripack on 16-12-16.
//

#include "err_asses.h"
#include "interface.h"
#include "print_truth_table.h"
#include <string>

BlifCompareResult compareBlifFiles(const std::string& model,
                                   const std::string& compared) {
    string m = model;
    string c = compared;
    TruthTable modelTruthTable = getTruthTable(m);
    TruthTable comparedTruthTable = getTruthTable(c);
    BlifCompareResult r;
    if (modelTruthTable.size() != comparedTruthTable.size()) {
        r.valid = false;
        r.msg = "Inconsistent size of comparison.";
        return r;
    }
    r.nInputs = modelTruthTable.numInput();
    r.nSamples = modelTruthTable.size();
    for (int i = 0; i < r.nSamples ; ++i) {
        if ((int)(modelTruthTable[i]) != (int)(comparedTruthTable[i]) ) {
            r.errorCount++;
        }
    }
    return r;
}

BlifCompareResult compareBlifs(const BlifBooleanNet &model,
                               const BlifBooleanNet &compared) {
}

BlifCompareResult fullCompareBlifs(const BlifBooleanNet &model,
                                   const BlifBooleanNet &compared) {
    BlifCompareResult r;
    if (model.nInputs() != compared.nInputs()) {
        r.valid = false;
        r.msg = "Inconsistent size of comparison.";
        return r;
    }
    r.nInputs = model.nInputs();
    r.nSamples = 1 << r.nInputs;

    TruthTable modelTruthTable = model.truthTable();
    TruthTable comparedTruthTable = compared.truthTable();

    for (int i = 0; i < r.nSamples ; ++i) {
        if ((int)(modelTruthTable[i]) != (int)(comparedTruthTable[i]) ) {
            r.errorCount++;
        }
    }
    return r;
}

BlifCompareResult sampleCompareBlifs(const BlifBooleanNet &model,
                                     const BlifBooleanNet &compared,
                                     int sampleSize) {
    std::srand(time(0));
    BlifCompareResult r;
    if (model.nInputs() != compared.nInputs()) {
        r.valid = false;
        r.msg = "Inconsistent size of comparison.";
        return r;
    }
    r.nInputs = model.nInputs();
    r.nSamples = sampleSize;
    if (r.nInputs < sizeof(int) * 8 - 2)
        if (sampleSize >= 1 << r.nInputs)
            return fullCompareBlifs(model, compared);
    vector<int> v(r.nInputs, 0);
    for (int i = 0; i < sampleSize; ++i) {
        for (int j = 0; j < v.size(); ++j) {
            v.at(j) = std::rand() & 1;
        }
        int o1 = model.evalAt(v);
        int o2 = compared.evalAt(v);
        if (o1 != o2) r.errorCount++;
    }
    return r;
}