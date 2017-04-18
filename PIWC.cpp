// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "PIWC.h"

#define MQTT_CONNECT_RETRY_SECONDS 5
#define QOS_AT_MOST_ONCE_0 0
#define TEMP_BUFFER_SIZE 1000
#define SEPARATOR "/"


MQTTClient_connectOptions mqttOptions_init = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions sslOptions_init = MQTTClient_SSLOptions_initializer ;

static void* takePictureThread(void* context) {
   PIWC* myThis = (PIWC*) context;
   myThis->takePictureLoop();
}

PIWC::PIWC(char* mqttBroker, char* certsDir) : _takePictureCond(PTHREAD_COND_INITIALIZER),
                                               _takePictureMutex(PTHREAD_MUTEX_INITIALIZER) {
   this->_myClient = NULL;
   this->_topic = NULL;
   this->_mqttBroker = mqttBroker;
   _certsDir = certsDir;
   memcpy(&_mqttOptions, &mqttOptions_init, sizeof(mqttOptions_init));
   memcpy(&_sslOptions, &sslOptions_init, sizeof(sslOptions_init));;


   if (strstr(_mqttBroker, "ssl://") == _mqttBroker) {
      // ssl is enabled so setup the options
      _mqttOptions.ssl = &_sslOptions;
      _sslOptions.trustStore = (char*) malloc(strlen(_certsDir) + strlen(SEPARATOR) + strlen(CA_CERT_FILE) + 1);
      _sslOptions.keyStore   = (char*) malloc(strlen(_certsDir) + strlen(SEPARATOR) + strlen(CLIENT_CERT_FILE) + 1);
      _sslOptions.privateKey   = (char*) malloc(strlen(_certsDir) + strlen(SEPARATOR) + strlen(CLIENT_KEY_FILE) + 1);
      strcpy((char*) _sslOptions.trustStore, _certsDir);
      strcpy((char*) _sslOptions.keyStore, _certsDir);
      strcpy((char*) _sslOptions.privateKey, _certsDir);
      strcat((char*) _sslOptions.trustStore, SEPARATOR);
      strcat((char*) _sslOptions.keyStore, SEPARATOR);
      strcat((char*) _sslOptions.privateKey, SEPARATOR);
      strcat((char*) _sslOptions.trustStore, CA_CERT_FILE);
      strcat((char*) _sslOptions.keyStore, CLIENT_CERT_FILE);
      strcat((char*) _sslOptions.privateKey, CLIENT_KEY_FILE);

      _sslOptions.enabledCipherSuites = "TLSv1.2";
   } else {
      _mqttOptions.ssl = NULL;
   }

   // It appears that if too much time is spent in the callback before we try to publish the response
   // then we don't get the message even on the topic even though the client tells us it was sent ok
   // We therefore start a new thead to do the work for us.  
   pthread_t threadId = 0;
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   int result = pthread_create(&threadId, &attr, &takePictureThread, (void*) this); 
   if (0 != result) { 
      printf("Failed to start thread: %d\n", result);
   }
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

   // request that the next picture be taken
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
   pthread_mutex_lock(&_takePictureMutex);
   pthread_cond_signal(&_takePictureCond);
   pthread_mutex_unlock(&_takePictureMutex);
}

void PIWC::takePictureLoop() {
   char buffer[1000];
   while (true) {
      // wait until we are signaled to take the next picture
      pthread_mutex_lock(&_takePictureMutex);
      pthread_cond_wait(&_takePictureCond, &_takePictureMutex);
      pthread_mutex_unlock(&_takePictureMutex);

      // now take the picture and publish when complete
      long long pictureId = time(NULL);
      sprintf(buffer, "./takepicture.sh %lu.jpg", pictureId);
      system(buffer);
      sprintf(buffer, "%lu.jpg", pictureId); 
      int result = MQTTClient_publish(_myClient, "house/camera/newpicture", strlen(buffer), buffer, 0, false, NULL);
      if (MQTTCLIENT_SUCCESS != result) {
         printf("publish failed:%d\n",result);
      }
   }
}
