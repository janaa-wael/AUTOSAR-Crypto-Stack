#ifndef CSM_CFG_H
#define CSM_CFG_H

#include "CSM_Types.h"
#include <stdint.h>
#include <stddef.h>

/* ===================[Feature Switches]=================== */
#define CSM_DEV_ERROR_DETECT      (1u)  /* Enable/disable DET checks */
#define CSM_USE_ASYNC_API         (1u)  /* Support async job processing */
#define CSM_USE_KEY_STORE         (1u)  /* Key lookup by keyId */

/* ===================[Size Limits]=================== */
#define CSM_MAX_CHANNELS          (4u)
#define CSM_MAX_JOBS              (16u)
#define CSM_MAX_KEYS              (8u)
#define CSM_MAX_PRIMITIVES        (8u)

/* Default timeout for jobs in ms */
#define CSM_DEFAULT_JOB_TIMEOUT_MS (1000u)

/* ===================[Type Definitions]=================== */

/* Type for key identifiers */
typedef uint32_t Csm_KeyIdType;

/* Type for crypto driver identifiers */
typedef uint32_t Csm_PrimitiveDriverIdType;

/* Key configuration */
typedef struct {
    Csm_KeyIdType keyId;        /* Unique key ID */
    const uint8_t *keyPtr;      /* Pointer to key bytes (NULL if HW handled) */
    size_t keyLen;              /* Length of key in bytes */
    uint32_t usageFlags;        /* Usage rights (encrypt/hash/etc.) */
} Csm_KeyConfigType;

/* Primitive → driver mapping */
typedef struct {
    Csm_PrimitiveType primitive;        /* e.g. CSM_PRIM_SHA256 */
    Csm_PrimitiveDriverIdType driverId; /* Maps to crypto driver */
} Csm_PrimitiveMappingType;

/* Channel configuration */
typedef struct {
    Csm_ChannelType channelId;        /* Channel index */
    Csm_KeyIdType defaultKeyId;       /* Default key for channel */
    uint32_t maxConcurrentJobs;       /* Limit for jobs in this channel */
    uint32_t defaultTimeoutMs;        /* Timeout override */
} Csm_ChannelConfigType;

/* Top-level configuration container */
typedef struct {
    const Csm_ChannelConfigType *channels;
    uint8_t numChannels;

    const Csm_KeyConfigType *keys;
    uint8_t numKeys;

    const Csm_PrimitiveMappingType *primitiveMap;
    uint8_t numPrimitiveMap;

    uint32_t maxJobs;
} Csm_ConfigType;

/* ===================[Callback Type]=================== */

/* Completion notification callback (for async API) */
typedef void (*Csm_JobCompleteCallbackType)(
    Csm_JobIdType jobId,
    Std_ReturnType result,
    void *userCtx
);

/* ===================[Externs]=================== */

/* Global configuration (defined in Csm_Cfg.c) */
extern const Csm_ConfigType Csm_Config;

/* Application-set callback for job completion */
extern Csm_JobCompleteCallbackType Csm_JobCompleteCallback;

/* ===================[Helper Macros]=================== */
#define CSM_INVALID_KEY_ID   ((Csm_KeyIdType)0xFFFFFFFFu)
#define CSM_INVALID_JOB_ID   ((Csm_JobIdType)0xFFFFFFFFu)



#endif