import math

def calcPeriod(note):
    return 16 * 1024 * 1024 / (math.pow(2, (note - 69) / 12.) * 880.)

def genNote(note):
    period = calcPeriod(note)

    if period < 65536:
        return '{' + str(round(period)) + ', 0b00001001}'
    elif period / 8 < 65536:
        return '{' + str(round(period / 8)) + ', 0b00001010}'
    else:
        return '{' + str(round(period / 64)) + ', 0b00001011}'


def genNoteDict():
    noteDict = "struct counterSetting noteDict[] = {\n"

    notes = ['\t' + genNote(note) for note in range(128)]

    noteDict += ',\n'.join(notes)
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

