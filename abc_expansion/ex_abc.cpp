//
// Created by tripack on 17-3-25.
//

#include <src/base/main/main.h>
#include "ex_abc.h"
#include <common.h>

extern "C" {
    #include <src/base/main/main.h>
}

ExAbc* ExAbc::instance = nullptr;

ExAbc& ExAbc::getInstance() {
    if (instance == nullptr)
        instance = new ExAbc;
    return *instance;
}

ExAbc::ExAbc() {
    Abc_Start();
    abcFramework = Abc_FrameGetGlobalFrame();

    this->execute("read_library " + DataPath.sub("mvsis") / "mcnc.bin");
}

ExAbc::~ExAbc() {
    empty();
    Abc_Stop();
    delete instance;
}

int ExAbc::execute(const std::string &str) {
    return Cmd_CommandExecute(
            (Abc_Frame_t*)abcFramework,
            str.c_str()
    );
}

int ExAbc::loadBlif(const std::string &file) {
    //std::cout << "ABC loading " << file << std::endl;
    std::string quotedFile = "'" + file + "'";
    return execute("read_blif " + quotedFile);
}

int ExAbc::writeBlif(const std::string &file) {
    std::string quotedFile = "'" + file + "'";
    return execute("write_blif " + quotedFile);
}

int ExAbc::toAig() {
    return execute("strash");
}

std::map<std::string, std::set<std::string> > ExAbc::Mffc() {
    std::map<std::string, std::set<std::string> > mffc;
    Abc_Ntk_t *pNtk = Abc_FrameReadNtk((Abc_Frame_t*)abcFramework);
    Abc_Obj_t *pNode;
    int i;
    Abc_NtkForEachNode(pNtk, pNode, i) {
            Vec_Ptr_t * vCone, * vSupp;
            Abc_Obj_t * pObj;
            int j;
            vCone = Vec_PtrAlloc( 100 );
            vSupp = Vec_PtrAlloc( 100 );
            Abc_NodeDeref_rec( pNode );
            Abc_NodeMffcConeSupp( pNode, vCone, vSupp );
            Abc_NodeRef_rec( pNode );
            std::set<std::string> mffc_set;
            Vec_PtrForEachEntry( Abc_Obj_t *, vCone, pObj, j )
                mffc_set.insert(Abc_ObjName(pObj));
            mffc.insert(std::make_pair(
                    Abc_ObjName(pNode),
                    mffc_set
            ));
            Vec_PtrFree( vCone );
            Vec_PtrFree( vSupp );
        }
    return mffc;
}

int ExAbc::nGates() {
    Abc_Ntk_t *pNtk = Abc_FrameReadNtk((Abc_Frame_t*)abcFramework);
    int nSingles = Abc_NtkGetBufNum(pNtk);

    //Abc_Print( 1," i/o =%5d/%5d", Abc_NtkPiNum(pNtk), Abc_NtkPoNum(pNtk) );
    if ( Abc_NtkIsNetlist(pNtk) ) {
        return Abc_NtkNodeNum(pNtk);
    }
    else if ( Abc_NtkIsStrash(pNtk) ) {
        return Abc_NtkNodeNum(pNtk);
    }
    else {
        return Abc_NtkNodeNum(pNtk);
    }
}

int ExAbc::empty() {
    return execute("empty");
}

void ExAbc::resyn2() {
    //alias resyn2 "b; rw; rf;
    //              b; rw; rwz;
    //              b; rfz; rwz;
    //              b"
    balance();
    rewrite();
    refactor();

    balance();
    rewrite();
    rewrite(true);

    balance();
    refactor(true);
    rewrite(true);

    balance();

}

void ExAbc::rewrite(bool z) {
    if (z)
        this->execute("rewrite -z");
    else
        this->execute("rewrite");
}

void ExAbc::balance() {
    this->execute("balance");
}

void ExAbc::refactor(bool z) {
    if (z)
        this->execute("refactor -z");
    else
        this->execute("refactor");
}

void ExAbc::map() {
    this->execute("sweep");
    this->execute("strash");
    this->execute("map");
}

void ExAbc::reboot() {
    Abc_Stop();
    Abc_Start();
    abcFramework = Abc_FrameGetGlobalFrame();
    this->execute("read_library " + DataPath.sub("mvsis") / "mcnc.bin");

}

double ExAbc::postMapArea() {
    Abc_Ntk_t *pNtk = Abc_FrameReadNtk((Abc_Frame_t*)abcFramework);
    return Abc_NtkGetMappedArea(pNtk);;
}


