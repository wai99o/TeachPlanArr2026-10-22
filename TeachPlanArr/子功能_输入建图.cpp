#include "course_system.h"

#include <cstdio>
#include <iostream>
#include <limits>
#include <string>

// ============================================================================
//  文件：子功能_输入建图.cpp
//  内容：数据输入模块
// ============================================================================

// ---------------------------------------------------------------------------
//  1. 输入基本信息
// ---------------------------------------------------------------------------
bool inputBasicInfo() {
    std::cout << "\n---- 1. 输入基本信息 ----\n";

    if (!ReadInt("请输入学期总数（1 ~ 10）：", 1, MAX_SEMESTER, semesterCount,
                 "学期数量输入错误！")) {
        return false;
    }

    if (!ReadInt("请输入每学期总学分上限（1 ~ 100）：", 1, 100, creditLimit,
                 "学分上限输入错误！")) {
        return false;
    }

    if (!ReadInt("请输入每学期必修课学分上限（1 ~ 100）：", 1, 100, requiredCreditLimit,
                 "必修课学分上限输入错误！")) {
        return false;
    }

    if (!ReadInt("请输入每学期选修课学分上限（1 ~ 100）：", 1, 100, electiveCreditLimit,
                 "选修课学分上限输入错误！")) {
        return false;
    }

    int n = 0;
    if (!ReadInt("请输入课程数量（1 ~ 60）：", 1, MAX_COURSE, n,
                 "课程数量输入错误！")) {
        return false;
    }

    initGraph(courseGraph, n);

    return true;
}

// ---------------------------------------------------------------------------
//  2. 输入课程信息
// ---------------------------------------------------------------------------
bool inputCourses() {
    std::cout << "\n---- 2. 输入课程信息 ----\n";

    const int n = courseGraph.maxCourses;

    for (int i = 0; i < n; ++i) {
        while (true) {
            std::string code;
            std::string fullName;
            int credit = 0;
            int isRequired = 1;

            std::cout << "请输入第 " << (i + 1)
                      << " 门课程的代号、名称、学分和是否必修（例如 C01 高等数学 2 1，1=必修 0=选修）：";
            if (!(std::cin >> code)) {
                std::cout << "\n输入结束。\n";
                return false;
            }

            if (!IsValidCourseCode(code)) {
                std::cout << "课程代号格式错误（应为 3 位字母或数字）！\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (FindCourse(code) != -1) {
                std::cout << "课程代号重复！\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            if (!(std::cin >> fullName)) {
                if (std::cin.eof()) {
                    std::cout << "\n输入结束。\n";
                    return false;
                }
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "课程名称输入错误，请重新输入！\n";
                continue;
            }

            if (!(std::cin >> credit)) {
                if (std::cin.eof()) {
                    std::cout << "\n输入结束。\n";
                    return false;
                }
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "学分输入格式错误，请重新输入！\n";
                continue;
            }

            if (credit < 1) {
                std::cout << "错误：课程学分必须大于 0！\n";
                continue;
            }
            if (credit > creditLimit) {
                std::cout << "错误：课程 " << code << " 的学分超过每学期总学分上限！\n";
                continue;
            }
            if (isRequired == 1 && credit > requiredCreditLimit) {
                std::cout << "错误：必修课 " << code << " 的学分超过每学期必修课学分上限！\n";
                continue;
            }
            if (isRequired == 0 && credit > electiveCreditLimit) {
                std::cout << "错误：选修课 " << code << " 的学分超过每学期选修课学分上限！\n";
                continue;
            }

            if (!(std::cin >> isRequired)) {
                if (std::cin.eof()) {
                    std::cout << "\n输入结束。\n";
                    return false;
                }
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "是否必修输入格式错误，请重新输入！\n";
                continue;
            }
            if (isRequired != 0 && isRequired != 1) {
                std::cout << "错误：是否必修只能为 1（必修）或 0（选修）！\n";
                continue;
            }

            addCourseToGraph(courseGraph, i + 1, code.c_str(), fullName.c_str(), credit, isRequired);
            break;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
//  3. 输入先修关系
// ---------------------------------------------------------------------------
bool inputPrerequisite() {
    std::cout << "\n---- 3. 输入先修关系 ----\n";

    int relationCount = 0;
    if (!ReadInt("请输入先修关系条数（0 ~ 3600）：", 0, MAX_COURSE * MAX_COURSE,
                 relationCount, "先修关系条数输入错误！")) {
        return false;
    }

    prereqPairs.clear();

    for (int i = 0; i < relationCount; ++i) {
        while (true) {
            std::string from;
            std::string to;

            std::cout << "请输入第 " << (i + 1)
                      << " 条先修关系（先修课程 后继课程，例如 C01 C03）：";
            if (!(std::cin >> from >> to)) {
                std::cout << "\n输入结束。\n";
                return false;
            }

            if (!IsValidCourseCode(from) || !IsValidCourseCode(to)) {
                std::cout << "课程代号格式错误（应为 3 位字母或数字）！\n";
                continue;
            }

            const int prereq = FindCourse(from);
            const int successor = FindCourse(to);

            if (prereq == -1) {
                std::cout << "错误：先修课程" << from << "不存在！\n";
                continue;
            }
            if (successor == -1) {
                std::cout << "错误：课程" << to << "不存在！\n";
                continue;
            }
            if (prereq == successor) {
                std::cout << "错误：先修课程不能是自身！\n";
                continue;
            }

            bool duplicated = false;
            for (const auto& edge : prereqPairs) {
                if (edge.first == prereq && edge.second == successor) {
                    duplicated = true;
                    break;
                }
            }
            if (duplicated) {
                std::cout << "该先修关系已存在，请重新输入！\n";
                continue;
            }

            prereqPairs.emplace_back(prereq, successor);
            addPrerequisite(courseGraph,
                            courseGraph.courses[prereq].courseId,
                            courseGraph.courses[successor].courseId);
            break;
        }
    }

    return true;
}
