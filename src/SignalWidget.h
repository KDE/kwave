#ifndef _SIGNAL_WIDGET_H_
#define _SIGNAL_WIDGET_H_ 1

#include <qwidget.h>
#include <qpainter.h>

class MouseMark;
class LabelList;
class LabelType;
class SignalManager;
class ProgressDialog;
class TimeOperation;
class MenuManager;

ProgressDialog *createProgressDialog (TimeOperation *operation,
	const char *caption);

//***********************************************************
class SignalWidget : public QWidget
//this class is mainly responsible for displaying signals in the time-domain
{
 Q_OBJECT
 public:
 	SignalWidget	(QWidget *parent,MenuManager *manage);
 	~SignalWidget	();

 int    mstosamples             (double);
 void 	setSignal		(const char *filename,int type);
 void 	saveSignal		(const char *filename, int bits,
                                 int type, bool selection=false);
 void 	saveBlocks		(int);
 void 	saveSelectedSignal	(const char *filename,int bits,bool selection=true);
 void 	setSignal		(SignalManager *signal);

 /**
  * sets the display offset [samples], does not refresh the screen
  * @param new_offset new value for the offset in samples, will be
  *                   internally limited to [0...length-1]
  */
 void setOffset(int new_offset);

 /**
  * sets a new zoom factor [samples/pixel], does not refresh the screen
  * @param new_zoom new zoom value, will be internally limited
  *                 to [length/width...1/width] (from full display to
  *                 one visible sample only)
  */
 void setZoom(double new_zoom);

 unsigned char   *getOverview   (int);
 int    checkPosition	        (int);
 void 	drawSelection		(int,int);

 void   addLabelType            (LabelType *);
 void   addLabelType            (const char *);
 int	doCommand	        (const char *);
 int    getSignalCount          ();
 int    getBitsPerSample        ();

 public slots:

 void   slot_setOffset(int new_offset);

 void 	refresh		();
 void	setOp	        (int);
 void	toggleChannel	(int);
 void	time		();

    /**
     * Zooms into the selected range between the left and right marker.
     */
    void zoomRange();

    /**
     * Zooms the signal to be fully visible. Equivalent to
     * setZoom(getFullZoom()).
     * @see #setZoom()
     * @see #getFullZoom()
     */
    void zoomAll();

    /**
     * Zooms the signal to one-pixel-per-sample. Equivalent to
     * setZoom(1.0).
     * @see #setZoom()
     * @see #getFullZoom()
     */
    void zoomNormal();

    /**
     * Zooms into the signal, the new display will show the middle
     * 33% of the current display.
     */
    void zoomIn();

    /**
     * Zooms the signal out, the current display will become the
     * middle 30% of the new display.
     */
    void zoomOut();

    /**
     * Returns the zoom value that will be used to fit the whole signal
     * into the current window.
     * @return zoom value [samples/pixel]
     */
    double getFullZoom();

 void   signalinserted  (int,int);
 void   signaldeleted   (int,int);
 void	estimateRange   (int,int);

 signals:

 void channelReset	();
 void playingfinished	();
 void viewInfo		(int,int,int);
 void selectedTimeInfo	(int);
 void timeInfo          (int);
 void rateInfo	        (int);
 void lengthInfo	(int);

    /**
     * Will be emitted if the zoom factor has changed due to a zoom
     * command or resize.
     * @param zoom value [samples/pixel]
     */
    void zoomInfo(double zoom);

 protected:
 void	setRange                (int,int,bool=true);
 void	selectRange		();
 void	updateChannels	        ();

 void	mousePressEvent		(QMouseEvent *);
 void	mouseReleaseEvent	(QMouseEvent *);  
 void	mouseMoveEvent		(QMouseEvent *);  
 void	paintEvent	        (QPaintEvent *);

    /**
     * Draws the signal as an overview with multiple samples per
     * pixel.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @param first the index of the first sample
     * @param last the index of the last sample
     */
    void drawOverviewSignal(int channel, int middle, int height,
	int first, int last);

    /**
     * Draws the signal and interpolates the pixels between the
     * samples. The interpolation is done by using a simple FIR
     * lowpass filter.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @see #calculateInterpolation()
     */
    void drawInterpolatedSignal(int channel,int middle, int height);

    /**
     * Draws the signal and connects the pixels between the samples
     * by using a simple poly-line. This gets used if the current zoom
     * factor is not suitable for either an overview nor an interpolated
     * signal display.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     */
    void drawPolyLineSignal(int channel,int middle, int height);

 void	calcTimeInfo	();
 void   loadLabel       ();
 void   appendLabel     ();
 void   deleteLabel     ();
 void   saveLabel       (const char *);
 void   addLabel        (const char *);
 void   jumptoLabel     ();
 void   markSignal      (const char *);
 void   markPeriods     (const char *);
 void   savePeriods     ();
 void   createSignal    (const char *);
 void   connectSignal   ();
 void   showDialog      (const char *);

 bool   checkForLabelCommand      (const char *);
 bool   checkForNavigationCommand (const char *);

private:

    /**
     * Converts a sample index into a pixel offset using the current zoom
     * value. Always rounds downwards.
     * @param pixels pixel offset
     * @return index of the sample
     */
    int pixels2samples(int pixels);

    /**
     * Converts a pixel offset into a sample index using the current zoom
     * value. Always rounds downwards.
     * @param sample index of the sample
     * @return pixel offset
     */
    int samples2pixels(int samples);

    /**
     * Fixes the zoom and the offset of the display so that no non-existing
     * samples (index < 0 or index >= length) have to be displayed and the
     * current display window of the signal fits into the screen.
     */
    void fixZoomAndOffset();

    /**
     * Calculates the parameters for interpolation of the graphical
     * display when zoomed in. Allocates (new) buffer for the
     * filter coefficients of the low pass filter used for interpolation.
     * @see #interpolation_alpha
     */
    void calculateInterpolation();

    /**
     * order of the low pass filter used for interpolation
     */
    int interpolation_order;

    /**
     * buffer for filter coefficients of the low pass used for
     * interpolation
     * @see #calculateInterpolation()
     */
    float *interpolation_alpha;

 int	offset;                 //offset from which signal is beeing displayed
 int	width,height;		//of this widget
 int	down;       		//flags if mouse is pressed
 double lasty; 
 double zoomy;
 double	zoom;			//number of samples represented by 1
				//vertical line on the screen
 int	playpointer,lastplaypointer;	 
 int	playing;		//flag if playing task is running...
 int	redraw;		        //flag for redrawing pixmap
 MouseMark     *select;
 SignalManager *signalmanage;
 QTimer	       *timer;
 QPainter       p;
 QPixmap       *pixmap;	//pixmap to be blitted to screen
 LabelList     *labels;        //linked list of markers
 LabelType     *markertype;    //selected marker type
 MenuManager   *manage;
};

#endif // _SIGNAL_WIDGET_H_
