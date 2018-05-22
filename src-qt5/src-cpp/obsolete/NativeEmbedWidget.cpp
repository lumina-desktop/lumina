//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeEmbedWidget.h"

#include <QPainter>
#include <QX11Info>
#include <QApplication>
#include <QScreen>
#include <QDebug>

#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_image.h>
//#include <xcb/render.h>
//#include <xcb/xcb_renderutil.h>
#include <xcb/composite.h>
#include <X11/extensions/Xdamage.h>

#define DISABLE_COMPOSITING true

/*inline xcb_render_pictformat_t get_pictformat(){
  static xcb_render_pictformat_t format = 0;
  if(format==0){
    xcb_render_query_pict_formats_reply_t *reply = xcb_render_query_pict_formats_reply( QX11Info::connection(), xcb_render_query_pict_formats(QX11Info::connection()), NULL);
    format = xcb_render_util_find_standard_format(reply, XCB_PICT_STANDARD_ARGB_32)->id;
    free(reply);
  }
  return format;
}


inline void renderWindowToWidget(WId id, QWidget *widget, bool hastransparency = true){
  //window and widget are assumed to be the same size
  //Pull the XCB pixmap out of the compositing layer
  xcb_pixmap_t pix = xcb_generate_id(QX11Info::connection());
    xcb_composite_name_window_pixmap(QX11Info::connection(), WIN->id(), pix);
  if(pix==0){ qDebug() << "Got blank pixmap!"; return; }

  xcb_render_picture_t pic_id = xcb_generate_id(QX11Info::connection());
  xcb_render_create_picture_aux(QX11Info::connection(), pic_id, pix, get_pictformat() , 0, NULL);
  //
  xcb_render_composite(QX11Info::connection(), hastransparency ? XCB_RENDER_PICT_OP_OVER : XCB_RENDER_PICT_OP_SRC, pic_id, XCB_RENDER_PICTURE_NONE, widget->x11RenderHandle(),
				0, 0, 0, 0, 0, 0, (uint16_t) widget->width(), (uint16_t) widget->height() );
}*/

#define CLIENT_EVENT_MASK (XCB_EVENT_MASK_PROPERTY_CHANGE |  \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_FOCUS_CHANGE | \
                          XCB_EVENT_MASK_POINTER_MOTION)

#define FRAME_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | \
                          XCB_EVENT_MASK_BUTTON_RELEASE | \
                          XCB_EVENT_MASK_POINTER_MOTION | \
                          XCB_EVENT_MASK_EXPOSURE | \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | \
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_ENTER_WINDOW)

inline void registerClientEvents(WId id, bool client = true){
  uint32_t values[] = {XCB_NONE};
  values[0] = client ? CLIENT_EVENT_MASK : FRAME_EVENT_MASK ;
  /*{ (XCB_EVENT_MASK_PROPERTY_CHANGE
			| XCB_EVENT_MASK_BUTTON_PRESS
			| XCB_EVENT_MASK_BUTTON_RELEASE
 			| XCB_EVENT_MASK_POINTER_MOTION
			| XCB_EVENT_MASK_BUTTON_MOTION
			| XCB_EVENT_MASK_EXPOSURE
			| XCB_EVENT_MASK_STRUCTURE_NOTIFY
			| XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
			| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
			| XCB_EVENT_MASK_ENTER_WINDOW)
			};*/
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, values);
}

// ============
//      PRIVATE
// ============
//Simplification functions for the XCB/XLib interactions
void NativeEmbedWidget::syncWinSize(QSize sz){
  if(WIN==0){ return; }
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
  //qDebug() << "Hide Embed Window";
  xcb_unmap_window(QX11Info::connection(), WIN->id());
}

void NativeEmbedWidget::showWindow(){
  //qDebug() << "Show Embed Window";
  xcb_map_window(QX11Info::connection(), WIN->id());
  reregisterEvents();
  if(!DISABLE_COMPOSITING){
    QTimer::singleShot(0,this, SLOT(repaintWindow()));
  }
}

QImage NativeEmbedWidget::windowImage(QRect geom){
  //if(DISABLE_COMPOSITING){
    if(!this->isVisible()){ return QImage(); } //nothing to grab yet
    QList<QScreen*> screens = static_cast<QApplication*>( QApplication::instance() )->screens();
    //for(int i=0; i<screens.length(); i++){
      //if(screens[i]->contains(this)){
    if(!screens.isEmpty()){
        return screens[0]->grabWindow(WIN->id(), geom.x(), geom.y(), geom.width(), geom.height()).toImage();
    }
      //}
    //}
    return QImage();
  /*}else{
    //Pull the XCB pixmap out of the compositing layer
    xcb_pixmap_t pix = xcb_generate_id(QX11Info::connection());
    xcb_composite_name_window_pixmap(QX11Info::connection(), WIN->id(), pix);
    if(pix==0){ qDebug() << "Got blank pixmap!"; return QImage(); }

    //Convert this pixmap into a QImage
    //xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, 0, 0, this->width(), this->height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, geom.x(), geom.y(), geom.width(), geom.height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    if(ximg == 0){ qDebug() << "Got blank image!"; return QImage(); }
    QImage img(ximg->data, ximg->width, ximg->height, ximg->stride, QImage::Format_ARGB32_Premultiplied);
    img = img.copy(); //detach this image from the XCB data structures before we clean them up, otherwise the QImage will try to clean it up a second time on window close and crash
    xcb_image_destroy(ximg);

    //Cleanup the XCB data structures
    xcb_free_pixmap(QX11Info::connection(), pix);

    return img;
  }*/
}
void NativeEmbedWidget::setWinUnpaused(){
  paused = false;
  winImage = QImage();
  if(!DISABLE_COMPOSITING){
    repaintWindow(); //update the cached image right away
  }else if(this->isVisible()){
    showWindow();
  }
  resyncWindow(); //make sure the window knows about the new location
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
    connect(WIN, SIGNAL(VisualChanged()), this, SLOT(repaintWindow()) ); //make sure we repaint the widget on visual change
  }else{
    xcb_reparent_window(QX11Info::connection(), WIN->id(), this->winId(), 0, 0);
    registerClientEvents(this->winId()); //child events get forwarded through the frame - watch this for changes too
    //Also use a partial-composite here - make sure the window pixmap is available even when the window is obscured
    xcb_composite_redirect_window(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    //xcb_composite_redirect_subwindows(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_MANUAL);
    //Also alert us when the window visual changes
    Damage dmgID = XDamageCreate(QX11Info::display(), WIN->id(), XDamageReportRawRectangles);

    WIN->addDamageID( (uint) dmgID); //save this for later
    connect(WIN, SIGNAL(VisualChanged()), this, SLOT(repaintWindow()) ); //make sure we repaint the widget on visual change
  }
  WIN->addFrameWinID(this->winId());
  registerClientEvents(WIN->id());
  //qDebug() << "Events Registered:" << WIN->id() << this->winId();
  return true;
}

bool NativeEmbedWidget::detachWindow(){
  xcb_reparent_window(QX11Info::connection(), WIN->id(), QX11Info::appRootWindow(), -1, -1);
  //WIN = 0;
  return true;
}

bool NativeEmbedWidget::isEmbedded(){
  return (WIN!=0);
}

void NativeEmbedWidget::raiseWindow(){
 if(DISABLE_COMPOSITING){ return; }
  uint32_t val = XCB_STACK_MODE_ABOVE;
  xcb_configure_window(QX11Info::connection(),  WIN->id(), XCB_CONFIG_WINDOW_STACK_MODE, &val);
}

void NativeEmbedWidget::lowerWindow(){
  if(DISABLE_COMPOSITING){ return; }
  uint32_t val = XCB_STACK_MODE_BELOW;
  xcb_configure_window(QX11Info::connection(),  WIN->id(), XCB_CONFIG_WINDOW_STACK_MODE, &val);
}

// ==============
//   PUBLIC SLOTS
// ==============
//Pause/resume
void NativeEmbedWidget::pause(){
  if(DISABLE_COMPOSITING){
    winImage = windowImage(QRect(QPoint(0,0), this->size()));
    hideWindow();
  }else{
    if(winImage.isNull()){ repaintWindow(); } //make sure we have one image already cached first
  }
  paused = true;
}

void NativeEmbedWidget::resume(){
  //paused = false;
  syncWinSize();
  if(DISABLE_COMPOSITING){
    //showWindow();
  }else{
    repaintWindow(); //update the cached image right away
  }
  QTimer::singleShot(10, this, SLOT(setWinUnpaused()) );
}

void NativeEmbedWidget::resyncWindow(){
   if(WIN==0){ return; }
  //syncWinSize();
  //if(DISABLE_COMPOSITING){
    // Specs say to send an artificial configure event to the window if the window was reparented into the frame
    QPoint loc = this->mapToGlobal( QPoint(0,0) );
    //Send an artificial configureNotify event to the window with the global position/size included
    xcb_configure_notify_event_t *event = (xcb_configure_notify_event_t*) calloc(32,1); //always 32-byes long, even if we don't need all of it
      event->x = loc.x();
      event->y = loc.y();
      event->width = this->width();
      event->height = this->height();
      event->border_width = 0;
      event->above_sibling = XCB_NONE;
      event->override_redirect = false;
      event->window = WIN->id();
      event->event = WIN->id();
      event->response_type = XCB_CONFIGURE_NOTIFY;
    xcb_send_event(QX11Info::connection(), false, WIN->id(), XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (char *) event);
    xcb_flush(QX11Info::connection());
    free(event);
  /*}else{
    //Window is floating invisibly - make sure it is in the right place
    //Make sure the window size is syncronized and visual up to date
    //syncWinSize();
    QTimer::singleShot(10, this, SLOT(repaintWindow()) );
  }*/

}

void NativeEmbedWidget::repaintWindow(){
  //if(DISABLE_COMPOSITING){ return; }
  //qDebug() << "Update Window Image:" << !paused;
  if(paused){ return; }
    /*QImage tmp = windowImage( QRect(QPoint(0,0), this->size()) );
    if(!tmp.isNull()){
      winImage = tmp;
    }else{ qDebug() << "Got Null Image!!"; }*/
    this->parentWidget()->update(); //visual changed - need to update the image on the widget
}

void NativeEmbedWidget::reregisterEvents(){
  if(WIN!=0){ registerClientEvents(WIN->id()); }
}

// ==============
//      PROTECTED
// ==============
void NativeEmbedWidget::resizeEvent(QResizeEvent *ev){
  QWidget::resizeEvent(ev);
  if(WIN!=0 && !paused){
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
    QPainter P(this);
      P.setClipping(true);
      P.setClipRect(0,0,this->width(), this->height());
      P.fillRect(ev->rect(), Qt::transparent);
  if(WIN==0){ return; }
  QRect geom = ev->rect(); //atomic updates
  //qDebug() << "Paint Rect:" << geom;
  QImage img;
  if(!paused){ img = windowImage(geom); }
  else if(!winImage.isNull()){
    if(winImage.size() == this->size()){ img = winImage.copy(geom); }
    else{ img = winImage.scaled(geom.size()); } //this is a fast transformation - might be slightly distorted
  }
  //Need to paint the image from the window onto the widget as an overlay
  P.drawImage( geom , img,  QRect(QPoint(0,0), img.size()), Qt::NoOpaqueDetection); //1-to-1 mapping


}

void NativeEmbedWidget::enterEvent(QEvent *ev){
  QWidget::enterEvent(ev);
  //qDebug() << "Enter Embed Widget";
  //raiseWindow(); //this->grabMouse();
}

void NativeEmbedWidget::leaveEvent(QEvent *ev){
  QWidget::leaveEvent(ev);
  /*qDebug() << "Leave Embed Widget";
  QPoint pt = QCursor::pos();
  QPoint relpt = this->parentWidget()->mapFromGlobal(pt);
  qDebug() << " - Geom:" << this->geometry() << "Global pt:" << pt << "Relative pt:" << relpt;
  if(!this->geometry().contains(relpt) ){ lowerWindow(); }*/
}

void NativeEmbedWidget::mouseMoveEvent(QMouseEvent *ev){
  QWidget::mouseMoveEvent(ev);
  //Forward this event on to the window
}

void NativeEmbedWidget::mousePressEvent(QMouseEvent *ev){
  QWidget::mousePressEvent(ev);
  //Forward this event on to the window
}

void NativeEmbedWidget::mouseReleaseEvent(QMouseEvent *ev){
  QWidget::mouseReleaseEvent(ev);
  //Forward this event on to the window
}

/*bool NativeEmbedWidget::nativeEvent(const QByteArray &eventType, void *message, long *result){
  if(eventType=="xcb_generic_event_t" && WIN!=0){
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
  }
  return false;
}*/
