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
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Thread synchronization: Ensures only one user can edit a file at a time
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

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
int load_student_data(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    int id = 1;
    if (fscanf(file, "%d", &id) != 1) {
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
    FILE *file = fopen("next_id.tmp", "w");
    if (!file) {
        printf("Error opening file for writing.\n");
        return;
    }
    fprintf(file, "%d\n", new_id);
    fclose(file);

    remove(path);
    rename("next_id.tmp", path);
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
    FILE *id_counter = fopen("data/next_id.txt", "r");
    if (!id_counter) {
        printf("Error opening ID counter file.\n");
        return;
    }
    fscanf(id_counter, "%d", &student.student_id);
    update_next_id("data/next_id.txt", student.student_id + 1);
    fclose(id_counter);

    // STEP 2: Create file for this student (filename = output_[ID].txt)
    char filename[256];
    snprintf(filename, sizeof(filename), "output_%d.txt", student.student_id);
    
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // STEP 3: Collect personal information from user
    printf("\n===== STUDENT PERSONAL INFORMATION =====\n");
    printf("Enter student name: ");
    scanf("%49s", student.name);
    fprintf(file, "NAME = %s\n", student.name);
    
    printf("Enter date of birth (DD/MM/YYYY): ");
    scanf("%10s", student.dateofbirth);
    fprintf(file, "DOB = %s\n", student.dateofbirth);
    
    printf("Enter student ID: ");
    scanf("%14s", student.studentid);
    fprintf(file, "STUDENT_ID = %s\n", student.studentid);
    
    printf("Enter father's name: ");
    scanf("%49s", student.father_name);
    fprintf(file, "FATHER_NAME = %s\n", student.father_name);
    
    printf("Enter mother's name: ");
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
    
    printf("Enter subject 1 name: ");
    scanf("%49s", student.subject1.name);
    fprintf(file, "SUBJECT1_NAME = %s\n", student.subject1.name);
    printf("Enter subject 1 grade: ");
    scanf("%f", &student.subject1.grade);
    fprintf(file, "SUBJECT1_GRADE = %.2f\n", student.subject1.grade);
    
    printf("Enter subject 2 name: ");
    scanf("%49s", student.subject2.name);
    fprintf(file, "SUBJECT2_NAME = %s\n", student.subject2.name);
    printf("Enter subject 2 grade: ");
    scanf("%f", &student.subject2.grade);
    fprintf(file, "SUBJECT2_GRADE = %.2f\n", student.subject2.grade);
    
    printf("Enter subject 3 name: ");
    scanf("%49s", student.subject3.name);
    fprintf(file, "SUBJECT3_NAME = %s\n", student.subject3.name);
    printf("Enter subject 3 grade: ");
    scanf("%f", &student.subject3.grade);
    fprintf(file, "SUBJECT3_GRADE = %.2f\n", student.subject3.grade);
    
    printf("Enter subject 4 name: ");
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
void edit_student(void) {
    // SECTION 1: Get student ID from user
    // Validates input to ensure we have a valid integer ID
    int id;
    printf("What student ID do you want to edit? ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        while (getchar() != '\n') { }
        return;
    }
    while (getchar() != '\n') { }  // clear trailing newline from input buffer

    // Construct filename based on student ID
    char filename[128];
    snprintf(filename, sizeof(filename), "output_%d.txt", id);

    // SECTION 2: Display menu of editable fields
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
    printf("7. Subject 1 Grade\n");
    printf("8. Subject 2 Grade\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1) {
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

    // SECTION 4: Input validation (TODO)
    // Validate new_value based on the choice made
    // Examples: name should be alphabetic, grade should be numeric, etc.
    // If invalid, print error message and return without modifying file

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
            fprintf(temp, "GRADE = %s\n", new_value);
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
        } else if (choice == 7 && strstr(line, "SUBJECT1_GRADE = ") == line) {
            fprintf(temp, "SUBJECT1_GRADE = %s\n", new_value);
            updated = 1;
        } else if (choice == 8 && strstr(line, "SUBJECT2_GRADE = ") == line) {
            fprintf(temp, "SUBJECT2_GRADE = %s\n", new_value);
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

    // Unlock mutex after file operations complete
    pthread_mutex_unlock(&file_mutex);

    printf("✓ Student %d updated successfully.\n\n", id);
}

/*
 * FUNCTION: main
 * ===============
 * Entry point of the Student Management System
 * 
 * Process:
 *   1. Shows menu: Add Student or Edit Student
 *   2. User selects choice
 *   3. Calls appropriate function
 *   4. Returns to menu or exits
 */
int main(void) {
    char status;
    student_t student;

    printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║  SCHOOL STUDENT MANAGEMENT SYSTEM                 ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n");
    printf("\n");

    do {
        printf("Do you want to create a new student or edit an existing one? (c/e): ");
        scanf(" %c", &status);
        if (status != 'c' && status != 'e') {
            printf("Invalid input. Please enter 'c' or 'e'.\n");
        }
    } while (status != 'c' && status != 'e');

    if (status == 'c') {
        add_student();
    }

    if (status == 'e') {
        edit_student();
    }

    return 0;
}