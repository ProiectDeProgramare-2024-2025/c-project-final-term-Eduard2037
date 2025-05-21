#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

#define MAX_CLASSES 10
#define MAX_STUDENTS 50
#define MAX_SUBJECTS 10
#define NAME_LEN 50
#define FILENAME "gradebook.txt"

// ANSI color codes for terminal
#define COLOR_RESET   "\x1b[0m"
#define COLOR_CLASS   "\x1b[1;34m"  // Bold Blue
#define COLOR_STUDENT "\x1b[1;32m"  // Bold Green
#define COLOR_SUBJECT "\x1b[1;33m"  // Bold Yellow
#define COLOR_GRADE   "\x1b[1;35m"  // Bold Magenta
#define COLOR_ERROR   "\x1b[1;31m"  // Bold Red
#define COLOR_INPUT   "\x1b[1;36m"  // Bold Cyan

typedef struct {
    char name[NAME_LEN];
    float grade;
} Subject;

typedef struct {
    char name[NAME_LEN];
    Subject subjects[MAX_SUBJECTS];
    int subjectCount;
} Student;

typedef struct {
    char name[NAME_LEN];
    Student students[MAX_STUDENTS];
    int studentCount;
} Class;

Class classes[MAX_CLASSES];
int classCount = 0;

// Clear screen
void clearScreen() {
    system(CLEAR);
}

// ======================
// File I/O functions
// ======================
void loadFromFile() {
    FILE *file = fopen(FILENAME, "r");
    if (!file) return;

    fscanf(file, "%d\n", &classCount);
    for (int i = 0; i < classCount; i++) {
        fscanf(file, "%49s %d\n", classes[i].name, &classes[i].studentCount);
        for (int j = 0; j < classes[i].studentCount; j++) {
            fscanf(file, "%49s %d\n", classes[i].students[j].name, &classes[i].students[j].subjectCount);
            for (int k = 0; k < classes[i].students[j].subjectCount; k++) {
                fscanf(file, "%49s %f\n", classes[i].students[j].subjects[k].name, &classes[i].students[j].subjects[k].grade);
            }
        }
    }

    fclose(file);
}

void saveToFile() {
    FILE *file = fopen(FILENAME, "w");
    if (!file) {
        printf(COLOR_ERROR "Error: Could not save to file %s\n" COLOR_RESET, FILENAME);
        return;
    }

    fprintf(file, "%d\n", classCount);
    for (int i = 0; i < classCount; i++) {
        fprintf(file, "%s %d\n", classes[i].name, classes[i].studentCount);
        for (int j = 0; j < classes[i].studentCount; j++) {
            fprintf(file, "%s %d\n", classes[i].students[j].name, classes[i].students[j].subjectCount);
            for (int k = 0; k < classes[i].students[j].subjectCount; k++) {
                fprintf(file, "%s %.2f\n", classes[i].students[j].subjects[k].name, classes[i].students[j].subjects[k].grade);
            }
        }
    }

    fclose(file);
}

// ======================
// Utility: Input validation & reading
// ======================
void readString(const char* prompt, char* buffer, int length) {
    while (1) {
        printf(COLOR_INPUT "%s" COLOR_RESET, prompt);
        if (fgets(buffer, length, stdin)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0'; // Remove trailing newline
            if (strlen(buffer) > 0) break;
        }
        printf(COLOR_ERROR "Input cannot be empty. Please try again.\n" COLOR_RESET);
    }
}

// Read float with validation and range check 0-100 for grades
float readGrade(const char* prompt) {
    char line[100];
    float grade;
    while (1) {
        printf(COLOR_INPUT "%s" COLOR_RESET, prompt);
        if (!fgets(line, sizeof(line), stdin)) {
            printf(COLOR_ERROR "Invalid input. Please try again.\n" COLOR_RESET);
            continue;
        }
        if (sscanf(line, "%f", &grade) != 1) {
            printf(COLOR_ERROR "Input is not a valid number. Try again.\n" COLOR_RESET);
            continue;
        }
        if (grade < 0.0 || grade > 100.0) {
            printf(COLOR_ERROR "Grade must be between 0 and 100. Try again.\n" COLOR_RESET);
            continue;
        }
        break;
    }
    return grade;
}

// ======================
// Find or add helpers
// ======================
int findClassIndex(const char* className) {
    for (int i = 0; i < classCount; i++) {
        if (strcmp(classes[i].name, className) == 0) return i;
    }
    return -1;
}

int addClass(const char* className) {
    if (classCount >= MAX_CLASSES) return -1;
    strcpy(classes[classCount].name, className);
    classes[classCount].studentCount = 0;
    return classCount++;
}

int findStudentIndex(Class* cls, const char* studentName) {
    for (int i = 0; i < cls->studentCount; i++) {
        if (strcmp(cls->students[i].name, studentName) == 0) return i;
    }
    return -1;
}

int addStudent(Class* cls, const char* studentName) {
    if (cls->studentCount >= MAX_STUDENTS) return -1;
    strcpy(cls->students[cls->studentCount].name, studentName);
    cls->students[cls->studentCount].subjectCount = 0;
    return cls->studentCount++;
}

int findSubjectIndex(Student* student, const char* subjectName) {
    for (int i = 0; i < student->subjectCount; i++) {
        if (strcmp(student->subjects[i].name, subjectName) == 0) return i;
    }
    return -1;
}

int addSubject(Student* student, const char* subjectName, float grade) {
    if (student->subjectCount >= MAX_SUBJECTS) return -1;
    strcpy(student->subjects[student->subjectCount].name, subjectName);
    student->subjects[student->subjectCount].grade = grade;
    return student->subjectCount++;
}

// ======================
// Core functionalities
// ======================
void addGrade() {
    clearScreen();
    printf("==== %s ====\n", "Add Grade");

    char className[NAME_LEN];
    char studentName[NAME_LEN];
    char subjectName[NAME_LEN];
    float grade;

    readString("Enter class name (e.g. CS101): ", className, NAME_LEN);
    readString("Enter student name (e.g. JohnDoe): ", studentName, NAME_LEN);
    readString("Enter subject name (e.g. Math): ", subjectName, NAME_LEN);
    grade = readGrade("Enter grade (0.0 - 100.0): ");

    int ci = findClassIndex(className);
    if (ci == -1) {
        ci = addClass(className);
        if (ci == -1) {
            printf(COLOR_ERROR "Maximum number of classes reached. Cannot add more.\n" COLOR_RESET);
            printf("Press Enter to continue...");
            getchar();
            return;
        }
    }

    Class* cls = &classes[ci];
    int si = findStudentIndex(cls, studentName);
    if (si == -1) {
        si = addStudent(cls, studentName);
        if (si == -1) {
            printf(COLOR_ERROR "Maximum number of students reached in this class. Cannot add more.\n" COLOR_RESET);
            printf("Press Enter to continue...");
            getchar();
            return;
        }
    }

    Student* student = &cls->students[si];
    int subjIndex = findSubjectIndex(student, subjectName);
    if (subjIndex != -1) {
        // Subject exists - ask to modify instead
        printf(COLOR_ERROR "Subject %s already exists for student %s.\n" COLOR_RESET,
            subjectName, studentName);
        printf("Use 'Modify Grade' option to change the grade.\n");
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    if (student->subjectCount >= MAX_SUBJECTS) {
        printf(COLOR_ERROR "Maximum number of subjects reached for this student.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    addSubject(student, subjectName, grade);

    printf(COLOR_CLASS "Class: %s\n" COLOR_RESET, className);
    printf(COLOR_STUDENT "Student: %s\n" COLOR_RESET, studentName);
    printf(COLOR_SUBJECT "Subject: %s\n" COLOR_RESET, subjectName);
    printf(COLOR_GRADE "Grade: %.2f\n" COLOR_RESET, grade);

    printf("Grade added successfully!\n");
    printf("Press Enter to return to menu...");
    getchar();
}

void deleteGrade() {
    clearScreen();
    printf("==== %s ====\n", "Delete Grade");

    char className[NAME_LEN];
    char studentName[NAME_LEN];
    char subjectName[NAME_LEN];

    readString("Enter class name to delete from: ", className, NAME_LEN);
    int ci = findClassIndex(className);
    if (ci == -1) {
        printf(COLOR_ERROR "Class not found.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    Class* cls = &classes[ci];

    readString("Enter student name: ", studentName, NAME_LEN);
    int si = findStudentIndex(cls, studentName);
    if (si == -1) {
        printf(COLOR_ERROR "Student not found.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    Student* student = &cls->students[si];

    readString("Enter subject name to delete: ", subjectName, NAME_LEN);
    int subjIndex = findSubjectIndex(student, subjectName);
    if (subjIndex == -1) {
        printf(COLOR_ERROR "Subject not found for this student.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    // Delete subject by shifting left
    for (int i = subjIndex; i < student->subjectCount - 1; i++) {
        student->subjects[i] = student->subjects[i+1];
    }
    student->subjectCount--;

    printf(COLOR_CLASS "Class: %s\n" COLOR_RESET, className);
    printf(COLOR_STUDENT "Student: %s\n" COLOR_RESET, studentName);
    printf(COLOR_SUBJECT "Subject %s deleted.\n" COLOR_RESET, subjectName);

    printf("Grade deleted successfully!\n");
    printf("Press Enter to return to menu...");
    getchar();
}

void modifyGrade() {
    clearScreen();
    printf("==== %s ====\n", "Modify Grade");

    char className[NAME_LEN];
    char studentName[NAME_LEN];
    char subjectName[NAME_LEN];
    float newGrade;

    readString("Enter class name: ", className, NAME_LEN);
    int ci = findClassIndex(className);
    if (ci == -1) {
        printf(COLOR_ERROR "Class not found.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    Class* cls = &classes[ci];

    readString("Enter student name: ", studentName, NAME_LEN);
    int si = findStudentIndex(cls, studentName);
    if (si == -1) {
        printf(COLOR_ERROR "Student not found.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }
    Student* student = &cls->students[si];

    readString("Enter subject name to modify: ", subjectName, NAME_LEN);
    int subjIndex = findSubjectIndex(student, subjectName);
    if (subjIndex == -1) {
        printf(COLOR_ERROR "Subject not found for this student.\n" COLOR_RESET);
        printf("Press Enter to continue...");
        getchar();
        return;
    }

    newGrade = readGrade("Enter new grade (0.0 - 100.0): ");

    student->subjects[subjIndex].grade = newGrade;

    printf(COLOR_CLASS "Class: %s\n" COLOR_RESET, className);
    printf(COLOR_STUDENT "Student: %s\n" COLOR_RESET, studentName);
    printf(COLOR_SUBJECT "Subject: %s\n" COLOR_RESET, subjectName);
    printf(COLOR_GRADE "New Grade: %.2f\n" COLOR_RESET, newGrade);

    printf("Grade modified successfully!\n");
    printf("Press Enter to return to menu...");
    getchar();
}

void showMainMenu() {
    clearScreen();
    printf("===== Gradebook System Menu =====\n");
    printf("1. Add Grade\n");
    printf("2. Delete Grade\n");
    printf("3. Modify Grade\n");
    printf("4. Exit\n");
    printf("Choose an option (1-4): ");
}

// ======================
// Main
// ======================
int main() {
    loadFromFile();

#ifdef _WIN32
    // Enable ANSI escape sequences on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif

    int option;
    char input[10];

    while (1) {
        showMainMenu();

        if (!fgets(input, sizeof(input), stdin)) {
            printf(COLOR_ERROR "Input error.\n" COLOR_RESET);
            continue;
        }
        if (sscanf(input, "%d", &option) != 1) {
            printf(COLOR_ERROR "Invalid option. Please enter a number (1-4).\n" COLOR_RESET);
            printf("Press Enter to continue...");
            getchar();
            continue;
        }

        switch (option) {
            case 1: addGrade(); break;
            case 2: deleteGrade(); break;
            case 3: modifyGrade(); break;
            case 4:
                saveToFile();
                clearScreen();
                printf("Exiting... Goodbye!\n");
                return 0;
            default:
                printf(COLOR_ERROR "Invalid option. Please enter a number between 1 and 4.\n" COLOR_RESET);
                printf("Press Enter to continue...");
                getchar();
        }
    }
    return 0;
}
