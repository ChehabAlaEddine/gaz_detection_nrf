
#ifndef MODULE_H
#define MODULE_H

typedef enum
{
normal,alert,config
}State;


typedef struct
   {
  // bool alert;
     State state;
		 char device_name[20], 
			 device_pw[20],
			 num[10],
			 msg[20],
			 MSG_STOP[16];
   }gaz_module;

void gaz_module_routine(gaz_module * module);
void gaz_module_init(gaz_module * module);
#endif

