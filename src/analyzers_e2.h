/*
 * analyzers_e2.h
 *
 *  Created on: Jul 16, 2014
 *      Author: dimalit
 */

#ifndef ANALYZERS_E2_H_
#define ANALYZERS_E2_H_

#include "model_e2.h"
#include "gui_factories.h"

class E2FieldAnalyzer: public OdeAnalyzerWidget{
private:
	E2Config* config;
	int states_count;
	FILE* to_gnuplot;
public:
	E2FieldAnalyzer(const E2Config* config);
	virtual void reset();
	virtual void processState(const OdeState* state, double time);
	virtual int getStatesCount();
	virtual ~E2FieldAnalyzer();

	static std::string getDisplayName(){
		return "field plotter for E2";
	}
};

REGISTER_ANALYZER_WIDGET_FACTORY(E2AnalyzerFactory, E2InstanceFactory, E2FieldAnalyzer, E2Config)

#endif /* ANALYZERS_E2_H_ */