#ifndef CSM_TYPES_H
#define CSM_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint32_t Csm_JobIdType;
typedef uint8_t Csm_ChannelType;
/* Return Codes */
typedef enum {
E_NOT_OK = -1,
E_OK,
E_PENDING,
E_BUSY
} Std_ReturnType;

/* Modes */
typedef enum {
CSM_MODE_SINGLE=0, CSM_MODE_START, CSM_MODE_UPDATE, CSM_MODE_FINISH
} Csm_OperationMode;

typedef enum {
CSM_SERVICE_HASH=1, CSM_SERVICE_ENCRYPT
} Csm_ServiceType;

typedef enum {
CSM_PRIM_SHA256=1, CSM_PRIM_AES_ECB
} Csm_PrimitiveType;

typedef struct {
    Csm_JobIdType jobId;
    Csm_ChannelType channelID;
    Csm_ServiceType service;
    Csm_PrimitiveType primitive;
    Csm_OperationMode mode;
    const uint8_t* input;
    size_t inputLen;
    uint8_t* output;
    size_t outputLen;
    uint32_t keyID;
    bool async;
    void* userCtx;
} Csm_JobType;

typedef enum {
    JOB_IDLE, JOB_STARTED, JOB_PENDING, JOB_RUNNING, JOB_DONE, JOB_ERROR
} Csm_JobState;

typedef struct {
    Csm_JobType job;
    Csm_JobState state;
    void* driverCtx;
    uint64_t timeout_ts;
} Csm_JobRecord;

#endif