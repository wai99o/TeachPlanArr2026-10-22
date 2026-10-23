#include "course_system.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// ============================================================================
//  文件：子功能_基础工具.cpp
//  内容：全局数据定义 + 基础工具函数
// ============================================================================

// ---------------------------------------------------------------------------
//  全局数据定义
// ---------------------------------------------------------------------------
int semesterCount = 0;
int creditLimit = 0;
int requiredCreditLimit = 0;
int electiveCreditLimit = 0;

CourseGraph courseGraph = {};
Semester semesters[MAX_SEMESTER] = {};

std::vector<int> prereqList[MAX_COURSE] = {};
std::vector<std::pair<int, int>> prereqPairs;

int topoOrder[MAX_COURSE] = {};
int topoCount = 0;

bool planReady = false;

// ---------------------------------------------------------------------------
//  控制台初始化
// ---------------------------------------------------------------------------
void InitConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

// ---------------------------------------------------------------------------
//  UTF-8 文件名 -> 文件系统路径
// ---------------------------------------------------------------------------
std::filesystem::path Utf8Path(const std::string& utf8) {
    return std::filesystem::u8path(utf8);
}

// ---------------------------------------------------------------------------
//  课程代号检查
// ---------------------------------------------------------------------------
bool IsValidCourseCode(const std::string& code) {
    if (code.size() != CODE_LEN) {
        return false;
    }
    for (char ch : code) {
        if (!std::isalnum(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
//  根据课程名称（代号）查找在课程图中的下标
// ---------------------------------------------------------------------------
int FindCourse(const std::string& name) {
    if (courseGraph.courses == nullptr) {
        return -1;
    }
    for (int i = 0; i < courseGraph.courseCount; ++i) {
        if (std::strcmp(courseGraph.courses[i].courseName, name.c_str()) == 0) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
//  把课程代号写入固定长度的字符数组
// ---------------------------------------------------------------------------
void CopyCourseId(char dst[CODE_BUF], const std::string& id) {
    std::snprintf(dst, CODE_BUF, "%s", id.c_str());
}

// ---------------------------------------------------------------------------
//  读取一个整数并检查范围
// ---------------------------------------------------------------------------
bool ReadInt(const std::string& prompt, int low, int high, int& value,
             const std::string& errorMsg) {
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> value)) {
            if (std::cin.eof()) {
                std::cout << "\n输入结束。\n";
                return false;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入格式错误，请重新输入！\n";
            continue;
        }
        if (value < low || value > high) {
            std::cout << errorMsg << "\n";
            continue;
        }
        return true;
    }
}
