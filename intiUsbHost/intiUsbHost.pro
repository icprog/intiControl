#
# This file is part of intiLED.
#
# intiLED is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# intiLED is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with intiLED.  If not, see <http://www.gnu.org/licenses/>.
#

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = intiUsbHost
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp \
           usb.cpp

HEADERS  += mainwindow.h \
            usb.h

FORMS    += mainwindow.ui

LIBS += -lusb-1.0

INCLUDEPATH += /usr/include/libusb-1.0\
               ../include

DEFINES += HOST_CTRL
