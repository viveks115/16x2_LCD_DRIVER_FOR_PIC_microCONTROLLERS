
#include "mcc_generated_files/mcc.h"
#include "LCD_16x2.h"

#define LCD_USE_8BIT_DATA
#define EXT_LCD_5_V

#ifndef LCD_USE_8BIT_DATA
  #define LCD_D3_OUT  LCD_D7_OUT
  #define LCD_D2_OUT  LCD_D6_OUT
  #define LCD_D1_OUT  LCD_D5_OUT
  #define LCD_D0_OUT  LCD_D4_OUT
  #define LCD_D3_IN   LCD_D7_IN
#endif

union LCDv8bit LCD_data;         // bitfield variable (bitwise acess)
unsigned short LCD_busy_cnt;

// -------------------- LCD-functions ------------------------------------------

void LCD_Init (void)
{
    LCD_RW = 0; LCD_RW_DIR = 0;
    LCD_RS = 0; LCD_RS_DIR = 0;
    LCD_E  = 0; LCD_E_DIR  = 0;

    LCD_DIR_OUT();

    LCD_busy_cnt = 1;		// busy_flag time out counter

    LCD_DELAY_5MS();LCD_DELAY_5MS();LCD_DELAY_5MS();    // wait for 15ms
// display reset procedure
    LCD_Write_Nibble(LCD_RESET);    LCD_DELAY_5MS();
    LCD_Write_Nibble(LCD_RESET);    LCD_DELAY_5MS();
    LCD_Write_Nibble(LCD_RESET);    LCD_DELAY_5MS();

#ifdef LCD_USE_8BIT_DATA
    LCD_Write_Nibble(EIGHT_BIT); while(LCD_Busy()){;} // wait
    #if defined EXT_LCD_3_3_V
        LCD_Command(EIGHT_BIT_TWO_LINE + EXT_INSTR_TBL_1);
        LCD_Command(EXT1_BIAS_1_5);
        LCD_Command(EXT1_CONTRAST + 0x0C);
        LCD_Command(EXT1_BOOST_ICON_C + BOOST_ON + 1);
        LCD_Command(EXT1_FOLLOWER + FOLLOWER_ON + 5);
    #elif defined EXT_LCD_5_V
        LCD_Command(EIGHT_BIT_TWO_LINE + EXT_INSTR_TBL_1);
        LCD_Command(EXT1_BIAS_1_4);
        LCD_Command(EXT1_CONTRAST + 0x04);
        LCD_Command(EXT1_BOOST_ICON_C + BOOST_OFF + 2);
        LCD_Command(EXT1_FOLLOWER + FOLLOWER_ON + 1);
    #else
        LCD_Command(EIGHT_BIT_TWO_LINE);
    #endif
#else
    LCD_Write_Nibble(FOUR_BIT); while(LCD_Busy()){;} // wait
    #if defined EXT_LCD_3_3_V
        LCD_Command(FOUR_BIT_TWO_LINE + EXT_INSTR_TBL_1);
        LCD_Command(EXT1_BIAS_1_5);
        LCD_Command(EXT1_CONTRAST + 0x0C);
        LCD_Command(EXT1_BOOST_ICON_C + BOOST_ON + 1);
        LCD_Command(EXT1_FOLLOWER + FOLLOWER_ON + 5);
    #elif defined EXT_LCD_5_V
        LCD_Command(FOUR_BIT_TWO_LINE + EXT_INSTR_TBL_1);
        LCD_Command(EXT1_BIAS_1_4);
        LCD_Command(EXT1_CONTRAST + 0x04);
        LCD_Command(EXT1_BOOST_ICON_C + BOOST_OFF + 2);
        LCD_Command(EXT1_FOLLOWER + FOLLOWER_ON + 1);
    #else
        LCD_Command(FOUR_BIT_TWO_LINE);
    #endif
#endif
    LCD_Command(DISPLAY_CTRL + DISPLAY_ON);// + BLINK_ON);
    LCD_Command(ENTRY_MODE + CURSOR_INC + DSHIFT_OFF);
    LCD_Clear();  
//    LCD_Command(CLEAR_DISPLAY);
    LCD_Home();
//    LCD_Command(RETURN_HOME);
}
// --------------------------------------------------------------

void LCD_signalsTest()
{
    LCD_RS_DIR = 0;
    LCD_RW_DIR = 0;
    LCD_E_DIR = 0;
    LCD_D4_DIR = 0;
    LCD_D5_DIR = 0;
    LCD_D6_DIR = 0;
    LCD_D7_DIR = 0;

    LCD_RS = 1;
    LCD_RW = 1;
    LCD_ENABLE();
    LCD_D4_OUT = 1;
    LCD_D5_OUT = 1;
    LCD_D6_OUT = 1;
    LCD_D7_OUT = 1;

    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DISABLE();
    LCD_D4_OUT = 0;
    LCD_D5_OUT = 0;
    LCD_D6_OUT = 0;
    LCD_D7_OUT = 0;
}


//****************************************************************************
//     _    ______________________________
// RS  _>--<______________________________
//     _____
// RW       \_____________________________
//                  __________________
// E   ____________/                  \___
//     _____________                ______
// DB  _____________>--------------<______
void LCD_Write_Nibble(unsigned char value)
{
    LCD_data.all = value;
    LCD_RS = LCD_CMD;
    LCD_RW = LCD_WR;

    LCD_D4_OUT = LCD_data.bit4;
    LCD_D5_OUT = LCD_data.bit5;
    LCD_D6_OUT = LCD_data.bit6;
    LCD_D7_OUT = LCD_data.bit7;
    LCD_STROBE();
}
// --------------------------------------------------------------

void LCD_Write(unsigned char value) // command or data
{
    LCD_data.all = value;
    LCD_RW = LCD_WR;

    LCD_D7_OUT = LCD_data.bit7;
    LCD_D6_OUT = LCD_data.bit6;
    LCD_D5_OUT = LCD_data.bit5;
    LCD_D4_OUT = LCD_data.bit4;
#ifndef LCD_USE_8BIT_DATA
    LCD_STROBE();
#endif

    LCD_D3_OUT = LCD_data.bit3;
    LCD_D2_OUT = LCD_data.bit2;
    LCD_D1_OUT = LCD_data.bit1;
    LCD_D0_OUT = LCD_data.bit0;
    LCD_STROBE();
}
// --------------------------------------------------------------

void LCD_ValueOut_00(unsigned char row, unsigned char col, short value,
                     unsigned char min_dig)
{
    char chBuffer, leading_0;

    LCD_Command(CURSOR_ADDR + (LINE_OFFSET * row) + col);
    leading_0 = 1;

    if (value < 0){
        chBuffer = '-';
        LCD_CharOut(chBuffer);
        value = -value;
    }
    else if(!leading_0 &&(min_dig == 1)){
        chBuffer = ' ';
        LCD_CharOut(chBuffer);
    }
    chBuffer = value / 10000;
    if(leading_0){
        if((chBuffer != 0)||(min_dig == 5)) leading_0 = 0;
    }
    if(!leading_0){
        LCD_CharOut(chBuffer +0x30);
    }
    value = value - (10000 * chBuffer);
    chBuffer = value / 1000;
    if(leading_0){
        if((chBuffer != 0)||(min_dig == 4)) leading_0 = 0;
    }
    if(!leading_0){
        LCD_CharOut(chBuffer +0x30);
    }
    value = value - (1000 * chBuffer);
    chBuffer = value / 100;
    if(leading_0){
        if((chBuffer != 0)||(min_dig == 3)) leading_0 = 0;
    }
    if(!leading_0){
        LCD_CharOut(chBuffer +0x30);
    }
    value = value - ((short)100 * chBuffer);
    chBuffer = value  / 10;
    if(leading_0){
        if((chBuffer != 0)||(min_dig == 2)) leading_0 = 0;
    }
    if(!leading_0){
        LCD_CharOut(chBuffer +0x30);
    }
    value = value - ((short)10 * chBuffer);
    chBuffer = value;
    LCD_CharOut(chBuffer +0x30);
}
// --------------------------------------------------------------

void LCD_ConstTextOut(unsigned char row, unsigned char col, const char *text)
{
    LCD_Command(CURSOR_ADDR + (LINE_OFFSET *  row) +  col);
    while (*text) {
        LCD_CharOut(*text);
        text++;
    }
}
// --------------------------------------------------------------

void LCD_TextOut(unsigned char row, unsigned char col, unsigned char *text)
{
    LCD_Command(CURSOR_ADDR + (LINE_OFFSET * row) +  col);
    while (*text) {
        LCD_CharOut(*text);
        text++;
    }
}
// --------------------------------------------------------------

//unsigned char LCD_Read(void)
//{
//    LCD_D4_DIR = 1;
//    LCD_D5_DIR = 1;
//    LCD_D6_DIR = 1;
//    LCD_D7_DIR = 1;
//    LCD_RW = LCD_RD; // Set read mode
//
//    LCD_ENABLE(); LCD_DELAY_1US();
//    LCD_data.bit7 = LCD_D7_IN;
//    LCD_data.bit6 = LCD_D6_IN;
//    LCD_data.bit5 = LCD_D5_IN;
//    LCD_data.bit4 = LCD_D4_IN;
//    LCD_DISABLE();
//
//    LCD_ENABLE(); LCD_DELAY_1US();
//    LCD_data.bit3 = LCD_D7_IN;
//    LCD_data.bit2 = LCD_D6_IN;
//    LCD_data.bit1 = LCD_D5_IN;
//    LCD_data.bit0 = LCD_D4_IN;
//    LCD_DISABLE();
//
//    return LCD_data.all;
//}

unsigned char LCD_Busy(void)
{
    if (LCD_busy_cnt >= LCD_TIMEOUT){
        LCD_busy_cnt = 1;
        return 0;                   // return -1 for time out ???????????????
    }

    LCD_RW = LCD_RD;
    LCD_RS = LCD_CMD;
    LCD_DIR_IN();

    LCD_ENABLE(); LCD_DELAY_1US();
    LCD_data.bit7 = LCD_D7_IN;
    LCD_DISABLE();
    LCD_ENABLE(); LCD_DELAY_1US();
//    LCD_data.bit3 = LCD_D3_IN;
    LCD_DISABLE();

    LCD_DIR_OUT();
    LCD_RW = LCD_WR;

    if (LCD_data.bit7 == LCD_BUSY) {
        LCD_busy_cnt++;
        return 1;
    } else {
        LCD_busy_cnt = 1;
        return 0;
    }
}
// -------------------- END LCD-functions ---------------------------
