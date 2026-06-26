#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <iterator>
#include "triangulation/dim2.h"
#include "triangulation/dim3.h"
#include "triangulation/dim4.h"
#include "utilities/randutils.h"
#include "triangulation/facetpairing.h"

using namespace std;
using namespace regina;

void pdm(std::string msg) {
    cerr << msg << "; ";
}

void usage(const char* progName, const string& error = string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    cerr << "Usage:" << endl;
    cerr << "      " << progName << " { isoSig } [ -c=closedOnly ] [ -i=idealOnly ] [ -r=realOnly ] \n\n";
    cerr << "       -c : Only print closed triangulations.\n";
    cerr << "       -i : Only print ideal triangulations.\n";
    cerr << "       -r : Only print triangulations with real boundary.\n";
    cerr << endl;
    exit(1);
}

bool argCharComp(char arr[], char c) {
    return arr[0] == '-' && arr[1] == c;
}

int main(int argc, char* argv[]) {
    bool closedOnly, idealOnly, realOnly;
    closedOnly = idealOnly = realOnly = false;
    
    bool preserveBdryComp = false;
    int prescribedBdryComp = 0;
    
    if (argc < 2) {
        usage(argv[0],"Error: No input given.");
    }
    if (2 < argc) {
        for (int i=2; i<argc; ++i) {
            if (argCharComp(argv[i],'c')) {
                closedOnly = true;
            }
            else if (argCharComp(argv[i],'i')) {
                idealOnly = true;
            }
            else if (argCharComp(argv[i],'r')) {
                realOnly = true;
            }
            else if (argCharComp(argv[i],'b')) {
                preserveBdryComp = true;
                if (argv[i+2] != NULL) {
                    prescribedBdryComp = stoi(argv[i]+=2);
                }
            }
            else {
                usage(argv[0],string("Invalid option: ")+argv[i]);
            }
        }
    }
    
    set<string> currSigs, intermediateSigs, closedSigs, idealSigs, realSigs;
    
    currSigs.emplace(argv[1]);
    
    while (!currSigs.empty()) {
        for (const auto& curr : currSigs) {
            Triangulation<4> tri = Triangulation<4>(curr);
            tri.orient();

            vector<pair<Simplex<4>*,int>> bdryData;
            if (preserveBdryComp) {
                for (const auto& bdryComp : tri.boundaryComponents()) {
                    if (bdryComp->index() != prescribedBdryComp) {
                        BoundaryComponent<4>* currBdryComp = bdryComp;
                        auto currFacets = currBdryComp->facets();
                        for (const auto& el : currFacets) {
                            auto emb = el->embedding(0);
                            Simplex<4>* pent = emb.simplex();
                            int currFacetIndx = emb.vertices()[4];
                            bdryData.emplace_back(pent,currFacetIndx);
                        }
                    }
                }
//                BoundaryComponent<4>* pbc = tri.boundaryComponent(prescribedBoundaryComponent);
//                auto pbcFacets = pbc->facets();
//                for (const auto& el : pbcFacets) {
//                    auto emb = el->embedding(0);
//                    Simplex<4>* pent = emb.simplex();
//                    int facet_i = emb.vertices()[4];
//                    bdryData.emplace_back(pent,facet_i);
//                    clog << pent->index() << " " << facet_i << endl;
//                }
            }
            else {
                for (Simplex<4>* pent : tri.pentachora()) {
                    for (int i=0; i<5; i++) {
                        if (pent->face<3>(i)->isBoundary()) {
                            bdryData.emplace_back(pent,i);
                        }
                    }
                }
            }
            
            for (const auto& x : bdryData) {
                for (const auto& y : bdryData) {
                    if ((x.first->index() <= y.first->index()) && (x != y)) {
                        for (int i=1; i<120; i+=2 /* Odd perms only */) {
                            Perm<5> rawPerm = Perm<5>::S5[i];
                            if (rawPerm[x.second] == y.second) {
                                x.first->join(x.second, y.first, Perm<5>(rawPerm));
                                
                                bool isBad = false;
                                
                                for (const Edge<4>* e : tri.edges()) {
                                    if (e->hasBadIdentification()) {
                                        isBad = true;
                                    }
                                    
                                    const Triangulation<2>& eLnk = e->buildLink();
                                    
                                    if (!eLnk.isOrientable()) {
                                        isBad = true;
                                    }
                                    
                                    if (eLnk.countBoundaryComponents() + eLnk.eulerChar() != 2) {
                                        isBad = true;
                                    }
                                }
                                
                                for (const auto& f : tri.triangles()) {
                                    if (f->hasBadIdentification()) {
                                        isBad = true;
                                    }
                                }
                                
                                if (!isBad) {
                                    string goodSig = tri.isoSig();
                                    if (tri.isConnected() && tri.isOrientable()) {
                                        if (!tri.hasBoundaryFacets()) {
                                            if (tri.isClosed() && !idealOnly && !realOnly) {
                                                closedSigs.emplace(goodSig);
                                            }
                                            else {
                                                if (!closedOnly && !realOnly) {
                                                    idealSigs.emplace(goodSig);
                                                }
                                            }
                                        }
                                        else {
                                            intermediateSigs.emplace(goodSig);
                                            if (tri.isValid() && !closedOnly && !idealOnly) {
                                                realSigs.emplace(goodSig);
                                            }
                                        }
                                    }
                                }
                                
                                x.first->unjoin(x.second);
                            }
                        }
                    }
                }
            }
        }
        
        currSigs.clear();
        currSigs.swap(intermediateSigs);

    }
    
    if (!closedSigs.empty()) {
        std::clog << std::endl;
        std::clog << "Closed (" << closedSigs.size() << "):" << std::endl;
        for (const auto& s : closedSigs) {
            std::cout << s << std::endl;
        }
    }
    if (!idealSigs.empty()) {
        std::clog << std::endl;
        std::clog << "Ideal (" << idealSigs.size() << "):" << std::endl;
        for (const auto& s : idealSigs) {
            std::cout << s << std::endl;
        }
    }
    if (!realSigs.empty()) {
        std::clog << std::endl;
        std::clog << "Real (" << realSigs.size() << "):" << std::endl;
        for (const auto& s : realSigs) {
            std::cout << s << std::endl;
        }
    }
    
    return 0;
}
