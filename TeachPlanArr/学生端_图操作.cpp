#include "student_head.h"

namespace StudentApp {

void buildGraph() {
    for (int i = 0; i < MAX_COURSE; ++i) {
        prereqList[i].clear();
        if (i < courseCount) courses[i].next.clear();
    }
    for (const auto& edge : prereqPairs) {
        const int prereq = edge.first;
        const int successor = edge.second;
        courses[prereq].next.push_back(successor);
        prereqList[successor].push_back(prereq);
    }
}

void calculateIndegree() {
    for (int i = 0; i < courseCount; ++i) {
        courses[i].indegree = static_cast<int>(prereqList[i].size());
        indegreeOf[i] = courses[i].indegree;
    }
}

} // namespace StudentApp
