#include "RS485_Wind_Speed_Transmitter.h"
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BROKER_ADDRESS "localhost"
#define BROKER_PORT 1883
#define MQTT_TOPIC "wind/sensor/speed"

int main() {
    char Address = 2;
    float speed;

    // Initialize Mosquitto library
    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Could not create Mosquitto client\n");
        return 1;
    }

    if (mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60)) {
        fprintf(stderr, "Error: Could not connect to MQTT broker\n");
        mosquitto_destroy(mosq);
        return 1;
    }
    printf("Connected to MQTT broker at %s:%d\n", BROKER_ADDRESS, BROKER_PORT);

    if (InitSensor("/dev/ttyUSB0") != 1) {
        printf("Failed to initialize serial port!\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    printf("Serial port initialized, waiting 1 second...\n");
    delayms(1000);

    if (!ModifyAddress(0, Address)) {
        printf("Address modification failed!\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    printf("Address modified successfully. Starting wind speed readings...\n");
    delayms(2000);

    while (1) {
        speed = readWindSpeed(Address);  // Replace with your actual speed-reading function
        if (speed >= 0) {
            printf("Wind Speed: %.2f m/s\n", speed);

            // Create JSON payload
            char payload[128];
            snprintf(payload, sizeof(payload), "{\"speed\": %.2f, \"unit\": \"m/s\"}", speed);

            // Publish to MQTT topic
            if (mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 0, false)) {
                fprintf(stderr, "Error: Failed to publish message\n");
            } else {
                printf("Published: %s\n", payload);
            }
        } else {
            fprintf(stderr, "Error reading sensor\n");
            break;
        }
        delayms(1000);  // Publish every second
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}