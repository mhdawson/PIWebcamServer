// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include "PIWC.h"

#define MQTT_CONNECT_RETRY_SECONDS 5
#define QOS_AT_MOST_ONCE_0 0
#define TEMP_BUFFER_SIZE 1000


PIWC::PIWC(char* mqttBroker) : _mqttOptions(MQTTClient_connectOptions_initializer) {
   this->_myClient = NULL;
   this->_topic = NULL;
   this->_mqttBroker = mqttBroker;
}


int PIWC::messageArrived(void *context, char* topicName, int topicLen, MQTTClient_message *message) {
   PIWC* myThis = (PIWC*)context;
   char buffer[TEMP_BUFFER_SIZE];
   int limit = TEMP_BUFFER_SIZE -1;
   if (message->payloadlen < limit) { 
      limit = message->payloadlen;
   }

   memset(buffer, 0, TEMP_BUFFER_SIZE);
   strncpy(buffer, (char*) message->payload, limit);

   myThis->takePicture();

   MQTTClient_freeMessage(&message);

   return 1;
}

void PIWC::connectionLost(void *context, char *cause) {
   PIWC* myThis = (PIWC*)context;

   while (true) {
      if (MQTTCLIENT_SUCCESS == MQTTClient_connect(myThis->_myClient, &(myThis->_mqttOptions))) {
         MQTTClient_subscribe(myThis->_myClient, myThis->_topic, QOS_AT_MOST_ONCE_0);
         break;
      }
      // wait 5 seconds and then try to reconnect
      usleep(MQTT_CONNECT_RETRY_SECONDS*1000*1000);
   }
}

void PIWC::listenForMessages(char* topic) {
   this->_topic = (char*) malloc(strlen(topic) + 1);
   if (NULL == this->_topic) {
      printf("Error, failed to allocation string for topic\n");
      return;
   }
   strcpy(this->_topic, topic);

   MQTTClient_create(&_myClient, _mqttBroker, "PICameraSevice", MQTTCLIENT_PERSISTENCE_NONE, (void*) this);
   MQTTClient_setCallbacks(_myClient, (void*) this, &(PIWC::connectionLost), &(PIWC::messageArrived), NULL);

   _mqttOptions.keepAliveInterval = 10;
   _mqttOptions.cleansession = 1;
   _mqttOptions.reliable = 0;

   int connectResult = MQTTClient_connect(_myClient, &_mqttOptions);
   if(MQTTCLIENT_SUCCESS != connectResult) {
      printf("Failed to connect to MQTT broker, exiting\n");
      return;
   }

   MQTTClient_subscribe(_myClient, this->_topic, QOS_AT_MOST_ONCE_0);

   while(1) {
      usleep(1000 * 1000);
      MQTTClient_yield();
   }
}

void PIWC::takePicture() {
   char buffer[1000];
   long long pictureId = time(NULL);
   sprintf(buffer, "./takepicture.sh %lu.jpg", pictureId);
   system(buffer);
   sprintf(buffer, "%lu.jpg", pictureId); 
   int result = MQTTClient_publish(_myClient, "house/camera/newpicture", strlen(buffer), buffer, 0, false, NULL);
   if (MQTTCLIENT_SUCCESS != result) {
      printf("publish failed:%d\n",result);
   }
}
