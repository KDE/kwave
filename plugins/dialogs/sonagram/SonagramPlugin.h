#ifndef _SONAGRAM_PLUGIN_H_
#define _SONAGRAM_PLUGIN_H_

#include <libgui/KwavePlugin.h>

class PluginContext;
class QStrList;
class SonagramWindow;

class SonagramPlugin: public KwavePlugin
{
    Q_OBJECT
public:
    /** Constructor */
    SonagramPlugin(PluginContext *c);
    virtual QStrList *setup(QStrList *previous_params);
    virtual int start(QStrList &params);

public slots:
    virtual void close();

private:
    SonagramWindow *sonagram_window;

};

#endif /* _SONAGRAM_PLUGIN_H_ */

/* end of module.h */
