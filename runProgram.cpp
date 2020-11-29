#include "Computer.h"
#include <iostream>

using namespace std;

Computer comp;

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

	cout << endl << endl;

	cout << "Computer RAM: " << endl;
	for (int i = 0; i < 16; i++) {
		cout << comp.RAM[i] << "\t";
		if (i % 4 == 3) cout << endl;
	}

	while (true) {
		comp.tick();
		char dummy; cin >> dummy;
	}
}
