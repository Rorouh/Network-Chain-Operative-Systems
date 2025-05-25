// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include "../inc/clog.h"

void save_operation(char* operation, char* log_filename) {
    FILE* log_file = fopen(log_filename, "a");
    if(log_file == NULL) {
        perror("Error ao abrir o ficheiro de log");
        exit(EXIT_FAILURE);
    }
    struct timespec t;
    save_time(&t);
    struct tm time = format_time(t);

    fprintf(log_file,
            "%04d%02d%02d %02d:%02d:%02d.%03ld %s\n",
            time.tm_year + 1900,
            time.tm_mon  + 1,
            time.tm_mday,
            time.tm_hour,
            time.tm_min,
            time.tm_sec,
            t.tv_nsec/1000000,
            operation
        );
    fclose(log_file);
}