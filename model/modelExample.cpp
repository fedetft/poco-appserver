#include "modelExample.h"

int fact(int n) {
    if(n == 0 || n == 1) {
        return 1;
    }
    return n*fact(n-1);
}
