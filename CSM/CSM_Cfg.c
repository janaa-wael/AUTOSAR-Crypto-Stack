#include "Csm_Types.h"
#include "Csm_Cfg.h"

/* ===================[Static State]=================== */
static bool Csm_Initialized = false;
static Csm_JobRecord Csm_JobTable[CSM_MAX_JOBS];
static Csm_JobCompleteCallbackType Csm_JobCb = NULL;
static const Csm_ConfigType *Csm_ConfigPtr = NULL;

/* ===================[API Implementation]=================== */

/* Initialize CSM with a configuration */
Std_ReturnType Csm_Init(const Csm_ConfigType *config)
{
    if (config == NULL)
    {
        return E_NOT_OK;
    }

    Csm_ConfigPtr = config;

    for (uint32_t i = 0; i < Csm_ConfigPtr->maxJobs; i++)
    {
        Csm_JobTable[i].state = JOB_IDLE;
        Csm_JobTable[i].driverCtx = NULL;
        Csm_JobTable[i].timeout_ts = 0;
    }

    Csm_Initialized = true;
    return E_OK;
}

/* Deinitialize the CSM */
Std_ReturnType Csm_DeInit(void)
{
    if (Csm_ConfigPtr == NULL)
    {
        return E_NOT_OK;
    }

    Csm_ConfigPtr = NULL;

    Csm_Initialized = false;
    return E_OK;
}

Std_ReturnType Csm_StartJob(const Csm_JobType *job)
{
    if (!Csm_Initialized || job == NULL) {
        return E_NOT_OK;
    }

    /* Find a free job slot */
    int slot = -1;
    for (uint32_t i = 0; i < CSM_MAX_JOBS; i++) {
        if (Csm_JobTable[i].state == JOB_IDLE) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        return E_BUSY;  /* No free slots */
    }

    /* Copy job into slot */
    Csm_JobTable[slot].job = *job;
    Csm_JobTable[slot].state = JOB_RUNNING;

    Std_ReturnType result = E_NOT_OK;

    /* === Look up primitive mapping from configuration === */
    if (Csm_ConfigPtr != NULL) 
    {
        for (uint32_t i = 0; i < Csm_ConfigPtr->numPrimitiveMap; i++) 
        {
            if (Csm_ConfigPtr->primitiveMap[i].primitive == job->primitive) 
            {
                Csm_PrimitiveDriverIdType driverId = Csm_ConfigPtr->primitiveMap[i].driverId;

                /* Dispatch to CryIf (stub for now, replace with real driver calls) */
                result = CryIf_ProcessPrimitive(driverId, job);

                break;
            }
        }
    }

    /* Update state */
    if (result == E_OK) 
    {
        Csm_JobTable[slot].state = JOB_DONE;
    } 
    else if (result == E_PENDING) 
    {
        Csm_JobTable[slot].state = JOB_PENDING;
    } 
    else 
    {
        Csm_JobTable[slot].state = JOB_ERROR;
    }

    /* Notify application immediately if not async */
    if (Csm_JobCb && result != E_PENDING) 
    {
        Csm_JobCb(job->jobId, result, job->userCtx);
    }

    return result;
}

Std_ReturnType Csm_CancelJob(Csm_JobIdType jobId)
{
   
    if(jobId > CSM_MAX_JOBS || !Csm_Initialized) {
        return E_NOT_OK;
    }
    
    for(uint32_t i = 0; i < CSM_MAX_JOBS; i++)
    {
        if(Csm_JobTable[i].job.jobId == jobId && Csm_JobTable[i].state != JOB_IDLE)
        {
            if (Csm_JobTable[i].state == JOB_RUNNING || Csm_JobTable[i].state == JOB_PENDING) 
            {
                Csm_JobTable[i].state = JOB_IDLE;
                /* Notify application if callback is registered */
                if (Csm_JobCb != NULL) 
                {
                    Csm_JobCb(jobId, E_NOT_OK, Csm_JobTable[i].job.userCtx);
                }

                return E_OK;
            }
            else
            {
                return E_NOT_OK;
            }
        }
    }
    return E_NOT_OK;
}

void Csm_SetJobCompleteCallback(Csm_JobCompleteCallbackType cb)
{

}