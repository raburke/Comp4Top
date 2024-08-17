/*
 Brute Force Builder (Basic/Stripped Back Version for BAB)
 
 cout: Intermediate sigs.
 clog: Everything else (including "end"/"finished" sigs).
 */

#include <iostream>
#include <vector>
#include <array>
#include <iterator>
#include <fstream>
#include <string>
#include <cstring>
#include "triangulation/dim2.h"
#include "triangulation/dim3.h"
#include "triangulation/dim4.h"
#include "triangulation/generic/facetpairing.h"
#include <chrono>

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "    " << progName << " { Isomorphism signature | File of isomorphism signatures }  [ (-i | -r)=secondIsoSig ] [ -s ] \n\n";
    std::cerr << "    -i : Insert a copy of the triangulation represented by the string \"secondIsoSig\" into the initially loaded triangulation. \n";
    std::cerr << "    -r : Repeatedly insert a copy of the triangulation represented by the string \"secondIsoSig\" into the initial triangulation on each run. \n";
    std::cerr << "    -s : Only perform a single pair of gluings (incompatible with the -r option). \n";
    exit(1);
}

int main(int argc, char* argv[]) {
    
    std::set<std::string> sigs;
    
    bool isFile = false;
    bool singleGluing = false;
    bool insertTri = false;
    std::string insertTriSig;
    bool repeatInsert = false;
    bool runRepeatCheck = false;
    
    if (argc < 2) {
        usage(argv[0], "ERROR: Please provide an isomorphism signature or file of isomorphism signatures.");
    }
    else if (2 <= argc) {
        for (int i=2; i<argc; ++i) {
            /* For some reason when I started stripping back to get this version,
            strncpy (I think) was giving me garbage so using the more clunky approach
             you see here.*/
//            char argShort[3];
//            strncpy(argShort,argv[i],3);
            if (argv[i][1] == 'i') {
                insertTri = true;
                insertTriSig = argv[i]+=3;
            }
            else if (argv[i][1] == 'r' && argv[i][2] == '=') {
                insertTri = true;
                repeatInsert = true;
                insertTriSig = argv[i]+=3;
            }
            else if (!strcmp(argv[i], "-r")) {
                repeatInsert = true;
                runRepeatCheck = true;
            }
            else if (!strcmp(argv[i], "-s")) {
                singleGluing = true;
            }
            else {
                usage(argv[0], std::string("Invalid option: ") + argv[i]);
            }
        }
    }
       
    if (runRepeatCheck) {
        if (repeatInsert && !insertTri) {
            usage(argv[0], std::string("ERROR: -r must either be used in conjunction with -i or of the form -r=secondIsoSig."));
        }
    }

    std::string arg1 = argv[1];
    for (char &c : arg1) {
        if (c == '.') {
            isFile = true;
            break;
        }
    }
        
    if (isFile) {
        std::ifstream sigsInputFile (arg1);
        std::string currentSigRead;
        if (sigsInputFile.is_open()) {
            while (getline(sigsInputFile,currentSigRead)) {
                if (insertTri && !repeatInsert) {
                    sigs.emplace(insertTriSig+currentSigRead);
                }
                else {
                    sigs.emplace(currentSigRead);
                }
                
            }
            sigsInputFile.close();
        }
        else {
            usage(argv[0], std::string("ERROR: Could not open isomorphism signature file ") + arg1);
        }
    }
    else {
        if (insertTri && !repeatInsert) {
            sigs.emplace(insertTriSig+arg1);
        }
        else {
            sigs.emplace(arg1);
        }
    }
        
    std::set<std::string> intSigs;
    std::set<std::string> closedSigs, idealSigs;
           
    auto t1 = std::chrono::high_resolution_clock::now();
        
    while (! sigs.empty()) {
    
        for (const auto& currentSig : sigs) {

            regina::Triangulation<4> tri;
                        
            if (repeatInsert) {
                std::string augmentedSig = insertTriSig+currentSig;
                tri = regina::Triangulation<4>::fromIsoSig(augmentedSig);
            }
            else {
                tri = regina::Triangulation<4>::fromIsoSig(currentSig);
            }
                        
            tri.orient();
            std::vector<std::pair<regina::Simplex<4>*,int>> boundaryData;

            for (regina::Simplex<4>* pent : tri.pentachora()) {
                for (int j=0; j<5; j++) {
                    if (pent->face<3>(j)->isBoundary()) {
                        boundaryData.emplace_back(pent,j);
                    }
                }
            }
                        
            for (const auto& xi : boundaryData) {
                for (const auto& xj : boundaryData) {
                    if ((xi.first->index() <= xj.first->index()) && (xi != xj)) {
                        for (int s5index = 1; s5index < 120; s5index += 2 /* Odd perms only */) {
                            regina::Perm<5> rawPerm = regina::Perm<5>::S5[s5index];
                            if (rawPerm[xi.second] == xj.second) {

                                xi.first->join(xi.second, xj.first, regina::Perm<5>(rawPerm));

                                bool isBad = false;
                                
                                /* Don't think we actually need this because it should be covered by the edge link check, right? */
//                                for (regina::Vertex<4>* v : tri.vertices()) {
//                                    const regina::Triangulation<3>& vLink = v->buildLink();
//                                    if (!vLink.isOrientable()) {
//                                        isBad = true;
//                                    }
//                                }
                                
                                for (regina::Edge<4>* e : tri.edges()) {
                                    
                                    if (e->hasBadIdentification()) {
                                        isBad = true;
                                    }
                                    
                                    const regina::Triangulation<2>& link = e->buildLink();
                                    
                                    if (!link.isOrientable()) {
                                        isBad = true;
                                    }
                                    if (link.countBoundaryComponents() + link.eulerChar() != 2) {
                                        isBad = true;
                                    }
                                }
                                
                                for (const auto& f : tri.triangles()) {
                                    if (f->hasBadIdentification()) {
                                        isBad = true;
                                    }
                                }
                                if (!isBad) {
                                    std::string newSig = tri.isoSig();
                                    if (tri.isConnected() && tri.isOrientable()) {
                                        if (!tri.hasBoundaryFacets()) {
                                            if (tri.isClosed()) {
                                                closedSigs.emplace(newSig);
                                            }
                                            else {
                                                idealSigs.emplace(newSig);
                                            }
                                        }
                                        else {
                                            if (intSigs.emplace(newSig).second) {
                                                std::cout << newSig << std::endl;
                                            }
                                        }
                                    }
                                }
                                
                                xi.first->unjoin(xi.second);
                                
                            }
                        }
                    }
                }
            }
        }
        
        if (singleGluing) {
            sigs.clear();
            break;
        }
        else {
            sigs.clear();
            sigs.swap(intSigs);
        }
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> ms_double = t2 - t1;
    
    if (!closedSigs.empty()) {
        std::clog << std::endl;
        std::clog << "Closed (" << closedSigs.size() << "):" << std::endl;
        for (const auto& s : closedSigs) {
            std::clog << s << std::endl;
        }
    }
    if (!idealSigs.empty()) {
        std::clog << std::endl;
        std::clog << "Ideal (" << idealSigs.size() << "):" << std::endl;
        for (const auto& s : idealSigs) {
            std::clog << s << std::endl;
        }
    }
    auto totalNumFound = closedSigs.size() + idealSigs.size();
    if (totalNumFound != 0) {
        std::clog << std::endl;
        std::clog << "Total number found: " << totalNumFound << " (" << closedSigs.size() << " Closed, " << idealSigs.size() << " Ideal)" << std::endl;
        std::clog << std::endl;
        std::clog << "Time taken: " << ms_double.count()/60000 << std::endl;
    }

    return 0;
}
