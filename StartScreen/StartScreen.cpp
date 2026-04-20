//Jake's startup menu (very cool)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <iostream>
#include <cstdlib>

struct Rect {
    int x, y, w, h;
    bool contains(int px, int py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

unsigned long allocColor(Display* display, const char* hex, unsigned long fallback) {
    Colormap cmap = DefaultColormap(display, DefaultScreen(display));
    XColor color;

    if (XParseColor(display, cmap, hex, &color) &&
        XAllocColor(display, cmap, &color)) {
        return color.pixel;
    }

    return fallback;
}





int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display\n";
        return 1;

    }

    int screen = DefaultScreen(display);

    unsigned long colorBackground = allocColor(display,"#0b0f19", BlackPixel(display,screen));
    unsigned long colorButton = allocColor(display,"#274a88", WhitePixel(display,screen));
    unsigned long colorButtonHover = allocColor(display,"#3262b4", WhitePixel(display,screen));
    unsigned long colorBorder = allocColor(display,"#2d3f64", WhitePixel(display,screen));
    unsigned long colorText = allocColor(display,"#ecf2ff", WhitePixel(display,screen));
    unsigned long colorSubtleText = allocColor(display,"#9db3d9", WhitePixel(display,screen));
    unsigned long colorPanel = allocColor(display,"#121a2b", WhitePixel(display,screen));
    unsigned long colorAccent = allocColor(display,"#4cc9f0", WhitePixel(display,screen));

    int winWidth = 650;
    int winHeight = 150;

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,
        winWidth, winHeight,
        1,
        colorBorder,
        colorBackground
        

    );

    XStoreName(display, window, "SARndbox Startup");

    XSelectInput(display, window,
        ExposureMask | ButtonPressMask | PointerMotionMask);

    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, nullptr);

    
    XFontStruct* font = XLoadQueryFont(display, "fixed");
    if (font) {
        XSetFont(display, gc, font->fid);
    }

    //Start Button setup
    Rect button;
    button.w = 270;
    button.h = 50;
    button.x = (winWidth - button.w) - 50;
    button.y = (winHeight - button.h) / 2 +25;

    bool hover = false;
    std::string label = "Start";

    // Calibrate Button setup
    Rect calibrateButton;
    calibrateButton.w = 130;
    calibrateButton.h = 25; // about half height of Start button
    calibrateButton.x = button.x; // align horizontally
    calibrateButton.y = button.y - calibrateButton.h - 10; // above Start button

    bool calibrateHover = false;
    std::string calibrateLabel = "Projector Calibration";

    Rect kinectButton;
    kinectButton.w = 130;
    kinectButton.h = calibrateButton.h; // same height
    kinectButton.x = calibrateButton.x + calibrateButton.w + 10; // to the right
    kinectButton.y = calibrateButton.y;

    bool kinectHover = false;
    std::string kinectLabel = "Kinect Calibration";

    //Color Flag
    Rect colorFlag;
    colorFlag.w = 10;
    colorFlag.h = 10;
    colorFlag.x = 20;
    colorFlag.y = 40;

    bool colorFlagHover = false;
    bool colorFlagChecked = true;
    std::string colorFlagLabel = "Elevation Color Flag";

    //Elvation line flag
    Rect lineFlag;
    lineFlag.w = 10;
    lineFlag.h = 10;
    lineFlag.x = colorFlag.x;
    lineFlag.y = colorFlag.y + 25; // below colorFlag with spacing

    bool lineFlagHover = false;
    bool lineFlagChecked = true;
    std::string lineFlagLabel = "Contour Line Flag";

    //Debug - used to be FPV before default
    Rect FPVflag;
    FPVflag.w = 10;
    FPVflag.h = 10;
    FPVflag.x = calibrateButton.x;
    FPVflag.y = calibrateButton.y - 25; // above calibrate button

    bool FPVflagHover = false;
    bool FPVflagChecked = false;
    std::string FPVflagLabel = "Debug Mode";

    //more rain
    Rect heavyRainFlag;
    heavyRainFlag.w = 10;
    heavyRainFlag.h = 10;
    heavyRainFlag.x = colorFlag.x;
    heavyRainFlag.y = lineFlag.y + 25; // below lineFlag

    bool heavyRainHover = false;
    bool heavyRainChecked = false;
    std::string heavyRainLabel = "Heavy Rain Flag";

    //toggle snowmelt
    Rect snowMeltFlag;
    snowMeltFlag.w = 10;
    snowMeltFlag.h = 10;
    snowMeltFlag.x = colorFlag.x;
    snowMeltFlag.y = heavyRainFlag.y + 25; // below heavyRainFlag

    bool snowMeltHover = false;
    bool snowMeltChecked = false;
    std::string snowMeltLabel = "Snow Melt Flag";

    while (true) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == Expose) {
            
            //Start Button
            // Button fill
            XSetForeground(display, gc, hover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                           button.x, button.y, button.w, button.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                           button.x, button.y, button.w, button.h);

            // Text centering
            int textWidth = XTextWidth(font, label.c_str(), label.length());
            int textX = button.x + (button.w - textWidth) / 2;
            int textY = button.y + button.h / 2 + 5;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        textX, textY,
                        label.c_str(), label.length());
            
            // Calibrate Button

            // Fill
            XSetForeground(display, gc, calibrateHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                        calibrateButton.x, calibrateButton.y,
                        calibrateButton.w, calibrateButton.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                        calibrateButton.x, calibrateButton.y,
                        calibrateButton.w, calibrateButton.h);

            // Text centering
            int calTextWidth = XTextWidth(font, calibrateLabel.c_str(), calibrateLabel.length());
            int calTextX = calibrateButton.x + (calibrateButton.w - calTextWidth) / 2;
            int calTextY = calibrateButton.y + calibrateButton.h / 2 + 5;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        calTextX, calTextY,
                        calibrateLabel.c_str(), calibrateLabel.length());
            
            // Kinect Calibration Button

            // Fill
            XSetForeground(display, gc, kinectHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                        kinectButton.x, kinectButton.y,
                        kinectButton.w, kinectButton.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                        kinectButton.x, kinectButton.y,
                        kinectButton.w, kinectButton.h);

            // Text centering
            int kTextWidth = XTextWidth(font, kinectLabel.c_str(), kinectLabel.length());
            int kTextX = kinectButton.x + (kinectButton.w - kTextWidth) / 2;
            int kTextY = kinectButton.y + kinectButton.h / 2 + 5;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        kTextX, kTextY,
                        kinectLabel.c_str(), kinectLabel.length());


            // Color Flag
            // Button fill
            XSetForeground(display, gc, colorButtonHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                           colorFlag.x, colorFlag.y, colorFlag.w, colorFlag.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                           colorFlag.x, colorFlag.y, colorFlag.w, colorFlag.h);

            //TEXT
                int tx = colorFlag.x + 15;
                int ty = colorFlag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        tx, ty,
                        colorFlagLabel.c_str(), colorFlagLabel.length());

            if (colorFlagChecked){
                const char* mark = "x";

                tx = colorFlag.x + 3;
                ty = colorFlag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc,
                        tx, ty,
                        mark, 1);
                XEvent redraw;

            }

            // Line Flag

            // Fill
            XSetForeground(display, gc, lineFlagHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                        lineFlag.x, lineFlag.y, lineFlag.w, lineFlag.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                        lineFlag.x, lineFlag.y, lineFlag.w, lineFlag.h);

            // Text
            int ltx = lineFlag.x + 15;
            int lty = lineFlag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        ltx, lty,
                        lineFlagLabel.c_str(), lineFlagLabel.length());

            // Check mark
            if (lineFlagChecked) {
                const char* mark = "x";

                ltx = lineFlag.x + 3;
                lty = lineFlag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc,
                            ltx, lty,
                            mark, 1);
                XEvent redraw;
            }

            // Debug Flag

            // Fill
            XSetForeground(display, gc, FPVflagHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                        FPVflag.x, FPVflag.y, FPVflag.w, FPVflag.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                        FPVflag.x, FPVflag.y, FPVflag.w, FPVflag.h);

            // Text
            int ftx = FPVflag.x + 15;
            int fty = FPVflag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        ftx, fty,
                        FPVflagLabel.c_str(), FPVflagLabel.length());

            // Check mark
            if (FPVflagChecked) {
                const char* mark = "x";

                ftx = FPVflag.x + 3;
                fty = FPVflag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc,
                            ftx, fty,
                            mark, 1);
                XEvent redraw;
            }
            // Heavy Rain Flag

            // Fill
            XSetForeground(display, gc, heavyRainHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                        heavyRainFlag.x, heavyRainFlag.y,
                        heavyRainFlag.w, heavyRainFlag.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                        heavyRainFlag.x, heavyRainFlag.y,
                        heavyRainFlag.w, heavyRainFlag.h);

            // Text
            int htx = heavyRainFlag.x + 15;
            int hty = heavyRainFlag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        htx, hty,
                        heavyRainLabel.c_str(), heavyRainLabel.length());

            // Check mark
            if (heavyRainChecked) {
                const char* mark = "x";

                htx = heavyRainFlag.x + 3;
                hty = heavyRainFlag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc,
                            htx, hty,
                            mark, 1);
                XEvent redraw;
            }
            // Snow Melt Flag

            // Fill
            XSetForeground(display, gc, snowMeltHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc,
                        snowMeltFlag.x, snowMeltFlag.y,
                        snowMeltFlag.w, snowMeltFlag.h);

            // Border
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc,
                        snowMeltFlag.x, snowMeltFlag.y,
                        snowMeltFlag.w, snowMeltFlag.h);

            // Text
            int stx = snowMeltFlag.x + 15;
            int sty = snowMeltFlag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc,
                        stx, sty,
                        snowMeltLabel.c_str(), snowMeltLabel.length());

            // Check mark
            if (snowMeltChecked) {
                const char* mark = "x";

                stx = snowMeltFlag.x + 3;
                sty = snowMeltFlag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc,
                            stx, sty,
                            mark, 1);
                XEvent redraw;
            }
        }

        else if (event.type == MotionNotify) {
            int mx = event.xmotion.x;
            int my = event.xmotion.y;

            bool newHover = button.contains(mx, my);
            if (newHover != hover) {
                hover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = calibrateButton.contains(mx, my);
            if (newHover != calibrateHover) {
                calibrateHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = colorFlag.contains(mx, my);
            if (newHover != colorFlagHover) {
                colorFlagHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = lineFlag.contains(mx, my);
            if (newHover != lineFlagHover) {
                lineFlagHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = FPVflag.contains(mx, my);
            if (newHover != FPVflagHover) {
                FPVflagHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = heavyRainFlag.contains(mx, my);
            if (newHover != heavyRainHover) {
                heavyRainHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = kinectButton.contains(mx, my);
            if (newHover != kinectHover) {
                kinectHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
            newHover = snowMeltFlag.contains(mx, my);
            if (newHover != snowMeltHover) {
                snowMeltHover = newHover;
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
                
            }
                
        }

        // button events yipee!

        else if (event.type == ButtonPress) {
            int mx = event.xbutton.x;
            int my = event.xbutton.y;

            if (button.contains(mx, my)) {
                

                std::string cmdLine = "cd ../arsandbox/bin && ./SARndbox -fpv";

                //added flags
                if (colorFlagChecked == true){
                    cmdLine = cmdLine + " -uhm";
                }
                if (lineFlagChecked == false){
                cmdLine = cmdLine + " -ncl"; 
                }
                if (FPVflagChecked == true){
                    cmdLine = cmdLine + " -debug";
                }
                if (heavyRainChecked == true){
                    cmdLine = cmdLine + " -rs .75";
                }
                if (snowMeltChecked == true){
                    cmdLine = cmdLine + " -sm 0";
                }
                
                std::cout << cmdLine << std::endl;
                
                //run cmd
                system(cmdLine.c_str());
                if (font) XFreeFont(display, font);
                XCloseDisplay(display);
                return 0;
            }
            if (calibrateButton.contains(mx, my)) {
                std::string cmdLine = "cd ../arsandbox/bin && ./CalibrateProjector";

                system(cmdLine.c_str());
                
                
                
            }
            if (kinectButton.contains(mx, my)) {
            std::string cmdLine = "cd ../kinect/bin && ./RawKinectViewer";

            system(cmdLine.c_str());

}
            if (colorFlag.contains(mx, my)){
                colorFlagChecked = !colorFlagChecked;
                XEvent redraw;

            }
            if (lineFlag.contains(mx, my)) {
                lineFlagChecked = !lineFlagChecked;
                XEvent redraw;

            }
            if (FPVflag.contains(mx, my)) {
                FPVflagChecked = !FPVflagChecked;
                XEvent redraw;
            }
            if (heavyRainFlag.contains(mx, my)) {
                heavyRainChecked = !heavyRainChecked;
            }
            if (snowMeltFlag.contains(mx, my)) {
                snowMeltChecked = !snowMeltChecked;
                XEvent redraw;
            }
            


            XEvent redraw;
            redraw.type = Expose;
            XSendEvent(display, window, False, ExposureMask, &redraw);
        }
    }

    if (font) XFreeFont(display, font);
    XCloseDisplay(display);
    return 0;
}
