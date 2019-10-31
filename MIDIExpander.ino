#include "MIDIUSB.h"
#include "noteDict.h"

#define STEP_PIN 14
#define DIRECTION_PIN 13

#define NO_NOTE 128

uint8_t currentNote = NO_NOTE;
int16_t pitchBend = 0;

uint8_t halfTrack = 0;
uint8_t direction;

void setup(){
	//setup pins
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIRECTION_PIN, OUTPUT);

	//setup timer for note generation

	/* Timer/Counter1 Control Register A
	 * Compare Output Mode A set to toggle (Bits 7:6 = 01)
	 * CTC mode (Bits 1:0 = 00) 
	 */
	TCCR1A = 0b01000000;
	/* Timer/Counter1 Control Register B
	 * CTC mode (Bits 4:3 = 01)
	 * Clock Select = No clock (Bits 2:0 = 000)
	 */
	TCCR1B = 0b00001000;
	/* Timer/Counter1 Interrupt Mask Register
	 * Output Compare A Match Interrupt Enable set (Bit 1 = 1)
	 */
	TIMSK1 = 0b00000010;
}

ISR(TIMER1_COMPA_vect){
	if(direction){
		halfTrack++;
		if(halfTrack == 159){
			direction = 0;
			digitalWrite(DIRECTION_PIN, LOW);
		}
	}else{
		halfTrack--;
		if(halfTrack == 0){
			direction = 1;
			digitalWrite(DIRECTION_PIN, HIGH);
		}
	}
}

void stopMidiNote(){
	/* Timer/Counter1 Control Register B
	 * CTC mode (Bits 4:3 = 01)
	 * Clock Select = No clock (Bits 2:0 = 000)
	 */
	TCCR1B = 0x00001000;
	currentNote = NO_NOTE;
}

void playMidiNote(uint8_t note){
	if(note != currentNote && note < 128){
		OCR1A = noteDict[note].ocr1a;
		TCCR1B = noteDict[note].tccr1b;
		/* Timer/Counter1
		 * Reset t 0
		 */
		TCNT1 = 0;
		currentNote = note;
	}
}

void handleNoteOnEvent(uint8_t note, uint8_t velocity){
	if(velocity != 0){
		playMidiNote(note);
	}else if(note == currentNote){
		stopMidiNote();
	}
}

void handleNoteOffEvent(uint8_t note){
	if(note == currentNote){
		stopMidiNote();
	}
}

void handlePitchBendEvent(uint8_t byte1, uint8_t byte2){
	pitchBend = ((byte2 << 8) | byte1) - 8192;
	//TODO: implement pitch bend value affecting pitch
}

void loop(){
	if(MidiUSB.available()){
		//get the MIDI packet
		midiEventPacket_t packet = MidiUSB.read();
		uint8_t velocity;
		
		switch(packet.header & 0xf0){
		case 0x80:
			//note off
			handleNoteOffEvent(packet.byte2);
			break;
		case 0x90:
			//note on
			handleNoteOnEvent(packet.byte2, packet.byte3);
			break;
		case 0xe0:
			//pitch bend
			handlePitchBendEvent(packet.byte2, packet.byte3);
			break;
		default:
			//ignore any other message
			break;
		}
	}

}