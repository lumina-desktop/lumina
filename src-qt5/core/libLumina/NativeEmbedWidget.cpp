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
#include <xcb/xcb_event.h>
#include <xcb/xcb_image.h>
#include <xcb/composite.h>
#include <X11/extensions/Xdamage.h>

#define DISABLE_COMPOSITING false

inline void registerClientEvents(WId id){
  uint32_t value_list[1] = { (XCB_EVENT_MASK_PROPERTY_CHANGE
			| XCB_EVENT_MASK_BUTTON_PRESS
			| XCB_EVENT_MASK_BUTTON_RELEASE
 			| XCB_EVENT_MASK_POINTER_MOTION
			| XCB_EVENT_MASK_BUTTON_MOTION
			| XCB_EVENT_MASK_EXPOSURE
			| XCB_EVENT_MASK_STRUCTURE_NOTIFY
			| XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
			| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
			| XCB_EVENT_MASK_ENTER_WINDOW)
			};
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, value_list);
}

// ============
//      PRIVATE
// ============
//Simplification functions for the XCB/XLib interactions
void NativeEmbedWidget::syncWinSize(QSize sz){
  if(WIN==0 || paused){ return; }
  else if(!sz.isValid()){ sz = this->size(); } //use the current widget size
  //qDebug() << "Sync Window Size:" << sz;
  //if(sz == winSize){ return; } //no change
  QPoint pt(0,0);
   if(!DISABLE_COMPOSITING){ pt = this->mapToGlobal(QPoint(0,0)); }
    const uint32_t valList[4] = {(uint32_t) pt.x(), (uint32_t) pt.y(), (uint32_t) sz.width(), (uint32_t) sz.height()};
    const uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
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
  reregisterEvents();
  QTimer::singleShot(0,this, SLOT(repaintWindow()));
}

QImage NativeEmbedWidget::windowImage(QRect geom){
  //Pull the XCB pixmap out of the compositing layer
  xcb_pixmap_t pix = xcb_generate_id(QX11Info::connection());

  /*xcb_composite_get_overlay_window_reply_t *wreply = xcb_composite_get_overlay_window_reply( QX11Info::connection(),
			xcb_composite_get_overlay_window_unchecked(QX11Info::connection(), WIN->id()), NULL);
  if(wreply!=0){
    xcb_composite_name_window_pixmap(QX11Info::connection(), wreply->overlay_win, pix);
    free(wreply);
  }else{*/
    xcb_composite_name_window_pixmap(QX11Info::connection(), WIN->id(), pix);
  //}
  if(pix==0){ qDebug() << "Got blank pixmap!"; return QImage(); }
  //Convert this pixmap into a QImage
  xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, 0, 0, this->width(), this->height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
  //xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, geom.x(), geom.y(), geom.width(), geom.height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
  if(ximg == 0){ qDebug() << "Got blank image!"; return QImage(); }
  QImage img(ximg->data, ximg->width, ximg->height, ximg->stride, QImage::Format_ARGB32_Premultiplied);
  img = img.copy(); //detach this image from the XCB data structures before we clean them up
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
  paused = false;
  this->setMouseTracking(true);
  //this->setSizeIncrement(2,2);
}

bool NativeEmbedWidget::embedWindow(NativeWindow *window){
  WIN = window;

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
  if(!DISABLE_COMPOSITING){
    xcb_composite_redirect_window(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);
    xcb_composite_redirect_subwindows(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //AUTOMATIC); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);

    //Now create/register the damage handler
    // -- XCB (Note: The XCB damage registration is completely broken at the moment - 9/15/15, Ken Moore)
    //  -- Retested 6/29/17 (no change) Ken Moore
    //xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
    //xcb_damage_create(QX11Info::connection(), dmgID, WIN->id(), XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
    // -- XLib (Note: This is only used because the XCB routine above does not work - needs to be fixed upstream in XCB itself).
    Damage dmgID = XDamageCreate(QX11Info::display(), WIN->id(), XDamageReportRawRectangles);

    WIN->addDamageID( (uint) dmgID); //save this for later
  }else{
    xcb_reparent_window(QX11Info::connection(), WIN->id(), this->winId(), 0, 0);
  }
  WIN->addFrameWinID(this->winId());
  connect(WIN, SIGNAL(VisualChanged()), this, SLOT(repaintWindow()) ); //make sure we repaint the widget on visual change

  registerClientEvents(WIN->id());
  //registerClientEvents(this->winId());
  //qDebug() << "Events Registered:" << WIN->id() << this->winId();
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

void NativeEmbedWidget::raiseWindow(){
  uint32_t val = XCB_STACK_MODE_ABOVE;
  xcb_configure_window(QX11Info::connection(),  WIN->id(), XCB_CONFIG_WINDOW_STACK_MODE, &val);
}

// ==============
//   PUBLIC SLOTS
// ==============
//Pause/resume
void NativeEmbedWidget::pause(){
  if(winImage.isNull()){ repaintWindow(); } //make sure we have one image already cached first
  paused = true;
}

void NativeEmbedWidget::resume(){
  paused = false;
  syncWinSize();
  //showWindow();
  repaintWindow(); //update the cached image right away
}

void NativeEmbedWidget::resyncWindow(){
   if(WIN==0){ return; }

  // Attempt 1 : spec says to send an artificial configure event to the window
  /*QRect geom = WIN->geometry();
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
  */

  // Attempt 2 : Just jitter the window size by 1 pixel really quick so the window knows to update it's geometry
  /*QSize sz = this->size();
  uint32_t valList[2] = {(uint32_t) sz.width()-1, (uint32_t) sz.height()};
    uint32_t mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    xcb_configure_window(QX11Info::connection(), WIN->id(), mask, valList);
    xcb_flush(QX11Info::connection());
  valList[0] = (uint32_t) sz.width();
    xcb_configure_window(QX11Info::connection(), WIN->id(), mask, valList);
    xcb_flush(QX11Info::connection());*/

  //Make sure the window size is syncronized and visual up to date
  syncWinSize();
  QTimer::singleShot(10, this, SLOT(repaintWindow()) );
}

void NativeEmbedWidget::repaintWindow(){
  if(DISABLE_COMPOSITING){ return; }
  //qDebug() << "Update Window Image:" << !paused;
  if(paused){ return; }
    QImage tmp = windowImage( QRect(QPoint(0,0), this->size()) );
    if(!tmp.isNull()){
      winImage = tmp;
    }else{ qDebug() << "Got Null Image!!"; }
  this->parentWidget()->update();
}

void NativeEmbedWidget::reregisterEvents(){
  if(WIN!=0){ registerClientEvents(WIN->id()); }
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
  if(WIN==0 || DISABLE_COMPOSITING){ QWidget::paintEvent(ev); return; }
  else if( winImage.isNull() ){ /*QTimer::singleShot(0, this, SLOT(repaintWindow()) );*/ return; }
  else if(paused){ return; }
  //else if(this->size()!=winSize){ QTimer::singleShot(0,this, SLOT(syncWinSize())); return; } //do not paint here - waiting to re-sync the sizes
  //else if(this->size() != winImage.size()){ QTimer::singleShot(0, this, SLOT(repaintWindow()) ); return; }
  //Need to paint the image from the window onto the widget as an overlay
  QRect geom = ev->rect(); //atomic updates
  //geom.adjust(-10,-10,10,10); //add an additional few pixels in each direction to be painted
  geom = geom.intersected(QRect(0,0,this->width(), this->height())); //ensure intersection with actual window
    if( !QRect(QPoint(0,0),winImage.size()).contains(geom) ){ QTimer::singleShot(0,this, SLOT(repaintWindow()) );return; }
    QPainter P(this);
      P.setClipping(true);
      P.setClipRect(0,0,this->width(), this->height());
    //qDebug() << "Paint Embed Window:" << geom << winImage.size();
    if(winImage.size() == this->size()){
      P.drawImage( geom , winImage, geom, Qt::NoOpaqueDetection); //1-to-1 mapping
      //Note: Qt::NoOpaqueDetection Speeds up the paint by bypassing the checks to see if there are [semi-]transparent pixels
      //  Since this is an embedded image - we fully expect there to be transparency all/most of the time.
    }else{
      P.drawImage( geom , winImage);
    }
    //else{ QImage scaled = winImage.scaled(geom.size()); P.drawImage(geom, scaled); }
    //P.drawImage( geom , winImage, geom, Qt::NoOpaqueDetection); //1-to-1 mapping
  //Note: Qt::NoOpaqueDetection Speeds up the paint by bypassing the checks to see if there are [semi-]transparent pixels
  //  Since this is an embedded image - we fully expect there to be transparency all/most of the time.

}

void NativeEmbedWidget::enterEvent(QEvent *ev){
  QWidget::enterEvent(ev);
  //this->grabMouse(); //xcb_grab_pointer_unchecked(QX11Info::connection(), );
}

void NativeEmbedWidget::leaveEvent(QEvent *ev){
  QWidget::leaveEvent(ev);
  //this->releaseMouse(); //xcb_ungrab_pointer(QX11Info::connection(), XCB_CURRENT_TIME);
}

bool NativeEmbedWidget::nativeEvent(const QByteArray &eventType, void *message, long *result){
  /*if(eventType=="xcb_generic_event_t" && WIN!=0){
    //Convert to known event type (for X11 systems)
    xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
    //qDebug() << "Got Embed Window Event:" << xcb_event_get_label(ev->response_type & XCB_EVENT_RESPONSE_TYPE_MASK) << xcb_event_get_request_label(ev->response_type);
    uint32_t mask = 0;
    switch( ev->response_type  & XCB_EVENT_RESPONSE_TYPE_MASK){
	    case XCB_BUTTON_PRESS:
		//This is a mouse button press
		mask = XCB_EVENT_MASK_BUTTON_PRESS;
		break;
	    case XCB_BUTTON_RELEASE:
		//This is a mouse button release
		//qDebug() << "Button Release Event";
		mask = XCB_EVENT_MASK_BUTTON_RELEASE;
		break;
	    case XCB_MOTION_NOTIFY:
		//This is a mouse movement event
		mask = XCB_EVENT_MASK_POINTER_MOTION;
	        break;
	    case XCB_ENTER_NOTIFY:
		//This is a mouse movement event when mouse goes over a new window
		mask = XCB_EVENT_MASK_ENTER_WINDOW;
	        break;
	    case XCB_LEAVE_NOTIFY:
		//This is a mouse movement event when mouse goes leaves a window
		mask = XCB_EVENT_MASK_LEAVE_WINDOW;
	        break;
	    default:
		mask = 0;
    }

    //Now forward this event on to the embedded window
    if(mask!=0){
      qDebug() << " - Got a mouse event";
      xcb_send_event(QX11Info::connection(), true, WIN->id(),mask, (char*) ev);
      return true;
    }
  }*/
  return false;
}
