//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeEmbedWidget.h"

#include <QPainter>
#include <QX11Info>
#include <QDebug>

#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_image.h>
#include <xcb/composite.h>
#include <X11/extensions/Xdamage.h>

#define NORMAL_WIN_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | 	\
			XCB_EVENT_MASK_BUTTON_RELEASE | 	\
 			XCB_EVENT_MASK_POINTER_MOTION |	\
			XCB_EVENT_MASK_BUTTON_MOTION |	\
			XCB_EVENT_MASK_EXPOSURE |		\
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
			XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
			XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
			XCB_EVENT_MASK_ENTER_WINDOW| \
			XCB_EVENT_MASK_PROPERTY_CHANGE)

inline void registerClientEvents(WId id){
  uint32_t value_list[1] = {NORMAL_WIN_EVENT_MASK};
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, value_list);
}

// ============
//      PRIVATE
// ============
//Simplification functions for the XCB/XLib interactions
void NativeEmbedWidget::syncWinSize(QSize sz){
  if(WIN==0){ return; }
  //qDebug() << "Sync Window Size:" << sz;
    xcb_configure_window_value_list_t  valList;
    valList.x = 0;
    valList.y = 0;
    valList.width = sz.width();
    valList.height = sz.height();
    uint16_t mask = 0;
      mask = mask | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
      mask = mask | XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    xcb_configure_window_aux(QX11Info::connection(), WIN->id(), mask, &valList);
    //xcb_flush(QX11Info::connection());
}

void NativeEmbedWidget::syncWidgetSize(QSize sz){
  //qDebug() << "Sync Widget Size:" << sz;
  this->resize(sz);
}

void NativeEmbedWidget::hideWindow(){
  xcb_unmap_window(QX11Info::connection(), WIN->id());
}

void NativeEmbedWidget::showWindow(){
  xcb_map_window(QX11Info::connection(), WIN->id());
}

QImage NativeEmbedWidget::windowImage(QRect geom){
  //Pull the XCB pixmap out of the compositing layer
  xcb_pixmap_t pix = xcb_generate_id(QX11Info::connection());
  xcb_composite_name_window_pixmap(QX11Info::connection(), WIN->id(), pix);
  if(pix==0){ return QImage(); }

  //Convert this pixmap into a QImage
  xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, geom.x(), geom.y(), geom.width(), geom.height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
  if(ximg == 0){ return QImage(); }
  QImage img(ximg->data, ximg->width, ximg->height, ximg->stride, QImage::Format_ARGB32_Premultiplied);
  img = img.copy(); //detach this image from the XCB data structures
  xcb_image_destroy(ximg);

  //Cleanup the XCB data structures
  xcb_free_pixmap(QX11Info::connection(), pix);

  return img;

}

// ============
//      PUBLIC
// ============
NativeEmbedWidget::NativeEmbedWidget(QWidget *parent) : QWidget(parent){
  WIN = 0; //nothing embedded yet
  this->setStyleSheet("background: transparent;"); //this widget should be fully-transparent to Qt itself (will paint on top of that)
}

bool NativeEmbedWidget::embedWindow(NativeWindow *window){
  WIN = window;
  //PIXBACK = xcb_generate_id(QX11Info::connection());
  xcb_reparent_window(QX11Info::connection(), WIN->id(), this->winId(), 0, 0);
  //Now send the embed event to the app
  //qDebug() << " - send _XEMBED event";
  /*xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = WIN->id();
    event.type = obj->ATOMS["_XEMBED"]; //_XEMBED
    event.data.data32[0] = XCB_TIME_CURRENT_TIME; //CurrentTime;
    event.data.data32[1] = 0; //XEMBED_EMBEDDED_NOTIFY
    event.data.data32[2] = 0;
    event.data.data32[3] = this->winId(); //WID of the container
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, WIN->id(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
  */
  //Now setup any redirects and return
  xcb_composite_redirect_window(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);
  xcb_composite_redirect_subwindows(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);

  //Now create/register the damage handler
  // -- XCB (Note: The XCB damage registration is completely broken at the moment - 9/15/15, Ken Moore)
  //  -- Retested 6/29/17 (no change) Ken Moore
  //xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
  //xcb_damage_create(QX11Info::connection(), dmgID, WIN->id(), XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
  // -- XLib (Note: This is only used because the XCB routine above does not work - needs to be fixed upstream in XCB itself).
  Damage dmgID = XDamageCreate(QX11Info::display(), WIN->id(), XDamageReportRawRectangles);

  WIN->addDamageID( (uint) dmgID); //save this for later
  WIN->addFrameWinID(this->winId());
  connect(WIN, SIGNAL(VisualChanged()), this, SLOT(update()) ); //make sure we repaint the widget on visual change

  registerClientEvents(WIN->id());
  registerClientEvents(this->winId());
  return true;
}

bool NativeEmbedWidget::detachWindow(){
  xcb_reparent_window(QX11Info::connection(), WIN->id(), QX11Info::appRootWindow(), -1, -1);
  WIN = 0;
  return true;
}

bool NativeEmbedWidget::isEmbedded(){
  return (WIN!=0);
}

// ==============
//   PUBLIC SLOTS
// ==============
void NativeEmbedWidget::resyncWindow(){
  QSize sz = this->size();
   if(WIN==0){ return; }
  //Just jitter the x origin of the window 1 pixel so the window knows to re-check it's global position
  //  before creating child windows (menu's in particular).

  //qDebug() << "Sync Window Size:" << sz;
    xcb_configure_window_value_list_t  valList;
    valList.x = 0;
    valList.y = 0;
    valList.width = sz.width();
    //valList.height = sz.height()-1;
    uint16_t mask = 0;
      mask = mask | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
      mask = mask | XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    //xcb_configure_window_aux(QX11Info::connection(), WIN->id(), mask, &valList);
   valList.height = sz.height();
    //xcb_flush(QX11Info::connection());
    xcb_configure_window_aux(QX11Info::connection(), WIN->id(), mask, &valList);
}

// ==============
//      PROTECTED
// ==============
void NativeEmbedWidget::resizeEvent(QResizeEvent *ev){
  if(WIN!=0){ syncWinSize(ev->size()); } //syncronize the window with the new widget size
  QWidget::resizeEvent(ev);
}

void NativeEmbedWidget::showEvent(QShowEvent *ev){
  if(WIN!=0){ showWindow(); }
  QWidget::showEvent(ev);
}

void NativeEmbedWidget::hideEvent(QHideEvent *ev){
  if(WIN!=0){ hideWindow(); }
  QWidget::hideEvent(ev);
}

void NativeEmbedWidget::paintEvent(QPaintEvent *ev){
  //QWidget::paintEvent(ev); //ensure all the Qt-compositing is done first
  if(WIN==0){ QWidget::paintEvent(ev); return; }
  //Need to paint the image from the window onto the widget as an overlay
  QRect geom = QRect(0,0,this->width(), this->height()); //always paint the whole window
  //qDebug() << "Get Paint image";
  QImage img = windowImage(geom);
  if(!img.isNull()){
    QPainter P(this);
    P.drawImage( geom , img, geom, Qt::NoOpaqueDetection); //1-to-1 mapping
    //qDebug() << "Painted Rect:" << ev->rect() << this->geometry();
  //Note: Qt::NoOpaqueDetection Speeds up the paint by bypassing the checks to see if there are [semi-]transparent pixels
  //  Since this is an embedded image - we fully expect there to be transparency most of the time.
  }
  //qDebug() << "Done Painting";
}
