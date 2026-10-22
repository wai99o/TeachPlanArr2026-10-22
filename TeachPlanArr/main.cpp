#include "head.h"

#include <fstream>
#include <iostream>
#include <string>

// ============================================================================
//  主函数（教务端主流程）
//
//  流程：
//    选择输入方式（手动 / 文件）
//      → 输入基本信息 → 输入课程信息 → 输入先修关系 → 数据合法性检查
//      → 拓扑排序（含环检测）
//      → 弹出菜单选择编排策略 → 结果检查 → 显示/保存 → 结束
// ============================================================================
int main() {
    InitConsole();

    std::cout << "================ 教学计划编排系统（教务端）================\n";

    // ---------- 选择输入方式 ----------
    std::cout << "\n请选择输入方式：\n";
    std::cout << "  1. 手动输入\n";
    std::cout << "  2. 从文件输入\n";

    int mode = 0;
    if (!ReadInt("请选择（1 或 2）：", 1, 2, mode, "选择错误，请输入 1 或 2！")) {
        return 1;
    }

    std::ifstream inputFile;
    std::streambuf* oldCinBuf = nullptr;

    if (mode == 2) {
        std::cout << "请输入数据文件名：";
        std::string fileName;
        if (!(std::cin >> fileName)) {
            std::cout << "\n输入结束，程序退出。\n";
            return 1;
        }

        inputFile.open(Utf8Path(fileName));
        if (!inputFile) {
            std::cout << "无法打开输入文件：" << fileName << "\n";
            return 1;
        }

        oldCinBuf = std::cin.rdbuf(inputFile.rdbuf());
        std::cout << "已从文件 " << fileName << " 读取数据：\n";
    }

    // ---------- 数据输入 ----------
    if (!inputBasicInfo()) {
        if (oldCinBuf) std::cin.rdbuf(oldCinBuf);
        return 1;
    }

    if (!inputCourses()) {
        if (oldCinBuf) std::cin.rdbuf(oldCinBuf);
        return 1;
    }

    if (!inputPrerequisite()) {
        if (oldCinBuf) std::cin.rdbuf(oldCinBuf);
        return 1;
    }

    if (oldCinBuf) {
        std::cin.rdbuf(oldCinBuf);
        inputFile.close();
    }

    // ---------- 数据合法性检查 ----------
    if (!checkData()) {
        std::cout << "\n数据不合法，程序结束。\n";
        return 1;
    }

    // ---------- 拓扑排序与环检测 ----------
    if (!topologicalSort(courseGraph, topoOrder, topoCount)) {
        std::cout << "\n先修关系存在环，无法进行教学计划编排，程序结束。\n";
        return 1;
    }

    std::cout << "\n拓扑排序成功，课程先后顺序：\n";
    for (int i = 0; i < topoCount; ++i) {
        std::cout << courseGraph.courses[topoOrder[i]].courseName;
        if (i + 1 < topoCount) {
            std::cout << " -> ";
        }
    }
    std::cout << "\n";

    // ---------- 进入菜单，选择编排策略 ----------
    menu();

    // ---------- 释放课程图内存 ----------
    destroyGraph(courseGraph);

    std::cout << "\n程序结束。\n";
    return 0;
}
