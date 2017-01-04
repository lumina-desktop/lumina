//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "GlobalDefines.h"
//Initialize any global structures here
LXCB *LWM::SYSTEM = 0;

//Local includes
#include "WMSession.h"
#include "LWindow.h"
#include <QDialog>


//#define DEBUG 0
int main(int argc, char ** argv)
{
    qDebug() << "Starting lumina-wm...";
    LTHEME::LoadCustomEnvSettings();
    LSingleApplication a(argc, argv, "lumina-wm");
    if(!a.isPrimaryProcess()){ return 0; } //Inputs forwarded on to the primary already
    LuminaThemeEngine themes(&a);
    
    //Setup the global structures
    LWM::SYSTEM = new LXCB();
    if( a.inputlist.contains("--test-win") ){
      //Simple override to test out the window class
      qDebug() << "Starting window test...";
      QLabel dlg(0, Qt::Window | Qt::BypassWindowManagerHint); //this test should be ignored by the current WM
      dlg.setText("Sample Window");
      dlg.setWindowTitle("Test");
      dlg.resize(200,100);
      dlg.setStyleSheet("background: rgba(255,255,255,100); color: black;");
      dlg.move(100,100);
      dlg.show();
      //dlg.move(100,100);
      qDebug() << " - Loading window frame...";
      LWindow win(dlg.winId()); //have it wrap around the dialog
      qDebug() << " - Show frame...";
      win.frame()->windowChanged(LWM::Show);
      qDebug() << " - Start event loop...";
      a.setQuitOnLastWindowClosed(true);
      return a.exec();
    }
    WMSession w;
    w.start(a.inputlist.contains("--test-ss"));
    QObject::connect(&themes, SIGNAL(updateIcons()), &w, SLOT(reloadIcons()) );
    QObject::connect(&a, SIGNAL(InputsAvailable(QStringList)), &w, SLOT(newInputsAvailable(QStringList)) );
    if(!a.inputlist.isEmpty()){ w.newInputsAvailable(a.inputlist); }
    int retCode = a.exec();
    
    return retCode;
}
