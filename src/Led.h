#include "GpioLogic.h"
#include "configuration.h"

/**
 * ledForceOn and ledForceOff both override the normal ledBlinker behavior (which is controlled by main)
 * ledTransmit provides visual feedback during radio transmissions
 * ledReceive provides visual feedback for incoming radio messages
 */
extern GpioVirtPin ledForceOn, ledBlink, ledTransmit, ledReceive;
