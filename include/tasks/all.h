/*
	tasks/all.h
	Include all of the task headers
	Only includes other files
 */

#ifndef __TASKS_ALL__H__
#define __TASKS_ALL__H__

//Special System Tasks
void task_system_first( );
void task_system_idle( );
void task_system_idle_instrumented( );
void task_system_extra_stack( );

void task_system_nameserver( );

void task_system_clock_server( );
void task_system_clock_notifier( );

void task_system_uart2_send_server( );
void task_system_uart2_recv_server( );

void task_system_uart1_send_server( );
void task_system_uart1_recv_server( );

void task_system_courier( );
void task_system_2way_courier( );
void task_system_single_msg_courier( );

void task_system_worker_delay( );
void task_system_worker_periodic_delay( );
void task_system_worker_delay_with_id( );
void task_system_worker_delay_until_with_id( );

void task_system_mdi_server( );
void task_system_sensor_server( );
void task_system_sensor_interpreter( );
void task_system_switch_server( );


//Normal User Taks
void task_user_cli( );
void task_user_clock( );
void task_user_draw_ui( );

void task_user_train_command_dispatcher( );
void task_user_train( );
void task_user_route_server( );

//tabbing user tasks
void task_user_display_tabber( );
void task_user_train_display_tab( );
void task_user_reservation_display_tab( );
void task_user_train_location_tab( );
void task_user_kern_debug_tab( );

//Special Test Tasks
void task_test_clock_accuracy( );
void task_test_srr_timing( );
void task_test_train( );

void task_test_loop_time( );

void task_test_zombie_reclaim( );
void task_test_zombie_reclaim_sub( );

//game tasks
void task_game_tab( );
void task_game_keyboard( );
void task_game_ai( );

#endif


