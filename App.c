/*-----------------------------------------------------------------------------
    File Inclusion
-----------------------------------------------------------------------------*/

#include "hsm_api.h"
#include "rsb_dab_hsm.h"


/*-----------------------------------------------------------------------------
    Global Variable Declaration
-----------------------------------------------------------------------------*/

/* Buffer for structure of type Ts_hsm which includes control values such as 
 * pointer to current,next and top states ,PID of HSM needed for HSM handling.
 */
PRIVATE Ts_hsm hsm_buffer ; 


/*	RSB_DAB_HSM   Messages	*/

Ts_msg st_RunLIDMsg  = {{GRA_DAB_HSM_PID, RUN_LID_COMMAND}};

Ts_msg st_SelectMsg  = {{GRA_DAB_HSM_PID, SELECT_SERVICE}};


/*-----------------------------------------------------------------------------
    Public Function Declaration
-----------------------------------------------------------------------------*/



void main()
{
		RSB_DAB_HSM_Init(&hsm_buffer);

		HSM_ON_MSG(&hsm_buffer, &st_RunLIDMsg);

		printf("...............\n");

		HSM_ON_MSG(&hsm_buffer, &st_SelectMsg);

		printf("...............\n");

		while (1);


}

