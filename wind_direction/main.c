#include "RS485_Wind_Speed_Transmitter.h"
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BROKER_ADDRESS "localhost"
#define BROKER_PORT 1883
#define MQTT_TOPIC "wind/sensor/data"

// Convert angle to direction
const char* getWindDirection(float angle) {
    if (angle < 0) return "Invalid";

    // Normalize the angle to 0-360 range
    while (angle >= 360.0) angle -= 360.0;

    if (angle >= 348.75 || angle < 11.25)  return "North";
    if (angle >= 11.25 && angle < 33.75)   return "North-Northeast";
    if (angle >= 33.75 && angle < 56.25)   return "Northeast";
    if (angle >= 56.25 && angle < 78.75)   return "East-Northeast";
    if (angle >= 78.75 && angle < 101.25)  return "East";
    if (angle >= 101.25 && angle < 123.75) return "East-Southeast";
    if (angle >= 123.75 && angle < 146.25) return "Southeast";
    if (angle >= 146.25 && angle < 168.75) return "South-Southeast";
    if (angle >= 168.75 && angle < 191.25) return "South";
    if (angle >= 191.25 && angle < 213.75) return "South-Southwest";
    if (angle >= 213.75 && angle < 236.25) return "Southwest";
    if (angle >= 236.25 && angle < 258.75) return "West-Southwest";
    if (angle >= 258.75 && angle < 281.25) return "West";
    if (angle >= 281.25 && angle < 303.75) return "West-Northwest";
    if (angle >= 303.75 && angle < 326.25) return "Northwest";
    if (angle >= 326.25 && angle < 348.75) return "North-Northwest";

    return "Invalid";
}

int main() {
    char Address = 2;
    float angle;

    // Initialize Mosquitto library
    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Could not create Mosquitto client\n");
        return 1;
    }

    if (mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60)) {
        fprintf(stderr, "Error: Could not connect to MQTT broker\n");
        return 1;
    }
    printf("Connected to MQTT broker at %s:%d\n", BROKER_ADDRESS, BROKER_PORT);

    if (InitSensor("/dev/ttyUSB1") != 1) {
        printf("Failed to initialize serial port!\n");
        return 1;
    }

    printf("Serial port initialized, waiting 1 second...\n");
    delayms(1000);

    if (!ModifyAddress(0, Address)) {
        printf("Address modification failed!\n");
        return 1;
    }

    printf("Address modified successfully. Starting readings...\n");
    delayms(2000);

    while (1) {
        angle = readWindSpeed(Address);
        if (angle >= 0) {
            const char* direction = getWindDirection(angle);
            printf("Angle: %.1f° | Direction: %s\n", angle, direction);

            // Create JSON payload
            char payload[128];
            snprintf(payload, sizeof(payload), "{\"angle\": %.1f, \"direction\": \"%s\"}", angle, direction);

            // Publish to MQTT topic
            if (mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 0, false)) {
                fprintf(stderr, "Error: Failed to publish message\n");
            } else {
                printf("Published: %s\n", payload);
            }
        } else {
            printf("Error reading sensor\n");
            break;
        }
        delayms(1000);
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}