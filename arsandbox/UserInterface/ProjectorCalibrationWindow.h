#ifndef USERINTERFACE_PROJECTORCALIBRATIONWINDOW_INCLUDED
#define USERINTERFACE_PROJECTORCALIBRATIONWINDOW_INCLUDED

#include <X11/Xlib.h>
#include <string>

class ProjectorCalibrationWindow
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
	Pixmap backBuffer;
	Drawable drawTarget;
	GC graphicsContext;
	Atom wmDeleteWindow;
	Cursor arrowCursor;
	Cursor handCursor;
	bool closeRequested;
	
	Rect captureButtonRect;
	Rect recaptureBackgroundButtonRect;
	Rect exitButtonRect;
	Rect runStreamTableButtonRect;
	Rect captureInfoIconRect;
	Rect recaptureInfoIconRect;
	Rect calibrationTipsIconRect;
	int hoverX;
	int hoverY;
	
	bool capturePointRequested;
	bool recaptureBackgroundRequested;
	unsigned int tiePointCount;
	
	unsigned long colorBackground;
	unsigned long colorPanel;
	unsigned long colorBorder;
	unsigned long colorText;
	unsigned long colorSubtleText;
	unsigned long colorButton;
	unsigned long colorButtonHover;
	unsigned long colorButtonActive;
	unsigned long colorAccent;
	unsigned long colorSuccess;
	unsigned long colorWarning;
	unsigned long colorError;
	
	XFontStruct* titleFont;
	XFontStruct* headingFont;
	XFontStruct* bodyFont;
	bool captureButtonFlash;
	bool recaptureButtonFlash;
	
	unsigned long allocColor(const char* name,unsigned long fallback) const;
	void setColor(unsigned long color);
	void drawButton(const Rect& rect,const char* label,bool hovered,bool active);
	void drawTextLine(int x,int y,const std::string& text);
	void drawInfoIcon(const Rect& rect,bool hovered);
	void drawTooltip(const Rect& anchor,const char* line1,const char* line2,const char* line3);
	void drawTipsTooltip(const Rect& anchor);
	void updateCursor(void);
	void draw(void);
	
	public:
	ProjectorCalibrationWindow(void);
	~ProjectorCalibrationWindow(void);
	
	bool processEvents(void);
	bool consumeCapturePointRequest(void);
	bool consumeRecaptureBackgroundRequest(void);
	void setTiePointCount(unsigned int newTiePointCount);
	
	};

#endif