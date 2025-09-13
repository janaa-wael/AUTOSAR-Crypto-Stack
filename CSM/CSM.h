#ifndef CSM_H
#define CSM_H

#include "Csm_Types.h"
#include "Csm_Cfg.h"

/* ===================[API Declarations]=================== */
Std_ReturnType Csm_Init(const Csm_ConfigType *config);
Std_ReturnType Csm_DeInit(void);
Std_ReturnType Csm_StartJob(const Csm_JobType *job);
Std_ReturnType Csm_CancelJob(Csm_JobIdType jobId);
void           Csm_SetJobCompleteCallback(Csm_JobCompleteCallbackType cb);
void Csm_MainFunction(void);
Std_ReturnType Csm_GetJobState(Csm_JobIdType jobId, Csm_JobState *state);
Std_ReturnType Csm_GetJobResult(Csm_JobIdType jobId, uint8_t *outBuf, size_t *outLen);


#endif /* CSM_H */