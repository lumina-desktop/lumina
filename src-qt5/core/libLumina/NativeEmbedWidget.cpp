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
  if(!sz.isValid()){ sz = this->size(); } //use the current widget size
  //qDebug() << "Sync Window Size:" << sz;
  if(sz == winSize){ return; } //no change
    const uint32_t valList[2] = {(uint32_t) sz.width(), (uint32_t) sz.height()};
    const uint32_t mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    xcb_configure_window(QX11Info::connection(), WIN->id(), mask, valList);
  winSize = sz; //save this for checking later
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
  this->setSizeIncrement(2,2);
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
  connect(WIN, SIGNAL(VisualChanged()), this, SLOT(repaintWindow()) ); //make sure we repaint the widget on visual change

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
   if(WIN==0){ return; }
  return; //skip the stuff below (not working)
  QRect geom = WIN->geometry();
  //Send an artificial configureNotify event to the window with the global position/size included
  xcb_configure_notify_event_t event;
    event.x = geom.x() + this->pos().x();
    event.y = geom.y() + this->pos().y();
    event.width = this->width();
    event.height = this->height();
    event.border_width = 0;
    event.above_sibling = XCB_NONE;
    event.override_redirect = false;
    event.window = WIN->id();
    event.event = WIN->id();
    event.response_type = XCB_CONFIGURE_NOTIFY;
  xcb_send_event(QX11Info::connection(), false, WIN->id(), XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char *) &event);

  xcb_flush(QX11Info::connection());
}

void NativeEmbedWidget::repaintWindow(){
  this->update();
}
// ==============
//      PROTECTED
// ==============
void NativeEmbedWidget::resizeEvent(QResizeEvent *ev){
  QWidget::resizeEvent(ev);
  if(WIN!=0){
    syncWinSize(ev->size());
  } //syncronize the window with the new widget size
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
  if(this->size()!=winSize){ return; } //do not paint here - waiting to re-sync the sizes
  if(WIN==0){ QWidget::paintEvent(ev); return; }
  //Need to paint the image from the window onto the widget as an overlay
  QRect geom = QRect(0,0,this->width(), this->height()); //always paint the whole window
  //qDebug() << "Get Paint image:" << ev->rect() << geom;
  //geom = ev->rect(); //atomic updates
  //geom.adjust(-1,-1,1,1); //add an additional pixel in each direction to be painted
  //geom = geom.intersected(QRect(0,0,this->width(), this->height())); //ensure intersection with actual window
  QImage img = windowImage(geom);
  if(!img.isNull()){
    if(img.size() != geom.size()){ return; }
    QPainter P(this);
    P.drawImage( geom , img, QRect(geom.topLeft(), img.size()), Qt::NoOpaqueDetection); //1-to-1 mapping
    //qDebug() << "Painted Rect:" << ev->rect() << this->geometry();
  //Note: Qt::NoOpaqueDetection Speeds up the paint by bypassing the checks to see if there are [semi-]transparent pixels
  //  Since this is an embedded image - we fully expect there to be transparency most of the time.
  }
  //qDebug() << "Done Painting";
}
