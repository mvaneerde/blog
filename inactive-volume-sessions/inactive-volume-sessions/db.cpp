// db.cpp

#include "common.h"

float DbFromAmp(float a) {
    return 20.0f * log10f(a);
}
