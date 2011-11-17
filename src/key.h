#ifndef KEY_H
#define KEY_H 

//#define Key_Release_Mask 0x10000000

#define Key_Escape 0x01000000                // misc keys
#define Key_Tab 0x01000009
#define Key_Backtab 0x01000002
#define Key_Backspace 0x01000008
#define Key_Return 0x0100000A
#define Key_Enter 0x01000005
#define Key_Insert 0x01000006
#define Key_Delete 0x01000007
#define Key_Pause 0x01000008
#define Key_Print 0x01000009
#define Key_SysReq 0x0100000a
#define Key_Clear 0x0100000b
#define Key_Home 0x01000010                // cursor movement
#define Key_End 0x01000011
#define Key_Left 0x01000012
#define Key_Up 0x01000013
#define Key_Right 0x01000014
#define Key_Down 0x01000015
#define Key_PageUp 0x01000016
#define Key_PageDown 0x01000017
#define Key_LeftShift 0x01000029                // modifiers
#define Key_RightShift 0x01000020                // modifiers
#define Key_LeftControl 0x01000018
#define Key_RightControl 0x01000021
#define Key_Meta 0x01000022
#define Key_LeftAlt 0x01000019
#define Key_RightAlt 0x01000023
#define Key_CapsLock 0x01000024
#define Key_NumLock 0x01000025
#define Key_ScrollLock 0x01000026
#define Key_F1 0x01000030                // function keys
#define Key_F2 0x01000031
#define Key_F3 0x01000032
#define Key_F4 0x01000033
#define Key_F5 0x01000034
#define Key_F6 0x01000035
#define Key_F7 0x01000036
#define Key_F8 0x01000037
#define Key_F9 0x01000038
#define Key_F10 0x01000039
#define Key_F11 0x0100003a
#define Key_F12 0x0100003b
#define Key_F13 0x0100003c
#define Key_F14 0x0100003d
#define Key_F15 0x0100003e
#define Key_F16 0x0100003f
#define Key_F17 0x01000040
#define Key_F18 0x01000041
#define Key_F19 0x01000042
#define Key_F20 0x01000043
#define Key_F21 0x01000044
#define Key_F22 0x01000045
#define Key_F23 0x01000046
#define Key_F24 0x01000047
#define Key_F25 0x01000048                // F25 .. F35 only on X11
#define Key_F26 0x01000049
#define Key_F27 0x0100004a
#define Key_F28 0x0100004b
#define Key_F29 0x0100004c
#define Key_F30 0x0100004d
#define Key_F31 0x0100004e
#define Key_F32 0x0100004f
#define Key_F33 0x01000050
#define Key_F34 0x01000051
#define Key_F35 0x01000052
#define Key_Super_L 0x01000053                 // extra keys
#define Key_Super_R 0x01000054
#define Key_Menu 0x01000055
#define Key_Hyper_L 0x01000056
#define Key_Hyper_R 0x01000057
#define Key_Help 0x01000058
#define Key_Direction_L 0x01000059
#define Key_Direction_R 0x01000060
#define Key_Space 0x20                // 7 bit printable ASCII
#define Key_Any Key_Space
#define Key_Exclam 0x21
#define Key_QuoteDbl 0x22
#define Key_NumberSign 0x23
#define Key_Dollar 0x24
#define Key_Percent 0x25
#define Key_Ampersand 0x26
#define Key_Apostrophe 0x27
#define Key_ParenLeft 0x28
#define Key_ParenRight 0x29
#define Key_Asterisk 0x2a
#define Key_Plus 0x2b
#define Key_Comma 0x2c
#define Key_Minus 0x2d
#define Key_Period 0x2e
#define Key_Slash 0x2f
#define Key_0 0x30
#define Key_1 0x31
#define Key_2 0x32
#define Key_3 0x33
#define Key_4 0x34
#define Key_5 0x35
#define Key_6 0x36
#define Key_7 0x37
#define Key_8 0x38
#define Key_9 0x39
#define Key_Colon 0x3a
#define Key_Semicolon 0x3b
#define Key_Less 0x3c
#define Key_Equal 0x3d
#define Key_Greater 0x3e
#define Key_Question 0x3f
#define Key_At 0x40
#define Key_A 0x41
#define Key_B 0x42
#define Key_C 0x43
#define Key_D 0x44
#define Key_E 0x45
#define Key_F 0x46
#define Key_G 0x47
#define Key_H 0x48
#define Key_I 0x49
#define Key_J 0x4a
#define Key_K 0x4b
#define Key_L 0x4c
#define Key_M 0x4d
#define Key_N 0x4e
#define Key_O 0x4f
#define Key_P 0x50
#define Key_Q 0x51
#define Key_R 0x52
#define Key_S 0x53
#define Key_T 0x54
#define Key_U 0x55
#define Key_V 0x56
#define Key_W 0x57
#define Key_X 0x58
#define Key_Y 0x59
#define Key_Z 0x5a
#define Key_BracketLeft 0x5b
#define Key_Backslash 0x5c
#define Key_BracketRight 0x5d
#define Key_AsciiCircum 0x5e
#define Key_Underscore 0x5f
#define Key_QuoteLeft 0x60
#define Key_BraceLeft 0x7b
#define Key_Bar 0x7c
#define Key_BraceRight 0x7d
#define Key_AsciiTilde 0x7e

#define     KEY_EVENT_DOWN  1
#define     KEY_EVENT_UP    2

struct KeyEvent
{
    int type;
    int code;
}__attribute__((packed));

#endif /* KEY_H */
