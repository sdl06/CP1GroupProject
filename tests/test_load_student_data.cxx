#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <filesystem>

namespace fs = std::filesystem;

// Simple scoped helper to run tests inside a temp directory so files stay isolated.
class ScopedTempDir {
public:
    ScopedTempDir() : old_path(fs::current_path()) {
        path = fs::temp_directory_path() / fs::path("cp1gp_test_XXXXXX");
        std::string tmpl = path.string();
        std::vector<char> mutable_path(tmpl.begin(), tmpl.end());
        mutable_path.push_back('\0');
        char *res = mkdtemp(mutable_path.data());
        if (!res) {
            throw std::runtime_error("mkdtemp failed");
        }
        path = res;
        fs::current_path(path);
    }
    ~ScopedTempDir() {
        fs::current_path(old_path);
        fs::remove_all(path);
    }
private:
    fs::path old_path;
    fs::path path;
};

TEST(LoadStudentData, NonexistentFileCreatesAndReturns1) {
    ScopedTempDir guard;
    const char *path = "next_id.txt";
    EXPECT_EQ(1, load_student_data(path));
    // File should now exist with the seeded value
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());
    int val = 0;
    ifs >> val;
    EXPECT_EQ(1, val);
}

TEST(LoadStudentData, ValidNumberFileReturnsThatNumber) {
    ScopedTempDir guard;
    std::ofstream ofs("next_id_valid_test.txt");
    ofs << "42\n";
    ofs.close();
    EXPECT_EQ(42, load_student_data("next_id_valid_test.txt"));
}

TEST(LoadStudentData, InvalidOrNegativeContentReturns1) {
    ScopedTempDir guard;
    std::ofstream ofs("next_id_invalid_test.txt");
    ofs << "-5\n";
    ofs.close();
    EXPECT_EQ(1, load_student_data("next_id_invalid_test.txt"));
}

// --- Additional tests for other functions in src/main.c ---

extern "C" {
    // Re-declare types and functions from main.c with C linkage
    typedef struct {
        char name[50];
        float grade;
    } subject_t;

    typedef struct {
        int student_id;
        char name[50];
        char studentid[15];
        int grade;
        char dateofbirth[11];
        char father_name[50];
        char mother_name[50];
        char phone_number[15];
        subject_t subject1;
        subject_t subject2;
        subject_t subject3;
        subject_t subject4;
        float average_grade;
    } student_t;

    void calculate_average(student_t *student);
    void update_next_id(const char *path, int new_id);
    int recompute_average_grade(const char *filename);
}

TEST(CalculateAverage, ComputesCorrectly) {
    student_t s;
    // zero init
    std::memset(&s, 0, sizeof(s));
    s.subject1.grade = 80.0f;
    s.subject2.grade = 90.0f;
    s.subject3.grade = 70.0f;
    s.subject4.grade = 60.0f;
    calculate_average(&s);
    float expected = (80.0f + 90.0f + 70.0f + 60.0f) / 4.0f;
    EXPECT_FLOAT_EQ(expected, s.average_grade);
}

TEST(UpdateNextId, ReplacesFileWithNewId) {
    ScopedTempDir guard;
    std::ofstream ofs("next_id_update_test.txt");
    ofs << "7\n";
    ofs.close();

    // Call function under test
    update_next_id("next_id_update_test.txt", 12345);

    // Read the file and verify contents
    std::ifstream ifs("next_id_update_test.txt");
    ASSERT_TRUE(ifs.is_open());
    int val = 0;
    ifs >> val;
    ifs.close();
    EXPECT_EQ(12345, val);
    EXPECT_FALSE(fs::exists("next_id.tmp"));
}

TEST(RecomputeAverageGrade, UpdatesDerivedAverage) {
    ScopedTempDir guard;
    std::ofstream ofs("student.txt");
    ofs << "SUBJECT1_GRADE = 80.0\n";
    ofs << "SUBJECT2_GRADE = 90.0\n";
    ofs << "SUBJECT3_GRADE = 70.0\n";
    ofs << "SUBJECT4_GRADE = 60.0\n";
    ofs << "AVERAGE_GRADE = 0.00\n";
    ofs.close();

    EXPECT_EQ(0, recompute_average_grade("student.txt"));

    std::ifstream ifs("student.txt");
    std::string contents((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("AVERAGE_GRADE = 75.00"), std::string::npos);
}
