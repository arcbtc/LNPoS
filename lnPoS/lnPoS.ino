// Don't forget to configure your hardware device in hardware_device.h:
#include "hardware_device.h"

#if HARDWARE_DEVICE == 1
  #include "lnPoSBareBones.cpp.h"
#elif HARDWARE_DEVICE == 2
  #include "lnPoSM5Stack.cpp.h"
#elif HARDWARE_DEVICE == 3
  #include "lnPoSTdisplay.cpp.h"
#else
  #error "HARDWARE_DEVICE is not set or invalid value! Check lnPoS/hardware_device.h and also make sure libraries/TFT_eSPI/User_Setup_Select.h oints to the absolute path of LNPoS/lnPoS/hardware_device.h"
#endif

// For now, loop() and setup() are implemented in device-specific functions, see *.cpp.h files.


// All common functions that are shared between the
// different device implementations will be placed below.
