#include "student_head.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>

namespace StudentApp {

Student::Student() {
    std::memset(id, 0, sizeof(id));
    currentSemester = 1;
    personalLimit = creditLimit;
    passed.assign(courseCount, false);
    for (int s = 0; s < MAX_SEMESTER; ++s) {
        for (int i = 0; i < MAX_COURSE; ++i) {
            selected[s][i] = -1;
        }
        semesterCredit[s] = 0;
    }
}

// ---------------------------------------------------------------------------
//  1. 初始化学生基本信息
// ---------------------------------------------------------------------------
bool initStudent(Student& stu) {
    std::cout << "\n---- 学生信息录入 ----\n";

    std::string sid;
    std::cout << "请输入你的学号（3 位字母/数字，例如 S01）：";
    if (!(std::cin >> sid)) {
        std::cout << "\n输入结束。\n";
        return false;
    }
    if (!IsValidCourseCode(sid)) {
        std::cout << "学号格式错误（应为 3 位字母或数字）！\n";
        return false;
    }
    CopyCourseId(stu.id, sid);

    if (!ReadInt("请输入你当前正在修的学期（1 ~ " + std::to_string(semesterCount) + "）：",
                 1, semesterCount, stu.currentSemester, "学期数量输入错误！")) {
        return false;
    }

    std::cout << "请输入你个人的每学期学分上限（默认 " << creditLimit << "，直接回车使用默认值）：";
    std::string limStr;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (std::getline(std::cin, limStr) && !limStr.empty()) {
        std::stringstream ss(limStr);
        int v = 0;
        if (ss >> v && v >= 1 && v <= creditLimit) {
            stu.personalLimit = v;
        } else {
            std::cout << "输入无效，使用默认值 " << creditLimit << "\n";
            stu.personalLimit = creditLimit;
        }
    } else {
        stu.personalLimit = creditLimit;
    }

    stu.passed.assign(courseCount, false);
    for (int s = 0; s < MAX_SEMESTER; ++s) {
        for (int i = 0; i < MAX_COURSE; ++i) {
            stu.selected[s][i] = -1;
        }
        stu.semesterCredit[s] = 0;
    }

    std::cout << "学生 " << stu.id << " 信息录入完成\n";
    std::cout << "  当前学期：第 " << stu.currentSemester << " 学期\n";
    std::cout << "  个人学分上限：" << stu.personalLimit << "\n";
    return true;
}

// ---------------------------------------------------------------------------
//  2. 输入已通过课程（可选）
// ---------------------------------------------------------------------------
bool inputPassedCourses(Student& stu) {
    std::cout << "\n请输入已通过的课程编号（直接回车表示没有）：";
    std::string line;
    if (!std::getline(std::cin, line) || line.empty()) {
        return true;
    }
    std::stringstream ss(line);
    std::string code;
    int count = 0;
    while (ss >> code) {
        int idx = FindCourse(code);
        if (idx == -1) {
            std::cout << "  警告：课程 " << code << " 不存在，已忽略\n";
            continue;
        }
        if (courses[idx].semester >= stu.currentSemester) {
            std::cout << "  警告：课程 " << code << " 在第 " << courses[idx].semester
                      << " 学期开设，还没到开课学期，已忽略\n";
            continue;
        }
        stu.passed[idx] = true;
        ++count;
    }
    std::cout << "  已标记 " << count << " 门课程为已通过\n";
    return true;
}

// ---------------------------------------------------------------------------
//  3. 选课合法性校验（核心）
// ---------------------------------------------------------------------------
bool validateSelection(const Student& stu, int courseIdx, int targetSemester, std::string& reason) {
    if (courseIdx < 0 || courseIdx >= courseCount) {
        reason = "课程编号无效";
        return false;
    }
    if (targetSemester < stu.currentSemester || targetSemester > semesterCount) {
        reason = "目标学期 " + std::to_string(targetSemester) + " 不在可选范围内（" +
                 std::to_string(stu.currentSemester) + " ~ " + std::to_string(semesterCount) + "）";
        return false;
    }

    bool open = false;
    for (int c : semesters[targetSemester - 1].courses) {
        if (c == courseIdx) { open = true; break; }
    }
    if (!open) {
        reason = "课程 " + std::string(courses[courseIdx].id) +
                 " 不在第 " + std::to_string(targetSemester) + " 学期开设（它在第 " +
                 std::to_string(courses[courseIdx].semester) + " 学期开设）";
        return false;
    }

    for (int prereq : prereqList[courseIdx]) {
        if (!stu.passed[prereq]) {
            reason = "课程 " + std::string(courses[courseIdx].id) +
                     " 的先修课 " + std::string(courses[prereq].id) + " 尚未完成";
            return false;
        }
    }

    if (stu.passed[courseIdx]) {
        reason = "课程 " + std::string(courses[courseIdx].id) + " 已经通过";
        return false;
    }

    for (int s = 0; s < MAX_SEMESTER; ++s) {
        for (int i = 0; i < MAX_COURSE; ++i) {
            if (stu.selected[s][i] == courseIdx) {
                reason = "课程 " + std::string(courses[courseIdx].id) +
                         " 已在第 " + std::to_string(s + 1) + " 学期选过";
                return false;
            }
        }
    }

    if (stu.semesterCredit[targetSemester - 1] + courses[courseIdx].credit > stu.personalLimit) {
        reason = "第 " + std::to_string(targetSemester) + " 学期学分 " +
                 std::to_string(stu.semesterCredit[targetSemester - 1]) +
                 " + 本课 " + std::to_string(courses[courseIdx].credit) +
                 " = " + std::to_string(stu.semesterCredit[targetSemester - 1] + courses[courseIdx].credit) +
                 " 超过个人上限 " + std::to_string(stu.personalLimit);
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
//  4. 手动选课
// ---------------------------------------------------------------------------
static void commitSelection(Student& stu, int courseIdx, int targetSemester) {
    for (int i = 0; i < MAX_COURSE; ++i) {
        if (stu.selected[targetSemester - 1][i] == -1) {
            stu.selected[targetSemester - 1][i] = courseIdx;
            break;
        }
    }
    stu.semesterCredit[targetSemester - 1] += courses[courseIdx].credit;
}

bool manualSelection(Student& stu, int targetSemester) {
    if (targetSemester < stu.currentSemester || targetSemester > semesterCount) {
        std::cout << "目标学期超出范围\n";
        return false;
    }
    std::cout << "\n---- 手动选课（第 " << targetSemester << " 学期）----\n";
    std::cout << "可选课程（专业课表在该学期开设、先修已过、未通过、未选过）：\n";
    std::cout << "--------------------------------------------------------------\n";
    int idx = 1;
    std::vector<int> candidates;
    for (int c : semesters[targetSemester - 1].courses) {
        if (!stu.passed[c]) {
            bool prereqOk = true;
            for (int p : prereqList[c]) {
                if (!stu.passed[p]) { prereqOk = false; break; }
            }
            const char* typeTag = courses[c].isRequired ? "必修" : "选修";
            if (!prereqOk) {
                std::cout << "  " << idx++ << ". " << courses[c].id << "(" << courses[c].credit
                          << "," << typeTag << ") [先修未满足]\n";
                continue;
            }
            candidates.push_back(c);
            std::cout << "  " << idx++ << ". " << courses[c].id << "(" << courses[c].credit
                      << "," << typeTag << ") [可选中]\n";
        }
    }
    std::cout << "当前该学期已选学分：" << stu.semesterCredit[targetSemester - 1]
              << " / " << stu.personalLimit << "\n";

    std::cout << "\n请输入要选的课程编号（多个用空格分隔，输入 done 结束）：";
    std::string line;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (!std::getline(std::cin, line) || line.empty()) return true;

    std::stringstream ss(line);
    std::string code;
    int okCount = 0, failCount = 0;
    while (ss >> code) {
        if (code == "done" || code == "q") break;
        int ci = FindCourse(code);
        if (ci == -1) {
            std::cout << "  课程 " << code << " 不存在\n";
            ++failCount;
            continue;
        }
        std::string reason;
        if (!validateSelection(stu, ci, targetSemester, reason)) {
            std::cout << "  无法选 " << code << "：" << reason << "\n";
            ++failCount;
            continue;
        }
        commitSelection(stu, ci, targetSemester);
        std::cout << "  已选 " << code << "(" << courses[ci].credit << ")\n";
        ++okCount;
    }
    std::cout << "\n本次选中 " << okCount << " 门，失败 " << failCount << " 门\n";
    return true;
}

// ---------------------------------------------------------------------------
//  5. 自动选课（内部贪心）
// ---------------------------------------------------------------------------
static std::vector<int> collectOpenCourses(const Student& stu, int targetSemester) {
    std::vector<int> pool;
    for (int c : semesters[targetSemester - 1].courses) {
        if (stu.passed[c]) continue;
        bool prereqOk = true;
        for (int p : prereqList[c]) {
            if (!stu.passed[p]) { prereqOk = false; break; }
        }
        if (prereqOk) pool.push_back(c);
    }
    return pool;
}

static bool tryAutoSelect(Student& stu, int targetSemester, SelectionStyle style) {
    std::vector<int> pool = collectOpenCourses(stu, targetSemester);
    if (pool.empty()) {
        std::cout << "第 " << targetSemester << " 学期没有可自动选的课程\n";
        return false;
    }

    switch (style) {
    case SelectionStyle::PreferencePrereq: {
        std::sort(pool.begin(), pool.end(), [](int a, int b) {
            if (courses[a].isRequired != courses[b].isRequired)
                return courses[a].isRequired > courses[b].isRequired;
            int pa = static_cast<int>(prereqList[a].size());
            int pb = static_cast<int>(prereqList[b].size());
            if (pa != pb) return pa < pb;
            return std::string(courses[a].id) < std::string(courses[b].id);
        });
        break;
    }
    case SelectionStyle::Balanced: {
        std::sort(pool.begin(), pool.end(), [](int a, int b) {
            if (courses[a].isRequired != courses[b].isRequired)
                return courses[a].isRequired > courses[b].isRequired;
            if (courses[a].credit != courses[b].credit) return courses[a].credit < courses[b].credit;
            return std::string(courses[a].id) < std::string(courses[b].id);
        });
        break;
    }
    case SelectionStyle::Concentrated: {
        std::sort(pool.begin(), pool.end(), [](int a, int b) {
            if (courses[a].isRequired != courses[b].isRequired)
                return courses[a].isRequired > courses[b].isRequired;
            if (courses[a].credit != courses[b].credit) return courses[a].credit > courses[b].credit;
            return std::string(courses[a].id) < std::string(courses[b].id);
        });
        break;
    }
    case SelectionStyle::Mixed: {
        std::sort(pool.begin(), pool.end(), [](int a, int b) {
            if (courses[a].isRequired != courses[b].isRequired)
                return courses[a].isRequired > courses[b].isRequired;
            int pa = static_cast<int>(prereqList[a].size());
            int pb = static_cast<int>(prereqList[b].size());
            if (pa != pb) return pa < pb;
            return courses[a].credit < courses[b].credit;
        });
        break;
    }
    }

    std::vector<int> picked;
    int usedCredit = stu.semesterCredit[targetSemester - 1];
    for (int c : pool) {
        if (usedCredit + courses[c].credit <= stu.personalLimit) {
            picked.push_back(c);
            usedCredit += courses[c].credit;
        }
    }

    if (picked.empty()) {
        std::cout << "第 " << targetSemester << " 学期剩余容量不足，无法自动选课\n";
        return false;
    }

    for (int c : picked) commitSelection(stu, c, targetSemester);

    const char* tag = "";
    switch (style) {
    case SelectionStyle::PreferencePrereq: tag = "先修优先型"; break;
    case SelectionStyle::Balanced:          tag = "均衡型";     break;
    case SelectionStyle::Concentrated:      tag = "集中型";     break;
    case SelectionStyle::Mixed:             tag = "综合型";     break;
    }
    std::cout << "\n自动选课完成（" << tag << "），共 " << picked.size()
              << " 门，学分合计 " << usedCredit << "\n  已选课程：";
    for (std::size_t k = 0; k < picked.size(); ++k) {
        if (k > 0) std::cout << " ";
        const int ci = picked[k];
        std::cout << courses[ci].id << "(" << courses[ci].credit
                  << (courses[ci].isRequired ? ",必" : ",选") << ")";
    }
    std::cout << "\n";
    return true;
}

bool autoSelect(Student& stu, int targetSemester, SelectionStyle style) {
    return tryAutoSelect(stu, targetSemester, style);
}

// ---------------------------------------------------------------------------
//  6. 生成多种差异大的选课方案（回溯枚举 + 筛选）
// ---------------------------------------------------------------------------
static int planScore(const std::vector<int>& plan) {
    int s = 0;
    for (int c : plan) s += courses[c].credit;
    return s;
}

static double planDiversity(const SelectionPlan& a, const SelectionPlan& b) {
    std::set<int> sa(a.courseIdxs.begin(), a.courseIdxs.end());
    std::set<int> sb(b.courseIdxs.begin(), b.courseIdxs.end());
    int diff = 0;
    for (int x : sa) if (sb.find(x) == sb.end()) ++diff;
    for (int x : sb) if (sa.find(x) == sb.end()) ++diff;
    return static_cast<double>(diff);
}

bool generateMultiplePlans(Student& stu, int targetSemester, int count,
                           std::vector<SelectionPlan>& plans) {
    std::vector<int> pool = collectOpenCourses(stu, targetSemester);
    if (pool.empty()) {
        std::cout << "第 " << targetSemester << " 学期没有可选课程\n";
        return false;
    }

    int minCredit = stu.semesterCredit[targetSemester - 1];
    std::vector<std::vector<int>> all;
    std::vector<int> cur;

    std::function<void(int, int)> dfs = [&](int idx, int used) {
        if (used > stu.personalLimit) return;
        if (idx == static_cast<int>(pool.size())) {
            if (!cur.empty() && used >= minCredit) all.push_back(cur);
            return;
        }
        cur.push_back(pool[idx]);
        dfs(idx + 1, used + courses[pool[idx]].credit);
        cur.pop_back();
        dfs(idx + 1, used);
    };
    dfs(0, minCredit);

    if (all.empty()) {
        std::cout << "没有满足条件的方案\n";
        return false;
    }

    std::sort(all.begin(), all.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
        return planScore(a) < planScore(b);
    });

    int take = std::min(count, static_cast<int>(all.size()));
    std::vector<int> pickedIdx;
    pickedIdx.push_back(0);

    while (static_cast<int>(pickedIdx.size()) < take) {
        int best = -1;
        double bestDiv = -1.0;
        for (int i = 0; i < static_cast<int>(all.size()); ++i) {
            bool already = false;
            for (int p : pickedIdx) if (p == i) { already = true; break; }
            if (already) continue;
            double d = 0.0;
            for (int p : pickedIdx) {
                SelectionPlan ap = {all[p], 0, ""};
                SelectionPlan bp = {all[i], 0, ""};
                d += planDiversity(ap, bp);
            }
            if (d > bestDiv) { bestDiv = d; best = i; }
        }
        if (best == -1) break;
        pickedIdx.push_back(best);
    }

    plans.clear();
    const char* styles[] = {"最小学分", "次优", "中等", "较重", "最大学分"};
    for (int k = 0; k < static_cast<int>(pickedIdx.size()); ++k) {
        SelectionPlan sp;
        sp.courseIdxs = all[pickedIdx[k]];
        sp.totalCredit = planScore(all[pickedIdx[k]]) - minCredit;
        if (k < 5) sp.styleTag = styles[k];
        else sp.styleTag = "方案" + std::to_string(k + 1);
        plans.push_back(sp);
    }

    std::cout << "\n已生成 " << plans.size() << " 种差异较大的选课方案：\n";
    std::cout << "--------------------------------------------------------------\n";
    for (int k = 0; k < static_cast<int>(plans.size()); ++k) {
        std::cout << "  方案 " << (k + 1) << "（" << plans[k].styleTag
                  << "，新增学分合计 " << plans[k].totalCredit << "）：";
        for (std::size_t j = 0; j < plans[k].courseIdxs.size(); ++j) {
            if (j > 0) std::cout << " ";
            const int ci = plans[k].courseIdxs[j];
            std::cout << courses[ci].id << "(" << courses[ci].credit
                      << (courses[ci].isRequired ? ",必" : ",选") << ")";
        }
        std::cout << "\n";
    }
    return true;
}

// ---------------------------------------------------------------------------
//  7. 打印学生进度拓扑图（DAG 标记）
// ---------------------------------------------------------------------------
void printProgressTopo(const Student& stu) {
    std::cout << "\n---- 个人学习进度（按先修拓扑顺序）----\n";
    std::cout << "  标记说明：[已通过]  [可选]  [未解锁]\n";
    std::cout << "--------------------------------------------------------------\n";

    std::vector<int> topo(courseCount);
    for (int i = 0; i < courseCount; ++i) topo[i] = i;
    std::sort(topo.begin(), topo.end(), [](int a, int b) {
        if (courses[a].semester != courses[b].semester)
            return courses[a].semester < courses[b].semester;
        return std::string(courses[a].id) < std::string(courses[b].id);
    });

    for (int c : topo) {
        const char* status = "";
        if (stu.passed[c]) {
            status = "[已通过]";
        } else {
            bool prereqOk = true;
            for (int p : prereqList[c]) {
                if (!stu.passed[p]) { prereqOk = false; break; }
            }
            bool open = courses[c].semester >= stu.currentSemester &&
                        courses[c].semester <= semesterCount;
            if (prereqOk && open) {
                status = "[可  选]";
            } else {
                status = "[未解锁]";
            }
        }

        std::cout << "  " << status << "  " << courses[c].id
                  << "(" << courses[c].credit
                  << (courses[c].isRequired ? ",必" : ",选") << ")  第"
                  << courses[c].semester << "学期  先修：";
        if (prereqList[c].empty()) {
            std::cout << "无";
        } else {
            for (std::size_t k = 0; k < prereqList[c].size(); ++k) {
                if (k > 0) std::cout << ",";
                std::cout << courses[prereqList[c][k]].id;
            }
        }
        std::cout << "\n";
    }
}

// ---------------------------------------------------------------------------
//  8. 打印学生个人课表
// ---------------------------------------------------------------------------
void printStudentSchedule(const Student& stu) {
    std::cout << "\n================ 学生 " << stu.id << " 的选课课表 ================\n";
    for (int s = 0; s < semesterCount; ++s) {
        std::cout << "第" << (s + 1) << "学期（已选学分 "
                  << stu.semesterCredit[s] << "/" << stu.personalLimit << "）：\n";
        bool any = false;
        for (int i = 0; i < MAX_COURSE; ++i) {
            if (stu.selected[s][i] != -1) {
                const int ci = stu.selected[s][i];
                std::cout << "  " << courses[ci].id
                          << "(" << courses[ci].credit
                          << (courses[ci].isRequired ? ",必" : ",选") << ")";
                any = true;
            }
        }
        if (!any) {
            std::cout << "  （无已选课程）";
        }
        std::cout << "\n";
    }
    std::cout << "==================================================================\n";
}

// ---------------------------------------------------------------------------
//  9. 保存学生课表到文件
// ---------------------------------------------------------------------------
bool saveStudentPlan(const Student& stu) {
    std::cout << "请输入保存文件名：";
    std::string fn;
    if (!(std::cin >> fn)) {
        std::cout << "\n取消保存。\n";
        return false;
    }
    std::ofstream file(Utf8Path(fn), std::ios::binary);
    if (!file) {
        std::cout << "无法打开文件：" << fn << "\n";
        return false;
    }
    const char bom[3] = {'\xEF', '\xBB', '\xBF'};
    file.write(bom, 3);

    std::ostringstream oss;
    oss << "========== 学生 " << stu.id << " 的选课课表 ==========\n";
    oss << "当前学期：第 " << stu.currentSemester << " 学期\n";
    oss << "个人学分上限：" << stu.personalLimit << "\n\n";

    int totalPicked = 0;
    for (int s = 0; s < semesterCount; ++s) {
        oss << "第" << (s + 1) << "学期（已选学分 "
            << stu.semesterCredit[s] << "/" << stu.personalLimit << "）：\n";
        for (int i = 0; i < MAX_COURSE; ++i) {
            if (stu.selected[s][i] != -1) {
                const int ci = stu.selected[s][i];
                oss << "  " << courses[ci].id
                    << "(" << courses[ci].credit
                    << (courses[ci].isRequired ? ",必" : ",选") << ")\n";
                ++totalPicked;
            }
        }
        oss << "\n";
    }
    oss << "已选课程总数：" << totalPicked << "\n";

    file.write(oss.str().data(), static_cast<std::streamsize>(oss.str().size()));
    file.close();
    std::cout << "已保存到：" << fn << "\n";
    return true;
}

// ---------------------------------------------------------------------------
//  10. 学生菜单
// ---------------------------------------------------------------------------
void studentMenu() {
    Student stu;
    if (!initStudent(stu)) return;
    inputPassedCourses(stu);

    while (true) {
        std::cout << "\n========== 学 生 选 课 终 端 ==========\n";
        std::cout << "  1. 手动选课（逐门选择）\n";
        std::cout << "  2. 自动选课（选择风格）\n";
        std::cout << "  3. 生成多种方案供选择\n";
        std::cout << "  4. 查看个人进度拓扑图\n";
        std::cout << "  5. 查看已选课程\n";
        std::cout << "  6. 保存选课课表到文件\n";
        std::cout << "  0. 退出\n";
        std::cout << "=======================================\n";

        int opt = -1;
        if (!ReadInt("请选择操作（0 ~ 6）：", 0, 6, opt, "请输入 0 ~ 6 之间的数字！")) {
            std::cout << "输入结束，程序退出。\n";
            return;
        }

        switch (opt) {
        case 1: {
            int ts;
            if (!ReadInt("选第几学期：", stu.currentSemester, semesterCount, ts,
                         "学期超出范围！")) break;
            manualSelection(stu, ts);
            break;
        }
        case 2: {
            int ts;
            if (!ReadInt("选第几学期：", stu.currentSemester, semesterCount, ts,
                         "学期超出范围！")) break;
            std::cout << "请选择自动选课风格：\n";
            std::cout << "  1. 先修优先型（优先选先修课数量少的）\n";
            std::cout << "  2. 均衡型（优先选学分低的，负担最轻）\n";
            std::cout << "  3. 集中型（优先选学分高的，先啃硬骨头）\n";
            std::cout << "  4. 综合型（先修优先 + 均衡学分）\n";
            int s;
            if (!ReadInt("选择（1~4）：", 1, 4, s, "请输入 1~4")) break;
            SelectionStyle ss =
                static_cast<SelectionStyle>(s - 1);
            autoSelect(stu, ts, ss);
            break;
        }
        case 3: {
            int ts;
            if (!ReadInt("选第几学期：", stu.currentSemester, semesterCount, ts,
                         "学期超出范围！")) break;
            int cnt;
            if (!ReadInt("要生成几种方案：", 1, 10, cnt, "请输入 1~10")) break;
            std::vector<SelectionPlan> plans;
            generateMultiplePlans(stu, ts, cnt, plans);
            if (plans.empty()) break;
            std::cout << "请选择你要应用到课表的方案编号（输入 0 不应用）：";
            std::string line;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (std::getline(std::cin, line) && !line.empty()) {
                int pi = std::stoi(line);
                if (pi >= 1 && pi <= static_cast<int>(plans.size())) {
                    for (int c : plans[pi - 1].courseIdxs) {
                        bool ok = true;
                        for (int s2 = 0; s2 < MAX_SEMESTER; ++s2) {
                            for (int i2 = 0; i2 < MAX_COURSE; ++i2) {
                                if (stu.selected[s2][i2] == c) { ok = false; break; }
                            }
                        }
                        if (!ok) continue;
                        commitSelection(stu, c, ts);
                    }
                    std::cout << "已应用方案 " << pi << "\n";
                }
            }
            break;
        }
        case 4:
            printProgressTopo(stu);
            break;
        case 5:
            printStudentSchedule(stu);
            break;
        case 6:
            saveStudentPlan(stu);
            break;
        case 0:
            std::cout << "再见！\n";
            return;
        default:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
//  学生端入口（供登录界面调用）
// ---------------------------------------------------------------------------
int studentMain() {
    InitConsole();

    std::cout << "\n================ 学生选课系统 ================\n\n";

    std::string defaultFile = "plan_data.txt";
    std::string fileName;
    std::cout << "请输入课表文件名（直接回车使用默认 " << defaultFile << "）：";
    if (!std::getline(std::cin, fileName) || fileName.empty()) {
        fileName = defaultFile;
    }

    if (!loadTeachingPlan(fileName)) {
        std::cout << "\n课表加载失败，返回登录界面。\n";
        return 1;
    }

    studentMenu();

    std::cout << "\n学生端结束，返回登录界面。\n";
    return 0;
}

} // namespace StudentApp
