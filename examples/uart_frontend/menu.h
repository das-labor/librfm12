

typedef struct{
	const char * text;
	void (*func)(void *); //function that is called when entry is entered 
	void (*draw)(void *);    //function that draws the value of the setting
	void * ref;           //ref for the functions
}menu_entry_t;

typedef struct{
	uint8_t num_entries;
	menu_entry_t * entries;
	void (*background_function)(void);
}menu_t;

typedef void(*hdlr_t)(void *);

void handle_menu(void * men);

void menu_exit(void * foo);

extern uint8_t leave_menu;

void handle_uint8_setting( uint8_t * var);
void show_uint8_setting(uint8_t * var);

void draw_parameter(char * str);

void draw_parameter_P(PGM_P str);
