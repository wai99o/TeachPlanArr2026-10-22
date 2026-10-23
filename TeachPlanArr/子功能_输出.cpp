#include "course_system.h"

#include <fstream>
#include <iostream>
#include <string>

// ============================================================================
//  文件：子功能_输出.cpp
//  内容：输出模块
// ============================================================================

// ---------------------------------------------------------------------------
//  生成教学计划文本（表格形式）
//
//  注意：UTF-8 下每个中文字符占 3 字节，列宽按字节数计算。
// ---------------------------------------------------------------------------
static std::string BuildPlanText() {
    // ---- 先计算各列的字节宽度 ----
    int semColWidth = 0;
    for (int s = 0; s < semesterCount; ++s) {
        std::string semStr = "第" + std::to_string(s + 1) + "学期";
        if (static_cast<int>(semStr.size()) > semColWidth) {
            semColWidth = static_cast<int>(semStr.size());
        }
    }
    const std::string semHeader = "学期";
    if (static_cast<int>(semHeader.size()) > semColWidth) {
        semColWidth = static_cast<int>(semHeader.size());
    }

    const std::string creditHeader = "学分";
    int creditColWidth = static_cast<int>(creditHeader.size());
    for (int s = 0; s < semesterCount; ++s) {
        int len = static_cast<int>(std::to_string(semesters[s].totalCredit).size());
        if (len > creditColWidth) creditColWidth = len;
    }

    const std::string courseHeader = "课程安排";
    int courseColWidth = static_cast<int>(courseHeader.size());
    const std::string emptyStr = "（无课程）";
    if (static_cast<int>(emptyStr.size()) > courseColWidth) {
        courseColWidth = static_cast<int>(emptyStr.size());
    }
    for (int s = 0; s < semesterCount; ++s) {
        std::string line;
        for (std::size_t i = 0; i < semesters[s].courses.size(); ++i) {
            const Course& c = courseGraph.courses[semesters[s].courses[i]];
            if (i > 0) line += " ";
            line += std::string(c.courseName) + "-" + std::string(c.fullName) + "(" +
                    std::to_string(c.credit) +
                    (c.isRequired ? ",必" : ",选") + ")";
        }
        if (line.empty()) line = emptyStr;
        if (static_cast<int>(line.size()) > courseColWidth) {
            courseColWidth = static_cast<int>(line.size());
        }
    }

    std::string text;
    text += "\n";
    text += "================ 教学计划 ================\n";

    auto makeBorder = [&](char left, char mid, char right, char fill) {
        std::string b;
        b += left;
        b += std::string(semColWidth, fill);
        b += mid;
        b += std::string(courseColWidth, fill);
        b += mid;
        b += std::string(creditColWidth, fill);
        b += right;
        return b;
    };

    text += makeBorder('+', '+', '+', '-') + "\n";
    text += "|" + semHeader +
            std::string(semColWidth - semHeader.size(), ' ') + "|" +
            courseHeader +
            std::string(courseColWidth - courseHeader.size(), ' ') + "|" +
            creditHeader +
            std::string(creditColWidth - creditHeader.size(), ' ') + "|\n";
    text += makeBorder('+', '+', '+', '-') + "\n";

    for (int s = 0; s < semesterCount; ++s) {
        std::string semStr = "第" + std::to_string(s + 1) + "学期";
        text += "|" + semStr +
                std::string(semColWidth - semStr.size(), ' ') + "|";

        std::string courseStr;
        if (semesters[s].courses.empty()) {
            courseStr = emptyStr;
        } else {
            for (std::size_t i = 0; i < semesters[s].courses.size(); ++i) {
                const Course& c = courseGraph.courses[semesters[s].courses[i]];
                if (i > 0) courseStr += " ";
                courseStr += std::string(c.courseName) + "-" + std::string(c.fullName) + "(" +
                             std::to_string(c.credit) +
                             (c.isRequired ? ",必" : ",选") + ")";
            }
        }
        text += courseStr +
                std::string(courseColWidth - courseStr.size(), ' ') + "|";

        std::string creditStr = std::to_string(semesters[s].totalCredit);
        text += creditStr +
                std::string(creditColWidth - creditStr.size(), ' ') + "|\n";

        if (s + 1 < semesterCount) {
            text += makeBorder('+', '+', '+', '-') + "\n";
        }
    }
    text += makeBorder('+', '+', '+', '-') + "\n";

    return text;
}

// ---------------------------------------------------------------------------
//  在屏幕上显示教学计划
// ---------------------------------------------------------------------------
void displayPlan() {
    if (!planReady) {
        std::cout << "当前没有教学计划，请先执行编排！\n";
        return;
    }

    std::cout << BuildPlanText();
}

// ---------------------------------------------------------------------------
//  保存教学计划到文件
// ---------------------------------------------------------------------------
bool savePlan() {
    if (!planReady) {
        std::cout << "当前没有可保存的教学计划，请先执行编排！\n";
        return false;
    }

    std::cout << "请输入输出文件名：";
    std::string fileName;
    if (!(std::cin >> fileName)) {
        std::cout << "\n输入结束，取消保存。\n";
        return false;
    }

    std::ofstream file(Utf8Path(fileName), std::ios::binary);
    if (!file) {
        std::cout << "无法打开输出文件：" << fileName << "\n";
        return false;
    }

    const std::string text = BuildPlanText();
    const char bom[3] = {'\xEF', '\xBB', '\xBF'};

    file.write(bom, 3);
    file.write(text.data(), static_cast<std::streamsize>(text.size()));

    if (!file) {
        std::cout << "写入文件失败：" << fileName << "\n";
        return false;
    }
    file.close();

    std::cout << "系统将教学计划保存到：" << fileName << "\n";
    return true;
}

// ---------------------------------------------------------------------------
//  导出结构化数据文件（供学生端加载）
//
//  文件格式：
//    第 1 行：S L R E N    学期总数、总学分上限、必修上限、选修上限、课程数量
//    第 2 ~ N+1 行：课程代号 课程名称 学分 所安排学期 是否必修（1=必修 0=选修）
//    第 N+2 行：先修关系条数 M
//    后续 M 行：先修课编号 后继课编号
// ---------------------------------------------------------------------------
bool savePlanData() {
    if (!planReady) {
        std::cout << "当前没有可导出的教学计划，请先执行编排！\n";
        return false;
    }

    std::cout << "请输入导出数据文件名（例如 plan_data.txt）：";
    std::string fileName;
    if (!(std::cin >> fileName)) {
        std::cout << "\n输入结束，取消导出。\n";
        return false;
    }

    std::ofstream file(Utf8Path(fileName), std::ios::binary);
    if (!file) {
        std::cout << "无法打开输出文件：" << fileName << "\n";
        return false;
    }

    file << semesterCount << " " << creditLimit << " "
         << requiredCreditLimit << " " << electiveCreditLimit << " "
         << courseGraph.courseCount << "\n";

    for (int i = 0; i < courseGraph.courseCount; ++i) {
        const Course& c = courseGraph.courses[i];
        file << c.courseName << " " << c.fullName << " " << c.credit << " "
             << c.semester << " " << c.isRequired << "\n";
    }

    // 导出先修关系
    file << prereqPairs.size() << "\n";
    for (const auto& p : prereqPairs) {
        file << courseGraph.courses[p.first].courseName << " "
             << courseGraph.courses[p.second].courseName << "\n";
    }

    if (!file) {
        std::cout << "写入文件失败：" << fileName << "\n";
        return false;
    }
    file.close();

    std::cout << "教学计划数据已导出到：" << fileName << "\n";
    std::cout << "（学生端可读取此文件获取必修课安排）\n";
    return true;
}
