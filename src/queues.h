#ifndef _QUEUES_H_
#define _QUEUES_H_

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "packet_t.h"

//BAS: should everything in this file be added to the PacketList class?

QueueHandle_t new_packet_queue_handle = NULL;

void initialize_queues() {
  new_packet_queue_handle = xQueueCreate(5, sizeof(Packet_t));
  if(new_packet_queue_handle == NULL) {
      /* One or more queues were not created successfully as there was not enough
      heap memory available.*/
      Serial.println("new_packet_queue_handle was not created successfully");
   }

  //send_to_influx_queue_handle = xQueueCreate(10, sizeof(whatever_this_is_a_queue_of));
}

/**
 * @brief Add a single packet to the new_packet_queue
 */

void add_packet_to_queue(Packet_t packet) {
    xQueueSend(new_packet_queue_handle, &packet, 100 / portTICK_RATE_MS);
}

bool read_packet_from_queue(Packet_t* packet) {
    return xQueueReceive(new_packet_queue_handle, &(*packet), 10) == pdPASS;
}

#endif // #ifndef _QUEUES_H_