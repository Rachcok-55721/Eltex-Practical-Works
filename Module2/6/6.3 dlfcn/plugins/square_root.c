#include "plugin_api.h"
#include <math.h>

static double operation(double a, double b) {
    (void)b;
    return sqrt(a);
}

PLUGIN_EXPORT const PluginInfo *getPluginInfo(void) {
    static const PluginInfo info = {"Square root", 1, operation};
    return &info;
}
