#
# cen64d.pro: CEN64 qmake file.
#
# CEN64D: Cycle-Accurate Nintendo 64 Debugger.
# Copyright (C) 2014, Tyler J. Stachecki.
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = cen64d
TEMPLATE  = app

SOURCES  += disassembly_view.cpp \
            main.cpp\
            main_window.cpp \
            memory_view.cpp \
            memory_window.cpp \
            network_handle.cpp \
            rdp_window.cpp \
            register_view.cpp \
            rsp_window.cpp \
            toggle_window.cpp \
            vr4300_window.cpp

HEADERS  += disassembly_view.h \
            main_window.h \
            memory_view.h \
            memory_window.h \
            network_handle.h \
            rdp_window.h \
            register_view.h \
            rsp_window.h \
            toggle_window.h \
            vr4300_window.h

FORMS    += main_window.ui

