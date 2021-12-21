# esp8266_general
General, non-specific classes for ESP8266 (possibly usable for other Arduino-class boards).

Classes available:
* PersistentBuffer: This class provides a buffer that is allocated to the largest size ever requested. It is intended to be used in your `loop` code, so that you can automatically have the largest buffer needed on the heap without needing to resize or reallocate every time. Usage:
  ```static ::grmcdorman::PersistentBuffer<char> buffer;
     buffer.reserve(required_size);
     // Use buffer.get(); 
  ```
