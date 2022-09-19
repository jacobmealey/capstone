//File: midi.c
//Date Created: 9/18/22
//Authors: Landyn Francis (landyn.francis@maine.edu) Jacob Mealey (jacob.mealey@maine.edu)
//Purpose: Source file containing MIDI packaging and sending functions

#include <stdint.h>
#include <stdio.h>
#include "midi.h"

uint8_t send_general_midi_message(uint8_t command_num, uint8_t channel_num, uint8_t note_num, uint8_t velocity, uint8_t pressure){
    //printf("Command number: %x\n", command_num);
	if ((command_num != NOTE_OFF) && (command_num != NOTE_ON) && (command_num != KEY_PRESSURE)){
		return 1;
	}

	if (command_num == KEY_PRESSURE){
		uint8_t message[3] = { command_num | channel_num, note_num, pressure};
    	tud_midi_stream_write(0, message, 3);
		return 0;
	}

	uint8_t message[3] = { command_num | channel_num, note_num, velocity};
    tud_midi_stream_write(0, message, 3);
	return 0;
}

uint8_t change_midi_volume(uint8_t channel_num, uint8_t volume){
	if (volume > 127){//If volume is too large, set it to max level
		volume=127;
	}
	uint8_t message[3] = {CONTROL_CHANGE|channel_num, VOLUME_CONTROLLER, volume};
	tud_midi_stream_write(0, message, 3);
	return 0;
}


