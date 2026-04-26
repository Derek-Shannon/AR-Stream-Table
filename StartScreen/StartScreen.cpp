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
    unsigned long colorAccent = allocColor(display,"#4cc9f0", WhitePixel(display,screen));

    int baseWidth = 650;
    int baseHeight = 220;
    
    int winWidth = baseWidth;
    int winHeight = baseHeight; 

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

    // Added StructureNotifyMask to listen for resize events
    XSelectInput(display, window,
        ExposureMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask);

    XMapWindow(display, window);

    GC gc = XCreateGC(display, window, 0, nullptr);
    
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    
    
    //XFontStruct* font = XLoadQueryFont(display, "10x20");
    XFontStruct* font = nullptr;
    auto loadFont = [&](float scaleY) {
        if (font) {
            XFreeFont(display, font);
            font = nullptr;
        }

        const char* fontName;

        if (scaleY < 0.99f) fontName = "6x10";
        else if (scaleY < 3.5f) fontName = "10x20";
        else fontName = "12x24";

        font = XLoadQueryFont(display, fontName);

        if (!font) {
            font = XLoadQueryFont(display, "12x24");
        }

        if (font) {
            XSetFont(display, gc, font->fid);
        }
        
    };

    
    /*if (font) {
        XSetFont(display, gc, font->fid);
    } */
     

    


    // Interactive state flags
    bool calibrateHover = false;
    bool kinectHover = false;
    bool FPVflagHover = false, FPVflagChecked = false;
    bool heavyRainHover = false, heavyRainChecked = false;
    bool snowMeltHover = false, snowMeltChecked = false;
    bool hover = false; // Start button hover

    // Rectangles for UI elements
    Rect calibrateButton, kinectButton, FPVflag, heavyRainFlag, snowMeltFlag, button;

    // Labels
    std::string calibrateLabel = "Projector Calibration";
    std::string kinectLabel = "Kinect Calibration";
    std::string FPVflagLabel = "Debug Mode";
    std::string heavyRainLabel = "Heavy Rain Flag";
    std::string snowMeltLabel = "Snow Melt Flag";
    std::string label = "Start Projection";
    std::string titleLeft = "Initial Calibration Options";
    std::string titleRight = "Operation Settings";

    // Layout variables updated by resizing
    int leftColumnX, rightColumnX, titleY;

    // Function to recalculate layout based on window size
    auto updateLayout = [&]() {
        float scaleX = (float)winWidth / (float)baseWidth;
        float scaleY = (float)winHeight / (float)baseHeight;

        leftColumnX = 20 * scaleX;
        rightColumnX = 350 * scaleX;
        int itemsStartY = 50 * scaleY;
        titleY = 30 * scaleY;

        // Calibrate Button
        calibrateButton.w = 220 * scaleX;
        calibrateButton.h = 25 * scaleY; 
        calibrateButton.x = leftColumnX; 
        calibrateButton.y = itemsStartY; 

        // Kinect Button
        kinectButton.w = 220 * scaleX;
        kinectButton.h = 25 * scaleY; 
        kinectButton.x = leftColumnX; 
        kinectButton.y = itemsStartY + (35 * scaleY);

        // Checkboxes (kept square using scaleX for uniformity)
        int boxSize = 10 * scaleX;
        if (boxSize < 10) boxSize = 10; // min size

        FPVflag.w = boxSize;
        FPVflag.h = boxSize;
        FPVflag.x = rightColumnX;
        FPVflag.y = itemsStartY;

        heavyRainFlag.w = boxSize;
        heavyRainFlag.h = boxSize;
        heavyRainFlag.x = rightColumnX;
        heavyRainFlag.y = FPVflag.y + (25 * scaleY); 

        snowMeltFlag.w = boxSize;
        snowMeltFlag.h = boxSize;
        snowMeltFlag.x = rightColumnX;
        snowMeltFlag.y = heavyRainFlag.y + (25 * scaleY); 

        // Start Button
        button.w = 270 * scaleX;
        button.h = 50 * scaleY;
        button.x = rightColumnX;
        button.y = snowMeltFlag.y + (35 * scaleY);

        loadFont(scaleY);
    };

    // Initial layout calculation
    updateLayout();

    while (true) {
        float scaleX = (float)winWidth / (float)baseWidth;
        float scaleY = (float)winHeight / (float)baseHeight;
        
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == ConfigureNotify) {
            // Update window dimensions when resized
            if (event.xconfigure.width != winWidth || event.xconfigure.height != winHeight) {
                winWidth = event.xconfigure.width;
                winHeight = event.xconfigure.height;
                updateLayout();
            }
        }
        else if (event.type == Expose) {
            
            // Draw Titles
            XSetForeground(display, gc, colorAccent);
            XDrawString(display, window, gc, leftColumnX, titleY, titleLeft.c_str(), titleLeft.length());
            XDrawString(display, window, gc, rightColumnX, titleY, titleRight.c_str(), titleRight.length());

            // --- START BUTTON ---
            XSetForeground(display, gc, hover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, button.x, button.y, button.w, button.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, button.x, button.y, button.w, button.h);

            int textWidth = XTextWidth(font, label.c_str(), label.length());
            int textX = button.x + (button.w - textWidth) / 2;
            //int textY = button.y + button.h / 2 + 5;
            int textY = button.y + button.h / 2 + (5 * scaleY);
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, textX, textY, label.c_str(), label.length());
            
            // --- CALIBRATE BUTTON ---
            XSetForeground(display, gc, calibrateHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, calibrateButton.x, calibrateButton.y, calibrateButton.w, calibrateButton.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, calibrateButton.x, calibrateButton.y, calibrateButton.w, calibrateButton.h);

            int calTextWidth = XTextWidth(font, calibrateLabel.c_str(), calibrateLabel.length());
            int calTextX = calibrateButton.x + (calibrateButton.w - calTextWidth) / 2;
            int calTextY = calibrateButton.y + calibrateButton.h / 2 + (5 * scaleY);
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, calTextX, calTextY, calibrateLabel.c_str(), calibrateLabel.length());
            
            // --- KINECT BUTTON ---
            XSetForeground(display, gc, kinectHover ? colorButtonHover : colorButton);
            XFillRectangle(display, window, gc, kinectButton.x, kinectButton.y, kinectButton.w, kinectButton.h);
            XSetForeground(display, gc, colorBorder);
            XDrawRectangle(display, window, gc, kinectButton.x, kinectButton.y, kinectButton.w, kinectButton.h);

            int kTextWidth = XTextWidth(font, kinectLabel.c_str(), kinectLabel.length());
            int kTextX = kinectButton.x + (kinectButton.w - kTextWidth) / 2;
            int kTextY = kinectButton.y + kinectButton.h / 2 + (5 * scaleY);
            XSetForeground(display, gc, colorText);
            XDrawString(display, window, gc, kTextX, kTextY, kinectLabel.c_str(), kinectLabel.length());

            // Helper lambda for checkboxes
            auto drawCheckbox = [&](Rect& rect, bool isHover, bool isChecked, std::string lbl) {
                XSetForeground(display, gc, isHover ? colorButtonHover : colorButton);
                XFillRectangle(display, window, gc, rect.x, rect.y, rect.w, rect.h);
                XSetForeground(display, gc, colorBorder);
                XDrawRectangle(display, window, gc, rect.x, rect.y, rect.w, rect.h);

                // Text Label offset scaled relative to box
                int tx = rect.x + rect.w + 5; 
                int ty = rect.y + rect.h - 2;
                XSetForeground(display, gc, colorText);
                XDrawString(display, window, gc, tx, ty, lbl.c_str(), lbl.length());

                if (isChecked) {
                    const char* mark = "o";
                    /*int markX = rect.x + (rect.w / 2) - 3;
                    int markY = rect.y + (rect.h / 2) + 4;*/
                    int markX = rect.x + (rect.w / 2) - 4;
                    int markY = rect.y + (rect.h / 2) + 4;
                    XSetForeground(display, gc, colorText);
                    XDrawString(display, window, gc, markX, markY, mark, 1);
                }
            };

            // Draw Checkboxes
            drawCheckbox(FPVflag, FPVflagHover, FPVflagChecked, FPVflagLabel);
            drawCheckbox(heavyRainFlag, heavyRainHover, heavyRainChecked, heavyRainLabel);
            drawCheckbox(snowMeltFlag, snowMeltHover, snowMeltChecked, snowMeltLabel);
        }

        else if (event.type == MotionNotify) {
            int mx = event.xmotion.x;
            int my = event.xmotion.y;

            bool redrawNeeded = false;

            auto checkHover = [&](Rect& rect, bool& currentHover) {
                bool newHover = rect.contains(mx, my);
                if (newHover != currentHover) {
                    currentHover = newHover;
                    redrawNeeded = true;
                }
            };

            checkHover(button, hover);
            checkHover(calibrateButton, calibrateHover);
            checkHover(kinectButton, kinectHover);
            checkHover(FPVflag, FPVflagHover);
            checkHover(heavyRainFlag, heavyRainHover);
            checkHover(snowMeltFlag, snowMeltHover);

            if (redrawNeeded) {
                XEvent redraw;
                redraw.type = Expose;
                XSendEvent(display, window, False, ExposureMask, &redraw);
            }
        }

        else if (event.type == ButtonPress) {
            int mx = event.xbutton.x;
            int my = event.xbutton.y;

            if (button.contains(mx, my)) {
                std::string cmdLine = "cd "+getExecutableDir()+"/../arsandbox/bin && ./SARndbox -fpv -uhm";

                if (FPVflagChecked) cmdLine += " -debug";
                if (heavyRainChecked) cmdLine += " -rs .75";
                if (snowMeltChecked) cmdLine += " -sm 0";
                
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