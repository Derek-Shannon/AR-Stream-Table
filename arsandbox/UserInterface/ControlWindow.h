#ifndef USERINTERFACE_CONTROLWINDOW_INCLUDED
#define USERINTERFACE_CONTROLWINDOW_INCLUDED

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "SensorUtility.h"
#include <string>

class ControlWindow
	{
	private:
	struct Rect
		{
		int x;
		int y;
		int w;
		int h;
		bool contains(int px,int py) const;
		};

	static const int minimizedWidth=1020;
	static const int minimizedHeight=560;
	static const int maximizedWidth=1920;
	static const int maximizedHeight=1080;
	static const int exportNameMaxLength=36;
	static const int testingTiltMin=0;
	static const int testingTiltMax=22;

	static const Rect exitButtonRect;
	static const Rect freezeButtonRect;
	static const Rect exportButtonRect;
	static const Rect drainButtonRect;
	static const Rect contourIntervalLabelRect;
	static const Rect contourIntervalButton075Rect;
	static const Rect contourIntervalButton1Rect;
	static const Rect contourIntervalButton2Rect;
	static const Rect contourIntervalButton4Rect;
	static const Rect testingLabelRect;
	static const Rect testingCheckboxRect;
	static const Rect testingSliderRect;
	static const Rect exportDialogRect;
	static const Rect exportDialogInputRect;
	static const Rect exportDialogCancelRect;
	static const Rect exportDialogOkRect;

	Display* display;
	Window window;
	GC graphicsContext;
	Atom wmDeleteWindow;
	Cursor arrowCursor;
	Cursor handCursor;
	bool closeRequested;

	bool waterSimulationOn;
	bool freezeOn;
	bool exportRequested;
	bool exportInProgress;
	bool isMaximized;
	bool removeWaterOn;
	double contourLineInterval;
	bool hoverInteractive;
	bool exportDialogVisible;
	int hoverX;
	int hoverY;
	double waterFlowRate;
	float appliedAngleValue;
	float sensorAngleValue;
	int testingTiltValue;
	bool testingEnabled;
	bool testingSliderDragging;
    int currentFps;
	SensorUtility* arduinoSensor;


    XFontStruct* titleFont;
    XFontStruct* sectionFont;
    XFontStruct* statFont;
    XFontStruct* titleFontLarge;
    XFontStruct* sectionFontLarge;
    XFontStruct* statFontLarge;

	unsigned long colorBackground;
	unsigned long colorPanel;
	unsigned long colorBorder;
	unsigned long colorButton;
	unsigned long colorButtonActive;
	unsigned long colorButtonHover;
	unsigned long colorButtonBorder;
	unsigned long colorText;
	unsigned long colorSubtleText;
	unsigned long colorAccent;
	unsigned long colorSuccess;
	unsigned long colorError;
	unsigned long colorOverlay;
	unsigned long colorInputBackground;
	unsigned long colorOkButton;
	unsigned long colorOkButtonHover;

	std::string exportRequestName;
	std::string exportNameInput;
	std::string exportDialogErrorMessage;
	std::string statusMessage;

	unsigned long allocColor(const char* name,unsigned long fallback) const;
	void setColor(unsigned long color);
	bool isInteractiveAt(int x,int y) const;
	unsigned long resolveButtonFill(bool active,bool hovered) const;
    void setFont(XFontStruct* font);
    void drawCenteredText(const Rect& rect,int baselineY,const char* label,XFontStruct* font,unsigned long color);
    void drawButton(const Rect& rect,const char* label,bool active=false,bool hovered=false,unsigned long fillColorOverride=0,unsigned long textColorOverride=0);
	void applyWindowState(void);
	void updateCursor(int x,int y);
	void openExportDialog(void);
	void closeExportDialog(void);
	void appendExportInput(char c);
	void eraseExportInput(void);
	void submitExportDialog(void);
	void drawExportDialog(void);
	void draw(void);
	int currentWindowWidth(void) const;
	int currentWindowHeight(void) const;
	XFontStruct* activeTitleFont(void) const;
	XFontStruct* activeSectionFont(void) const;
	XFontStruct* activeStatFont(void) const;
	Rect scaledRect(const Rect& rect) const;

	
	public:
	enum ExportStatus
		{
		EXPORT_IDLE,
		EXPORT_PENDING,
		EXPORT_SUCCESS,
		EXPORT_ERROR
		};

	private:
	ExportStatus exportStatus;

	public:
	ControlWindow(void);
	~ControlWindow(void);

	bool getFreezeState(void) const;
	bool getDrainState(void) const;
	double getContourLineInterval(void) const;
	float getAppliedTiltValue(void) const;
	double getWaterFlowRate(void) const;
	void setFreezeState(bool state);
	void setWaterFlowRate(double newWaterFlowRate);
    void setCurrentFps(int newCurrentFps);
	bool consumeExportRequest(std::string& requestedName);
	void showExportDialogError(const std::string& message,const std::string& attemptedName=std::string());
	void setExportStatus(ExportStatus newStatus,const std::string& message=std::string());
	

	bool processEvents(void);
	};

#endif