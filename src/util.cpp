#include"util.h"
#include<stdio.h>
#include<stdlib.h>
void ErrorIf(bool condition, const char* err_msg) {
    if (condition == true) {
        perror(err_msg);
        exit(EXIT_FAILURE);
    }
}
