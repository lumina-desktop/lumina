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

//xcb_pixmap_t PIXBACK; //backend pixmap that compositing redirects to

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
  //Need the graphics context of the window
  /*xcb_gcontext_t gc = xcb_generate_id(QX11Info::connection());
    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(QX11Info::connection())).data;
   uint32_t values[1];
    values[0] = screen->black_pixel;
  xcb_create_gc(QX11Info::connection(),
	gc,
	this->winId(),
	XCB_GC_BACKGROUND,
	values );
  xcb_pixmap_t pix = xcb_generate_id(QX11Info::connection());
  xcb_composite_name_window_pixmap(QX11Info::connection(), WIN->id(), pix);
  //Now copy this pixmap onto widget
  xcb_copy_area(QX11Info::connection(), pix, this->winId(), gc, geom.x(),geom.y(),geom.x(),geom.y(),geom.width(), geom.height());
  xcb_free_pixmap(QX11Info::connection(), pix);
  return QImage();*/

  /*xcb_put_image(QX11Info::connection(), XCB_IMAGE_FORMAT_Z_PIXMAP, pix, gc, sz.width(), sz.height(), 0, 0, */
  /*xcb_image_t *img = xcb_image_get(QX11Info::connection(), WIN->id(), geom.x(), geom.y(), geom.width(), geom.height(), 1, XCB_IMAGE_FORMAT_Z_PIXMAP);
  if(img==0){ return QImage(); }
  QImage image(geom.size(), QImage::Format_ARGB32);
   image.loadFromData(img->data, img->size);
  return image;*/

  return QImage();
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
  //this->SelectInput(WIN->id(), true); //Notify of structure changes
  //xcb_composite_redirect_window(QX11Info::connection(), WIN->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);

  //xcb_composite_name_window_pixmap(QX11Info::connection(), WIN->id(), PIXBACK);
  //Now map the window (will be a transparent child of the container)
  //xcb_map_window(QX11Info::connection(), WIN->id());
  //xcb_map_window(QX11Info::connection(), this->winId());
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
  QImage img = windowImage(ev->rect());
  if(!img.isNull()){
    QPainter P(this);
    P.drawImage( ev->rect() , img, ev->rect(), Qt::NoOpaqueDetection); //1-to-1 mapping
  //Note: Qt::NoOpaqueDetection Speeds up the paint by bypassing the checks to see if there are [semi-]transparent pixels
  //  Since this is an embedded image - we fully expect there to be transparency most of the time.
  }else{
    QWidget::paintEvent(ev);
  }
}
