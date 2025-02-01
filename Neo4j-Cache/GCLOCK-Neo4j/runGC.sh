#!/bin/bash
CMD="java --add-opens java.base/java.nio=ALL-UNNAMED -jar GCNeo4jEmbeddedJAR/GCNeo4jEmbed.jar"
DASH=_

#Parameters:
db_path="../DB"
graph="soclj"
cache_size=5
query_type=0
depth=3
NW=15
NS=6
workload_file="../workloads/WorkloadTraceSocLJ500K"
workload_size=5000


CMDTEXT="$CMD $db_path $graph $cache_size $query_type $depth $NW $NS $workload_file $workload_size"
echo $CMDTEXT
eval $CMDTEXT
