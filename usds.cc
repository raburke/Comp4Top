#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>
#include <future>
#include "triangulation/dim4.h"
#include "utilities/randutils.h"

struct MoveResult {
    regina::Triangulation<4> triangulation;
    std::vector<std::string> moves;
};

std::string describeMove(const std::string& moveType, const std::string& objectType,
        size_t index) {
    std::ostringstream out;
    out << moveType << "(" << objectType << "=" << index << ")";
    return out.str();
}

std::string formatMoves(const std::vector<std::string>& moves) {
    if (moves.empty()) {
        return "\nnone";
    }

    std::ostringstream out;
    for (size_t i = 0; i < moves.size(); ++i) {
        out << "\n" << moves[i];
    }
    return out.str();
}

void clearProgressLine(bool progress) {
    if (progress) {
        std::cerr << "\r" << std::string(120, ' ') << "\r" << std::flush;
    }
}

void printProgress(bool progress, int epoch, int epochs,
        const std::string& phase, const regina::Triangulation<4>& T,
        ssize_t bestEdgeCount, int twoFourAttempt = -1,
        int twoFourCap = -1) {
    if (!progress) {
        return;
    }

    std::cerr << "\r" << std::string(120, ' ') << "\r"
        << "epoch " << epoch << "/" << epochs
        << " | " << phase
        << " | edges " << T.countEdges()
        << " | size " << T.size()
        << " | best edges " << bestEdgeCount;
    if (twoFourAttempt >= 0) {
        std::cerr << " | 2-4 " << twoFourAttempt << "/" << twoFourCap;
    }
    std::cerr << std::flush;
}

void pdm(std::string msg) {
    std::cerr << msg << "; ";
}

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "    " << progName << " { neoSig }" <<
    " [ -h=maxHeight] [ -w=maxWidth ] [ -e=epochs ] [ -s=seed ] [ -R ] \n\n";
    std::cerr << "    -h : Maximum number of 2-4 moves to perform (up) " <<
    "(default 10).\n";
    std::cerr << "    -w : Maximum number of 3-3 and 4-4 moves to perform (side) "
    << "(default 30). \n";
    std::cerr << "    -e : Number of epochs (default 10). \n";
    std::cerr << "    -s : Use a given random seed.\n";
    std::cerr << "    -R : Do *not* use random choices (random choices used by defaul).\n";
    std::cerr << "    -d : Print data during run, only print final sig at end.\n";
    std::cerr << "    --verbose : Print the move log for each successful simplification.\n";
    std::cerr << "    --lookahead : Sample candidate moves and choose the best descent result.\n";
    std::cerr << "    --progress : Print a live progress line during the run.\n";
    std::cerr << "    --bulkup : Perform only the maximum-length 2-4 up sequence.\n";
    std::cerr << std::endl;
    std::cerr << "Example usage: " << std::endl;
    std::cerr << progName << " mLvAwAQAPQQcfffhijgjgjkkklklllaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -h2 -w20 -e25 -s123 \n";
    //std::cerr << "    -t : Number of threads to use. \n";
    exit(1);
}

MoveResult down(regina::Triangulation<4> T, bool verbose,
        regina::Triangulation<4>* ) {
    std::vector<std::string> moves;

    bool changed = true;

    while (changed) {
        changed = false;

        for (regina::Edge<4>* edge : T.edges()) {
            if (T.has20(edge)) {
                if (verbose) {
                    moves.emplace_back(describeMove("2-0", "edge",
                        edge->index()));
                }
                T.move20(edge);
                changed = true;
                break;
            }
        }

        if (changed) {
            continue;
        }

        for (regina::Triangle<4>* triangle : T.triangles()) {
            if (T.has20(triangle)) {
                if (verbose) {
                    moves.emplace_back(describeMove("2-0", "triangle",
                        triangle->index()));
                }
                T.move20(triangle);
                changed = true;
                break;
            }
        }
    }
    
    return { std::move(T), std::move(moves) };
}

constexpr size_t LOOKAHEAD_SAMPLES = 8;

struct LookaheadScore {
    ssize_t edges;
    size_t size;
};

struct LookaheadResult {
    size_t candidate;
    LookaheadScore score;
    bool valid;
};

bool isBetterScore(const LookaheadScore& candidate,
        const LookaheadScore& best) {
    return candidate.edges < best.edges ||
        (candidate.edges == best.edges && candidate.size < best.size);
}

size_t selectLookaheadCandidate(size_t availableSize, size_t sample,
        bool rnd, std::vector<size_t>& selected) {
    if (!rnd || availableSize <= LOOKAHEAD_SAMPLES) {
        selected.emplace_back(sample);
        return sample;
    }

    while (true) {
        size_t candidate = regina::RandomEngine::rand(availableSize);
        bool seen = false;
        for (size_t previous : selected) {
            if (previous == candidate) {
                seen = true;
                break;
            }
        }
        if (!seen) {
            selected.emplace_back(candidate);
            return candidate;
        }
    }
}

LookaheadResult evaluateThreeThreeLookahead(regina::Triangulation<4> trial,
        size_t candidate, size_t triangleIndex) {
    if (!trial.pachner(trial.triangle(triangleIndex))) {
        return { candidate, { 0, 0 }, false };
    }
    MoveResult downResult = down(std::move(trial), false, nullptr);
    return {
        candidate,
        {
            static_cast<ssize_t>(downResult.triangulation.countEdges()),
            downResult.triangulation.size()
        },
        true
    };
}

LookaheadResult evaluateFourFourLookahead(regina::Triangulation<4> trial,
        size_t candidate, size_t edgeIndex) {
    if (!trial.move44(trial.edge(edgeIndex))) {
        return { candidate, { 0, 0 }, false };
    }
    MoveResult downResult = down(std::move(trial), false, nullptr);
    return {
        candidate,
        {
            static_cast<ssize_t>(downResult.triangulation.countEdges()),
            downResult.triangulation.size()
        },
        true
    };
}

LookaheadResult evaluateTwoFourLookahead(regina::Triangulation<4> trial,
        size_t candidate, size_t tetrahedronIndex) {
    if (!trial.pachner(trial.tetrahedron(tetrahedronIndex))) {
        return { candidate, { 0, 0 }, false };
    }
    MoveResult downResult = down(std::move(trial), false, nullptr);
    return {
        candidate,
        {
            static_cast<ssize_t>(downResult.triangulation.countEdges()),
            downResult.triangulation.size()
        },
        true
    };
}

regina::Triangle<4>* chooseThreeThreeMove(regina::Triangulation<4>& T,
        const std::vector<regina::Triangle<4>*>& available, bool rnd,
        int sideAttempt, bool lookahead) {
    size_t availableSize = available.size();
    if (!lookahead) {
        if (rnd) {
            return available[regina::RandomEngine::rand(availableSize)];
        }
        return available[sideAttempt % availableSize];
    }

    size_t samples = std::min(availableSize, LOOKAHEAD_SAMPLES);
    std::vector<size_t> selected;
    std::vector<std::future<LookaheadResult>> futures;
    futures.reserve(samples);
    size_t bestChoice = 0;
    LookaheadScore bestScore { -1, 0 };

    for (size_t sample = 0; sample < samples; ++sample) {
        size_t candidate = selectLookaheadCandidate(availableSize, sample,
            rnd, selected);
        size_t triangleIndex = available[candidate]->index();
        futures.emplace_back(std::async(std::launch::async,
            evaluateThreeThreeLookahead, regina::Triangulation<4>(T),
            candidate, triangleIndex));
    }

    for (std::future<LookaheadResult>& future : futures) {
        LookaheadResult result = future.get();
        if (!result.valid) {
            continue;
        }
        if (bestScore.edges < 0 || isBetterScore(result.score, bestScore)) {
            bestScore = result.score;
            bestChoice = result.candidate;
        }
    }

    return available[bestChoice];
}

regina::Edge<4>* chooseFourFourMove(regina::Triangulation<4>& T,
        const std::vector<regina::Edge<4>*>& available, bool rnd,
        int sideAttempt, bool lookahead) {
    size_t availableSize = available.size();
    if (!lookahead) {
        if (rnd) {
            return available[regina::RandomEngine::rand(availableSize)];
        }
        return available[sideAttempt % availableSize];
    }

    size_t samples = std::min(availableSize, LOOKAHEAD_SAMPLES);
    std::vector<size_t> selected;
    std::vector<std::future<LookaheadResult>> futures;
    futures.reserve(samples);
    size_t bestChoice = 0;
    LookaheadScore bestScore { -1, 0 };

    for (size_t sample = 0; sample < samples; ++sample) {
        size_t candidate = selectLookaheadCandidate(availableSize, sample,
            rnd, selected);
        size_t edgeIndex = available[candidate]->index();
        futures.emplace_back(std::async(std::launch::async,
            evaluateFourFourLookahead, regina::Triangulation<4>(T),
            candidate, edgeIndex));
    }

    for (std::future<LookaheadResult>& future : futures) {
        LookaheadResult result = future.get();
        if (!result.valid) {
            continue;
        }
        if (bestScore.edges < 0 || isBetterScore(result.score, bestScore)) {
            bestScore = result.score;
            bestChoice = result.candidate;
        }
    }

    return available[bestChoice];
}

regina::Tetrahedron<4>* chooseTwoFourMove(regina::Triangulation<4>& T,
        const std::vector<regina::Tetrahedron<4>*>& available, bool rnd,
        int attempt, bool lookahead) {
    size_t availableSize = available.size();
    if (!lookahead) {
        if (rnd) {
            return available[regina::RandomEngine::rand(availableSize)];
        }
        return available[attempt % availableSize];
    }

    size_t samples = std::min(availableSize, LOOKAHEAD_SAMPLES);
    std::vector<size_t> selected;
    std::vector<std::future<LookaheadResult>> futures;
    futures.reserve(samples);
    size_t bestChoice = 0;
    LookaheadScore bestScore { -1, 0 };

    for (size_t sample = 0; sample < samples; ++sample) {
        size_t candidate = selectLookaheadCandidate(availableSize, sample,
            rnd, selected);
        size_t tetrahedronIndex = available[candidate]->index();
        futures.emplace_back(std::async(std::launch::async,
            evaluateTwoFourLookahead, regina::Triangulation<4>(T),
            candidate, tetrahedronIndex));
    }

    for (std::future<LookaheadResult>& future : futures) {
        LookaheadResult result = future.get();
        if (!result.valid) {
            continue;
        }
        if (bestScore.edges < 0 || isBetterScore(result.score, bestScore)) {
            bestScore = result.score;
            bestChoice = result.candidate;
        }
    }

    return available[bestChoice];
}

MoveResult side(regina::Triangulation<4> T, int cap, bool rnd, bool verbose,
        bool lookahead, regina::Triangulation<4>* logTri) {
    int sideAttempt = 0;
    std::vector<std::string> moves;
    
    std::vector<regina::Triangle<4>*> threeThreeAvailable;
    std::vector<regina::Edge<4>*> fourFourAvailable;
    
    regina::Triangle<4>* threeThreeChoice;
    regina::Edge<4>* fourFourChoice;
    
    while (sideAttempt < cap) {
        
        // First try 3-3 moves (do the cheaper move first).
        threeThreeAvailable.clear();
        for (regina::Triangle<4>* tri : T.triangles()) {
            if (T.hasPachner(tri)) {
                threeThreeAvailable.emplace_back(tri);
            }
        }
        
        if (!threeThreeAvailable.empty()) {
            threeThreeChoice = chooseThreeThreeMove(T, threeThreeAvailable,
                rnd, sideAttempt, lookahead);
            
            if (verbose) {
                moves.emplace_back(describeMove("3-3", "triangle",
                    threeThreeChoice->index()));
            }
            T.pachner(threeThreeChoice);
            MoveResult downResult = down(std::move(T), verbose, logTri);
            T = std::move(downResult.triangulation);
            if (verbose) {
                moves.insert(moves.end(), downResult.moves.begin(), downResult.moves.end());
            }
        }
        
        // Now try 4-4 moves.
        fourFourAvailable.clear();
        for (regina::Edge<4>* e : T.edges()) {
            if (T.has44(e)) {
                fourFourAvailable.emplace_back(e);
            }
        }
        
        if (!fourFourAvailable.empty()) {
            fourFourChoice = chooseFourFourMove(T, fourFourAvailable, rnd,
                sideAttempt, lookahead);
            
            if (verbose) {
                moves.emplace_back(describeMove("4-4", "edge",
                    fourFourChoice->index()));
            }
            T.move44(fourFourChoice);
            MoveResult downResult = down(std::move(T), verbose, logTri);
            T = std::move(downResult.triangulation);
            if (verbose) {
                moves.insert(moves.end(), downResult.moves.begin(), downResult.moves.end());
            }
        }
        
        sideAttempt++;
    }
    
    return { std::move(T), std::move(moves) };
    
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
    bool verbose = false;
    bool useLookahead = false;
    bool showProgress = false;
    bool bulkUp = false;
    
    if (argc < 2) {
        usage(argv[0], "Error: No input provided.");
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
            else if (!strcmp(argv[i],"--verbose")) {
                verbose = true;
            }
            else if (!strcmp(argv[i],"--lookahead")) {
                useLookahead = true;
            }
            else if (!strcmp(argv[i],"--progress")) {
                showProgress = true;
            }
            else if (!strcmp(argv[i],"--bulkup")) {
                bulkUp = true;
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
        }
        else {
            std::cerr << "Using seed " << userSeed << std::endl;
            rndSeed = userSeed;
        }
        {
            regina::RandomEngine engine;
            engine.engine().seed(rndSeed);
        }

    }
    
    int currentEpoch = 1;
    
    std::string initSig, newSig;
    initSig = newSig = argv[1];
    
    regina::Triangulation<4> working(initSig);
    
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
    std::vector<regina::Tetrahedron<4>*> twoFourAvailable;
    regina::Tetrahedron<4>* twoFourChoice;
    std::vector<std::string> movesSinceLastReduction;
    std::string moveLogStartSig = newSig;
    std::string bestSig = newSig;
    ssize_t bestEdgeCount = oldEdgeCount;
    size_t bestSize = working.size();
    //size_t edgeDiff = newEdgeCount - oldEdgeCount;

    auto startTime = std::chrono::system_clock::now();
    
    while (currentEpoch <= epochs) {

        working = regina::Triangulation<4>(newSig);
        printProgress(showProgress, currentEpoch, epochs, "epoch start",
            working, bestEdgeCount);
        if (verbose) {
            movesSinceLastReduction.clear();
            moveLogStartSig = newSig;
        }
        
        printProgress(showProgress, currentEpoch, epochs, "down",
            working, bestEdgeCount);
        MoveResult downResult = down(std::move(working), verbose,
            nullptr); // Try going down first if we can.
        working = std::move(downResult.triangulation);
        printProgress(showProgress, currentEpoch, epochs, "down complete",
            working, bestEdgeCount);
        if (verbose) {
            movesSinceLastReduction.insert(
                movesSinceLastReduction.end(),
                downResult.moves.begin(),
                downResult.moves.end());
        }
        newEdgeCount = working.countEdges();
        if (newEdgeCount < oldEdgeCount) {
            newSig = working.neoSig();
            bestSig = newSig;
            bestEdgeCount = newEdgeCount;
            bestSize = working.size();
            if (verbose) {
                clearProgressLine(showProgress);
                std::cerr << "from " << moveLogStartSig
                    << "\nto " << newSig
                    << formatMoves(movesSinceLastReduction)
                    << std::endl;
            } else {
                if (!dataOnly) {
                    std::cout << newSig << std::endl;
                }
                clearProgressLine(showProgress);
                std::cerr << currentEpoch << "," <<
//            oldEdgeCount - newEdgeCount << "," <<
                oldEdgeCount << "," << newEdgeCount << ",0" << std::endl;
            }
            oldEdgeCount = newEdgeCount;
            if (verbose) {
                movesSinceLastReduction.clear();
                moveLogStartSig = newSig;
            }
            working = regina::Triangulation<4>(newSig);
        }
        
        printProgress(showProgress, currentEpoch, epochs, "side",
            working, bestEdgeCount);
        MoveResult sideResult = side(std::move(working), sideCap, useRandom,
            verbose, useLookahead, nullptr);
        working = std::move(sideResult.triangulation);
        printProgress(showProgress, currentEpoch, epochs, "side complete",
            working, bestEdgeCount);
        if (verbose) {
            movesSinceLastReduction.insert(
                movesSinceLastReduction.end(),
                sideResult.moves.begin(),
                sideResult.moves.end());
        }
        newEdgeCount = working.countEdges();
        if (newEdgeCount < oldEdgeCount) {
            newSig = working.neoSig();
            bestSig = newSig;
            bestEdgeCount = newEdgeCount;
            bestSize = working.size();
            if (verbose) {
                clearProgressLine(showProgress);
                std::cerr << "from " << moveLogStartSig
                    << "\nto " << newSig
                    << formatMoves(movesSinceLastReduction)
                    << std::endl;
            } else {
                if (!dataOnly) {
                    std::cout << newSig << std::endl;
                }
                clearProgressLine(showProgress);
                std::cerr << currentEpoch << "," <<
//            oldEdgeCount - newEdgeCount << "," <<
                oldEdgeCount << "," << newEdgeCount << ",0" << std::endl;
            }
            oldEdgeCount = newEdgeCount;
            if (verbose) {
                movesSinceLastReduction.clear();
                moveLogStartSig = newSig;
            }
            working = regina::Triangulation<4>(newSig);
        }
        
        twoFourAttempts = bulkUp ? twoFourCap : 0;
        while (bulkUp ? (twoFourAttempts == twoFourCap) :
                (twoFourAttempts < twoFourCap)) {
            printProgress(showProgress, currentEpoch, epochs, "up",
                working, bestEdgeCount, twoFourAttempts, twoFourCap);
            for (int i=0; i<twoFourAttempts; i++) {
                twoFourAvailable.clear();
                for (regina::Tetrahedron<4>*tet : working.tetrahedra()) {
                    if (working.hasPachner(tet)) {
                        twoFourAvailable.emplace_back(tet);
                    }
                }
                
                if (twoFourAvailable.empty()) {
                    break;
                }

                printProgress(showProgress, currentEpoch, epochs, "2-4 move",
                    working, bestEdgeCount, i + 1, twoFourAttempts);
                twoFourChoice = chooseTwoFourMove(working, twoFourAvailable,
                    useRandom, i, useLookahead);
                
                if (verbose) {
                    movesSinceLastReduction.emplace_back(
                        describeMove("2-4", "tetrahedron",
                            twoFourChoice->index()));
                }
                working.pachner(twoFourChoice);
            }
            
            printProgress(showProgress, currentEpoch, epochs, "post-up down",
                working, bestEdgeCount, twoFourAttempts, twoFourCap);
            downResult = down(std::move(working), verbose,
                nullptr);
            working = std::move(downResult.triangulation);
            printProgress(showProgress, currentEpoch, epochs,
                "post-up down complete", working, bestEdgeCount,
                twoFourAttempts, twoFourCap);
            if (verbose) {
                movesSinceLastReduction.insert(
                    movesSinceLastReduction.end(),
                    downResult.moves.begin(),
                    downResult.moves.end());
            }
            newEdgeCount = working.countEdges();
            if (newEdgeCount < oldEdgeCount) {
                newSig = working.neoSig();
                bestSig = newSig;
                bestEdgeCount = newEdgeCount;
                bestSize = working.size();
                if (verbose) {
                    clearProgressLine(showProgress);
                    std::cerr << "from " << moveLogStartSig
                        << "\nto " << newSig
                        << formatMoves(movesSinceLastReduction)
                        << std::endl;
                } else {
                    if (!dataOnly) {
                        std::cout << newSig << std::endl;
                    }
                    clearProgressLine(showProgress);
                    std::cerr << currentEpoch << "," <<
//                oldEdgeCount - newEdgeCount << "," <<
                    oldEdgeCount << "," << newEdgeCount << "," << twoFourAttempts
                    << std::endl;
                }
//                std::cerr << currentEpoch << " | " << oldEdgeCount - newEdgeCount << " | 2-4: " << twoFourAttempts << std::endl;
                oldEdgeCount = newEdgeCount;
                if (verbose) {
                    movesSinceLastReduction.clear();
                    moveLogStartSig = newSig;
                }
                working = regina::Triangulation<4>(newSig);
            }
            
            printProgress(showProgress, currentEpoch, epochs, "post-up side",
                working, bestEdgeCount, twoFourAttempts, twoFourCap);
            sideResult = side(std::move(working), sideCap, useRandom, verbose,
                useLookahead, nullptr);
            working = std::move(sideResult.triangulation);
            printProgress(showProgress, currentEpoch, epochs,
                "post-up side complete", working, bestEdgeCount,
                twoFourAttempts, twoFourCap);
            if (verbose) {
                movesSinceLastReduction.insert(
                    movesSinceLastReduction.end(),
                    sideResult.moves.begin(),
                    sideResult.moves.end());
            }
            newEdgeCount = working.countEdges();
            if (newEdgeCount < oldEdgeCount) {
                newSig = working.neoSig();
                bestSig = newSig;
                bestEdgeCount = newEdgeCount;
                bestSize = working.size();
                if (verbose) {
                    clearProgressLine(showProgress);
                    std::cerr << "from " << moveLogStartSig
                        << "\nto " << newSig
                        << formatMoves(movesSinceLastReduction)
                        << std::endl;
                } else {
                    if (!dataOnly) {
                        std::cout << newSig << std::endl;
                    }
                    clearProgressLine(showProgress);
//                std::cerr << currentEpoch << " | " << oldEdgeCount - newEdgeCount << " | 2-4: " << twoFourAttempts << std::endl;
                    std::cerr << currentEpoch << "," <<
//                oldEdgeCount - newEdgeCount << "," <<
                    oldEdgeCount << "," << newEdgeCount << "," << twoFourAttempts
                    << std::endl;
                }
                oldEdgeCount = newEdgeCount;
                if (verbose) {
                    movesSinceLastReduction.clear();
                    moveLogStartSig = newSig;
                }
                working = regina::Triangulation<4>(newSig);
            }
            
            if (bulkUp) {
                break;
            }
            twoFourAttempts++;
        }
        
        newSig = working.neoSig();
        
        currentEpoch+=1;
    }
    
    auto endTime = std::chrono::system_clock::now();
    clearProgressLine(showProgress);
        
    std::chrono::duration<double> elapsedSeconds = endTime-startTime;
    std::time_t startStamp = std::chrono::system_clock::to_time_t(startTime);
    std::time_t endStamp = std::chrono::system_clock::to_time_t(endTime);
    std::cerr << "Started " << std::ctime(&startStamp)
            << "Finished " << std::ctime(&endStamp)
            << "Elapsed time: " << elapsedSeconds.count() << "s"
            << std::endl;
    ssize_t totalReduction = initEdgeCount - bestEdgeCount;

    if (totalReduction <= 0) {
        std::cerr << "Failed to simplify this run." << std::endl;
    }
    else {
        std::cerr << "Reduced " << totalReduction << " edges (" << initEdgeCount
                << " -> " << bestEdgeCount <<")." << std::endl;
    }

    std::cerr << "Final sig:" << std::endl;
    std::cout << bestSig << std::endl;
    std::cerr << "Final triangulation size: " << bestSize << std::endl;
    return 0;
}
