#ifndef _SONAGRAM_PLUGIN_H_
#define _SONAGRAM_PLUGIN_H_

#include <libgui/KwavePlugin.h>

class PluginContext;
class QStrList;

class SonagramPlugin: public KwavePlugin
{
public:
    /** Constructor */
    SonagramPlugin(PluginContext *c);
    virtual QStrList *setup(QStrList *previous_params);
    virtual int execute(QStrList &params);
};

#endif /* _SONAGRAM_PLUGIN_H_ */

/* end of module.h */
