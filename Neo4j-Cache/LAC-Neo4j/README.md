# README
The source code in this repository is intended to run an embedded-Neo4j java application to run various queries on input graphs. 
This project requires JAVA 23 or above and it is test in Ubuntu 20.04
The Neo4j version is used in the project is 5.20.

The folders in this reporsitory and their purposes are as follows:

**1. LACNeo4jEmbeddedJAR:** This folder contains Neo4j jar files whose io module has been modified according to LAC's principles. It also contains a jar file called "LACNeo4jEmbed.jar" whose source code is also given under this repository and explained in (4).

**2. runLAC.sh:** This script file runs "LACNeo4jEmbed.jar" with the user-defined parameters. It includes the command "--add-opens java.base/java.nio=ALL-UNNAMED" to establish direct IO. First set parameter in runLAC.sh and then run as ./runLAC.sh. Parameter setting manual is given at the end of this file.

1 and 2 can be used to test LAC in Neo4j to skip building and embedding Neo4j into the java application in 4.

**3. Neo4jLAC:** This is "/io" module of Neo4j source code with LAC implementation. (The only module is modified for now). You can download the full source code here: https://github.com/neo4j/neo4j/tree/5.20 and replace the "/community/io" module with this folder. Then, it can be build by running "mvn clean install -DskipTests -T1C -Dspotless.check.skip=true" under "/neo4j". The generated jar files can be found under "packaging/standalone/target". You can include these jar files to embedded Neo4j into the java application.

**4. Neo4jEmbedded: ** This is the source code for the java application where the embedded Neo4j is created and the queries are implemented by using Java API. It requires including Neo4j libraries which can be done by including the following lines in pom.xml after Neo4j-LAC is build in (3):

<dependencies>
        <!-- Neo4j Embedded -->
        <dependency>
            <groupId>org.neo4j</groupId>
            <artifactId>neo4j</artifactId>
            <version>5.20.0-SNAPSHOT</version>
        </dependency>
</dependencies>

If you are using an IDE, the following line need to be added into Build and Run configurations. This is required to perform direct IO:

--add-opens java.base/java.nio=ALL-UNNAMED


## Parameter Setting in runLAC.sh
**dbPATH:** Set the path for the databases. You can find the example databases under \DB directory. (Example: "..\DB")

**graph:** Specify the name of the graph under "\DB\data\databases". (Example: "soclj")

**cache_size:** Specify the cache size percentage; 5, 10, 20 or 30.

**query_type:** Specify the query type. 0: SPSP, 1:FR-ALL, 2:FR-HOT, 3:RW

**depth**: Determines the depth of a bi-SPSP query to traverse until it finds a path. You can leave it as is in runLAC.sh, if SPSP query is not chosen.

**NW**: number of walks initiated on a vertex in Random Walks. You can leave it as is in runLAC.sh, if RW query is not chosen.

**NS**: number of steps in depth from a vertex in Random Walks. You can leave it as is in runLAC.sh, if RW query is not chosen.

**workload_file**: This file contains two integer in each line representing the start and end vertices for queries (end vertex is only needed for SPSP query type). The number of lines determines the maximum number for workload size. Integers for start and end vertices should be in the range of 0 to N, where N is the number of vertices in the input graph. (Example: “Neo4j-Cache/workloads/WorkloadTraceSocLJ500K”)

**workload_size**: Determines the number of query to be run in the specified query type.

The output shows the number of page faults after running the specified type and number of queries when the cache size is as set in runLAC.sh. The output also includes execution time in ms.
