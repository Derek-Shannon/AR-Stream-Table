#include "RawKinectViewerWindow.h"

#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

namespace
	{
	const bool xThreadsInitialized=XInitThreads()!=0;
	}

RawKinectViewerWindow::RawKinectViewerWindow(void)
	:display(0),window(0),graphicsContext(0),
	 arrowCursor(0),handCursor(0),
	 closeRequested(false),
	 hoverX(-1),hoverY(-1),
	 //capturePointRequested(false),recaptureBackgroundRequested(false),
	 headline("Status: Waiting for target detection."),
	 detail("Place the CD calibration target at the projected white line intersection. Keep it flat and centered before capture."),
	 //detectionMessage("Target Detection: Waiting for a single CD target."),
	 //readinessMessage("Capture Readiness: Not ready yet."),
	 //tiePointCountMessage("Points Collected: 0"),
	 //stageMessage("Calibration Stage: First pass in progress."),
	 titleFont(0),headingFont(0),bodyFont(0),statusFont(0),
	 captureButtonFlash(false),recaptureButtonFlash(false)
	{
	(void)xThreadsInitialized;
	display=XOpenDisplay(0);
	if(display==0)
		throw std::runtime_error("RawKinectViewerWindow: Cannot open X display");
	
	const int screen=DefaultScreen(display);
	window=XCreateSimpleWindow(display,RootWindow(display,screen),0,0,windowWidth,windowHeight,1,BlackPixel(display,screen),BlackPixel(display,screen));
	XStoreName(display,window,"Raw Kinect Viewer Control Window");
	
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
	
	colorBackground=allocColor("#0b0f19",BlackPixel(display,screen));
	colorPanel=allocColor("#121a2b",WhitePixel(display,screen));
	colorBorder=allocColor("#2d3f64",WhitePixel(display,screen));
	colorText=allocColor("#ecf2ff",WhitePixel(display,screen));
	colorSubtleText=allocColor("#9db3d9",WhitePixel(display,screen));
	colorButton=allocColor("#274a88",WhitePixel(display,screen));
	colorButtonHover=allocColor("#3262b4",WhitePixel(display,screen));
	colorButtonActive=allocColor("#1f8f5f",WhitePixel(display,screen));
	colorAccent=allocColor("#4cc9f0",WhitePixel(display,screen));
	colorSuccess=allocColor("#4ad295",WhitePixel(display,screen));
	colorWarning=allocColor("#ffcf5a",WhitePixel(display,screen));
	colorError=allocColor("#ff6b6b",WhitePixel(display,screen));
	
	titleFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--40-*-*-*-*-*-*-*");
	headingFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--30-*-*-*-*-*-*-*");
	bodyFont=XLoadQueryFont(display,"-misc-dejavu sans-medium-r-normal--26-*-*-*-*-*-*-*");
	statusFont=XLoadQueryFont(display,"-misc-dejavu sans-medium-r-normal--30-*-*-*-*-*-*-*");
	if(titleFont==0)
		titleFont=XLoadQueryFont(display,"9x15bold");
	if(headingFont==0)
		headingFont=XLoadQueryFont(display,"10x20");
	if(bodyFont==0)
		bodyFont=XLoadQueryFont(display,"9x15");
	if(statusFont==0)
		statusFont=XLoadQueryFont(display,"10x20");
	
	captureButtonRect.x=80;
	captureButtonRect.y=160;
	captureButtonRect.w=820;
	captureButtonRect.h=110;
	
	recaptureBackgroundButtonRect.x=980;
	recaptureBackgroundButtonRect.y=160;
	recaptureBackgroundButtonRect.w=860;
	recaptureBackgroundButtonRect.h=110;
	
	captureInfoIconRect.x=captureButtonRect.x+captureButtonRect.w-40;
	captureInfoIconRect.y=captureButtonRect.y+14;
	captureInfoIconRect.w=24;
	captureInfoIconRect.h=24;
	recaptureInfoIconRect.x=recaptureBackgroundButtonRect.x+recaptureBackgroundButtonRect.w-40;
	recaptureInfoIconRect.y=recaptureBackgroundButtonRect.y+14;
	recaptureInfoIconRect.w=24;
	recaptureInfoIconRect.h=24;

	XMapRaised(display,window);
	draw();
	}

RawKinectViewerWindow::~RawKinectViewerWindow(void)
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
		if(statusFont!=0)
			XFreeFont(display,statusFont);
		if(window!=0)
			XDestroyWindow(display,window);
		XCloseDisplay(display);
		}
	}

unsigned long RawKinectViewerWindow::allocColor(const char* name,unsigned long fallback) const
	{
	XColor color;
	XColor exact;
	Colormap colormap=DefaultColormap(display,DefaultScreen(display));
	if(XAllocNamedColor(display,colormap,name,&color,&exact)!=0)
		return color.pixel;
	return fallback;
	}

void RawKinectViewerWindow::setColor(unsigned long color)
	{
	XSetForeground(display,graphicsContext,color);
	}

void RawKinectViewerWindow::drawButton(const Rect& rect,const char* label,bool hovered,bool active)
	{
	setColor(active?colorButtonActive:(hovered?colorButtonHover:colorButton));
	XFillRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);	
	if(active)
		{
		setColor(colorAccent);
		XDrawRectangle(display,window,graphicsContext,rect.x+2,rect.y+2,rect.w-4,rect.h-4);
		}
	setColor(colorText);
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	XDrawString(display,window,graphicsContext,rect.x+30,rect.y+65,label,int(strlen(label)));
	}

void RawKinectViewerWindow::drawTextLine(int x,int y,const std::string& text)
	{
	XDrawString(display,window,graphicsContext,x,y,text.c_str(),int(text.size()));
	}

void RawKinectViewerWindow::drawInfoIcon(const Rect& rect,bool hovered)
	{
	setColor(hovered?colorAccent:colorSubtleText);
	XDrawArc(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h,0,360*64);
	setColor(colorText);
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	XDrawString(display,window,graphicsContext,rect.x+8,rect.y+18,"i",1);
	}

void RawKinectViewerWindow::drawTooltip(const Rect& anchor,const char* line1,const char* line2,const char* line3)
	{
	const int tooltipWidth=760;
	const int tooltipHeight=92;
	const int tooltipX=anchor.x-tooltipWidth+anchor.w;
	const int tooltipY=anchor.y+anchor.h+8;
	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,tooltipX,tooltipY,tooltipWidth,tooltipHeight);
	setColor(colorAccent);
	XDrawRectangle(display,window,graphicsContext,tooltipX,tooltipY,tooltipWidth,tooltipHeight);
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorText);
	XDrawString(display,window,graphicsContext,tooltipX+12,tooltipY+28,line1,int(strlen(line1)));
	XDrawString(display,window,graphicsContext,tooltipX+12,tooltipY+54,line2,int(strlen(line2)));
	XDrawString(display,window,graphicsContext,tooltipX+12,tooltipY+80,line3,int(strlen(line3)));
	}

void RawKinectViewerWindow::updateCursor(void)
	{
	const bool overButton=captureButtonRect.contains(hoverX,hoverY)||recaptureBackgroundButtonRect.contains(hoverX,hoverY)||captureInfoIconRect.contains(hoverX,hoverY)||recaptureInfoIconRect.contains(hoverX,hoverY);
	XDefineCursor(display,window,overButton?handCursor:arrowCursor);
	}

void RawKinectViewerWindow::draw(void)
	{
	setColor(colorBackground);
	XFillRectangle(display,window,graphicsContext,0,0,windowWidth,windowHeight);
	
	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,40,30,1840,1020);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,40,30,1840,1020);
	
	if(titleFont!=0)
		XSetFont(display,graphicsContext,titleFont->fid);
	setColor(colorAccent);
	drawTextLine(70,95,"Raw Kinect Viewer Control Window");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorSubtleText);
		drawTextLine(70,140,"Manual Capture Instructions");
	
	drawButton(captureButtonRect,"Capture Tie Point",captureButtonRect.contains(hoverX,hoverY),captureButtonFlash);
	drawButton(recaptureBackgroundButtonRect,"Re-Capture Background",recaptureBackgroundButtonRect.contains(hoverX,hoverY),recaptureButtonFlash);
	drawInfoIcon(captureInfoIconRect,captureInfoIconRect.contains(hoverX,hoverY));
	drawInfoIcon(recaptureInfoIconRect,recaptureInfoIconRect.contains(hoverX,hoverY));
	if(captureInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(captureInfoIconRect,
			"Capture Tie Point records one calibration point at current crosshairs.",
			"Use when one target is centered and flat.",
			"This button advances crosshairs to the next calibration point.");
	if(recaptureInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(recaptureInfoIconRect,
			"Re-Capture Background refreshes base sand depth after surface changes.",
			"Use after digging/filling/re-shaping or if detection becomes unstable.",
			"Red screen confirms background capture in progress.");
	captureButtonFlash=false;
	recaptureButtonFlash=false;
	
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorAccent);
	drawTextLine(70,340,"Calibration Steps");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorSubtleText);
	drawTextLine(70,380,"1) [ONLY IF SETTING KEYBOARD BINDS] Bind Capture tool: hold key/button 1 to open tool menu, highlight 'Capture', release 1.");
	setColor(colorText);
	drawTextLine(70,420,"2) [ONLY IF SETTING KEYBOARD BINDS] In 'Creating Capture Tool...' prompt, press/release key/button 2 for Capture Background.");
	setColor(colorText);
	drawTextLine(70,460,"3) When a GREEN Circle Appears, capture tie point: press key/button 1 OR click 'Capture Tie Point' button.");
	setColor(colorSuccess);
	drawTextLine(70,500,"4) Re-capture background: press key/button 2 OR click 'Re-Capture Background' button.");
	setColor(colorWarning);
	drawTextLine(70,540,"5) Red screen means background re-capture is running and this does not move crosshairs.");
	setColor(colorError);
	drawTextLine(70,580,"6) If red appears when expecting tie point capture, background capture was triggered instead.");
	setColor(colorText);
	drawTextLine(70,620,"7) Keep Calibration target visible, centered on crosshairs before each capture.");
	setColor(colorSubtleText);
	drawTextLine(70,660,"8) Assigned keys/buttons still work at all times, buttons are an additional input option.");
	
	if(statusFont!=0)
		XSetFont(display,graphicsContext,statusFont->fid);
	if(headline.find("failed")!=std::string::npos)
		setColor(colorError);
	else if(headline.find("Ready")!=std::string::npos||headline.find("detected")!=std::string::npos)
		setColor(colorSuccess);
	else if(headline.find("background")!=std::string::npos)
		setColor(colorWarning);
	else
		setColor(colorAccent);
	drawTextLine(70,720,headline);
	//if(bodyFont!=0)
		//XSetFont(display,graphicsContext,bodyFont->fid);
	//setColor(colorSubtleText);
	//drawTextLine(70,760,detail);
	//drawTextLine(70,805,detectionMessage);
	//drawTextLine(70,845,readinessMessage);
	//drawTextLine(70,885,tiePointCountMessage);
	//drawTextLine(70,925,stageMessage);
	
	XFlush(display);
	}

bool RawKinectViewerWindow::processEvents(void)
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
			
			// case ButtonPress:
			// 	if(captureButtonRect.contains(event.xbutton.x,event.xbutton.y))
			// 		{
			// 		capturePointRequested=true;
			// 		captureButtonFlash=true;
			// 		}
			// 	else if(recaptureBackgroundButtonRect.contains(event.xbutton.x,event.xbutton.y))
			// 		{
			// 		recaptureBackgroundRequested=true;
			// 		recaptureButtonFlash=true;
			// 		}
			// 	draw();
			// 	break;
			
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

// bool RawKinectViewerWindow::consumeCapturePointRequest(void)
// 	{
// 	bool result=capturePointRequested;
// 	capturePointRequested=false;
// 	return result;
// 	}

// bool RawKinectViewerWindow::consumeRecaptureBackgroundRequest(void)
// 	{
// 	bool result=recaptureBackgroundRequested;
// 	recaptureBackgroundRequested=false;
// 	return result;
// 	}

// void RawKinectViewerWindow::updateStatus(bool capturingBackground,bool capturingTiePoint,bool tiePointCaptureFailed,int detectedTargetCount,unsigned int collectedTiePointCount,int firstPassPointGoal)
// 	{
// 	headline="Status: Waiting for target detection.";
// 	detail="Place the CD calibration target at the projected white-line intersection. Keep it flat, centered, and remove your hands before capture.";
// 	detectionMessage="Target Detection: Waiting for a single CD target.";
// 	readinessMessage="Capture Readiness: Not ready yet.";
	
// 	if(capturingBackground)
// 		{
// 		headline="Status: Capturing background model...";
// 		detail="Background update in progress. Keep sandbox clear until complete.";
// 		detectionMessage="Target Detection: Paused during background capture.";
// 		readinessMessage="Capture Readiness: Waiting for background update.";
// 		}
// 	else if(capturingTiePoint)
// 		{
// 		headline="Status: Capturing calibration point...";
// 		detail="Hold target steady; remove hands until capture completes.";
// 		detectionMessage="Target Detection: Locked for active capture.";
// 		readinessMessage="Capture Readiness: Capture in progress.";
// 		}
// 	else if(tiePointCaptureFailed)
// 		{
// 		headline="Status: Capture failed.";
// 		detail="Need exactly one stable CD target. Reposition target and try again.";
// 		detectionMessage="Target Detection: Unstable or multiple targets.";
// 		readinessMessage="Capture Readiness: Not ready.";
// 		}
// 	else if(detectedTargetCount==1)
// 		{
// 		headline="Status: Target detected. Ready to capture.";
// 		detail="Remove hands and press Capture Point now.";
// 		detectionMessage="Target Detection: Single target detected.";
// 		readinessMessage="Capture Readiness: Ready.";
// 		}
// 	else if(detectedTargetCount>1)
// 		{
// 		headline="Status: Multiple targets detected.";
// 		detail="Leave only one CD target visible for reliable capture.";
// 		detectionMessage="Target Detection: Multiple targets detected.";
// 		readinessMessage="Capture Readiness: Not ready.";
// 		}
	
// 	char message[256];
// 	snprintf(message,sizeof(message),"Points Collected: %u (first-pass goal: %d). Additional tie points can be added after first pass.",collectedTiePointCount,firstPassPointGoal);
// 	tiePointCountMessage=message;
// 	if(int(collectedTiePointCount)<firstPassPointGoal)
// 		{
// 		snprintf(message,sizeof(message),"Calibration Stage: First pass in progress (%d more suggested).",firstPassPointGoal-int(collectedTiePointCount));
// 		stageMessage=message;
// 		}
// 	else
// 		stageMessage="Calibration Stage: First pass complete. Add varied-height points to improve alignment.";
	
// 	draw();
// 	}