//#define SERIAL_EN

#ifdef SERIAL_EN
  #define SERIALBEGIN()   {Serial.begin(115200);}
  #define DEBUG(input)   {Serial.print(input);}
  #define DEBUGf(input, ...)   {Serial.printf(input, __VA_ARGS__);}
  #define DEBUGln(input) {Serial.println(input);}
  #define SERIALFLUSH() {Serial.flush();}
#else
  #define SERIALBEGIN();
  #define DEBUG(input);
  #define DEBUGf(input, ...);
  #define DEBUGln(input);
  #define SERIALFLUSH();
#endif