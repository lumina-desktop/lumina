
#include <NativeWindowSystem.h>

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

        //case XK_nobreakspace                  0x00a0  /* U+00A0 NO-BREAK SPACE */
        //case XK_exclamdown                    0x00a1  /* U+00A1 INVERTED EXCLAMATION MARK */
        //case XK_cent                          0x00a2  /* U+00A2 CENT SIGN */
        //case XK_sterling                      0x00a3  /* U+00A3 POUND SIGN */
        //case XK_currency                      0x00a4  /* U+00A4 CURRENCY SIGN */
        //case XK_yen                           0x00a5  /* U+00A5 YEN SIGN */
        //case XK_brokenbar                     0x00a6  /* U+00A6 BROKEN BAR */
        //case XK_section                       0x00a7  /* U+00A7 SECTION SIGN */
        //case XK_diaeresis                     0x00a8  /* U+00A8 DIAERESIS */
        //case XK_copyright                     0x00a9  /* U+00A9 COPYRIGHT SIGN */
        //case XK_ordfeminine                   0x00aa  /* U+00AA FEMININE ORDINAL INDICATOR */
        //case XK_guillemotleft                 0x00ab  /* U+00AB LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
        //case XK_notsign                       0x00ac  /* U+00AC NOT SIGN */
        //case XK_hyphen                        0x00ad  /* U+00AD SOFT HYPHEN */
        //case XK_registered                    0x00ae  /* U+00AE REGISTERED SIGN */
        //case XK_macron                        0x00af  /* U+00AF MACRON */
        //case XK_degree                        0x00b0  /* U+00B0 DEGREE SIGN */
        //case XK_plusminus                     0x00b1  /* U+00B1 PLUS-MINUS SIGN */
        //case XK_twosuperior                   0x00b2  /* U+00B2 SUPERSCRIPT TWO */
        //case XK_threesuperior                 0x00b3  /* U+00B3 SUPERSCRIPT THREE */
        //case XK_acute                         0x00b4  /* U+00B4 ACUTE ACCENT */
        //case XK_mu                            0x00b5  /* U+00B5 MICRO SIGN */
        //case XK_paragraph                     0x00b6  /* U+00B6 PILCROW SIGN */
        //case XK_periodcentered                0x00b7  /* U+00B7 MIDDLE DOT */
        //case XK_cedilla                       0x00b8  /* U+00B8 CEDILLA */
        //case XK_onesuperior                   0x00b9  /* U+00B9 SUPERSCRIPT ONE */
        //case XK_masculine                     0x00ba  /* U+00BA MASCULINE ORDINAL INDICATOR */
        //case XK_guillemotright                0x00bb  /* U+00BB RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
        //case XK_onequarter                    0x00bc  /* U+00BC VULGAR FRACTION ONE QUARTER */
        //case XK_onehalf                       0x00bd  /* U+00BD VULGAR FRACTION ONE HALF */
        //case XK_threequarters                 0x00be  /* U+00BE VULGAR FRACTION THREE QUARTERS */
        //case XK_questiondown                  0x00bf  /* U+00BF INVERTED QUESTION MARK */
        //case XK_Agrave                        0x00c0  /* U+00C0 LATIN CAPITAL LETTER A WITH GRAVE */
        //case XK_Aacute                        0x00c1  /* U+00C1 LATIN CAPITAL LETTER A WITH ACUTE */
        //case XK_Acircumflex                   0x00c2  /* U+00C2 LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
        //case XK_Atilde                        0x00c3  /* U+00C3 LATIN CAPITAL LETTER A WITH TILDE */
        //case XK_Adiaeresis                    0x00c4  /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS */
        //case XK_Aring                         0x00c5  /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE */
        //case XK_AE                            0x00c6  /* U+00C6 LATIN CAPITAL LETTER AE */
        //case XK_Ccedilla                      0x00c7  /* U+00C7 LATIN CAPITAL LETTER C WITH CEDILLA */
        //case XK_Egrave                        0x00c8  /* U+00C8 LATIN CAPITAL LETTER E WITH GRAVE */
        //case XK_Eacute                        0x00c9  /* U+00C9 LATIN CAPITAL LETTER E WITH ACUTE */
        //case XK_Ecircumflex                   0x00ca  /* U+00CA LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
        //case XK_Ediaeresis                    0x00cb  /* U+00CB LATIN CAPITAL LETTER E WITH DIAERESIS */
        //case XK_Igrave                        0x00cc  /* U+00CC LATIN CAPITAL LETTER I WITH GRAVE */
        //case XK_Iacute                        0x00cd  /* U+00CD LATIN CAPITAL LETTER I WITH ACUTE */
        //case XK_Icircumflex                   0x00ce  /* U+00CE LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
        //case XK_Idiaeresis                    0x00cf  /* U+00CF LATIN CAPITAL LETTER I WITH DIAERESIS */
        //case XK_ETH                           0x00d0  /* U+00D0 LATIN CAPITAL LETTER ETH */
        //case XK_Eth                           0x00d0  /* deprecated */
        //case XK_Ntilde                        0x00d1  /* U+00D1 LATIN CAPITAL LETTER N WITH TILDE */
        //case XK_Ograve                        0x00d2  /* U+00D2 LATIN CAPITAL LETTER O WITH GRAVE */
        //case XK_Oacute                        0x00d3  /* U+00D3 LATIN CAPITAL LETTER O WITH ACUTE */
        //case XK_Ocircumflex                   0x00d4  /* U+00D4 LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
        //case XK_Otilde                        0x00d5  /* U+00D5 LATIN CAPITAL LETTER O WITH TILDE */
        //case XK_Odiaeresis                    0x00d6  /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS */
        //case XK_multiply                      0x00d7  /* U+00D7 MULTIPLICATION SIGN */
        //case XK_Oslash                        0x00d8  /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        //case XK_Ooblique                      0x00d8  /* U+00D8 LATIN CAPITAL LETTER O WITH STROKE */
        //case XK_Ugrave                        0x00d9  /* U+00D9 LATIN CAPITAL LETTER U WITH GRAVE */
        //case XK_Uacute                        0x00da  /* U+00DA LATIN CAPITAL LETTER U WITH ACUTE */
        //case XK_Ucircumflex                   0x00db  /* U+00DB LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
        //case XK_Udiaeresis                    0x00dc  /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS */
        //case XK_Yacute                        0x00dd  /* U+00DD LATIN CAPITAL LETTER Y WITH ACUTE */
        //case XK_THORN                         0x00de  /* U+00DE LATIN CAPITAL LETTER THORN */
        //case XK_Thorn                         0x00de  /* deprecated */
        //case XK_ssharp                        0x00df  /* U+00DF LATIN SMALL LETTER SHARP S */
        //case XK_agrave                        0x00e0  /* U+00E0 LATIN SMALL LETTER A WITH GRAVE */
        //case XK_aacute                        0x00e1  /* U+00E1 LATIN SMALL LETTER A WITH ACUTE */
        //case XK_acircumflex                   0x00e2  /* U+00E2 LATIN SMALL LETTER A WITH CIRCUMFLEX */
        //case XK_atilde                        0x00e3  /* U+00E3 LATIN SMALL LETTER A WITH TILDE */
        //case XK_adiaeresis                    0x00e4  /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS */
        //case XK_aring                         0x00e5  /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE */
        //case XK_ae                            0x00e6  /* U+00E6 LATIN SMALL LETTER AE */
        //case XK_ccedilla                      0x00e7  /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA */
        //case XK_egrave                        0x00e8  /* U+00E8 LATIN SMALL LETTER E WITH GRAVE */
        //case XK_eacute                        0x00e9  /* U+00E9 LATIN SMALL LETTER E WITH ACUTE */
        //case XK_ecircumflex                   0x00ea  /* U+00EA LATIN SMALL LETTER E WITH CIRCUMFLEX */
        //case XK_ediaeresis                    0x00eb  /* U+00EB LATIN SMALL LETTER E WITH DIAERESIS */
        //case XK_igrave                        0x00ec  /* U+00EC LATIN SMALL LETTER I WITH GRAVE */
        //case XK_iacute                        0x00ed  /* U+00ED LATIN SMALL LETTER I WITH ACUTE */
        //case XK_icircumflex                   0x00ee  /* U+00EE LATIN SMALL LETTER I WITH CIRCUMFLEX */
        //case XK_idiaeresis                    0x00ef  /* U+00EF LATIN SMALL LETTER I WITH DIAERESIS */
        //case XK_eth                           0x00f0  /* U+00F0 LATIN SMALL LETTER ETH */
        //case XK_ntilde                        0x00f1  /* U+00F1 LATIN SMALL LETTER N WITH TILDE */
        //case XK_ograve                        0x00f2  /* U+00F2 LATIN SMALL LETTER O WITH GRAVE */
        //case XK_oacute                        0x00f3  /* U+00F3 LATIN SMALL LETTER O WITH ACUTE */
        //case XK_ocircumflex                   0x00f4  /* U+00F4 LATIN SMALL LETTER O WITH CIRCUMFLEX */
        //case XK_otilde                        0x00f5  /* U+00F5 LATIN SMALL LETTER O WITH TILDE */
        //case XK_odiaeresis                    0x00f6  /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS */
        //case XK_division                      0x00f7  /* U+00F7 DIVISION SIGN */
        //case XK_oslash                        0x00f8  /* U+00F8 LATIN SMALL LETTER O WITH STROKE */
        //case XK_ooblique                      0x00f8  /* U+00F8 LATIN SMALL LETTER O WITH STROKE */
        //case XK_ugrave                        0x00f9  /* U+00F9 LATIN SMALL LETTER U WITH GRAVE */
        //case XK_uacute                        0x00fa  /* U+00FA LATIN SMALL LETTER U WITH ACUTE */
        //case XK_ucircumflex                   0x00fb  /* U+00FB LATIN SMALL LETTER U WITH CIRCUMFLEX */
        //case XK_udiaeresis                    0x00fc  /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS */
        //case XK_yacute                        0x00fd  /* U+00FD LATIN SMALL LETTER Y WITH ACUTE */
        //case XK_thorn                         0x00fe  /* U+00FE LATIN SMALL LETTER THORN */
        //case XK_ydiaeresis                    0x00ff  /* U+00FF LATIN SMALL LETTER Y WITH DIAERESIS */

        //case XK_Aogonek                       0x01a1  /* U+0104 LATIN CAPITAL LETTER A WITH OGONEK */
        //case XK_breve                         0x01a2  /* U+02D8 BREVE */
        //case XK_Lstroke                       0x01a3  /* U+0141 LATIN CAPITAL LETTER L WITH STROKE */
        //case XK_Lcaron                        0x01a5  /* U+013D LATIN CAPITAL LETTER L WITH CARON */
        //case XK_Sacute                        0x01a6  /* U+015A LATIN CAPITAL LETTER S WITH ACUTE */
        //case XK_Scaron                        0x01a9  /* U+0160 LATIN CAPITAL LETTER S WITH CARON */
        //case XK_Scedilla                      0x01aa  /* U+015E LATIN CAPITAL LETTER S WITH CEDILLA */
        //case XK_Tcaron                        0x01ab  /* U+0164 LATIN CAPITAL LETTER T WITH CARON */
        //case XK_Zacute                        0x01ac  /* U+0179 LATIN CAPITAL LETTER Z WITH ACUTE */
        //case XK_Zcaron                        0x01ae  /* U+017D LATIN CAPITAL LETTER Z WITH CARON */
        //case XK_Zabovedot                     0x01af  /* U+017B LATIN CAPITAL LETTER Z WITH DOT ABOVE */
        //case XK_aogonek                       0x01b1  /* U+0105 LATIN SMALL LETTER A WITH OGONEK */
        //case XK_ogonek                        0x01b2  /* U+02DB OGONEK */
        //case XK_lstroke                       0x01b3  /* U+0142 LATIN SMALL LETTER L WITH STROKE */
        //case XK_lcaron                        0x01b5  /* U+013E LATIN SMALL LETTER L WITH CARON */
        //case XK_sacute                        0x01b6  /* U+015B LATIN SMALL LETTER S WITH ACUTE */
        //case XK_caron                         0x01b7  /* U+02C7 CARON */
        //case XK_scaron                        0x01b9  /* U+0161 LATIN SMALL LETTER S WITH CARON */
        //case XK_scedilla                      0x01ba  /* U+015F LATIN SMALL LETTER S WITH CEDILLA */
        //case XK_tcaron                        0x01bb  /* U+0165 LATIN SMALL LETTER T WITH CARON */
        //case XK_zacute                        0x01bc  /* U+017A LATIN SMALL LETTER Z WITH ACUTE */
        //case XK_doubleacute                   0x01bd  /* U+02DD DOUBLE ACUTE ACCENT */
        //case XK_zcaron                        0x01be  /* U+017E LATIN SMALL LETTER Z WITH CARON */
        //case XK_zabovedot                     0x01bf  /* U+017C LATIN SMALL LETTER Z WITH DOT ABOVE */
        //case XK_Racute                        0x01c0  /* U+0154 LATIN CAPITAL LETTER R WITH ACUTE */
        //case XK_Abreve                        0x01c3  /* U+0102 LATIN CAPITAL LETTER A WITH BREVE */
        //case XK_Lacute                        0x01c5  /* U+0139 LATIN CAPITAL LETTER L WITH ACUTE */
        //case XK_Cacute                        0x01c6  /* U+0106 LATIN CAPITAL LETTER C WITH ACUTE */
        //case XK_Ccaron                        0x01c8  /* U+010C LATIN CAPITAL LETTER C WITH CARON */
        //case XK_Eogonek                       0x01ca  /* U+0118 LATIN CAPITAL LETTER E WITH OGONEK */
        //case XK_Ecaron                        0x01cc  /* U+011A LATIN CAPITAL LETTER E WITH CARON */
        //case XK_Dcaron                        0x01cf  /* U+010E LATIN CAPITAL LETTER D WITH CARON */
        //case XK_Dstroke                       0x01d0  /* U+0110 LATIN CAPITAL LETTER D WITH STROKE */
        //case XK_Nacute                        0x01d1  /* U+0143 LATIN CAPITAL LETTER N WITH ACUTE */
        //case XK_Ncaron                        0x01d2  /* U+0147 LATIN CAPITAL LETTER N WITH CARON */
        //case XK_Odoubleacute                  0x01d5  /* U+0150 LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
        //case XK_Rcaron                        0x01d8  /* U+0158 LATIN CAPITAL LETTER R WITH CARON */
        //case XK_Uring                         0x01d9  /* U+016E LATIN CAPITAL LETTER U WITH RING ABOVE */
        //case XK_Udoubleacute                  0x01db  /* U+0170 LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
        //case XK_Tcedilla                      0x01de  /* U+0162 LATIN CAPITAL LETTER T WITH CEDILLA */
        //case XK_racute                        0x01e0  /* U+0155 LATIN SMALL LETTER R WITH ACUTE */
        //case XK_abreve                        0x01e3  /* U+0103 LATIN SMALL LETTER A WITH BREVE */
        //case XK_lacute                        0x01e5  /* U+013A LATIN SMALL LETTER L WITH ACUTE */
        //case XK_cacute                        0x01e6  /* U+0107 LATIN SMALL LETTER C WITH ACUTE */
        //case XK_ccaron                        0x01e8  /* U+010D LATIN SMALL LETTER C WITH CARON */
        //case XK_eogonek                       0x01ea  /* U+0119 LATIN SMALL LETTER E WITH OGONEK */
        //case XK_ecaron                        0x01ec  /* U+011B LATIN SMALL LETTER E WITH CARON */
        //case XK_dcaron                        0x01ef  /* U+010F LATIN SMALL LETTER D WITH CARON */
        //case XK_dstroke                       0x01f0  /* U+0111 LATIN SMALL LETTER D WITH STROKE */
        //case XK_nacute                        0x01f1  /* U+0144 LATIN SMALL LETTER N WITH ACUTE */
        //case XK_ncaron                        0x01f2  /* U+0148 LATIN SMALL LETTER N WITH CARON */
        //case XK_odoubleacute                  0x01f5  /* U+0151 LATIN SMALL LETTER O WITH DOUBLE ACUTE */
        //case XK_rcaron                        0x01f8  /* U+0159 LATIN SMALL LETTER R WITH CARON */
        //case XK_uring                         0x01f9  /* U+016F LATIN SMALL LETTER U WITH RING ABOVE */
        //case XK_udoubleacute                  0x01fb  /* U+0171 LATIN SMALL LETTER U WITH DOUBLE ACUTE */
        //case XK_tcedilla                      0x01fe  /* U+0163 LATIN SMALL LETTER T WITH CEDILLA */
        //case XK_abovedot                      0x01ff  /* U+02D9 DOT ABOVE */
        //case XK_Hstroke                       0x02a1  /* U+0126 LATIN CAPITAL LETTER H WITH STROKE */
        //case XK_Hcircumflex                   0x02a6  /* U+0124 LATIN CAPITAL LETTER H WITH CIRCUMFLEX */
        //case XK_Iabovedot                     0x02a9  /* U+0130 LATIN CAPITAL LETTER I WITH DOT ABOVE */
        //case XK_Gbreve                        0x02ab  /* U+011E LATIN CAPITAL LETTER G WITH BREVE */
        //case XK_Jcircumflex                   0x02ac  /* U+0134 LATIN CAPITAL LETTER J WITH CIRCUMFLEX */
        //case XK_hstroke                       0x02b1  /* U+0127 LATIN SMALL LETTER H WITH STROKE */
        //case XK_hcircumflex                   0x02b6  /* U+0125 LATIN SMALL LETTER H WITH CIRCUMFLEX */
        //case XK_idotless                      0x02b9  /* U+0131 LATIN SMALL LETTER DOTLESS I */
        //case XK_gbreve                        0x02bb  /* U+011F LATIN SMALL LETTER G WITH BREVE */
        //case XK_jcircumflex                   0x02bc  /* U+0135 LATIN SMALL LETTER J WITH CIRCUMFLEX */
        //case XK_Cabovedot                     0x02c5  /* U+010A LATIN CAPITAL LETTER C WITH DOT ABOVE */
        //case XK_Ccircumflex                   0x02c6  /* U+0108 LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
        //case XK_Gabovedot                     0x02d5  /* U+0120 LATIN CAPITAL LETTER G WITH DOT ABOVE */
        //case XK_Gcircumflex                   0x02d8  /* U+011C LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
        //case XK_Ubreve                        0x02dd  /* U+016C LATIN CAPITAL LETTER U WITH BREVE */
        //case XK_Scircumflex                   0x02de  /* U+015C LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
        //case XK_cabovedot                     0x02e5  /* U+010B LATIN SMALL LETTER C WITH DOT ABOVE */
        //case XK_ccircumflex                   0x02e6  /* U+0109 LATIN SMALL LETTER C WITH CIRCUMFLEX */
        //case XK_gabovedot                     0x02f5  /* U+0121 LATIN SMALL LETTER G WITH DOT ABOVE */
        //case XK_gcircumflex                   0x02f8  /* U+011D LATIN SMALL LETTER G WITH CIRCUMFLEX */
        //case XK_ubreve                        0x02fd  /* U+016D LATIN SMALL LETTER U WITH BREVE */
        //case XK_scircumflex                   0x02fe  /* U+015D LATIN SMALL LETTER S WITH CIRCUMFLEX */
        //case XK_kra                           0x03a2  /* U+0138 LATIN SMALL LETTER KRA */
        //case XK_kappa                         0x03a2  /* deprecated */
        //case XK_Rcedilla                      0x03a3  /* U+0156 LATIN CAPITAL LETTER R WITH CEDILLA */
        //case XK_Itilde                        0x03a5  /* U+0128 LATIN CAPITAL LETTER I WITH TILDE */
        //case XK_Lcedilla                      0x03a6  /* U+013B LATIN CAPITAL LETTER L WITH CEDILLA */
        //case XK_Emacron                       0x03aa  /* U+0112 LATIN CAPITAL LETTER E WITH MACRON */
        //case XK_Gcedilla                      0x03ab  /* U+0122 LATIN CAPITAL LETTER G WITH CEDILLA */
        //case XK_Tslash                        0x03ac  /* U+0166 LATIN CAPITAL LETTER T WITH STROKE */
        //case XK_rcedilla                      0x03b3  /* U+0157 LATIN SMALL LETTER R WITH CEDILLA */
        //case XK_itilde                        0x03b5  /* U+0129 LATIN SMALL LETTER I WITH TILDE */
        //case XK_lcedilla                      0x03b6  /* U+013C LATIN SMALL LETTER L WITH CEDILLA */
        //case XK_emacron                       0x03ba  /* U+0113 LATIN SMALL LETTER E WITH MACRON */
        //case XK_gcedilla                      0x03bb  /* U+0123 LATIN SMALL LETTER G WITH CEDILLA */
        //case XK_tslash                        0x03bc  /* U+0167 LATIN SMALL LETTER T WITH STROKE */
        //case XK_ENG                           0x03bd  /* U+014A LATIN CAPITAL LETTER ENG */
        //case XK_eng                           0x03bf  /* U+014B LATIN SMALL LETTER ENG */
        //case XK_Amacron                       0x03c0  /* U+0100 LATIN CAPITAL LETTER A WITH MACRON */
        //case XK_Iogonek                       0x03c7  /* U+012E LATIN CAPITAL LETTER I WITH OGONEK */
        //case XK_Eabovedot                     0x03cc  /* U+0116 LATIN CAPITAL LETTER E WITH DOT ABOVE */
        //case XK_Imacron                       0x03cf  /* U+012A LATIN CAPITAL LETTER I WITH MACRON */
        //case XK_Ncedilla                      0x03d1  /* U+0145 LATIN CAPITAL LETTER N WITH CEDILLA */
        //case XK_Omacron                       0x03d2  /* U+014C LATIN CAPITAL LETTER O WITH MACRON */
        //case XK_Kcedilla                      0x03d3  /* U+0136 LATIN CAPITAL LETTER K WITH CEDILLA */
        //case XK_Uogonek                       0x03d9  /* U+0172 LATIN CAPITAL LETTER U WITH OGONEK */
        //case XK_Utilde                        0x03dd  /* U+0168 LATIN CAPITAL LETTER U WITH TILDE */
        //case XK_Umacron                       0x03de  /* U+016A LATIN CAPITAL LETTER U WITH MACRON */
        //case XK_amacron                       0x03e0  /* U+0101 LATIN SMALL LETTER A WITH MACRON */
        //case XK_iogonek                       0x03e7  /* U+012F LATIN SMALL LETTER I WITH OGONEK */
        //case XK_eabovedot                     0x03ec  /* U+0117 LATIN SMALL LETTER E WITH DOT ABOVE */
        //case XK_imacron                       0x03ef  /* U+012B LATIN SMALL LETTER I WITH MACRON */
        //case XK_ncedilla                      0x03f1  /* U+0146 LATIN SMALL LETTER N WITH CEDILLA */
        //case XK_omacron                       0x03f2  /* U+014D LATIN SMALL LETTER O WITH MACRON */
        //case XK_kcedilla                      0x03f3  /* U+0137 LATIN SMALL LETTER K WITH CEDILLA */
        //case XK_uogonek                       0x03f9  /* U+0173 LATIN SMALL LETTER U WITH OGONEK */
        //case XK_utilde                        0x03fd  /* U+0169 LATIN SMALL LETTER U WITH TILDE */
        //case XK_umacron                       0x03fe  /* U+016B LATIN SMALL LETTER U WITH MACRON */
        //case XK_Wcircumflex                0x1000174  /* U+0174 LATIN CAPITAL LETTER W WITH CIRCUMFLEX */
        //case XK_wcircumflex                0x1000175  /* U+0175 LATIN SMALL LETTER W WITH CIRCUMFLEX */
        //case XK_Ycircumflex                0x1000176  /* U+0176 LATIN CAPITAL LETTER Y WITH CIRCUMFLEX */
        //case XK_ycircumflex                0x1000177  /* U+0177 LATIN SMALL LETTER Y WITH CIRCUMFLEX */
        //case XK_Babovedot                  0x1001e02  /* U+1E02 LATIN CAPITAL LETTER B WITH DOT ABOVE */
        //case XK_babovedot                  0x1001e03  /* U+1E03 LATIN SMALL LETTER B WITH DOT ABOVE */
        //case XK_Dabovedot                  0x1001e0a  /* U+1E0A LATIN CAPITAL LETTER D WITH DOT ABOVE */
        //case XK_dabovedot                  0x1001e0b  /* U+1E0B LATIN SMALL LETTER D WITH DOT ABOVE */
        //case XK_Fabovedot                  0x1001e1e  /* U+1E1E LATIN CAPITAL LETTER F WITH DOT ABOVE */
        //case XK_fabovedot                  0x1001e1f  /* U+1E1F LATIN SMALL LETTER F WITH DOT ABOVE */
        //case XK_Mabovedot                  0x1001e40  /* U+1E40 LATIN CAPITAL LETTER M WITH DOT ABOVE */
        //case XK_mabovedot                  0x1001e41  /* U+1E41 LATIN SMALL LETTER M WITH DOT ABOVE */
        //case XK_Pabovedot                  0x1001e56  /* U+1E56 LATIN CAPITAL LETTER P WITH DOT ABOVE */
        //case XK_pabovedot                  0x1001e57  /* U+1E57 LATIN SMALL LETTER P WITH DOT ABOVE */
        //case XK_Sabovedot                  0x1001e60  /* U+1E60 LATIN CAPITAL LETTER S WITH DOT ABOVE */
        //case XK_sabovedot                  0x1001e61  /* U+1E61 LATIN SMALL LETTER S WITH DOT ABOVE */
        //case XK_Tabovedot                  0x1001e6a  /* U+1E6A LATIN CAPITAL LETTER T WITH DOT ABOVE */
        //case XK_tabovedot                  0x1001e6b  /* U+1E6B LATIN SMALL LETTER T WITH DOT ABOVE */
        //case XK_Wgrave                     0x1001e80  /* U+1E80 LATIN CAPITAL LETTER W WITH GRAVE */
        //case XK_wgrave                     0x1001e81  /* U+1E81 LATIN SMALL LETTER W WITH GRAVE */
        //case XK_Wacute                     0x1001e82  /* U+1E82 LATIN CAPITAL LETTER W WITH ACUTE */
        //case XK_wacute                     0x1001e83  /* U+1E83 LATIN SMALL LETTER W WITH ACUTE */
        //case XK_Wdiaeresis                 0x1001e84  /* U+1E84 LATIN CAPITAL LETTER W WITH DIAERESIS */
        //case XK_wdiaeresis                 0x1001e85  /* U+1E85 LATIN SMALL LETTER W WITH DIAERESIS */
        //case XK_Ygrave                     0x1001ef2  /* U+1EF2 LATIN CAPITAL LETTER Y WITH GRAVE */
        //case XK_ygrave                     0x1001ef3  /* U+1EF3 LATIN SMALL LETTER Y WITH GRAVE */
        //case XK_OE                            0x13bc  /* U+0152 LATIN CAPITAL LIGATURE OE */
        //case XK_oe                            0x13bd  /* U+0153 LATIN SMALL LIGATURE OE */
        //case XK_Ydiaeresis                    0x13be  /* U+0178 LATIN CAPITAL LETTER Y WITH DIAERESIS */
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
