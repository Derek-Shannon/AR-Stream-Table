# Master Makefile - Simple Local Build
VRUI_DIR = vrui
KINECT_DIR = kinect
SANDBOX_DIR = arsandbox
STARTSCREEN_DIR = StartScreen

.PHONY: all clean vrui kinect sandbox

all: vrui kinect sandbox start icon
	@echo "-------------------------------------------------------"
	@echo "Build complete! Your files are in their local folders:"
	@echo "Kinect tools:  ./$(KINECT_DIR)/bin"
	@echo "Sandbox app:   ./$(SANDBOX_DIR)/bin"
	@echo "-------------------------------------------------------"

vrui:
	@echo "--- Building Vrui ---"
	# Vrui often needs a config if it's a fresh download
	$(MAKE) -C $(VRUI_DIR) config
	$(MAKE) -j$(nproc) -C $(VRUI_DIR)
	@echo "--- Installing Vrui ---"
	$(MAKE) -C $(VRUI_DIR) install

kinect: vrui
	@echo "--- Building Kinect Extensions ---"
	# Run config first alone to ensure headers are mapped
	$(MAKE) -C $(KINECT_DIR) PROJECT_ROOT=$(shell pwd)/$(KINECT_DIR) config
	# Then compile everything at high speed
	$(MAKE) -j$(nproc) -C $(KINECT_DIR) PROJECT_ROOT=$(shell pwd)/$(KINECT_DIR)
	@echo "--- Installing Kinect ---"
	$(MAKE) -C $(KINECT_DIR) install

sandbox: kinect
	@echo "--- Building AR Sandbox ---"
	$(MAKE) -C $(SANDBOX_DIR) config
	$(MAKE) -j$(nproc) -C $(SANDBOX_DIR)

start: sandbox
	@echo "--- Building StartScreen ---"
	g++ $(STARTSCREEN_DIR)/StartScreen.cpp -o $(STARTSCREEN_DIR)/StartSARndbox \
		-lX11

icon:
	@echo "Creating desktop icon for StreamySandbox..."
	@# Get the real user's home directory even if running with sudo
	$(eval REAL_HOME := $(shell bash -c 'eval echo ~$${SUDO_USER:-$$USER}'))
	
	@echo "[Desktop Entry]" > $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Version=1.0" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Type=Application" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Name=AR Sandbox" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Comment=Launch AR Sandbox tool" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Exec=$(shell pwd)/$(STARTSCREEN_DIR)/StartSARndbox" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Icon=$(shell pwd)/$(SANDBOX_DIR)/share/SARndbox.png" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Terminal=true" >> $(REAL_HOME)/Desktop/StreamySandbox.desktop
	
	@# Make the shortcut executable
	@chmod +x $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@# Fix permissions so the file isn't locked by root
	@chown $${SUDO_USER:-$$USER} $(REAL_HOME)/Desktop/StreamySandbox.desktop
	@echo "Desktop icon created successfully!"

clean:
	$(MAKE) -C $(KINECT_DIR) clean
	$(MAKE) -C $(KINECT_DIR) uninstall
	$(MAKE) -C $(SANDBOX_DIR) clean
	$(MAKE) -C $(VRUI_DIR) clean
	$(MAKE) -C $(VRUI_DIR) uninstall
	rm -f $(STARTSCREEN_DIR)/StartSARndbox
