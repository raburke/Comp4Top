# *Katie* and USDS
## Software for Triangulating and Simplifying 4-Manifold Triangulations
This repository contains source code, and example data, for triangulating smooth 4-manifolds via Kirby diagrams, and a heuristic for simplifying the resulting triangulations.

The main contents of this repository are:

 - `katie.cc`: the source code for *Katie* -- the main program for building triangulations from Kirby diagrams.
 - `usds.cc`: a utility which implements the Up-Side-Down Simplification heuristic.
 -  `bfb.cc`: source for *Brute Force Builder* which builds triangulations from a seed triangulation with (real) boundary.

All of the C++ programs here need to be compiled against the *Regina* libraries.
 
In addition, there are also a number of Python scripts (to be used within *Regina*) for building certain triangulations and complexes, a rough work-in-progress script for locating embedded (minimal) tori within 4-manifold triangulations, and a *Regina* data file containing various triangulations discussed in [this paper](https://arxiv.org/abs/2402.15087).

--- *Rhuaidi Burke* (rhuaidi.burke@uq.edu.au)
