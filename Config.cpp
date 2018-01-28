#include "Config.h"

void pinModes(){
  pinMode(LIGHT_BARRIER,	INPUT_PULLUP);
  pinMode(BATT_VOLTAGE,		INPUT		    );
  
  pinMode(POTI,				    INPUT		    );
  
  pinMode(BIG_BUTTON,		  INPUT_PULLUP);
  pinMode(SCHUSS_BUTTON,	INPUT_PULLUP);
  pinMode(BUTTON_1, 		  INPUT_PULLUP);
  pinMode(BUTTON_2,			  INPUT_PULLUP);
  pinMode(BUTTON_3,			  INPUT_PULLUP);

  pinMode(SWITCH_MOTOR,   INPUT_PULLUP);
  pinMode(SWITCH_SCHUSS,	INPUT_PULLUP);
  pinMode(SWITCH_KEEPER,	INPUT_PULLUP);
  pinMode(SWITCH_BODENS,	INPUT_PULLUP);
  pinMode(SWITCH_DEBUG,		INPUT_PULLUP);
  pinMode(SWITCH_MOTOR,		INPUT_PULLUP);
  pinMode(SWITCH_B,		  	INPUT_PULLUP);
  pinMode(SWITCH_A,		  	INPUT_PULLUP);
  
  pinMode(ROTARY_BUTTON,	INPUT_PULLUP);
  pinMode(ROTARY_A,			  INPUT_PULLUP);
  pinMode(ROTARY_B,			  INPUT_PULLUP);
  
  pinMode(INFO_LED,	  	OUTPUT		  );
  pinMode(BUZZER,			    OUTPUT		  );
  pinMode(MATRIX_LED,		  OUTPUT		  );
  
  pinMode(J_A8,				    INPUT		    );
  pinMode(J_A9,				    INPUT		    );
  pinMode(J_A10,			    INPUT		    );
  
  pinMode(INT_BODENSENSOR,INPUT		    );
  pinMode(INT_US,			    OUTPUT		  );
  
  pinMode(SCHUSS,			    OUTPUT		  );
  pinMode(BODEN_LED,		  OUTPUT		  );
  
  pinMode(DRDY,				    INPUT		    );
  pinMode(TILTSWITCH,		  INPUT_PULLUP);
  
  pinMode(M0_CURR,			  INPUT		    );
  pinMode(M1_CURR,			  INPUT		    );
  pinMode(M2_CURR,			  INPUT		    );
  pinMode(M3_CURR,			  INPUT		    );
}
