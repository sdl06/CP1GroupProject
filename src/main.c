#include <stdio.h>
#include <time.h>

typedef struct {
    char name[50];
    float grade;
} subject_t;

typedef struct {
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

int main() {

    char status;
    student_t student;

    do {
        printf("Do you want to create a new student or edit an existing one? (c/e): ");
        scanf(" %c", &status);
    } while (status != 'c' && status != 'e');

    if (status == 'c') {
        
    }


    char filename[100];
    snprintf(filename, sizeof(filename), "%s_%d.txt", name, grade);

    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1; 
    }
}