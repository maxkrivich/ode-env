/*
 * rpc.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: dimalit
 */

#include <iostream>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <vector>

using namespace std;

// Написать: библиотеку типа RPC для запуска процесса
pid_t rpc_call(const char* cmd, int* rf, int* wf){
	int ends_to_child[2];
	int ends_from_child[2];
	assert(pipe(ends_to_child)==0);
	assert(pipe(ends_from_child)==0);

//	sprintf(cmd, "../e1/Debug/e1 -M %d -E %f -phi %f -b %f -ksi %s -tol %f -step %f", m, E, phi, b, buf_ksi, tol, step);
//	system(cmd);
	pid_t pid=0;
	pid = fork();
	if(pid == 0){		// child

		close(0);
		dup2(ends_to_child[0], 0);	// pipe read->stdin
//		close(ends_to_child[0]); - it was closed by dup2!!
		close(ends_to_child[1]);

		close(1);
		dup2(ends_from_child[1], 1);	// dup pipe write end to stdout
//		close(ends_from_child[1]); - it was closed by dup2!!
		close(ends_from_child[0]);

//		close(ends[0]);		// XXX Strange it gives SIGPIPE when uncommented...

		char* buf = strdup(cmd);
		std::vector<char*> argv;
		char* tok = strtok(buf, " ");
		while(tok){
			argv.push_back(tok);
			tok = strtok(NULL, " ");
		}
		argv.push_back(NULL);

		execvp(argv[0], &(argv[0]));
		perror(NULL);
	}

	// will send initial state
	*wf = ends_to_child[1];
	close(ends_to_child[0]);

	// will receive results
	*rf = ends_from_child[0];
	close(ends_from_child[1]);

	return pid;
}
