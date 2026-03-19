#include "ControlWindow.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdexcept>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

bool ControlWindow::Rect::contains(int px,int py) const
	{
	return px>=x&&px<x+w&&py>=y&&py<y+h;
	}

unsigned long ControlWindow::allocColor(const char* name,unsigned long fallback) const
	{
	XColor color;
	XColor exact;
	Colormap colormap=DefaultColormap(display,DefaultScreen(display));
	if(XAllocNamedColor(display,colormap,name,&color,&exact)!=0)
		return color.pixel;
	return fallback;
	}

void ControlWindow::setColor(unsigned long color)
	{
	XSetForeground(display,graphicsContext,color);
	}

bool ControlWindow::isInteractiveAt(int x,int y) const
	{
	return exitButtonRect.contains(x,y)||freezeButtonRect.contains(x,y)||exportButtonRect.contains(x,y)||
	       unfreezeButtonRect.contains(x,y)||removeWaterButtonRect.contains(x,y)||
	       sliderTrackRect.contains(x,y)||sliderApplyRect.contains(x,y);
	}

unsigned long ControlWindow::resolveButtonFill(bool active,bool hovered) const
	{
	if(active)
		return colorButtonActive;
	if(hovered)
		return colorButtonHover;
	return colorButton;
	}

void ControlWindow::drawButton(const Rect& rect,const char* label,bool active,bool hovered)
	{
	setColor(resolveButtonFill(active,hovered));
	XFillRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	setColor(colorButtonBorder);
	XDrawRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	setColor(colorText);
	XDrawString(display,window,graphicsContext,rect.x+14,rect.y+rect.h/2+5,label,int(strlen(label)));
	}

void ControlWindow::setAngleFromMouse(int mouseX)
	{
	int minX=sliderTrackRect.x;
	int maxX=sliderTrackRect.x+sliderTrackRect.w;
	if(mouseX<minX)
		mouseX=minX;
	if(mouseX>maxX)
		mouseX=maxX;
	double t=double(mouseX-minX)/double(sliderTrackRect.w);
	int newValue=int(t*22.0+0.5);
	if(newValue<0)
		newValue=0;
	if(newValue>22)
		newValue=22;
	sliderAngleValue=newValue;
	}

void ControlWindow::updateCursor(int x,int y)
	{
	hoverX=x;
	hoverY=y;
	bool interactive=isInteractiveAt(x,y);
	if(interactive!=hoverInteractive)
		{
		hoverInteractive=interactive;
		XDefineCursor(display,window,hoverInteractive?handCursor:arrowCursor);
		}
	}

void ControlWindow::draw(void)
	{
	setColor(colorBackground);
	XFillRectangle(display,window,graphicsContext,0,0,windowWidth,windowHeight);

	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,0,0,windowWidth,34);
	setColor(colorBorder);
	XDrawLine(display,window,graphicsContext,0,34,windowWidth,34);

	setColor(colorSubtleText);
	setColor(colorText);
	XDrawString(display,window,graphicsContext,10,22,"Control Window V1",17);

	setColor(colorSubtleText);
	XDrawString(display,window,graphicsContext,10,62,"Control for Stream Table",24);

	setColor(colorText);
	XDrawString(display,window,graphicsContext,10,102,"Table View: ",11);

	XDrawString(display,window,graphicsContext,10,128,"Real World View",15);

	drawButton(removeWaterButtonRect,"Drain Simulated Water",false,removeWaterButtonRect.contains(hoverX,hoverY));

	/* Split layout in two panes */
	setColor(WhitePixel(display,DefaultScreen(display)));
	XDrawLine(display,window,graphicsContext,windowWidth/2,46,windowWidth/2,windowHeight-16);

	drawButton(exitButtonRect,"Exit",false,exitButtonRect.contains(hoverX,hoverY));
	setColor(colorText);
	char fpsBuffer[48];
	snprintf(fpsBuffer,sizeof(fpsBuffer),"FPS: %d", (int)currentFps);
	XDrawString(display,window,graphicsContext,760,88,fpsBuffer,int(strlen(fpsBuffer)));
	char angleBuffer[48];
	snprintf(angleBuffer,sizeof(angleBuffer),"Current Angle: %d",appliedAngleValue);
	XDrawString(display,window,graphicsContext,850,88,angleBuffer,int(strlen(angleBuffer)));

	drawButton(freezeButtonRect,"Freeze Topography",freezeOn,freezeButtonRect.contains(hoverX,hoverY));
	drawButton(exportButtonRect,"Export Topography",exportInProgress,exportButtonRect.contains(hoverX,hoverY));
	if(exportStatus==EXPORT_SUCCESS)
		{
		setColor(colorSuccess);
		const char* exportMessage="Screenshot saved";
		XDrawString(display,window,graphicsContext,774,176,exportMessage,int(strlen(exportMessage)));
		}
	else if(exportStatus==EXPORT_ERROR)
		{
		setColor(colorError);
		const char* exportMessage="Error: Please Try Again";
		XDrawString(display,window,graphicsContext,774,176,exportMessage,int(strlen(exportMessage)));
		}
	drawButton(unfreezeButtonRect,"Unfreeze Topography",unfreezeOn,unfreezeButtonRect.contains(hoverX,hoverY));

	setColor(colorText);
	XDrawString(display,window,graphicsContext,540,270,"Table Tilt Slider (0 - 22 degrees)",34);
	char selectedBuffer[48];
	snprintf(selectedBuffer,sizeof(selectedBuffer),"Angle Selected: %d",sliderAngleValue);
	XDrawString(display,window,graphicsContext,760,270,selectedBuffer,int(strlen(selectedBuffer)));

	setColor(colorBorder);
	XFillRectangle(display,window,graphicsContext,sliderTrackRect.x,sliderTrackRect.y+sliderTrackRect.h/2-2,sliderTrackRect.w,4);
	int knobX=sliderTrackRect.x+(sliderAngleValue*sliderTrackRect.w)/22;
	setColor(colorAccent);
	XFillRectangle(display,window,graphicsContext,sliderTrackRect.x,sliderTrackRect.y+sliderTrackRect.h/2-2,knobX-sliderTrackRect.x,4);
	setColor(colorText);
	XFillRectangle(display,window,graphicsContext,knobX-6,sliderTrackRect.y+sliderTrackRect.h/2-9,12,18);

	drawButton(sliderApplyRect,"Apply",false,sliderApplyRect.contains(hoverX,hoverY));

	setColor(colorSubtleText);
	XDrawString(display,window,graphicsContext,986,24,"_  []  X",9);

	XFlush(display);
	}

ControlWindow::ControlWindow(void)
	:display(0),window(0),graphicsContext(0),wmDeleteWindow(None),arrowCursor(None),handCursor(None),closeRequested(false),
	 waterSimulationOn(false),freezeOn(false),exportRequested(false),exportInProgress(false),unfreezeOn(true),removeWaterOn(false),draggingAngleSlider(false),hoverInteractive(false),hoverX(0),hoverY(0),
	 sliderAngleValue(0),appliedAngleValue(0), currentFps(0),
	 colorBackground(0),colorPanel(0),colorBorder(0),colorButton(0),
	 colorButtonActive(0),colorButtonHover(0),colorButtonBorder(0),colorText(0),colorSubtleText(0),colorAccent(0),colorSuccess(0),colorError(0),
	 exportStatus(EXPORT_IDLE)
	{
	display=XOpenDisplay(0);
	if(display==0)
		throw std::runtime_error("Unable to open X11 display for control window");

	const int screen=DefaultScreen(display);
	window=XCreateSimpleWindow(display,RootWindow(display,screen),80,80,windowWidth,windowHeight,1,BlackPixel(display,screen),BlackPixel(display,screen));
	XStoreName(display,window,"GUI for Stream Table Control");
	XSelectInput(display,window,ExposureMask|StructureNotifyMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|EnterWindowMask|LeaveWindowMask);

	wmDeleteWindow=XInternAtom(display,"WM_DELETE_WINDOW",False);
	XSetWMProtocols(display,window,&wmDeleteWindow,1);

	graphicsContext=XCreateGC(display,window,0,0);
	arrowCursor=XCreateFontCursor(display,XC_left_ptr);
	handCursor=XCreateFontCursor(display,XC_hand2);
	XDefineCursor(display,window,arrowCursor);

	colorBackground=allocColor("#1a1b1f",BlackPixel(display,screen));
	colorPanel=allocColor("#202227",BlackPixel(display,screen));
	colorBorder=allocColor("#3a3f47",WhitePixel(display,screen));
	colorButton=allocColor("#323845",BlackPixel(display,screen));
	colorButtonActive=allocColor("#5f7dd8",WhitePixel(display,screen));
	colorButtonHover=allocColor("#4a5a83",WhitePixel(display,screen));
	colorButtonBorder=allocColor("#53627f",WhitePixel(display,screen));
	colorText=allocColor("#f2f4f8",WhitePixel(display,screen));
	colorSubtleText=allocColor("#c7cbd4",WhitePixel(display,screen));
	colorAccent=allocColor("#87a8ff",WhitePixel(display,screen));
	colorSuccess=allocColor("#4caf50",WhitePixel(display,screen));
	colorError=allocColor("#ff5c5c",WhitePixel(display,screen));


	XMapWindow(display,window);
	XFlush(display);
	}

ControlWindow::~ControlWindow(void)
	{
	if(display!=0)
		{
		if(graphicsContext!=0)
			XFreeGC(display,graphicsContext);
		if(arrowCursor!=None)
			XFreeCursor(display,arrowCursor);
		if(handCursor!=None)
			XFreeCursor(display,handCursor);
		if(window!=0)
			XDestroyWindow(display,window);
		XCloseDisplay(display);
		}
	}

bool ControlWindow::getFreezeState(void) const
	{
	return freezeOn;
	}

void ControlWindow::setFreezeState(bool state)
	{
	if(freezeOn!=state)
		{
		freezeOn=state;
		unfreezeOn=!state;
		draw();
		}
	}

void ControlWindow::setCurrentFps(int newCurrentFps)
	{
	if(newCurrentFps<0.0)
		newCurrentFps=0.0;
	if(fabs(currentFps-newCurrentFps)>=0.05)
		{
		currentFps=newCurrentFps;
		draw();
		}
	}

bool ControlWindow::consumeExportRequest(void)
	{
	if(!exportRequested)
		return false;
	exportRequested=false;
	return true;
	}

void ControlWindow::setExportStatus(ExportStatus newStatus)
	{
	exportStatus=newStatus;
	exportInProgress=newStatus==EXPORT_PENDING;
	draw();
	}

bool ControlWindow::processEvents(void)
	{
	while(display!=0&&XPending(display)>0)
		{
		XEvent event;
		XNextEvent(display,&event);

		if(event.type==Expose||event.type==ConfigureNotify)
			draw();
		else if(event.type==EnterNotify||event.type==MotionNotify)
			{
			int mx=event.type==MotionNotify?event.xmotion.x:event.xcrossing.x;
			int my=event.type==MotionNotify?event.xmotion.y:event.xcrossing.y;
			updateCursor(mx,my);
			if(draggingAngleSlider)
				setAngleFromMouse(mx);
			draw();
			}
		else if(event.type==LeaveNotify)
			{
			hoverInteractive=false;
			XDefineCursor(display,window,arrowCursor);
			draw();
			}
		else if(event.type==ButtonPress)
			{
			const int x=event.xbutton.x;
			const int y=event.xbutton.y;
			updateCursor(x,y);
			if(exitButtonRect.contains(x,y))
				closeRequested=true;
			else if(freezeButtonRect.contains(x,y))
				{
				freezeOn=true;
				unfreezeOn=false;
				}
			else if(exportButtonRect.contains(x,y))
				{
				exportRequested=true;
				exportInProgress=true;
				exportStatus=EXPORT_PENDING;
				}
			else if(unfreezeButtonRect.contains(x,y))
				{
				unfreezeOn=true;
				freezeOn=false;
				}
			else if(removeWaterButtonRect.contains(x,y))
				removeWaterOn=!removeWaterOn;
			else if(sliderTrackRect.contains(x,y))
				{
				draggingAngleSlider=true;
				setAngleFromMouse(x);
				}
			else if(sliderApplyRect.contains(x,y))
				appliedAngleValue=sliderAngleValue;
			draw();
			}
		else if(event.type==ButtonRelease)
			draggingAngleSlider=false;
		else if(event.type==ClientMessage)
			{
			if(Atom(event.xclient.data.l[0])==wmDeleteWindow)
				closeRequested=true;
			}
		else if(event.type==DestroyNotify)
			closeRequested=true;
		}

	return closeRequested;
	}

const ControlWindow::Rect ControlWindow::exitButtonRect={650,70,90,32};
const ControlWindow::Rect ControlWindow::freezeButtonRect={540,122,210,36};
const ControlWindow::Rect ControlWindow::exportButtonRect={774,122,210,36};
const ControlWindow::Rect ControlWindow::unfreezeButtonRect={540,172,210,36};
const ControlWindow::Rect ControlWindow::removeWaterButtonRect{10,170,220,40};
const ControlWindow::Rect ControlWindow::sliderTrackRect={540,304,444,28};
const ControlWindow::Rect ControlWindow::sliderApplyRect={872,350,112,36};
