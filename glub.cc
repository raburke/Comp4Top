#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include "triangulation/dim4.h"
#include "triangulation/dim3.h"
#include "triangulation/triangulation.h"

using namespace regina;

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "      " << progName
        << " { isoSig } [ isoSig2 ]"
        << " [ --orientation=<preserving,reversing> ]\n";
    std::cerr << std::endl;
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage(argv[0], "Error: No sig provided.");
    }
    if (4 < argc) {
        usage(argv[0], "Error: Expected at most 3 arguments.");
    }

    std::string sig1, sig2;
    int orientation = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        const std::string prefix = "--orientation=";
        if (arg.rfind(prefix, 0) == 0) {
            std::string value = arg.substr(prefix.size());
            if (value == "preserving") {
                orientation = 1;
            } else if (value == "reversing") {
                orientation = -1;
            } else {
                usage(argv[0], "Error: Unknown orientation.");
            }
        } else if (arg.rfind("--", 0) == 0) {
            usage(argv[0], "Error: Unknown option.");
        } else if (sig1.empty()) {
            sig1 = arg;
        } else if (sig2.empty()) {
            sig2 = arg;
        } else {
            usage(argv[0], "Error: Expected at most 2 isoSig arguments.");
        }
    }

    if (sig1.empty()) {
        usage(argv[0], "Error: No sig provided.");
    }

    std::string sig = sig1 + sig2;
    
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
    size_t unfilteredIsos = 0;
    b0b.findAllIsomorphisms(b1b, [&allIsos, &orientation, &unfilteredIsos](
        const Isomorphism<3>& currIso) {
            ++unfilteredIsos;
            if (orientation == 1 && !currIso.isEven()) {
                return false;
            }
            if (orientation == -1 && currIso.isEven()) {
                return false;
            }
            allIsos.emplace_back(currIso);
            return false;
        }
    );
    
    if (unfilteredIsos == 0) {
        std::cerr << "Error: Boundaries are not combinatorially isomorphic!\n";
        exit(1);
    }
    if (allIsos.empty()) {
        std::cerr << "Error: No boundary isomorphisms match the requested "
            << "orientation.\n";
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
        // Perm<5> ansPerm;
		for (int i=0; i<currIso.size(); i++) {
            // TODO: Document construction of permutation.
            Perm<5> perm = b1->facet(currIso.simpImage(i))->embedding(0).vertices() * Perm<5>::extend(currIso.facetPerm(i)) * b0->facet(i)->embedding(0).vertices().inverse();
            
            Simplex<4>* me = working.pentachoron(b0Simps[i]);
            Simplex<4>* you = working.pentachoron(b1Simps[currIso.simpImage(i)]);
            int facet = b0Facets[i];
            
            me->join(facet,you,perm);
			// ansPerm = perm;
        }
        std::cout << working.isoSig() << std::endl;
    }
    
    return 0;
}
