//
//  dgt2.cc
//  DGT Version 2.0
//
//  Created by Rhuaidi Antonio Burke on 22/06/23.
//  Copyright © 2023 Regina Development Team. All rights reserved.
//
//  PRE-RELEASE VERSION. IF POSSIBLE WAIT FOR "KATIE" SOURCE.
//  THIS VERSION HAS BUGS!

#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include <array>
#include <tuple>
#include <set>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stack>
#include <string>
#include <sstream>
#include <numeric>
#include <unistd.h>
#include <string>

#include <triangulation/dim3.h>
#include <triangulation/dim4.h>
#include <link/link.h>

bool printDebugInfo = false;

struct vertex {
    int vertexID = -1;
    int strandID = -1;
    int component = -1;
};

vertex emptyVertex = vertex{-1,-1,-1};

struct edge {
    vertex v1 {};
    vertex v2 {};
    int colour {};
};

typedef std::vector<std::array<int, 4>> pdcode;

std::ostream& operator<<(std::ostream& os, const std::array<int, 4>& arr) {
    os << "(" << arr[0] << ", " << arr[1] << ", " << arr[2] << ", " << arr[3] << ")";
    return os;
}

bool operator <(const vertex& x, const vertex& y) {
    return std::tie(x.vertexID, x.strandID, x.component) < std::tie(y.vertexID, y.strandID, y.component);
}

bool operator ==(const vertex& x, const vertex& y) {
    return ((x.vertexID == y.vertexID) && (x.strandID == y.strandID) && (x.component == y.component));
}

std::ostream& operator<<(std::ostream& os, const vertex& v) {
//    os << "(" << v.vertexID << ", " << v.strandID << ", " << v.component << ")";
    os << "(" << v.vertexID << ", " << v.component << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const edge& e) {
    os << "(" << e.v1 << ", " << e.v2 << ", " << e.colour << ")";
    return os;
}

bool isCurl(const regina::StrandRef &ref) {
    bool ans = false;
    // Check here if something breaks in future.
    if ((ref.next().crossing()->index() == ref.crossing()->index()) || (ref.prev().crossing()->index() == ref.crossing()->index())) {
        ans = true;
    }
    return ans;
}

template <typename T, typename D>
std::ostream& operator<<(std::ostream& os, const std::pair<T, D> &p)
{
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::tuple<T, T, T> &p)
{
    os << "[" << std::get<0>(p) << ", " << std::get<1>(p) << ", " << std::get<2>(p) << "],";
    return os;
}

bool isNumber(const std::string &str) {
    for (char const &c : str) {
        if ((c != '-') && (std::isdigit(c) == 0)) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool contains(std::vector<T> a, std::vector<T> b) {
    for (const auto& a_element : a) {
        if (std::find(b.begin(), b.end(), a_element) == b.end()) {
            return false;
        }
    }
    return true;
}

template <int dim>
class graph {
    std::map<vertex, std::array<vertex, dim+1>> adjList;
    
public:
    
    static int uniqueID() {
        // TODO: Make thread safe, but mutex seems to break things...
        static int nextID = 0;
        return nextID++;
    }
    
    std::map<vertex, std::array<vertex, dim+1>> adjacencyList() {
        return adjList;
    }
    
    void print_graph() {
        for (const auto& [vert,nbrs] : adjList) {
            std::cout << vert << ": ";
            for (const auto& v : nbrs) {
                std::cout << v << ", ";
            }
            std::cout << std::endl;
        }
    }

    void add_edge(edge e) {
        adjList[e.v1][e.colour] = e.v2;
        adjList[e.v2][e.colour] = e.v1;
    }
    
    void add_edges(const std::vector<edge>& el) {
        for (const auto& e : el) {
            add_edge(e);
        }
    }
    
    std::vector<vertex> vertices() {
        std::vector<vertex> vertList;
        
        for (const auto& [key,val] : adjList) {
            vertList.emplace_back(key);
        }
        
        return vertList;
    }
    
    std::vector<edge> edges() {
        std::vector<edge> edgeList;
        
        for (const auto& [vert, nbrs] : adjList) {
            for (int i=0; i<dim+1; i++) {
                if ((vert < nbrs[i]) && (vert.vertexID != 0)) {
                    edgeList.push_back({vert,nbrs[i],i});
                }
            }
        }
        
        return edgeList;
    }
    
    void print_vertices() {
        for (const auto& [key,val] : adjList) {
            std::cout << key << "\n";
        }
        std::cout << std::endl;
    }
    
    void print_edges() {
        for (const auto& [vert, nbrs] : adjList) {
            for (int i=0; i<dim+1; i++) {
                if ((vert < nbrs[i]) && (vert.vertexID != 0)) {
                    std::cout << "[" << vert << ", " << nbrs[i] << ", " << i << "],\n";
                }
            }
        }
    }
    
    void DEBUGremainingNo4ColVerts() {
        std::vector<vertex> verts = vertices();
        if (printDebugInfo) {
            int counter = 0;
            for (const auto& v : verts) {
                if (adjList[v][4] == emptyVertex) {
                    counter++;
                }
            }
            std::cout << "Number of remaining vertices without a colour 4 edge: " << counter << std::endl;
        }
    }
        
    void DEBUGno4ColVerts() {
        if (printDebugInfo) {
            std::cout << "These vertices have no 4-coloured edge:\n";
            std::vector<vertex> verts = vertices();
            int counter = 0;
            for (const auto& v : verts) {
                if (adjList[v][4] == emptyVertex) {
                    counter++;
                    std::cout << v << std::endl;
                }
            }
            std::cout << "Total number: " << counter << std::endl;
    //        std::cout << std::endl;
        }
    }
    
    void disjoint_union(graph<dim> h) {
        std::vector<edge> hEdges = h.edges();
        
        int currentID = uniqueID();
        
        for (auto e : hEdges) {
            vertex v1, v2;
            v1 = e.v1;
            v2 = e.v2;
            v1.component = currentID;
            v2.component = currentID;

            int col = e.colour;
            
            adjList[v1][col] = v2;
            adjList[v2][col] = v1;
        }
    }
    
    void pd_sub(const pdcode& code) {
        vertex newNbr_i;
        for (auto [vert, nbrs] : adjList) {
            for (int i=0; i<dim+1; i++) {
                if (nbrs[i].vertexID != 0) {
                    switch (nbrs[i].strandID) {
                        case 0:
                            break;
                        case 1:
                            newNbr_i = {nbrs[i].vertexID,code[vert.component][0],nbrs[i].component};
                            adjList.erase(nbrs[i]);
                            adjList[vert][i] = newNbr_i;
                            adjList[newNbr_i][i] = vert;
//                            nbrs[i].strandID = code[vert.component][0];
                            break;
                        case 2:
                            newNbr_i = {nbrs[i].vertexID,code[vert.component][1],nbrs[i].component};
                            adjList.erase(nbrs[i]);
                            adjList[vert][i] = newNbr_i;
                            adjList[newNbr_i][i] = vert;
//                            nbrs[i].strandID = code[vert.component][1];
                            break;
                        case 3:
                            newNbr_i = {nbrs[i].vertexID,code[vert.component][2],nbrs[i].component};
                            adjList.erase(nbrs[i]);
                            adjList[vert][i] = newNbr_i;
                            adjList[newNbr_i][i] = vert;
//                            nbrs[i].strandID = code[vert.component][2];
                            break;
                        case 4:
                            newNbr_i = {nbrs[i].vertexID,code[vert.component][3],nbrs[i].component};
                            adjList.erase(nbrs[i]);
                            adjList[vert][i] = newNbr_i;
                            adjList[newNbr_i][i] = vert;
//                            nbrs[i].strandID = code[vert.component][3];
                            break;
                    }
                }
            }
        }
    }
    
    std::vector<std::pair<vertex, vertex>> fuse_list() {
        
        // Think this should actually be a standalone function which operates on a graph rather than a method of graph class but anyway, put it here for now/debugging...
        // (otherwise really need to do some input checking...)
        
        /*
         Let V_i = (c_i, v_i, s_i), V_j = (c_j, v_j, s_j)
         Criteria in the if statement below are as follows:
         1. Avoids duplicate pairs (works because elements are ordered).
         2. Only operate on "outer" vertices ("internal" vertices denoted via s = 0).
         3. c_i ≠ c_j (different "components")
         4. s_i = s_j (same strand/PD element)
         5. v_j mod 4 = (5 - (v_i mod 4)) mod 4.
         */
        
        std::vector<std::pair<vertex, vertex>> ans;
        
        for (auto const& [v1,nbrs1] : adjList) {
            for (auto const& [v2,nbrs2] : adjList) {
                if (
                    (v1.component < v2.component) &&
                    (v1.strandID != 0) && (v2.strandID != 0) &&
                    (v1.component != v2.component) &&
                    (v1.strandID == v2.strandID) &&
                    ((v1.vertexID)%4 == (5-((v2.vertexID)%4))%4)) {
                    ans.push_back(std::make_pair(v1, v2));
                }
            }
        }
        
        return ans;
        
    }
    
    void fuse(vertex v1, vertex v2) {
        
        // TODO: Should sanitise things - e.g. make sure v1 has as many neighbours as v2 and that their colours match up, etc.
        
        std::array<vertex, dim+1> v1nbrs = adjList[v1]; // {v1n1,v1n2,...,v1n(dim+1)}
        std::array<vertex, dim+1> v2nbrs = adjList[v2]; // {v2n1,v2n2,...,v2n(dim+1)}

        adjList.erase(v1);
        adjList.erase(v2);

        for (int i=0; i<dim+1; i++) {
            adjList[v1nbrs[i]][i] = v2nbrs[i];
            adjList[v2nbrs[i]][i] = v1nbrs[i];
        }
        adjList.erase({0,0,0});
    }
    
    std::vector<std::array<vertex, 4>> quadriGraphFind() {
        /*
         A quadricolour is a cyclic subgraph of the form:
         
                    P1 -|-|-|- P2 ~~~~~~~ P3
                     \_                   .
                       \__                .
                          \___            .
                              \____       .
                                   \_____ P0
         
         Where: ----- = 0
                -|-|- = 1
                ~~~~~ = 2
                ..... = 3
         
         In terms of the adjacency list structure, a quadricolour will look like:
         
         P0 : {P1, **, **, P3, **}
         P1 : {P0, P2, **, **, **}
         P2 : {**, P1, P3, **, **}
         P3 : {**, **, P2, P0, **}
         
         Hence we locate a quadricolour as follows.
         Let P0 = vert, then in the adjaceny list we have:
         
         vert : {P0n1, P0n2, P0n3, P0n4, 0}
         
         We then look at P0n1's and P0n3's neighbours.
         If we are in a quadricolour, then P0n1's 2nd neighbour will be the same as P0n3's 3rd neighbour.
         This completely determines the cycle.
         */

        std::vector<std::array<vertex, 4>> ans;
        
        std::array<vertex, 2> tmp;
        
        for (const auto& [vert, nbrs] : adjList) {
            if (!(vert == emptyVertex)) {
                tmp[0] = adjList[vert][0];
                tmp[1] = adjList[vert][3];
                if (adjList[tmp[0]][1] == adjList[tmp[1]][2]) {
                    std::array<vertex, 4> currentQuadri;
                    currentQuadri[0] = vert;
                    currentQuadri[1] = tmp[0];
                    currentQuadri[2] = adjList[tmp[0]][1];
                    currentQuadri[3] = tmp[1];
                    ans.emplace_back(currentQuadri);
                }
            }
        }

        return ans;

    }
    
    size_t size() {
        return adjList.size();
    }
    
    void addQuadriEdges(std::vector<std::array<vertex, 4>> quadriVect) {
        if (printDebugInfo) {
            std::cout << "Performing quadri substitutions..." << std::endl;
        }
        for (const auto& quadri : quadriVect) {
            adjList[quadri[0]][4] = quadri[1];
            adjList[quadri[1]][4] = quadri[0];
            
            adjList[quadri[2]][4] = quadri[3];
            adjList[quadri[3]][4] = quadri[2];
            
            vertex P4 = adjList[quadri[3]][1];
            vertex P5 = adjList[quadri[0]][1];
            
            adjList[P4][4] = P5;
            adjList[P5][4] = P4;
        }
        if (printDebugInfo) {
            std::cout << "Successfully performed quadri substitutions!" << std::endl;
        }
    }
    
    void addOneHandleIdentEdges(std::vector<std::pair<vertex, vertex>> oneHandleMarkedVertPairs) {
        if (printDebugInfo) {
            std::cout << "Adding 1-handle identification edges..." << std::endl;
        }
        for (const auto& pair : oneHandleMarkedVertPairs) {
            edge e = {pair.first,pair.second,4};
            add_edge(e);
        }
        if (printDebugInfo) {
            std::cout << "Successfully added 1-handle identification edges!" << std::endl;
        }
    }
    
    void addHighlightEdges(std::vector<std::vector<regina::StrandRef>> highlightCrossings) {
        if (printDebugInfo) {
            std::cout << "Adding highlight edges..." << std::endl;
        }
        std::vector<vertex> verts = vertices();
        
        std::vector<std::vector<vertex>> highlightGraphOverCrossingVertices;
        std::vector<std::vector<vertex>> highlightGraphUnderCrossingVertices;
        std::vector<std::vector<vertex>> highlightGraphCurlCrossingVertices;
        
        for (const auto& highlightVector : highlightCrossings) {
            for (const auto& ref : highlightVector) {
                std::vector<vertex> currentCrossingVertices;
                for (const auto& v : verts) {
                    if (v.component == ref.crossing()->index()) {
                        currentCrossingVertices.emplace_back(v);
                    }
                }
                if ((ref.strand() == 0) && !(isCurl(ref))) {
                    highlightGraphUnderCrossingVertices.emplace_back(currentCrossingVertices);
                }
                else if ((ref.strand() == 1) && !(isCurl(ref))) {
                    highlightGraphOverCrossingVertices.emplace_back(currentCrossingVertices);
                }
                else if (isCurl(ref)) {
                    highlightGraphCurlCrossingVertices.emplace_back(currentCrossingVertices);
                }
            }
        }
        
        // Under
        for (const auto& vect : highlightGraphUnderCrossingVertices) {
            for (const auto& x : vect) {
                for (const auto& y : vect) {
                    if (x < y) {
                        if (
                            ((x.vertexID == 1) && (y.vertexID == 6)) ||
                            ((x.vertexID == 2) && (y.vertexID == 5)) ||
                            ((x.vertexID == 3) && (y.vertexID == 4)) ||
                            ((x.vertexID == 7) && (y.vertexID == 8))
                            ) {
                            edge e = {x,y,4};
                            add_edge(e);
                        }
                    }
                }
            }
        }

        // Over
        for (const auto& vect : highlightGraphOverCrossingVertices) {
            for (const auto& x : vect) {
                for (const auto& y : vect) {
                    if (x < y) {
                        if (
                            ((x.vertexID == 1) && (y.vertexID == 2)) ||
                            ((x.vertexID == 5) && (y.vertexID == 6))
                            ) {
                            edge e = {x,y,4};
                            add_edge(e);
                        }
                    }
                }
            }
        }

        // Curl
        for (const auto& vect : highlightGraphCurlCrossingVertices) {
            for (const auto& x : vect) {
                for (const auto& y : vect) {
                    if (x < y) {
                        if ((adjList[x][4] == emptyVertex) && (adjList[y][4] == emptyVertex)) {
                            if (
                                ((x.vertexID == 1) && (y.vertexID == 4)) ||
                                ((x.vertexID == 2) && (y.vertexID == 3))
                                ) {
                                edge e = {x,y,4};
                                add_edge(e);
                            }
                        }
                    }
                }
            }
        }
        
        if (printDebugInfo) {
            std::cout << "Successfully added highlight edges!" << std::endl;
        }

    }
    
    void addDoubleOneEdges() {
        if (printDebugInfo) {
            std::cout << "Doubling open colour-1 edges..." << std::endl;
        }

        for (const auto& [vert,nbrs] : adjList) {
            if (
                (vert < nbrs[1]) &&
                (adjList[vert][4] == emptyVertex) &&
                (adjList[nbrs[1]][4] == emptyVertex)
                ) {
                edge e = {vert,nbrs[1],4};
                add_edge(e);
            }
        }
        if (printDebugInfo) {
            std::cout << "Successfully doubled open colour-1 edges!" << std::endl;
        }
    }
    
    void addRemainderEdges() {
        if (printDebugInfo) {
            std::cout << "Adding remainder edges..." << std::endl;
        }

        std::vector<vertex> verts = vertices();
        for (const auto& x : verts) {
            if (!(x == emptyVertex)) {
                if (adjList[x][4] == emptyVertex) {
                    vertex y=x;
                    int i;
                    int n=0;
                    do {
                        i = 4*(n%2)+(n+1)%2;
                        y = adjList[y][i];
                        n+=1;
                    } while (!(adjList[y][4] == emptyVertex));
                    edge e = {x,y,4};
                    add_edge(e);
                }
            }
        }
        if (printDebugInfo) {
            std::cout << "Successfully added remainder edges!" << std::endl;
        }

    }
    
    void tempBugFix() {
        std::vector<vertex> verts = vertices();
        for (const auto& x : verts) {
            for (const auto& y : verts) {
                if ((adjList[y][4] == x) && (adjList[x][4] == emptyVertex)) {
                    adjList[x][4] = y;
                }
            }
        }
    }
    
    void cleanup() {
        adjList.erase(emptyVertex);
    }
    
};

long getIndex(std::vector<vertex> v, vertex K) {
    // 2022 Rewrite: (Doc) Given a list of vertices v, and a vertex K, return the index of K in v ?
    
    long ans = -1;
    
    auto it = std::find_if(v.begin(), v.end(), [&cv = K](const vertex& vert) -> bool {return cv == vert;});

    if (it != v.end()) {
        long index = it - v.begin();
        ans = index;
    }
    else {
        ans = -1;
    }
    
    return ans;
}

void printGluingList(graph<4> G) {
    std::vector<vertex> verts = G.vertices();
    std::vector<edge> edges = G.edges();
    std::cout << "[";
    for (const auto& edge : edges) {
        // trick to print commas after everything except the last line :D
        if (&edge != &edges.back()) {
            std::cout << "[" << getIndex(verts, edge.v1) << ", " << getIndex(verts, edge.v2) <<", " << edge.colour << "],\n";
        }
        else {
            std::cout << "[" << getIndex(verts, edge.v1) << ", " << getIndex(verts, edge.v2) <<", " << edge.colour << "]]\n";
        }
    }
}

std::vector<std::tuple<int,int,int>> gluingList(graph<4> G) {
    std::vector<vertex> verts = G.vertices();
    std::vector<edge> edges = G.edges();
    std::vector<std::tuple<int,int,int>> ans;
    ans.reserve(edges.size());
    for (const auto& elem : edges) {
        ans.emplace_back(getIndex(verts, elem.v1),getIndex(verts, elem.v2),elem.colour);
    }
    return ans;
}

std::vector<int> pdc_orientations(pdcode code) {

    std::array<int, 4> eovInit = {0,0,0,0};
    
    std::array<int, 4> negative = {1,1,-1,-1};
    std::array<int, 4> positive = {1,-1,-1,1};
    
    long pdLength = code.size();
    int numberOfStrands = 2*pdLength;

    std::vector<std::array<int, 4>> extendedOrientationVector(pdLength,eovInit);
        
    std::vector<std::vector<bool>> visited(pdLength,std::vector<bool>(4,false));
    std::vector<int> seenStrands;
    
    int i = 0, j = 0;
    int currentStrand = code[i][j];
    int count = 1;
    
    while (!visited[i][j]) {
        
        bool carry = false;
        int carryRow;

        visited[i][j] = true;
        seenStrands.emplace_back(currentStrand);

        if (count%2 == 1) {
            extendedOrientationVector[i][j] = 1;
        }
        else {
            extendedOrientationVector[i][j] = -1;
        }
        count++;
        
        j = (j+2)%4;
        
        currentStrand = code[i][j];
        visited[i][j] = true;
        seenStrands.emplace_back(currentStrand);
        
        if (count%2 == 1) {
            extendedOrientationVector[i][j] = 1;
        }
        else {
            extendedOrientationVector[i][j] = -1;
        }
        count++;
        
        if (std::count(seenStrands.begin(),seenStrands.end(),currentStrand) == 2) {
            carry = true;
            for (int row=0; row<pdLength; row++) {
                if (!visited[row][0]) {
                    currentStrand = code[row][0];
                    carryRow = row;
                    break;
                }
            }
        }
        
        int nextI = -1, nextJ = -1;
        if (!carry) {
            for (int row=0; row<pdLength; row++) {
                for (int col=0; col<4; col++) {
                    if (!visited[row][col] && code[row][col] == currentStrand) {
                        nextI = row;
                        nextJ = col;
                        break;
                    }
                }
                if (nextI != -1) {
                    break;
                }
            }
        }
        else {
            nextI = carryRow;
            nextJ = 0;
        }
        
        i = nextI;
        j = nextJ;
        
        if (nextI == -1 && nextJ == -1) {
            break;
        }
    }
    
    std::vector<int> orientations;

    for (const auto& x : extendedOrientationVector) {
//      DEBUG: Print the current EOV tuple.
//        std::cout << x << std::endl;
        if (x == positive) {
            orientations.push_back(1);
        }
        else if (x == negative) {
            orientations.push_back(-1);
        }
    }

    return orientations;
    
}

std::vector<int> pdc_xtype(const pdcode& code) {
    /*
    xtype = "crossing type"
    This distinguishes between a regular crossing,
    and the four different PD code tuples that
    can arise from a curl.
    */

    std::vector<int> ans;
    
    for (const auto& x : code) {
        if (x[2]==x[3]) {
            // (a,b,x,x) Positive
            ans.emplace_back(1);
        }
        else if (x[0]==x[1]) {
            // (x,x,c,d) Positive
            ans.emplace_back(2);
        }
        else if (x[1]==x[2]) {
            // (a,x,x,d) Negative
            ans.emplace_back(3);
        }
        else if (x[0]==x[3]) {
            // (x,b,c,x) Negative
            ans.emplace_back(4);
        }
        else {
            // regular crossing
            ans.emplace_back(0);
        }
    }
    
    return ans;
}

std::vector<std::pair<int, int>> pdc_xotype(const pdcode& code) {
    /*
     Probably too python-esque at the moment (running functions which create vectors, etc...)
     */
    std::vector<std::pair<int, int>> ans;

    std::vector<int> pdc_os = pdc_orientations(code);
    
    std::vector<int> pdc_xs = pdc_xtype(code);
        
    for (size_t i=0; i<code.size(); i++) {
        ans.emplace_back(pdc_xs[i], pdc_os[i]);
    }
    return ans;

}

void DEBUGwalkAroundLink(regina::Link lnk) {
    std::cout << "Debug link walkaround:" << std::endl;
    for (const auto& comp : lnk.components()) {
        auto ref = comp;
        do {
            std::cout << ref << ", ";
            ref = ref.next();
        } while (ref != comp);
        std::cout << std::endl;
    }
}

graph<4> posCross, negCross, posCurlA, posCurlB, negCurlA, negCurlB;

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    
    std::cerr << "Usage:" << std::endl;
    std::cerr << "    " << progName << " \"PD Code\", "
        " { -3, --dim3 | -4, --dim4 } "
        "[ -g, --graph ] [ -d, --debug ]\n"
        "    " << progName << " [ -v, --version | -?, --help ]\n\n";
    std::cerr << "    -3, --dim3 : Build a 3-manifold via integer "
        "Dehn surgery\n";
    std::cerr << "    -4, --dim4 : Build a 4-manifold by attaching "
        "1- and 2-handles along a framed link\n\n";
    std::cerr << "    -g, --graph : Output an edge-coloured graph, "
        "not an isomorphism signature\n";
    std::cerr << "    -d, --debug : Display construction information\n";
//    std::cerr << "    -r, --real  : Build a triangulation with real boundary "
//        "(not ideal or closed)\n\n";
    std::cerr << "    -v, --version : Show which version of Regina "
        "is being used\n";
    std::cerr << "    -?, --help    : Display this help\n";
    
    exit(1);
}

int main(int argc, char* argv[]) {
    
    int dimFlag = 4; // Default to build a 4-manifold.
    bool outputGraph = false;
    bool realBdy = false; // Not currently used.
    
    // Check for standard arguments:
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0)
            usage(argv[0]);
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            if (argc != 2)
                usage(argv[0],
                    "Option --version cannot be used with "
                        "any other arguments.");
            std::cout << PACKAGE_BUILD_STRING << std::endl;
            exit(0);
        }
    }
    
    /*
     outputGraph:
     -----------
     false  :   output an isomorphism signature (default)
     true   :   output a graph edge list
     
     realBdy:
     --------
     false  :   non-spherical boundary -> ideal,
                spherical boundary -> capped off (closed manifold)
     true   :   non-spherical boundary -> real,
                spherical boundary -> real (retains spherical boundary)
     */
    
    pdcode pdcTmp;
    
    /*
     START Process PD Code
     */
    std::string rawPDinput;

    if (argc < 2) {
        usage(argv[0], "Please provide a PD code.");
    }
    else if (2 <= argc && argc < 5) {
        for (int i=1; i<argc; ++i) {
            if (argv[i][0] == '[' || argv[i][0] == 'P' || argv[i][0] == '(' || isdigit(argv[i][0])) {
                rawPDinput = argv[1];
            }
            else if (!strcmp(argv[i], "-3") || !strcmp(argv[i], "--dim3")) {
                dimFlag = 3;
            }
            else if (!strcmp(argv[i], "-4") || !strcmp(argv[i], "--dim4")) {
                dimFlag = 4;
            }
            else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--graph")) {
                outputGraph = true;
            }
            else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
                printDebugInfo = true;
            }
            else {
                usage(argv[0], std::string("Invalid option: ") + argv[i]);
            }
        }
    }

    for (char &c : rawPDinput) {
        if (!isdigit(c)) {
            c = ' ';
        }
    }

    std::vector<int> tmpRawPDVect;
    std::stringstream ss_pdc(rawPDinput);
    int val;
    while (ss_pdc >> val) {
        tmpRawPDVect.push_back(val);
    }

    for (int i=0; i<tmpRawPDVect.size(); i+=4) {
        std::array<int, 4> tmpa;
        for (int j=0; j<4; j++) {
            tmpa[j] = tmpRawPDVect[i+j];
        }
        pdcTmp.push_back(tmpa);
    }
    /*
     END Process PD Code
     */
    
    regina::Link tmpLinkObj = regina::Link::fromPD(pdcTmp.begin(), pdcTmp.end());
        
    size_t numberOfLinkComponents = tmpLinkObj.countComponents();

    std::cout << "Framing or 1-Handle (x) Placement: ";
    std::vector<int> framingVector(numberOfLinkComponents,INFINITY), twoHandleFramings;
    std::vector<bool> isOneHandleVector(numberOfLinkComponents,false);
    
    for (int i=0; i<numberOfLinkComponents; i++) {
        std::string tmp;
        std::cin >> tmp;
        if (!isNumber(tmp)) {
            isOneHandleVector[i] = true;
            framingVector[i] = 0;
        }
        else {
            framingVector[i] = std::stoi(tmp);
            twoHandleFramings.emplace_back(std::stoi(tmp));
        }
    }
    
    bool existOneHandles = std::any_of(isOneHandleVector.begin(),isOneHandleVector.end(),[](bool isOneHandle){return isOneHandle == true;});
    
    std::vector<regina::StrandRef> oneHandleComponentRefs, twoHandleComponentRefs;
    std::vector<std::set<int>> oneHandleCrossingIndices;

    // Populate oneHandleComponentRefs and twoHandleComponentRefs
    for (int i=0; i<numberOfLinkComponents; i++) {
        if (isOneHandleVector[i]) {
            oneHandleComponentRefs.emplace_back(tmpLinkObj.component(i));
        }
        else {
            twoHandleComponentRefs.emplace_back(tmpLinkObj.component(i));
        }
    }

    size_t numberOfTwoHandles = twoHandleComponentRefs.size();

    // Populate oneHandleCrossingIndices
    for (const auto& oneHandleRef : oneHandleComponentRefs) {
        std::set<int> currentOneHandleCrossingIndices;
        auto currentOneHandleRef = oneHandleRef;
        do {
            currentOneHandleCrossingIndices.emplace(currentOneHandleRef.crossing()->index());
            currentOneHandleRef = currentOneHandleRef.next();
        } while (currentOneHandleRef != oneHandleRef);
        oneHandleCrossingIndices.emplace_back(currentOneHandleCrossingIndices);
    }
        
    // Populate oneTwoCommons
    std::vector<std::vector<regina::StrandRef>> oneTwoCommons;
    for (const auto& twoHandleRef : twoHandleComponentRefs) {
        std::vector<regina::StrandRef> currentOneTwoCommons;
        auto currentTwoHandleRef = twoHandleRef;
        do {
            for (const auto& oneHandleIndices : oneHandleCrossingIndices) {
                for (const auto& oneHandleIndex : oneHandleIndices) {
                    if (currentTwoHandleRef.crossing()->index() == oneHandleIndex) {
                        currentOneTwoCommons.emplace_back(currentTwoHandleRef);
                    }
                }
            }
            currentTwoHandleRef = currentTwoHandleRef.next();
        } while (currentTwoHandleRef != twoHandleRef);
        oneTwoCommons.emplace_back(currentOneTwoCommons);
    }
    
//    std::cout << "1/2 Commons:\n";
//    for (const auto& x : oneTwoCommons) {
//        for (const auto& y : x) {
//            std::cout << y << ", ";
//        }
//        std::cout << std::endl;
//    }
//    std::cout << std::endl;
        
    /*
     START FRAMING PROCEDURE
     */
    std::vector<regina::StrandRef> r1FramingSites;
    for (int i=0; i<numberOfTwoHandles; i++) {
        auto currentTwoHandle = twoHandleComponentRefs[i];
        auto currentOneTwoCommons = oneTwoCommons[i];
        bool found = false;
        for (const auto& x : currentOneTwoCommons) {
            if (std::find(currentOneTwoCommons.begin(),currentOneTwoCommons.end(),x.next()) != currentOneTwoCommons.end()) {
                r1FramingSites.emplace_back(x);
                found = true;
                break;
            }
        }
        if (!found) {
            r1FramingSites.emplace_back(currentTwoHandle);
        }
    }
    
//    std::cout << "R1 Framing Site Refs:\n";
//    for (const auto& x : r1FramingSites) {
//        std::cout << x << std::endl;
//    }
//    std::cout << std::endl;
//
    if (printDebugInfo) {
        std::cout << "Pre-framing walk around:" << std::endl;
        DEBUGwalkAroundLink(tmpLinkObj);
        std::cout << std::endl;
    }
    
    std::vector<long> twoHandleWrithes;
    for (const auto& twoHandle : twoHandleComponentRefs) {
        twoHandleWrithes.emplace_back(tmpLinkObj.writheOfComponent(twoHandle));
    }
    
    for (int i=0; i<numberOfTwoHandles; i++) {
        
        long currentTwoHandleWrithe = twoHandleWrithes[i];
        int currentTwoHandleFramingEntry = twoHandleFramings[i];
        regina::StrandRef r1FramingSiteRef = r1FramingSites[i];
        
        if (currentTwoHandleWrithe > currentTwoHandleFramingEntry) {
            if (printDebugInfo) {
                std::cout << "Self-framing 2-handle " << i << "... (--)\n";
            }
            do {
                tmpLinkObj.r1(r1FramingSiteRef, 0 /* left */, -1, false, true);
                --currentTwoHandleWrithe;
            } while (currentTwoHandleWrithe != currentTwoHandleFramingEntry);
        }
        else if (currentTwoHandleWrithe < currentTwoHandleFramingEntry) {
            if (printDebugInfo) {
                std::cout << "Self-framing 2-handle " << i << "... (++)\n";
            }
            do {
                tmpLinkObj.r1(r1FramingSiteRef, 0 /* left */, 1, false, true);
                ++currentTwoHandleWrithe;
            } while (currentTwoHandleWrithe != currentTwoHandleFramingEntry);
        }
        else if ((dimFlag == 4) && (currentTwoHandleWrithe == currentTwoHandleFramingEntry)) {
            if (printDebugInfo) {
                std::cout << "Adding additional pair of cancelling curls to 2-handle " << i << " to guarantee existence of a quadricolour...\n";
            }
            tmpLinkObj.r1(r1FramingSiteRef, 0, 1, false, true);
            tmpLinkObj.r1(r1FramingSiteRef, 0, -1, false, true);
        }
    }

    // debug
    if (printDebugInfo) {
        std::cout << std::endl;
        std::cout << "Link should now be self-framed:" << std::endl;
        for (int i=0; i<numberOfLinkComponents; i++) {
            std::cout << "Component " << i << ": ";
            if (isOneHandleVector[i]) {
                std::cout << "1-handle (" << tmpLinkObj.writheOfComponent(i) << ")" << std::endl;
            }
            else {
                std::cout << "2-handle, writhe " << tmpLinkObj.writheOfComponent(i) << std::endl;
            }
        }
        std::cout << std::endl;
    }
    // end debug
    /*
     END FRAMING PROCEDURE
     */
    
    /*
     The framing procedure means we need to recompute relevant link data
     so that indices, etc. match between the link object and graph objects
     generated later on.
     */
    regina::Link framedLnkObj = regina::Link(tmpLinkObj.pd());
    tmpLinkObj = framedLnkObj;
    
    // Repopulate oneHandleComponentRefs and twoHandleComponentRefs
    oneHandleComponentRefs.clear();
    twoHandleComponentRefs.clear();
    for (int i=0; i<numberOfLinkComponents; i++) {
        if (isOneHandleVector[i]) {
            oneHandleComponentRefs.emplace_back(tmpLinkObj.component(i));
        }
        else {
            twoHandleComponentRefs.emplace_back(tmpLinkObj.component(i));
        }
    }
    
    // Repopulate oneHandleCrossingIndices
    oneHandleCrossingIndices.clear();
    for (const auto& oneHandleRef : oneHandleComponentRefs) {
        std::set<int> currentOneHandleCrossingIndices;
        auto currentOneHandleRef = oneHandleRef;
        do {
            currentOneHandleCrossingIndices.emplace(currentOneHandleRef.crossing()->index());
            currentOneHandleRef = currentOneHandleRef.next();
        } while (currentOneHandleRef != oneHandleRef);
        oneHandleCrossingIndices.emplace_back(currentOneHandleCrossingIndices);
    }
        
    // Repopulate oneTwoCommons
    oneTwoCommons.clear();
    for (const auto& twoHandleRef : twoHandleComponentRefs) {
        std::vector<regina::StrandRef> currentOneTwoCommons;
        auto currentTwoHandleRef = twoHandleRef;
        do {
            for (const auto& oneHandleIndices : oneHandleCrossingIndices) {
                for (const auto& oneHandleIndex : oneHandleIndices) {
                    if (currentTwoHandleRef.crossing()->index() == oneHandleIndex) {
                        currentOneTwoCommons.emplace_back(currentTwoHandleRef);
                    }
                }
            }
            currentTwoHandleRef = currentTwoHandleRef.next();
        } while (currentTwoHandleRef != twoHandleRef);
        oneTwoCommons.emplace_back(currentOneTwoCommons);
    }
    
    if (printDebugInfo) {
        std::cout << "Post-framing walk around:" << std::endl;
        DEBUGwalkAroundLink(tmpLinkObj);
        std::cout << std::endl;
    }
    
    /*
     START 1-HANDLE MARKED CROSSINGS
     */
    /*
     Assuming one handle is traversed counter-clockwise,
     ◯.first is the "leftmost" crossing, and
     ◯.second is the "rightmost" crossing.
     
     TODO: Figure out how to differentiate counter-clockwise/clockwise. Until then, insist 1-handles are drawn counter-clockwise, else "garbage in, garbage out".
     */
    std::vector<std::pair<regina::StrandRef,regina::StrandRef>> oneHandleMarkedCrossingRefs;
    for (const auto& oneHandle : oneHandleComponentRefs) {
        std::pair<regina::StrandRef,regina::StrandRef> currentOneHandlePair;
        auto currentRef = oneHandle;
        do {
            if ((currentRef.strand() == 0) && (currentRef.next().strand() == 1)) {
                currentOneHandlePair.first = currentRef;
            }
            if ((currentRef.strand() == 1) && (currentRef.next().strand() == 0)) {
                currentOneHandlePair.second = currentRef.next();
            }
            currentRef = currentRef.next();
        } while (currentRef != oneHandle);
        oneHandleMarkedCrossingRefs.emplace_back(currentOneHandlePair);
    }
    // debug
    if (printDebugInfo) {
        std::cout << "1-Handle Marked Crossings:" << std::endl;
        for (const auto& pair : oneHandleMarkedCrossingRefs) {
            std::cout << pair.first << ", " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    // end debug
    /*
     END 1-HANDLE MARKED CROSSINGS
     */
       
    /*
     START LINK QUADRICOLOUR SEARCH
     */
    std::vector<std::vector<std::pair<regina::StrandRef,regina::StrandRef>>> quadricolourCandidates;
    for (const auto& twoHandle : twoHandleComponentRefs) {
        std::vector<std::pair<regina::StrandRef,regina::StrandRef>> currentQuadricolourCandidates;
        auto currentRef = twoHandle;
        do {
            if (isCurl(currentRef)) {
                auto next = currentRef.next().next();
                // The current crossing is a curl and the next crossing is a curl of the same sign.
                if (isCurl(next) && (currentRef.strand() == next.strand())) {
                    currentQuadricolourCandidates.emplace_back(currentRef,next);
                }
                // The current crossing is a curl and the next crossing is an undercrossing.
                if (!isCurl(next) && (next.strand() == 0)) {
                    currentQuadricolourCandidates.emplace_back(currentRef,next);
                }
                currentRef = next;
            }
            else {
                // The current crossing is an undercrossing and the next crossing is a curl.
                if ((currentRef.strand() == 0) && (isCurl(currentRef.next()))) {
                    currentQuadricolourCandidates.emplace_back(currentRef.next(),currentRef);
                }
                currentRef = currentRef.next();
            }
        } while (currentRef != twoHandle);
        quadricolourCandidates.emplace_back(currentQuadricolourCandidates);
    }
        
    std::vector<std::pair<regina::StrandRef,regina::StrandRef>> quadriPairRefs(numberOfTwoHandles);
    // For future ref: if something breaks, maybe check here first.
    for (int i=0; i<numberOfTwoHandles; i++) {
        auto quadriList = quadricolourCandidates[i];
        for (const auto& quadriPair : quadriList) {
            if (!isCurl(quadriPair.second)) {
                quadriPairRefs[i] = quadriPair;
                break;
            }
            else {
                quadriPairRefs[i] = quadriPair;
                break;
            }
        }
    }
    // debug
    if (printDebugInfo) {
        std::cout << "Quadricolour crossings:" << std::endl;
        for (const auto& pair : quadriPairRefs) {
            std::cout << pair.first << ", " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    // end debug
    /*
     END LINK QUADRICOLOUR SEARCH
     */
    
    /*
     START HIGHLIGHTING PROCEDURE
     */
    std::vector<std::vector<regina::StrandRef>> highlightCrossings;
    std::vector<bool> walkOppDirVec;
    if (existOneHandles) {
        for (int i=0; i<numberOfTwoHandles; i++) {
            std::vector<regina::StrandRef> currentTwoHandleHighlightedCrossings;
            
            bool WALK_OPPOSITE_DIRECTION = false;
            
            auto currentTwoHandle = oneTwoCommons[i];
            auto currentQuadriPair = quadriPairRefs[i];
            auto currentRef = currentQuadriPair.first;
                    
            if ((currentRef.next().next() == currentQuadriPair.second) && !isCurl(currentQuadriPair.second)) {
                WALK_OPPOSITE_DIRECTION = true;
            }
            
            std::vector<regina::StrandRef> neededCrossings;
            for (const auto& ref : currentTwoHandle) {
                if (WALK_OPPOSITE_DIRECTION) {
                    if (ref.prev().crossing()->index() != currentRef.crossing()->index()) {
                        neededCrossings.emplace_back(ref);
                    }
                }
                else {
                    if (ref.next().crossing()->index() != currentRef.crossing()->index()) {
                        neededCrossings.emplace_back(ref);
                    }
                }
            }
            
            walkOppDirVec.emplace_back(WALK_OPPOSITE_DIRECTION);
                    
            do {
                if (WALK_OPPOSITE_DIRECTION) {
                    if (isCurl(currentRef)) {
                        currentTwoHandleHighlightedCrossings.emplace_back(currentRef.prev());
                        currentRef = currentRef.prev().prev();
                    }
                    else {
                        currentTwoHandleHighlightedCrossings.emplace_back(currentRef);
                        currentRef = currentRef.prev();
                    }
                }
                else {
                    if (isCurl(currentRef)) {
                        currentTwoHandleHighlightedCrossings.emplace_back(currentRef);
                        currentRef = currentRef.next().next();
                    }
                    else {
                        currentTwoHandleHighlightedCrossings.emplace_back(currentRef);
                        currentRef = currentRef.next();
                    }
                }
            } while (!contains(neededCrossings,currentTwoHandleHighlightedCrossings));
            
//            currentTwoHandleHighlightedCrossings.erase(currentTwoHandleHighlightedCrossings.begin());
//            currentTwoHandleHighlightedCrossings.pop_back();
            
            highlightCrossings.emplace_back(currentTwoHandleHighlightedCrossings);
        }
                
        // debug
        if (printDebugInfo) {
            std::cout << "Highlighted crossings:" << std::endl;
            for (const auto& twoHandle : highlightCrossings) {
                for (const auto& ref : twoHandle) {
                    std::cout << ref << ", ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
        // end debug
    }
    /*
     END HIGHLIGHTING PROCEDURE
     */
    
    pdcode pdc = tmpLinkObj.pdData();
    
    graph<4> pdc_g;
    
    vertex v1  = { 1,0}, v2  = { 2,0}, v3  = { 3,0}, v4  = { 4,0};
    vertex v5  = { 5,0}, v6  = { 6,0}, v7  = { 7,0}, v8  = { 8,0};
    vertex v9  = { 9,1}, v10 = {10,1}, v11 = {11,1}, v12 = {12,1};
    vertex v13 = {13,2}, v14 = {14,2}, v15 = {15,2}, v16 = {16,2};
    vertex v17 = {17,3}, v18 = {18,3}, v19 = {19,3}, v20 = {20,3};
    vertex v21 = {21,4}, v22 = {22,4}, v23 = {23,4}, v24 = {24,4};
    
    vertex pca5 = {5,1}, pca6  = { 6,1}, pca7  = { 7,1}, pca8  = { 8,1};
    vertex pca9 = {9,2}, pca10 = {10,2}, pca11 = {11,2}, pca12 = {12,2};

    vertex pcb5 = {5,4}, pcb6  = { 6,4}, pcb7  = { 7,4}, pcb8  = { 8,4};
    vertex pcb9 = {9,3}, pcb10 = {10,3}, pcb11 = {11,3}, pcb12 = {12,3};

    vertex nca5 = {5,1}, nca6  = { 6,1}, nca7  = { 7,1}, nca8  = { 8,1};
    vertex nca9 = {9,4}, nca10 = {10,4}, nca11 = {11,4}, nca12 = {12,4};

    vertex ncb5 = {5,2}, ncb6  = { 6,2}, ncb7  = { 7,2}, ncb8  = { 8,2};
    vertex ncb9 = {9,3}, ncb10 = {10,3}, ncb11 = {11,3}, ncb12 = {12,3};
        
    std::vector<edge> posCross_el = {
        {v1, v6, 0},    {v1,v16,1}, {v1,v8,2},  {v1,v2,3},
        {v2, v5, 0},    {v2,v13,1}, {v2,v3,2},
        {v3, v11,0},    {v3,v12,1},             {v3,v8,3},
        {v4, v10,0},    {v4,v9, 1}, {v4,v5,2},  {v4,v7,3},
                        {v5,v24,1},             {v5,v6,3},
                        {v6,v21,1}, {v6,v7,2},
        {v7, v19,0},    {v7,v20,1},
        {v8, v18,0},    {v8,v17,1},
        {v14,v23,0},
        {v15,v22,0}
    };
    
    std::vector<edge> negCross_el = {
        {v1, v6, 0},    {v1,v24,1}, {v1,v8,2},  {v1,v2,3},
        {v2, v5, 0},    {v2,v21,1}, {v2,v3,2},
        {v3, v19,0},    {v3,v20,1},             {v3,v8,3},
        {v4, v18,0},    {v4,v17,1}, {v4,v5,2},  {v4,v7,3},
                        {v5,v16,1},             {v5,v6,3},
                        {v6,v13,1}, {v6,v7,2},
        {v7, v11,0},    {v7,v12,1},
        {v8, v10,0},    {v8,v9, 1},
        {v14,v23,0},
        {v15,v22,0}
    };
    
    std::vector<edge> posCurlA_el = {
        {v1,pca6, 0},   {v1,pca9, 1},   {v1,v2,2},  {v1,v4,3},
        {v2,pca7, 0},   {v2,pca8, 1},               {v2,v3,3},
        {v3,pca10,0},   {v3,pca5, 1},   {v3,v4,2},
        {v4,pca11,0},   {v4,pca12,1}
    };
    
    std::vector<edge> posCurlB_el = {
        {v1,pcb6, 0},   {v1,pcb9, 1},   {v1,v2,2},  {v1,v4,3},
        {v2,pcb7, 0},   {v2,pcb8, 1},               {v2,v3,3},
        {v3,pcb10,0},   {v3,pcb5, 1},   {v3,v4,2},
        {v4,pcb11,0},   {v4,pcb12,1}
    };
    
    std::vector<edge> negCurlA_el = {
        {v1,nca6, 0},   {v1,nca5, 1},   {v1,v2,2},  {v1,v4,3},
        {v2,nca7, 0},   {v2,nca12,1},               {v2,v3,3},
        {v3,nca10,0},   {v3,nca9, 1},   {v3,v4,2},
        {v4,nca11,0},   {v4,nca8, 1}
    };
    
    std::vector<edge> negCurlB_el = {
        {v1,ncb6, 0},   {v1,ncb5, 1},   {v1,v2,2},  {v1,v4,3},
        {v2,ncb7, 0},   {v2,ncb12,1},               {v2,v3,3},
        {v3,ncb10,0},   {v3,ncb9, 1},   {v3,v4,2},
        {v4,ncb11,0},   {v4,ncb8, 1}
    };
    
    posCross.add_edges(posCross_el);
    negCross.add_edges(negCross_el);
    posCurlA.add_edges(posCurlA_el);
    posCurlB.add_edges(posCurlB_el);
    negCurlA.add_edges(negCurlA_el);
    negCurlB.add_edges(negCurlB_el);

    std::vector<std::pair<int, int>> pdc_xot = pdc_xotype(pdc);
    
    long total_crossing_counter = 1;
    for (auto p : pdc_xot) {
//        std::cout << std::left << std::setw(6) << total_crossing_counter;
        ++total_crossing_counter;
        if ((p.first == 0) && (p.second == 1)) {
//            std::cout << "Generating Positive Crossing...\n";
            pdc_g.disjoint_union(posCross);
        }
        else if ((p.first == 0) && (p.second == -1)) {
//            std::cout << "Generating Negative Crossing...\n";
            pdc_g.disjoint_union(negCross);
        }
        else if (p.first == 1) {
//            std::cout << "Generating Positive Curl of Type A (a,b,x,x)...\n";
            pdc_g.disjoint_union(posCurlA);
        }
        else if (p.first == 2) {
//            std::cout << "Generating Positive Curl of Type B (x,x,c,d)...\n";
            pdc_g.disjoint_union(posCurlB);
        }
        else if (p.first == 3) {
//            std::cout << "Generating Negative Curl of Type A (a,x,x,d)...\n";
            pdc_g.disjoint_union(negCurlA);
        }
        else if (p.first == 4) {
//            std::cout << "Generating Negative Curl of Type B (x,b,c,x)...\n";
            pdc_g.disjoint_union(negCurlB);
        }
    }

    pdc_g.pd_sub(pdc);
            
    std::vector<std::pair<vertex, vertex>> pdc_fl = pdc_g.fuse_list();
    
    for (auto p : pdc_fl) {
        pdc_g.fuse(p.first, p.second);
    }
    
    if (dimFlag == 4) {
        std::vector<std::array<vertex, 4>> ql = pdc_g.quadriGraphFind();

        std::vector<std::array<vertex, 4>> finalGraphQuadriList(numberOfTwoHandles);
        
        // debug
        if (printDebugInfo) {
            std::cout << "Graph quadris:" << std::endl;
            for (const auto& quadri : ql) {
                std::cout << quadri[0].component << ", " << quadri[1].component << ", " << quadri[2].component << ", " << quadri[3].component << std::endl;
            }
            std::cout << std::endl;
        }
        //end debug
        
        for (int i=0; i<numberOfTwoHandles; i++) {
            auto linkQuadri = quadriPairRefs[i];
            std::set<int> linkIndices;
            linkIndices.emplace(linkQuadri.first.crossing()->index());
            linkIndices.emplace(linkQuadri.second.crossing()->index());
            for (const auto& graphQuadri : ql) {
                std::set<int> graphIndices;
                for (const auto& index : graphQuadri) {
                    graphIndices.emplace(index.component);
                }
                if (graphIndices == linkIndices) {
                    finalGraphQuadriList[i] = graphQuadri;
                    break;
                }
            }
        }
        
        // debug
        if (printDebugInfo) {
            std::cout << "Final Graph quadris:" << std::endl;
            for (const auto& quadri : finalGraphQuadriList) {
                std::cout << quadri[0].component << ", " << quadri[1].component << ", " << quadri[2].component << ", " << quadri[3].component << std::endl;
            }
            std::cout << std::endl;
        }
        //end debug

        std::vector<vertex> verts = pdc_g.vertices();
        
        std::vector<std::pair<vertex, vertex>> oneHandleMarkedVerts;
        for (const auto& p : oneHandleMarkedCrossingRefs) {
            int leftComp = p.first.crossing()->index();
            int rightComp = p.second.crossing()->index();
            std::pair<vertex, vertex> currentPair;
            
            int leftOrientation = pdc_xot[leftComp].second;
            int rightOrientation = pdc_xot[rightComp].second;
            
            if (leftOrientation == 1) {
                currentPair.first = {7,0,leftComp};
            }
            if (leftOrientation == -1) {
                currentPair.first = {3,0,leftComp};
            }
            if (rightOrientation == 1) {
                currentPair.second = {4,0,rightComp};
            }
            if (rightOrientation == -1) {
                currentPair.second = {8,0,rightComp};
            }

            oneHandleMarkedVerts.emplace_back(currentPair);
        }

        // debug
        if (printDebugInfo) {
            std::cout << "One-Handle Marked Vertices:" << std::endl;
            for (const auto& p : oneHandleMarkedVerts) {
                std::cout << p << std::endl;
            }
            std::cout << std::endl;
        }
        // end debug
        
        if (printDebugInfo) {
            std::cout << "Number of graph vertices: " << pdc_g.size() << std::endl;
            std::cout << std::endl;
        }
        
        pdc_g.DEBUGremainingNo4ColVerts();
        
        pdc_g.addQuadriEdges(finalGraphQuadriList);
        pdc_g.DEBUGremainingNo4ColVerts();
        if (existOneHandles) {
//            pdc_g.DEBUGremainingNo4ColVerts();
            pdc_g.addOneHandleIdentEdges(oneHandleMarkedVerts);
            pdc_g.DEBUGremainingNo4ColVerts();
            pdc_g.addHighlightEdges(highlightCrossings);
            pdc_g.addDoubleOneEdges();
            pdc_g.addRemainderEdges();
        }
        else {
            pdc_g.addDoubleOneEdges();
        }
        pdc_g.cleanup();
        pdc_g.DEBUGno4ColVerts();

        if (outputGraph == false) {
            std::vector<std::tuple<int,int,int>> gl = gluingList(pdc_g);

            regina::Triangulation<4> tmp_t;
            regina::Perm<5> perm;
            std::vector<regina::Simplex<4>> tmp_s;
            tmp_t.newPentachora(pdc_g.size());
            for (const auto& g : gl) {
                tmp_t.pentachoron(std::get<0>(g))->join(std::get<2>(g),tmp_t.pentachoron(std::get<1>(g)),perm);
            }
            std::cout << std::endl;
            std::cout << "Triangulation is valid: " << (tmp_t.isValid() ? "Yes" : "NO") << std::endl;
            
            std::cout << "\r " << std::endl;
            std::cout << "\rHere is the isomorphism signature:\n" << std::flush;
            std::cout << tmp_t.isoSig() << "\n";
            
        }
        else {
            std::cout << "\rHere is the coloured graph edge list/facet identification list:\n" << std::flush;
            printGluingList(pdc_g);
        }
    }
    else if (dimFlag == 3) {
        if (outputGraph == false) {

            pdc_g.cleanup();

            std::vector<std::tuple<int,int,int>> gl = gluingList(pdc_g);
            
            regina::Triangulation<3> tmp_t;
            regina::Perm<4> perm;
            std::vector<regina::Simplex<3>> tmp_s;
            tmp_t.newTetrahedra(pdc_g.size());
            for (const auto& g : gl) {
                tmp_t.tetrahedron(std::get<0>(g))->join(std::get<2>(g),tmp_t.tetrahedron(std::get<1>(g)),perm);
            }

            std::cout << "\r " << std::endl;
            std::cout << "\rHere is the isomorphism signature:\n" << std::flush;
            std::cout << tmp_t.isoSig() << "\n";
        }
        else {
            std::cout << "\rHere is the coloured graph edge list/facet identification list:\n" << std::flush;
            printGluingList(pdc_g);
        }
    }
    return 0;
}
