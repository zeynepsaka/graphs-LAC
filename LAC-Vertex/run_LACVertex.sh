#!/bin/bash
CMD="time ./LacVertex"
DASH=_

#Parameters:
vertex_file="datasets/socLJ_goDBGNode.txt"
edge_file="datasets/socLJ_goDBGEdge.txt"
#vertex_file="datasets/socLJ_goNode.txt"
#edge_file="datasets/socLJ_goEdge.txt"
workload_file="workloads/WorkloadTraceSocLJ500K"
workload_size=500000
cache_size=20
policy=3
query_type=4
SP_r=3
NW=14
NS=6

CMDTEXT="$CMD $vertex_file $edge_file $workload_file $workload_size $cache_size $policy $query_type $SP_r $NW $NS"
echo $CMDTEXT
eval $CMDTEXT


#Remove comments for batch run and to print output in corresponding files
    # for query_type in 0 1 2 3 4 5
    # do
	#     for cache_size in 2 5 10 20 30
	#     do
	# 		for policy in 1 3
	# 		do
	# 		    FILENAME="$vertex_file$DASH$query_type$DASH$cache_size$DASH$policy.txt"
	# 		    CMDTEXT="$CMD $vertex_file $edge_file $workload_file $workload_size $cache_size $policy $query_type $SP_r $NW $NS"
	# 		    echo $CMDTEXT
	# 		    eval $CMDTEXT |& tee $FILENAME
	# 		    sleep 5
	# 		done #policy
	#     done #CACHE
    #  done #QUERY


