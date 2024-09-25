#include <cstdlib>
#include "graph.h"
#include "utils.h"
#include <chrono>

int main(int argc, char *argv[]) {
  
  const char* vertex_file = argv[1];
  const char* edge_file = argv[2];
  const char* workload_file = argv[3];
  int workload_size = atoi(argv[4]);
  int cache_size = atoi(argv[5]);
  int policy = atoi(argv[6]);
  int query_type = atoi(argv[7]);
  int SP_r = atoi(argv[8]);
  int NW = atoi(argv[9]);
  int NS = atoi(argv[10]);

  Graph graph(512, cache_size);
  std::vector<std::vector<int> > fromToData; 

  graph.createGraphFromFile(edge_file);
  graph.loadDataToStorage(vertex_file, policy);
  fromToData = readWorkloadPairs(workload_file);
  graph.maxSPLevel = SP_r;
  double p=graph.edges.size()/graph.nodes.size();

  int tempQuery = -1;
  std::vector<int> queryTypeMix;
  if(query_type == 5){
  	queryTypeMix = readQueryTypeMix("queryTypeMixed");
  	tempQuery = 5;
  }
    
  double totalPolicyTime = 0;
  int retID = 0;
  for (int i = 0; i <workload_size; i++) { 
  	if(tempQuery == 5){
  		query_type = queryTypeMix[i];
      if(query_type == 0 || query_type == 3){
        continue;
      }
    }
  	
  	if(query_type == 0){
  		totalPolicyTime += graph.bidiSP(fromToData[i][0], fromToData[i][1]); //SPSP-r
      		if(i==50000) break; //SPSP queries maximum workload size
  	}
  	else if(query_type == 1){
  		totalPolicyTime +=  graph.fr_all_2(fromToData[i][0]); //FR-ALL-2
  	}
  	else if(query_type == 2){
  		totalPolicyTime += graph.fr_all(fromToData[i][0], 0); //FR-ALL
  	}
  	else if(query_type == 3){
  		totalPolicyTime += graph.fr_all(fromToData[i][0], p); //FR-HOT
  	}
  	else if(query_type == 4){
  		totalPolicyTime +=  graph.rw(fromToData[i][0], NS, NW); //RW
  	}
  }
 

  long numMiss = graph.printStats();

  double totalTime = (numMiss * 0.05) + totalPolicyTime;
  printf("total time 0.05: %f\n", totalTime);

  totalTime = (numMiss * 0.15) + totalPolicyTime;
  printf("total time 0.15: %f\n", totalTime);

  totalTime = (numMiss * 0.45) + totalPolicyTime;
  printf("total time 0.45: %f\n", totalTime);
  
  totalTime = (numMiss * 1.35) + totalPolicyTime;
  printf("total time 1.35: %f\n", totalTime);

 
  return 0;
}

  
  