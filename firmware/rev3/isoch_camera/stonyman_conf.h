#ifndef __STONYMAN_CONF_H
#define __STONYMAN_CONF_H

// Choose USB or SD transmission
#define USB_SEND
//#define SD_SEND

// Select whether to use only primary camera or use both
#define SINGLE_CAM
//#define DUAL_CAM

// Select primary camera, will be used for single-camera functions
// (overriden by CIDER_MODE)
#define EYE_CAM_PRIMARY
//#define OUT_CAM_PRIMARY

// CIDER MODE CURRENTLY NOT IMPLEMENTED!!
// Uncomment *one* of these to use one of the built-in eye tracking models
// Requires a parameter file, see readme for details
// Selecting one of these overrides some other config settings
#define ANN_MODE
//#define CIDER_MODE

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