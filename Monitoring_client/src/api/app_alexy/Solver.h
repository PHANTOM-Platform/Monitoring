/*======================
 Copyright (c) 2016, Alexey CHEPTSOV
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =======================
 */

#include <stdio.h>

using namespace std;

class Solver {
public:
	Solver() {
	}
	;
	~Solver() {
	}
	;

public:
	virtual int simulateBranch(float integrationStep, float *P, float *Q,
			float A, float B, float C, int nrApproxElements) {
	}
	;

	virtual int simulate() {
	}
	;

};

class Euler: public Solver {
public:

	Euler() :
			Solver() {
	}
	;
	/*
	 Euler(float integrationStep) :
	 Solver(integrationStep) {
	 }
	 ;
	 */

public:
	int simulateBranch(float integrationStep, float *P, float *Q, float A,
			float B, float C, int nrApproxElements) {

		//cout << "Simulating branch " << (*branch).getParams()->getName() << ": "
		//		<< endl;

		// Q's
		for (int i = 0; i < nrApproxElements; i++)
			Q[i] = ((P[i] - P[i + 1]) * A - (Q[i] * fabs(Q[i])) * B)
					* integrationStep + Q[i];

		// P's
		for (int i = 1; i < nrApproxElements; i++)
			P[i] = ((Q[i - 1] - Q[i]) * C) * integrationStep + P[i];

		for (int i = 0; i < nrApproxElements; i++)
			cout << "Q[" << i << "]=" << Q[i] << ",";
		cout << endl;

		for (int i = 0; i < nrApproxElements + 1; i++)
			cout << "P[" << i << "]=" << P[i] << ",";
		cout << endl;

		return 0;
	}
	;
	int simulateVertex(float integrationStep, float* Pvertex, float Qaverage,
			float Caverage) {

		*Pvertex = *Pvertex + (Qaverage * Caverage) * integrationStep;

		return 0;
	}
	;
}
;

