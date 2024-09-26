This repository is intended to simulate an out of core graph processing where the graph is laid out in pages on disk and memory cache fetches pages on demand. The objective is to compare Locality Aware Cache Replacement Policy (LAC) against its competitors while running various traversal queries.

LAC: Simulation is on both vertex and edge pages to compare LAC against GCLOCK.

LAC-Vertex: Simulation is on vertex pages only to compare LAC against GRASP.
