#include "course_system.h"

#include <cstring>
#include <iostream>
#include <queue>

// ============================================================================
//  文件：子功能_图操作.cpp
//  内容：图结构模块（实现 course_system.h 中声明的图操作函数）
// ============================================================================

// ---------------------------------------------------------------------------
//  初始化课程图
// ---------------------------------------------------------------------------
void initGraph(CourseGraph& g, int maxNum) {
    g.maxCourses = maxNum;
    g.courseCount = 0;
    g.courses = new Course[maxNum];
    std::memset(g.courses, 0, sizeof(Course) * maxNum);
}

// ---------------------------------------------------------------------------
//  销毁课程图
// ---------------------------------------------------------------------------
void destroyGraph(CourseGraph& g) {
    if (g.courses == nullptr) {
        return;
    }
    for (int i = 0; i < g.courseCount; ++i) {
        EdgeNode* p = g.courses[i].firstEdge;
        while (p != nullptr) {
            EdgeNode* next = p->next;
            delete p;
            p = next;
        }
        g.courses[i].firstEdge = nullptr;
    }
    delete[] g.courses;
    g.courses = nullptr;
    g.courseCount = 0;
    g.maxCourses = 0;
}

// ---------------------------------------------------------------------------
//  向图中添加课程顶点
// ---------------------------------------------------------------------------
int addCourseToGraph(CourseGraph& g, int id, const char* code, const char* fullName,
                     int credit, int isRequired) {
    if (g.courseCount >= g.maxCourses) {
        return -1;
    }
    Course& c = g.courses[g.courseCount];
    c.courseId = id;
    std::snprintf(c.courseName, MAX_NAME_LEN, "%s", code);
    std::snprintf(c.fullName, MAX_NAME_LEN, "%s", fullName ? fullName : code);
    c.credit = credit;
    c.isRequired = isRequired;
    c.inDegree = 0;
    c.outDegree = 0;
    c.firstEdge = nullptr;
    c.semester = 0;
    return g.courseCount++;
}

// ---------------------------------------------------------------------------
//  添加先修关系边 prereqId -> nextId
// ---------------------------------------------------------------------------
void addPrerequisite(CourseGraph& g, int prereqId, int nextId) {
    const int prereqIdx = findCourseIndex(g, prereqId);
    const int nextIdx = findCourseIndex(g, nextId);
    if (prereqIdx == -1 || nextIdx == -1) {
        return;
    }

    EdgeNode* node = new EdgeNode;
    node->adjVex = nextIdx;
    node->next = g.courses[prereqIdx].firstEdge;
    g.courses[prereqIdx].firstEdge = node;

    g.courses[prereqIdx].outDegree++;
    g.courses[nextIdx].inDegree++;

    prereqList[nextIdx].push_back(prereqIdx);
}

// ---------------------------------------------------------------------------
//  按 courseId 查找课程在数组中的下标
// ---------------------------------------------------------------------------
int findCourseIndex(CourseGraph& g, int courseId) {
    for (int i = 0; i < g.courseCount; ++i) {
        if (g.courses[i].courseId == courseId) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
//  打印图的邻接表
// ---------------------------------------------------------------------------
void printGraph(CourseGraph& g) {
    std::cout << "\n---- 课程先修关系图（邻接表）----\n";
    for (int i = 0; i < g.courseCount; ++i) {
        std::cout << g.courses[i].courseName << "(" << g.courses[i].credit
                  << "学分) -> ";
        EdgeNode* p = g.courses[i].firstEdge;
        if (p == nullptr) {
            std::cout << "无后继";
        } else {
            while (p != nullptr) {
                std::cout << g.courses[p->adjVex].courseName;
                if (p->next != nullptr) {
                    std::cout << ", ";
                }
                p = p->next;
            }
        }
        std::cout << "\n";
    }
}

// ---------------------------------------------------------------------------
//  拓扑排序（Kahn 算法）+ 环检测
// ---------------------------------------------------------------------------
bool topologicalSort(CourseGraph& g, int topoOrder[], int& topoCount) {
    std::queue<int> q;

    int* indegree = new int[g.courseCount];
    for (int i = 0; i < g.courseCount; ++i) {
        indegree[i] = g.courses[i].inDegree;
        if (indegree[i] == 0) {
            q.push(i);
        }
    }

    topoCount = 0;
    while (!q.empty()) {
        const int current = q.front();
        q.pop();
        topoOrder[topoCount++] = current;

        EdgeNode* p = g.courses[current].firstEdge;
        while (p != nullptr) {
            if (--indegree[p->adjVex] == 0) {
                q.push(p->adjVex);
            }
            p = p->next;
        }
    }

    delete[] indegree;

    if (topoCount < g.courseCount) {
        return false;
    }
    return true;
}
