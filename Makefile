# Master Makefile - Simple Local Build
VRUI_DIR = vrui
KINECT_DIR = kinect
SANDBOX_DIR = arsandbox

.PHONY: all clean vrui kinect sandbox

all: vrui kinect sandbox
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

kinect: vrui
	@echo "--- Building Kinect Extensions ---"
	# Run config first alone to ensure headers are mapped
	$(MAKE) -C $(KINECT_DIR) PROJECT_ROOT=$(shell pwd)/$(KINECT_DIR) config
	# Then compile everything at high speed
	$(MAKE) -j$(nproc) -C $(KINECT_DIR) PROJECT_ROOT=$(shell pwd)/$(KINECT_DIR)

sandbox: kinect
	@echo "--- Building AR Sandbox ---"
	$(MAKE) -C $(SANDBOX_DIR) config
	$(MAKE) -j$(nproc) -C $(SANDBOX_DIR)

clean:
	$(MAKE) -C $(VRUI_DIR) clean
	$(MAKE) -C $(KINECT_DIR) clean
	$(MAKE) -C $(SANDBOX_DIR) clean
