#include "Computer.h"
#include <iostream>
#include <bitset>
#include <chrono>
#include <thread>

using namespace std;

Computer comp;

void clearScreen(int la=50) {
	while (la--) cout << "\n";
}

int main() {

	bool ended = false;
	for (int i = 0; i < 16; i++) {
		cout << i << ": ";
		if (ended) {
			comp.RAM[i] = 0;
			continue;
		}

		int mem; cin >> mem;
		if (mem == -1) {
			comp.RAM[i] = 0;
			ended = true;
			continue;
		}

		comp.RAM[i] = mem;
	}


	cout << "\n\nComputer RAM: \n";
	for (int i = 0; i < 16; i++) {
		cout << comp.RAM[i] << "\t";
		if (i % 4 == 3) cout << "\n";
	}

	comp.debug = false;

	while (comp.PC < 16) {
		comp.tick();

		clearScreen();

		cout << "PC:\t" << bitset<4>(comp.PC) << "\n";
		cout << "SC:\t" << bitset<4>(comp.SC) << "\n";
		cout << "MAR:\t" << bitset<4>(comp.MAR) << "\n";

		cout << "A:\t" << bitset<8>(comp.A) << "\n";
		cout << "B:\t" << bitset<8>(comp.B) << "\n";
		cout << "IR:\t" << bitset<8>(comp.IR) << "\n";
		cout << "ALU:\t" << bitset<8>(comp.ALU) << "\n";
		cout << "BUS:\t" << bitset<8>(comp.ALU) << "\n";

		cout << "FLAGS:\t" << bitset<2>(comp.FLAGS) << "\n";

		cout << "RAM:" << endl;
		for (int i = 0; i < 16; i++) {
			cout << i << ":\t" << bitset<8>(comp.RAM[i]) << "\n";
		}

		cout << "OUTPUT:\t" << bitset<8>(comp.OUT) << " = " << comp.OUT << "\n";

		this_thread::sleep_for(chrono::milliseconds(10));
	} 
}
