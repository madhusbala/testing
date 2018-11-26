/*-----------------------------------------------------------------------------
    File Inclusion
-----------------------------------------------------------------------------*/
#include<stdio.h>
#include"hsm_api.h"


/*-----------------------------------------------------------------------------
			  Variable Definition(static) 
-----------------------------------------------------------------------------*/


/* This message is delivered while entering into a state.
 *
 * While transiting from current state to target state , ENTRY message(st_EntryMsg)
 * is received by all parent states of target state including target state. 
 */
static Ts_msg st_EntryMsg = {{HSM_PID, HSM_LID_ENTRY}};


/* This message is delivered while starting a state.
 *
 * During state transition,once the target state is reached, START message(st_StartMsg) will be 
 * delivered to target state soon after ENTRY message. 
 * 
 * Note: START message is not delivered to all parent states of target state but only delivered to 
 *		 target state.
 */
static Ts_msg st_StartMsg  = {{HSM_PID, HSM_LID_START}};



/* This message is delivered while leaving a state */
static Ts_msg st_ExitMsg  = {{HSM_PID, HSM_LID_EXIT }};


/*-----------------------------------------------------------------------------
			  Macro Definition
-----------------------------------------------------------------------------*/

/*
 * Calls current state handler with the given message
 *
 * Use this macro to check whether given message
 * can be handled by current state of HSM
 *
 * param[in]  
 *			pst_state		pointer to the state whose function handler will be called 
 *			pst_me			Pointer to the HSM which shall perform the transition 
 *			pst_msg			Pointer to the message to be handled		
 *			
 *
 */
#define HSM_STATE_HANDLER_CALL(pst_state, pst_me, pst_msg) ((*(pst_state)->fp_hndlr )((pst_me), (pst_msg)))

/*-----------------------------------------------------------------------------
    Private Function Definitions
-----------------------------------------------------------------------------*/


PRIVATE const Ts_hsm_state* Find_Least_Cmn_Ancestor(const Ts_hsm_state  *pst_curr,const Ts_hsm_state  *pst_next)
{
	const Ts_hsm_state *pst_lca = NULL ;
	
	while(pst_curr != NULL )
	{
		const Ts_hsm_state *pst_top = pst_next ;
		
		while(pst_top != NULL )
		{
			if( pst_curr == pst_top )
			{
				pst_lca   =  pst_curr ;		/* Assign current state as LCA state   */
				pst_top   =  NULL;			/*Condition breaks inner while loop */
				pst_curr  =  NULL;			/*Condition breaks outer while loop */
			}
			else
			{
				pst_top   = pst_top -> pst_parent ;		/* */
			}
		
		}
		
		/* Checking pst_curr is Valid State */
		if(pst_curr != NULL)
		{
			pst_curr = pst_curr -> 	pst_parent;
		}
		else
		{
				//   Do Nothing . Just for MISRA C
		}
	
	}
	
	return pst_lca ;

}

/*-----------------------------------------------------------------------------
    Public Function Definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/* 
 * Description:  This function will start a constructed HSM. This means it will
 * let the current state handler handle the start event and will react on the 
 * possible following state transitions, until they are finished.
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 
 * Return_Val
 *			 None 		
 */
/*===========================================================================*/
void HSM_OnStart(Ts_hsm *pst_me)
{
		if(pst_me != NULL)
		{
    
			/* At start of the hsm the current state is the top state */
			pst_me->pst_curr = pst_me->pst_top;
	        

			/*Calling state handler of Top State with Entry msg*/
			(void)HSM_STATE_HANDLER_CALL(pst_me->pst_curr, pst_me, &st_EntryMsg);
	        
			/* Send first start event to the top state and handle possible state transitions */
			HSM_OnMsg(pst_me, &st_StartMsg);
		} 
}



/*===========================================================================*/
/* 
 * Description:  This function will start the handling of an message by a HSM by 
 * calling State handler of the current state with the given message. Depending on 
 * the return value of the handler, the HSM performs one of the following 
 * a) Performs a state transition if next state is updated in state handler   
 * b) Give the message to superior state if return value of the handling function 
 *	  is not NULL
 * c) Do nothing more
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 pst_msg		Pointer to the message to be handled  
 *
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_OnMsg(Ts_hsm *pst_me, Ts_msg *pst_msg)
{
		


		if(pst_me != NULL)
		{
				
			const Ts_hsm_state *pst_lstate = pst_me -> pst_curr ;
			
			while(pst_lstate != NULL )
			{
			
				/*  HSM_STATE_HANDLER_CALL() calls Current state handler to check whether given message can be handled by current state of HSM  */
				if(  (  HSM_STATE_HANDLER_CALL(pst_lstate, pst_me,pst_msg) == NULL )  ||  ( pst_lstate -> pst_parent == NULL )  )
				{
					/* Checking for Transition */	
					while( pst_me -> pst_next != NULL)
					{
						const Ts_hsm_state *pst_lca = NULL ;
						
						/* Finds LCA for current and next State */
						pst_lca = Find_Least_Cmn_Ancestor(pst_me-> pst_curr,pst_me -> pst_next );
						
						/* Checking whether LCA is found */
						if(pst_lca != NULL)
						{
							const Ts_hsm_state  *pst_Temp_State = pst_me -> pst_curr ;
							
							/* Exit all states till LCA is reached */
							while(pst_Temp_State != pst_lca)
							{
								/* Calling current state handler with Exit msg */
								HSM_STATE_HANDLER_CALL(pst_Temp_State, pst_me,&st_ExitMsg);
								
								/* Assigning current state to parent state of the one which we left*/
								pst_me -> pst_curr = pst_Temp_State -> pst_parent ;
								
								pst_Temp_State = pst_Temp_State -> pst_parent ;
								
							}
							
						{
							/* pst_Trace_Path stores list of states from Next state to lca */
							const Ts_hsm_state  *pst_Trace_Path[HSM_MAX_STATE_NESTING] ={NULL};
							
							UINT8 u8_Trace_Index = 0;
							
							for(pst_Temp_State = pst_me -> pst_next ;pst_Temp_State != pst_lca  ; pst_Temp_State = pst_Temp_State -> pst_parent   )
							{
								pst_Trace_Path[u8_Trace_Index] = pst_Temp_State ;	 
								
								if(u8_Trace_Index >= HSM_MAX_STATE_NESTING )
								{
									printf("Error :  Maximum Nesting state limit is crossed \n ");
									
								}	
								u8_Trace_Index++;
								
							}	 
							
							/* Entering into the states from LCA to next state */
							while(u8_Trace_Index > 0)
							{
								u8_Trace_Index--;
								
								/* Assigning current state to the one which we are going to enter  */
								pst_me -> pst_curr =  pst_Trace_Path[u8_Trace_Index];
								
								/*Calling current state handler with Entry msg*/
								HSM_STATE_HANDLER_CALL(pst_me -> pst_curr,pst_me,&st_EntryMsg );
										
							
							}		
						}	
							/* Transition is done */
							
							pst_me ->  pst_next = NULL ;  /* Set next State to NULL as transition done*/
							
							/*Calling current state handler with Start msg*/
							HSM_STATE_HANDLER_CALL(pst_me -> pst_curr,pst_me,&st_StartMsg);
							
							
							
						}	
						else
						{
							printf("Error : ");
							
						}	
					}
					pst_lstate = NULL;
				}
				else
				{
					pst_lstate = pst_lstate -> pst_parent ;		/* Give message to parent state as can't handle in current state */	
				}
			
			}
			
		}


}


/*===========================================================================*/
/* 
 * Description :  This function is called to perform a state transition from the 
 * actual state to the target state. The state transition is not performed immediatley 
 * after the call, but after handling the actual message.
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 pst_target		Pointer to the target state 
 *
 * Return_Val
			 
			None 		
 */
/*===========================================================================*/
void HSM_StateTransition(Ts_hsm* pst_me, const Ts_hsm_state* pst_target)
{
   
    if (pst_target == NULL)
    {
		printf("Error : HSM transition to NULL\n");    
    }
    if (pst_me->pst_next != NULL)
    {
		printf("Error : No next state \n");
    }
    pst_me->pst_next = pst_target;
	
}


/*===========================================================================*/
/* 
 * Description:  This function creates the HSM based on the values given as 
 * parameters. All data within the HSM are initialized. 
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 pst_top        Pointer to the  top state  
 *			 u16_pid		Pid of the HSM	
 *
 * Return_Val	
			
			 None 		
 */
/*===========================================================================*/
void HSM_HsmCtor(Ts_hsm *pst_me, const Ts_hsm_state *pst_top, UINT16 u16_pid)
{
    
    pst_me->pst_curr  = NULL;
    pst_me->pst_next  = NULL;
	pst_me->pst_top   = pst_top;
	pst_me->u16_pid	  = u16_pid;

}