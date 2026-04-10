#ifndef CALIBRATIONWINDOW_KINECTCALIBRATIONWINDOW_INCLUDED
#define CALIBRATIONWINDOW_KINECTCALIBRATIONWINDOW_INCLUDED

#include <X11/Xlib.h>
#include <string>

/* Forward declaration so we can hold a pointer without a circular include */
class RawKinectViewer;

class KinectCalibrationWindow
	{
	private:
	struct Rect
		{
		int x;
		int y;
		int w;
		int h;
		bool contains(int px,int py) const
			{
			return px>=x&&px<x+w&&py>=y&&py<y+h;
			}
		};
	
	static const int windowWidth=1920;
	static const int windowHeight=1080;
	
	Display* display;
	Window window;
	GC graphicsContext;
	Atom wmDeleteWindow;
	Cursor arrowCursor;
	Cursor handCursor;
	bool closeRequested;
	
	/* Pointer back to the owning application for triggering actions */
	RawKinectViewer* viewer;
	
	/* Button rects */
	Rect averageFramesButtonRect;
	Rect extractPlanesButtonRect;
	Rect measure3DButtonRect;
	Rect finishCalibrationButtonRect;
	
	/* Info icon rects */
	Rect averageFramesInfoIconRect;
	Rect extractPlanesInfoIconRect;
	Rect measure3DInfoIconRect;
	
	int hoverX;
	int hoverY;
	
	/* Colors */
	unsigned long colorBackground;
	unsigned long colorPanel;
	unsigned long colorBorder;
	unsigned long colorText;
	unsigned long colorSubtleText;
	unsigned long colorButton;
	unsigned long colorButtonHover;
	unsigned long colorButtonActive;
	unsigned long colorFinish;
	unsigned long colorFinishHover;
	unsigned long colorAccent;
	unsigned long colorSuccess;
	unsigned long colorWarning;
	unsigned long colorError;
	
	XFontStruct* titleFont;
	XFontStruct* headingFont;
	XFontStruct* bodyFont;
	
	unsigned long allocColor(const char* name,unsigned long fallback) const;
	void setColor(unsigned long color);
	void drawButton(const Rect& rect,const char* label,bool hovered,unsigned long baseColor);
	void drawTextLine(int x,int y,const std::string& text);
	void drawInfoIcon(const Rect& rect,bool hovered);
	void drawTooltip(const Rect& anchor,const char* line1,const char* line2,const char* line3);
	void updateCursor(void);
	void draw(void);
	
	public:
	/* Constructor takes a pointer to the owning RawKinectViewer */
	KinectCalibrationWindow(RawKinectViewer* viewer);
	~KinectCalibrationWindow(void);
	
	bool processEvents(void);
	};

#endif
