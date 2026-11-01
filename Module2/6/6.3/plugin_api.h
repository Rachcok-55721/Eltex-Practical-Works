#ifndef PLUGIN_API_H
#define PLUGIN_API_H

typedef double (*Operation)(double, double);
typedef struct {
    const char *name;
    int arguments;
    Operation function;
} PluginInfo;

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT
#endif

typedef const PluginInfo *(*GetPluginInfo)(void);
#endif
