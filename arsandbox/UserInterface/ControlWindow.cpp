#include "ControlWindow.h"
#include "SensorUtility.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdexcept>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

namespace
	{
	std::string trimWhitespace(const std::string& value)
		{
		std::string::size_type begin=0;
		while(begin<value.size()&&isspace(static_cast<unsigned char>(value[begin]))!=0)
			++begin;
		std::string::size_type end=value.size();
		while(end>begin&&isspace(static_cast<unsigned char>(value[end-1]))!=0)
			--end;
		return value.substr(begin,end-begin);
		}

	bool isValidViewNameCharacter(unsigned char c)
		{
		return c>=32&&c!='/'&&c!='\\'&&c!=':'&&c!='*'&&c!='?'&&c!='"'&&c!='<'&&c!='>'&&c!='|';
		}
	}

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
	if(exportDialogVisible)
		{
		const Rect cancelRect=scaledRect(exportDialogCancelRect);
		const Rect okRect=scaledRect(exportDialogOkRect);
		const Rect inputRect=scaledRect(exportDialogInputRect);
		return cancelRect.contains(x,y)||okRect.contains(x,y)||inputRect.contains(x,y);
		}
	return scaledRect(exitButtonRect).contains(x,y)||scaledRect(freezeButtonRect).contains(x,y)||scaledRect(exportButtonRect).contains(x,y)||
	       scaledRect(drainButtonRect).contains(x,y)||
	       scaledRect(contourIntervalButton075Rect).contains(x,y)||scaledRect(contourIntervalButton1Rect).contains(x,y)||
	       scaledRect(contourIntervalButton2Rect).contains(x,y)||scaledRect(contourIntervalButton4Rect).contains(x,y)||
	       scaledRect(testingCheckboxRect).contains(x,y)||scaledRect(testingSliderRect).contains(x,y);
	}

unsigned long ControlWindow::resolveButtonFill(bool active,bool hovered) const
	{
	if(active)
		return colorButtonActive;
	if(hovered)
		return colorButtonHover;
	return colorButton;
	}

void ControlWindow::setFont(XFontStruct* font)
{
	if(font!=0)
		XSetFont(display,graphicsContext,font->fid);
	}

void ControlWindow::drawCenteredText(const Rect& rect,int baselineY,const char* label,XFontStruct* font,unsigned long color)
	{
	setFont(font);
	const int textWidth=XTextWidth(font,label,int(strlen(label)));
	setColor(color);
	XDrawString(display,window,graphicsContext,rect.x+(rect.w-textWidth)/2,baselineY,label,int(strlen(label)));
	}

void ControlWindow::drawButton(const Rect& rect,const char* label,bool active,bool hovered,unsigned long fillColorOverride,unsigned long textColorOverride)
	{
	setColor(fillColorOverride!=0?fillColorOverride:resolveButtonFill(active,hovered));
	XFillRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	setColor(colorButtonBorder);
	XDrawRectangle(display,window,graphicsContext,rect.x,rect.y,rect.w,rect.h);
	drawCenteredText(rect, rect.y+rect.h/2+6,label,activeSectionFont(),textColorOverride!=0?textColorOverride:colorText);
	}


void ControlWindow::applyWindowState(void)
	{
	const int width=currentWindowWidth();
	const int height=currentWindowHeight();
	XResizeWindow(display,window,width,height);
	XSizeHints sizeHints;
	sizeHints.flags=PMinSize|PMaxSize;
	sizeHints.min_width=width;
	sizeHints.max_width=width;
	sizeHints.min_height=height;
	sizeHints.max_height=height;
	XSetWMNormalHints(display,window,&sizeHints);
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
void ControlWindow::openExportDialog(void)
	{
	exportDialogVisible=true;
	exportDialogErrorMessage.clear();
	hoverInteractive=false;
	XDefineCursor(display,window,arrowCursor);
	}

void ControlWindow::closeExportDialog(void)
	{
	exportDialogVisible=false;
	exportDialogErrorMessage.clear();
	exportNameInput.clear();
	}

void ControlWindow::showExportDialogError(const std::string& message,const std::string& attemptedName)
	{
	exportDialogVisible=true;
	exportDialogErrorMessage=message;
	if(!attemptedName.empty())
		exportNameInput=attemptedName;
	statusMessage.clear();
	exportRequested=false;
	exportInProgress=false;
	exportStatus=EXPORT_IDLE;
	draw();
	}

void ControlWindow::appendExportInput(char c)
	{
	if(exportNameInput.size()>=exportNameMaxLength)
		{
		exportDialogErrorMessage="Name is too long";
		return;
		}
	if(!isValidViewNameCharacter(static_cast<unsigned char>(c)))
		{
		exportDialogErrorMessage="Use letters, numbers, spaces, - or _";
		return;
		}
	exportNameInput.push_back(c);
	exportDialogErrorMessage.clear();
	}

void ControlWindow::eraseExportInput(void)
	{
	if(!exportNameInput.empty())
		exportNameInput.erase(exportNameInput.size()-1);
	exportDialogErrorMessage.clear();
	}

void ControlWindow::submitExportDialog(void)
	{
	std::string trimmedName=trimWhitespace(exportNameInput);
	if(trimmedName.empty())
		{
		exportDialogErrorMessage="Please enter a view name";
		return;
		}
	for(std::string::const_iterator it=trimmedName.begin();it!=trimmedName.end();++it)
		if(!isValidViewNameCharacter(static_cast<unsigned char>(*it)))
			{
			exportDialogErrorMessage="Name contains invalid characters";
			return;
			}
	exportRequestName=trimmedName;
	exportRequested=true;
	exportInProgress=true;
	exportStatus=EXPORT_PENDING;
	statusMessage.clear();
	closeExportDialog();
	}

void ControlWindow::drawExportDialog(void)
	{
	const int width=currentWindowWidth();
	const int height=currentWindowHeight();
	const Rect dialogRect=scaledRect(exportDialogRect);
	const Rect inputRect=scaledRect(exportDialogInputRect);
	const Rect cancelRect=scaledRect(exportDialogCancelRect);
	const Rect okRect=scaledRect(exportDialogOkRect);
	XFontStruct* titleDisplayFont=activeTitleFont();
	XFontStruct* section=activeSectionFont();

	setColor(colorOverlay);
	XFillRectangle(display,window,graphicsContext,0,0,width,height);

	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,dialogRect.x,dialogRect.y,dialogRect.w,dialogRect.h);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,dialogRect.x,dialogRect.y,dialogRect.w,dialogRect.h);

	const char* title="Export Topography";
	setFont(titleDisplayFont);
	setColor(colorText);
	XDrawString(display,window,graphicsContext,dialogRect.x+24,dialogRect.y+52,title,int(strlen(title)));

	const char* prompt="Please name your view";
	setFont(section);
	XDrawString(display,window,graphicsContext,dialogRect.x+24,dialogRect.y+102,prompt,int(strlen(prompt)));

	setColor(colorInputBackground);
	XFillRectangle(display,window,graphicsContext,inputRect.x,inputRect.y,inputRect.w,inputRect.h);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,inputRect.x,inputRect.y,inputRect.w,inputRect.h);
	setColor(colorText);
	setFont(section);
	XDrawString(display,window,graphicsContext,inputRect.x+12,inputRect.y+30,exportNameInput.c_str(),int(exportNameInput.size()));
	if(exportDialogVisible)
		{
		int textWidth=XTextWidth(section,exportNameInput.c_str(),int(exportNameInput.size()));
		int cursorX=inputRect.x+12+textWidth;
		XDrawLine(display,window,graphicsContext,cursorX,inputRect.y+8,cursorX,inputRect.y+30);
		}

	if(!exportDialogErrorMessage.empty())
		{
		setColor(colorError);
		XDrawString(display,window,graphicsContext,dialogRect.x+24,dialogRect.y+256,exportDialogErrorMessage.c_str(),int(exportDialogErrorMessage.size()));
		}

	drawButton(cancelRect,"Cancel",false,cancelRect.contains(hoverX,hoverY));
	drawButton(okRect,"OK",false,okRect.contains(hoverX,hoverY),okRect.contains(hoverX,hoverY)?colorOkButtonHover:colorOkButton,colorText);
	}

void ControlWindow::draw(void)
	{
	const int width=currentWindowWidth();
	const int height=currentWindowHeight();
	const Rect exitRect=scaledRect(exitButtonRect);
	const Rect freezeRect=scaledRect(freezeButtonRect);
	const Rect exportRect=scaledRect(exportButtonRect);
	const Rect drainRect=scaledRect(drainButtonRect);
	const Rect contourLabelRect=scaledRect(contourIntervalLabelRect);
	const Rect contour075Rect=scaledRect(contourIntervalButton075Rect);
	const Rect contour1Rect=scaledRect(contourIntervalButton1Rect);
	const Rect contour2Rect=scaledRect(contourIntervalButton2Rect);
	const Rect contour4Rect=scaledRect(contourIntervalButton4Rect);
	const Rect testingLabelRectScaled=scaledRect(testingLabelRect);
	const Rect testingCheckboxRectScaled=scaledRect(testingCheckboxRect);
	const Rect testingSliderRectScaled=scaledRect(testingSliderRect);
	XFontStruct* title=activeTitleFont();
	XFontStruct* section=activeSectionFont();
	XFontStruct* stat=activeStatFont();

	setFont(section);
	setColor(colorBackground);
	XFillRectangle(display,window,graphicsContext,0,0,width,height);

	setColor(colorPanel);
	XFillRectangle(display,window,graphicsContext,0,0,width,50);
	setColor(colorBorder);
	XDrawLine(display,window,graphicsContext,0,50,width,50);

	setColor(colorText);
	setFont(title);
	XDrawString(display,window,graphicsContext,10,38,"Control Window V4",17);

	setFont(title);
	setColor(colorSubtleText);
	XDrawString(display,window,graphicsContext,10,98,"Control for Stream Table",24);
	const char* drainButtonLabel="Hold to Drain Digital Water";
	drawButton(drainRect,drainButtonLabel,removeWaterOn,drainRect.contains(hoverX,hoverY));
	drawCenteredText(contourLabelRect,contourLabelRect.y+30,"Adjust Contour Interval:",section,colorText);
	drawButton(contour075Rect,"0.75 cm",fabs(contourLineInterval-0.75)<1.0e-6,contour075Rect.contains(hoverX,hoverY));
	drawButton(contour1Rect,"1 cm",fabs(contourLineInterval-1.0)<1.0e-6,contour1Rect.contains(hoverX,hoverY));
	drawButton(contour2Rect,"2 cm",fabs(contourLineInterval-2.0)<1.0e-6,contour2Rect.contains(hoverX,hoverY));
	drawButton(contour4Rect,"4 cm",fabs(contourLineInterval-4.0)<1.0e-6,contour4Rect.contains(hoverX,hoverY));

	// Test
	//Arduino loop
	float previousSensorAngleValue = sensorAngleValue;
	if(arduinoSensor->isActive())
		sensorAngleValue = arduinoSensor->getRoll();
	if (std::abs(sensorAngleValue - previousSensorAngleValue) < 0.15f) {
        sensorAngleValue = previousSensorAngleValue;
    }
	appliedAngleValue=testingEnabled?testingTiltValue:int(sensorAngleValue* 10.0f) / 10.0f;

	drawCenteredText(testingLabelRectScaled,testingLabelRectScaled.y+30,"Just for SEECS testing",section,colorSubtleText);
	setColor(colorBorder);
	XDrawRectangle(display,window,graphicsContext,testingCheckboxRectScaled.x,testingCheckboxRectScaled.y,testingCheckboxRectScaled.w,testingCheckboxRectScaled.h);
	if(testingEnabled)
		{
		setColor(colorSuccess);
		XDrawLine(display,window,graphicsContext,testingCheckboxRectScaled.x+3,testingCheckboxRectScaled.y+testingCheckboxRectScaled.h/2,
		          testingCheckboxRectScaled.x+testingCheckboxRectScaled.w/2,testingCheckboxRectScaled.y+testingCheckboxRectScaled.h-4);
		XDrawLine(display,window,graphicsContext,testingCheckboxRectScaled.x+testingCheckboxRectScaled.w/2,testingCheckboxRectScaled.y+testingCheckboxRectScaled.h-4,
		          testingCheckboxRectScaled.x+testingCheckboxRectScaled.w-3,testingCheckboxRectScaled.y+3);
		}
	setColor(colorText);
	setFont(section);
	XDrawString(display,window,graphicsContext,testingCheckboxRectScaled.x+testingCheckboxRectScaled.w+12,testingCheckboxRectScaled.y+testingCheckboxRectScaled.h-6,"Enable for testing just for SEECS",33);
	const int clampedTilt=testingTiltValue<testingTiltMin?testingTiltMin:(testingTiltValue>testingTiltMax?testingTiltMax:testingTiltValue);
	setColor(testingEnabled?colorButtonBorder:colorBorder);
	XDrawRectangle(display,window,graphicsContext,testingSliderRectScaled.x,testingSliderRectScaled.y+testingSliderRectScaled.h/2-3,testingSliderRectScaled.w,6);
	setColor(testingEnabled?colorAccent:colorBorder);
	const int sliderFillWidth=((clampedTilt-testingTiltMin)*testingSliderRectScaled.w)/(testingTiltMax-testingTiltMin);
	XFillRectangle(display,window,graphicsContext,testingSliderRectScaled.x,testingSliderRectScaled.y+testingSliderRectScaled.h/2-2,sliderFillWidth,4);
	const int sliderY=testingSliderRectScaled.y+testingSliderRectScaled.h/2;
	const int handleX=testingSliderRectScaled.x+((clampedTilt-testingTiltMin)*testingSliderRectScaled.w)/(testingTiltMax-testingTiltMin);
	setColor(testingEnabled?colorText:colorSubtleText);
	XFillArc(display,window,graphicsContext,handleX-8,sliderY-8,16,16,0,360*64);
	char sliderBuffer[80];
	snprintf(sliderBuffer,sizeof(sliderBuffer),"Mock Tilt: %d deg",testingTiltValue);
	setColor(testingEnabled?colorText:colorSubtleText);
	XDrawString(display,window,graphicsContext,testingSliderRectScaled.x,testingSliderRectScaled.y-8,sliderBuffer,int(strlen(sliderBuffer)));
	XDrawString(display,window,graphicsContext,testingSliderRectScaled.x,testingSliderRectScaled.y+testingSliderRectScaled.h+16,"0 deg",5);
	XDrawString(display,window,graphicsContext,testingSliderRectScaled.x+testingSliderRectScaled.w-36,testingSliderRectScaled.y+testingSliderRectScaled.h+16,"22 deg",6);
	/* Split layout in two panes */
	setColor(WhitePixel(display,DefaultScreen(display)));
	XDrawLine(display,window,graphicsContext,width/2,62,width/2,height-16);

	drawButton(exitRect,"Exit",false,exitRect.contains(hoverX,hoverY),colorError,colorText);
	Rect fpsBase={540,58,444,40};
	Rect angleBase={540,98,444,40};
	const Rect fpsRect=scaledRect(fpsBase);
	const Rect angleRect=scaledRect(angleBase);
	char fpsBuffer[48];
	snprintf(fpsBuffer,sizeof(fpsBuffer),"FPS: %d",currentFps);
	XFontStruct* mediumLabelFont=titleFontLarge!=0?titleFontLarge:(statFont!=0?statFont:section);
	drawCenteredText(fpsRect,fpsRect.y+35,fpsBuffer,mediumLabelFont,colorText);
	char angleBuffer[48];
	snprintf(angleBuffer,sizeof(angleBuffer),"Current Table Tilt: %.1f",appliedAngleValue);
	drawCenteredText(angleRect,angleRect.y+35,angleBuffer,mediumLabelFont,colorSuccess);

	/* Topography toggle and export button layout */
	char freezeLabel[64];
	snprintf(freezeLabel,sizeof(freezeLabel),"Freeze Topography: %s",freezeOn?"ON":"OFF");
	drawButton(freezeRect,freezeLabel,freezeOn,freezeRect.contains(hoverX,hoverY));
	drawButton(exportRect,"Export Topography",exportInProgress,exportRect.contains(hoverX,hoverY));
	
	/*Screenshot message success or failure*/
	if(!statusMessage.empty())
		{
		setColor(exportStatus==EXPORT_ERROR?colorError:(exportStatus==EXPORT_SUCCESS?colorSuccess:colorSubtleText));
		Rect statusBase={540,222,444,44};
		const Rect statusRect=scaledRect(statusBase);
		drawCenteredText(statusRect,statusRect.y-20,statusMessage.c_str(),statFont!=0?statFont:section,exportStatus==EXPORT_ERROR?colorError:(exportStatus==EXPORT_SUCCESS?colorSuccess:colorSubtleText));
		}

	Rect tiltLabelBase={540,214,444,56};
	const Rect tiltLabelRect=scaledRect(tiltLabelBase);
	drawCenteredText(tiltLabelRect,tiltLabelRect.y+36,"Current Table Tilt",title,colorText);

	Rect indicatorBase={670,266,180,180};
	const Rect indicatorRect=scaledRect(indicatorBase);
	const int centerX=indicatorRect.x+indicatorRect.w/2;
	const int centerY=indicatorRect.y+indicatorRect.h/2;
	const int radius=(indicatorRect.w<indicatorRect.h?indicatorRect.w:indicatorRect.h)/2;
	setColor(colorSuccess);
	XFillArc(display,window,graphicsContext,centerX-radius,centerY-radius,radius*2,radius*2,0,360*64);
	setColor(colorBorder);
	XDrawArc(display,window,graphicsContext,centerX-radius,centerY-radius,radius*2,radius*2,0,360*64);
	char tiltValue[32];
	snprintf(tiltValue,sizeof(tiltValue),"%.1f",appliedAngleValue);
	XFontStruct* largeValueFont=statFontLarge!=0?statFontLarge:(titleFontLarge!=0?titleFontLarge:stat);
	setFont(largeValueFont);
	const int valueWidth=XTextWidth(largeValueFont,tiltValue,int(strlen(tiltValue)));
	setColor(colorText);
	XDrawString(display,window,graphicsContext,centerX-valueWidth/2,centerY+14,tiltValue,int(strlen(tiltValue)));

	if(exportDialogVisible)
		drawExportDialog();

	XFlush(display);
	}

ControlWindow::ControlWindow(void)
	:display(0),window(0),graphicsContext(0),wmDeleteWindow(None),arrowCursor(None),handCursor(None),closeRequested(false),
	 waterSimulationOn(false),freezeOn(false),exportRequested(false),exportInProgress(false),isMaximized(true),removeWaterOn(false),contourLineInterval(0.75),hoverInteractive(false),exportDialogVisible(false),hoverX(0),hoverY(0),
	 waterFlowRate(0.0),appliedAngleValue(0),sensorAngleValue(0),testingTiltValue(0),testingEnabled(false),testingSliderDragging(false), currentFps(0), arduinoSensor(0),
	 titleFont(0),sectionFont(0),statFont(0),titleFontLarge(0),sectionFontLarge(0),statFontLarge(0),
	 colorBackground(0),colorPanel(0),colorBorder(0),colorButton(0),
	 colorButtonActive(0),colorButtonHover(0),colorButtonBorder(0),colorText(0),colorSubtleText(0),colorAccent(0),colorSuccess(0),colorError(0),colorOverlay(0),colorInputBackground(0),colorOkButton(0),colorOkButtonHover(0),
	 exportRequestName(),exportNameInput(),exportDialogErrorMessage(),statusMessage(),
	 exportStatus(EXPORT_IDLE)
	{
	display=XOpenDisplay(0);
	if(display==0)
		throw std::runtime_error("Unable to open X11 display for control window");

	const int screen=DefaultScreen(display);
	window=XCreateSimpleWindow(display,RootWindow(display,screen),80,80,minimizedWidth,minimizedHeight,1,BlackPixel(display,screen),BlackPixel(display,screen));
	XStoreName(display,window,"GUI for Stream Table Control");
	XSelectInput(display,window,ExposureMask|StructureNotifyMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|EnterWindowMask|LeaveWindowMask|KeyPressMask);

	wmDeleteWindow=XInternAtom(display,"WM_DELETE_WINDOW",False);
	XSetWMProtocols(display,window,&wmDeleteWindow,1);

	graphicsContext=XCreateGC(display,window,0,0);
	arrowCursor=XCreateFontCursor(display,XC_left_ptr);
	handCursor=XCreateFontCursor(display,XC_hand2);
	XDefineCursor(display,window,arrowCursor);

	titleFont=XLoadQueryFont(display,"10x20");
	sectionFont=XLoadQueryFont(display,"9x15");
	statFont=XLoadQueryFont(display,"12x24");
	titleFontLarge=XLoadQueryFont(display,"-misc-fixed-bold-r-normal--34-*-*-*-*-*-*-*");
	sectionFontLarge=XLoadQueryFont(display,"10x20");
	statFontLarge=XLoadQueryFont(display,"-misc-fixed-bold-r-normal--70-*-*-*-*-*-*-*");
	if(statFontLarge==0)
		statFontLarge=XLoadQueryFont(display,"-misc-fixed-bold-r-normal--48-*-*-*-*-*-*-*");
	if(titleFont==0)
		titleFont=XQueryFont(display,XGContextFromGC(DefaultGC(display,screen)));
	if(sectionFont==0)
		sectionFont=titleFont;
	if(statFont==0)
		statFont=titleFont;
	if(titleFontLarge==0)
		titleFontLarge=titleFont;
	if(sectionFontLarge==0)
		sectionFontLarge=sectionFont;
	if(statFontLarge==0)
		statFontLarge=statFont;
	setFont(sectionFont);

	applyWindowState();

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

	colorOverlay=allocColor("#111318",BlackPixel(display,screen));
	colorInputBackground=allocColor("#111318",BlackPixel(display,screen));
	colorOkButton=allocColor("#2e7d32",WhitePixel(display,screen));
	colorOkButtonHover=allocColor("#37933c",WhitePixel(display,screen));

	//Arduino tilt
	arduinoSensor = new SensorUtility("/dev/ttyUSB0", 115200);
	arduinoSensor->start();

	XMapWindow(display,window);
	XFlush(display);
	}

ControlWindow::~ControlWindow(void)
	{
	if(display!=0)
		{
		if(titleFont!=0&&titleFont!=sectionFont&&titleFont!=statFont)
			XFreeFont(display,titleFont);
		if(sectionFont!=0&&sectionFont!=titleFont&&sectionFont!=statFont)
			XFreeFont(display,sectionFont);
		if(statFont!=0&&statFont!=titleFont&&statFont!=sectionFont)
			XFreeFont(display,statFont);
		if(titleFontLarge!=0&&titleFontLarge!=titleFont&&titleFontLarge!=sectionFont&&titleFontLarge!=statFont)
			XFreeFont(display,titleFontLarge);
		if(sectionFontLarge!=0&&sectionFontLarge!=titleFont&&sectionFontLarge!=sectionFont&&sectionFontLarge!=statFont&&sectionFontLarge!=titleFontLarge)
			XFreeFont(display,sectionFontLarge);
		if(statFontLarge!=0&&statFontLarge!=titleFont&&statFontLarge!=sectionFont&&statFontLarge!=statFont&&statFontLarge!=titleFontLarge&&statFontLarge!=sectionFontLarge)
			XFreeFont(display,statFontLarge);
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
		draw();
		}
	}

void ControlWindow::setWaterFlowRate(double newWaterFlowRate)
	{
	if(newWaterFlowRate<0.0)
		newWaterFlowRate=0.0;
	if(newWaterFlowRate>1.0)
		newWaterFlowRate=1.0;
	if(waterFlowRate!=newWaterFlowRate)
		{
		waterFlowRate=newWaterFlowRate;
		draw();
		}
	}

void ControlWindow::setCurrentFps(int newCurrentFps)
	{
	if(newCurrentFps<0)
		newCurrentFps=0;
	if(currentFps!=newCurrentFps)
		{
		currentFps=newCurrentFps;
		draw();
		}
	}

bool ControlWindow::consumeExportRequest(std::string& requestedName)
	{
	if(!exportRequested)
		return false;
	exportRequested=false;
	requestedName=exportRequestName;
	exportRequestName.clear();
	return true;
	}

void ControlWindow::setExportStatus(ExportStatus newStatus,const std::string& message)
	{
	exportStatus=newStatus;
	exportInProgress=newStatus==EXPORT_PENDING;
	statusMessage=message;
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
			if(event.type==MotionNotify&&testingSliderDragging)
				{
				const Rect testingSlider=scaledRect(testingSliderRect);
				const int dx=mx-testingSlider.x;
				testingTiltValue=testingTiltMin+(dx*(testingTiltMax-testingTiltMin))/testingSlider.w;
				if(testingTiltValue<testingTiltMin)
					testingTiltValue=testingTiltMin;
				if(testingTiltValue>testingTiltMax)
					testingTiltValue=testingTiltMax;
				}
			updateCursor(mx,my);
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
				const Rect exitRect=scaledRect(exitButtonRect);
				const Rect freezeRect=scaledRect(freezeButtonRect);
				const Rect exportRect=scaledRect(exportButtonRect);
			const Rect drainRect=scaledRect(drainButtonRect);
			const Rect contour075Rect=scaledRect(contourIntervalButton075Rect);
			const Rect contour1Rect=scaledRect(contourIntervalButton1Rect);
			const Rect contour2Rect=scaledRect(contourIntervalButton2Rect);
			const Rect contour4Rect=scaledRect(contourIntervalButton4Rect);
			const Rect testingCheckbox=scaledRect(testingCheckboxRect);
			const Rect testingSlider=scaledRect(testingSliderRect);
				const Rect cancelRect=scaledRect(exportDialogCancelRect);
				const Rect okRect=scaledRect(exportDialogOkRect);
				if(exportDialogVisible)
					{
				if(cancelRect.contains(x,y))
					closeExportDialog();
				else if(okRect.contains(x,y))
					submitExportDialog();
					}
			else if(exitRect.contains(x,y))
				closeRequested=true;
			else if(freezeRect.contains(x,y))
				freezeOn=!freezeOn;
			else if(exportRect.contains(x,y))
				{
				exportNameInput.clear();
				openExportDialog();
				}
			else if(drainRect.contains(x,y))
				{
				removeWaterOn=true;
				}
			else if(contour075Rect.contains(x,y))
				contourLineInterval=0.75;
			else if(contour1Rect.contains(x,y))
				contourLineInterval=1.0;
			else if(contour2Rect.contains(x,y))
				contourLineInterval=2.0;
			else if(contour4Rect.contains(x,y))
				contourLineInterval=4.0;
			else if(testingCheckbox.contains(x,y))
				{
				testingEnabled=!testingEnabled;
				}
			else if(testingEnabled&&testingSlider.contains(x,y))
				{
				const int dx=x-testingSlider.x;
				testingTiltValue=testingTiltMin+(dx*(testingTiltMax-testingTiltMin))/testingSlider.w;
				if(testingTiltValue<testingTiltMin)
					testingTiltValue=testingTiltMin;
				if(testingTiltValue>testingTiltMax)
					testingTiltValue=testingTiltMax;
				testingSliderDragging=true;
				}
			draw();
			}
		else if(event.type==ButtonRelease)
			{
			if(event.xbutton.button==Button1)
				removeWaterOn=false;
			testingSliderDragging=false;
			draw();
			}
		else if(event.type==KeyPress)
			{
			if(exportDialogVisible)
				{
				KeySym keySym=NoSymbol;
				char buffer[16];
				const int count=XLookupString(&event.xkey,buffer,sizeof(buffer),&keySym,0);
				if(keySym==XK_Return||keySym==XK_KP_Enter)
					submitExportDialog();
				else if(keySym==XK_Escape)
					closeExportDialog();
				else if(keySym==XK_BackSpace||keySym==XK_Delete)
					eraseExportInput();
				else
					for(int i=0;i<count;++i)
						if(buffer[i]>=32&&buffer[i]<127)
							appendExportInput(buffer[i]);
				draw();
				}
			}
		else if(event.type==ClientMessage)
			{
			if(Atom(event.xclient.data.l[0])==wmDeleteWindow)
				closeRequested=true;
			}
		else if(event.type==DestroyNotify)
			closeRequested=true;
		}
	if(closeRequested == true){
		arduinoSensor->stop();
	}

	return closeRequested;
	}

const ControlWindow::Rect ControlWindow::exitButtonRect={844,468,140,40};
const ControlWindow::Rect ControlWindow::freezeButtonRect={540,156,210,36};
const ControlWindow::Rect ControlWindow::exportButtonRect={774,156,210,36};
const ControlWindow::Rect ControlWindow::drainButtonRect={24,130,420,56};
const ControlWindow::Rect ControlWindow::contourIntervalLabelRect={24,206,420,40};
const ControlWindow::Rect ControlWindow::contourIntervalButton075Rect={24,250,206,56};
const ControlWindow::Rect ControlWindow::contourIntervalButton1Rect={238,250,206,56};
const ControlWindow::Rect ControlWindow::contourIntervalButton2Rect={24,314,206,56};
const ControlWindow::Rect ControlWindow::contourIntervalButton4Rect={238,314,206,56};
const ControlWindow::Rect ControlWindow::testingLabelRect={24,392,420,36};
const ControlWindow::Rect ControlWindow::testingCheckboxRect={24,438,16,16};
const ControlWindow::Rect ControlWindow::testingSliderRect={24,480,320,36};
const ControlWindow::Rect ControlWindow::exportDialogRect={300,150,420,220};
const ControlWindow::Rect ControlWindow::exportDialogInputRect={320,235,380,34};
const ControlWindow::Rect ControlWindow::exportDialogCancelRect={320,315,120,34};
const ControlWindow::Rect ControlWindow::exportDialogOkRect={580,315,120,34};


bool ControlWindow::getDrainState(void) const
	{
	return removeWaterOn;
	}

double ControlWindow::getContourLineInterval(void) const
	{
	return contourLineInterval;
	}

float ControlWindow::getAppliedTiltValue(void) const
	{
	return testingEnabled?testingTiltValue:sensorAngleValue;
	}

double ControlWindow::getWaterFlowRate(void) const
	{
	return 0.0;
	}

int ControlWindow::currentWindowWidth(void) const
	{
	return isMaximized?maximizedWidth:minimizedWidth;
	}

int ControlWindow::currentWindowHeight(void) const
	{
	return isMaximized?maximizedHeight:minimizedHeight;
	}

XFontStruct* ControlWindow::activeTitleFont(void) const
	{
	return isMaximized&&titleFontLarge!=0?titleFontLarge:titleFont;
	}

XFontStruct* ControlWindow::activeSectionFont(void) const
	{
	return isMaximized&&sectionFontLarge!=0?sectionFontLarge:sectionFont;
	}

XFontStruct* ControlWindow::activeStatFont(void) const
	{
	return isMaximized&&statFontLarge!=0?statFontLarge:statFont;
	}

ControlWindow::Rect ControlWindow::scaledRect(const Rect& rect) const
	{
	const double sx=double(currentWindowWidth())/double(minimizedWidth);
	const double sy=double(currentWindowHeight())/double(minimizedHeight);
	Rect result;
	result.x=int(rect.x*sx+0.5);
	result.y=int(rect.y*sy+0.5);
	result.w=int(rect.w*sx+0.5);
	result.h=int(rect.h*sy+0.5);
	return result;
	}