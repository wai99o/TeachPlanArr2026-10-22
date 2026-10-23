#include "student_head.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

namespace StudentApp {

static void SkipCommentAndBlank(std::ifstream& file, std::string& line) {
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first != std::string::npos) {
                size_t last = line.find_last_not_of(" \t\r\n");
                line = line.substr(first, last - first + 1);
                return;
            }
        }
    }
}

bool loadTeachingPlan(const std::string& filename) {
    std::ifstream file(Utf8Path(filename));
    if (!file) {
        std::cerr << "错误：无法打开课表文件 " << filename << "\n";
        return false;
    }

    std::cout << "\n---- 加载课表文件：" << filename << " ----\n";

    std::string line;
    SkipCommentAndBlank(file, line);

    int loadedSemCount = 0, loadedCreditLimit = 0, loadedReqLimit = 0, loadedElecLimit = 0, loadedCourseCount = 0;
    if (sscanf(line.c_str(), "%d %d %d %d %d", &loadedSemCount, &loadedCreditLimit, &loadedReqLimit, &loadedElecLimit, &loadedCourseCount) != 5) {
        std::cerr << "错误：第1行格式应为 '学期总数 总学分上限 必修上限 选修上限 课程数量'\n";
        return false;
    }
    if (loadedSemCount < 1 || loadedSemCount > MAX_SEMESTER) {
        std::cerr << "错误：学期总数超出范围 1~" << MAX_SEMESTER << "\n";
        return false;
    }
    if (loadedCourseCount < 1 || loadedCourseCount > MAX_COURSE) {
        std::cerr << "错误：课程数量超出范围 1~" << MAX_COURSE << "\n";
        return false;
    }
    if (loadedCreditLimit < 1) {
        std::cerr << "错误：总学分上限必须 >= 1\n";
        return false;
    }
    if (loadedReqLimit < 1 || loadedElecLimit < 1) {
        std::cerr << "错误：必修/选修学分上限必须 >= 1\n";
        return false;
    }

    semesterCount = loadedSemCount;
    creditLimit = loadedCreditLimit;
    requiredCreditLimit = loadedReqLimit;
    electiveCreditLimit = loadedElecLimit;
    courseCount = loadedCourseCount;

    for (int i = 0; i < courseCount; ++i) {
        SkipCommentAndBlank(file, line);
        char code[CODE_BUF] = {};
        char name[MAX_NAME_LEN] = {};
        int credit = 0, sem = 0, isReq = 1;
        if (sscanf(line.c_str(), "%3s %49s %d %d %d", code, name, &credit, &sem, &isReq) != 5) {
            std::cerr << "错误：第 " << (i + 2)
                      << " 行课程信息格式应为 '课程代号 课程名称 学分 学期 是否必修'\n";
            return false;
        }
        if (!IsValidCourseCode(code)) {
            std::cerr << "错误：课程编号 " << code << " 格式不合法\n";
            return false;
        }
        if (credit < 1) {
            std::cerr << "错误：课程 " << code << " 学分必须 > 0\n";
            return false;
        }
        if (sem < 1 || sem > semesterCount) {
            std::cerr << "错误：课程 " << code << " 的学期 " << sem << " 超出范围\n";
            return false;
        }
        CopyCourseId(courses[i].id, code);
        std::snprintf(courses[i].name, MAX_NAME_LEN, "%s", name);
        courses[i].credit = credit;
        courses[i].semester = sem;
        courses[i].isRequired = (isReq != 0) ? 1 : 0;
        courses[i].indegree = 0;
        courses[i].next.clear();
    }

    for (int s = 0; s < semesterCount; ++s) {
        semesters[s].courses.clear();
        semesters[s].totalCredit = 0;
    }
    for (int i = 0; i < courseCount; ++i) {
        const int sem = courses[i].semester - 1;
        semesters[sem].courses.push_back(i);
        semesters[sem].totalCredit += courses[i].credit;
    }

    SkipCommentAndBlank(file, line);
    int prereqCount = 0;
    if (sscanf(line.c_str(), "%d", &prereqCount) != 1) {
        std::cerr << "错误：先修关系条数缺失\n";
        return false;
    }
    prereqPairs.clear();

    for (int i = 0; i < prereqCount; ++i) {
        SkipCommentAndBlank(file, line);
        char from[CODE_BUF] = {}, to[CODE_BUF] = {};
        if (sscanf(line.c_str(), "%3s %3s", from, to) != 2) {
            std::cerr << "错误：第 " << (i + 1) << " 条先修关系格式应为 '先修课 后继课'\n";
            return false;
        }
        const int prereq = FindCourse(from);
        const int succ = FindCourse(to);
        if (prereq == -1 || succ == -1) {
            std::cerr << "错误：先修关系中的课程不存在：" << from << " -> " << to << "\n";
            return false;
        }
        bool dup = false;
        for (const auto& e : prereqPairs) {
            if (e.first == prereq && e.second == succ) { dup = true; break; }
        }
        if (!dup && prereq != succ) {
            prereqPairs.emplace_back(prereq, succ);
        }
    }

    file.close();

    buildGraph();
    calculateIndegree();

    std::cout << "课表加载成功！\n";
    std::cout << "  学期总数：" << semesterCount
              << "，总学分上限：" << creditLimit
              << "，必修上限：" << requiredCreditLimit
              << "，选修上限：" << electiveCreditLimit
              << "，课程数量：" << courseCount
              << "，先修关系：" << prereqPairs.size() << " 条\n";

    std::cout << "\n当前专业课表：\n";
    for (int s = 0; s < semesterCount; ++s) {
        std::cout << "  第" << (s + 1) << "学期：";
        for (std::size_t k = 0; k < semesters[s].courses.size(); ++k) {
            const int idx = semesters[s].courses[k];
            if (k > 0) std::cout << " ";
            std::cout << courses[idx].id << "-" << courses[idx].name
                      << "(" << courses[idx].credit
                      << (courses[idx].isRequired ? ",必" : ",选") << ")";
        }
        std::cout << "  [学分合计 " << semesters[s].totalCredit << "]\n";
    }

    planReady = true;
    return true;
}

} // namespace StudentApp
