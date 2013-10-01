#include "Tx7Receiver.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

boolean Tx7Receiver::m_message[45];
unsigned int Tx7Receiver::m_nb_message = 0;
float Tx7Receiver::m_temperature = 0.0;
int Tx7Receiver::m_is_data_received = 0;
int Tx7Receiver::m_sensor_id = 0;
pthread_cond_t Tx7Receiver::m_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t Tx7Receiver::m_mutex = PTHREAD_MUTEX_INITIALIZER;

Tx7Receiver::Tx7Receiver(int gpioPin, int sensorId)
    : m_gpio_pin(gpioPin) {
    wiringPiISR(this->m_gpio_pin, INT_EDGE_BOTH, &handleInterrupt);
    m_sensor_id = sensorId;
}

void Tx7Receiver::waitForValue() {
  pthread_mutex_lock(&Tx7Receiver::m_mutex);
  while(!Tx7Receiver::available()) {
    pthread_cond_wait(&Tx7Receiver::m_cond, &Tx7Receiver::m_mutex);
  }
  pthread_mutex_unlock(&Tx7Receiver::m_mutex);
}

bool Tx7Receiver::available() {
  return this->m_is_data_received;
}

void Tx7Receiver::resetAvailable() {
  this->m_is_data_received = 0;
}

float Tx7Receiver::getReceivedValue() {
  pthread_mutex_lock(&Tx7Receiver::m_mutex);
  float value = Tx7Receiver::m_temperature;
  pthread_mutex_unlock(&Tx7Receiver::m_mutex);

  return value;
}

void Tx7Receiver::setReceivedValue(float value) {
  pthread_mutex_lock(&Tx7Receiver::m_mutex);
  Tx7Receiver::m_temperature = value;
  Tx7Receiver::m_is_data_received = 1;
  pthread_mutex_unlock(&Tx7Receiver::m_mutex);
  pthread_cond_broadcast(&Tx7Receiver::m_cond);
}

void Tx7Receiver::handleInterrupt() {

  static unsigned int duration;
  static unsigned int changeCount;
  static unsigned long lastTime;
  static unsigned int repeatCount;
  static unsigned int inProgress;
  static unsigned int expectingHigh;
  static unsigned int expectingLow;
  static unsigned int resetDuration;
  static unsigned int done;

  long m_time = micros();
  duration = m_time - lastTime;
  lastTime = m_time;

#ifdef DEBUG
if (done) {
  printf("After 44, duration = %d\n", duration);
  printf("%d %d\n", digitalRead(2), duration);
  done++;
  if (done == 5) {
    done = 0;
  }
}
#endif

  if (duration > 3000) {
    expectingHigh = 1;
    expectingLow = 0;
    Tx7Receiver::m_nb_message = 0;
    resetDuration = duration;
  } else {
    if (expectingHigh) {
      if (duration > 1400 * 0.6 && duration < 1400 * 1.5) {
        expectingHigh = 0;
        expectingLow = 1;
        inProgress = 0;
#ifdef DEBUG
if (Tx7Receiver::m_nb_message == 43) {
printf("Got High for 0\n");
}
#endif
        #ifdef DEBUG
        //printf("Got High for 0\n");
        #endif
      } else if (duration > 600 * 0.6 && duration < 600 * 1.5) {
        expectingHigh = 0;
        expectingLow = 1;
        inProgress = 1;
#ifdef DEBUG
if (Tx7Receiver::m_nb_message == 43) {
printf("Got High for 1\n");
}
#endif
        #ifdef DEBUG
        //printf("Got High for 1\n");
        #endif
      } else {
        expectingHigh = 0;
        expectingLow = 0;

        #ifdef DEBUG
        if ( Tx7Receiver::m_nb_message > 2 ) {
          time_t rawtime;
          struct tm * timeinfo;
          char buffer [80];

          rawtime = time(NULL);
          timeinfo = localtime (&rawtime);

          strftime (buffer,80,"%I:%M:%S.",timeinfo);

          printf("!!!Abortd after %d messages %s!!!\n", Tx7Receiver::m_nb_message, buffer);
        }
        #endif
      }
    } else if (expectingLow) {
      if (duration > 1000 * 0.6 && duration < 1000 * 1.5) {
        expectingHigh = 1;
        expectingLow = 0;
        Tx7Receiver::m_message[Tx7Receiver::m_nb_message] = inProgress;
        Tx7Receiver::m_nb_message++;
        #ifdef DEBUG
        //printf("Bit %d received OK\n", Tx7Receiver::m_nb_message);
        #endif
      } else {
       expectingHigh = 0;
       expectingLow = 0;
     }
    }

    // Message is complete, decode it
    if (Tx7Receiver::m_nb_message == 43) {
      int CRC=0;
      for (int i=0;i<=36;i+=4) {
        CRC+=Tx7Receiver::m_message[i]*8+Tx7Receiver::m_message[i+1]*4+Tx7Receiver::m_message[i+2]*2+Tx7Receiver::m_message[i+3];
      }

      CRC &= 0xF;
//      if (CRC && CRC==Tx7Receiver::m_message[40]*8+Tx7Receiver::m_message[41]*4+Tx7Receiver::m_message[42]*2+Tx7Receiver::m_message[43]) {
      if (1) {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer [80];

        rawtime = time(NULL);
        timeinfo = localtime (&rawtime);

        strftime (buffer,80,"%I:%M:%S.",timeinfo);

        float temperature=10*(Tx7Receiver::m_message[20]*8+Tx7Receiver::m_message[21]*4+Tx7Receiver::m_message[22]*2+Tx7Receiver::m_message[23]*1-5)+Tx7Receiver::m_message[24]*8+Tx7Receiver::m_message[25]*4+Tx7Receiver::m_message[26]*2+Tx7Receiver::m_message[27]+(Tx7Receiver::m_message[28]*8+Tx7Receiver::m_message[29]*4+Tx7Receiver::m_message[30]*2+Tx7Receiver::m_message[31])/10.0;
        int sensorId=10*(Tx7Receiver::m_message[12]*8+Tx7Receiver::m_message[13]*4+Tx7Receiver::m_message[14]*2+Tx7Receiver::m_message[15]*1)+Tx7Receiver::m_message[16]*4+Tx7Receiver::m_message[17]*2+Tx7Receiver::m_message[18];

        #ifdef DEBUG
        printf("sensorId = %d, temperature : %f, CRC=%d, reset duration=%d, time=%s\n", sensorId, temperature, CRC, resetDuration, buffer);
        #endif

        if (sensorId == Tx7Receiver::m_sensor_id) {
            Tx7Receiver::setReceivedValue(temperature);
    
            #ifdef DEBUG
            printf("CRC : expected %d, got %d\n", Tx7Receiver::m_message[40]*8+Tx7Receiver::m_message[41]*4+Tx7Receiver::m_message[42]*2+Tx7Receiver::m_message[43], CRC );
            for (int i=0;i<44;i++) {
              printf("%d", Tx7Receiver::m_message[i]);
              if ((i+1)%4==0)
                printf(" ");
            }
            printf("\n");
            #endif
    
            expectingHigh = 0;
            expectingLow = 0;
            Tx7Receiver::m_nb_message = 0;
    
            #ifndef DEBUG
            // We are not expecting another message for about 1 minute, we can sleep for 55 second to limit CPU usage
            sleep(55);
            #endif
    
            done = 1;
         } else {
            FILE *f = fopen("/tmp/weatherSensors.txt", "w");
            if (f == NULL) {
                fprintf(stderr, "Error opening sensor file!\n");
            }

            fprintf(f, "id=%d, value=%f\n", sensorId, temperature);

            fclose(f);
         }
       } else {
        #ifdef DEBUG
        printf("****************CRC failed*******************\n");
        printf("CRC failed : expected %d, got %d\n", Tx7Receiver::m_message[40]*8+Tx7Receiver::m_message[41]*4+Tx7Receiver::m_message[42]*2+Tx7Receiver::m_message[43], CRC );

        for (int i=0;i<44;i++) {
          printf("%d", Tx7Receiver::m_message[i]);
          if ((i+1)%4==0)
            printf(" ");
        }
        printf("\n");
        #endif

        expectingHigh = 0;
        expectingLow = 0;
        Tx7Receiver::m_nb_message = 0;
      }
    }
  }
}

