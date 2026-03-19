#ifndef USERINTERFACE_CONTROLWINDOW_INCLUDED
#define USERINTERFACE_CONTROLWINDOW_INCLUDED

#include <X11/Xlib.h>

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

	static const int windowWidth=1020;
	static const int windowHeight=560;

	static const Rect exitButtonRect;
	static const Rect freezeButtonRect;
	static const Rect exportButtonRect;
	static const Rect unfreezeButtonRect;
	static const Rect removeWaterButtonRect;
	static const Rect applyChangesRect;
	static const Rect waterCheckboxRect;
	static const Rect sliderTrackRect;
	static const Rect sliderApplyRect;

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
	bool unfreezeOn;
	bool removeWaterOn;
	bool draggingAngleSlider;
	bool hoverInteractive;
	int hoverX;
	int hoverY;
	int sliderAngleValue;
	int appliedAngleValue;
    int currentFps;

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

	unsigned long allocColor(const char* name,unsigned long fallback) const;
	void setColor(unsigned long color);
	bool isInteractiveAt(int x,int y) const;
	unsigned long resolveButtonFill(bool active,bool hovered) const;
	void drawButton(const Rect& rect,const char* label,bool active=false,bool hovered=false);
	void drawCheckbox(const Rect& rect,const char* label,bool checked,bool hovered);
	void setAngleFromMouse(int mouseX);
	void updateCursor(int x,int y);
	void draw(void);

	
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
	void setFreezeState(bool state);
    void setCurrentFps(int newCurrentFps);
	bool consumeExportRequest(void);
	void setExportStatus(ExportStatus newStatus);

	bool processEvents(void);
	};

#endif
