/*
 * ChartAnalyzer.h
 *
 *  Created on: Jan 29, 2015
 *      Author: dimalit
 */

#ifndef CHARTANALYZER_H_
#define CHARTANALYZER_H_

#include "gui_interfaces.h"
#include "Gnuplot.h"
#include "model_e3.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <vector>

class ChartAnalyzer: public OdeAnalyzerWidget {
public:
	typedef E3Config Config;
	typedef E3State State;
private:
	std::vector<Gnuplot*> plots;
	int states_count;

	Gtk::VBox vbox;
	Gtk::Button btn_add;
	Gtk::Button btn_reset;

	const OdeState *last_state, *last_d_state;
	double last_time;
public:
	ChartAnalyzer(const OdeConfig* config);
	virtual void loadConfig(const OdeConfig* config){}
	virtual void reset();
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual int getStatesCount();
	virtual ~ChartAnalyzer();

	void addChart(std::vector<std::string> vars, std::string x_axis_var);

	static std::string getDisplayName(){
		return "customizable chart";
	}
private:
	void on_save_clicked(Gnuplot* ptr);
	void on_add_clicked();
	void on_del_chart_clicked(Gtk::Widget* w, const Gnuplot* ptr);
	void on_writeback_clicked(Gnuplot* ptr);
	void on_restore_clicked(Gnuplot* ptr);
};

//REGISTER_ANALYZER_WIDGET_CLASS(ChartAnalyzer)

#endif /* CHARTANALYZER_H_ */