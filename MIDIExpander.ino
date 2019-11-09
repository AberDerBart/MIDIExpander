#include "MIDIUSB.h"
#include "noteDict.h"
#include "defs.h"

#define STEP_PIN 15
#define DIRECTION_PIN 14

int16_t pitchBend = 0;
bool noteChanged = false;

uint8_t halfTrack = 0;
uint8_t direction = 0;
uint8_t step = 0;

void setup(){
	//setup pins
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIRECTION_PIN, OUTPUT);

	initStack();

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
	if(step){
		digitalWrite(STEP_PIN, LOW);
		step = 0;
	}else{
		digitalWrite(STEP_PIN, HIGH);
		step = 1;
	}
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
	
	if(noteChanged){
		// TODO: adjust pitchBend
		uint8_t note = getStackTop();

		if(note == NO_NOTE){
			/* Timer/Counter1 Control Register B
			 * CTC mode (Bits 4:3 = 01)
			 * Clock Select = No clock (Bits 2:0 = 000)
			 */
			TCCR1B = 0x00001000;
		}else{
			if(pitchBend == 0){
				OCR1A = noteDict[note].ocr1a;
				TCCR1B = noteDict[note].tccr1b;
			}else{
				uint16_t off_ocr1a = 0;
				int16_t bend = pitchBend;
				
				if(pitchBend < 0){
					// calculate using only positive pitch bends by adjusting one note down
					note--;
					bend += PITCH_BEND_NEUTRAL;
				}

				if(noteDict[note].tccr1b == noteDict[note + 1].tccr1b){
					// off_ocr1a = ((noteDict[note].ocr1a - noteDict[note + 1].ocr1a) * bend) / PITCH_BEND_NEUTRAL;
					// the next line is equivalent to the one above
					off_ocr1a = ((noteDict[note].ocr1a - noteDict[note + 1].ocr1a) * bend) << 13;
				}else{
					// off_ocr1a = ((noteDict[note].ocr1a - (noteDict[note + 1].ocr1a / 8)) * bend) / PITCH_BEND_NEUTRAL;
					// the next line is equivalent to the one above
					off_ocr1a = ((noteDict[note].ocr1a - (noteDict[note + 1].ocr1a << 3)) * bend) << 13;
				}

				OCR1A = noteDict[note].ocr1a + off_ocr1a;
				TCCR1B = noteDict[note].tccr1b;
			}
		}

		noteChanged = false;
	}
}

void stopMidiNote(uint8_t note){
	uint8_t top = removeNoteFromStack(note);
	noteChanged = true;
}

void playMidiNote(uint8_t note){
	if(note != getStackTop() && note < 128){
		uint8_t top = pushNoteOnStack(note);
		noteChanged = true;
	}
}

void handleNoteOnEvent(uint8_t note, uint8_t velocity){
	if(velocity != 0){
		playMidiNote(note);
	}else{
		stopMidiNote(note);
	}
}

void handleNoteOffEvent(uint8_t note){
	stopMidiNote(note);
}

void handlePitchBendEvent(uint8_t byte1, uint8_t byte2){
	pitchBend = ((byte2 << 8) | byte1) - PITCH_BEND_NEUTRAL;
	noteChanged = true;
	//TODO: implement pitch bend value affecting pitch
}

void loop(){
	//get the MIDI packet
	midiEventPacket_t packet = MidiUSB.read();
	uint8_t velocity;
	
	switch(packet.header & 0x0f){
	case 0x08:
		//note off
		handleNoteOffEvent(packet.byte2);
		break;
	case 0x09:
		//note on
		handleNoteOnEvent(packet.byte2, packet.byte3);
		break;
	case 0x0e:
		//pitch bend
		handlePitchBendEvent(packet.byte2, packet.byte3);
		break;
	default:
		//ignore any other message
		break;
	}
}
