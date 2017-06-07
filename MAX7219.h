
#ifndef MAX7219_h
#define MAX7219_h


// 7219 Registers
#define REG_NO_OP           0x00
#define REG_DIGIT_0         0x01
#define REG_DIGIT_1         0x02
#define REG_DIGIT_2         0x03
#define REG_DIGIT_3         0x04
#define REG_DIGIT_4         0x05
#define REG_DIGIT_5         0x06
#define REG_DIGIT_6         0x07
#define REG_DIGIT_7         0x08
#define REG_DECODE_MODE     0x09
#define REG_INTENSITY       0x0A
#define REG_SCAN_LIMIT      0x0B
#define REG_SHUTDOWN        0x0C
#define REG_DISPLAY_TEST    0x0F


//ScanLimit
#define	ScanLimit_Digit0     0x00
#define ScanLimit_Digit0To1  0x01
#define ScanLimit_Digit0To2  0x02
#define ScanLimit_Digit0To3  0x03
#define ScanLimit_Digit0To4  0x04
#define ScanLimit_Digit0To5  0x05
#define ScanLimit_Digit0To6  0x06
#define ScanLimit_Digit0To7  0x07


// ShutdownModes
#define	ShutdownMode_Shutdown            0x00
#define	ShutdownMode_NormalOperation     0x01

// DisplayTestModes
#define NormalOperation  0x00
#define TestMode         0x01

#define  Intensity_Level0   0x00
#define  Intensity_Level1   0x01
#define  Intensity_Level2   0x02
#define  Intensity_Level3   0x03
#define  Intensity_Level4   0x04
#define  Intensity_Level5   0x05
#define  Intensity_Level6   0x06
#define  Intensity_Level7   0x07
#define  Intensity_Level8   0x08
#define  Intensity_Level9   0x09
#define  Intensity_Level10  0x0A
#define  Intensity_Level11  0x0B
#define  Intensity_Level12  0x0C
#define  Intensity_Level13  0x0D
#define  Intensity_Level14  0x0E
#define  Intensity_Level15  0x0F
 
// DecodeMode 
#define   DecodeMode_NoDecode     0x00
#define   DecodeMode_Digit0       0x01
#define   DecodeMode_Digit1       0x02
#define   DecodeMode_Digit2       0x04
#define   DecodeMode_Digit3       0x08
#define   DecodeMode_Digit4       0x10
#define   DecodeMode_Digit5       0x20
#define   DecodeMode_Digit6       0x40
#define   DecodeMode_Digit7       0x80
#define   DecodeMode_AllDigits    0xFF


#endif // 7219 definition end

