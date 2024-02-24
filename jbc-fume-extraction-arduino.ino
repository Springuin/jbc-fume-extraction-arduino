#include "JBCComm.h"


JBCComm jbcComm = JBCComm();

uint32_t fumeExtractorTimer;
#define FUME_EXTRACT_ON_DELAY 1000
#define FUME_EXTRACT_OFF_DELAY 3000

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
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        Serial.println("Extraction off");
        digitalWrite(LED_BUILTIN, LOW);
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
    pinMode(LED_BUILTIN, OUTPUT);
    fumeExtractorSet(false);
    jbcComm.RegisterToolstatusUpdatedCallback(toolStatusUpdated);
}

void loop() {
    jbcComm.Process();
}
