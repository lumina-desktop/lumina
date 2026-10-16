//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "PanelWidget.h"
#include "LPlugins.h"

MainWindow::MainWindow(int screen, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    loading(false),
    cscreen(screen)
{
    ui->setupUi(this);
    
    // Initialize settings and plugin info
    settings = new QSettings("lumina-desktop", "desktopsettings");
    PINFO = new LPlugins();
    
    // Set window properties
    setWindowTitle(QString("Panel Configuration - Screen %1").arg(screen + 1));
    setWindowIcon(LXDG::findIcon("preferences-system-windows", ""));
    
    // Setup menus and UI
    setupMenus();
    updateIcons();
    
    // Create panels container layout
    QHBoxLayout *panels_layout = new QHBoxLayout();
    panels_layout->setContentsMargins(0, 0, 0, 0);
    panels_layout->setAlignment(Qt::AlignLeft);
    panels_layout->addStretch();
    ui->scroll_panels->widget()->setLayout(panels_layout);
    
    // Setup screen selector
    setupScreenSelector();
    
    // Connect signals
    connect(ui->tool_panels_add, SIGNAL(clicked()), this, SLOT(newPanel()));
    connect(ui->combo_screen, SIGNAL(currentIndexChanged(int)), this, SLOT(screenChanged()));
    
    // Load current settings
    LoadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete settings;
    delete PINFO;
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(LXDG::findIcon("document-save", ""));
    connect(saveAction, &QAction::triggered, this, &MainWindow::SaveSettings);
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setIcon(LXDG::findIcon("application-exit", ""));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = helpMenu->addAction("&About");
    aboutAction->setIcon(LXDG::findIcon("help-about", ""));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::SaveSettings()
{
    QString screenID = QApplication::screens().at(cscreen)->name();
    QString DPrefix = "desktop-" + screenID + "/";
    settings->setValue(DPrefix + "panels", PANELS.length());
    
    for(int i = 0; i < PANELS.length(); i++) {
        PANELS[i]->SaveSettings(settings);
    }
    
    settings->sync(); // save to disk right now
    statusBar()->showMessage("Settings saved", 2000);
    
    // Reload in a moment to get updated plugin IDs
    QTimer::singleShot(1000, this, SLOT(LoadSettings()));
}

void MainWindow::LoadSettings()
{
    loading = true;
    
    QString screenID = QApplication::screens().at(cscreen)->name();
    QString DPrefix = "desktop-" + screenID + "/";
    int panelnumber = settings->value(DPrefix + "panels", -1).toInt();
    if(panelnumber < 0) { panelnumber = 0; }
    
    QHBoxLayout *panels_layout = static_cast<QHBoxLayout*>(ui->scroll_panels->widget()->layout());
    
    // Remove extra panels (if any)
    for(int i = panelnumber; i < PANELS.length(); i++) {
        PanelWidget *tmp = PANELS.takeAt(i);
        delete tmp;
        i--;
    }
    
    int current_count = panels_layout->count() - 1;
    
    // Update current panels
    for(int i = 0; i < current_count; i++) {
        PANELS[i]->LoadSettings(settings, cscreen, i);
    }
    
    // Create new panels
    for(int i = current_count; i < panelnumber; i++) {
        PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
        tmp->LoadSettings(settings, cscreen, i);
        PANELS << tmp;
        connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()));
        connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)));
        panels_layout->insertWidget(panels_layout->count() - 1, tmp);
    }
    
    QApplication::processEvents();
    loading = false;
    setupProfiles();
}

void MainWindow::updateIcons()
{
    ui->tool_panels_add->setIcon(LXDG::findIcon("list-add", ""));
    ui->tool_profile->setIcon(LXDG::findIcon("document-import", ""));
}

void MainWindow::panelValChanged()
{
    ui->tool_panels_add->setEnabled(PANELS.length() < 12);
    if(!loading) { 
        settingChanged(); 
    }
}

void MainWindow::newPanel()
{
    // Create a new Panel widget
    PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
    tmp->LoadSettings(settings, cscreen, PANELS.length());
    PANELS << tmp;
    connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()));
    connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)));
    static_cast<QBoxLayout*>(ui->scroll_panels->widget()->layout())->insertWidget(PANELS.length() - 1, tmp);
    
    // Update the widget first (2 necessary for scroll below to work)
    ui->scroll_panels->update();
    QApplication::processEvents();
    QApplication::processEvents();
    ui->scroll_panels->ensureWidgetVisible(tmp);
    panelValChanged();
}

void MainWindow::removePanel(int pan)
{
    // Connected to a signal from the panel widget
    bool changed = false;
    for(int i = 0; i < PANELS.length(); i++) {
        int num = PANELS[i]->PanelNumber();
        if(num == pan) {
            delete PANELS.takeAt(i);
            i--;
            changed = true;
        } else if(num > pan) {
            PANELS[i]->ChangePanelNumber(num - 1);
            changed = true;
        }
    }
    if(!changed) { return; } // nothing done
    panelValChanged();
}

void MainWindow::setupProfiles()
{
    if(ui->tool_profile->menu() == 0) {
        ui->tool_profile->setMenu(new QMenu(this));
        connect(ui->tool_profile->menu(), SIGNAL(triggered(QAction*)), this, SLOT(applyProfile(QAction*)));
    } else { 
        ui->tool_profile->menu()->clear(); 
    }
    
    ui->tool_profile->menu()->addSection("Profiles");
    QAction *act = ui->tool_profile->menu()->addAction("No Panels");
    act->setWhatsThis("none");
    act = ui->tool_profile->menu()->addAction("Windows");
    act->setWhatsThis("windows");
    act = ui->tool_profile->menu()->addAction("GNOME2/MATE");
    act->setWhatsThis("gnome2");
    act = ui->tool_profile->menu()->addAction("XFCE");
    act->setWhatsThis("xfce");
    act = ui->tool_profile->menu()->addAction("Mac OSX");
    act->setWhatsThis("osx");
}

void MainWindow::applyProfile(QAction *act)
{
    QString screenID = QApplication::screens().at(cscreen)->name();
    QString DPrefix = "desktop-" + screenID + "/";
    QString PPrefix = "panel_" + screenID + "."; // NEED TO APPEND PANEL NUMBER (0+)
    
    if(act->whatsThis() == "none") {
        settings->setValue(DPrefix + "panels", 0); // number of panels
    } else if(act->whatsThis() == "windows") {
        settings->setValue(DPrefix + "panels", 1); // number of panels
        // Panel 1 settings (index 0)
        settings->setValue(PPrefix + "0/customColor", false);
        settings->setValue(PPrefix + "0/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height() * 0.04)); // 4% of screen height
        settings->setValue(PPrefix + "0/hidepanel", false);
        settings->setValue(PPrefix + "0/lengthPercent", 100);
        settings->setValue(PPrefix + "0/location", "bottom");
        settings->setValue(PPrefix + "0/pinLocation", "center");
        settings->setValue(PPrefix + "0/pluginlist", QStringList() << "systemstart" << "taskmanager" << "spacer" << "systemtray" << "clock");
    }
    // Add other profiles as needed...
    
    // Now flush the settings to disk and reload the interface
    settings->sync(); // save to disk right now
    QTimer::singleShot(1000, this, SLOT(LoadSettings()));
}

void MainWindow::applyImport(QAction *act)
{
    applyImport(act->whatsThis());
}

void MainWindow::applyImport(QString fromID)
{
    QString cID = QApplication::screens().at(cscreen)->name();
    
    // First find all the values associated with this ID
    int pannum = settings->value("desktop-" + fromID + "/panels").toInt();
    QStringList pans = settings->allKeys().filter("panel_" + fromID);
    fromID.prepend("panel_");
    
    // Save the number of panels which is active
    settings->setValue("desktop-" + cID + "/panels", pannum);
    
    // Now move over all the panel settings associated with the fromID
    cID.prepend("panel_");
    for(int i = 0; i < pans.length(); i++) {
        QString newvar = pans[i];
        newvar.replace(fromID, cID);
        settings->setValue(newvar, settings->value(pans[i]));
    }
    
    // Now flush the settings to disk and reload the interface
    settings->sync(); // save to disk right now
    QTimer::singleShot(1000, this, SLOT(LoadSettings()));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About Panel Configuration",
        "Lumina Control Center - Panel Configuration\n\n"
        "Configure desktop panels and their plugins.\n\n"
        "Copyright (c) 2024, jt(q5sys)\n"
        "Available under the 3-clause BSD license");
}

void MainWindow::settingChanged()
{
    // Mark that settings have changed
    statusBar()->showMessage("Settings changed - remember to save", 5000);
}

void MainWindow::setupScreenSelector()
{
    // Populate screen selector with available screens
    ui->combo_screen->clear();
    QList<QScreen*> screens = QApplication::screens();
    for(int i = 0; i < screens.length(); i++) {
        QString screenName = screens[i]->name();
        if(screenName.isEmpty()) {
            screenName = QString("Screen %1").arg(i + 1);
        }
        ui->combo_screen->addItem(screenName, i);
    }
    
    // Set current screen
    if(cscreen < screens.length()) {
        ui->combo_screen->setCurrentIndex(cscreen);
    }
}

void MainWindow::screenChanged()
{
    if(loading) return; // Don't process during loading
    
    int newScreen = ui->combo_screen->currentData().toInt();
    if(newScreen != cscreen && newScreen >= 0 && newScreen < QApplication::screens().length()) {
        cscreen = newScreen;
        setWindowTitle(QString("Panel Configuration - Screen %1").arg(cscreen + 1));
        LoadSettings(); // Reload settings for the new screen
    }
}
