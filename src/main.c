#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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

void calculate_average(student_t *student) {
    student->average_grade = (student->subject1.grade + student->subject2.grade +
                              student->subject3.grade + student->subject4.grade) / 4.0f;
}

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



void add_student() {
    // Function to add a new student
    student_t student;
    printf("Enter student name: \n");
    scanf("%s", &student.name);
    printf("Enter date of birth (DD/MM/YYYY): \n");
    scanf("%s", &student.dateofbirth);

    char filename[256];
    snprintf(filename, sizeof(filename), "%s_%s.txt", student.name, student.dateofbirth);
    FILE *file = fopen(filename, "a");

    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    fprintf(file, "Name: %s\n", student.name);
    fprintf(file, "Date of Birth: %s\n", student.dateofbirth);
    printf("Enter student ID: \n");
    scanf("%s", &student.studentid);
    fprintf(file, "Student ID: %s\n", student.studentid);
    printf("Enter student grade: \n");
    scanf("%d", &student.grade);
    fprintf(file, "Grade: %d\n", student.grade);
    printf("Enter father's name: \n");
    scanf("%s", &student.father_name);
    fprintf(file, "Father's Name: %s\n", student.father_name);
    printf("Enter mother's name: \n");
    scanf("%s", &student.mother_name);
    fprintf(file, "Mother's Name: %s\n", student.mother_name);
    printf("Enter phone number: \n");
    scanf("%s", &student.phone_number);
    fprintf(file, "Phone Number: %s\n", student.phone_number);
    printf("Enter subject 1 name: \n");
    scanf("%s", &student.subject1.name);
    fprintf(file, "Subject 1 Name: %s\n", student.subject1.name);
    printf("Enter subject 1 grade: \n");
    scanf("%f", &student.subject1.grade);
    fprintf(file, "Subject 1 Grade: %.2f\n", student.subject1.grade);
    printf("Enter subject 2 name: \n");
    scanf("%s", &student.subject2.name);
    fprintf(file, "Subject 2 Name: %s\n", student.subject2.name);
    printf("Enter subject 2 grade: \n");
    scanf("%f", &student.subject2.grade);
    fprintf(file, "Subject 2 Grade: %.2f\n", student.subject2.grade);
    printf("Enter subject 3 name: \n");
    scanf("%s", &student.subject3.name);
    fprintf(file, "Subject 3 Name: %s\n", student.subject3.name);
    printf("Enter subject 3 grade: \n");
    scanf("%f", &student.subject3.grade);
    fprintf(file, "Subject 3 Grade: %.2f\n", student.subject3.grade);
    printf("Enter subject 4 name: \n");
    scanf("%s", &student.subject4.name);
    fprintf(file, "Subject 4 Name: %s\n", student.subject4.name);
    printf("Enter subject 4 grade: \n");
    scanf("%f", &student.subject4.grade);
    fprintf(file, "Subject 4 Grade: %.2f\n", student.subject4.grade);
    calculate_average(&student);
    fprintf(file, "Average Grade: %.2f\n", student.average_grade);
    printf("Student added successfully. Check data folder\n");
    fclose(file);
}

void edit_student() {
    // Function to edit an existing student
        int student_id;
        printf("What student ID do you want to edit?\n");
        scanf(student_id);
        char filename[100];
        snprintf(filename, sizeof(filename), "output_%d.txt", student_id);
        FILE* original;
        original = fopen(filename, "r");
        if (!original) {
            printf("no file found, please create it");
            return -1;
        }

}
int main() {

    char status;
    student_t student;

    do {
        printf("Do you want to create a new student or edit an existing one? (c/e): ");
        scanf(" %c", &status);
        if (status != 'c' && status != 'e') {
            printf("Invalid input.\n");
        }
    } while (status != 'c' && status != 'e');

    if (status == 'c') {
        add_student();
    }

    if (status == 'e') {



    }



}