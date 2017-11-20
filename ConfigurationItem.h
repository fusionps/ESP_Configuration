#ifndef ConigurationIten_h
#define ConfigurationItem_h

#define LARGEST_CONF_VALUE 20
#define MAX_STR 20


class ConfigurationItem {
    public:
        ConfigurationItem(const char* key, unsigned char length): _length(length), key(key){}
        virtual int getPrintable(char* buf) =0;
        virtual bool setValue(void* value) =0; // This sets by passing in instance of object
        virtual bool set(char* str) =0; // This set by passing in char* - i.e. what is saved on flash
        virtual void* get() =0;
        virtual void setDefault() =0;
        String toString();
        unsigned char getLength(){ return _length; };
        const char* key;
    protected:
        unsigned char _length; // Serialised version
        bool _initialised = false;
};

String ConfigurationItem::toString(){
    char buf[LARGEST_CONF_VALUE];
    this->getPrintable(buf);
    return String(buf);
}

class IPAddressConf: public ConfigurationItem {
    public:
        IPAddressConf(const char* key, IPAddress defaultIP): _default(defaultIP), ConfigurationItem(key, 15){}
        virtual bool setValue(void* value){
            _value = IPAddress();
            if(_value.fromString((*(IPAddress*)value).toString())){
                _initialised = true;
                return true;
            }
            return false;
        }
        virtual bool set(char* str){
            _value = IPAddress();
            if(_value.fromString(str)){
                _initialised = true;
                return true;
            }
            return false;
                
        }
        virtual IPAddress* get(){
            if(!_initialised){
                return &_default;
            }
            return &_value;
        }
        virtual int getPrintable(char* buf){
            IPAddress* activeIP;
            if(!_initialised)
                activeIP = &_default;
            else
                activeIP = &_value;
            sprintf(buf,"%03u.%03u.%03u.%03u", (*activeIP)[0], (*activeIP)[1], (*activeIP)[2], (*activeIP)[3]);
            return _length;
        }
        virtual void setDefault(){
            _value = _default;
        }
    protected:
        IPAddress _default, _value;
};

class EnumConfig: public ConfigurationItem {
    public:
        EnumConfig(const char* key, int defaultValue): _default(defaultValue), ConfigurationItem(key, 10){} // Stores as HEX or int (4 bytes ascii)
        virtual bool setValue(void* value){
            _value = *(int*)value;
            _initialised = true;
            return true;
        }
        virtual bool set(char* str){
            _value = (int)strtol(str, NULL, 0);
            _initialised = true;
            return true;
        }
        virtual int* get(){
            if(_initialised)
                return &_value;
            else
                return &_default;
        }
        virtual int getPrintable(char* buf){
            if(_initialised)
                return sprintf(buf, "0x%08x", _value);
            else
                return sprintf(buf, "0x%08x", _default);
        }
        virtual void setDefault(){
            _value = _default;
        }
    private:
        int _default, _value;
};

class CharArray: public ConfigurationItem {
    public:
        CharArray(const char* key, const char* defaultValue): ConfigurationItem(key, MAX_STR){
            strncpy(_default, defaultValue, _length);
        }
        virtual int getPrintable(char* buf){
            strncpy(buf, _value, _length);
            return _length;
        }
        virtual bool setValue(void* value){
            strncpy(_value, (char*)value, _length);
            _value[MAX_STR] = 0;
            return true;
        }
        virtual bool set(char* str){
            strncpy(_value, str, _length);
            return true;
        }
        virtual void* get(){
            return _value;
        }
        virtual void setDefault(){
            strncpy(_value, _default, _length);
            return;
        }
    
    
    private:
        char _value[MAX_STR];
        char _default[MAX_STR];
};

class Boolean: public ConfigurationItem {
    public:
        Boolean(const char* key, bool defaultValue): _default(defaultValue), ConfigurationItem(key, 1){}
        virtual int getPrintable(char* buf){
            sprintf(buf, "%d", _value);
            return 1;
        }
        virtual bool setValue(void* value){
            _initialised = true;
            _value = *(bool*)value;
            return true;
        }
        virtual bool set(char* str){
            _initialised = true;
            if(str[0] == '1')
                _value = true;
            else
                _value = false;
            return true;
        }
        virtual bool* get(){
            if(_initialised)
                return &_value;
            else 
                return &_default;
        }
        virtual void setDefault(){
            _value = _default;
        }
    
    
    private:
        bool _value;
        bool _default;
};

#endif