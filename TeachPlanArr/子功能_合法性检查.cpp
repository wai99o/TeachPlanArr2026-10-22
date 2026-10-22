#include "head.h"

#include <cstring>
#include <iostream>
#include <string>

// ============================================================================
//  文件：子功能_合法性检查.cpp
//  内容：数据合法性检查
// ============================================================================

bool checkData() {
    std::cout << "\n---- 4. 数据合法性检查 ----\n";

    bool ok = true;

    if (semesterCount < 1 || semesterCount > MAX_SEMESTER) {
        std::cout << "学期数量输入错误！\n";
        ok = false;
    }

    if (courseGraph.courseCount < 1 || courseGraph.courseCount > MAX_COURSE) {
        std::cout << "课程数量输入错误！\n";
        ok = false;
    }

    if (creditLimit < 1) {
        std::cout << "学分上限输入错误！\n";
        ok = false;
    }

    if (!ok) {
        return false;
    }

    for (int i = 0; i < courseGraph.courseCount; ++i) {
        const std::string name(courseGraph.courses[i].courseName);

        if (!IsValidCourseCode(name)) {
            std::cout << "第 " << (i + 1) << " 门课程的代号格式错误！\n";
            ok = false;
        }

        for (int j = 0; j < i; ++j) {
            if (name == courseGraph.courses[j].courseName) {
                std::cout << "课程代号重复：" << name << "！\n";
                ok = false;
            }
        }

        if (courseGraph.courses[i].credit < 1) {
            std::cout << "错误：课程 " << name << " 的学分必须大于 0！\n";
            ok = false;
        } else if (courseGraph.courses[i].credit > creditLimit) {
            std::cout << "错误：课程 " << name << " 的学分超过每学期学分上限！\n";
            ok = false;
        }
    }

    for (const auto& edge : prereqPairs) {
        if (edge.first < 0 || edge.first >= courseGraph.courseCount ||
            edge.second < 0 || edge.second >= courseGraph.courseCount) {
            std::cout << "错误：先修关系中的课程不存在！\n";
            ok = false;
        } else if (edge.first == edge.second) {
            std::cout << "错误：先修课程不能是自身！\n";
            ok = false;
        }
    }

    if (ok) {
        std::cout << "数据合法性检查通过！\n";
    } else {
        std::cout << "数据合法性检查未通过！\n";
    }

    return ok;
}
