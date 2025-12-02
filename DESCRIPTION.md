# Student Manager (C, file-based)

This is a simple student management tool written in C. It stores each student record as a text file under `data/students/`, and uses a counter file `data/next_id.txt` to issue sequential IDs.

## What it does
- Add students: prompts for names (no spaces supported), family name, contact info, grade, four subject names/grades; writes a text file per student.
- Edit students: previews the current file, then lets you edit name, family name, phone, parents, DOB, grade, or any of the four subject grades. Subject edits automatically recompute `AVERAGE_GRADE`.
- Reset: wipes all student files and resets the ID counter to 1.

## Concurrency model
- Uses `pthread_mutex_t file_mutex` to serialize file writes so concurrent callers do not race. The menu itself runs inline; threading is present only to guard critical sections if multiple threads invoke these functions elsewhere.

## Intentional limitations
- Names, family names, and subject names are read with `%s`, so no spaces.
- Input validation is minimal: numeric checks for grade/subject grades; other fields are basic strings.
- Data is plain text; there is no database or binary format.
