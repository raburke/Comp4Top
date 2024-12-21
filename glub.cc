#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include "triangulation/dim4.h"
#include "triangulation/dim3.h"
#include "triangulation/detail/triangulation.h"

using namespace regina;

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "      " << progName << " { isoSig } \n";
    std::cerr << std::endl;
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage(argv[0], "Error: No sig provided.");
    }
    
    std::string sig = argv[1];
    
    const Triangulation<4>& tri = Triangulation<4>::fromIsoSig(sig);
    
    /*
     Begin sanity checking
     */
    if (!tri.hasBoundaryFacets()) {
        std::cerr << "Error: Triangulation does not have any real boundary.\n";
        std::cerr << "       Truncate any ideal boundary components or\n";
        std::cerr << "       choose a different triangulation.\n";
        exit(1);
    }
    int numBdryComps = tri.countBoundaryComponents();
    int realCount = 0;

    std::vector<int> bdryIndices;
    if (numBdryComps < 2) {
        std::cerr << "Error: Triangulation does not have at least 2 boundary components.\n";
        exit(1);
    }
    for (int i=0; i<numBdryComps; i++) {
        const auto& currBdry = tri.boundaryComponent(i);
        if (currBdry->isReal()) {
            bdryIndices.emplace_back(i);
        }
    }
    if (bdryIndices.size() != 2) {
        std::cerr << "Error: Triangulation does not have 2 real boundary components.\n";
        exit(1);
    }
    int b0Indx, b1Indx;
    b0Indx = bdryIndices[0];
    b1Indx = bdryIndices[1];

    const auto& b0 = tri.boundaryComponent(b0Indx);
    const auto& b1 = tri.boundaryComponent(b1Indx);

    if (b0->size() != b1->size()) {
        std::cerr << "Error: Boundary components are different sizes!\n";
        exit(1);
    }

    const auto& b0b = b0->build();
    const auto& b1b = b1->build();
    
    std::vector<Isomorphism<3>> allIsos;
    const auto& findAllIsos = b0b.findAllIsomorphisms(b1b, [&allIsos](
        const Isomorphism<3>& currIso) {
            allIsos.emplace_back(currIso);
            return false;
        }
    );
    
    if (allIsos.empty()) {
        std::cerr << "Error: Boundaries are not combinatorially isomorphic!\n";
        exit(1);
    }
    
    /*
     End sanity checking
     */
        
    std::vector<int> b0Simps, b1Simps;
    std::vector<Face<4,3>> b0Embs, b1Embs;
    std::vector<int> b0Facets;
    for (int i=0; i<b0->size(); i++) {
        int b0CurrSimp = b0->facet(i)->embedding(0).simplex()->index();
        int b1CurrSimp = b1->facet(i)->embedding(0).simplex()->index();
        b0Simps.emplace_back(b0CurrSimp);
        b1Simps.emplace_back(b1CurrSimp);
        int b0CurrFacet = b0->facet(i)->embedding(0).vertices()[4];
        b0Facets.emplace_back(b0CurrFacet);
    }
    
    for (const auto& currIso : allIsos) {
        Triangulation<4> working(tri);
        for (int i=0; i<currIso.size(); i++) {
            // TODO: Document construction of permutation.
            Perm<5> perm = b1->facet(currIso.simpImage(i))->embedding(0).vertices() * Perm<5>::extend(currIso.facetPerm(i)) * b0->facet(i)->embedding(0).vertices().inverse();
            
            Simplex<4>* me = working.pentachoron(b0Simps[i]);
            Simplex<4>* you = working.pentachoron(b1Simps[currIso.simpImage(i)]);
            int facet = b0Facets[i];
            
            me->join(facet,you,perm);
        }
        std::cout << working.isoSig() << std::endl;
    }
    
    return 0;
}

