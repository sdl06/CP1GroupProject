#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>

extern "C" {
    int load_student_data(const char *path);
}

static std::string write_temp(const std::string &name, const std::string &content) {
    std::string path = std::string("/tmp/") + name;
    std::ofstream ofs(path.c_str());
    ofs << content;
    ofs.close();
    return path;
}

TEST(LoadStudentData, NonexistentFileReturns1) {
    // Choose a filename that is unlikely to exist
    const char *path = "/tmp/nonexistent_file_hopefully_unique_12345.tmp";
    // If the file truly doesn't exist, load_student_data should return 1
    EXPECT_EQ(1, load_student_data(path));
}

TEST(LoadStudentData, ValidNumberFileReturnsThatNumber) {
    std::string path = write_temp("next_id_valid_test.txt", "42\n");
    EXPECT_EQ(42, load_student_data(path.c_str()));
    std::remove(path.c_str());
}

TEST(LoadStudentData, InvalidContentReturns1) {
    std::string path = write_temp("next_id_invalid_test.txt", "abc\n");
    EXPECT_EQ(1, load_student_data(path.c_str()));
    std::remove(path.c_str());
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
    void add_student();
    void edit_student();
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
    std::string path = write_temp("next_id_update_test.txt", "7\n");
    // Call function under test
    update_next_id(path.c_str(), 12345);

    // Read the file and verify contents
    std::ifstream ifs(path.c_str());
    ASSERT_TRUE(ifs.is_open());
    int val = 0;
    ifs >> val;
    ifs.close();
    EXPECT_EQ(12345, val);
    std::remove(path.c_str());
    // cleanup any stray temp file used by implementation
    std::remove("next_id.tmp");
}

TEST(EditStudent, Callable) {
    // empty function in main.c -- ensure it can be called
    edit_student();
    SUCCEED();
}

TEST(AddStudent, CreatesFileFromSimulatedStdin) {
    // Prepare simulated stdin with whitespace-separated tokens.
    // Use date without slashes to avoid creating directories when used in filename.
    std::string input = "TestName 01012000 S12345 85 FatherName MotherName 5551234 Math 90.0 Eng 80.0 Sci 70.0 Hist 60.0\n";
    std::string inpath = write_temp("add_student_input.txt", input);

    // Redirect stdin to our temp file
    FILE *saved_stdin = stdin;
    FILE *f = freopen(inpath.c_str(), "r", stdin);
    ASSERT_NE(f, nullptr);

    // Call function under test
    add_student();

    // Restore stdin
    fflush(stdin);
    stdin = saved_stdin;

    // Verify the expected output file was created
    std::string outname = "TestName_01012000.txt";
    std::ifstream ofs(outname.c_str());
    ASSERT_TRUE(ofs.is_open());
    // basic sanity check: file should contain the name and DOB
    std::string contents;
    std::getline(ofs, contents, '\0');
    ofs.close();
    EXPECT_NE(contents.find("Name: TestName"), std::string::npos);
    EXPECT_NE(contents.find("Date of Birth: 01012000"), std::string::npos);

    // cleanup
    std::remove(inpath.c_str());
    std::remove(outname.c_str());
}
// optional: main provided by gtest's linker when building the test binary