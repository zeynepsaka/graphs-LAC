# README
The source code in this repository is intended to simulate an out of core graph processing where the graph is laid out in pages on disk and memory cache fetches pages on demand. The objective is to compare Locality Aware Cache Replacement Policy (LAC) against GRASP’s hit promotions on vertex pages while running various traversal queries.

1. Run ./build.sh to compile binary.
2. Set parameters in run_LACVertex.sh.
3. The simulation can be run as ./run_LACVertex.sh

## Parameter Setting

**vertex_file**: Input vertex order. Vertices will be laid out in pages according to the order in this file. Example: “socLJ-goDBGNode.txt”

**edge_file**: Input edge order. Each line represents an edge of the input graph. First integer is the start vertex and the second integer is the end vertex of the edge. Start vertices should follow the order the vertex_file. Example: “/datasets/socLJ-goDBGEdge.txt”

(If policy is set to LAC (policy=1), vertex and edge file need to be ordered by using Gorder only. Example: “socLJ-goNode.txt”.
If policy is set to GRASP (policy=3), vertex and edge file need to be ordered by using Gorder first and then DBG. Example: “socLJ-goDBGNode.txt”.)

**workload_file**: This file contains two integer in each line representing the start and end vertices for queries (end vertex is only needed for SPSP query type). The number of lines determines the maximum number for workload size. Integers for start and end vertices should be in the range of 0 to N, where N is the number of vertices in the input graph. Example: “/workloads/WorkloadTraceSocLJ500K”

**workload_size**: Determines the number of query to be run in the specified query type.

**cache_size**: Determines the percentage for cache size (2, 5, 10, 20, 30).

**policy**: Determines the policy. 1: LAC, 3: GRASP.

**query_type**: 0: SPSP, 1:FR-ALL-2, 2:FR-ALL, 3:FR-HOT, 4:RW, 5:MIX

**SP-r**: Determines the depth of a SPSP query to traverse until it finds a path. You can leave it as is in run_LAC.sh, if SPSP query is not chosen.

**NW**: number of walks initiated on a vertex in Random Walks. You can leave it as is in run_LAC.sh, if RW query is not chosen.

**NS**: number of steps in depth from a vertex in Random Walks. You can leave it as is in run_LAC.sh, if RW query is not chosen.

The output shows the number of hits and misses after running the specified type and number of queries when the cache size and policy is as set in run_LAC.sh. The output also includes the latency in ms for different IO latency parameters in ms (0.05, 0.15, 0.45, 1.35).


