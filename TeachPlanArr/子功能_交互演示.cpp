#include "course_system.h"

#include <iostream>

// ============================================================================
//  文件：子功能_交互演示.cpp
//  内容：系统菜单
// ============================================================================

void menu() {
    while (true) {
        std::cout << "\n========== 教 学 计 划 编 排 ==========\n";
        std::cout << "  1. 均衡编排（各学期学习负担尽量均匀）\n";
        std::cout << "  2. 集中编排（课程尽量集中在前几个学期）\n";
        std::cout << "  3. 显示教学计划\n";
        std::cout << "  4. 保存教学计划到文件（可读表格）\n";
        std::cout << "  5. 导出教学计划数据（供学生端加载）\n";
        std::cout << "  0. 退出\n";
        std::cout << "=======================================\n";

        int choice = -1;
        if (!ReadInt("请选择操作（0 ~ 5）：", 0, 5, choice,
                     "选择错误，请输入 0 ~ 5 之间的数字！")) {
            std::cout << "输入结束，程序退出。\n";
            return;
        }

        switch (choice) {
        case 1:
        case 2: {
            const bool scheduled =
                (choice == 1) ? balancedSchedule() : concentratedSchedule();

            if (!scheduled) {
                planReady = false;
                std::cout << "未生成教学计划。\n";
                break;
            }

            if (checkSchedule()) {
                planReady = true;
                displayPlan();
            } else {
                planReady = false;
                std::cout << "编排结果未通过合法性检查，未生成教学计划。\n";
            }
            break;
        }
        case 3:
            displayPlan();
            break;
        case 4:
            savePlan();
            break;
        case 5:
            savePlanData();
            break;
        case 0:
            return;
        default:
            break;
        }
    }
}
