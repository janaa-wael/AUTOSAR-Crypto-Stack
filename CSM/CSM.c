#include "CSM_Types.h"
#include "CSM_Cfg.h"
#include "CSM.h"


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

static int Csm_FindJobSlotById(Csm_JobIdType jobId)
{
    /* If CSM is not initialized, there are no valid jobs */
    if (!Csm_Initialized) {
        return -1;
    }

    for (uint32_t i = 0; i < CSM_MAX_JOBS; ++i) {
        /* Only consider non-idle slots to avoid matching stale entries */
        if (Csm_JobTable[i].state != JOB_IDLE &&
            Csm_JobTable[i].job.jobId == jobId) {
            return (int)i;
        }
    }
    return -1;
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
    if(cb != NULL)
    {
        Csm_JobCb = cb;
    }   
}
void Csm_MainFunction(void)
{
        if (!Csm_Initialized || Csm_ConfigPtr == NULL) {
        return;
    }

    for (uint32_t i = 0; i < CSM_MAX_JOBS; i++) {
        if (Csm_JobTable[i].state == JOB_PENDING) {
            /* If CryIf exposes a poll/status API, use it to check completion.
               We call CryIf_PollStatus(driverId, jobId) which should return:
                 - E_OK if finished successfully
                 - E_PENDING if still running
                 - E_NOT_OK if finished with error
               If you don't have such API, omit polling and rely on CryIf callbacks. */

            /* find driverId mapped for this job's primitive (search config) */
            Csm_PrimitiveDriverIdType driverId = 0;
            bool found = false;
            if (Csm_ConfigPtr != NULL) {
                for (uint32_t m = 0; m < Csm_ConfigPtr->numPrimitiveMap; m++) {
                    if (Csm_ConfigPtr->primitiveMap[m].primitive == Csm_JobTable[i].job.primitive) {
                        driverId = Csm_ConfigPtr->primitiveMap[m].driverId;
                        found = true;
                        break;
                    }
                }
            }

            if (found) {
                /* CryIf_PollStatus is optional — replace with your implementation */
                extern Std_ReturnType CryIf_PollStatus(Csm_PrimitiveDriverIdType driverId, Csm_JobIdType jobId);
                Std_ReturnType r = CryIf_PollStatus(driverId, Csm_JobTable[i].job.jobId);
                if (r == E_OK || r == E_NOT_OK) {
                    /* Completed (either success or error) */
                    Csm_InternalJobComplete(i, r);
                } else {
                    /* Still pending — optionally check for timeouts here */
                    /* Example: check timeout_ts (if you populated it previously) */
                    /* if (Csm_JobTable[i].timeout_ts != 0 && time_now() > Csm_JobTable[i].timeout_ts) handle timeout */
                }
            }
        }
    }
}

Std_ReturnType Csm_GetJobState(Csm_JobIdType jobId, Csm_JobState *state)
{
    if (!Csm_Initialized || state == NULL) 
    {
        return E_NOT_OK;
    }

    int slot = Csm_FindJobSlotById(jobId);
    if (slot < 0) 
    {
        return E_NOT_OK;
    }

    *state = Csm_JobTable[slot].state;
    return E_OK;
}

Std_ReturnType Csm_GetJobResult(Csm_JobIdType jobId, uint8_t *outBuf, size_t *outLen)
{
    if (!Csm_Initialized || outBuf == NULL || outLen == NULL) 
    {
        return E_NOT_OK;
    }

    int slot = Csm_FindJobSlotById(jobId);
    if (slot < 0) {
        return E_NOT_OK;
    }

        if (Csm_JobTable[slot].state == JOB_DONE) {
        /* job->job.output points to result, job->job.outputLen indicates expected length */
        size_t avail = Csm_JobTable[slot].job.outputLen;
        size_t toCopy = (*outLen < avail) ? *outLen : avail;
        if (toCopy > 0 && Csm_JobTable[slot].job.output != NULL) {
            memcpy(outBuf, Csm_JobTable[slot].job.output, toCopy);
        }
        *outLen = toCopy;

        /* Optionally: free/reset slot or keep as DONE until app acknowledges */
        /* For simplicity, mark slot idle so it can be reused */
        Csm_JobTable[slot].state = JOB_IDLE;
        return E_OK;
    } 
    else if (Csm_JobTable[slot].state == JOB_PENDING || Csm_JobTable[slot].state == JOB_RUNNING) 
    {
        return E_PENDING;
    } 
    else 
    {
        return E_NOT_OK;
    }
}
