#include "course_system.h"
#include "student_head.h"

#include <fstream>
#include <iostream>
#include <string>

// ============================================================================
//  主函数（统一入口：登录界面 → 选择教务端 / 学生端）
// ============================================================================

// ---------------------------------------------------------------------------
//  教务端主流程
// ---------------------------------------------------------------------------
int adminMain() {
    std::cout << "\n================ 教学计划编排系统（教务端）================\n";

    // ---------- 选择输入方式 ----------
    std::cout << "\n请选择输入方式：\n";
    std::cout << "  1. 手动输入\n";
    std::cout << "  2. 从文件输入\n";
    std::cout << "（可直接输入文件路径，将自动从文件读取）\n";

    int mode = 0;
    std::string modeInput;
    std::cout << "请选择（1 或 2），或直接输入文件名：";
    if (!(std::cin >> modeInput)) {
        std::cout << "\n输入结束，程序退出。\n";
        return 1;
    }

    std::string fileName;
    if (modeInput == "1") {
        mode = 1;
    } else if (modeInput == "2") {
        mode = 2;
        std::cout << "请输入数据文件名：";
        if (!(std::cin >> fileName)) {
            std::cout << "\n输入结束，程序退出。\n";
            return 1;
        }
    } else {
        // 用户直接输入了文件路径，当作从文件输入
        mode = 2;
        fileName = modeInput;
    }

    std::ifstream inputFile;
    std::streambuf* oldCinBuf = nullptr;

    if (mode == 2) {
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

    std::cout << "\n教务端结束，返回登录界面。\n";
    return 0;
}

// ---------------------------------------------------------------------------
//  主函数：登录界面
// ---------------------------------------------------------------------------
int main() {
    InitConsole();

    while (true) {
        std::cout << "\n";
        std::cout << "=============================================\n";
        std::cout << "       教学计划编排与选课系统\n";
        std::cout << "=============================================\n";
        std::cout << "  1. 教务端（教学计划编排）\n";
        std::cout << "  2. 学生端（选课系统）\n";
        std::cout << "  0. 退出\n";
        std::cout << "=============================================\n";

        int choice = 0;
        if (!ReadInt("请选择登录端（0 ~ 2）：", 0, 2, choice,
                     "输入错误，请输入 0、1 或 2！")) {
            break;
        }

        // 清空输入缓冲区中残留的换行符，避免影响各端的 getline 读取
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
        case 1:
            adminMain();
            break;
        case 2:
            StudentApp::studentMain();
            break;
        case 0:
            std::cout << "\n程序结束，再见！\n";
            return 0;
        default:
            break;
        }
    }

    return 0;
}
