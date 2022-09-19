//File: midi.h
//Date Created: 9/18/22
//Authors: Landyn Francis (landyn.francis@maine.edu) Jacob Mealey (jacob.mealey@maine.edu)
//Purpose: Header file containing MIDI command codes, and other MIDI related defines
#include <stdint.h>
//MIDI COMMAND SET
#define NOTE_OFF			0x80	//2 Data Bytes: Note #, Velocity
#define NOTE_ON				0x90	//2 Data Bytes: Note #, Velocity (Note ON with 0 Velocity = NOTE OFF)
#define KEY_PRESSURE		0xA0	//2 Data Bytes: Note #, Pressure
#define CONTROL_CHANGE		0xB0	//2 Data Bytes: Controller #, Value)
#define PROGRAM_CHANGE		0xC0	//1 Data Byte: Program number
#define CHANNEL_PRESSURE	0xD0	//1 Data Byte: Channel Pressure
#define PITCH_BEND			0xE0	//2 Data Bytes: (LSB,MSB)
#define SYS_EXCL_START		0xF0	//Variable Data Bytes
#define SYS_COMMON			0xF1	//0, 1 or 2 Bytes
#define SYS_EXCL_END		0xF7	//0 Data Bytes
#define SYS_REAL_TIME		0xF8	//0 Data Bytes

#define VOLUME_CONTROLLER	7		// Controller number to control volume

//Packs up and sends "general" MIDI message
//i.e. NOTE OFF / NOTE ON / KEY PRESSURE /
//pressure = 0 for NOTE OFF and NOTE ON messages
//velocity = 0 for KEY PRESSURE MESSAGES
//Return Values: 0 (success) 1 (error)
uint8_t send_general_midi_message(uint8_t command_num, uint8_t channel_num, uint8_t note_num, uint8_t velocity, uint8_t pressure);

