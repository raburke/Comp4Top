# *Katie*, USDS, and more
## Software for Triangulating and Simplifying 4-Manifold Triangulations
This repository contains source code, and example data, for triangulating smooth 4-manifolds via Kirby diagrams, and a heuristic for simplifying the resulting triangulations.

The main contents of this repository are:

 - `katie.cc`: the source code for *Katie* -- the main program for building triangulations from Kirby diagrams.
 - `usds.cc`: a utility which implements the Up-Side-Down Simplification heuristic.
 -  `bfb.cc`: source for *Brute Force Builder* which builds triangulations from a seed triangulation with (real) boundary.

All of the C++ programs here need to be compiled against the [*Regina*](https://regina-normal.github.io/) libraries. This is most easily accomplished on Linux or MacOS using the new `regina-helper` utility available via the [latest build of Regina](https://github.com/regina-normal/regina). Windows users will need to wait for the compiled binaries, to be bundled with the official release of Regina 7.4.
 
In addition, there are also a number of Python scripts (to be used within *Regina*) for building certain triangulations and complexes, a rough work-in-progress script for locating embedded (minimal) tori within 4-manifold triangulations, and a *Regina* data file containing various triangulations discussed in [this paper](https://arxiv.org/abs/2402.15087).

## *GluB*
This is a new (added Dec 2024) utility for gluing two 4-manifolds with boundary together. Given either (i) a 4-manifold with two real boundary components, or (ii) two 4-manifolds each with one real boundary component, `glub` (*Glu*e *B*oundaries) will glue the boundaries together using all possible combinatorial isomorphisms of the boundaries and enumerate the resulting triangulations. As suggested by the previous sentence, the boundaries in question must be combinatorially isomorphic (not just homeomorphic).

-- *Rhuaidi Burke* (rhuaidi.burke@uq.edu.au)
