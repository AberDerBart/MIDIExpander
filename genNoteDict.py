import math

def calcPeriod(note):
    return 16000000 / (math.pow(2, (note - 69) / 12.) * 880.)

def genNote(note):
    period = calcPeriod(note)

    if period < 65536:
        return '{' + str(round(period)) + ', 0b00001001}'
    elif period / 8 < 65536:
        return '{' + str(round(period / 8)) + ', 0b00001010}'
    else:
        return '{' + str(round(period / 64)) + ', 0b00001011}'

def noteName(note):
    letters = ['A', 'A#', 'B', 'C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#']

    letterIndex = (note - 21) % 12
    octave = (note - 12) // 12

    return letters[letterIndex] + str(octave)


def genNoteDict():
    noteDict = "struct counterSetting noteDict[] = {\n"

    notes = ['\t' + genNote(note) + ', // ' + noteName(note) for note in range(128)]

    noteDict += '\n'.join(notes)
    noteDict += '\n};'

    return noteDict

if __name__ == '__main__':
    print('#include <stdint.h>')
    print()
    print('struct counterSetting{')
    print('\tuint16_t ocr1a;')
    print('\tuint8_t  tccr1b;')
    print('};')
    print()
    print(genNoteDict())

