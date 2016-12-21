#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "bnet.h"
#include "searchMFFC.h"
#include "print_truth_table.h"

#include <truth_table.h>
#include "interface.h"

using namespace std;

typedef long long unsigned int ulli;
ulli power2(int power);
void getBits(ulli n, int* vec, int digit);

string getMFFC(string infile, int minInput, int maxInput) {
    FILE *fp;
    fp = fopen(infile.c_str(), "r");
    BnetNetwork *net = Bnet_ReadNetwork(fp);
    if (net == NULL) {
        cout << "Syntax error in " << infile << endl;
        exit(2);
    }
    fclose(fp);

    stringstream s;
    searchMFFC(net, s, minInput, maxInput);
    Bnet_FreeNetwork(net);
    return s.str();
}

BlifBooleanNet::BlifBooleanNet(const std::string &file) {
    filename = file;

    ddmanager = NULL;		/* pointer to DD manager */
    FILE *fp;
    fp = fopen(file.c_str(), "r");
    if (fp == NULL) assert(0);
    net = Bnet_ReadNetwork(fp);

    ddmanager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    if (ddmanager == NULL) exit(-2);
    cudd_build_v2(net, &ddmanager, file.c_str(), BNET_GLOBAL_DD);

    fclose(fp);
}

int BlifBooleanNet::nInputs() const {
    return net->ninputs;
}

BlifBooleanNet::~BlifBooleanNet() {
    Bnet_FreeNetwork_Bdd(net, ddmanager);
    Cudd_Quit(ddmanager);
}

TruthTable BlifBooleanNet::truthTable() const {
    BnetNode *auxnd;
    st_lookup(net->hash,net->outputs[0],&auxnd);
    DdNode *ddnode_pt = auxnd->dd;

    int nInputs = net->ninputs;
    vector<int> out;

    ulli max = power2(nInputs);
    int *ivec = new int[nInputs];
    for (ulli i = 0; i < max ; ++i) {
        getBits(i, ivec, nInputs);
        int n =
                (Cudd_ReadOne(ddmanager) == Cudd_Eval(ddmanager, ddnode_pt, ivec));
        out.push_back(n);
    }
    delete[] ivec;

    TruthTable table(nInputs, out);
    for (int j = 0; j < nInputs; ++j) {
        table.getName(j) = net->inputs[j];
    }
    table.outName = auxnd->name;

    return table;
}

int BlifBooleanNet::evalAt(const std::vector<int> &v,
                           const std::string& name) const {
    if (v.size() != net->ninputs)
        assert(0);
    BnetNode *auxnd;
    st_lookup(net->hash,net->outputs[0],&auxnd);
    DdNode *ddnode_pt = auxnd->dd;
    int n = (Cudd_ReadOne(ddmanager)
             == Cudd_Eval(ddmanager, ddnode_pt, (int*)(v.data()) ));
    return n;
}

const std::string &BlifBooleanNet::name() const {
    return filename;
}

int BlifBooleanNet::nodeCount() const {
    int count = 0;
    BnetNode *n = net->nodes;
    while (n != NULL) {
        count++;
        n = n->next;
    }
    return count;
}

int BlifBooleanNet::gateCount() const {
    int c = nodeCount();
    int n = c - net->npis - net->npos;
    return n >= 0 ? n : 0;
}

const std::string BlifBooleanNet::netName() const {
    return net->name;
}

BlifBooleanNet BlifBooleanNet::getMFFC(int minInput,
                                       int maxInput) const {
    assert(minInput >= 0);
    assert(maxInput >= minInput);
    std::string fname = std::string("mffc.blif");
    ofstream fs(fname);
    searchMFFC(net, fs, minInput, maxInput);
    fs.close();
    return BlifBooleanNet(fname);
}

int BlifBooleanNet::nOutputs() const {
    return net->noutputs;
}

std::set<std::string> BlifBooleanNet::inputNodeSet() const {
    std::set<std::string> s;
    for (int i = 0; i < net->ninputs; ++i) {
        s.insert(net->inputs[i]);
    }
    return s;
}

std::set<std::string> BlifBooleanNet::outputNodeSet() const {
    std::set<std::string> s;
    for (int i = 0; i < net->noutputs; ++i) {
        s.insert(net->outputs[i]);
    }
    return s;
}

std::set<std::string> BlifBooleanNet::totalNodeSet() const {
    std::set<std::string> s;
    BnetNode *node = net->nodes;
    while (node != NULL) {
        s.insert(node->name);
        node = node->next;
    }
    return s;
}

std::vector<int> BlifBooleanNet::evalAllOutputAt(const std::vector<int> &v) const {
    if (v.size() != net->ninputs)
        assert(0);
    std::vector<int> out;
    for (int i = 0; i < net->noutputs; ++i) {
        BnetNode *auxnd;
        st_lookup(net->hash, net->outputs[i], &auxnd);
        DdNode *ddnode_pt = auxnd->dd;
        int n = (Cudd_ReadOne(ddmanager)
                 == Cudd_Eval(ddmanager, ddnode_pt, (int *) (v.data())));
        out.push_back(n);
    }
    return out;
}

ulli power2(int power) {
    return ((ulli)1) << power;
}

void getBits(ulli n, int* vec, int digit) {
    for (int i = 0; i < digit; ++i) {

        if ((n & power2(i)) > 0)
            vec[i] = 1;
        else
            vec[i] = 0;
    }
}