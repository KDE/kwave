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
    SonagramPlugin(PluginContext &c);

    /** Destructor */
    virtual ~SonagramPlugin();
    virtual QStrList *setup(QStrList *previous_params);
    virtual int start(QStrList &params);

private slots:

    /**
     * Connected to the SonagramWindow's "destroyed()" signal.
     * @see #sonagram_window
     */
    void windowClosed();

private:
    /** the main view of the plugin, a SonagramWindow */
    SonagramWindow *sonagram_window;

};

#endif /* _SONAGRAM_PLUGIN_H_ */

/* end of module.h */
