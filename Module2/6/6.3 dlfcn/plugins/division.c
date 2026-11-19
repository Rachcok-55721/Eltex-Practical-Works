#include "plugin_api.h"
#include <math.h>

static double operation(double a, double b) {
    
    return a / b;
}

PLUGIN_EXPORT const PluginInfo *getPluginInfo(void) {
    static const PluginInfo info = {"Division", 2, operation};
    return &info;
}
