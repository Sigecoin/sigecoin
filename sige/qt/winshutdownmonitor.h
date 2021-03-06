// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIGE_QT_WINSHUTDOWNMONITOR_H
#define SIGE_QT_WINSHUTDOWNMONITOR_H

#ifdef WIN32
#include <windows.h> // HWND

#include <QByteArray>
#include <QString>

#if QT_VERSION >= 0x050000
#include <QAbstractNativeEventFilter>

class WinShutdownMonitor : public QAbstractNativeEventFilter
{
public:
    /** Implements QAbstractNativeEventFilter interface for processing Windows messages */
    bool nativeEventFilter(const QByteArray &eventType, void *pMessage, long *pnResult);

    /** Register the reason for blocking shutdown on Windows to allow clean client exit */
    static void registerShutdownBlockReason(const QString& strReason, const HWND& mainWinId);
};
#endif
#endif

#endif // SIGE_QT_WINSHUTDOWNMONITOR_H
