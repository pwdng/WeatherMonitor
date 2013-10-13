#ifndef _Tx17Receiver_h
#define _Tx17Receiver_h

#include <wiringPi.h>
#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C"{
#endif
typedef uint8_t boolean;
typedef uint8_t byte;

#ifdef __cplusplus
}
#endif


class Tx17Receiver {

  public:
    Tx17Receiver(int gpioPin, int sensorId);
    
    bool available();
    void resetAvailable();
	
    void waitForValue();
    float getReceivedValue();

  private:
    int m_gpio_pin;

    static int m_sensor_id;

    static void handleInterrupt();
    static void setReceivedValue(float value);

    static float m_temperature;
    static int m_is_data_received;

    static boolean m_message[45];
    static unsigned int m_nb_message;

    static pthread_cond_t  m_cond;
    static pthread_mutex_t m_mutex;
    
};

#endif
