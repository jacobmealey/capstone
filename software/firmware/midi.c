//File: midi.c
//Date Created: 9/18/22
//Authors: Landyn Francis (landyn.francis@maine.edu) Jacob Mealey (jacob.mealey@maine.edu)
//Purpose: Source file containing MIDI packaging and sending functions

#include <stdint.h>
#include <stdio.h>
#include "midi.h"
#include "keys.h"

// Send a general MIDI Message 
// A "General MIDI Message" can be any of the following
// command_num: which MIDI command to send
// channel_num: which MIDI channel to use (leave as 0)
// note_num: note value for MIDI message
// velocity: MIDI velocity of keypress
// pressure: MIDI pressure of keypress (TBD)
uint8_t send_general_midi_message(uint8_t command_num, uint8_t channel_num, uint8_t note_num, uint8_t velocity, uint8_t pressure){
    // Check for appropriate command number
	if ((command_num != NOTE_OFF) && (command_num != NOTE_ON) && (command_num != KEY_PRESSURE)){
		return 1;
	}

	// Pressure command (not implemented)
	if (command_num == KEY_PRESSURE){
		// Prepare MIDI Packet
		uint8_t message[3] = { command_num | channel_num, note_num, pressure};
		// Send MIDI message
    	tud_midi_stream_write(0, message, 3);
		return 0;
	}

	// Prepare MIDI Packet
	uint8_t message[3] = { command_num | channel_num, note_num, velocity};
	// Send MIDI message
    tud_midi_stream_write(0, message, 3);
	return 0;
}

// Send a MIDI volume message
uint8_t change_midi_volume(uint8_t channel_num, uint8_t volume){
	// Double check if volume value is too large
	if (volume > 127){
		keyboard_global->volume = 127;
		volume = 127;
	}
	// Prepare MIDI package
	uint8_t message[3] = {CONTROL_CHANGE|channel_num, VOLUME_CONTROLLER, volume};
	// Send MIDI message
	tud_midi_stream_write(0, message, 3);
	return 0;
}

// Send MIDI Volume mute message
// This specific controller number sends a note off for each key
uint8_t mute_midi_volume(uint8_t channel_num){
	// Prepare MIDI Packet
	uint8_t message[3] = {CONTROL_CHANGE | channel_num, MUTE_CONTROLLER, 0x00};
	// Send MIDI message
	tud_midi_stream_write(0, message, 3);
	return 0;
}


