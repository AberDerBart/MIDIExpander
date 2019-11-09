#include <stdint.h>
#include "defs.h"

struct note;

struct stackNote{
	uint8_t prev;
	uint8_t next;

	bool active;
};

struct{
	struct stackNote notes[128];
	uint8_t top;
} noteStack;

void initStack(){
	for(uint8_t i = 0; i < 128; i++){
		noteStack.notes[i] = {NO_NOTE, NO_NOTE, false};
	}

	noteStack.top = NO_NOTE;
}

uint8_t removeNoteFromStack(uint8_t note){
	if(noteStack.notes[note].active){
		uint8_t prev = noteStack.notes[note].prev;
		uint8_t next = noteStack.notes[note].next;

		if(prev != NO_NOTE){
			noteStack.notes[prev].next = next;
		}

		if(next != NO_NOTE){
			noteStack.notes[next].prev = prev;
		}else{
			noteStack.top = prev;
		}

		noteStack.notes[note].active = false;
		noteStack.notes[note].prev = NO_NOTE;
		noteStack.notes[note].next = NO_NOTE;
	}
	
	return noteStack.top;
}

uint8_t pushNoteOnStack(uint8_t note){

	if(noteStack.notes[note].active){
		removeNoteFromStack(note);
	}

	noteStack.notes[note].prev = noteStack.top;
	noteStack.notes[note].active = true;
	noteStack.top = note;

	return noteStack.top;
}

inline uint8_t getStackTop(){
	return noteStack.top;
}
