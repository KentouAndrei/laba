AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = myprogram
myprogram_SOURCES = main.cpp FuncA.cpp FuncA.h
check_PROGRAMS = test
test_SOURCES = test.cpp FuncA.cpp
PACKAGE = myprogram
VERSION = 1.0
PACKAGE_BUGREPORT = www.vandrey@gmail.com

CTRLF_DIR = $(CURDIR)/debian
CTRLF_NAME = $(CTRLF_DIR)/control

.PHONY: deb debug

deb:
	mkdir -p deb/DEBIAN
	echo "Package: myprogram" > deb/DEBIAN/control
	echo "Version: 1.0" >> deb/DEBIAN/control
	echo "Section: utils" >> deb/DEBIAN/control
	echo "Priority: optional" >> deb/DEBIAN/control
	echo "Architecture: amd64" >> deb/DEBIAN/control
	echo "Maintainer: Your Name <your.email@example.com>" >> deb/DEBIAN/control
	echo "Description: A short description of the package" >> deb/DEBIAN/control


debug:
	@echo "${PACKAGE}"
	@echo "${VERSION}"
	@echo "${PACKAGE_BUGREPORT}"

check: test
	@./test
