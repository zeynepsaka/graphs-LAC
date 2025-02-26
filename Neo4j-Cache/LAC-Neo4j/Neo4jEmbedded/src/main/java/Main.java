//package org.example;

import org.neo4j.configuration.GraphDatabaseSettings;
import org.neo4j.dbms.api.DatabaseManagementService;
import org.neo4j.dbms.api.DatabaseManagementServiceBuilder;
import org.neo4j.graphalgo.GraphAlgoFactory;
import org.neo4j.graphdb.*;
import org.neo4j.graphdb.traversal.TraversalDescription;
import org.neo4j.graphdb.traversal.*;
import org.neo4j.internal.helpers.collection.Iterables;
import org.neo4j.io.pagecache.PageCache;
import org.neo4j.io.pagecache.PageCursor;
import org.neo4j.io.pagecache.impl.muninn.MuninnPageCache;
import org.neo4j.io.pagecache.impl.muninn.MuninnPageCursor;

import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.AtomicReferenceArray;
import java.util.stream.StreamSupport;


public class Main {

    public static HashMap<String, String> nodeIdToElementIdMap = new HashMap<>();
    public static HashMap<String, Integer> nodeIdToDegreeMap = new HashMap<>();


    static long initialFaults = 0;
    static long initialHits = 0;
    static long totalTime = 0;

    public static void main(String[] args) {

        String graph = "soclj";
        int cacheSize = 5;
        int query = 0; //spsp:0, fr-all:1, fr-hot:2, rw:3
        int NWALK = 15;
        int NSTEP = 3;
        int depth = 3;
        depth = (int)Math.ceil(depth/2.0); //bi-directional from each side
        int p = NWALK;
        int workloadSize = 50000;
        String workloadFilePath = "";
        String dbPath = "";

        if (args.length > 0) {
            dbPath = args[0];
            graph = args[1]; //flickr soclj amazon cit twitterrv
            cacheSize = Integer.parseInt(args[2]);
            query = Integer.parseInt(args[3]);

            depth = Integer.parseInt(args[4]);
            depth = (int)Math.ceil(depth/2.0); //bi-directional from each side
            NWALK = Integer.parseInt(args[5]);
            NSTEP = Integer.parseInt(args[6]);
            p=NWALK;
            workloadFilePath = args[7];
            workloadSize = Integer.parseInt(args[8]);
        }
        File DB_PATH = new File(dbPath);

        long cache = 5; //it will be updated after store files' counts return
        // Initialize the embedded database for db stats
        long pageCacheSizeInBytes = cache * 1024L * 1024L;
        DatabaseManagementService managementService = new DatabaseManagementServiceBuilder(DB_PATH.toPath())
                .setConfig(GraphDatabaseSettings.pagecache_direct_io, true)
                .setConfig(GraphDatabaseSettings.initial_default_database, graph)
                .setConfig(GraphDatabaseSettings.pagecache_warmup_enabled, false)
                .setConfig(GraphDatabaseSettings.pagecache_memory, pageCacheSizeInBytes)
                .setConfig(GraphDatabaseSettings.pagecache_scan_prefetch, 0)
                .build();
        GraphDatabaseService db = managementService.database(graph);
        //System.out.println(pageCacheSizeInBytes);

        int[] nums = dbStats(db, cacheSize);
        managementService.shutdown();

        cache = (long) nums[3];
        pageCacheSizeInBytes = cache * 1024L * 1024L;
        //System.out.println(pageCacheSizeInBytes);

            managementService = new DatabaseManagementServiceBuilder(DB_PATH.toPath())
                    .setConfig(GraphDatabaseSettings.pagecache_direct_io, true)
                    .setConfig(GraphDatabaseSettings.initial_default_database, graph)
                    .setConfig(GraphDatabaseSettings.pagecache_warmup_enabled, false)
                    .setConfig(GraphDatabaseSettings.pagecache_memory, pageCacheSizeInBytes)
                    .setConfig(GraphDatabaseSettings.pagecache_scan_prefetch, 0)
                    .build();
            db = managementService.database(graph);
            MuninnPageCursor.diskPagesRel = new float[nums[0]];
            MuninnPageCursor.diskPagesNode = new float[nums[1]];
            MuninnPageCursor.diskPagesProp = new float[nums[2]];

            Runtime.getRuntime().addShutdownHook(new Thread(managementService::shutdown));

            try {
                findNodesElementID(nodeIdToElementIdMap, db);
                initialFaults = PageCursor.tracer.faults();
                initialHits = PageCursor.tracer.hits();
                PageCursor.relMiss = 0;
                PageCursor.nodeMiss = 0;
                PageCursor.otherMiss = 0;

                System.out.println("Query: " + query);

                if (query == 0) {
                    biSPSP(db, workloadFilePath, depth, workloadSize); //2000
                } else if (query == 1) {
                    frALLAPI(db, workloadFilePath, 1, -1, workloadSize); //50000
                } else if (query == 2) {
                    findNodesDegree(db);
                    initialFaults = PageCursor.tracer.faults();
                    initialHits = PageCursor.tracer.hits();
                    PageCursor.relMiss = 0;
                    PageCursor.nodeMiss = 0;
                    PageCursor.otherMiss = 0;
                    frALLHOTAPI(db, workloadFilePath, 1, p, workloadSize); //50000
                }  else if (query == 3) {
                    randomWalk2(db, workloadFilePath, NSTEP, NWALK, workloadSize);//5000
                }

            } catch (IOException e) {
                e.printStackTrace();
            }
            managementService.shutdown();
    }

    public static int[] dbStats(GraphDatabaseService db, int cacheSize){

        int[] nums = new int[4];
        int i = 0;
        try (Transaction tx = db.beginTx()) {
            Result result = tx.execute("CALL db.stats.retrieve('GRAPH COUNTS')");
            while (result.hasNext()) {
                Map<String, Object> stats = result.next();
                Map<String, Object> stats2 = (Map<String, Object>) stats.get("data");
                for (String key2 : stats2.keySet()) {
                    ArrayList<Map<String, Object>> relList = (ArrayList<Map<String, Object>>) stats2.get(key2);
                    for(String k : relList.getFirst().keySet()){
                        nums[i++] = Integer.valueOf(relList.getFirst().get(k).toString());
                        //System.out.println(k + ": " + relList.getFirst().get(k));
                        if(i>1) break;
                    }
                    if(i>1) break;
                }
            }

            tx.commit();
        }
        nums[0] = (int)Math.ceil(nums[0]/200.0);
        nums[1] = (int)Math.ceil(nums[1]/350.0);
        nums[2] = nums[1] * 2;
        int size  = (int)(Math.ceil(((nums[0]+nums[1]+nums[2])*8)/1024.0)*0.05);
        if (cacheSize == 10) {
            size *= 2;
        } else if (cacheSize == 20) {
            size *= 4;
        } else if (cacheSize == 30) {
            size *= 6;
        }
        nums[3] = size;
        //System.out.println(nums[0] + " " + nums[1] + " " + nums[2]  + ", cache size:" + nums[3]);
        return nums;
    }

    private static void randomWalk2(GraphDatabaseService db, String workloadFilePath, int NSTEP, int NWALK, int workloadSize) throws IOException{

        Random random = new Random(1);
        try (BufferedReader br = new BufferedReader(new FileReader(workloadFilePath))) {
            String line;
            try (Transaction tx = db.beginTx()) {

                for (int i = 0; i < workloadSize; i++) {
                    //System.out.println(i);
                    line = br.readLine();
                    String[] ids = line.split(" ");
                    String startElementId = ids[0].trim();
                    String startInternal = nodeIdToElementIdMap.get(startElementId);
                    long startTime = System.currentTimeMillis();
                    Node startNode = tx.getNodeByElementId(startInternal);
                    for(int k = 1; k<NWALK+1; k++) {
                        random = new Random(k);
                        for (int j = 0; j < NSTEP; j++) {
                            startNode.getElementId();
                            // Get relationships of the current node
                            Iterable<Relationship> relationships = startNode.getRelationships(Direction.OUTGOING);
                            // Randomly select the next relationship
                            Relationship[] relArray = toArray(relationships);
                            if (relArray.length == 0) {
                                //System.out.println("No more neighbors to traverse.");
                                break;
                            }

                            Relationship randomRel = relArray[random.nextInt(relArray.length)];
                            randomRel.getElementId();
                            startNode = randomRel.getOtherNode(startNode);
                        }
                    }
                    long endTime = System.currentTimeMillis();
                    long duration = endTime - startTime;
                    totalTime += duration;
                }

                tx.commit();
                //System.out.println("Initial faults: " + initialFaults);
                System.out.println("Total faults: " + (PageCursor.tracer.faults()-initialFaults));
                System.out.println("Node faults: " + PageCursor.nodeMiss);
                System.out.println("Rel faults: " + PageCursor.relMiss);
                System.out.println("Other faults: " + PageCursor.otherMiss);
                System.out.println("Execution time: " + totalTime + " ms");
            }
        }
    }

    private static Relationship[] toArray(Iterable<Relationship> relationships) {
        return StreamSupport.stream(relationships.spliterator(), false).toArray(Relationship[]::new);
    }

    public static long frALLAPI(GraphDatabaseService db, String workloadFilePath, int depth, int p, int workloadSize) throws IOException{
        try (BufferedReader br = new BufferedReader(new FileReader(workloadFilePath))) {
            String line;
            try (Transaction tx = db.beginTx()) {
                TraversalDescription traversal = tx.traversalDescription()
                        .breadthFirst()
                        .relationships(RelationshipType.withName("CONNECT"), Direction.OUTGOING)
                        .evaluator(Evaluators.toDepth(depth)); // Limit to the specified depth

                for (int i = 0; i < workloadSize; i++) {
                    //System.out.println(i);
                    line = br.readLine();
                    String[] ids = line.split(" ");
                    String startElementId = ids[0].trim();
                    String startInternal = nodeIdToElementIdMap.get(startElementId);
                    long startTime = System.currentTimeMillis();
                    Node startNode = tx.getNodeByElementId(startInternal);
                    Traverser traverser = traversal.traverse(startNode);
                    for (Path path : traverser) {
                        for (Node n : path.nodes()) {
                            n.getElementId();
                            n.getProperty(" propID");
                        }
                        for (Relationship r : path.relationships()) {
                            r.getElementId();
                        }
                    }
                    long endTime = System.currentTimeMillis();
                    long duration = endTime - startTime;
                    totalTime += duration;
                }

                tx.commit();
                //System.out.println("Initial faults: " + initialFaults);
                System.out.println("Total faults: " + (PageCursor.tracer.faults()-initialFaults));
                System.out.println("Node faults: " + PageCursor.nodeMiss);
                System.out.println("Rel faults: " + PageCursor.relMiss);
                System.out.println("Other faults: " + PageCursor.otherMiss);
                System.out.println("Execution time: " + totalTime + " ms");
            }
        }
        return totalTime;
    }

    public static long frALLHOTAPI(GraphDatabaseService db, String workloadFilePath, int depth, int p, int workloadSize) throws IOException{
        try (BufferedReader br = new BufferedReader(new FileReader(workloadFilePath))) {
            String line;
            try (Transaction tx = db.beginTx()) {
                TraversalDescription traversal = tx.traversalDescription()
                        .breadthFirst()
                        .relationships(RelationshipType.withName("CONNECT"), Direction.OUTGOING)
                        .evaluator(Evaluators.toDepth(depth)); // Limit to the specified depth

                for (int i = 0; i < workloadSize; i++) {
                    //System.out.println(i);
                    line = br.readLine();
                    String[] ids = line.split(" ");
                    String startElementId = ids[0].trim();
                    String startInternal = nodeIdToElementIdMap.get(startElementId);

                    long startTime = System.currentTimeMillis();
                    Node startNode = tx.getNodeByElementId(startInternal);
                    Traverser traverser = traversal.traverse(startNode);
                    for (Path path : traverser) {
                        for (Node n : path.nodes()) {
                            if(nodeIdToDegreeMap.get(n.getElementId())>=p){
                                n.getElementId();
                                n.getProperty(" propID");
                            }
                        }
                        for (Relationship r : path.relationships()) {
                            if(nodeIdToDegreeMap.get(r.getStartNode().getElementId())>=p){
                                r.getElementId();
                            }
                        }
                    }
                    long endTime = System.currentTimeMillis();
                    long duration = endTime - startTime;
                    totalTime += duration;
                }

                tx.commit();
                //System.out.println("Initial faults: " + initialFaults);
                System.out.println("Total faults: " + (PageCursor.tracer.faults()-initialFaults));
                System.out.println("Node faults: " + PageCursor.nodeMiss);
                System.out.println("Rel faults: " + PageCursor.relMiss);
                System.out.println("Other faults: " + PageCursor.otherMiss);
                System.out.println("Execution time: " + totalTime + " ms");
            }
        }
        return totalTime;
    }

    private static void findNodesElementID(HashMap<String, String> nodeIdToElementIdMap, GraphDatabaseService db) {

        // Iterate through all nodes and store their internal IDs and element IDs
        try (Transaction tx = db.beginTx()) {
            int count = 0;
            for (Node node : tx.getAllNodes()) {
                //System.out.println(count++);
                String s = node.getElementId();
                int i = s.lastIndexOf(":");
                String id = s.substring(i+1);
                //String elementId = node.getElementId();
                nodeIdToElementIdMap.put(id, s); //2 -> elementID
                //nodeIdToDegreeMap.put(s, node.getDegree(Direction.OUTGOING));
            }
            tx.commit();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private static void findNodesDegree(GraphDatabaseService db) {
        try (Transaction tx = db.beginTx()) {
            for (Node node : tx.getAllNodes()) {
                String s = node.getElementId();
                nodeIdToDegreeMap.put(s, node.getDegree(Direction.OUTGOING));
            }
            tx.commit();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public static long biSPSP(GraphDatabaseService db, String workloadFilePath, int depth, int workloadSize) throws IOException {

        try (BufferedReader br = new BufferedReader(new FileReader(workloadFilePath))) {
            String line;
            try (Transaction tx = db.beginTx()) {
                for (int i = 0; i <workloadSize; i++) {
                    System.out.println(i);
                    line = br.readLine();
                    String[] ids = line.split(" ");
                    String startElementId = ids[0].trim();
                    String endElementId = ids[1].trim();

                    String startInternal = nodeIdToElementIdMap.get(startElementId);
                    String endInternal = nodeIdToElementIdMap.get(endElementId);

                    // Locate the start and end nodes by their elementId
                    long startTime = System.currentTimeMillis();
                    Node startNode = tx.getNodeByElementId(startInternal);
                    Node endNode = tx.getNodeByElementId(endInternal);
                    
                    bidirectionalShortestPath(startNode, endNode,depth);

                    long endTime = System.currentTimeMillis();
                    long duration = endTime - startTime;
                    totalTime += duration;
                }
                tx.commit();
                //System.out.println("Initial faults: " + initialFaults);
                System.out.println("Total faults: " + (PageCursor.tracer.faults()-initialFaults));
                System.out.println("Node faults: " + PageCursor.nodeMiss);
                System.out.println("Rel faults: " + PageCursor.relMiss);
                System.out.println("Other faults: " + PageCursor.otherMiss);
                System.out.println("Execution time: " + totalTime + " ms");
            }
        }
        return totalTime;
    }

    static class PathResult {
        List<Node> path;
        Set<Node> touchedNodes;

        PathResult(List<Node> path, Set<Node> touchedNodes) {
            this.path = path;
            this.touchedNodes = touchedNodes;
        }
    }

    public static void bidirectionalShortestPath(Node startNode, Node endNode, int maxDepth) { //PathResult
        // Queues for bidirectional search
        Queue<NodeDepth> startQueue = new LinkedList<>();
        Queue<NodeDepth> endQueue = new LinkedList<>();

        // Sets for visited nodes
        Set<Node> startVisited = new HashSet<>();
        Set<Node> endVisited = new HashSet<>();

        // Maps to track predecessors for reconstructing paths
        Map<Node, Node> startPredecessors = new HashMap<>();
        Map<Node, Node> endPredecessors = new HashMap<>();

        // Add start and end nodes to their respective queues with depth 0
        startQueue.add(new NodeDepth(startNode, 0));
        startVisited.add(startNode);
        startNode.getProperty(" propID");

        endQueue.add(new NodeDepth(endNode, 0));
        endVisited.add(endNode);
        endNode.getProperty(" propID");

        Node meetingNode = null;

        // Bidirectional BFS
        while (!startQueue.isEmpty() && !endQueue.isEmpty()) {
            if (bidirectionalStep(startQueue, startVisited, startPredecessors, endVisited, maxDepth)) {
                meetingNode = findMeetingNode(startVisited, endVisited);
                break;
            }

            if (bidirectionalStep(endQueue, endVisited, endPredecessors, startVisited, maxDepth)) {
                meetingNode = findMeetingNode(startVisited, endVisited);
                break;
            }
        }
    }

    private static boolean bidirectionalStep(Queue<NodeDepth> queue, Set<Node> visited,
                                                Map<Node, Node> predecessors, Set<Node> otherVisited, int maxDepth) {
        if (!queue.isEmpty()) {
            NodeDepth current = queue.poll();
            Node currentNode = current.node;
            currentNode.getProperty(" propID");
            int currentDepth = current.depth;

            if (currentDepth >= maxDepth) {
                return false;
            }

            for (Relationship relationship : currentNode.getRelationships(Direction.BOTH)) {
                Node neighbor = relationship.getOtherNode(currentNode);
                if (!visited.contains(neighbor)) {
                    visited.add(neighbor);
                    neighbor.getProperty(" propID");
                    predecessors.put(neighbor, currentNode);
                    queue.add(new NodeDepth(neighbor, currentDepth + 1));

                    if (otherVisited.contains(neighbor)) {
                        return true; // Meeting point found
                    }
                }
            }
        }
        return false;
    }

    private static Node findMeetingNode(Set<Node> startVisited, Set<Node> endVisited) {
        for (Node node : startVisited) {
            if (endVisited.contains(node)) {
                return node;
            }
        }
        return null;
    }

    static class NodeDepth {
        Node node;
        int depth;

        NodeDepth(Node node, int depth) {
            this.node = node;
            this.depth = depth;
        }
    }

}
