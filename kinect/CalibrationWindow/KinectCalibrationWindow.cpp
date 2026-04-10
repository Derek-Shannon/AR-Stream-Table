#include "KinectCalibrationWindow.h"
#include "../RawKinectViewer.h"

#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

namespace
	{
	const bool xThreadsInitialized=XInitThreads()!=0;
	}

KinectCalibrationWindow::KinectCalibrationWindow(RawKinectViewer* viewer)
	:display(0),window(0),graphicsContext(0),
	 arrowCursor(0),handCursor(0),
	 closeRequested(false),
	 viewer(viewer),
	 hoverX(-1),hoverY(-1),
	 titleFont(0),headingFont(0),bodyFont(0)
	{
	(void)xThreadsInitialized;
	display=XOpenDisplay(0);
	if(display==0)
		throw std::runtime_error("KinectCalibrationWindow: Cannot open X display");
	
	const int screen=DefaultScreen(display);
	window=XCreateSimpleWindow(display,RootWindow(display,screen),0,0,windowWidth,windowHeight,1,BlackPixel(display,screen),BlackPixel(display,screen));
	XStoreName(display,window,"Kinect Calibration Control Window");
	
	XSetWindowAttributes attrs;
	attrs.override_redirect=False;
	XChangeWindowAttributes(display,window,CWOverrideRedirect,&attrs);
	
	XSelectInput(display,window,ExposureMask|ButtonPressMask|PointerMotionMask|StructureNotifyMask|KeyPressMask);
	graphicsContext=XCreateGC(display,window,0,0);
	
	wmDeleteWindow=XInternAtom(display,"WM_DELETE_WINDOW",False);
	XSetWMProtocols(display,window,&wmDeleteWindow,1);
	arrowCursor=XCreateFontCursor(display,XC_left_ptr);
	handCursor=XCreateFontCursor(display,XC_hand2);
	XDefineCursor(display,window,arrowCursor);
	
	/* Allocate colors — same palette as the rest of the calibration UI */
	colorBackground=allocColor("#0b0f19",BlackPixel(display,screen));
	colorPanel=allocColor("#121a2b",WhitePixel(display,screen));
	colorBorder=allocColor("#2d3f64",WhitePixel(display,screen));
	colorText=allocColor("#ecf2ff",WhitePixel(display,screen));
	colorSubtleText=allocColor("#9db3d9",WhitePixel(display,screen));
	colorButton=allocColor("#274a88",WhitePixel(display,screen));
	colorButtonHover=allocColor("#3262b4",WhitePixel(display,screen));
	colorButtonActive=allocColor("#1f8f5f",WhitePixel(display,screen));
	colorFinish=allocColor("#b02020",WhitePixel(display,screen));
	colorFinishHover=allocColor("#d43030",WhitePixel(display,screen));
	colorAccent=allocColor("#4cc9f0",WhitePixel(display,screen));
	colorSuccess=allocColor("#4ad295",WhitePixel(display,screen));
	colorWarning=allocColor("#ffcf5a",WhitePixel(display,screen));
	colorError=allocColor("#ff6b6b",WhitePixel(display,screen));
	
	/* Load fonts */
	titleFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--48-*-*-*-*-*-*-*");
	headingFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--32-*-*-*-*-*-*-*");
	bodyFont=XLoadQueryFont(display,"-misc-dejavu sans-medium-r-normal--24-*-*-*-*-*-*-*");
	if(titleFont==0)
		titleFont=XLoadQueryFont(display,"9x15bold");
	if(headingFont==0)
		headingFont=XLoadQueryFont(display,"10x20");
	if(bodyFont==0)
		bodyFont=XLoadQueryFont(display,"9x15");
	
	/* --- Button layout --- */
	const int btnY=170;
	const int btnH=110;
	const int btnW=560;
	const int btnGap=40;
	const int btnStartX=70;
	
	averageFramesButtonRect.x=btnStartX;
	averageFramesButtonRect.y=btnY;
	averageFramesButtonRect.w=btnW;
	averageFramesButtonRect.h=btnH;
	
	extractPlanesButtonRect.x=btnStartX+btnW+btnGap;
	extractPlanesButtonRect.y=btnY;
	extractPlanesButtonRect.w=btnW;
	extractPlanesButtonRect.h=btnH;
	
	measure3DButtonRect.x=btnStartX+(btnW+btnGap)*2;
	measure3DButtonRect.y=btnY;
	measure3DButtonRect.w=btnW;
	measure3DButtonRect.h=btnH;
	
	/* Info icons sit in the top-right corner of each button */
	const int iconSize=28;
	const int iconMargin=10;
	averageFramesInfoIconRect.x=averageFramesButtonRect.x+averageFramesButtonRect.w-iconSize-iconMargin;
	averageFramesInfoIconRect.y=averageFramesButtonRect.y+iconMargin;
	averageFramesInfoIconRect.w=iconSize;
	averageFramesInfoIconRect.h=iconSize;
	
	extractPlanesInfoIconRect.x=extractPlanesButtonRect.x+extractPlanesButtonRect.w-iconSize-iconMargin;
	extractPlanesInfoIconRect.y=extractPlanesButtonRect.y+iconMargin;
	extractPlanesInfoIconRect.w=iconSize;
	extractPlanesInfoIconRect.h=iconSize;
	
	measure3DInfoIconRect.x=measure3DButtonRect.x+measure3DButtonRect.w-iconSize-iconMargin;
	measure3DInfoIconRect.y=measure3DButtonRect.y+iconMargin;
	measure3DInfoIconRect.w=iconSize;
	measure3DInfoIconRect.h=iconSize;
	
	/* Finish button — bottom right of the panel */
	finishCalibrationButtonRect.w=400;
	finishCalibrationButtonRect.h=90;
	finishCalibrationButtonRect.x=windowWidth-40-finishCalibrationButtonRect.w-70;
	finishCalibrationButtonRect.y=windowHeight-40-finishCalibrationButtonRect.h-50;
	
	/* Lock the window size so the user cannot resize it */
	XSizeHints* sizeHints=XAllocSizeHints();
	sizeHints->flags=PMinSize|PMaxSize;
	sizeHints->min_width=windowWidth;
	sizeHints->max_width=windowWidth;
	sizeHints->min_height=windowHeight;
	sizeHints->max_height=windowHeight;
	XSetWMNormalHints(display,window,sizeHints);
	XFree(sizeHints);
	
	XMapRaised(display,window);
	}

KinectCalibrationWindow::~KinectCalibrationWindow(void)
	{
	if(display!=0)
		{
		if(arrowCursor!=0)
			XFreeCursor(display,arrowCursor);
		if(handCursor!=0)
			XFreeCursor(display,handCursor);
		if(graphicsContext!=0)
			XFreeGC(display,graphicsContext);
		if(titleFont!=0)
			XFreeFont(display,titleFont);
		if(headingFont!=0)
			XFreeFont(display,headingFont);
		if(bodyFont!=0)
			XFreeFont(display,bodyFont);
		if(window!=0)
			XDestroyWindow(display,window);
		XCloseDisplay(display);
		}
	}

unsigned long KinectCalibrationWindow::allocColor(const char* name,unsigned long fallback) const
	{
	XColor color;
	XColor exact;
	Colormap colormap=DefaultColormap(display,DefaultScreen(display));
	if(XAllocNamedColor(display,colormap,name,&color,&exact)!=0)
		return color.pixel;
	return fallback;
	}

void KinectCalibrationWindow::setColor(unsigned long color)
	{
	XSetForeground(display,graphicsContext,color);
	}

void KinectCalibrationWindow::drawButton(const Rect& rect,const char* label,bool hovered,unsigned long baseColor)
	{
	/* Hover color: finish button has its own hover, active (green) buttons lighten slightly */
	unsigned long fillColor;
	if(hovered)
		fillColor=(baseColor==colorFinish)?colorFinishHover:colorButtonHover;
	else
		fillColor=baseColor;
	setColor(fillColor);
	XFillRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	/* Border */
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	/* Label — centred vertically, padded left */
	setColor(colorText);
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	XDrawString(display,window,graphicsContext,rect.x+30,rect.y+(rect.h/2)+12,label,int(strlen(label)));
	}

void KinectCalibrationWindow::drawTextLine(int x,int y,const std::string& text)
	{
	XDrawString(display,window,graphicsContext,x,y,text.c_str(),int(text.size()));
	}

void KinectCalibrationWindow::drawInfoIcon(const Rect& rect,bool hovered)
	{
	setColor(hovered?colorAccent:colorSubtleText);
	XDrawArc(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h,0,360*64);
	setColor(colorText);
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	XDrawString(display,window,graphicsContext,rect.x+10,rect.y+21,"i",1);
	}

void KinectCalibrationWindow::drawTooltip(const Rect& anchor,const char* line1,const char* line2,const char* line3)
	{
	const int tooltipWidth=800;
	const int tooltipHeight=96;
	int tooltipX=anchor.x+anchor.w/2-tooltipWidth/2;
	if(tooltipX+tooltipWidth>windowWidth-40)
		tooltipX=windowWidth-40-tooltipWidth;
	if(tooltipX<40)
		tooltipX=40;
	const int tooltipY=anchor.y+anchor.h+8;
	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,tooltipX,tooltipY,tooltipWidth,tooltipHeight);
	setColor(colorAccent);
	XDrawRectangle(display,window,graphicsContext,tooltipX,tooltipY,tooltipWidth,tooltipHeight);
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorText);
	XDrawString(display,window,graphicsContext,tooltipX+14,tooltipY+30,line1,int(strlen(line1)));
	XDrawString(display,window,graphicsContext,tooltipX+14,tooltipY+58,line2,int(strlen(line2)));
	XDrawString(display,window,graphicsContext,tooltipX+14,tooltipY+84,line3,int(strlen(line3)));
	}

void KinectCalibrationWindow::updateCursor(void)
	{
	const bool overAny=
		averageFramesButtonRect.contains(hoverX,hoverY)||
		extractPlanesButtonRect.contains(hoverX,hoverY)||
		measure3DButtonRect.contains(hoverX,hoverY)||
		finishCalibrationButtonRect.contains(hoverX,hoverY)||
		averageFramesInfoIconRect.contains(hoverX,hoverY)||
		extractPlanesInfoIconRect.contains(hoverX,hoverY)||
		measure3DInfoIconRect.contains(hoverX,hoverY);
	XDefineCursor(display,window,overAny?handCursor:arrowCursor);
	}

void KinectCalibrationWindow::draw(void)
	{
	/* Read live state from the viewer for button color feedback */
	const bool averagingActive        = viewer!=0 && viewer->showAverageFrame;
	const bool extractPlanesActive    = viewer!=0 && viewer->isExtractPlanesToolBound();
	const bool measure3DActive        = viewer!=0 && viewer->isMeasure3DToolBound();

	/* Background */
	setColor(colorBackground);
	XFillRectangle(display,window,graphicsContext,0,0,windowWidth,windowHeight);
	
	/* Main panel */
	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,40,30,windowWidth-80,windowHeight-60);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,40,30,windowWidth-80,windowHeight-60);
	
	/* Title */
	if(titleFont!=0)
		XSetFont(display,graphicsContext,titleFont->fid);
	setColor(colorAccent);
	drawTextLine(70,100,"Calibrate Kinect Control Window");
	
	/* Quick-help buttons subtitle */
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorSubtleText);
	drawTextLine(70,150,"Quick Help Buttons (hover information icon for more information)");
	
	/* Three quick-help buttons — green when their tool/state is active, blue otherwise */
	drawButton(averageFramesButtonRect,"Average Frames",
		averageFramesButtonRect.contains(hoverX,hoverY),
		averagingActive?colorButtonActive:colorButton);
	drawButton(extractPlanesButtonRect,"Set Extract Planes Tool",
		extractPlanesButtonRect.contains(hoverX,hoverY),
		extractPlanesActive?colorButtonActive:colorButton);
	drawButton(measure3DButtonRect,"Set Measure 3D Positions Tool",
		measure3DButtonRect.contains(hoverX,hoverY),
		measure3DActive?colorButtonActive:colorButton);
	
	/* Info icons */
	drawInfoIcon(averageFramesInfoIconRect,averageFramesInfoIconRect.contains(hoverX,hoverY));
	drawInfoIcon(extractPlanesInfoIconRect,extractPlanesInfoIconRect.contains(hoverX,hoverY));
	drawInfoIcon(measure3DInfoIconRect,measure3DInfoIconRect.contains(hoverX,hoverY));
	
	/* Tooltips — only one shows at a time */
	if(averageFramesInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(averageFramesInfoIconRect,
			"Average Frames captures a stable depth average over many frames.",
			"Remove ALL hands, persons, or other moving objects from within the frame while averaging.",
			"Use this before extracting planes to reduce depth noise.");
	else if(extractPlanesInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(extractPlanesInfoIconRect,
			"Sets the Extract Planes Tool to your 'E' key.",
			"Move cursor to the top left corner of your flat plane.",
			"Press and hold 'E' while moving your cursor to the bottom right corner of the flat plane, release.");
	else if(measure3DInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(measure3DInfoIconRect,
			"Sets the Measure 3D Positions Tool to your 'M' key.",
			"Move the cursor to the correct, inner corner of the table.",
			"Press the 'M' key to capture the 3D position of that corner.");
	
	/* Calibration steps heading */
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorAccent);
	drawTextLine(70,360,"Kinect Calibration Steps:");
	
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorText);
	drawTextLine(70,405, "1.  Cover the steam table (or a portion of it) with a flat piece of cardboard or other material");
	drawTextLine(70,445, "2.  Zoom in (using mouse wheel down) and pan (holding 'Z' and dragging mouse) to enlarge the depth view of the table");
	drawTextLine(70,485, "3.  Remove ALL hands, persons, and other moving objects within the depth viewer and press the 'Average Frames' button.");
	drawTextLine(70,525, "    DO NOT place hands or move anything within the depth viewer for 3 seconds. Otherwise, press 'Average Frames' and retry");
	drawTextLine(70,565, "4.  Press the 'Set Extract Planes Tool' button. This will assign the tool to your 'E' key");
	drawTextLine(70,605, "5.  Move your cursor to the top left corner of the flat plane on your table");
	drawTextLine(70,645, "    Press and hold your 'E' key while dragging cursor to the bottom right corner of the flat plane. Release the 'E' Key");
	drawTextLine(70,685, "6.  Toggle the Average Frames OFF by pressing the 'Average Frames' button");
	drawTextLine(70,725, "7.  Remove the cardboard, or flat material, from the table and repeat Step 3 to toggle Average Frames ON");
	drawTextLine(70,765, "8.  Press the 'Set Measure 3D Positions Tool' button. This will assign the tool to your 'M' key");
	drawTextLine(70,805, "9.  Move your cursor to INSIDE of the Lower-Left corner of the table and press the 'M' key");
	drawTextLine(70,845, "10. Repeat Step 9 for the Lower-Right, then the Upper-Left, and then the Upper-Right corners");
	
	/* Finish Calibration button — red, bottom-right */
	drawButton(finishCalibrationButtonRect,"Finish Kinect Calibration",
		finishCalibrationButtonRect.contains(hoverX,hoverY),colorFinish);
	
	XFlush(display);
	}

bool KinectCalibrationWindow::processEvents(void)
	{
	while(XPending(display)>0)
		{
		XEvent event;
		XNextEvent(display,&event);
		switch(event.type)
			{
			case Expose:
				draw();
				break;
			
			case MotionNotify:
				hoverX=event.xmotion.x;
				hoverY=event.xmotion.y;
				updateCursor();
				draw();
				break;
			
			case ButtonPress:
				if(averageFramesButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					if(viewer!=0)
						viewer->toggleAverageFrames();
					}
				else if(extractPlanesButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					if(viewer!=0)
						viewer->bindExtractPlanesTool();
					}
				else if(measure3DButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					if(viewer!=0)
						viewer->bindMeasure3DTool();
					}
				else if(finishCalibrationButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					closeRequested=true;
					}
				draw();
				break;
			
			case ClientMessage:
				if(Atom(event.xclient.data.l[0])==wmDeleteWindow)
					closeRequested=true;
				break;
			
			default:
				break;
			}
		}
	return closeRequested;
	}
