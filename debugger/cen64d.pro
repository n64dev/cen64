#
# cen64d.pro: CEN64 qmake file.
#
# CEN64D: Cycle-Accurate Nintendo 64 Debugger.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cen64d
TEMPLATE = app

SOURCES  += main.cpp\
            mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

