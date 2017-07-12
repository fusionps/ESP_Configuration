#include <FS.h>
#include "IPAddress.h"
#include "ESP_Configuration.h"

#define NUM_CONFIGS 1
ConfigurationItem* list[NUM_CONFIGS] = {
    new CharArray("test", "Hello")
};

Configuration* config = new Configuration(list, NUM_CONFIGS);

void setup(){
    Serial.begin(115200);
    Serial.println("HELLO!");
    
    config->init();
//    config->setDefaults();
    
    Serial.print("Reading saved test config: ");
    Serial.println((char*)config->getConfiguration("test"));
    
    Serial.print("Setting config: "); config->setConfiguration("test", (void*)"Yooooo");Serial.println("Yooooo");
    Serial.print("Getting config: "); Serial.println((char*)config->getConfiguration("test"));
}

void loop(){
    
}