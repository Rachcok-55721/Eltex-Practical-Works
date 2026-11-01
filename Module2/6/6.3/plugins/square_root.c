#include <math.h>
#include "../plugin_api.h"

// Математическая операция
static double calculate(double a, double b) {
    (void)b;
    return sqrt(a);
}

// Описание операции для калькулятора
PLUGIN_EXPORT const PluginInfo *getPluginInfo(void) {
    static const PluginInfo info = {"Square root", 1, calculate};
    return &info;
}
