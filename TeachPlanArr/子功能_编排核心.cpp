#include "course_system.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// ============================================================================
//  文件：子功能_编排核心.cpp
//  内容：教学计划编排
// ============================================================================

// ---------------------------------------------------------------------------
//  清空教学计划
// ---------------------------------------------------------------------------
static void clearPlan() {
    for (int s = 0; s < MAX_SEMESTER; ++s) {
        semesters[s].courses.clear();
        semesters[s].totalCredit = 0;
    }
    for (int i = 0; i < courseGraph.courseCount; ++i) {
        courseGraph.courses[i].semester = 0;
    }
}

// ---------------------------------------------------------------------------
//  计算课程 v 最早可以安排的学期
// ---------------------------------------------------------------------------
static int computeEarliest(int v) {
    int earliest = 1;
    for (int prereq : prereqList[v]) {
        if (courseGraph.courses[prereq].semester + 1 > earliest) {
            earliest = courseGraph.courses[prereq].semester + 1;
        }
    }
    return earliest;
}

// ---------------------------------------------------------------------------
//  确定排课处理顺序
// ---------------------------------------------------------------------------
static std::vector<int> buildOrder() {
    const int n = courseGraph.courseCount;
    std::vector<int> level(n, 0);

    for (int i = 0; i < topoCount; ++i) {
        const int current = topoOrder[i];
        EdgeNode* p = courseGraph.courses[current].firstEdge;
        while (p != nullptr) {
            if (level[p->adjVex] < level[current] + 1) {
                level[p->adjVex] = level[current] + 1;
            }
            p = p->next;
        }
    }

    std::vector<int> order(n);
    for (int i = 0; i < n; ++i) {
        order[i] = i;
    }

    std::sort(order.begin(), order.end(), [&level](int a, int b) {
        if (level[a] != level[b]) {
            return level[a] < level[b];
        }
        if (courseGraph.courses[a].credit != courseGraph.courses[b].credit) {
            return courseGraph.courses[a].credit > courseGraph.courses[b].credit;
        }
        return std::string(courseGraph.courses[a].courseName) <
               std::string(courseGraph.courses[b].courseName);
    });

    return order;
}

// ---------------------------------------------------------------------------
//  按单学期目标学分上限尝试排课
// ---------------------------------------------------------------------------
static bool tryScheduleWithLimit(int targetLimit) {
    clearPlan();

    const int n = courseGraph.courseCount;
    if (n <= 0) {
        return false;
    }

    const std::vector<int> order = buildOrder();

    for (int v : order) {
        int semester = computeEarliest(v);

        while (semester <= semesterCount) {
            if (semesters[semester - 1].totalCredit +
                    courseGraph.courses[v].credit <= targetLimit) {
                courseGraph.courses[v].semester = semester;
                semesters[semester - 1].courses.push_back(v);
                semesters[semester - 1].totalCredit += courseGraph.courses[v].credit;
                break;
            }
            ++semester;
        }

        if (semester > semesterCount) {
            clearPlan();
            return false;
        }
    }

    for (int s = 0; s < semesterCount; ++s) {
        std::sort(semesters[s].courses.begin(), semesters[s].courses.end(),
                  [](int a, int b) {
                      return std::string(courseGraph.courses[a].courseName) <
                             std::string(courseGraph.courses[b].courseName);
                  });
    }

    return true;
}

// ---------------------------------------------------------------------------
//  均衡编排
// ---------------------------------------------------------------------------
bool balancedSchedule() {
    std::cout << "\n---- 均衡编排 ----\n";

    int totalCredit = 0;
    int maxSingleCredit = 0;
    for (int i = 0; i < courseGraph.courseCount; ++i) {
        totalCredit += courseGraph.courses[i].credit;
        if (courseGraph.courses[i].credit > maxSingleCredit) {
            maxSingleCredit = courseGraph.courses[i].credit;
        }
    }

    const int average = (totalCredit + semesterCount - 1) / semesterCount;
    int lowerBound = std::max(maxSingleCredit, average);
    if (lowerBound < 1) {
        lowerBound = 1;
    }

    for (int target = lowerBound; target <= creditLimit; ++target) {
        if (tryScheduleWithLimit(target)) {
            std::cout << "均衡编排完成，单学期目标学分上限：" << target << "\n";
            return true;
        }
    }

    std::cout << "无法在给定的学期总数和学分上限下安排全部课程，均衡编排失败！\n";
    return false;
}

// ---------------------------------------------------------------------------
//  集中编排
// ---------------------------------------------------------------------------
bool concentratedSchedule() {
    std::cout << "\n---- 集中编排 ----\n";

    if (tryScheduleWithLimit(creditLimit)) {
        std::cout << "集中编排完成。\n";
        return true;
    }

    std::cout << "无法在给定的学期总数和学分上限下安排全部课程，集中编排失败！\n";
    return false;
}

// ---------------------------------------------------------------------------
//  编排结果检查
// ---------------------------------------------------------------------------
bool checkSchedule() {
    std::cout << "\n---- 6. 编排结果检查 ----\n";

    bool ok = true;
    int totalCredit = 0;

    for (int s = 0; s < semesterCount; ++s) {
        if (semesters[s].totalCredit > creditLimit) {
            std::cout << "错误：第 " << (s + 1) << " 学期学分 " << semesters[s].totalCredit
                      << " 超过每学期学分上限 " << creditLimit << "！\n";
            ok = false;
        }
    }

    for (int u = 0; u < courseGraph.courseCount; ++u) {
        if (courseGraph.courses[u].semester == 0) {
            std::cout << "错误：课程 " << courseGraph.courses[u].courseName
                      << " 尚未安排！\n";
            ok = false;
            continue;
        }
        totalCredit += courseGraph.courses[u].credit;

        EdgeNode* p = courseGraph.courses[u].firstEdge;
        while (p != nullptr) {
            const int v = p->adjVex;
            if (courseGraph.courses[v].semester == 0) {
                p = p->next;
                continue;
            }
            if (courseGraph.courses[u].semester >= courseGraph.courses[v].semester) {
                std::cout << "错误：课程 " << courseGraph.courses[u].courseName
                          << "（第 " << courseGraph.courses[u].semester
                          << " 学期）没有安排在课程 " << courseGraph.courses[v].courseName
                          << "（第 " << courseGraph.courses[v].semester
                          << " 学期）之前！\n";
                ok = false;
            }
            p = p->next;
        }
    }

    int placed[MAX_COURSE] = {};
    for (int s = 0; s < semesterCount; ++s) {
        for (int v : semesters[s].courses) {
            if (v < 0 || v >= courseGraph.courseCount) {
                std::cout << "错误：教学计划中存在无效的课程编号！\n";
                ok = false;
                continue;
            }
            ++placed[v];
        }
    }
    for (int i = 0; i < courseGraph.courseCount; ++i) {
        if (placed[i] != 1) {
            std::cout << "错误：课程 " << courseGraph.courses[i].courseName
                      << " 在计划中出现 " << placed[i] << " 次，应恰好出现 1 次！\n";
            ok = false;
        }
    }

    int plannedCredit = 0;
    for (int s = 0; s < semesterCount; ++s) {
        plannedCredit += semesters[s].totalCredit;
    }
    if (plannedCredit != totalCredit) {
        std::cout << "错误：计划中学分合计 " << plannedCredit << " 与课程总学分 "
                  << totalCredit << " 不一致！\n";
        ok = false;
    }

    if (ok) {
        std::cout << "教学计划检查通过！\n";
    } else {
        std::cout << "教学计划检查未通过！\n";
    }

    return ok;
}
