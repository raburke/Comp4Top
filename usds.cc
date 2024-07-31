#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include <cstring>
#include <vector>
#include "triangulation/dim4.h"
#include "utilities/randutils.h"

void pdm(std::string msg) {
    std::cerr << msg << "; ";
}

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "    " << progName << " { Isomorphism signature }" <<
    " [ -h=maxHeight] [ -w=maxWidth ] [ -e=epochs ] [ -s=seed ] [ -R ] \n\n";
    std::cerr << "    -h : Maximum number of 2-4 moves to perform (up) " <<
    "(default 10).\n";
    std::cerr << "    -w : Maximum number of 3-3 and 4-4 moves to perform (side) "
    << "(default 30). \n";
    std::cerr << "    -e : Number of epochs (default 10). \n";
    std::cerr << "    -s : Use a given random seed.\n";
    std::cerr << "    -R : Do *not* use random choices (random choices used by defaul).\n";
    std::cerr << "    -d : Print data during run, only print final sig at end.\n";
    std::cerr << std::endl;
    std::cerr << "Example usage: " << std::endl;
    std::cerr << progName << " mLvAwAQAPQQcfffhijgjgjkkklklllaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -h2 -w20 -e25 -s123 \n";
    //std::cerr << "    -t : Number of threads to use. \n";
    exit(1);
}

regina::Triangulation<4> down(regina::Triangulation<4> T) {
    std::vector<regina::Edge<4>*> edgeTwoZeroVector;
    std::vector<regina::Triangle<4>*> triangleTwoZeroVector;

    bool tryEdgeDescent = true;
    bool tryTriangleDescent = true;
    
    while (tryEdgeDescent) {
        
        edgeTwoZeroVector.clear();
       
        for (regina::Edge<4>* edge : T.edges()) {
            if (T.twoZeroMove(edge, true, false)) {
                edgeTwoZeroVector.emplace_back(edge);
            }
        }
    
        for (regina::Edge<4>* edge : edgeTwoZeroVector) {
            T.twoZeroMove(edge, false, true);
            break;
        }

        if (edgeTwoZeroVector.empty()) {
            tryEdgeDescent = false;
            break;
        }

    }
    
    while (tryTriangleDescent) {

        triangleTwoZeroVector.clear();
        
        for (regina::Triangle<4>* triangle : T.triangles()) {
            if (T.twoZeroMove(triangle, true, false)) {
                triangleTwoZeroVector.emplace_back(triangle);
            }
        }
         
        for (regina::Triangle<4>* triangle : triangleTwoZeroVector) {
            T.twoZeroMove(triangle, false, true);
            break;
        }

        if (triangleTwoZeroVector.empty()) {
            tryTriangleDescent = false;
            break;
        }
    }
    
    return T;
}

regina::Triangulation<4> side(regina::Triangulation<4> T, int cap, bool rnd) {
    int sideAttempt = 0;
    unsigned long threeThreeAvailableSize, fourFourAvailableSize;
    
    std::vector<regina::Triangle<4>*> threeThreeAvailable;
    std::vector<regina::Edge<4>*> fourFourAvailable;
    
    regina::Triangle<4>* threeThreeChoice;
    regina::Edge<4>* fourFourChoice;
    
    while (sideAttempt < cap) {
        
        // First try 3-3 moves (do the cheaper move first).
        threeThreeAvailable.clear();
        for (regina::Triangle<4>* tri : T.triangles()) {
            if (T.pachner(tri,true,false)) {
                threeThreeAvailable.emplace_back(tri);
            }
        }
        
        if (!threeThreeAvailable.empty()) {
            threeThreeAvailableSize = threeThreeAvailable.size();
            if (rnd) {
                threeThreeChoice = threeThreeAvailable[rand()%threeThreeAvailableSize];
            }
            else {
                threeThreeChoice = threeThreeAvailable[sideAttempt%threeThreeAvailableSize];
            }
            
            T.pachner(threeThreeChoice, false, true);
            T = down(T);
        }
        
        // Now try 4-4 moves.
        fourFourAvailable.clear();
        for (regina::Edge<4>* e : T.edges()) {
            if (T.fourFourMove(e,true,false)) {
                fourFourAvailable.emplace_back(e);
            }
        }
        
        if (!fourFourAvailable.empty()) {
            fourFourAvailableSize = fourFourAvailable.size();
            if (rnd) {
                fourFourChoice = fourFourAvailable[rand()%fourFourAvailableSize];
            }
            else {
                fourFourChoice = fourFourAvailable[sideAttempt%fourFourAvailableSize];
            }
            
            T.fourFourMove(fourFourChoice, false, true);
            T = down(T);
        }
        
        sideAttempt++;
    }
    
    return T;
    
}

bool argCharComp(char arr[], char c) {
    return arr[0] == '-' && arr[1] == c;
}

int main(int argc, char* argv[]) {
    int twoFourCap = 10;
    int sideCap = 30;
    int epochs = 10;
    bool useRandom = true;
    bool useUserSeed = false;
    long rndSeed, userSeed;
    bool dataOnly = false;
    
    if (argc < 2) {
        usage(argv[0], "Error: No isomorphism signature provided.");
    }
    if (2 < argc) {
        for (int i=2; i<argc; ++i) {
            if (argCharComp(argv[i],'h')) {
                twoFourCap = std::stoi(argv[i]+=2);
            }
            else if (argCharComp(argv[i],'w')) {
                sideCap = std::stoi(argv[i]+=2);
            }
            else if (argCharComp(argv[i],'e')) {
                epochs = std::stoi(argv[i]+=2);
            }
            else if (argCharComp(argv[i],'s')) {
                useUserSeed = true;
                userSeed = std::stoi(argv[i]+=2);
            }
            else if (!strcmp(argv[i],"-R")) {
                useRandom = false;
            }
            else if (!strcmp(argv[i],"-d")) {
                dataOnly = true;
            }
            else {
                usage(argv[0], std::string("Invalid option: ")+argv[i]);
            }
        }
    }
    
    if (useRandom) {
        if (!useUserSeed) {
            rndSeed = (long)time(0);
            std::cerr << "Random seed: " << rndSeed << std::endl;
            srand(rndSeed);
        }
        else {
            std::cerr << "Using seed " << userSeed << std::endl;
            srand(userSeed);
        }

    }
    
    int currentEpoch = 1;
    
    std::string initSig, newSig;
    initSig = newSig = argv[1];
    
    regina::Triangulation<4> working = regina::Triangulation<4>::fromIsoSig(initSig);
    
    ssize_t newEdgeCount, oldEdgeCount, initEdgeCount;
    oldEdgeCount = newEdgeCount = initEdgeCount = working.countEdges();
    /*
     oldEdgeCount and newEdgeCount are dynamic:
     each time we successfully simplify T
     oldEdgeCount gets updated to newEdgeCount,
     and we repeat.
     
     initEdgeCount is fixed, we use it for a final
     comparison at the end.
     */
    
    int attempts;
    double timeTaken;
    
    int twoFourAttempts = 0;
    unsigned long twoFourAvailableSize;
    std::vector<regina::Tetrahedron<4>*> twoFourAvailable;
    regina::Tetrahedron<4>* twoFourChoice;
    
    //size_t edgeDiff = newEdgeCount - oldEdgeCount;

    auto startTime = std::chrono::system_clock::now();
    
    while (currentEpoch < epochs) {

        working = regina::Triangulation<4>::fromIsoSig(newSig);
        
        working = down(working); // Try going down first if we can.
        newEdgeCount = working.countEdges();
        if (newEdgeCount < oldEdgeCount) {
            newSig = working.isoSig();
            if (!dataOnly) {
                std::cout << newSig << std::endl;
            }
            std::cerr << currentEpoch << "," <<  
//            oldEdgeCount - newEdgeCount << "," <<
            oldEdgeCount << "," << newEdgeCount << ",0" << std::endl;
            oldEdgeCount = newEdgeCount;
        }
        
        working = side(working, sideCap, useRandom);
        newEdgeCount = working.countEdges();
        if (newEdgeCount < oldEdgeCount) {
            newSig = working.isoSig();
            if (!dataOnly) {
                std::cout << newSig << std::endl;
            }
            std::cerr << currentEpoch << "," <<  
//            oldEdgeCount - newEdgeCount << "," <<
            oldEdgeCount << "," << newEdgeCount << ",0" << std::endl;
            oldEdgeCount = newEdgeCount;
        }
        
        twoFourAttempts = 0;
        while (twoFourAttempts < twoFourCap) {
            for (int i=0; i<twoFourAttempts; i++) {
                twoFourAvailable.clear();
                for (regina::Tetrahedron<4>*tet : working.tetrahedra()) {
                    if (working.pachner(tet,true,false)) {
                        twoFourAvailable.emplace_back(tet);
                    }
                }
                
                twoFourAvailableSize = twoFourAvailable.size();
                
                if (useRandom) {
                    twoFourChoice = twoFourAvailable[rand()%twoFourAvailableSize];
                }
                else {
                    twoFourChoice = twoFourAvailable[i%twoFourAvailableSize];
                }
                
                working.pachner(twoFourChoice, false, true);
            }
            
            working = down(working);
            newEdgeCount = working.countEdges();
            if (newEdgeCount < oldEdgeCount) {
                newSig = working.isoSig();
                if (!dataOnly) {
                    std::cout << newSig << std::endl;
                }
                std::cerr << currentEpoch << "," <<  
//                oldEdgeCount - newEdgeCount << "," <<
                oldEdgeCount << "," << newEdgeCount << "," << twoFourAttempts << std::endl;
//                std::cerr << currentEpoch << " | " << oldEdgeCount - newEdgeCount << " | 2-4: " << twoFourAttempts << std::endl;
                oldEdgeCount = newEdgeCount;
            }
            
            working = side(working, sideCap, useRandom);
            newEdgeCount = working.countEdges();
            if (newEdgeCount < oldEdgeCount) {
                newSig = working.isoSig();
                if (!dataOnly) {
                    std::cout << newSig << std::endl;
                }
//                std::cerr << currentEpoch << " | " << oldEdgeCount - newEdgeCount << " | 2-4: " << twoFourAttempts << std::endl;
                std::cerr << currentEpoch << "," <<  
//                oldEdgeCount - newEdgeCount << "," <<
                oldEdgeCount << "," << newEdgeCount << "," << twoFourAttempts << std::endl;
                oldEdgeCount = newEdgeCount;
            }
            
            twoFourAttempts++;
        }
        
        newSig = working.isoSig();
        
        currentEpoch+=1;
    }
    
    auto endTime = std::chrono::system_clock::now();
        
    std::chrono::duration<double> elapsedSeconds = endTime-startTime;
    std::time_t startStamp = std::chrono::system_clock::to_time_t(startTime);
    std::time_t endStamp = std::chrono::system_clock::to_time_t(endTime);
    std::cerr << "Started " << std::ctime(&startStamp)
            << "Finished " << std::ctime(&endStamp)
            << "Elapsed time: " << elapsedSeconds.count() << "s"
            << std::endl;
    auto endEdgeCount = working.countEdges();
    auto totalReduction = initEdgeCount - endEdgeCount;
    std::cerr << "Final iso sig:" << std::endl;
    std::cout << working.isoSig() << std::endl;

    if (initEdgeCount - endEdgeCount <= 0) {
        std::cerr << "Failed to simplify this run." << std::endl;
    }
    else {
        std::cerr << "Reduced " << totalReduction << " edges (" << initEdgeCount << " -> " << endEdgeCount <<")." << std::endl;
    }
    return 0;
}
