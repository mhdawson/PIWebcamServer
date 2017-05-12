// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "PIWC.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

// define error codes
#define MISSING_BROKER             -1
#define MISSING_CERT_DIR           -2 
#define INVALID_CERT_DIR           -3 
#define MISSING_CERT_FILES         -4

#define MQTT_REQUEST_TOPIC_INDEX    1
#define MQTT_REPLY_TOPIC_INDEX      2
#define MQTT_PARAMTER_INDEX         3
#define MQTT_CERTS_PARAMTER_INDEX   4
#define MIN_PARAMTERS               4

#define NUM_REQUIRED_CERT_FILES 3 

int main(int argc, char *argv[]) {
   char* certsDir = NULL;

   // validate we got the required parameters 
   if (argc < MIN_PARAMTERS) {
      printf("Usage: PIWCMain requestTopic replyTopic mqtt_url <cert info dir>\n");
      return MISSING_BROKER;
   }

   if (strstr(argv[MQTT_PARAMTER_INDEX], "ssl://") == argv[MQTT_PARAMTER_INDEX]) {
      // certificates and keys 
      // the names of the files in the directory specified must be client.cert client.key and ca.cert
      if (argc < (MQTT_CERTS_PARAMTER_INDEX + 1)) {
         printf("Cert info dir required for connection to broker with ssl - exiting\n");
         printf("Usage: PI433Main mqtt_url <cert info dir>\n");
         return MISSING_CERT_DIR;
      }

      // validate the directory exists
      DIR* dirPtr = opendir(argv[MQTT_CERTS_PARAMTER_INDEX]);
      if(NULL == dirPtr) {
         printf("Cert info dir was invalid - exiting\n");
         return INVALID_CERT_DIR;
      } 
    
      struct dirent *nextEntry;
      int requiredFilesFound = 0;
      while ((nextEntry = readdir(dirPtr)) != NULL) {
         if ((strcmp(nextEntry->d_name, CA_CERT_FILE) == 0) ||
             (strcmp(nextEntry->d_name, CLIENT_CERT_FILE) == 0) ||
             (strcmp(nextEntry->d_name, CLIENT_KEY_FILE) == 0)) {
            requiredFilesFound++;
         }
      }
      closedir(dirPtr);

      if (requiredFilesFound != NUM_REQUIRED_CERT_FILES) { 
         printf("Missing cert info files, directory must contain client.key, client.cert and ca.cert\n");
         return MISSING_CERT_FILES;
      }

      certsDir = argv[MQTT_CERTS_PARAMTER_INDEX];
   }

   PIWC cameraService = PIWC(argv[MQTT_PARAMTER_INDEX], certsDir);
   cameraService.listenForMessages(argv[MQTT_REQUEST_TOPIC_INDEX],
                                   argv[MQTT_REPLY_TOPIC_INDEX]);
}
