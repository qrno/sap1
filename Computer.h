#include <iostream>
#include <string>
#include <map>
using namespace std;

struct Computer {
	Computer() {
		for (int i = 0; i < 16; i++)
			RAM[i] = 0;
		for (int i = 0; i < 16; i++)
			for (int j = 0; j < 6; j++)
				MICRO[i][j] = 0;

		SIG["HLT"] = 0b1000000000000000; // Halt Clock
		SIG["MI"]  = 0b0100000000000000; // Memory Address In
		SIG["RO"]  = 0b0010000000000000; // Ram Out
		SIG["RI"]  = 0b0001000000000000; // Ram In
		SIG["IO"]  = 0b0000100000000000; // Instruction Data Out
		SIG["II"]  = 0b0000010000000000; // Instruction Data In
		SIG["AO"]  = 0b0000001000000000; // A Out
		SIG["AI"]  = 0b0000000100000000; // A In
		SIG["EO"]  = 0b0000000010000000; // ALU Out
		SIG["SU"]  = 0b0000000001000000; // ALU Sub
		SIG["BI"]  = 0b0000000000100000; // B In
		SIG["OI"]  = 0b0000000000010000; // Output In
		SIG["CE"]  = 0b0000000000001000; // Increase PC
		SIG["CO"]  = 0b0000000000000100; // PC Out
		SIG["JP"]  = 0b0000000000000010; // BUS -> PC
		SIG["FI"]  = 0b0000000000000001; // Flags In

		FLG["CF"] = 0b01;
		FLG["ZF"] = 0b10;

		MC["NOP"] = 0b0000; // No Operation
		MC["LDA"] = 0b0001; // Load value of RAM data into RA
		MC["ADD"] = 0b0010; // Calculate RA + RAM data and put in RA
		MC["SUB"] = 0b0011; // Calculate RA - RAM data and put in RA
		MC["STA"] = 0b0100; // Store RA into the RAM data
		MC["LDI"] = 0b0101; // Store instruction ID into RA
		MC["JMP"] = 0b0110; // Set PC to instruction ID
		MC["JC"]  = 0b0111; // Jump if carry
		MC["JZ"]  = 0b1000; // Jump if zero
		MC["OUT"] = 0b1110; // Put RA into OUT
		MC["HLT"] = 0b1111; // Halt the clock

		MICRO[MC["LDA"]][2] = SIG["IO"] | SIG["MI"];
		MICRO[MC["LDA"]][3] = SIG["RO"] | SIG["AI"];

		MICRO[MC["ADD"]][2] = SIG["IO"] | SIG["MI"];
		MICRO[MC["ADD"]][3] = SIG["RO"] | SIG["BI"];
		MICRO[MC["ADD"]][4] = SIG["EO"] | SIG["AI"] | SIG["FI"];

		MICRO[MC["SUB"]][2] = SIG["IO"] | SIG["MI"];
		MICRO[MC["SUB"]][3] = SIG["RO"] | SIG["BI"];
		MICRO[MC["SUB"]][4] = SIG["SU"] | SIG["EO"] | SIG["AI"] | SIG["FI"];

		MICRO[MC["STA"]][2] = SIG["IO"] | SIG["MI"];
		MICRO[MC["STA"]][3] = SIG["AO"] | SIG["RI"];

		MICRO[MC["LDI"]][2] = SIG["IO"] | SIG["AI"];

		MICRO[MC["JMP"]][2] = SIG["IO"] | SIG["JP"];

		MICRO[MC["OUT"]][2] = SIG["AO"] | SIG["OI"];

		MICRO[MC["HLT"]][2] = SIG["HLT"];
	};

	int PC	= 0b0000;
	int MAR = 0b0000;
	int A   = 0b00000000;
	int B   = 0b00000000;
	int ALU = 0b00000000;
	int OUT = 0b00000000;
	int IR  = 0b00000000;
	int SC  = 0b000;
	int FLAGS = 0b00;
	int BUS = 0b00000000;

	int RAM[16];

	int CW = 0b0000000000000000;

	map<string, int> SIG;
	map<string, int> FLG;
	map<string, int> MC;
	int MICRO[16][7];

	bool running = true;
	bool debug   = false;

	void tick() {
		if (running) {
			CW = decode_instruction(SC, IR >> 4, FLAGS);
			if (debug) log_tick(PC, SC, CW);
			update_modules(CW);
		}
	}

	int decode_instruction(int step, int instruction, int flags) {
		// Puts the PC'st value of the RAM in the IR
		if (step == 0) return SIG["MI"] | SIG["CO"];
		if (step == 1) return SIG["RO"] | SIG["II"] | SIG["CE"];

		// Jump Carry
		if (instruction == MC["JC"] && (FLAGS & FLG["CF"]) && (step == 2)) {
			return SIG["IO"] | SIG["JP"];
		}

		// Zero Carry
		if (instruction == MC["JZ"] && (FLAGS & FLG["ZF"]) && (step == 2)) {
			return SIG["IO"] | SIG["JP"];
		}

		return MICRO[instruction][step];
	}

	void update_modules(int word) {
		// Operations outputting to BUS
		if (word & SIG["RO"]) {
			BUS = RAM[MAR];
			if (debug) log_ma(MAR, RAM[MAR], true);
		}

		if (word & SIG["IO"]) {
			BUS = IR & 0b1111;
			if (debug) log_reg("IR OUT", IR);
		}

		if (word & SIG["AO"]) {
			BUS = A;
			if (debug) log_reg("A OUT", A);
		}

		if (word & SIG["CO"]) {
			BUS = PC;
			if (debug) log_reg("Counter OUT", PC);
		}

		// ALU always gets updated
		int result;
		if (word & SIG["SU"]) result = A - B;
		else result = A + B;
		ALU = result & 0b11111111;

		if (word & SIG["FI"]) {
			FLAGS = 0b00;
			if (result > 0b11111111) {
				FLAGS |= FLG["CF"];		
			}
			if (result == 0) {
				FLAGS |= FLG["ZF"];
			}
			if (debug) log_reg("Flags In", FLAGS);
		}

		// ALU OUT (EO) must happen before AI 
		if (word & SIG["EO"]) {
			BUS = ALU;
			if (debug) log_reg("ALU OUT", ALU);
		}

		// Operations that read
		if (word & SIG["MI"]) {
			MAR = BUS & 0b1111;
			if (debug) log_reg("MAR IN", MAR);
		}

		if (word & SIG["RI"]) {
			RAM[MAR] = BUS;
			if (debug) log_ma(MAR, BUS, false);
		}

		if (word & SIG["II"]) {
			IR = BUS;
			if (debug) log_reg("IR In", IR);
		}

		if (word & SIG["AI"]) {
			A = BUS;
			if (debug) log_reg("A In", A);
		}

		if (word & SIG["BI"]) {
			B = BUS;
			if (debug) log_reg("B In", B);
		}

		if (word & SIG["OI"]) {
			OUT = BUS;
			if (debug) log_reg("OUT In", OUT);
		}

		if (word & SIG["JP"]) {
			PC = BUS & 0b1111;
			if (debug) log_reg("PC In", PC);
		}

		if (word & SIG["CE"]) {
			PC++;
			if (debug) log_reg("Counter Enabled", PC);
		}

		SC = (SC+1 & 0b111);

		if (word & SIG["HLT"]) {
			running = false;
		}
	}
	
	void log_reg(string id, int val) {
		cout << id << " = " << val << endl;
	}

	void log_ma(int MAR, int VAL, int out) {
		if (out)
			cout << "RAM[" << MAR << "] OUT = " << VAL << endl; 
		else
			cout << "RAM[" << MAR << "] IN = " << VAL << endl; 
	}

	void log_tick(int PC, int SC, int CW) {
		cout << "PC: " << PC << " ";
		cout << "SC: " << SC << " ";
		cout << "CW: " << CW << " ";
		cout << endl;
	}
};
