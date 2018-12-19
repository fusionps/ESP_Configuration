#include "ESP_Configuration.h"
#include "ConfigurationItem.h"
#include <FS.h>
#include <SPIFFS.h>

int min(int a, int b){
    if(a < b) return a;
    return b;
}

void Configuration::init(){
    // Here is where we handle loadingSavedConfiguration or saving Default
    SPIFFS.begin();
    resetPointer();
    if(!loadConfiguration()){ // this will return false if there is no config in the FS, or if there is but with less data points
        Serial.println("COULD NOT FIND THINGO");
        saveConfiguration(); // This should save the default parameters into the FS
    } else {
        Serial.println("LOADED THINGS");
    }
    Serial.println(stuff);
}

ConfigurationItem* Configuration::findConfigWithKey(const char* key){
    resetPointer();
    do {
        if(strcmp(currentConfig->key, key) == 0){
            return currentConfig;
        }
    } while (nextValue());
    return 0;
}

bool Configuration::checkForTwenty(){
    // checks if we are running on a 20 char limit
    bool twenty = false;
    if(!SPIFFS.exists("/config.txt")){
        return false;
    }

    File f = SPIFFS.open("/config.txt", "r");
    if(!f)
        return false;

    char buf[21];

    if(f.size() > 20){
        f.readBytes(buf, 20);
        buf[21] = 0x0;
        if(strcmp(buf, "Hello") == 0){
            twenty = true;
        }
    }
    f.close();
    return twenty;
}

bool Configuration::loadForTwenty(){
    // Connect to FS, try and load config.txt, if no file return false
    // Read until 0x00, is length is different to TOTAL_CONFIG_LENGTH, we load what we can into setVal(so we overwrite default), then return false to save the rest
    bool intact = true;
    if(!SPIFFS.exists("/config.txt")){
        return false;
    }
    File f = SPIFFS.open("/config.txt", "r");
    if(!f)
        return false;
    
    resetPointer();

    char buf[21];
    bool reachedEnd = false;

    Serial.printf("f.size is %d, f.position is %d, currentConfig->getLength() is %d, min is %d\n", f.size(), f.position(), currentConfig->getLength(), min(currentConfig->getLength(), 20));

    do {
        if(f.size() - f.position() < min(currentConfig->getLength(), 20) || reachedEnd){
            // Config file is not big enough, lets move on.
            Serial.println("bad one found! ");
            intact = false;
            currentConfig->setDefault(); // this will make sure, when we save, we have loaded the default values for the values we didn't have
            reachedEnd = true;
            continue;
        } else {
            // good
            Serial.println("Good one found! ");
        }
        
        f.readBytes(buf, min(currentConfig->getLength(), 20));
        buf[min(currentConfig->getLength(), 20)] = 0x0;
        currentConfig->set(buf);
        Serial.println(currentConfig->toString());
        memset(buf, 0, 21); //cleanup BUF
    } while (nextValue());
    f.close();
    return intact;
}

void* Configuration::getConfiguration(const char* key){
    ConfigurationItem* item = findConfigWithKey(key);
    if(item){
        return item->get();
    } else {
        return &blank;
    }
}

bool Configuration::setConfiguration(const char* key, void* value){
    if(findConfigWithKey(key)->setValue(value)){
        return saveConfiguration();
    }
    return false;
}

bool Configuration::resetPointer(){
    pointer = 0;
    currentConfig = configList[pointer];
}

bool Configuration::nextValue(){
    if(pointer < numConfigs - 1){
        pointer++;
        currentConfig = configList[pointer];
        return true;
    }
    currentConfig = configList[pointer];
    pointer = 0;
    return false;
}

bool Configuration::setDefaults(){
//    Serial.println("I AM BEING CALLED");
    resetPointer();
    do {
        currentConfig->setDefault();
    } while (nextValue());
    saveConfiguration();
}

bool Configuration::loadConfiguration(){
    // Connect to FS, try and load config.txt, if no file return false
    // Read until 0x00, is length is different to TOTAL_CONFIG_LENGTH, we load what we can into setVal(so we overwrite default), then return false to save the rest
    bool intact = true;
    if(!SPIFFS.exists("/config.txt")){
        return false;
    }
    File f = SPIFFS.open("/config.txt", "r");
    if(!f)
        return false;
    
    resetPointer();
    char buf[LARGEST_CONF_VALUE];
    do {
        if(f.size() - f.position() < currentConfig->getLength()){
            // Config file is not big enough, lets move on.
            intact = false;
            currentConfig->setDefault(); // this will make sure, when we save, we have loaded the default values for the values we didn't have
            continue;
        }
        
        f.readBytes(buf, currentConfig->getLength());
        buf[currentConfig->getLength()] = 0x0;
        currentConfig->set(buf);
        memset(buf, 0, LARGEST_CONF_VALUE); //cleanup BUF
    } while (nextValue());
    f.close();
    return intact;
}

bool Configuration::saveConfiguration(){
    // Connect to FS, loop through configs and concat getPrintable into strings
    File f = SPIFFS.open("/config.txt", "w+");
    if(!f){
        return false;
    }
    resetPointer();
    unsigned char buf[SAVE_BUFFER];
    unsigned char* pos = &buf[0];
    int length = 0;
    do {
        length += currentConfig->getPrintable((char*)pos);
        pos = &buf[0] + length;
    } while(nextValue());
    
    buf[length+1] = 0x00;
    f.write(buf, length+1);
    f.close();
    return true;
}

bool Configuration::keyExists(char* key){
    if(findConfigWithKey(key))
        return true;
    return false;
}