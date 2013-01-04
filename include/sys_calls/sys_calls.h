/*
	sys_calls.h
	ALL system call declarations should go here
	including those which are just wrappers
	to system task communication
 */

#ifndef __SYS_CALLS__H__
#define __SYS_CALLS__H__

#include <lib/all.h>

//Task Managment
void Pass( );
void Exit( );
int Create( int priority, void(*code) );
int MyTid( );
int MyParentTid( );
void Quit( );
int KernGlobalsPointer( );

//Inter-task Communication
int Send( int Tid, char *msg, int msglen, char *reply, int replylen );
int Receive( int *tid, char *msg, int msglen );
int Reply( int tid, char *reply, int replylen );
int CourierSend( int Tid, char *msg, int msglen );

//Event Waiting
int AwaitEvent( int eventid, char *event, int eventlen );

//Name Server
int RegisterAs( char * name );
int WhoIs( char * name );

//Clock Server
int Time( );
int Delay( );
int DelayUntil( );


//worker tasks
int WorkerDelay( int delay );
int WorkerPeriodicDelay( int delay );
int WorkerDelayUntil( int delay );
int WorkerDelayWithId( int delay, int id );
int WorkerDelayUntilWithId( int delay, int id );


//printing
void HijackCOM2( );
int Getc( int channel );
int Putc( int channel, char ch );
int Putbuff( int channel, struct print_buffer* buff );
void Printf( int channel, char *fmt, ... );
void CommandOutput( char *fmt, ... );
void DisplaySensorList( int* recent_list );
void DisplaySwitch( int switch_num, char c );


//Helper Tasks
int Courier( int tid1, int tid2 );
int Train( int train_number, int track );

//Trains
void Abort( );
void PrintTrainCalibration( );
void UpdatePositionDisplay( int train_number, int speed, char* pos_node, int pos_offset, int error, int max_error, char* dest_node, int dest_offset, char* state );
void TrainLocationTabRegister( int train_number, struct train_data* data_pointer );
void TrainGameAIRegister(int train_number, struct train_data* data_pointer);
int AddTrain( int train, int track );
int SetTrainDirection( int train, int direction );
int SetTrainSpeed( int train, int speed );
int ReverseTrain( int train );
int SOSTrain( int train, int sensor );

//Switches
int SetSwitch( int sw, int setting );
int SwitchStatus( int sw );

//Sensors
int QueryAllSensors( char * sensorbuf );
int LastSensor( );

//tabbing
void TabRegister( char* title );
void TabLeft( );
void TabRight( );

#endif
