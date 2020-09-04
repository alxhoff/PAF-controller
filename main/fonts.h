
#ifndef Fonts
#define Fonts

typedef struct {
    const char FontWidth;    /*!< Font width in pixels */
    const char FontHeight;   /*!< Font height in pixels */
    const short *data; /*!< Pointer to data font data array */
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif

