#pragma once
#include<assert.h>
//#define __DEBUG
#ifdef __DEBUG
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...) 
#endif
#include<stdio.h>
void ErrorIf(bool condition, const char* err_msg);