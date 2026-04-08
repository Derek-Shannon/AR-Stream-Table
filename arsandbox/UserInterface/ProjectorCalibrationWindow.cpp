#include "ProjectorCalibrationWindow.h"

#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

namespace
	{
	const bool xThreadsInitialized=XInitThreads()!=0;
	const unsigned int minTiePointsRequired=12;
	}

ProjectorCalibrationWindow::ProjectorCalibrationWindow(void)
	:display(0),window(0),graphicsContext(0),
	 arrowCursor(0),handCursor(0),
	 closeRequested(false),
	 hoverX(-1),hoverY(-1),
	 capturePointRequested(false),recaptureBackgroundRequested(false),resetCalibrationRequested(false),finishCalibrationRequested(false),
	 tiePointCount(0),
	 titleFont(0),headingFont(0),bodyFont(0),emphasisFont(0),
	 captureButtonFlash(false),recaptureButtonFlash(false)
	{
	(void)xThreadsInitialized;
	display=XOpenDisplay(0);
	if(display==0)
		throw std::runtime_error("ProjectorCalibrationWindow: Cannot open X display");
	
	const int screen=DefaultScreen(display);
	window=XCreateSimpleWindow(display,RootWindow(display,screen),0,0,windowWidth,windowHeight,1,BlackPixel(display,screen),BlackPixel(display,screen));
	XStoreName(display,window,"Projector Calibration Control Window");
	
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
	
	titleFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--90-*-*-*-*-*-*-*");
	headingFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--46-*-*-*-*-*-*-*");
	bodyFont=XLoadQueryFont(display,"-misc-dejavu sans-medium-r-normal--34-*-*-*-*-*-*-*");
	emphasisFont=XLoadQueryFont(display,"-misc-dejavu sans-bold-r-normal--34-*-*-*-*-*-*-*");
	if(titleFont==0)
		titleFont=XLoadQueryFont(display,"9x15bold");
	if(headingFont==0)
		headingFont=XLoadQueryFont(display,"10x20");
	if(bodyFont==0)
		bodyFont=XLoadQueryFont(display,"9x15");
	if(emphasisFont==0)
		emphasisFont=bodyFont;
	
	captureButtonRect.x=80;
	captureButtonRect.y=160;
	captureButtonRect.w=820;
	captureButtonRect.h=110;
	
	recaptureBackgroundButtonRect.x=980;
	recaptureBackgroundButtonRect.y=160;
	recaptureBackgroundButtonRect.w=860;
	recaptureBackgroundButtonRect.h=110;

	exitButtonRect.x=1700;
	exitButtonRect.y=50;
	exitButtonRect.w=140;
	exitButtonRect.h=60;

	runStreamTableButtonRect.x=1500;
	runStreamTableButtonRect.y=880;
	runStreamTableButtonRect.w=340;
	runStreamTableButtonRect.h=70;
	resetButtonRect.x=1140;
	resetButtonRect.y=880;
	resetButtonRect.w=320;
	resetButtonRect.h=70;
	
	captureInfoIconRect.x=captureButtonRect.x+captureButtonRect.w-40;
	captureInfoIconRect.y=captureButtonRect.y+14;
	captureInfoIconRect.w=24;
	captureInfoIconRect.h=24;
	recaptureInfoIconRect.x=recaptureBackgroundButtonRect.x+recaptureBackgroundButtonRect.w-40;
	recaptureInfoIconRect.y=recaptureBackgroundButtonRect.y+14;
	recaptureInfoIconRect.w=24;
	recaptureInfoIconRect.h=24;
	calibrationTipsIconRect.x=88;
	calibrationTipsIconRect.y=326;
	calibrationTipsIconRect.w=18;
	calibrationTipsIconRect.h=18;

	XMapRaised(display,window);
	draw();
	}

ProjectorCalibrationWindow::~ProjectorCalibrationWindow(void)
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
		if(emphasisFont!=0&&emphasisFont!=bodyFont)
			XFreeFont(display,emphasisFont);
		if(window!=0)
			XDestroyWindow(display,window);
		XCloseDisplay(display);
		}
	}

unsigned long ProjectorCalibrationWindow::allocColor(const char* name,unsigned long fallback) const
	{
	XColor color;
	XColor exact;
	Colormap colormap=DefaultColormap(display,DefaultScreen(display));
	if(XAllocNamedColor(display,colormap,name,&color,&exact)!=0)
		return color.pixel;
	return fallback;
	}

void ProjectorCalibrationWindow::setColor(unsigned long color)
	{
	XSetForeground(display,graphicsContext,color);
	}

void ProjectorCalibrationWindow::drawButton(const Rect& rect,const char* label,bool hovered,bool active)
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

void ProjectorCalibrationWindow::drawTextLine(int x,int y,const std::string& text)
	{
	XDrawString(display,window,graphicsContext,x,y,text.c_str(),int(text.size()));
	}

void ProjectorCalibrationWindow::drawInfoIcon(const Rect& rect,bool hovered)
	{
	setColor(hovered?colorAccent:colorSubtleText);
	XDrawArc(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h,0,360*64);
	setColor(colorText);
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	const char* infoLabel="i";
	const int textWidth=bodyFont!=0?XTextWidth(bodyFont,infoLabel,1):0;
	const int textX=rect.x+(rect.w-textWidth)/2 + 1;
	const int textY=bodyFont!=0?rect.y+(rect.h+bodyFont->ascent-bodyFont->descent)/2:rect.y+rect.h-4;
	XDrawString(display,window,graphicsContext,textX,textY,infoLabel,1);
	}

void ProjectorCalibrationWindow::drawTooltip(const Rect& anchor,const char* line1,const char* line2,const char* line3)
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

void ProjectorCalibrationWindow::drawTipsTooltip(const Rect& anchor)
	{
	const int tooltipWidth=980;
	const int tooltipHeight=250;
	const int tooltipX=anchor.x+anchor.w+14;
	const int tooltipY=anchor.y-20;
	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,tooltipX,tooltipY,tooltipWidth,tooltipHeight);
	setColor(colorAccent);
	XDrawRectangle(display,window,graphicsContext,tooltipX,tooltipY,tooltipWidth,tooltipHeight);
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	const char* line0="Calibration Tips";
	const char* line1="- Hold the disk flat and steady";
	const char* line2="- Try to keep hands and body out of view";
	const char* line3="- Capture tie points on flat sand, raised areas, and inside holes";
	const char* line4="- Green circle size may differ from calibration disk (that is okay!)";
	const char* line5="- Add more tie points if needed and then red crosshair should stay centered when complete";
	const char* line6="- Exiting the Calibration Projector early without saving will cancel all tie points";
	XDrawString(display,window,graphicsContext,tooltipX+14,tooltipY+40,line0,int(strlen(line0)));
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	XDrawString(display,window,graphicsContext,tooltipX+18,tooltipY+76,line1,int(strlen(line1)));
	XDrawString(display,window,graphicsContext,tooltipX+18,tooltipY+106,line2,int(strlen(line2)));
	XDrawString(display,window,graphicsContext,tooltipX+18,tooltipY+136,line3,int(strlen(line3)));
	XDrawString(display,window,graphicsContext,tooltipX+18,tooltipY+166,line4,int(strlen(line4)));
	XDrawString(display,window,graphicsContext,tooltipX+18,tooltipY+196,line5,int(strlen(line5)));
	XDrawString(display,window,graphicsContext,tooltipX+18,tooltipY+226,line6,int(strlen(line6)));
	}

void ProjectorCalibrationWindow::updateCursor(void)
	{
	const bool overButton=
		captureButtonRect.contains(hoverX,hoverY)||
		recaptureBackgroundButtonRect.contains(hoverX,hoverY)||
		exitButtonRect.contains(hoverX,hoverY)||
		resetButtonRect.contains(hoverX,hoverY)||
		(tiePointCount>=minTiePointsRequired&&runStreamTableButtonRect.contains(hoverX,hoverY))||
		captureInfoIconRect.contains(hoverX,hoverY)||
		recaptureInfoIconRect.contains(hoverX,hoverY)||
		calibrationTipsIconRect.contains(hoverX,hoverY);
	XDefineCursor(display,window,overButton?handCursor:arrowCursor);
	}

void ProjectorCalibrationWindow::draw(void)
	{
	setColor(colorBackground);
	XFillRectangle(display,window,graphicsContext,0,0,windowWidth,windowHeight);
	
	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,40,30,1840,1020);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,40,30,1840,1020);
	
	if(titleFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorAccent);
	drawTextLine(70,95,"Projector Calibration Control Window");

	setColor(colorError);
	XFillRectangle(display,window,graphicsContext,exitButtonRect.x,exitButtonRect.y,exitButtonRect.w,exitButtonRect.h);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,exitButtonRect.x,exitButtonRect.y,exitButtonRect.w,exitButtonRect.h);
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	const char* exitLabel="Exit";
	const int exitLabelWidth=headingFont!=0?XTextWidth(headingFont,exitLabel,int(strlen(exitLabel))):0;
	const int exitBaseline=headingFont!=0?exitButtonRect.y+(exitButtonRect.h+headingFont->ascent-headingFont->descent)/2:exitButtonRect.y+42;
	drawTextLine(exitButtonRect.x+(exitButtonRect.w-exitLabelWidth)/2,exitBaseline,exitLabel);


	setColor(colorError);
	XFillRectangle(display,window,graphicsContext,resetButtonRect.x,resetButtonRect.y,resetButtonRect.w,resetButtonRect.h);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,resetButtonRect.x,resetButtonRect.y,resetButtonRect.w,resetButtonRect.h);
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	const char* resetLabel="Reset";
	const int resetLabelWidth=headingFont!=0?XTextWidth(headingFont,resetLabel,int(strlen(resetLabel))):0;
	const int resetBaseline=headingFont!=0?resetButtonRect.y+(resetButtonRect.h+headingFont->ascent-headingFont->descent)/2:resetButtonRect.y+47;
	drawTextLine(resetButtonRect.x+(resetButtonRect.w-resetLabelWidth)/2,resetBaseline,resetLabel);
	if(tiePointCount>=minTiePointsRequired)
		{
		setColor(colorSuccess);
		XFillRectangle(display,window,graphicsContext,runStreamTableButtonRect.x,runStreamTableButtonRect.y,runStreamTableButtonRect.w,runStreamTableButtonRect.h);
		setColor(colorBorder);
		XDrawRectangle(display,window,graphicsContext,runStreamTableButtonRect.x,runStreamTableButtonRect.y,runStreamTableButtonRect.w,runStreamTableButtonRect.h);
		if(headingFont!=0)
			XSetFont(display,graphicsContext,headingFont->fid);
		setColor(colorText);
		const char* runLabel="Save Projector Calibration";
		const int runLabelWidth=headingFont!=0?XTextWidth(headingFont,runLabel,int(strlen(runLabel))):0;
		const int runBaseline=headingFont!=0?runStreamTableButtonRect.y+(runStreamTableButtonRect.h+headingFont->ascent-headingFont->descent)/2:runStreamTableButtonRect.y+47;
		drawTextLine(runStreamTableButtonRect.x+(runStreamTableButtonRect.w-runLabelWidth)/2,runBaseline,runLabel);
		}

	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorSubtleText);
	
	drawButton(captureButtonRect,"Capture Tie Point",captureButtonRect.contains(hoverX,hoverY),captureButtonFlash);
	drawButton(recaptureBackgroundButtonRect,"Re-Capture Background",recaptureBackgroundButtonRect.contains(hoverX,hoverY),recaptureButtonFlash);
	drawInfoIcon(captureInfoIconRect,captureInfoIconRect.contains(hoverX,hoverY));
	drawInfoIcon(recaptureInfoIconRect,recaptureInfoIconRect.contains(hoverX,hoverY));
	drawInfoIcon(calibrationTipsIconRect,calibrationTipsIconRect.contains(hoverX,hoverY));
	if(captureInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(captureInfoIconRect,
			"Capture Tie Point records one calibration point at current crosshairs.",
			"Use when one target is centered and flat.",
			"This button advances crosshairs to the next calibration point.");
	if(recaptureInfoIconRect.contains(hoverX,hoverY))
		drawTooltip(recaptureInfoIconRect,
			"Re-Capture Background refreshes base sand depth after surface changes.",
			"Use after digging/filling physical sand in stream table.",
			"Red screen confirms background capture in progress.");
	const bool showCalibrationTipsTooltip=calibrationTipsIconRect.contains(hoverX,hoverY);
	captureButtonFlash=false;
	recaptureButtonFlash=false;

	const int statusCenterX=1410;
	const int statusCenterY=560;
	const int statusRadius=185;
	const unsigned int requiredTiePoints=minTiePointsRequired;
	const double progress=tiePointCount<requiredTiePoints?double(tiePointCount)/double(requiredTiePoints):1.0;
	const bool complete=tiePointCount>=requiredTiePoints;
	const int progressAngle=int(progress*360.0*64.0);

	setColor(colorBackground);
	XFillArc(display,window,graphicsContext,statusCenterX-statusRadius,statusCenterY-statusRadius,statusRadius*2,statusRadius*2,0,360*64);
	XSetLineAttributes(display,graphicsContext,16,LineSolid,CapButt,JoinRound);
	setColor(colorBorder);
	XDrawArc(display,window,graphicsContext,statusCenterX-statusRadius+8,statusCenterY-statusRadius+8,(statusRadius-8)*2,(statusRadius-8)*2,0,360*64);
	if(progressAngle>0)
		{
		setColor(colorSuccess);
		XDrawArc(display,window,graphicsContext,statusCenterX-statusRadius+8,statusCenterY-statusRadius+8,(statusRadius-8)*2,(statusRadius-8)*2,90*64,-progressAngle);
		}
	XSetLineAttributes(display,graphicsContext,1,LineSolid,CapButt,JoinMiter);
	setColor(colorBorder);
	XDrawArc(display,window,graphicsContext,statusCenterX-statusRadius,statusCenterY-statusRadius,statusRadius*2,statusRadius*2,0,360*64);

	const char* statusText=complete?"Complete":"In Progress";
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	const int statusTextWidth=headingFont!=0?XTextWidth(headingFont,statusText,int(strlen(statusText))):0;
	setColor(complete?colorSuccess:colorWarning);
	drawTextLine(statusCenterX-statusTextWidth/2,statusCenterY-10,statusText);

	char tiePointMessage[96];
	snprintf(tiePointMessage,sizeof(tiePointMessage),"Tie Points Collected: %u / 12",tiePointCount);
	if(emphasisFont!=0)
		XSetFont(display,graphicsContext,emphasisFont->fid);
	const int tiePointMessageWidth=emphasisFont!=0?XTextWidth(emphasisFont,tiePointMessage,int(strlen(tiePointMessage))):0;
	setColor(colorText);
	drawTextLine(statusCenterX-tiePointMessageWidth/2,statusCenterY+45,tiePointMessage);
	
	if(titleFont!=0)
		XSetFont(display,graphicsContext, headingFont->fid);
	setColor(colorAccent);
	drawTextLine(120,340,"Calibration Steps:");

	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	drawTextLine(120,390,"1. Align the disk");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorText);
	drawTextLine(145,425,"Place the calibration disk flat above the sand and keep the disk still.");
	drawTextLine(145,455,"Move the disk until the white crosshair in the software is centered with the disk crosshair.");

	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	drawTextLine(120,500,"2. Capture the tie point");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorText);
	drawTextLine(145,535,"Hold the disk still (steady and flat) and press \"Capture Tie Point\" to record current position.");

	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	drawTextLine(120,580,"3. Move the disk to next location on the stream table");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorText);
	drawTextLine(145,615,"Capture at least 12 tie points across the stream table, including flat areas and raised sand.");
	drawTextLine(145,645,"This includes tie points inside holes/above sand so both high and low surface levels are recorded.");

	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorWarning);
	drawTextLine(120,690,"4. Optional: Capture below the sand");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorWarning);
	drawTextLine(145,725,"Dig a hole in the sand, then press \"Re-Capture Background\" and wait for the screen to turn red and return to normal.");
	drawTextLine(145,755,"Place the disk flat at the bottom of the hole, center the white crosshair on it, and press \"Capture Tie Point\".");

	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorSuccess);
	drawTextLine(120,800,"5. Verify calibration");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorSuccess);
	drawTextLine(145,835,"After at least 12 tie points, place the disk anywhere in the stream table.");
	drawTextLine(145,865,"Confirm the red crosshair is aligned with the disk (a green circle should appear on the disk).");
	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorText);
	drawTextLine(145,905,"Then");
	setColor(colorSuccess);
	drawTextLine(200,905,"Save Projector Calibration");
	setColor(colorText);
	drawTextLine(475,905,"to save all tie points");

	if(headingFont!=0)
		XSetFont(display,graphicsContext,headingFont->fid);
	setColor(colorWarning);
	drawTextLine(120,950,"6. Optional: Continue adding tie points");
	if(bodyFont!=0)
		XSetFont(display,graphicsContext,bodyFont->fid);
	setColor(colorWarning);
	drawTextLine(145,985,"Repeat steps 1-4 until satisfied with the calibration accuracy.");

	if(showCalibrationTipsTooltip)
		drawTipsTooltip(calibrationTipsIconRect);
	
	XFlush(display);
	}

bool ProjectorCalibrationWindow::processEvents(void)
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
				if(captureButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					capturePointRequested=true;
					captureButtonFlash=true;
					}
				else if(recaptureBackgroundButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					recaptureBackgroundRequested=true;
					recaptureButtonFlash=true;
					}
				else if(exitButtonRect.contains(event.xbutton.x,event.xbutton.y))
					closeRequested=true;
				else if(resetButtonRect.contains(event.xbutton.x,event.xbutton.y))
					resetCalibrationRequested=true;
				else if(tiePointCount>=minTiePointsRequired&&runStreamTableButtonRect.contains(event.xbutton.x,event.xbutton.y))
					{
					finishCalibrationRequested=true;
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

bool ProjectorCalibrationWindow::consumeCapturePointRequest(void)
	{
	bool result=capturePointRequested;
	capturePointRequested=false;
	return result;
	}

bool ProjectorCalibrationWindow::consumeRecaptureBackgroundRequest(void)
	{
	bool result=recaptureBackgroundRequested;
	recaptureBackgroundRequested=false;
	return result;
	}


bool ProjectorCalibrationWindow::consumeResetCalibrationRequest(void)
	{
	bool result=resetCalibrationRequested;
	resetCalibrationRequested=false;
	return result;
	}

bool ProjectorCalibrationWindow::consumeFinishCalibrationRequest(void)
	{
	bool result=finishCalibrationRequested;
	finishCalibrationRequested=false;
	return result;
	}

void ProjectorCalibrationWindow::setTiePointCount(unsigned int newTiePointCount)
	{
	if(tiePointCount==newTiePointCount)
		return;
	tiePointCount=newTiePointCount;
	draw();
	}