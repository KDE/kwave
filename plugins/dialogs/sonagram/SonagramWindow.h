#ifndef _SONAGRAM_WINDOW_H_
#define _SONAGRAM_WINDOW_H_ 1

#include <ktmainwindow.h>

#include <libkwave/gsl_fft.h>

class KStatusBar;
class QImage;
class OverViewWidget;
class SonagramContainer;
class ScaleWidget;
class CornerPatchWidget;

class ImageView;
class SonagramContainer;

//***********************************************************************
class SonagramWindow : public KTMainWindow
{
    Q_OBJECT
public:
    SonagramWindow(const QString &);
    ~SonagramWindow();


    /**
     * Sets a new signal and displays it.
     * @param input array of samples in float format, normed
     *              between [-1.0 and 1.0]
     * @param size number of samples in the input array
     * @points
     * @windowtype selects a window function for fft
     * @rate sample rate in samples per seconds
     */
    void setSignal(double *input, int size, int points,
	           int windowtype, int rate);

public slots:

    void close();
    void save();
    void load();
    void toSignal();
    void setName(const QString &name);
    void setInfo(double, double);
    void setRange(int, int, int);

signals:

protected:
    void createImage();
    void createPalette();
    // void     resizeEvent     (QResizeEvent *);

private:
    KStatusBar *status;
    QImage *image;
    ImageView *view;
    OverViewWidget *overview;
    SonagramContainer *mainwidget;
    ScaleWidget *xscale, *yscale;
    CornerPatchWidget *corner;
    int points;
    int rate;
    int length;
    int image_width;
    int y;
    complex **data;
    double max;
};

#endif // _SONOGRAM_WINDOW_H_
