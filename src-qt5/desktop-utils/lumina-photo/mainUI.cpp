//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2018, Loïc Bartoletti
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainUI.h"
#include "ui_mainUI.h"
#include <QDebug>

#include <LUtils.h>
#include <LuminaXDG.h>
#include <QDesktopServices>
#include <QFileDialog>
#include <QImageWriter>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>

#include <QtPrintSupport>

MainUI::MainUI () : QMainWindow (), ui (new Ui::MainUI ())
{
    settings = LUtils::openSettings ("lumina-desktop", "lumina-photo", this);
    ppath = settings->value ("previous-path", QDir::homePath ()).toString ();
    pfilter = settings->value ("previous-filter", "image/jpeg").toString ();

    ui->setupUi (this);
    // Any special UI changes

    graphicsScene = new QGraphicsScene ();
    ui->photoView->setScene (graphicsScene);

    setupConnections ();
    setupIcons ();
    setupFilters ();
}

MainUI::~MainUI ()
{
}

void MainUI::loadArguments (QStringList args)
{
    files.clear ();
    // Parse out the arguments
    for (int i = 0; i < args.length (); i++)
    {
        if (!QImageReader::imageFormat (args[i]).isEmpty ())
        {
            files.append (args[i]);
        }
    }
    if (files.count () > 0)
    {
        idx = 0;
        loadFile (files.at (0));
    }
}

void MainUI::setupConnections ()
{
    connect (ui->actionZoom_In, SIGNAL (triggered ()), this, SLOT (zoomInToggled ()));
    connect (ui->actionZoom_Out, SIGNAL (triggered ()), this, SLOT (zoomOutToggled ()));
    connect (ui->action_Fit_to_windows, SIGNAL (triggered ()), this, SLOT (fitZoomToggled ()));
    connect (ui->action_Normal_Zoom, SIGNAL (triggered ()), this, SLOT (normalZoomToggled ()));
    connect (ui->action_Exit, SIGNAL (triggered ()), this, SLOT (closeApplication ()));
    connect (ui->action_Open, SIGNAL (triggered ()), this, SLOT (open ()));
    connect (ui->action_Save_as, SIGNAL (triggered ()), this, SLOT (save ()));
    connect (ui->action_Print, SIGNAL (triggered ()), this, SLOT (print ()));
    connect (ui->actionNext, SIGNAL (triggered ()), this, SLOT (nextToggled ()));
    connect (ui->actionBack, SIGNAL (triggered ()), this, SLOT (backToggled ()));
    connect (ui->actionFirst, SIGNAL (triggered ()), this, SLOT (firstToggled ()));
    connect (ui->actionLast, SIGNAL (triggered ()), this, SLOT (lastToggled ()));
}

void MainUI::setupIcons ()
{
    ui->action_Exit->setIcon (LXDG::findIcon ("application-close", "dialog-close"));
    ui->action_Fit_to_windows->setIcon (LXDG::findIcon ("zoom-fit-best", ""));
    ui->action_Normal_Zoom->setIcon (LXDG::findIcon ("zoom-original", ""));
    ui->actionZoom_In->setIcon (LXDG::findIcon ("zoom-in", ""));
    ui->actionZoom_Out->setIcon (LXDG::findIcon ("zoom-out", ""));
    ui->actionFirst->setIcon (LXDG::findIcon ("arrow-left-double", ""));
    ui->actionBack->setIcon (LXDG::findIcon ("arrow-left", ""));
    ui->actionLast->setIcon (LXDG::findIcon ("arrow-right-double", ""));
    ui->actionNext->setIcon (LXDG::findIcon ("arrow-right", ""));
}

void MainUI::setupFilters ()
{
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes ())
        mimeTypeFiltersReader.append (mimeTypeName);

    mimeTypeFiltersReader.sort ();

    foreach (const QByteArray &mimeTypeName, QImageWriter::supportedMimeTypes ())
        mimeTypeFiltersWriter.append (mimeTypeName);

    mimeTypeFiltersWriter.sort ();
}
// ==== PRIVATE SLOTS ====
void MainUI::fitZoomToggled ()
{
    ui->photoView->zoomFit ();
}

void MainUI::normalZoomToggled ()
{
    ui->photoView->zoomNormal ();
}

void MainUI::zoomOutToggled ()
{
    ui->photoView->zoomOut ();
}

void MainUI::zoomInToggled ()
{
    ui->photoView->zoomIn ();
}

void MainUI::firstToggled ()
{
    if (idx != 0)
    {
        idx = 0;
        loadFile (files.at (idx));
    }
}

void MainUI::lastToggled ()
{
    if (idx != files.count () - 1)
    {
        idx = files.count () - 1;
        loadFile (files.at (idx));
    }
}

void MainUI::backToggled ()
{
    if (idx > 0)
    {
        idx -= 1;
        loadFile (files.at (idx));
    }
}

void MainUI::nextToggled ()
{
    if (idx < files.count () - 1)
    {
        idx += 1;
        loadFile (files.at (idx));
    }
}

void MainUI::closeApplication ()
{
    this->close ();
}

bool MainUI::loadFile (const QString file)
{
    // Load the file as-is
    QImageReader reader (file);
    reader.setAutoTransform (true);
    actualImage = reader.read ();
    if (actualImage.isNull ())
    {
        QMessageBox::information (
        this, QGuiApplication::applicationDisplayName (),
        tr ("Cannot load %1: %2").arg (QDir::toNativeSeparators (file), reader.errorString ()));
        return false;
    }
    graphicsScene->clear ();
    graphicsScene->setSceneRect (actualImage.rect ());
    graphicsScene->addPixmap (QPixmap::fromImage (actualImage));

    ui->photoView->zoomFit ();
    return true;
}

void MainUI::open ()
{
    QFileDialog dialog (this, tr ("Open File"), ppath);
    dialog.setFileMode (QFileDialog::ExistingFiles);
    dialog.setAcceptMode (QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters (mimeTypeFiltersReader);
    dialog.selectMimeTypeFilter (pfilter);
    if (dialog.exec ())
    {
        files.clear ();
        files = dialog.selectedFiles ();
        idx = 0;

        ppath = files.at (idx).section ("/", 0, -2); // just the directory
        settings->setValue ("previous-path", ppath);
        pfilter = dialog.selectedMimeTypeFilter ();
        settings->setValue ("previous-filter", pfilter);
        loadFile (files.at (idx));
    }
}

bool MainUI::save ()
{
    QFileDialog dialog (this, tr ("Save as"), ppath);
    dialog.setFileMode (QFileDialog::AnyFile);
    dialog.setAcceptMode (QFileDialog::AcceptSave);
    dialog.setMimeTypeFilters (mimeTypeFiltersWriter);
    if (dialog.exec ())
    {
        QString filename = dialog.selectedFiles ().first ();
        QImageWriter writer (filename);

        if (!writer.write (actualImage))
        {
            QMessageBox::information (this, QGuiApplication::applicationDisplayName (),
                                      tr ("Cannot write %1: %2").arg (QDir::toNativeSeparators (filename)),
                                      writer.errorString ());
            return false;
        }
    }

    return true;
}

void MainUI::print ()
{
    QPrinter printer;
    QPrintDialog dialog (&printer, this);
    if (dialog.exec ())
    {
        QPainter painter (&printer);
        QRect rect = painter.viewport ();
        QSize size = actualImage.size ();
        size.scale (rect.size (), Qt::KeepAspectRatio);
        painter.setViewport (rect.x (), rect.y (), size.width (), size.height ());
        painter.setWindow (actualImage.rect ());
        painter.drawPixmap (0, 0, QPixmap::fromImage (actualImage));
    }
}
