// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __qt_sigecoin_h__
#define __qt_sigecoin_h__

#include "platformstyle.h"
#include "scheduler.h"

#include <QApplication>
#include <QThread>

/** Class encapsulating Sigecoin startup and shutdown.
 * Allows running startup and shutdown in a different thread from the UI thread.
 */
class SigecoinEngine: public QObject
{
    Q_OBJECT
public:
    explicit SigecoinEngine();

public Q_SLOTS:
    void initialize();
    void shutdown();

Q_SIGNALS:
    void initializeResult(int retval);
    void shutdownResult(int retval);
    void runawayException(const QString &message);

private:
    boost::thread_group threadGroup;
    CScheduler scheduler;

    /// Pass fatal exception message to UI thread
    void handleRunawayException(const std::exception *e);
};



class ClientModel;
class SigecoinGUI;
class NetworkStyle;
class OptionsModel;
class WalletModel;
class PaymentServer;

/** Main Sigecoin application object */
class SigecoinApplication: public QApplication
{
    Q_OBJECT
public:
    explicit SigecoinApplication(int &argc, char **argv);
    ~SigecoinApplication();

// #ifdef ENABLE_WALLET
    /// Create payment server
    void createPaymentServer();
// #endif
    /// parameter interaction/setup based on rules
    void parameterSetup();
    /// Create options model
    void createOptionsModel(bool resetSettings);
    /// Create main window
    void createWindow(const NetworkStyle *networkStyle);
    /// Create splash screen
    void createSplashScreen(const NetworkStyle *networkStyle);

    /// Request core initialization
    void requestInitialize();
    /// Request core shutdown
    void requestShutdown();

    /// Get process return value
    int getReturnValue() { return returnValue; }

    /// Get window identifier of QMainWindow (SigecoinGUI)
    WId getMainWinId() const;

public Q_SLOTS:
    void initializeResult(int retval);
    void shutdownResult(int retval);
    /// Handle runaway exceptions. Shows a message box with the problem and quits the program.
    void handleRunawayException(const QString &message);

Q_SIGNALS:
    void requestedInitialize();
    void requestedShutdown();
    void stopThread();
    void splashFinished(QWidget *window);

private:
    QThread* coreThread;
    OptionsModel* optionsModel;
    ClientModel* clientModel;
    SigecoinGUI* window;
    QTimer *pollShutdownTimer;
// #ifdef ENABLE_WALLET
    PaymentServer* paymentServer;
    WalletModel* walletModel;
// #endif
    int returnValue;
    const PlatformStyle *platformStyle;
    std::unique_ptr<QWidget> shutdownWindow;

    void startThread();
};

#endif  /* __qt_sigecoin_h__ */ 
