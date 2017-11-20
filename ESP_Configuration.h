/*

    ESP Arduino Core Configuration Library
    Allows storing or variables to the Flash in a convenient API
    Maximum variable sizes are as follows:
    
        Type            Size (bytes)
        
        charArray       20
        boolean         1
        ipAddress       15
        
        
*/
#ifndef Coniguration_h
#define Configuration_h
#include <Arduino.h>
#include "ConfigurationItem.h"

//#define TOTAL_CONFIG_LENGTH 336
//#define NUM_CONFIGS 19

#define SAVE_BUFFER 1000

#if defined(ESP32)
    #include "nvs.h"    

    const char * nvs_errors[] = { "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENTGH"};
    #define nvs_error(e) (((e)>ESP_ERR_NVS_BASE)?nvs_errors[(e)&~(ESP_ERR_NVS_BASE)]:nvs_errors[0])

#endif


class Configuration {
    public:
        Configuration(ConfigurationItem** list, size_t num){
            configList = list;
            numConfigs = num;
        };
        
        void init();
        void* getConfiguration(const char*);
        bool setConfiguration(const char*, void*);
        bool saveConfiguration();
        bool loadConfiguration(); // save and load current instances of configs
        bool setDefaults();
        bool keyExists(char*);
    private: 
    
        ConfigurationItem** configList = 0;
        ConfigurationItem* findConfigWithKey(const char*);
    
        unsigned char pointer = 0;
        ConfigurationItem* currentConfig = 0;
    
        bool resetPointer();
        bool nextValue();
        size_t numConfigs;
    
        uint32_t _handle;
        char stuff[100];
};


void Configuration::init(){
    // Here is where we handle loadingSavedConfiguration or saving Default
#if defined(ESP32)
    esp_err_t err = nvs_open("config", NVS_READWRITE, &_handle);
    if(err){
        sprintf(stuff, "nvs_open failed: %s", nvs_error(err));
        Serial.println(stuff);
//        return false;
    } else {
        Serial.println("Opened thing");
    }
#else
    SPIFFS.begin();
#endif
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

void* Configuration::getConfiguration(const char* key){
    return findConfigWithKey(key)->get();
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

#if defined(ESP32)
bool Configuration::loadConfiguration(){
    // Connect to FS, try and load config.txt, if no file return false
    // Read until 0x00, is length is different to TOTAL_CONFIG_LENGTH, we load what we can into setVal(so we overwrite default), then return false to save the rest
    bool intact = true;
    size_t len = 1000;
    char value[SAVE_BUFFER];
    esp_err_t err = nvs_get_blob(_handle, "config", value, &len);
    if(err){
        memset(stuff, 0, 100);
        sprintf(stuff, "nvs_get_str len fail: %s %s", "config", nvs_error(err));
        Serial.println(stuff);
        return 0;
    } else {
        Serial.println("Read string: ");Serial.println(value);
    }
    int pos = 0;
    resetPointer();
    char buf[LARGEST_CONF_VALUE];
    do {
        if(len - pos < currentConfig->getLength()){
            // Config file is not big enough, lets move on.
            intact = false;
            currentConfig->setDefault(); // this will make sure, when we save, we have loaded the default values for the values we didn't have
            continue;
        }
        
        for(int i = 0; i < currentConfig->getLength(); i++){
            buf[i] = value[pos++];
        }
        
        buf[currentConfig->getLength()] = 0x0;
        currentConfig->set(buf);
        memset(buf, 0, LARGEST_CONF_VALUE); //cleanup BUF
    } while (nextValue());
    
    return intact;
}

bool Configuration::saveConfiguration(){
    // Connect to FS, loop through configs and concat getPrintable into strings
    
    resetPointer();
    char buf[SAVE_BUFFER];
    char* pos = &buf[0];
    int length = 0;
    do {
        length += currentConfig->getPrintable((char*)pos);
        pos = &buf[0] + length;
    } while(nextValue());
    
    buf[length+1] = 0x00;
    
    esp_err_t err = nvs_set_blob(_handle, "config", buf, length+1);
    if(err){
        memset(stuff, 0, 100);
        sprintf(stuff, "nvs_set_str fail: %s %s", "config", nvs_error(err));
        Serial.println(stuff);
        return 0;
    }
    err = nvs_commit(_handle);
    if(err){
        memset(stuff, 0, 100);
        sprintf(stuff, "nvs_commit fail: %s %s", "config", nvs_error(err));
        Serial.println(stuff);
        return 0;
    }
    return true;
}

#else

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
    
#endif

bool Configuration::keyExists(char* key){
    if(findConfigWithKey(key))
        return true;
    return false;
}

#endif



