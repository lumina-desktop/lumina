
#include "NativeWindowSystem.h"

#include <QKeySequence>
#include <QX11Info>

// XCB/X11 Includes
#define XK_MISCELLANY
#define XK_XKB_KEYS
#define XK_LATIN1
#define XK_LATIN2
#define XK_LATIN3
#define XK_LATIN4
#define XK_LATIN8
#define XK_LATIN9
//NOTE: Look at the keysymdef.h file for additional define/characters which we may need later
#include <X11/keysymdef.h>
#include <xcb/xcb_keysyms.h>


//Small simplification functions
Qt::Key NativeWindowSystem::KeycodeToQt(int keycode){
  static xcb_key_symbols_t *SYM = 0;
  if(SYM==0){ SYM = xcb_key_symbols_alloc(QX11Info::connection()); }
  xcb_keysym_t symbol = xcb_key_symbols_get_keysym(SYM, keycode,0);
  //not sure about the "column" input - we want raw keys though so ignore the "modified" key states (columns) for now
  //qDebug() << "Try to convert keycode to Qt::Key:" << keycode << symbol;
  //Now map this symbol to the appropriate Qt::Key enumeration
  switch(symbol){
	//FUNCTION KEYS
	case XK_F1: return Qt::Key_F1;
	case XK_F2: return Qt::Key_F2;
	case XK_F3: return Qt::Key_F3;
	case XK_F4: return Qt::Key_F4;
	case XK_F5: return Qt::Key_F5;
	case XK_F6: return Qt::Key_F6;
	case XK_F7: return Qt::Key_F7;
	case XK_F8: return Qt::Key_F8;
	case XK_F9: return Qt::Key_F9;
	case XK_F10: return Qt::Key_F10;
	case XK_F11: return Qt::Key_F11;
	case XK_F12: return Qt::Key_F12;
	case XK_F13: return Qt::Key_F13;
	case XK_F14: return Qt::Key_F14;
	case XK_F15: return Qt::Key_F15;
	case XK_F16: return Qt::Key_F16;
	case XK_F17: return Qt::Key_F17;
	case XK_F18: return Qt::Key_F18;
	case XK_F19: return Qt::Key_F19;
	case XK_F20: return Qt::Key_F20;
	case XK_F21: return Qt::Key_F21;
	case XK_F22: return Qt::Key_F22;
	case XK_F23: return Qt::Key_F23;
	case XK_F24: return Qt::Key_F24;
	case XK_F25: return Qt::Key_F25;
	case XK_F26: return Qt::Key_F26;
	case XK_F27: return Qt::Key_F27;
	case XK_F28: return Qt::Key_F28;
	case XK_F29: return Qt::Key_F29;
	case XK_F30: return Qt::Key_F30;
	case XK_F31: return Qt::Key_F31;
	case XK_F32: return Qt::Key_F32;
	case XK_F33: return Qt::Key_F33;
	case XK_F34: return Qt::Key_F34;
	case XK_F35: return Qt::Key_F35;
	//Miscellaneous Keys
	case XK_BackSpace: return Qt::Key_Backspace;
	case XK_Delete: return Qt::Key_Delete;
	//case XK_LineFeed: return Qt::Key_Backspace;
	case XK_Clear: return Qt::Key_Clear;
	case XK_Return: return Qt::Key_Return;
	case XK_Pause: return Qt::Key_Pause;
	case XK_Scroll_Lock: return Qt::Key_ScrollLock;
	case XK_Sys_Req: return Qt::Key_SysReq;
	case XK_Escape: return Qt::Key_Escape;
	case XK_Select: return Qt::Key_Select;
	case XK_Print: return Qt::Key_Print;
	//case XK_Execute: return Qt::Key_Execute;
	case XK_Insert: return Qt::Key_Insert;
	case XK_Undo: return Qt::Key_Undo;
	case XK_Redo: return Qt::Key_Redo;
	case XK_Menu: return Qt::Key_Menu;
	case XK_Find: return Qt::Key_Find;
	case XK_Cancel: return Qt::Key_Cancel;
	case XK_Help: return Qt::Key_Help;
	//case XK_Break: return Qt::Key_Break;
	//case XK_Mode_switch: return Qt::Key_Backspace;
	//case XK_script_switch: return Qt::Key_Backspace;
	case XK_Num_Lock: return Qt::Key_NumLock;
	//Cursor  Controls
	case XK_Home: return Qt::Key_Home;
	case XK_Left: return Qt::Key_Left;
	case XK_Up: return Qt::Key_Up;
	case XK_Right: return Qt::Key_Right;
	case XK_Down: return Qt::Key_Down;
	//case XK_Prior: return Qt::Key_Backspace;
	case XK_Page_Up: return Qt::Key_PageUp;
	case XK_Page_Down: return Qt::Key_PageDown;
	//case XK_Next: return Qt::Key_Backspace;
	case XK_End: return Qt::Key_End;
	//case XK_Begin: return Qt::Key_Backspace;
	// Keypad Functions and numbers
	case XK_KP_Space: return Qt::Key_Space;
	case XK_KP_Tab: return Qt::Key_Tab;
	case XK_KP_Enter: return Qt::Key_Enter;
	case XK_KP_F1: return Qt::Key_F1;
	case XK_KP_F2: return Qt::Key_F2;
	case XK_KP_F3: return Qt::Key_F3;
	case XK_KP_F4: return Qt::Key_F4;
	case XK_KP_Home: return Qt::Key_Home;
	case XK_KP_Left: return Qt::Key_Left;
	case XK_KP_Up: return Qt::Key_Up;
	case XK_KP_Right: return Qt::Key_Right;
	case XK_KP_Down: return Qt::Key_Down;
	//case XK_KP_Prior: return Qt::Key_
	case XK_KP_Page_Up: return Qt::Key_PageUp;
	//case XK_KP_Next: return Qt::Key_
	case XK_KP_Page_Down: return Qt::Key_PageDown;
	case XK_KP_End: return Qt::Key_End;
	//case XK_KP_Begin: return Qt::Key_
	case XK_KP_Insert: return Qt::Key_Insert;
	case XK_KP_Delete: return Qt::Key_Delete;
	case XK_KP_Equal: return Qt::Key_Equal;
	case XK_KP_Multiply: return Qt::Key_Asterisk;
	case XK_KP_Add: return Qt::Key_Plus;
	case XK_KP_Separator: return Qt::Key_Comma;  //X11 definitions say this is often comma
	case XK_KP_Subtract: return Qt::Key_Minus;
	case XK_KP_Decimal: return Qt::Key_Period;
	case XK_KP_Divide: return Qt::Key_Slash;
	case XK_KP_0: return Qt::Key_0;
	case XK_KP_1: return Qt::Key_1;
	case XK_KP_2: return Qt::Key_2;
	case XK_KP_3: return Qt::Key_3;
	case XK_KP_4: return Qt::Key_4;
	case XK_KP_5: return Qt::Key_5;
	case XK_KP_6: return Qt::Key_6;
	case XK_KP_7: return Qt::Key_7;
	case XK_KP_8: return Qt::Key_8;
	case XK_KP_9: return Qt::Key_9;
	// Modifier Keys
	case XK_Shift_L: return Qt::Key_Shift;
	case XK_Shift_R: return Qt::Key_Shift;
	case XK_Control_L: return Qt::Key_Control;
	case XK_Control_R: return Qt::Key_Control;
	case XK_Caps_Lock: return Qt::Key_CapsLock;
	//case XK_Shift_Lock: return Qt::Key_ShiftLock;
	case XK_Meta_L: return Qt::Key_Meta;
	case XK_Meta_R: return Qt::Key_Meta;
	case XK_Alt_L: return Qt::Key_Alt;
	case XK_Alt_R: return Qt::Key_Alt;
	case XK_Super_L: return Qt::Key_Super_L;
	case XK_Super_R: return Qt::Key_Super_R;
	case XK_Hyper_L: return Qt::Key_Hyper_L;
	case XK_Hyper_R: return Qt::Key_Hyper_R;
	case XK_space: return Qt::Key_Space;
	case XK_exclam: return Qt::Key_Exclam;
	case XK_quotedbl: return Qt::Key_QuoteDbl;
	case XK_numbersign: return Qt::Key_NumberSign;
	case XK_dollar: return Qt::Key_Dollar;
	case XK_percent: return Qt::Key_Percent;
	case XK_ampersand: return Qt::Key_Ampersand;
	case XK_apostrophe: return Qt::Key_Apostrophe;
	case XK_parenleft: return Qt::Key_ParenLeft;
	case XK_parenright: return Qt::Key_ParenRight;
	case XK_asterisk: return Qt::Key_Asterisk;
	case XK_plus: return Qt::Key_Plus;
	case XK_comma: return Qt::Key_Comma;
	case XK_minus: return Qt::Key_Minus;
	case XK_period: return Qt::Key_Period;
	case XK_slash: return Qt::Key_Slash;
	case XK_0: return Qt::Key_0;
	case XK_1: return Qt::Key_1;
	case XK_2: return Qt::Key_2;
	case XK_3: return Qt::Key_3;
	case XK_4: return Qt::Key_4;
	case XK_5: return Qt::Key_5;
	case XK_6: return Qt::Key_6;
	case XK_7: return Qt::Key_7;
	case XK_8: return Qt::Key_8;
	case XK_9: return Qt::Key_9;
	case XK_colon: return Qt::Key_Colon;
	case XK_semicolon: return Qt::Key_Semicolon;
	case XK_less: return Qt::Key_Less;
	case XK_equal: return Qt::Key_Equal;
	case XK_greater: return Qt::Key_Greater;
	case XK_question: return Qt::Key_Question;
	case XK_at: return Qt::Key_At;
	case XK_A: return Qt::Key_A;
	case XK_B: return Qt::Key_B;
	case XK_C: return Qt::Key_C;
	case XK_D: return Qt::Key_D;
	case XK_E: return Qt::Key_E;
	case XK_F: return Qt::Key_F;
	case XK_G: return Qt::Key_G;
	case XK_H: return Qt::Key_H;
	case XK_I: return Qt::Key_I;
	case XK_J: return Qt::Key_J;
	case XK_K: return Qt::Key_K;
	case XK_L: return Qt::Key_L;
	case XK_M: return Qt::Key_M;
	case XK_N: return Qt::Key_N;
	case XK_O: return Qt::Key_O;
	case XK_P: return Qt::Key_P;
	case XK_Q: return Qt::Key_Q;
	case XK_R: return Qt::Key_R;
	case XK_S: return Qt::Key_S;
	case XK_T: return Qt::Key_T;
	case XK_U: return Qt::Key_U;
	case XK_V: return Qt::Key_V;
	case XK_W: return Qt::Key_W;
	case XK_X: return Qt::Key_X;
	case XK_Y : return Qt::Key_Y;
	case XK_Z: return Qt::Key_Z;
	case XK_bracketleft: return Qt::Key_BracketLeft;
	case XK_backslash: return Qt::Key_Backslash;
	case XK_bracketright: return Qt::Key_BracketRight;
	case XK_asciicircum: return Qt::Key_AsciiCircum;
	case XK_underscore: return Qt::Key_Underscore;
	case XK_grave: return Qt::Key_Agrave;
	case XK_a: return Qt::Key_A;
	case XK_b: return Qt::Key_B;
	case XK_c: return Qt::Key_C;
	case XK_d: return Qt::Key_D;
	case XK_e: return Qt::Key_E;
	case XK_f : return Qt::Key_F;
	case XK_g: return Qt::Key_G;
	case XK_h: return Qt::Key_H;
	case XK_i: return Qt::Key_I;
	case XK_j: return Qt::Key_J;
	case XK_k: return Qt::Key_K;
	case XK_l: return Qt::Key_L;
	case XK_m: return Qt::Key_M;
	case XK_n: return Qt::Key_N;
	case XK_o: return Qt::Key_O;
	case XK_p: return Qt::Key_P;
	case XK_q: return Qt::Key_Q;
	case XK_r: return Qt::Key_R;
	case XK_s: return Qt::Key_S;
	case XK_t : return Qt::Key_T;
	case XK_u: return Qt::Key_U;
	case XK_v: return Qt::Key_V;
	case XK_w: return Qt::Key_W;
	case XK_x: return Qt::Key_X;
	case XK_y: return Qt::Key_Y;
	case XK_z: return Qt::Key_Z;
        case  XK_braceleft: return Qt::Key_BraceLeft;
        case  XK_bar: return Qt::Key_Bar;
        case  XK_braceright: return Qt::Key_BraceRight;
        case  XK_asciitilde: return Qt::Key_AsciiTilde;

        case XK_nobreakspace: return Qt::Key_nobreakspace;
        case XK_exclamdown: return Qt::Key_exclamdown;
        case XK_cent: return Qt::Key_cent;
        case XK_sterling: return Qt::Key_sterling;
        case XK_currency: return Qt::Key_currency;
        case XK_yen: return Qt::Key_yen;
        case XK_brokenbar: return Qt::Key_brokenbar;
        case XK_section: return Qt::Key_section;
        case XK_diaeresis: return Qt::Key_diaeresis;
        case XK_copyright: return Qt::Key_copyright;
        case XK_ordfeminine: return Qt::Key_ordfeminine;
        case XK_guillemotleft: return Qt::Key_guillemotleft;
        case XK_notsign: return Qt::Key_notsign;
        case XK_hyphen: return Qt::Key_hyphen;
        case XK_registered: return Qt::Key_registered;
        case XK_macron: return Qt::Key_macron;
        case XK_degree: return Qt::Key_degree;
        case XK_plusminus: return Qt::Key_plusminus;
        case XK_twosuperior: return Qt::Key_twosuperior;
        case XK_threesuperior: return Qt::Key_threesuperior;
        case XK_acute: return Qt::Key_acute;
        case XK_mu: return Qt::Key_mu;
        case XK_paragraph: return Qt::Key_paragraph;
        case XK_periodcentered: return Qt::Key_periodcentered;
        case XK_cedilla: return Qt::Key_cedilla;
        case XK_onesuperior: return Qt::Key_onesuperior;
        case XK_masculine: return Qt::Key_masculine;
        case XK_guillemotright: return Qt::Key_guillemotright;
        case XK_onequarter: return Qt::Key_onequarter;
        case XK_onehalf: return Qt::Key_onehalf;
        case XK_threequarters: return Qt::Key_threequarters;
        case XK_questiondown: return Qt::Key_questiondown;
        case XK_Agrave: return Qt::Key_Agrave;
        case XK_Aacute: return Qt::Key_Aacute;
        case XK_Acircumflex: return Qt::Key_Acircumflex;
        case XK_Atilde: return Qt::Key_Atilde;
        case XK_Adiaeresis: return Qt::Key_Adiaeresis;
        case XK_Aring: return Qt::Key_Aring;
        case XK_AE: return Qt::Key_AE;
        case XK_Ccedilla: return Qt::Key_Ccedilla;
        case XK_Egrave: return Qt::Key_Egrave;
        case XK_Eacute: return Qt::Key_Eacute;
        case XK_Ecircumflex: return Qt::Key_Ecircumflex;
        case XK_Ediaeresis: return Qt::Key_Ediaeresis;
        case XK_Igrave: return Qt::Key_Igrave;
        case XK_Iacute: return Qt::Key_Iacute;
        case XK_Icircumflex: return Qt::Key_Icircumflex;
        case XK_Idiaeresis: return Qt::Key_Idiaeresis;
        case XK_ETH: return Qt::Key_ETH;
        //case XK_Eth: return Qt::Key_Eth;
        case XK_Ntilde: return Qt::Key_Ntilde;
        case XK_Ograve: return Qt::Key_Ograve;
        case XK_Oacute: return Qt::Key_Oacute;
        case XK_Ocircumflex: return Qt::Key_Ocircumflex;
        case XK_Otilde: return Qt::Key_Otilde;
        case XK_Odiaeresis: return Qt::Key_Odiaeresis;
        case XK_multiply: return Qt::Key_multiply;
        //case XK_Oslash: return Qt::Key_AsciiTilde;
        case XK_Ooblique: return Qt::Key_Ooblique;
        case XK_Ugrave: return Qt::Key_Ugrave;
        case XK_Uacute: return Qt::Key_Uacute;
        case XK_Ucircumflex: return Qt::Key_Ucircumflex;
        case XK_Udiaeresis: return Qt::Key_Udiaeresis;
        case XK_Yacute: return Qt::Key_Yacute;
        case XK_THORN: return Qt::Key_THORN;
        //case XK_Thorn: return Qt::Key_AsciiTilde;
        case XK_ssharp: return Qt::Key_ssharp;
        /*case XK_agrave: return Qt::Key_AsciiTilde;
        case XK_aacute: return Qt::Key_AsciiTilde;
        case XK_acircumflex: return Qt::Key_AsciiTilde;
        case XK_atilde: return Qt::Key_AsciiTilde;
        case XK_adiaeresis: return Qt::Key_AsciiTilde;
        case XK_aring: return Qt::Key_AsciiTilde;
        case XK_ae: return Qt::Key_AsciiTilde;
        case XK_ccedilla: return Qt::Key_AsciiTilde;
        case XK_egrave: return Qt::Key_AsciiTilde;
        case XK_eacute: return Qt::Key_AsciiTilde;
        case XK_ecircumflex: return Qt::Key_AsciiTilde;
        case XK_ediaeresis: return Qt::Key_AsciiTilde;
        case XK_igrave: return Qt::Key_AsciiTilde;
        case XK_iacute: return Qt::Key_AsciiTilde;
        case XK_icircumflex: return Qt::Key_AsciiTilde;
        case XK_idiaeresis: return Qt::Key_AsciiTilde;
        case XK_eth: return Qt::Key_AsciiTilde;
        case XK_ntilde: return Qt::Key_AsciiTilde;
        case XK_ograve: return Qt::Key_AsciiTilde;
        case XK_oacute: return Qt::Key_AsciiTilde;
        case XK_ocircumflex: return Qt::Key_AsciiTilde;
        case XK_otilde: return Qt::Key_AsciiTilde;
        case XK_odiaeresis: return Qt::Key_AsciiTilde;
        case XK_division: return Qt::Key_AsciiTilde;
        case XK_oslash: return Qt::Key_AsciiTilde;
        case XK_ooblique: return Qt::Key_AsciiTilde;
        case XK_ugrave: return Qt::Key_AsciiTilde;
        case XK_uacute: return Qt::Key_AsciiTilde;
        case XK_ucircumflex: return Qt::Key_AsciiTilde;
        case XK_udiaeresis: return Qt::Key_AsciiTilde;
        case XK_yacute: return Qt::Key_AsciiTilde;
        case XK_thorn: return Qt::Key_AsciiTilde;
        case XK_ydiaeresis: return Qt::Key_AsciiTilde;

        case: XK_Agonek: return Qt::Key_AsciiTilde;
        case XK_breve: return Qt::Key_AsciiTilde;
        case XK_Lstroke: return Qt::Key_AsciiTilde;
        case XK_Lcaron: return Qt::Key_AsciiTilde;
        case XK_Sacute: return Qt::Key_AsciiTilde;
        case XK_Scaron: return Qt::Key_AsciiTilde;
        case XK_Scedilla: return Qt::Key_AsciiTilde;
        case XK_Tcaron: return Qt::Key_AsciiTilde;
        case XK_Zacute: return Qt::Key_AsciiTilde;
        case XK_Zcaron: return Qt::Key_AsciiTilde;
        case XK_Zabovedot: return Qt::Key_AsciiTilde;
        case XK_aogonek: return Qt::Key_AsciiTilde;
        case XK_ogonek: return Qt::Key_AsciiTilde;
        case XK_lstroke: return Qt::Key_AsciiTilde;
        case XK_lcaron: return Qt::Key_AsciiTilde;
        case XK_sacute: return Qt::Key_AsciiTilde;
        case XK_caron: return Qt::Key_AsciiTilde;
        case XK_scaron: return Qt::Key_AsciiTilde;
        case XK_scedilla: return Qt::Key_AsciiTilde;
        case XK_tcaron: return Qt::Key_AsciiTilde;
        case XK_zacute: return Qt::Key_AsciiTilde;
        case XK_doubleacute: return Qt::Key_AsciiTilde;
        case XK_zcaron: return Qt::Key_AsciiTilde;
        case XK_zabovedot: return Qt::Key_AsciiTilde;
        case XK_Racute: return Qt::Key_AsciiTilde;
        case XK_Abreve: return Qt::Key_AsciiTilde;
        case XK_Lacute: return Qt::Key_AsciiTilde;
        case XK_Cacute: return Qt::Key_AsciiTilde;
        case XK_Ccaron: return Qt::Key_AsciiTilde;
        case XK_Eogonek: return Qt::Key_AsciiTilde;
        case XK_Ecaron: return Qt::Key_AsciiTilde;
        case XK_Dcaron: return Qt::Key_AsciiTilde;
        case XK_Dstroke: return Qt::Key_AsciiTilde;
        case XK_Nacute: return Qt::Key_AsciiTilde;
        case XK_Ncaron: return Qt::Key_AsciiTilde;
        case XK_Odoubleacute: return Qt::Key_AsciiTilde;
        case XK_Rcaron: return Qt::Key_AsciiTilde;
        case XK_Uring: return Qt::Key_AsciiTilde;
        case XK_Udoubleacute: return Qt::Key_AsciiTilde;
        case XK_Tcedilla: return Qt::Key_AsciiTilde;
        case XK_racute: return Qt::Key_AsciiTilde;
        case XK_abreve: return Qt::Key_AsciiTilde;
        case XK_lacute: return Qt::Key_AsciiTilde;
        case XK_cacute: return Qt::Key_AsciiTilde;
        case XK_ccaron: return Qt::Key_AsciiTilde;
        case XK_eogonek: return Qt::Key_AsciiTilde;
        case XK_ecaron: return Qt::Key_AsciiTilde;
        case XK_dcaron: return Qt::Key_AsciiTilde;
        case XK_dstroke: return Qt::Key_AsciiTilde;
        case XK_nacute: return Qt::Key_AsciiTilde;
        case XK_ncaron: return Qt::Key_AsciiTilde;
        case XK_odoubleacute: return Qt::Key_AsciiTilde;
        case XK_rcaron: return Qt::Key_AsciiTilde;
        case XK_uring: return Qt::Key_AsciiTilde;
        case XK_udoubleacute: return Qt::Key_AsciiTilde;
        case XK_tcedilla: return Qt::Key_AsciiTilde;
        case XK_abovedot: return Qt::Key_AsciiTilde;
        case XK_Hstroke: return Qt::Key_AsciiTilde;
        case XK_Hcircumflex: return Qt::Key_AsciiTilde;
        case XK_Iabovedot: return Qt::Key_AsciiTilde;
        case XK_Gbreve: return Qt::Key_AsciiTilde;
        case XK_Jcircumflex: return Qt::Key_AsciiTilde;
        case XK_hstroke: return Qt::Key_AsciiTilde;
        case XK_hcircumflex: return Qt::Key_AsciiTilde;
        case XK_idotless: return Qt::Key_AsciiTilde;
        case XK_gbreve: return Qt::Key_AsciiTilde;
        case XK_jcircumflex: return Qt::Key_AsciiTilde;
        case XK_Cabovedot: return Qt::Key_AsciiTilde;
        case XK_Ccircumflex: return Qt::Key_AsciiTilde;
        case XK_Gabovedot: return Qt::Key_AsciiTilde;
        case XK_Gcircumflex: return Qt::Key_AsciiTilde;
        case XK_Ubreve: return Qt::Key_AsciiTilde;
        case XK_Scircumflex: return Qt::Key_AsciiTilde;
        case XK_cabovedot: return Qt::Key_AsciiTilde;
        case XK_ccircumflex: return Qt::Key_AsciiTilde;
        case XK_gabovedot: return Qt::Key_AsciiTilde;
        case XK_gcircumflex: return Qt::Key_AsciiTilde;
        case XK_ubreve: return Qt::Key_AsciiTilde;
        case XK_scircumflex: return Qt::Key_AsciiTilde;
        case XK_kra: return Qt::Key_AsciiTilde;
        case XK_kappa: return Qt::Key_AsciiTilde;
        case XK_Rcedilla: return Qt::Key_AsciiTilde;
        case XK_Itilde: return Qt::Key_AsciiTilde;
        case XK_Lcedilla: return Qt::Key_AsciiTilde;
        case XK_Emacron: return Qt::Key_AsciiTilde;
        case XK_Gcedilla: return Qt::Key_AsciiTilde;
        case XK_Tslash: return Qt::Key_AsciiTilde;
        case XK_rcedilla: return Qt::Key_AsciiTilde;
        case XK_itilde: return Qt::Key_AsciiTilde;
        case XK_lcedilla: return Qt::Key_AsciiTilde;
        case XK_emacron: return Qt::Key_AsciiTilde;
        case XK_gcedilla: return Qt::Key_AsciiTilde;
        case XK_tslash: return Qt::Key_AsciiTilde;
        case XK_ENG: return Qt::Key_AsciiTilde;
        case XK_eng: return Qt::Key_AsciiTilde;
        case XK_Amacron: return Qt::Key_AsciiTilde;
        case XK_Iogonek: return Qt::Key_AsciiTilde;
        case XK_Eabovedot: return Qt::Key_AsciiTilde;
        case XK_Imacron: return Qt::Key_AsciiTilde;
        case XK_Ncedilla: return Qt::Key_AsciiTilde;
        case XK_Omacron: return Qt::Key_AsciiTilde;
        case XK_Kcedilla: return Qt::Key_AsciiTilde;
        case XK_Uogonek: return Qt::Key_AsciiTilde;
        case XK_Utilde: return Qt::Key_AsciiTilde;
        case XK_Umacron: return Qt::Key_AsciiTilde;
        case XK_amacron: return Qt::Key_AsciiTilde;
        case XK_iogonek: return Qt::Key_AsciiTilde;
        case XK_eabovedot: return Qt::Key_AsciiTilde;
        case XK_imacron: return Qt::Key_AsciiTilde;
        case XK_ncedilla: return Qt::Key_AsciiTilde;
        case XK_omacron: return Qt::Key_AsciiTilde;
        case XK_kcedilla: return Qt::Key_AsciiTilde;
        case XK_uogonek: return Qt::Key_AsciiTilde;
        case XK_utilde: return Qt::Key_AsciiTilde;
        case XK_umacron: return Qt::Key_AsciiTilde;
        case XK_Wcircumflex: return Qt::Key_AsciiTilde;
        case XK_wcircumflex: return Qt::Key_AsciiTilde;
        case XK_Ycircumflex: return Qt::Key_AsciiTilde;
        case XK_ycircumflex: return Qt::Key_AsciiTilde;
        case XK_Babovedot: return Qt::Key_AsciiTilde;
        case XK_babovedot: return Qt::Key_AsciiTilde;
        case XK_Dabovedot: return Qt::Key_AsciiTilde;
        case XK_dabovedot: return Qt::Key_AsciiTilde;
        case XK_Fabovedot: return Qt::Key_AsciiTilde;
        case XK_fabovedot: return Qt::Key_AsciiTilde;
        case XK_Mabovedot: return Qt::Key_AsciiTilde;
        case XK_mabovedot: return Qt::Key_AsciiTilde;
        case XK_Pabovedot: return Qt::Key_AsciiTilde;
        case XK_pabovedot: return Qt::Key_AsciiTilde;
        case XK_Sabovedot: return Qt::Key_AsciiTilde;
        case XK_sabovedot: return Qt::Key_AsciiTilde;
        case XK_Tabovedot: return Qt::Key_AsciiTilde;
        case XK_tabovedot: return Qt::Key_AsciiTilde;
        case XK_Wgrave: return Qt::Key_AsciiTilde;
        case XK_wgrave: return Qt::Key_AsciiTilde;
        case XK_Wacute: return Qt::Key_AsciiTilde;
        case XK_wacute: return Qt::Key_AsciiTilde;
        case XK_Wdiaeresis: return Qt::Key_AsciiTilde;
        case XK_wdiaeresis: return Qt::Key_AsciiTilde;
        case XK_Ygrave: return Qt::Key_AsciiTilde;
        case XK_ygrave: return Qt::Key_AsciiTilde;
        case XK_OE: return Qt::Key_AsciiTilde;
        case XK_oe: return Qt::Key_AsciiTilde;
        case XK_Ydiaeresis: return Qt::Key_AsciiTilde;*/
	default:
	  qDebug() << "Unknown Key";
  }
  qDebug() << " -- Simple Qt Map:" << (Qt::Key)(symbol);
  qDebug() << " -- Key Sequence Map:" << QKeySequence(symbol);
  qDebug() << " - Not implemented yet";
  return Qt::Key_unknown;
}

NativeWindowSystem::MouseButton NativeWindowSystem::MouseToQt(int keycode){
  switch(keycode){
     case  1:
      return NativeWindowSystem::LeftButton;
    case  3:
      return NativeWindowSystem::RightButton;
    case  2:
      return NativeWindowSystem::MidButton;
    case  4:
      return NativeWindowSystem::WheelUp;
    case  5:
      return NativeWindowSystem::WheelDown;
    case  6:
      return NativeWindowSystem::WheelLeft;
    case  7:
      return NativeWindowSystem::WheelRight;
    case  8:
      return NativeWindowSystem::BackButton; //Not sure if this is correct yet (1/27/17)
    case  9:
      return NativeWindowSystem::ForwardButton; //Not sure if this is correct yet (1/27/17)
    default:
      return NativeWindowSystem::NoButton;
  }
}
