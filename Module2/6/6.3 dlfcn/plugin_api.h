#ifndef PLUGIN_API_H
#define PLUGIN_API_H

typedef double (*Operation)(double, double);

typedef struct {
    const char *name;
    int arguments;
    Operation function;
} PluginInfo;

/* On Linux, globally visible functions in .so are exported by default. */
#define PLUGIN_EXPORT

typedef const PluginInfo *(*GetPluginInfo)(void);

#endif
