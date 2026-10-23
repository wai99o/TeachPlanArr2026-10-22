#include "student_head.h"

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

namespace StudentApp {

int semesterCount = 0;
int creditLimit = 0;
int courseCount = 0;

Course courses[MAX_COURSE] = {};
Semester semesters[MAX_SEMESTER] = {};

int indegreeOf[MAX_COURSE] = {};
std::vector<int> prereqList[MAX_COURSE] = {};
std::vector<std::pair<int, int>> prereqPairs;

int topoOrder[MAX_COURSE] = {};
int topoCount = 0;

bool planReady = false;

void InitConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

std::filesystem::path Utf8Path(const std::string& utf8) {
    return std::filesystem::u8path(utf8);
}

bool IsValidCourseCode(const std::string& code) {
    if (code.size() != CODE_LEN) return false;
    for (char ch : code) {
        if (!std::isalnum(static_cast<unsigned char>(ch))) return false;
    }
    return true;
}

int FindCourse(const std::string& id) {
    for (int i = 0; i < courseCount; ++i) {
        if (std::strcmp(courses[i].id, id.c_str()) == 0) return i;
    }
    return -1;
}

void CopyCourseId(char dst[CODE_BUF], const std::string& id) {
    std::snprintf(dst, CODE_BUF, "%s", id.c_str());
}

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

} // namespace StudentApp
