#include "JBCComm.h"


JBCComm jbcComm = JBCComm();

/* Note: Pins 9-13 are used for the interface to the USB host shield */
const unsigned int LEDPIN = 8;

uint32_t fumeExtractorTimer;
#define FUME_EXTRACT_ON_DELAY 1000
// The Weller KH-E has it's own delay, so the off delay can be short. When using a relay, this can be longer, e.g. 3 seconds.
#define FUME_EXTRACT_OFF_DELAY 10

typedef enum {
    fe_Off,
    fe_OnDelay,
    fe_On,
    fe_Runout,
} FumeExtractionState;
FumeExtractionState feState = fe_Off;

void fumeExtractorSet(bool on) {
    if (on) {
        Serial.println("Extraction on");
        digitalWrite(LEDPIN, HIGH);
    } else {
        Serial.println("Extraction off");
        digitalWrite(LEDPIN, LOW);
    }
}

void fumeExtractorProcess(bool active) {
    switch(feState) {
        case fe_Off:
            if (active) {
                fumeExtractorTimer = millis();
                feState = fe_OnDelay;
                Serial.println("On delay");
            }
            break;
        case fe_OnDelay:
            if (!active) {
                feState = fe_Off;
            } else if (millis() - fumeExtractorTimer > FUME_EXTRACT_ON_DELAY) {
                feState = fe_On;
                fumeExtractorSet(true);
            }
            break;
        case fe_On:
            if (!active) {
                feState = fe_Runout;
                fumeExtractorTimer = millis();
                Serial.println("Runout");
            }
            break;
        case fe_Runout:
            if(active) {
                feState = fe_On;
            } else if (millis() - fumeExtractorTimer > FUME_EXTRACT_OFF_DELAY) {
                feState = fe_Off;
                fumeExtractorSet(false);
            }
            break;
    }
}

void toolStatusUpdated() {
    fumeExtractorProcess(jbcComm.toolStatus.desold);
}

void setup() {
    Serial.begin(115200);
    jbcComm.Init();
    pinMode(LEDPIN, OUTPUT);
    fumeExtractorSet(false);
    jbcComm.RegisterToolstatusUpdatedCallback(toolStatusUpdated);
}

void loop() {
    uint8_t result = jbcComm.Process();
    if (result != 0) {
        // Reinitialize attempt
        Serial.print("Reinitializing. Result: ");
        Serial.println(result);
        
        // Delay to avoid overflooding the UART
        delay(1000);
        
        jbcComm.Init();
    }    
}
