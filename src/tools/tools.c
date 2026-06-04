#include "tools.h"

int contains_int(const int *arr, int len, int elem) {
    int i;
    for (i = 0; i < len; i++) {
        if (arr[i] == elem) return 1;
    }
    return 0;
}