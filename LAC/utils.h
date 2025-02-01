#include <vector>
#include <fstream>

std::vector<std::vector<int> > readWorkloadPairs(const char* filename){
    std::ifstream infile(filename);
    std::vector<std::vector<int> > ret;
    int src,dst,temp;
    while (infile >> src >> dst) { //while (infile >> src >> dst >> temp) {
        std::vector<int> v;
        v.push_back(src);
        v.push_back(dst);
        ret.push_back(v);
    }
    return ret;
}

std::vector<int> readQueryTypeMix(const char* filename){
    std::ifstream infile(filename);
    std::vector<int> ret;
    int query;
    while (infile >> query) { //while (infile >> src >> dst >> temp) {
        ret.push_back(query);
    }
    return ret;
}
