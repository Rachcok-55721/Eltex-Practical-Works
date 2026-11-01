#include "../plugin_api.h"

// Математическая операция
static double calculate(double a, double b) {
    return a / b;
}

// Описание операции для калькулятора
PLUGIN_EXPORT const PluginInfo *getPluginInfo(void) {
    static const PluginInfo info = {"Division", 2, calculate};
    return &info;
}
