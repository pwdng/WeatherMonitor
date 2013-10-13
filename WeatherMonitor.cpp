/*
  RF_Sniffer
  
  Hacked from http://code.google.com/p/rc-switch/
  
  by @justy to provide a handy RF code sniffer
*/

#include "Tx17Receiver.h"
#include "HttpNotifier.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

// Temperature should not change by more than 5 degrees between 2 readings.
#define INCREMENT_LIMIT 5

int main(int argc, char *argv[]) {
    // Parse arguments
    char* pid_file = NULL;
    char* server_url = NULL;
    int gpio_pin = 0;
    int probe_id = 0;
    int c;
    int arg_error = 0;
#ifdef DEBUG
    while ((c = getopt(argc, argv, "n:i:")) != -1)
#else
    while ((c = getopt(argc, argv, "p:u:n:i:")) != -1)
#endif
        switch (c) {
            case 'p':
                pid_file = optarg;
                break;
            case 'u':
                server_url = optarg;
                break;
            case 'n':
                gpio_pin = atoi(optarg);
                break;
            case 'i':
                probe_id = atoi(optarg);
                break;
            case '?':
                arg_error = 1;
        }
     
#ifdef DEBUG
    if (!probe_id || arg_error) {
        fprintf(stderr, "Expecting options -n 'GPIO pin number' -i 'Probe ID'\n");
        return 1;
    }
#else
    if (!pid_file || !server_url || !probe_id || arg_error) {
        fprintf(stderr, "Expecting options -p 'PID filename' -u 'Web service URL' -n 'GPIO pin number' -i 'Probe ID'\n");
        return 1;
    }
#endif

    /* Write pid to file. This is of the daemon status */
    FILE *f = fopen(pid_file, "w");
    if (f != NULL) {
        int pid = getpid();
        fprintf(f, "%d\n", pid);
        fclose(f);
    }

    if(wiringPiSetup() == -1) {
        fprintf(stderr, "Could not initialize GPIO\n");
        return 0;
    }

    Tx17Receiver tx17_receiver(gpio_pin, probe_id);

#ifndef DEBUG
    HttpNotifier http_notifier(server_url);
#endif

    int starting = 1;
    float last_value = 0.0;
    float last_recorded_value = 0.0;
    float value = 0.0;
    time_t last_recorded_time, current_time;
    
    time(&last_recorded_time);
   
    while(1) {
  
        tx17_receiver.waitForValue();
#ifdef DEBUG
        printf("Value available...\n");
#endif
        if (tx17_receiver.available()) {
    
            time(&current_time);
            value = tx17_receiver.getReceivedValue();

            if (!starting && fabs(last_value - value) > INCREMENT_LIMIT) {
                // Ignore value as it is probaby bogus
                // CRC should catch these errors, but CRC is broken
#ifdef DEBUG
                printf("Received bogus value: %d\n", value);
#endif
            } else {
                if ((fabs(value - last_recorded_value) >= 0.5) ||
                    (difftime(current_time, last_recorded_time) > 3600) ||
                    starting) {
                    // Update server if delta >= 0.5 degress of last update was over 1 hour ago
                    // or if service is starting
                    last_recorded_time = current_time;
                    last_recorded_value = value;

#ifdef DEBUG
                    printf("Sending %f to server\n", value);
#endif

#if defined(SEND_DATA) && !defined(DEBUG)
                    http_notifier.sendValue(value);
#endif

#ifdef WRITE_TO_FILE
                    FILE *f = fopen("/tmp/temperature.txt", "w");
                    if (f == NULL) {
                        printf("Error opening output file!\n");
                    }

                    fprintf(f, "reading=%f\n", value);

                    fclose(f);
#endif
                } 
                // Update last value
                last_value = value;
            }
 
            tx17_receiver.resetAvailable();
        }

        starting = 0;
    }
    return 0;
}

