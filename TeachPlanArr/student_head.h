#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

// ============================================================================
//  学生端专用头文件（独立命名空间 StudentApp，避免与教务端符号冲突）
// ============================================================================

namespace StudentApp {

constexpr int MAX_COURSE = 60;
constexpr int MAX_SEMESTER = 10;
constexpr int CODE_LEN = 3;
constexpr int CODE_BUF = CODE_LEN + 1;

struct Course {
    char id[CODE_BUF];
    int credit;
    int indegree;
    std::vector<int> next;
    int semester;
};

struct Semester {
    std::vector<int> courses;
    int totalCredit;
};

struct Student {
    char id[CODE_BUF];
    int currentSemester;
    int personalLimit;
    std::vector<bool> passed;
    int selected[MAX_SEMESTER][MAX_COURSE];
    int semesterCredit[MAX_SEMESTER];

    Student();
};

enum class SelectionStyle { PreferencePrereq, Balanced, Concentrated, Mixed };

struct SelectionPlan {
    std::vector<int> courseIdxs;
    int totalCredit;
    std::string styleTag;
};

extern int semesterCount;
extern int creditLimit;
extern int courseCount;
extern Course courses[MAX_COURSE];
extern Semester semesters[MAX_SEMESTER];
extern int indegreeOf[MAX_COURSE];
extern std::vector<int> prereqList[MAX_COURSE];
extern std::vector<std::pair<int, int>> prereqPairs;
extern int topoOrder[MAX_COURSE];
extern int topoCount;
extern bool planReady;

void InitConsole();
std::filesystem::path Utf8Path(const std::string& utf8);
bool IsValidCourseCode(const std::string& code);
int FindCourse(const std::string& id);
void CopyCourseId(char dst[CODE_BUF], const std::string& id);
bool ReadInt(const std::string& prompt, int low, int high, int& value,
             const std::string& errorMsg);

void buildGraph();
void calculateIndegree();

bool loadTeachingPlan(const std::string& filename);

bool initStudent(Student& stu);
bool inputPassedCourses(Student& stu);
bool validateSelection(const Student& stu, int courseIdx, int targetSemester, std::string& reason);
bool manualSelection(Student& stu, int targetSemester);
bool autoSelect(Student& stu, int targetSemester, SelectionStyle style);
bool generateMultiplePlans(Student& stu, int targetSemester, int count, std::vector<SelectionPlan>& plans);
void printProgressTopo(const Student& stu);
void printStudentSchedule(const Student& stu);
bool saveStudentPlan(const Student& stu);
void studentMenu();

// 学生端入口（原 main 改名，供登录界面调用）
int studentMain();

} // namespace StudentApp
