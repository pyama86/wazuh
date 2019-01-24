/*
 * Wazuh Module for remote key requests
 * Copyright (C) 2015-2019, Wazuh Inc.
 * November 25, 2018.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "wmodules.h"
#include <os_net/os_net.h>
#include "shared.h"

#undef minfo
#undef mwarn
#undef merror
#undef mdebug1
#undef mdebug2

#define minfo(msg, ...) _mtinfo(WM_KEY_REQUEST_LOGTAG, __FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define mwarn(msg, ...) _mtwarn(WM_KEY_REQUEST_LOGTAG, __FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define merror(msg, ...) _mterror(WM_KEY_REQUEST_LOGTAG, __FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define mdebug1(msg, ...) _mtdebug1(WM_KEY_REQUEST_LOGTAG, __FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define mdebug2(msg, ...) _mtdebug2(WM_KEY_REQUEST_LOGTAG, __FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)

static void * wm_policy_monitoring_main(wm_policy_monitoring_t * data);   // Module main function. It won't return
static void wm_policy_monitoring_destroy(wm_policy_monitoring_t * data);  // Destroy data
static int wm_policy_monitoring_do_scan(wm_policy_monitoring_t * data);  // Do policy scan
cJSON *wm_policy_monitoring_dump(const wm_policy_monitoring_t * data);     // Read config

const wm_context WM_POLICY_MONITORING_CONTEXT = {
    PM_WM_NAME,
    (wm_routine)wm_policy_monitoring_main,
    (wm_routine)wm_policy_monitoring_destroy,
    (cJSON * (*)(const void *))wm_policy_monitoring_dump
};

typedef enum _request_type{
    W_TYPE_ID,W_TYPE_IP
} _request_type_t;

// Module main function. It won't return
void * wm_policy_monitoring_main(wm_policy_monitoring_t * data) {
    // If module is disabled, exit
    if (data->enabled) {
        minfo("Module started");
    } else {
        minfo("Module disabled. Exiting.");
        pthread_exit(NULL);
    }

    wm_policy_monitoring_do_scan(data);
    
    return NULL;
}

static int wm_policy_monitoring_do_scan(wm_policy_monitoring_t * data) {
    FILE *fp;
    int i = 0;

    /* Read every policy file */
    for(i = 0; data->profile[i]; i++) {
        char path[PATH_MAX];

        sprintf(path,"%s/%s",DEFAULTDIR ROOTCHECKCFG_DIR,data->profile[i]);

        fp = fopen(path,"r");

        if(!fp) {
            mwarn("Policy file not found '%s'",path);
            goto next;
        }

        /* Yaml parsing */
        yaml_document_t document;
        cJSON * object;
        char * json;

        if (yaml_parse_file(path, &document)) {
            merror("Policy file could not be parsed '%s'",path);
            goto next;
        }

        if (object = yaml2json(&document), !object) {
            merror("Transforming yaml to json '%s'",path);
            goto next;
        }

        yaml_document_delete(&document);
        json = cJSON_Print(object);
        minfo("JSON: %s",json);
next:
        if(fp){
            fclose(fp);
        }
       
    }
    
    return 0;
}

// Destroy data
void wm_policy_monitoring_destroy(wm_policy_monitoring_t * data) {
    os_free(data);
}

cJSON *wm_policy_monitoring_dump(const wm_policy_monitoring_t *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON *wm_wd = cJSON_CreateObject();
    cJSON_AddStringToObject(wm_wd,"enabled","yes");
    cJSON_AddStringToObject(wm_wd, "scan_on_start", data->scan_on_start ? "yes" : "no");
    cJSON_AddItemToObject(root,"policy-monitoring",wm_wd);
    return root;
}