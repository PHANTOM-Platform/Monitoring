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

#include "Solver.h"

class NetworkParams {

public:
	NetworkParams() {
	}
	;
	NetworkParams(int nrBranches, int nrVertexes, int* A, char* name) {
		this->name = name;
		this->A = A;
		this->nrBranches = nrBranches;
		this->nrVertexes = nrVertexes;
	}
	;
	~NetworkParams() {
		delete[] A;
	}
	;

	int* getA() {
		return A;
	}
	;
	int getElementOfA(int vertexNr, int branchNr) {
		return A[vertexNr * nrBranches + branchNr];
	}
	;
	char* getName() {
		return name;
	}
	;
	int getNrBranches() {
		return nrBranches;
	}
	;
	int getNrVertexes() {
		return nrVertexes;
	}
	;

private:
	int* A;
	char* name;
	int nrBranches;
	int nrVertexes;
};

class BranchParams {
public:
	BranchParams() {
	}
	;
	BranchParams(float A, float B, float C, int nrApproximationElements,
			char* name) {
		this->A = A;
		this->B = B;
		this->C = C;
		this->nrApproximationElements = nrApproximationElements;
		this->name = name;
	}
	;

private:
	int nrApproximationElements;
	float A;
	float B;
	float C;
	char* name;

public:
	int getNrApproxElements() {
		return nrApproximationElements;
	}
	char* getName() {
		return name;
	}
	float getA() {
		return A;
	}
	float getB() {
		return A;
	}
	float getC() {
		return C;
	}
};

class VertexParams {
public:
	VertexParams() {
	}
	;
	VertexParams(bool isExternal, float H_external, char* name) {
		this->isExternal = isExternal;
		this->H_external = H_external;
		this->name = name;
	}
	;
private:
	float H_external;
	bool isExternal; // true if connected to an external depression source
	char* name;

public:
	float getH_external() {
		return H_external;
	}
	bool getIsExternal() {
		return isExternal;
	}
	char* getName() {
		return name;
	}
};

class Branch {
public:
	Branch() {

	}
	Branch(BranchParams *branchparams, NetworkParams *networkparams) {
		this->networkparams = networkparams;
		init(branchparams);
	}
	;
	~Branch() {
		delete[] P;
		delete[] Q;
	}
	;

	BranchParams* getParams() {
		return &branchparams;
	}
	;

private:
	// aerodynamical paramemeters
	float* P; // pressure
	float* Q; // air flow

	// aerodynamical properties of the element
	BranchParams branchparams;

	// network parameters
	NetworkParams *networkparams;

public:
	void init(BranchParams *branchparams) {

		//TODO:
		//check whether Q and P have been allocated

		this->branchparams = *branchparams;

		this->Q = new float[this->branchparams.getNrApproxElements()];
		for (int i = 0; i < this->branchparams.getNrApproxElements(); i++)
			this->Q[i] = 0;

		P = new float[this->branchparams.getNrApproxElements() + 1];
		for (int i = 0; i < this->branchparams.getNrApproxElements() + 1; i++)
			P[i] = 0;
	}

	int simulateBranchFlow(float integrationStep) {
		cout << "Simulating branch " << branchparams.getName() << endl;

		Euler *solver = new Euler();
		solver->simulateBranch(integrationStep, P, Q, branchparams.getA(),
				branchparams.getB(), branchparams.getC(),
				branchparams.getNrApproxElements());

		/*for (int i = 0; i < branchparams.getNrApproxElements(); i++)
		 cout << "Q[" << i << "]=" << Q[i] << ",";
		 cout << endl;*/

		return 0;
	}

	void updateP_Start(float newP) {
		P[0] = newP;
	}

	void updateP_End(float newP) {
		P[this->branchparams.getNrApproxElements()] = newP;
	}

	float* getP() {
		return P;
	}

	float* getQ() {
		return Q;
	}

private:
	int fetchPressure() {

		return 0;
	}
};

class Vertex {
public:
	Vertex() {
	}
	;
	Vertex(VertexParams *vertexparams, NetworkParams *networkparams) {
		this->networkparams = networkparams;
		init(vertexparams);
	}
	;
	~Vertex() {

	}
	;
	VertexParams* getParams() {
		return &vertexparams;
	}
	;

private:
	// aerodynamical properties of the element
	VertexParams vertexparams;

	// network parameters
	NetworkParams* networkparams;

	// aerodynamical paramemeters
	float P; // pressure
	float QxC_average;

public:
	void init(VertexParams *vertexparams) {
		this->vertexparams = *vertexparams;

		P = this->vertexparams.getH_external();
		QxC_average = 0;
	}

	float getP() {
		return P;
	}

	void setP(float P) {
		this->P = P;
	}

	void setQxC_average(float QxC_average) {
		this->QxC_average = QxC_average;
	}

	int computeVertexPressure(float integrationStep) {
		cout << "Simulating vertex " << vertexparams.getName() << endl;
		P = P + (QxC_average) * integrationStep;

		return 0;
	}

};
