/*
 * ============================================================================
 * STUDENT MANAGEMENT SYSTEM FOR SCHOOLS
 * ============================================================================
 * 
 * This program manages student records for educational institutions.
 * It allows administrators to:
 *   - Add new student records with personal and academic information
 *   - Edit existing student records
 *   - Store data persistently in text files
 * 
 * Each student record includes:
 *   - Personal Information: Name, Date of Birth, Parent Names, Contact Details
 *   - Academic Information: Grades in 4 subjects, Average Grade Calculation
 *   - Unique Student ID for identification
 * 
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

// Thread synchronization: Ensures only one user can edit a file at a time
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

// LIMITATIONS (intentional for this simple file-based tool):
// - Names use %s scanning, so no spaces in names/family names/subjects.
// - Only numeric subject grades and grade edits are accepted.
// - Student data is stored as individual text files under data/students/.

/*
 * Subject Structure
 * ================
 * Stores information about a single subject/course
 * - name: Name of the subject (e.g., Mathematics, English)
 * - grade: Numerical grade received in this subject
 */
typedef struct {
    char name[50];
    float grade;
} subject_t;

/*
 * Student Structure
 * =================
 * Complete record for a single student
 * 
 * Identification:
 *   - student_id: Unique number assigned by system
 *   - studentid: Official student ID (may differ from student_id)
 * 
 * Personal Information:
 *   - name: Full name of student
 *   - dateofbirth: Date of birth (format: DD/MM/YYYY)
 *   - father_name: Father's name
 *   - mother_name: Mother's name
 *   - phone_number: Contact phone number
 * 
 * Academic Information:
 *   - grade: Overall grade/class level
 *   - subject1-4: Four subjects with individual grades
 *   - average_grade: Calculated average of all 4 subject grades
 */
typedef struct {
    int student_id;
    char name[50];
    char family_name[50];
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

// All persistent files live under data/, student files under data/students/.
static const char *DATA_DIR = "data";
static const char *STUDENT_DIR = "data/students";

/*
 * FUNCTION: calculate_average
 * ===========================
 * Calculates the average grade across all 4 subjects
 * 
 * Parameters:
 *   - student: Pointer to student record to update
 * 
 * How it works:
 *   - Adds up all 4 subject grades
 *   - Divides by 4 to get the average
 *   - Stores result in student.average_grade
 */
void calculate_average(student_t *student) {
    student->average_grade = (student->subject1.grade + student->subject2.grade +
                              student->subject3.grade + student->subject4.grade) / 4.0f;
}

/*
 * FUNCTION: load_student_data
 * ============================
 * Reads the next available student ID from file
 * 
 * Parameters:
 *   - path: Path to the ID counter file
 * 
 * Returns:
 *   - Next available student ID number
 */
int ensure_data_directories(void) {
    if (mkdir(DATA_DIR, 0777) != 0 && errno != EEXIST) {
        perror("unable to create data directory");
        return -1;
    }
    if (mkdir(STUDENT_DIR, 0777) != 0 && errno != EEXIST) {
        perror("unable to create student directory");
        return -1;
    }
    return 0;
}

int load_student_data(const char *path) {
    if (ensure_data_directories() != 0) {
        return -1;
    }
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        // Counter file missing: seed it with 1 so IDs start at a safe default
        file = fopen(path, "w");
        if (!file) {
            perror("unable to create ID counter");
            return -1;
        }
        fprintf(file, "1\n");
        fclose(file);
        return 1;
    }

    int id = 1;
    if (fscanf(file, "%d", &id) != 1 || id < 1) {
        // Corrupt or negative content: reset to a sane value
        id = 1;
    }
    fclose(file);
    return id;
}

/*
 * FUNCTION: update_next_id
 * =========================
 * Updates the counter file with the next available student ID
 * Ensures each new student gets a unique ID
 * 
 * Parameters:
 *   - path: Path to the ID counter file
 *   - new_id: The next ID number to assign
 */
void update_next_id(const char *path, int new_id) {
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    FILE *file = fopen(temp_path, "w");
    if (!file) {
        printf("Error opening file for writing.\n");
        return;
    }
    fprintf(file, "%d\n", new_id);
    fclose(file);

    remove(path);
    rename(temp_path, path);
}

/*
 * FUNCTION: recompute_average_grade
 * ==================================
 * Re-reads subject grades from disk and refreshes AVERAGE_GRADE.
 * Used after subject grade edits to keep derived data correct.
 */
int recompute_average_grade(const char *filename) {
    float g1 = 0.0f, g2 = 0.0f, g3 = 0.0f, g4 = 0.0f;
    int have1 = 0, have2 = 0, have3 = 0, have4 = 0;
    char line[512];

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen for average recompute");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "SUBJECT1_GRADE = %f", &g1) == 1) {
            have1 = 1;
        } else if (sscanf(line, "SUBJECT2_GRADE = %f", &g2) == 1) {
            have2 = 1;
        } else if (sscanf(line, "SUBJECT3_GRADE = %f", &g3) == 1) {
            have3 = 1;
        } else if (sscanf(line, "SUBJECT4_GRADE = %f", &g4) == 1) {
            have4 = 1;
        }
    }
    fclose(file);

    if (!(have1 && have2 && have3 && have4)) {
        printf("Warning: unable to recompute average, missing subject grades.\n");
        return -1;
    }

    float avg = (g1 + g2 + g3 + g4) / 4.0f;

    FILE *original = fopen(filename, "r");
    if (!original) {
        perror("fopen original during average recompute");
        return -1;
    }

    char tempname[160];
    snprintf(tempname, sizeof(tempname), "%s.avgtmp", filename);
    FILE *temp = fopen(tempname, "w");
    if (!temp) {
        perror("fopen temp during average recompute");
        fclose(original);
        return -1;
    }

    int average_written = 0;
    while (fgets(line, sizeof(line), original)) {
        if (strncmp(line, "AVERAGE_GRADE = ", 16) == 0) {
            fprintf(temp, "AVERAGE_GRADE = %.2f\n", avg);
            average_written = 1;
        } else {
            fputs(line, temp);
        }
    }
    if (!average_written) {
        fprintf(temp, "AVERAGE_GRADE = %.2f\n", avg);
    }

    fclose(original);
    fclose(temp);

    if (remove(filename) != 0) {
        perror("remove during average recompute");
        remove(tempname);
        return -1;
    }
    if (rename(tempname, filename) != 0) {
        perror("rename during average recompute");
        remove(tempname);
        return -1;
    }

    return 0;
}

/*
 * FUNCTION: reset_system
 * ======================
 * Resets next_id to 1 and removes all student files in the student directory.
 * Uses the mutex to avoid races with ongoing edits.
 */
void reset_system(void) {
    if (ensure_data_directories() != 0) {
        return;
    }

    pthread_mutex_lock(&file_mutex);

    FILE *id_file = fopen("data/next_id.txt", "w");
    if (!id_file) {
        perror("unable to reset ID counter");
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    fprintf(id_file, "1\n");
    fclose(id_file);

    DIR *dir = opendir(STUDENT_DIR);
    if (!dir) {
        perror("unable to open student directory");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", STUDENT_DIR, entry->d_name);
        remove(path);
    }
    closedir(dir);

    pthread_mutex_unlock(&file_mutex);
    printf("System reset: ID counter set to 1 and all student files removed.\n");
}

/*
 * FUNCTION: add_student
 * ======================
 * Main function to add a new student to the system
 * 
 * Process:
 *   1. Gets next available student ID from counter file
 *   2. Prompts user for all student information
 *   3. Creates a new file named output_[STUDENT_ID].txt
 *   4. Writes all information to file in KEY = VALUE format
 *   5. Calculates and stores average grade
 * 
 * Data Flow:
 *   User Input → Student Structure → File Storage
 */
void add_student(void) {
    // STEP 1: Initialize student and get next available ID
    student_t student;
    int next_id = load_student_data("data/next_id.txt");
    if (next_id < 1) {
        return;
    }
    student.student_id = next_id;
    update_next_id("data/next_id.txt", student.student_id + 1);

    // STEP 2: Create file for this student (filename = output_[ID].txt)
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/output_%d.txt", STUDENT_DIR, student.student_id);
    
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // STEP 3: Collect personal information from user
    printf("\n===== STUDENT PERSONAL INFORMATION =====\n");
    printf("Enter student name (no spaces supported): ");
    scanf("%49s", student.name);
    fprintf(file, "NAME = %s\n", student.name);
    printf("Enter family name (no spaces supported): ");
    scanf("%49s", student.family_name);
    fprintf(file, "FAMILY_NAME = %s\n", student.family_name);
    
    printf("Enter date of birth (DD/MM/YYYY): ");
    scanf("%10s", student.dateofbirth);
    fprintf(file, "DOB = %s\n", student.dateofbirth);
    
    printf("Enter student ID: ");
    scanf("%14s", student.studentid);
    fprintf(file, "STUDENT_ID = %s\n", student.studentid);
    
    printf("Enter father's name (no spaces supported): ");
    scanf("%49s", student.father_name);
    fprintf(file, "FATHER_NAME = %s\n", student.father_name);
    
    printf("Enter mother's name (no spaces supported): ");
    scanf("%49s", student.mother_name);
    fprintf(file, "MOTHER_NAME = %s\n", student.mother_name);
    
    printf("Enter phone number: ");
    scanf("%14s", student.phone_number);
    fprintf(file, "PHONE_NUMBER = %s\n", student.phone_number);

    // STEP 4: Collect academic information
    printf("\n===== STUDENT ACADEMIC INFORMATION =====\n");
    printf("Enter student grade/class level: ");
    scanf("%d", &student.grade);
    fprintf(file, "GRADE = %d\n", student.grade);
    
    // STEP 5: Collect grades for 4 subjects
    printf("\n===== SUBJECT GRADES (4 Subjects) =====\n");
    
    printf("Enter subject 1 name (no spaces supported): ");
    scanf("%49s", student.subject1.name);
    fprintf(file, "SUBJECT1_NAME = %s\n", student.subject1.name);
    printf("Enter subject 1 grade: ");
    scanf("%f", &student.subject1.grade);
    fprintf(file, "SUBJECT1_GRADE = %.2f\n", student.subject1.grade);
    
    printf("Enter subject 2 name (no spaces supported): ");
    scanf("%49s", student.subject2.name);
    fprintf(file, "SUBJECT2_NAME = %s\n", student.subject2.name);
    printf("Enter subject 2 grade: ");
    scanf("%f", &student.subject2.grade);
    fprintf(file, "SUBJECT2_GRADE = %.2f\n", student.subject2.grade);
    
    printf("Enter subject 3 name (no spaces supported): ");
    scanf("%49s", student.subject3.name);
    fprintf(file, "SUBJECT3_NAME = %s\n", student.subject3.name);
    printf("Enter subject 3 grade: ");
    scanf("%f", &student.subject3.grade);
    fprintf(file, "SUBJECT3_GRADE = %.2f\n", student.subject3.grade);
    
    printf("Enter subject 4 name (no spaces supported): ");
    scanf("%49s", student.subject4.name);
    fprintf(file, "SUBJECT4_NAME = %s\n", student.subject4.name);
    printf("Enter subject 4 grade: ");
    scanf("%f", &student.subject4.grade);
    fprintf(file, "SUBJECT4_GRADE = %.2f\n", student.subject4.grade);
    
    // STEP 6: Calculate and save average grade
    calculate_average(&student);
    fprintf(file, "AVERAGE_GRADE = %.2f\n", student.average_grade);
    
    printf("\n✓ Student added successfully!\n");
    printf("✓ Student ID: %d\n", student.student_id);
    printf("✓ File saved: %s\n\n", filename);
    fclose(file);
}

/*
 * FUNCTION: edit_student
 * =======================
 * Allows modification of existing student records
 * 
 * Process:
 *   1. User enters student ID to find
 *   2. Displays menu of editable fields
 *   3. User selects which field to modify
 *   4. User enters new value
 *   5. System reads original file, finds target field, updates it
 *   6. Writes updated data to temporary file
 *   7. Replaces original file with updated version
 * 
 * Safety Features:
 *   - Uses mutex lock to prevent concurrent file access
 *   - Creates temporary file before modifying original
 *   - Only replaces original if update was successful
 *   - Cleans up temporary files on error
 */
void print_student_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open %s for preview.\n", filename);
        return;
    }
    printf("\n----- Current Record (%s) -----\n", filename);
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        fputs(line, stdout);
    }
    printf("----- End Record -----\n\n");
    fclose(file);
}

void edit_student(void) {
    // SECTION 1: Get student ID from user
    // Validates input to ensure we have a valid integer ID
    if (ensure_data_directories() != 0) {
        return;
    }
    int id;
    printf("What student ID do you want to edit? ");
    if (scanf("%d", &id) != 1 || id < 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n') { }
        return;
    }
    while (getchar() != '\n') { }  // clear trailing newline from input buffer

    // Construct filename based on student ID
    char filename[128];
    snprintf(filename, sizeof(filename), "%s/output_%d.txt", STUDENT_DIR, id);

    // SECTION 2: Show current values then display menu of editable fields
    print_student_file(filename);

    // Display menu of editable fields
    // User selects which field they want to modify
    int choice;
    printf("\n===== EDIT STUDENT RECORD =====\n");
    printf("What do you want to edit?\n");
    printf("1. Name\n");
    printf("2. Grade\n");
    printf("3. Phone Number\n");
    printf("4. Father's Name\n");
    printf("5. Mother's Name\n");
    printf("6. Date of Birth\n");
    printf("7. Family Name\n");
    printf("8. Subject 1 Grade\n");
    printf("9. Subject 2 Grade\n");
    printf("10. Subject 3 Grade\n");
    printf("11. Subject 4 Grade\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 11) {
        printf("Invalid choice.\n");
        while (getchar() != '\n') { }
        return;
    }
    while (getchar() != '\n') { }  // clear trailing newline

    // SECTION 3: Get new value from user
    // Uses fgets to safely read user input without buffer overflow
    char new_value[256];
    printf("Enter new value: ");
    if (!fgets(new_value, sizeof(new_value), stdin)) {
        printf("Input error.\n");
        return;
    }
    // Remove trailing newline from fgets input
    new_value[strcspn(new_value, "\n")] = '\0';

    // SECTION 4: Input validation
    int int_value = 0;
    float float_value = 0.0f;
    char trailing;
    if (choice == 2) {
        if (sscanf(new_value, "%d %c", &int_value, &trailing) != 1 || int_value < 0) {
            printf("Invalid numeric input for grade.\n");
            return;
        }
    } else if (choice == 8 || choice == 9 || choice == 10 || choice == 11) {
        if (sscanf(new_value, "%f %c", &float_value, &trailing) != 1) {
            printf("Invalid numeric input for subject grade.\n");
            return;
        }
    }

    // SECTION 5: File modification with mutex lock
    // Lock the file_mutex to ensure thread-safe file operations
    // This prevents two users from editing the same file simultaneously
    pthread_mutex_lock(&file_mutex);

    // Open original student file for reading
    FILE *original = fopen(filename, "r");
    if (!original) {
        pthread_mutex_unlock(&file_mutex);
        perror("fopen original");
        return;
    }

    // Create temporary file to write updated student data
    // We use a temp file to avoid losing data if update fails
    char tempname[128];
    snprintf(tempname, sizeof(tempname), "temp_%d.txt", id);
    FILE *temp = fopen(tempname, "w");
    if (!temp) {
        fclose(original);
        pthread_mutex_unlock(&file_mutex);
        perror("fopen temp");
        return;
    }

    // SECTION 6: Process file line-by-line
    // Read original file and write to temp, updating the selected field
    char line[512];
    int updated = 0;  // Flag to track if target field was found and updated

    while (fgets(line, sizeof(line), original)) {
        // Each choice corresponds to a different student field
        // When we find the matching field, we replace it with new value
        if (choice == 1 && strstr(line, "NAME = ") == line) {
            fprintf(temp, "NAME = %s\n", new_value);
            updated = 1;
        } else if (choice == 2 && strstr(line, "GRADE = ") == line) {
            fprintf(temp, "GRADE = %d\n", int_value);
            updated = 1;
        } else if (choice == 3 && strstr(line, "PHONE_NUMBER = ") == line) {
            fprintf(temp, "PHONE_NUMBER = %s\n", new_value);
            updated = 1;
        } else if (choice == 4 && strstr(line, "FATHER_NAME = ") == line) {
            fprintf(temp, "FATHER_NAME = %s\n", new_value);
            updated = 1;
        } else if (choice == 5 && strstr(line, "MOTHER_NAME = ") == line) {
            fprintf(temp, "MOTHER_NAME = %s\n", new_value);
            updated = 1;
        } else if (choice == 6 && strstr(line, "DOB = ") == line) {
            fprintf(temp, "DOB = %s\n", new_value);
            updated = 1;
        } else if (choice == 7 && strstr(line, "FAMILY_NAME = ") == line) {
            fprintf(temp, "FAMILY_NAME = %s\n", new_value);
            updated = 1;
        } else if (choice == 8 && strstr(line, "SUBJECT1_GRADE = ") == line) {
            fprintf(temp, "SUBJECT1_GRADE = %.2f\n", float_value);
            updated = 1;
        } else if (choice == 9 && strstr(line, "SUBJECT2_GRADE = ") == line) {
            fprintf(temp, "SUBJECT2_GRADE = %.2f\n", float_value);
            updated = 1;
        } else if (choice == 10 && strstr(line, "SUBJECT3_GRADE = ") == line) {
            fprintf(temp, "SUBJECT3_GRADE = %.2f\n", float_value);
            updated = 1;
        } else if (choice == 11 && strstr(line, "SUBJECT4_GRADE = ") == line) {
            fprintf(temp, "SUBJECT4_GRADE = %.2f\n", float_value);
            updated = 1;
        } else {
            // Copy unchanged lines as-is
            fputs(line, temp);
        }
    }

    fclose(original);
    fclose(temp);

    // SECTION 7: Verify update and replace original file
    if (!updated) {
        printf("Warning: target field not found. No changes made.\n");
        remove(tempname);  // Clean up temporary file
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    // Replace original file with updated temporary file
    if (remove(filename) != 0) {
        perror("remove");
        pthread_mutex_unlock(&file_mutex);
        return;
    }
    if (rename(tempname, filename) != 0) {
        perror("rename");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    // Second pass: refresh derived average when a subject grade changes
    if (choice == 8 || choice == 9 || choice == 10 || choice == 11) {
        recompute_average_grade(filename);
    }

    // Unlock mutex after file operations complete
    pthread_mutex_unlock(&file_mutex);

    printf("✓ Student %d updated successfully.\n\n", id);
}

/*
 * FUNCTION: main
 * ===============
 * Entry point of the Student Management System
 * Threads are supported only to guard against races via mutex; this menu runs inline.
 */
#ifndef UNIT_TEST
int main(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║  SCHOOL STUDENT MANAGEMENT SYSTEM                 ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n");
    printf("\n");

    int running = 1;
    while (running) {
        int choice = 0;
        int ch;
        printf("Menu:\n");
        printf("1. Add student\n");
        printf("2. Edit student\n");
        printf("3. Reset system (wipe student files, reset IDs)\n");
        printf("4. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }
        while ((ch = getchar()) != '\n' && ch != EOF) { }

        switch (choice) {
            case 1:
                add_student();
                break;
            case 2:
                edit_student();
                break;
            case 3:
                reset_system();
                break;
            case 4:
                running = 0;
                break;
            default:
                printf("Invalid menu selection.\n");
                break;
        }
    }

    return 0;
}
#endif
