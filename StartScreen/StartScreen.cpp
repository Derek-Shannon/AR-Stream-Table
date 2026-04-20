//Jake's startup menu (very cool)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#include <filesystem>

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

std::string getExecutableDir() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::filesystem::path(buffer).parent_path().string();
    }
    return "";
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

    // Reduced height slightly since we removed two checkboxes
    int winWidth = 650;
    int winHeight = 220; 

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

    // Layout Anchors
    int leftColumnX = 20;
    int rightColumnX = 350;
    int itemsStartY = 50;

    // --- LEFT COLUMN: Calibration ---

    // Calibrate Button setup
    Rect calibrateButton;
    calibrateButton.w = 200;
    calibrateButton.h = 25; 
    calibrateButton.x = leftColumnX; 
    calibrateButton.y = itemsStartY; 

    bool calibrateHover = false;
    std::string calibrateLabel = "Projector Calibration";

    // Kinect Button setup
    Rect kinectButton;
    kinectButton.w = 200;
    kinectButton.h = 25; 
    kinectButton.x = leftColumnX; 
    kinectButton.y = itemsStartY + 35; // Placed below Projector Calib

    bool kinectHover = false;
    std::string kinectLabel = "Kinect Calibration";

    // --- RIGHT COLUMN: Operation Settings ---

    // Debug Mode Flag
    Rect FPVflag;
    FPVflag.w = 10;
    FPVflag.h = 10;
    FPVflag.x = rightColumnX;
    FPVflag.y = itemsStartY; // Moved to the top of the right column

    bool FPVflagHover = false;
    bool FPVflagChecked = false;
    std::string FPVflagLabel = "Debug Mode";

    // Heavy Rain Flag
    Rect heavyRainFlag;
    heavyRainFlag.w = 10;
    heavyRainFlag.h = 10;
    heavyRainFlag.x = rightColumnX;
    heavyRainFlag.y = FPVflag.y + 25; 

    bool heavyRainHover = false;
    bool heavyRainChecked = false;
    std::string heavyRainLabel = "Heavy Rain Flag";

    // Snow Melt Flag
    Rect snowMeltFlag;
    snowMeltFlag.w = 10;
    snowMeltFlag.h = 10;
    snowMeltFlag.x = rightColumnX;
    snowMeltFlag.y = heavyRainFlag.y + 25; 

    bool snowMeltHover = false;
    bool snowMeltChecked = false;
    std::string snowMeltLabel = "Snow Melt Flag";

    // Start Button setup (Bottom Right)
    Rect button;
    button.w = 270;
    button.h = 50;
    button.x = rightColumnX;
    button.y = snowMeltFlag.y + 35; // Placed below the last checkbox

    bool hover = false;
    std::string label = "Start Projection";

    // Titles
    std::string titleLeft = "Initial Calibration Options";
    std::string titleRight = "Operation Settings";

    while (true) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == Expose) {
            
            // Draw Titles
            XSetForeground(display, gc, colorAccent);
            XDrawString(display, window, gc, leftColumnX, 30, titleLeft.c_str(), titleLeft.length());
            XDrawString(display, window, gc, rightColumnX, 30, titleRight.c_str(), titleRight.length());

            // --- START BUTTON ---
            XSetForeground(display, gc, hover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, button.x, button.y, button.w, button.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, button.x, button.y, button.w, button.h);

            int textWidth = XTextWidth(font, label.c_str(), label.length());
            int textX = button.x + (button.w - textWidth) / 2;
            int textY = button.y + button.h / 2 + 5;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, textX, textY, label.c_str(), label.length());
            
            // --- CALIBRATE BUTTON ---
            XSetForeground(display, gc, calibrateHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, calibrateButton.x, calibrateButton.y, calibrateButton.w, calibrateButton.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, calibrateButton.x, calibrateButton.y, calibrateButton.w, calibrateButton.h);

            int calTextWidth = XTextWidth(font, calibrateLabel.c_str(), calibrateLabel.length());
            int calTextX = calibrateButton.x + (calibrateButton.w - calTextWidth) / 2;
            int calTextY = calibrateButton.y + calibrateButton.h / 2 + 5;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, calTextX, calTextY, calibrateLabel.c_str(), calibrateLabel.length());
            
            // --- KINECT BUTTON ---
            XSetForeground(display, gc, kinectHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, kinectButton.x, kinectButton.y, kinectButton.w, kinectButton.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, kinectButton.x, kinectButton.y, kinectButton.w, kinectButton.h);

            int kTextWidth = XTextWidth(font, kinectLabel.c_str(), kinectLabel.length());
            int kTextX = kinectButton.x + (kinectButton.w - kTextWidth) / 2;
            int kTextY = kinectButton.y + kinectButton.h / 2 + 5;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, kTextX, kTextY, kinectLabel.c_str(), kinectLabel.length());

            // --- DEBUG FLAG ---
            XSetForeground(display, gc, FPVflagHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, FPVflag.x, FPVflag.y, FPVflag.w, FPVflag.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, FPVflag.x, FPVflag.y, FPVflag.w, FPVflag.h);

            int ftx = FPVflag.x + 15;
            int fty = FPVflag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, ftx, fty, FPVflagLabel.c_str(), FPVflagLabel.length());

            if (FPVflagChecked) {
                const char* mark = "x";
                ftx = FPVflag.x + 3;
                fty = FPVflag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc, ftx, fty, mark, 1);
            }

            // --- HEAVY RAIN FLAG ---
            XSetForeground(display, gc, heavyRainHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, heavyRainFlag.x, heavyRainFlag.y, heavyRainFlag.w, heavyRainFlag.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, heavyRainFlag.x, heavyRainFlag.y, heavyRainFlag.w, heavyRainFlag.h);

            int htx = heavyRainFlag.x + 15;
            int hty = heavyRainFlag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, htx, hty, heavyRainLabel.c_str(), heavyRainLabel.length());

            if (heavyRainChecked) {
                const char* mark = "x";
                htx = heavyRainFlag.x + 3;
                hty = heavyRainFlag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc, htx, hty, mark, 1);
            }

            // --- SNOW MELT FLAG ---
            XSetForeground(display, gc, snowMeltHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, snowMeltFlag.x, snowMeltFlag.y, snowMeltFlag.w, snowMeltFlag.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, snowMeltFlag.x, snowMeltFlag.y, snowMeltFlag.w, snowMeltFlag.h);

            int stx = snowMeltFlag.x + 15;
            int sty = snowMeltFlag.y + 9;
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, stx, sty, snowMeltLabel.c_str(), snowMeltLabel.length());

            if (snowMeltChecked) {
                const char* mark = "x";
                stx = snowMeltFlag.x + 3;
                sty = snowMeltFlag.y + 8;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc, stx, sty, mark, 1);
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
                std::string cmdLine = "cd "+getExecutableDir()+"/../arsandbox/bin && ./SARndbox -fpv";

                if (FPVflagChecked == true) cmdLine += " -debug";
                if (heavyRainChecked == true) cmdLine += " -rs .75";
                if (snowMeltChecked == true) cmdLine += " -sm 0";
                
                std::cout << cmdLine << std::endl;
                
                system(cmdLine.c_str());
                if (font) XFreeFont(display, font);
                XCloseDisplay(display);
                return 0;
            }
            if (calibrateButton.contains(mx, my)) {
                std::string cmdLine = "cd "+getExecutableDir()+"/../arsandbox/bin && ./CalibrateProjector";
                system(cmdLine.c_str());
            }
            if (kinectButton.contains(mx, my)) {
                std::string cmdLine = "cd "+getExecutableDir()+"/../kinect/bin && ./RawKinectViewer";
                system(cmdLine.c_str());
            }
            
            // Toggles
            if (FPVflag.contains(mx, my)) FPVflagChecked = !FPVflagChecked;
            if (heavyRainFlag.contains(mx, my)) heavyRainChecked = !heavyRainChecked;
            if (snowMeltFlag.contains(mx, my)) snowMeltChecked = !snowMeltChecked;

            XEvent redraw;
            redraw.type = Expose;
            XSendEvent(display, window, False, ExposureMask, &redraw);
        }
    }

    if (font) XFreeFont(display, font);
    XCloseDisplay(display);
    return 0;
}