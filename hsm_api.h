#ifndef HSM_API_H_
#define HSM_API_H_

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include<stdio.h>




/*-----------------------------------------------------------------------------
    Type Definitions
-----------------------------------------------------------------------------*/

typedef  unsigned	short	UINT16;
typedef  unsigned	char	UINT8;
typedef  signed		char  	SINT8;
typedef  unsigned	int		UINT32;


/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/

#define PRIVATE static
#define PUBLIC


/* Maximum number of parameters in one slot message */
#define MSG_MAX_PARAM_SLOT					((UINT8)8)

/* Maximum number of allowable nesting of states in an HSM*/
#define HSM_MAX_STATE_NESTING				((UINT8)10)



/************* Local IDs for HSM default messages	*****************/


/*Local ID for ENTRY message */
#define HSM_LID_ENTRY						((UINT16)1)

/*Local ID for START message */
#define HSM_LID_START						((UINT16)2)

/*Local ID for EXIT message */
#define HSM_LID_EXIT						((UINT16)3)

/* Logical package ID */
#define HSM_PID								((UINT8)0xff)


/*-----------------------------------------------------------------------------
    Type Definitions
-----------------------------------------------------------------------------*/

/* Structure to combine PID and LID as address
 *
 * This structure represents an address of a sender/receiver of the message. 
 */
typedef struct 
{
    UINT16        u16_pid;			/*package ID*/
    UINT16        u16_lid;			/*Local ID (message ID)*/
} Ts_pid_lid;


/*
 * Structure of Message slot 
 *
 * This structure represents type and data of a parameter for a slot message.
 */
typedef struct 
{
    UINT8                u8_type;     /* The type of the parameter								    */
    UINT32               u32_data;    /* Data contains an integer or a 32bit pointer to external data */
}Ts_sys_data_slot;



/* This structure represents parameters of Message */
typedef struct 
{
    Ts_pid_lid				st_dest_id;        /* Address of message receiver     */
    Ts_pid_lid				st_src_id;         /* Address of message sender       */
    UINT8					u8_status;         /* Message status field (internal use only!) */
    UINT8       			u8_transactionID;  /* Transaction ID to differ identical messages for different transactions */
    SINT8       			s8_resultcode;     /* Resultcode for return messages */
    Ts_sys_data_slot		ast_slots[MSG_MAX_PARAM_SLOT]; /**< Array to store Message data */
} Ts_msg;



/* Type definition of the basic struct for a HSM
 *
 * To create a HSM, define a variable of this type in your data structure as first 
 * item. This struct includes controlling values( pointer to current,next and top states ,PID of HSM)
 * needed for HSM handling.
 */
typedef struct Hsm Ts_hsm; 

 
/* Type definition for a HSM-state
 *
 * This includes pointer to parent state and pointer to function handler of that state. 
 * Use the macro HSM_CREATE_STATE() for defining a state in HSM. 
 */
typedef struct Hsm_state Ts_hsm_state;


/* Type definition for function pointer to state handler function
 *
 * Use this prototype to define a pointer to any state handler function in HSM 
 */
typedef Ts_msg* (*Tfp_hsm_evt_hndlr)(Ts_hsm* pst_me, Ts_msg* pst_msg);



/* Structure for HSM-state 
 *
 * This includes pointer to parent state and pointer to function handler of that state.
 * Use the macro HSM_CREATE_STATE() for defining a state in HSM.
 */
struct 	Hsm_state			
{
    const Ts_hsm_state      *pst_parent;		/* pointer to parent state, or NULL if this is the Top state */
    Tfp_hsm_evt_hndlr        fp_hndlr;			/* HSM state handling function */
    UINT8					*pu8_state_name;	/* char pointer to string represents state name */
};



/*
 * Structure holds data for HSM handling .
 *
 * This structure includes controlling values( pointer to current,next and top states ,PID of HSM)
 * needed for HSM handling.
 */
struct Hsm
{
    const Ts_hsm_state  *pst_curr;    /* pointer to the current active state */
    const Ts_hsm_state  *pst_next;    /* pointer to next state, if transition was taken, otherwise NULL */
    const Ts_hsm_state  *pst_top;     /* pointer to top state */
    UINT16           	 u16_pid;     /* PID of the HSM owner */
};



/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/

/*
 * Performs a state transition
 *
 * Use this macro for transiting from current state to target state in a HSM  
 * This macro will invoke HSM_StateTransition() function. 
 *	
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling 
 *			 pst_target		Pointer to the target state 	
 * 
 */
#define HSM_STATE_TRANSITION(pst_me,pst_target)   (HSM_StateTransition((Ts_hsm*)(pst_me), (pst_target)))


/*
 * Starts the HSM
 *
 * This macro will allow the current state handler to handle the START message and will perform  
 * the possible state transitions until they are completed. 
 * This macro will invoke HSM_OnStart() function.
 *
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling  			
 *	
 */
#define HSM_ON_START(pst_me)             (HSM_OnStart((Ts_hsm*)(pst_me)))


/*
 * Handles a message in the given HSM
 *
 * Use this macro when there is any message to be handled. 
 * This macro will invoke HSM_OnMsg() function.
 *
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling  			
 *			 pst_msg		Pointer to the message to be handled
 */
#define HSM_ON_MSG(pst_me,pst_msg)        (HSM_OnMsg((Ts_hsm*)(pst_me),(pst_msg)))


/*
 * Defines a state in an HSM 
 *
 * Use this macro when new state has to be created in an HSM 
 * This macro creates a state of type Ts_hsm_state.
 */
#define HSM_CREATE_STATE(state_name,pst_parent_state,function_handler,Desp)  \
	  static const Ts_hsm_state state_name = { (pst_parent_state), (function_handler) , (Desp) } 





/*
 * Creates HSM base structure
 *
 * Use this macro to construct an HSM
 *
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling  			
 *			 pst_top		Pointer to the top state 
 *			 u16_pid		Pid of the HSM 
 */
#define HSM_CTOR(pst_me,pst_top,u16_pid) (HSM_HsmCtor( (pst_me), (pst_top),(u16_pid) ))


/*-----------------------------------------------------------------------------
    Public Function Declaration
-----------------------------------------------------------------------------*/

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
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_StateTransition(Ts_hsm* pst_me, const Ts_hsm_state* pst_target) ;



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
void HSM_OnMsg(Ts_hsm *pst_me, Ts_msg *pst_msg) ;


/*===========================================================================*/
/* 
 * Description:  This function will start a constructed HSM. This means it will
 * let the current state handler handle the start event and will react on the 
 * possible following state transitions, until they are finished.
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_OnStart(Ts_hsm *pst_me);


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
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_HsmCtor(Ts_hsm* pst_me, const Ts_hsm_state* pst_top,UINT16 u16_pid);

#endif /* End of HSM_API_H */