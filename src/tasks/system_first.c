#include <userspace.h>

//main task definition


void task_system_first( ) {
	int tid = 0;

	//create the name server
	tid = Create( PRIORITY_NAME_SERVER, &task_system_nameserver );
	//assert(tid == NAMESERVER_TID,"tid == NAMESERVER_TID");

	//regist as the first task
	RegisterAs( "first_task" );

	//Create the idle task
	tid = Create( PRIORITY_IDLE, &task_system_idle );

	//create the clock server and notifier
	tid = Create( PRIORITY_CLOCK_SERVER, &task_system_clock_server );
	//tid = Create(PRIORITY_CLOCK_NOTIFIER, &task_system_clock_notifier);

	//create the terminal IO servers
	tid = Create( PRIORITY_TERM_OUT_SERVER, &task_system_uart2_send_server );
	tid = Create( PRIORITY_TERM_IN_SERVER, &task_system_uart2_recv_server );

	//create the train IO servers
	tid = Create( PRIORITY_TRAIN_OUT_SERVER, &task_system_uart1_send_server );
	tid = Create( PRIORITY_TRAIN_IN_SERVER, &task_system_uart1_recv_server );

	tid = Create( PRIORITY_MDI_SERVER, &task_system_mdi_server );

	tid = Create( PRIORITY_SENSOR_SERVER, &task_system_sensor_server );

	tid = Create( PRIORITY_SWITCH_SERVER, &task_system_switch_server );

	tid = Create( PRIORITY_SENSOR_INTERPRETER, &task_system_sensor_interpreter );

	//create the command dispatcher
	tid = Create( PRIORITY_USER_TRAIN_COMMAND_DISPATCHER, &task_user_train_command_dispatcher );

	//create the route server
	tid = Create( PRIORITY_USER_ROUTE_SERVER, &task_user_route_server );

	//start the tabs
	tid = Create( PRIORITY_USER_DISPLAY_TABBER, &task_user_display_tabber );

	//Draw the UI
	tid = Create( PRIORITY_USER_DRAW_UI, &task_user_draw_ui );

	//Start the clock draw task
	tid = Create( PRIORITY_USER_CLOCK, &task_user_clock );

	//Start the CLI
	tid = Create( PRIORITY_USER_CLI, &task_user_cli );

	//tid = Create(PRIORITY_USER_TRAIN_DISPLAY_TAB,&task_user_train_display_tab);
	tid = Create( PRIORITY_USER_TRAIN_DISPLAY_TAB, &task_user_train_location_tab );

	tid = Create( PRIORITY_USER_TRAIN_DISPLAY_TAB, &task_user_reservation_display_tab );

	//Game tasks
	tid = Create( PRIORITY_GAME_TAB, &task_game_tab );
	tid = Create( PRIORITY_GAME_AI, &task_game_ai );
	tid = Create( PRIORITY_GAME_KEYBOARD, &task_game_keyboard );

	//KERNEL DEBUG TAB
	//tid = Create( 28, &task_user_kern_debug_tab);

	tid = Create( PRIORITY_IDLE_HIGH, &task_system_idle_instrumented );

	Exit( );
}

