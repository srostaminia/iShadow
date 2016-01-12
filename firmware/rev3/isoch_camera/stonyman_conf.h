#ifndef __STONYMAN_CONF_H
#define __STONYMAN_CONF_H

// Record video on eye camera or not
#define EYE_VIDEO_ON
//#define EYE_VIDEO_OFF

// Record video on out camera or not
#define OUT_VIDEO_ON
//#define OUT_VIDEO_OFF

// CIDER_TRACKING NOT IMPLEMENTED!!
// Do eye tracking with ANN or CIDER model (mutually exclusive)
// #define ANN_TRACKING
// #define CIDER_TRACKING

// Choose USB or SD transmission
// Note: USB can only handle one video stream, default is eye
//#define USB_SEND
#define SD_SEND

// Comment out to collect data row-wise instead of column-wise
#define COLUMN_COLLECT

// Enables use of an uploaded binary file for FPN masks and eye model parameters
#define USE_PARAM_FILE

// OUTMODE CURRENTLY NOT IMPLEMENTED!!
// Uncomment to use outdoor settings
//#define OUTMODE

// LED FUNCTIONS NOT FULLY IMPLEMENTED!!
// Comment out to leave LEDs on at all times
//#define LED_DUTY_CYCLE

// USB_16BIT CURRENTLY NOT IMPLEMENTED!!
// Uncomment one or the other for USB pixel transmission rate
// (doesn't affect SD card)
//#define USB_16BIT
#define USB_8BIT

// OUTDOOR_SWITCH CURRENTLY NOT IMPLEMENTED!
// Uncomment to switch to outdoor mode based on photodiode
//#define OUTDOOR_SWITCH

#endif // __STONYMAN_CONF_H