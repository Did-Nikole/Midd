# ==============================================================================
# Midd - Makefile
# ==============================================================================
# midd extracts precise, bounded intervals of lines from a stream or file.
# This Makefile builds the project using C++20.

# Compiler and Flags
CXX          := g++
CXXFLAGS     := -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
                -Wcast-align -Wconversion -Wsign-conversion -Wnull-dereference \
                -Iinclude -Iinclude/extern -MMD -MP

# Optimization & Debug Flags
OPTFLAGS     := -O3
DEBUGFLAGS   := -g

# Directories
SRC_DIR      := src
BUILD_DIR    := build

# Installation paths
PREFIX       ?= /usr/local
BINDIR       ?= $(PREFIX)/bin
MANDIR       ?= $(PREFIX)/share/man/man1

# Debian packaging settings
PACKAGE_NAME    := midd
PACKAGE_VERSION ?= $(shell awk -F: '/version/ {print $$2}' version)
PACKAGE_ARCH    := $(shell dpkg --print-architecture 2>/dev/null || echo "amd64")
DEB_STAGE_DIR   := build/deb_pkg
DIST_DIR        := dist

# Target executable name
TARGET       := midd

# Source and Object files
SRCS         := $(wildcard $(SRC_DIR)/*.cpp)
OBJS         := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS         := $(OBJS:.o=.d)

# ------------------------------------------------------------------------------
# Build Rules
# ------------------------------------------------------------------------------

# Default target (Release build)
all: CXXFLAGS += $(OPTFLAGS)
all: $(TARGET)

# Debug target (adds debug symbols)
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJS)
	@awk -F: '/version/ { \
		split($$2, a, "."); \
		a[3]++; \
		new_ver=a[1]"."a[2]"."a[3]; \
		print "version:" new_ver > "version"; \
		print "Bumped version to " new_ver; \
	}' version
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "=============================================="
	@echo " Successfully built: $@"
	@echo "=============================================="

# Rule to compile source files to object files in the build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Include generated dependency files (so changes to headers trigger rebuilds)
-include $(DEPS)

# ------------------------------------------------------------------------------
# Cleaning Rules
# ------------------------------------------------------------------------------

# Clean object files and dependency files
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build objects and dependencies."

# Clean everything including the executable and packaged binaries
fclean: clean
	rm -f $(TARGET)
	rm -rf $(DIST_DIR)
	@echo "Cleaned executable and distribution files."

# Rebuild target
re: fclean all

# ------------------------------------------------------------------------------
# Installation Rules
# ------------------------------------------------------------------------------

# Install the application and the man page
install: all midd.1
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)
	cp -f $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)
	cp -f midd.1 $(DESTDIR)$(MANDIR)/midd.1
	chmod 644 $(DESTDIR)$(MANDIR)/midd.1
	@echo "Installed $(TARGET) to $(DESTDIR)$(BINDIR) and man page to $(DESTDIR)$(MANDIR)"

# Uninstall the application and the man page
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(MANDIR)/midd.1
	@echo "Uninstalled $(TARGET) and man page."

# ------------------------------------------------------------------------------
# Debian Packaging
# ------------------------------------------------------------------------------

# Build a Debian package in the dist/ folder
deb: all
	@NEW_VERSION=$$(awk -F: '/version/ {print $$2}' version); \
	$(MAKE) build-deb PACKAGE_VERSION=$$NEW_VERSION

build-deb: all midd.1
	@echo "Building Debian package version $(PACKAGE_VERSION)..."
	rm -rf $(DEB_STAGE_DIR)
	mkdir -p $(DEB_STAGE_DIR)/DEBIAN
	mkdir -p $(DEB_STAGE_DIR)/usr/bin
	mkdir -p $(DEB_STAGE_DIR)/usr/share/man/man1
	# Copy files to staging area
	cp -f $(TARGET) $(DEB_STAGE_DIR)/usr/bin/$(TARGET)
	chmod 755 $(DEB_STAGE_DIR)/usr/bin/$(TARGET)
	cp -f midd.1 $(DEB_STAGE_DIR)/usr/share/man/man1/midd.1
	chmod 644 $(DEB_STAGE_DIR)/usr/share/man/man1/midd.1
	# Generate DEBIAN/control file
	@echo "Package: $(PACKAGE_NAME)" > $(DEB_STAGE_DIR)/DEBIAN/control
	@echo "Version: $(PACKAGE_VERSION)" >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo "Section: utils" >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo "Priority: optional" >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo "Architecture: $(PACKAGE_ARCH)" >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo "Maintainer: Nikole Smith <appsolutionz.com>" >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo "Description: Truncates and displays the start and end of text streams." >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo " midd is a high-performance command-line utility that extracts" >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo " the head and tail of an input stream and discards the middle," >> $(DEB_STAGE_DIR)/DEBIAN/control
	@echo " ensuring a constant memory footprint even with multi-gigabyte logs." >> $(DEB_STAGE_DIR)/DEBIAN/control
	# Build the package
	mkdir -p $(DIST_DIR)
	dpkg-deb --build $(DEB_STAGE_DIR) $(DIST_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(PACKAGE_ARCH).deb
	@echo "Debian package successfully built: $(DIST_DIR)/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(PACKAGE_ARCH).deb"

# ------------------------------------------------------------------------------
# RPM Packaging
# ------------------------------------------------------------------------------

RPM_STAGE_DIR   := build/rpm_pkg
RPM_ARCH        ?= $(shell rpm --eval '%_arch' 2>/dev/null || uname -m)

# Build an RPM package in the dist/ folder
rpm: all
	@NEW_VERSION=$$(awk -F: '/version/ {print $$2}' version); \
	$(MAKE) build-rpm PACKAGE_VERSION=$$NEW_VERSION

build-rpm: all midd.1
	@echo "Building RPM package version $(PACKAGE_VERSION)..."
	rm -rf $(RPM_STAGE_DIR)
	mkdir -p $(RPM_STAGE_DIR)/SPECS
	mkdir -p $(RPM_STAGE_DIR)/SOURCES
	# Copy target and man page to SOURCES
	cp -f $(TARGET) $(RPM_STAGE_DIR)/SOURCES/$(TARGET)
	cp -f midd.1 $(RPM_STAGE_DIR)/SOURCES/midd.1
	# Generate SPECS/midd.spec file
	@echo "Name:           $(PACKAGE_NAME)" > $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "Version:        $(PACKAGE_VERSION)" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "Release:        1%{?dist}" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "Summary:        Midd utility for cropping middle of files" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "License:        GPL" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "URL:            https://github.com/user/$(PACKAGE_NAME)" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "%description" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "midd extracts precise, bounded intervals of lines from a stream or file." >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "%install" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "mkdir -p %{buildroot}%{BINDIR}" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "mkdir -p %{buildroot}%{MANDIR}" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "cp -p %{_sourcedir}/$(TARGET) %{buildroot}%{BINDIR}/$(TARGET)" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "cp -p %{_sourcedir}/midd.1 %{buildroot}%{MANDIR}/midd.1" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "%files" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "%{BINDIR}/$(TARGET)" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	@echo "%{MANDIR}/midd.1" >> $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	# Build the package
	mkdir -p $(DIST_DIR)
	rpmbuild --define "_topdir $(CURDIR)/$(RPM_STAGE_DIR)" --define "BINDIR $(BINDIR)" --define "MANDIR $(MANDIR)" -bb $(RPM_STAGE_DIR)/SPECS/$(PACKAGE_NAME).spec
	cp -f $(RPM_STAGE_DIR)/RPMS/$(RPM_ARCH)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-1.$(RPM_ARCH).rpm $(DIST_DIR)/
	@echo "RPM package successfully built: $(DIST_DIR)/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-1.$(RPM_ARCH).rpm"

midd.1: midd.1.md version
	pandoc -s -t man -M footer="Version $(PACKAGE_VERSION)" midd.1.md -o midd.1


.PHONY: all debug clean fclean re install uninstall deb rpm build-deb build-rpm
