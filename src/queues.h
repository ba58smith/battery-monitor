#ifndef _QUEUES_H_
#define _QUEUES_H_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "packet_t.h"

QueueHandle_t new_packet_queue_handle = NULL;
QueueHandle_t send_to_influx_queue_handle = NULL;

void initialize_queues() {
  new_packet_queue_handle = xQueueCreate(5, sizeof(Packet_t));
  if(new_packet_queue_handle == NULL) {
      /* The queue was not created successfully as there was not enough
      heap memory available.*/
      Serial.println("new_packet_queue_handle was not created successfully");
   }

  send_to_influx_queue_handle = xQueueCreate(10, sizeof(Packet_t));
  if(send_to_influx_queue_handle == NULL) {
      /* The queue was not created successfully as there was not enough
      heap memory available.*/
      Serial.println("send_to_influx_queue_handle was not created successfully");
   }
}

/**
 * @brief Add a single packet to the new_packet_queue
 */

void add_packet_to_queue(Packet_t packet) {
    xQueueSend(new_packet_queue_handle, &packet, 100 / portTICK_RATE_MS);
}

/**
 * @brief Read a single packet from the new_packet_queue
 */

bool read_packet_from_queue(Packet_t* packet) {
    return xQueueReceive(new_packet_queue_handle, &(*packet), 10) == pdPASS;
}

/**
 * @brief Add a single packet to the influx_queue
 */

void add_packet_to_influx_queue(Packet_t packet) {
    xQueueSend(send_to_influx_queue_handle, &packet, 100 / portTICK_RATE_MS);
}

/**
 * @brief Read a single packet from the influx_queue
 */

bool read_packet_from_influx_queue(Packet_t* packet) {
    return xQueueReceive(send_to_influx_queue_handle, &(*packet), 10) == pdPASS;
}

#endif // #ifndef _QUEUES_H_