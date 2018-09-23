/*

    ESP Arduino Core Configuration Library
    Allows storing or variables to the Flash in a convenient API
    Maximum variable sizes are as follows:
    
        Type            Size (bytes)
        
        charArray       20
        boolean         1
        ipAddress       15
        
        
*/
#ifndef Configuration_h
#define Configuration_h
#include <Arduino.h>
#include "ConfigurationItem.h"

//#define TOTAL_CONFIG_LENGTH 336
//#define NUM_CONFIGS 19

#define SAVE_BUFFER 1000

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
        const char* blank = "";
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

#endif



