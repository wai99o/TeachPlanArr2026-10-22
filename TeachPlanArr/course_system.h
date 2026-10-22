#ifndef COURSE_SYSTEM_H
#define COURSE_SYSTEM_H

// ==========================================
//          常量定义
// ==========================================
#define MAX_REQUIRED    50      // 必修课最大数量
#define MAX_ELECTIVE    50      // 选修课最大数量
#define MAX_NAME_LEN    50      // 课程名称最大长度
#define MAX_ID_LEN      20      // 课程/学号最大长度
#define MAX_COURSES    100      // 图中课程最大数
#define MAX_SELECTED    30      // 学生最多可选课数
#define MAX_STUDENTS   200      // 最多学生数

// ==========================================
//    一、二维数组（必修课 + 选修课）
//    第二维：[0]课程名 [1]课程ID [2]学分 [3]是否已修(1/0)
// ==========================================
extern char requiredCourses[MAX_REQUIRED][4][MAX_NAME_LEN];
extern int  requiredCount;

extern char electiveCourses[MAX_ELECTIVE][4][MAX_NAME_LEN];
extern int  electiveCount;

// ==========================================
//    二、课程图数据结构
// ==========================================

// --- 边节点（邻接表中的边节点）---
struct EdgeNode {
    int       adjVex;     // 邻接点在课程数组中的下标
    EdgeNode* next;       // 指向下一条边
};

// --- 课程结构体 ---
struct Course {
    int       courseId;       // 课程编号
    char      courseName[MAX_NAME_LEN]; // 课程名称（教务端存课程代号如 "C01"）
    int       credit;         // 学分
    int       isRequired;     // 1=必修  0=选修
    int       inDegree;       // 入度（先修课程数量）
    int       outDegree;      // 出度（后继课程数量）
    EdgeNode* firstEdge;      // 邻接表头指针（第一条出边）
    int       semester;       // 所安排的学期（1~S；0 表示尚未安排，教务端排课用）
};

// --- 课程图结构体（邻接表存储）---
struct CourseGraph {
    Course* courses;          // 课程动态数组
    int     courseCount;      // 当前课程数量
    int     maxCourses;       // 课程数组最大容量
};

// ==========================================
//    三、学生选课系统结构体
// ==========================================

// --- 选修课结构体 ---
struct ElectiveCourse {
    int   electiveId;             // 选修课编号
    char  electiveName[MAX_NAME_LEN]; // 选修课名称
    int   credit;                 // 学分
    int   maxCapacity;            // 最大选课人数
    int   currentCount;           // 当前已选人数
};

// --- 学生结构体 ---
struct Student {
    int   studentId;              // 学号
    char  studentName[MAX_NAME_LEN]; // 姓名
    int   selectedIds[MAX_SELECTED]; // 已选选修课编号列表
    int   selectedCount;          // 已选课程数量
    int   totalCredit;            // 已选总学分
    int   maxCredit;              // 学期学分上限
};

// ==========================================
//    四、函数声明
// ==========================================

// ---- 二维数组操作 ----
void initRequiredArray();                          // 初始化必修课数组
void initElectiveArray();                          // 初始化选修课数组
int  addRequiredCourse(const char* name, int id,  // 添加必修课
    int credit, int finished);
int  addElectiveCourse(const char* name, int id,  // 添加选修课
    int credit, int finished);
int  findRequiredById(int id);                     // 按ID查必修课下标
int  findElectiveById(int id);                     // 按ID查选修课下标
void printRequiredArray();                         // 打印必修课列表
void printElectiveArray();                         // 打印选修课列表

// ---- 图操作 ----
void initGraph(CourseGraph& g, int maxNum);        // 初始化图
void destroyGraph(CourseGraph& g);                 // 销毁图（释放内存）
int  addCourseToGraph(CourseGraph& g, int id,      // 图中添加课程
    const char* name, int credit,
    int isRequired);
void addPrerequisite(CourseGraph& g,               // 添加先修关系
    int prereqId, int nextId);
int  findCourseIndex(CourseGraph& g, int courseId); // 按ID找课程下标
void printGraph(CourseGraph& g);                   // 打印图的邻接表
bool topologicalSort(CourseGraph& g, int topoOrder[],
    int& topoCount);              // 拓扑排序

// ---- 学生选课系统操作 ----
void initStudent(Student& s, int id, const char* name,
    int maxCredit);                   // 初始化学生
void initElectiveInfo(ElectiveCourse& e, int id,   // 初始化选修课信息
    const char* name, int credit,
    int maxCap);
int  selectElective(Student& s, ElectiveCourse& e); // 学生选课
int  dropElective(Student& s, ElectiveCourse& e);   // 学生退课
void printStudentInfo(Student& s);                  // 打印学生选课信息
void printElectiveInfo(ElectiveCourse& e);          // 打印选修课信息

#endif // COURSE_SYSTEM_H
