INCLUDES= -I ./include
FLAGS=-Wall -g
OBJECTS=./build/gb_emulator.o ./build/gb_cpu.o ./build/gb_cartridge.o ./build/gb_data_bus.o ./build/gb_instructions.o ./build/gb_instruction_processing.o ./build/gb_memory.o ./build/gb_stack.o
all: ${OBJECTS}
	gcc  ${FLAGS} ${INCLUDES} ./src/main.c ${OBJECTS} -L ./lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -o ./bin/main

./build/gb_emulator.o:src/gb_emulator.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_emulator.c -c -o ./build/gb_emulator.o

./build/gb_cpu.o:src/gb_cpu.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_cpu.c -c -o ./build/gb_cpu.o

./build/gb_cartridge.o:src/gb_cartridge.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_cartridge.c -c -o ./build/gb_cartridge.o

./build/gb_memory.o:src/gb_memory.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_memory.c -c -o ./build/gb_memory.o

./build/gb_data_bus.o:src/gb_data_bus.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_data_bus.c -c -o ./build/gb_data_bus.o

./build/gb_instructions.o:src/gb_instructions.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_instructions.c -c -o ./build/gb_instructions.o

./build/gb_instruction_processing.o:src/gb_instruction_processing.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_instruction_processing.c -c -o ./build/gb_instruction_processing.o

./build/gb_stack.o:src/gb_stack.c
	gcc ${FLAGS} ${INCLUDES} ./src/gb_stack.c -c -o ./build/gb_stack.o