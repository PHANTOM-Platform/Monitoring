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
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>

#include "Elements.h"

extern "C" {
#include "../src/mf_api.h"
}

using namespace std;

void initNetwork(string name, NetworkParams** netparams, int nrBranches,
		int nrVertexes) {

	/*
	 *	 	 B1  B2  B3  B4  B5  B6  B7  B8
	 *
	 *	N1	-1,  0,  0,  0,  0,  0,  0,  0,
	 * 	N2	 1, -1,  0,  0,  0,  0, -1, -1,
	 *	N3	 0,  1,  0,  0, -1, -1,  0,  0,
	 *	N4	 0,  0, -1,  0,  1,  1,  0,  0,
	 *	N5	 0,  0,  1, -1,  0,  0,  1,  1
	 *	N6	 0,  0,  0,  1,  0,  0,  0,  0
	 *
	 */

	*netparams = new NetworkParams(nrBranches, nrVertexes, new int[6 * 8] { -1,
			0, 0, 0, 0, 0, 0, 0, 1, -1, 0, 0, 0, 0, -1, -1, 0, 1, 0, 0, -1, -1,
			0, 0, 0, 0, -1, 0, 1, 1, 0, 0, 0, 0, 1, -1, 0, 0, 1, 1, 0, 0, 0, 1,
			0, 0, 0, 0 }, &name[0]);
	//(*netparams)->getA()[0]

}

Branch* initBranch(BranchParams *branchparams, NetworkParams *netparams) {
	Branch* branch = new Branch(branchparams, netparams);

	cout << "Branch " << (branch->getParams())->getName() << " with A="
			<< (branch->getParams())->getA() << ", B="
			<< (branch->getParams())->getB() << ", C="
			<< (branch->getParams())->getC() << " has been initialized" << endl;

	//return new Branch(branchparams, netparams);
	return branch;
}

void initBranches(Branch** branches, NetworkParams* netparams) {
	BranchParams *branchparams = new BranchParams(0.0655, 0.00252, 259.68, 10,
			"Branch1");
	branches[0] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.35, 0.00252, 1385, 10, "Branch2");
	branches[1] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.35, 0.00252, 1385, 10, "Branch3");
	branches[2] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.08, 0.00252, 319.6, 10, "Branch4");
	branches[3] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.02, 0.004, 150.2, 10, "Branch5");
	branches[4] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.034, 0.009, 160.15, 10, "Branch6");
	branches[5] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.021, 0.004, 174.465, 10, "Branch7");
	branches[6] = initBranch(branchparams, netparams);

	branchparams = new BranchParams(0.043, 0.0055, 290.9, 10, "Branch8");
	branches[7] = initBranch(branchparams, netparams);
}

Vertex* initVertex(VertexParams *vertexparams, NetworkParams *netparams) {

	Vertex* vertex = new Vertex(vertexparams, netparams);

	cout << "Vertex " << (vertex->getParams())->getName() << " external="
			<< (vertex->getParams())->getIsExternal() << " with H_external="
			<< (vertex->getParams())->getH_external() << " has been initialized"
			<< endl;

//return new Vertex(vertexparams, netparams);
	return vertex;
}

void initVertexes(Vertex** vertexes, NetworkParams* netparams) {
	VertexParams *vertexparams = new VertexParams(true, 4100, "Node1");
	vertexes[0] = initVertex(vertexparams, netparams);

	vertexparams = new VertexParams(false, 0, "Node2");
	vertexes[1] = initVertex(vertexparams, netparams);

	vertexparams = new VertexParams(false, 0, "Node3");
	vertexes[2] = initVertex(vertexparams, netparams);

	vertexparams = new VertexParams(false, 0, "Node4");
	vertexes[3] = initVertex(vertexparams, netparams);

	vertexparams = new VertexParams(false, 0, "Node5");
	vertexes[4] = initVertex(vertexparams, netparams);

	vertexparams = new VertexParams(true, 0, "Node6");
	vertexes[5] = initVertex(vertexparams, netparams);
}

void simulation_loop(Branch** branches, Vertex** vertexes,
		NetworkParams* netparams, int loopNr, float integrationStep) {

	int nrBranches = netparams->getNrBranches();
	int nrVertexes = netparams->getNrVertexes();

	/*
	 *   1. Propagation of boundary conditions
	 */

	//   1.1. Pressures from vertexes to branches
	for (int i = 0; i < nrBranches; i++)
		for (int j = 0; j < nrVertexes; j++) {
			if (netparams->getElementOfA(j, i) == 1) //branch enters into the vertex
				(*branches[i]).updateP_End((*vertexes[j]).getP());
			else if (netparams->getElementOfA(j, i) == -1) //branch leaves from the vertex
				(*branches[i]).updateP_Start((*vertexes[j]).getP());
		}
	/*
	 *   2. Numerical solution step
	 */

	//   2.1. Airflow in branches
	for (int i = 0; i < nrBranches; i++)
		(*branches[i]).simulateBranchFlow(integrationStep);

	//   2.2. Boundary conditions in vertexes
	for (int j = 0; j < nrVertexes; j++) {

		bool isExternal = (*vertexes[j]).getParams()->getIsExternal();
		if (!isExternal) {

			float Qaverage = 0;
			float Caverage = 0;

			float QxC_average = 0;

			int totNrQ = 0;

			for (int i = 0; i < nrBranches; i++) {
				if (netparams->getElementOfA(j, i) == 1) { //branch enters into the vertex
					totNrQ++;

					int nrApproxElements =
							(*branches[i]).getParams()->getNrApproxElements();
					float* Qtemp = (*branches[i]).getQ();
					QxC_average += (*branches[i]).getParams()->getC()
							* Qtemp[nrApproxElements];

				} else if (netparams->getElementOfA(j, i) == -1) { //branch leaves from the vertex
					totNrQ++;

					int nrApproxElements =
							(*branches[i]).getParams()->getNrApproxElements();
					float* Qtemp = (*branches[i]).getQ();
					QxC_average -= (*branches[i]).getParams()->getC()
							* Qtemp[0];

					/* old interpretation of the boundary conditions

					 Caverage += (*branches[i]).getParams()->getC();

					 float* Qtemp = (*branches[i]).getQ();
					 Qaverage -= Qtemp[0];
					 */
				}
			}

			(*vertexes[j]).setQxC_average(QxC_average);

		}

		/**/cout << "Debug 1: Vertex " << j << " P= " << (*vertexes[j]).getP()
				<< endl;/**/
	}

	//   2.3. Pressures in vertexes
	for (int j = 0; j < nrVertexes; j++) {
		(*vertexes[j]).computeVertexPressure(integrationStep);
	}
}

int main() {

// initialization of the network parameters
	NetworkParams *netparams;

	initNetwork("TestNet", &netparams, 8, 6);
	cout << "Network " << netparams->getName() << " with "
			<< netparams->getNrBranches() << " branches and "
			<< netparams->getNrVertexes() << " vertexes has been initialized"
			<< endl;

// initialization of branch elements
	Branch *branches[netparams->getNrBranches()];
	initBranches(&(*branches), netparams);

// initialization of vertex elements
	Vertex *vertexes[netparams->getNrVertexes()];
	initVertexes(&(*vertexes), netparams);

// simulation loops
	float integrationStep = 0.001;
	int nrLoops = 5000;

/* MONITORING START */
	metrics m_resources;
	m_resources.num_metrics = 2;
	m_resources.local_data_storage = 1;
	m_resources.sampling_interval[0] = 1000; // 1s
	strcpy(m_resources.metrics_names[0], "resources_usage");

	m_resources.sampling_interval[1] = 1000; // 1s
	strcpy(m_resources.metrics_names[1], "disk_io");

	//m_resources.sampling_interval[2] = 1000; // 1s
	//strcpy(m_resources.metrics_names[2], "power");

	char *datapath = mf_start("localhost:3033", "laptop", &m_resources);

	for (int n = 0; n < nrLoops; n++) {
		cout << "LOOP " << n << endl;

        //clock_t begin_time = clock();
        auto begin_time = std::chrono::high_resolution_clock::now();
		simulation_loop(&(*branches), &(*vertexes), netparams, n,
				integrationStep);
		auto end_time = std::chrono::high_resolution_clock::now();
		//float duration = float( clock () - begin_time ) /  CLOCKS_PER_SEC;
		std::chrono::duration<double, std::milli> duration = end_time-begin_time;
		cout << "Loop duration (ms): " << duration.count() << endl;

	/* MONITORING
	    I'd like to store here the duration of each loop --> duration
	*/	
		char metric_value[8] = {'\0'};
		sprintf(metric_value, "%f", duration);
		mf_user_metric("duration", metric_value);
	}

/* MONITORING END */
	mf_end();
	
	/* MONITORING
	    I'd like to store here the total nr. of completed loops --> nrLoops
	*/
	char metric_value[8] = {'\0'};
	sprintf(metric_value, "%d", nrLoops);
	mf_user_metric("nrLoops", metric_value);		
	
/* MONITORING SEND */
	char *experiment_id = mf_send("localhost:3033", "dummy", "t1", "laptop");
	printf("\n> experiment_id is %s\n", experiment_id);

	cout << "Simulation finished";
	return 0;
}
