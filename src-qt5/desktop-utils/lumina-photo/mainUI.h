//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2018, Loïc Bartoletti
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PHOTO_MAIN_UI_H
#define _LUMINA_PHOTO_MAIN_UI_H

#include "PhotoView.h"
#include <QAction>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QSettings>


namespace Ui
{
class MainUI;
};

class MainUI : public QMainWindow
{
    Q_OBJECT
    public:
    MainUI ();
    ~MainUI ();

    void loadArguments (QStringList);

private:
    Ui::MainUI *ui;

    void setupConnections ();
    void setupIcons ();
    void setupFilters();

    bool loadFile (const QString); // simplification for loading files

    QGraphicsScene *graphicsScene;
    QImage actualImage;

    QStringList files;
    int idx = 0;

    QStringList mimeTypeFiltersReader;
    QStringList mimeTypeFiltersWriter;

    QString ppath; //previous file path
    QString pfilter; //previous filter
    QSettings *settings;

    private slots:
    void open ();
    bool save();
    void print();
    void closeApplication ();

    // Toolbar actions
    void fitZoomToggled ();
    void normalZoomToggled ();
    void zoomOutToggled ();
    void zoomInToggled ();
    void nextToggled ();
    void backToggled ();
    void lastToggled ();
    void firstToggled ();
};

#endif
