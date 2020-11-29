#include <iostream>
#include <string>
#include <fstream>
#include "Computer.h"
using namespace std;

Computer dummy;

int assemble(string op, int arg=0) {
	if (op == "VAL") return (arg & 0b11111111);
	if (dummy.MC.find(op) == dummy.MC.end())
		cout << "ERROR: operation \"" << op
			<< "\" doesn't exist" << endl;
	int hn = dummy.MC[op];
	return (hn << 4 | (arg & 0b1111));
}

int main() {

	string filename;
	cout << "Insert filename: ";
	cin >> filename;

	ofstream of("programs/" + filename);

	bool ended = false;
	for (int i = 0; i < 16; i++) {
		if (ended) { of << 0 << endl; continue; }

		cout << i << ": ";
		string op; cin >> op;
		if (op == "END") {
			ended = true;
			of << 0 << endl;
			continue;
		}

		int arg = 0;

		if (op != "NOP" &&
			op != "OUT" &&
			op != "HLT") {
			cin >> arg;
		}

		of << assemble(op, arg) << endl;
	}

	of.close();

	return 0;
}
