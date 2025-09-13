#include "Csm_Types.h"
#include "Csm_Cfg.h"

/* === Primitive mappings (logical → driver) === */
static const Csm_PrimitiveMappingType Csm_Primitives[] = {
    { CSM_PRIM_SHA256, 1u },   /* SHA256 → driver 1 */
    { CSM_PRIM_AES_ECB, 2u }   /* AES-ECB → driver 2 */
};

/* === Channels (job queues) === */
static const Csm_ChannelConfigType Csm_Channels[] = {
    { 0u, CSM_INVALID_KEY_ID, 2u, CSM_DEFAULT_JOB_TIMEOUT_MS } /* Channel 0 */
};

/* === Keys (key store) === */
static const Csm_KeyConfigType Csm_Keys[] = {
    { 1u, NULL, 16u, 0u }   /* Key ID 1, 16 bytes, usageFlags=0 */
};

/* === Top-level CSM config === */
const Csm_ConfigType Csm_Config = {
    .channels = Csm_Channels,
    .numChannels = sizeof(Csm_Channels) / sizeof(Csm_ChannelConfigType),

    .keys = Csm_Keys,
    .numKeys = sizeof(Csm_Keys) / sizeof(Csm_KeyConfigType),

    .primitiveMap = Csm_Primitives,
    .numPrimitiveMap = sizeof(Csm_Primitives) / sizeof(Csm_PrimitiveMappingType),

    .maxJobs = CSM_MAX_JOBS
};
