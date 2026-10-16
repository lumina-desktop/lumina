//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <LuminaXDG.h>
#include <LuminaOS.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loading = false;
    
    // Set window properties
    this->setWindowTitle(tr("Window Manager Settings"));
    this->setWindowIcon(LXDG::findIcon("preferences-system-windows", ""));
    
    // Setup UI
    setupMenus();
    setupConnections();
    
    // Load current settings
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu *fileMenu = ui->menubar->addMenu(tr("&File"));
    
    QAction *saveAction = fileMenu->addAction(tr("&Save Settings"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(LXDG::findIcon("document-save", ""));
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveSettings);
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setIcon(LXDG::findIcon("application-exit", ""));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    
    // Help menu
    QMenu *helpMenu = ui->menubar->addMenu(tr("&Help"));
    
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(LXDG::findIcon("help-about", ""));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupConnections()
{
    // Connect UI signals to slots
    connect(ui->combo_session_wfocus, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::settingChanged);
    connect(ui->combo_session_wloc, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::settingChanged);
    connect(ui->combo_session_wtheme, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::sessionThemeChanged);
    connect(ui->spin_session_wkspaces, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::settingChanged);
    connect(ui->text_file, &QPlainTextEdit::textChanged, 
            this, &MainWindow::settingChanged);
    connect(ui->radio_simple, &QRadioButton::toggled, 
            this, &MainWindow::switchEditor);
}

void MainWindow::loadSettings()
{
    loading = true;
    
    if(ui->radio_simple->isChecked()) {
        ui->stackedWidget->setCurrentWidget(ui->page_simple);
        
        // Setup focus policy options
        ui->combo_session_wfocus->clear();
        ui->combo_session_wfocus->addItem(tr("Click To Focus"), "ClickToFocus");
        ui->combo_session_wfocus->addItem(tr("Active Mouse Focus"), "MouseFocus");
        ui->combo_session_wfocus->addItem(tr("Strict Mouse Focus"), "StrictMouseFocus");
        
        // Setup window placement options
        ui->combo_session_wloc->clear();
        ui->combo_session_wloc->addItem(tr("Align in a Row"), "RowSmartPlacement");
        ui->combo_session_wloc->addItem(tr("Align in a Column"), "ColSmartPlacement");
        ui->combo_session_wloc->addItem(tr("Cascade"), "CascadePlacement");
        ui->combo_session_wloc->addItem(tr("Underneath Mouse"), "UnderMousePlacement");
        
        // Setup window themes
        ui->combo_session_wtheme->clear();
        QStringList dirs;
        dirs << LOS::AppPrefix() + "share/fluxbox/styles" 
             << QDir::homePath() + "/.fluxbox/styles";
        
        QFileInfoList fbstyles;
        for(int i = 0; i < dirs.length(); i++) {
            QDir fbdir(dirs[i]);
            if(fbdir.exists()) {
                fbstyles << fbdir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, 
                                               QDir::Name | QDir::IgnoreCase);
            }
        }
        
        QString lastdir;
        for(int i = 0; i < fbstyles.length(); i++) {
            if(lastdir != fbstyles[i].absolutePath()) {
                lastdir = fbstyles[i].absolutePath();
                if(ui->combo_session_wtheme->count() > 0) {
                    ui->combo_session_wtheme->insertSeparator(ui->combo_session_wtheme->count());
                }
            }
            ui->combo_session_wtheme->addItem(fbstyles[i].fileName(), fbstyles[i].absoluteFilePath());
        }
        
        // Load current settings from fluxbox-init file
        QStringList FB = readFile(QString(getenv("XDG_CONFIG_HOME")) + "/lumina-desktop/fluxbox-init");
        QString val;
        
        // Window placement
        val = FB.filter("session.screen0.windowPlacement:").join("").section(":", 1, 1).simplified();
        int index = ui->combo_session_wloc->findData(val);
        if(index < 0) index = 0;
        ui->combo_session_wloc->setCurrentIndex(index);
        
        // Window focus
        val = FB.filter("session.screen0.focusModel:").join("").section(":", 1, 1).simplified();
        index = ui->combo_session_wfocus->findData(val);
        if(index < 0) index = 0;
        ui->combo_session_wfocus->setCurrentIndex(index);
        
        // Window theme
        val = FB.filter("session.styleFile:").join("").section(":", 1, 1).simplified();
        index = ui->combo_session_wtheme->findData(val);
        if(index < 0) index = 0;
        ui->combo_session_wtheme->setCurrentIndex(index);
        
        // Number of workspaces
        val = FB.filter("session.screen0.workspaces:").join("").section(":", 1, 1).simplified();
        if(!val.isEmpty()) {
            ui->spin_session_wkspaces->setValue(val.toInt());
        }
        
    } else {
        // Advanced editor
        ui->stackedWidget->setCurrentWidget(ui->page_advanced);
        ui->text_file->setPlainText(readFile(QString(getenv("XDG_CONFIG_HOME")) + "/lumina-desktop/fluxbox-init").join("\n"));
    }
    
    QApplication::processEvents();
    loading = false;
}

void MainWindow::saveSettings()
{
    QStringList FB;
    QStringList FBNS;
    
    FBNS = readFile(QString(getenv("XDG_CONFIG_HOME")) + "/lumina-desktop/fluxbox-init -no-slit");
    
    if(ui->radio_simple->isChecked()) {
        FB = readFile(QString(getenv("XDG_CONFIG_HOME")) + "/lumina-desktop/fluxbox-init");
        
        // Window placement
        int index = FB.indexOf(FB.filter("session.screen0.windowPlacement:").join(""));
        QString line = "session.screen0.windowPlacement:\t" + 
                      ui->combo_session_wloc->itemData(ui->combo_session_wloc->currentIndex()).toString();
        if(index < 0) {
            FB << line;
        } else {
            FB[index] = line;
        }
        
        // Window focus
        index = FB.indexOf(FB.filter("session.screen0.focusModel:").join(""));
        line = "session.screen0.focusModel:\t" + 
               ui->combo_session_wfocus->itemData(ui->combo_session_wfocus->currentIndex()).toString();
        if(index < 0) {
            FB << line;
        } else {
            FB[index] = line;
        }
        
        // Window theme (for no-slit file)
        index = FBNS.indexOf(FBNS.filter("session.styleFile:").join(""));
        line = "session.styleFile:\t" + 
               ui->combo_session_wtheme->itemData(ui->combo_session_wtheme->currentIndex()).toString();
        if(index < 0) {
            FBNS << line;
        } else {
            FBNS[index] = line;
        }
        
        // Window theme (for main file)
        index = FB.indexOf(FB.filter("session.styleFile:").join(""));
        line = "session.styleFile:\t" + 
               ui->combo_session_wtheme->itemData(ui->combo_session_wtheme->currentIndex()).toString();
        if(index < 0) {
            FB << line;
        } else {
            FB[index] = line;
        }
        
        // Workspace number
        index = FB.indexOf(FB.filter("session.screen0.workspaces:").join(""));
        line = "session.screen0.workspaces:\t" + QString::number(ui->spin_session_wkspaces->value());
        if(index < 0) {
            FB << line;
        } else {
            FB[index] = line;
        }
        
    } else {
        // Advanced editor
        FB = ui->text_file->toPlainText().split("\n");
    }
    
    // Save the fluxbox settings
    bool ok = overwriteFile(QString(getenv("XDG_CONFIG_HOME")) + "/lumina-desktop/fluxbox-init", FB);
    overwriteFile(QString(getenv("XDG_CONFIG_HOME")) + "/lumina-desktop/fluxbox-init -no-slit", FBNS);
    
    if(ok) {
        showStatusMessage(tr("Settings saved successfully"));
    } else {
        showStatusMessage(tr("Error: Could not save fluxbox-init file"));
        qDebug() << "Warning: Could not save fluxbox-init";
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About Window Manager Settings"),
        tr("<h3>Window Manager Settings</h3>"
           "<p>Configure window manager behavior, themes, and workspace settings.</p>"
           "<p>Part of the Lumina Control Center (LCC) suite.</p>"
           "<p><b>Version:</b> 1.0<br>"
           "<b>Copyright:</b> 2024, jt(q5sys)<br>"
           "<b>License:</b> 3-clause BSD</p>"));
}

void MainWindow::settingChanged()
{
    if(!loading) {
        // Settings have been modified
        showStatusMessage(tr("Settings modified - remember to save"));
    }
}

void MainWindow::sessionThemeChanged()
{
    // Update the Fluxbox Theme preview
    QString previewfile = ui->combo_session_wtheme->itemData(ui->combo_session_wtheme->currentIndex()).toString();
    previewfile.append(previewfile.endsWith("/") ? "preview.jpg" : "/preview.jpg");
    
    if(QFile::exists(previewfile)) {
        ui->label_session_wpreview->setPixmap(QPixmap(previewfile));
    } else {
        ui->label_session_wpreview->setText(tr("No Preview Available"));
    }
    
    settingChanged();
}

void MainWindow::switchEditor()
{
    loadSettings();
}

void MainWindow::updateIcons()
{
    // Update icons when theme changes
    this->setWindowIcon(LXDG::findIcon("preferences-system-windows", ""));
}

void MainWindow::showStatusMessage(const QString &message, int timeout)
{
    ui->statusbar->showMessage(message, timeout);
}

// File operations
QStringList MainWindow::readFile(QString path)
{
    QStringList out;
    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream txt(&file);
        while(!txt.atEnd()) {
            out << txt.readLine();
        }
        file.close();
    }
    return out;
}

bool MainWindow::overwriteFile(QString path, QStringList contents)
{
    QFile file(path);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream txt(&file);
        for(int i = 0; i < contents.length(); i++) {
            txt << contents[i] + "\n";
        }
        file.close();
        return true;
    }
    return false;
}
