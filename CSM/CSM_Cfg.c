#include "Csm_Types.h"
#include "Csm_Cfg.h"

/* ===================[Static State]=================== */
static bool Csm_Initialized = false;
static Csm_JobRecord Csm_JobTable[CSM_MAX_JOBS];
static Csm_JobCompleteCallbackType Csm_JobCb = NULL;
static const Csm_ConfigType* Csm_ConfigPtr = NULL;

/* ===================[API Implementation]=================== */

/* Initialize CSM with a configuration */
Std_ReturnType Csm_Init(const Csm_ConfigType *config)
{
    if(config == NULL) 
    {
        return E_NOT_OK;
    }  

    Csm_ConfigPtr = config;

    for(uint32_t i = 0; i < Csm_ConfigPtr->maxJobs; i++)
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
    if(Csm_ConfigPtr == NULL)
    {
        return E_NOT_OK;
    }
    
    Csm_ConfigPtr = NULL;

    Csm_Initialized = false;
    return E_OK;
}

/* Start a new crypto job */
Std_ReturnType Csm_StartJob(const Csm_JobType *job)
{

}
