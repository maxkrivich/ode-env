/*
 * Gnuplot.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: dimalit
 */

#include "Gnuplot.h"
#include "rpc.h"

#include <sstream>
#include <cstdio>

using namespace google::protobuf;

Gnuplot::Gnuplot() {
	width = 300;
	height = 300;
	title = "";
	x_axis = "";			// time/index
	derivative_x = false;
	style = STYLE_LINES;

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
	to_gnuplot = fdopen(wf, "wb");

	if(!to_gnuplot){
		perror(NULL);
		assert(false);
	}

	update_view();
}

void Gnuplot::processState(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	printPlotCommand(to_gnuplot, msg, d_msg, time);
}

void Gnuplot::printPlotCommand(FILE* fp, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(msg);

	const Descriptor* desc = msg->GetDescriptor();
	const Reflection* refl = msg->GetReflection();
	const Descriptor* d_desc = d_msg ? d_msg->GetDescriptor() : NULL;
	const Reflection* d_refl = d_msg ? d_msg->GetReflection() : NULL;

	// exit if we need derivatives but don't have them
//	if(d_msg == NULL || d_msg->ByteSize() <= 32){	// XXX: means it is "nearly empty"
//		return;
//	}

//	msg->PrintDebugString();
//	d_msg->PrintDebugString();

	//////////////// 1 plot xxx with xxx, yyy with yyy /////////////////

	std::ostringstream plot_command;
	plot_command << "plot ";

	for(int i=0; i<series.size(); i++){
		const serie& s = series[i];
		if(i!=0)
			plot_command << ", ";

		std::string style = this->style == STYLE_LINES ? "lines" : "points ps 0.2";
		string title = s.var_name;
		if(s.derivative)
			title += '\'';
		plot_command << "'-' with " << style << " title '" << title <<"'";
	}// for
	plot_command << "\n";
	fprintf(fp, plot_command.str().c_str());
	fflush(fp);

	//////////// 2 give series ////////////////////
	for(int i=0; i<series.size(); i++){
		serie& s = series[i];

		// simple field
		if(s.var_name.find('.') == std::string::npos){
			// if need time
			if(getXAxisTime()){
				const FieldDescriptor* fd = s.derivative ? d_desc->FindFieldByName(s.var_name) : desc->FindFieldByName(s.var_name);
				assert(fd);
				double val = s.derivative ? d_refl->GetDouble(*d_msg, fd) : refl->GetDouble(*msg, fd);
				s.data_cache.push_back(make_pair(time, val));

				for(int i=0; i<s.data_cache.size(); i++){
					fprintf(fp, "%.10lf %.10lf\n", s.data_cache[i].first, s.data_cache[i].second);
				}// for
			}// if need time
			else{
				const FieldDescriptor* yfd = s.derivative ? d_desc->FindFieldByName(s.var_name) : desc->FindFieldByName(s.var_name);
					assert(yfd);
				const FieldDescriptor* xfd = derivative_x ? d_desc->FindFieldByName(this->x_axis) : desc->FindFieldByName(this->x_axis);
					assert(xfd);
				double yval = s.derivative ? d_refl->GetDouble(*d_msg, yfd) : refl->GetDouble(*msg, yfd);
				double xval = derivative_x ? d_refl->GetDouble(*d_msg, xfd) : refl->GetDouble(*msg, xfd);
				fprintf(fp, "%.10lf %.10lf\n", xval, yval);
			}// not need time
		}// if simple field
		else{	// repeated field
			std::string f1 = s.var_name.substr(0, s.var_name.find('.'));
				assert(f1.size());
			std::string f2 = s.var_name.substr(s.var_name.find('.')+1);
				assert(f2.size());

			const FieldDescriptor* fd1 = desc->FindFieldByName(f1);
			const FieldDescriptor* d_fd1 = d_desc ? d_desc->FindFieldByName(f1) : NULL;
				assert(!d_desc || fd1);
			int n = refl->FieldSize(*msg, fd1);
				assert(!d_fd1 || refl->FieldSize(*msg, fd1) == d_refl->FieldSize(*msg, d_fd1));

			for(int i=0; i<n; i++){
				const Message& m2 = s.derivative ? d_refl->GetRepeatedMessage(*d_msg, d_fd1, i) : refl->GetRepeatedMessage(*msg, fd1, i);
				const Descriptor* d2 = m2.GetDescriptor();
				const Reflection* r2 = m2.GetReflection();
				const FieldDescriptor* fd2 = d2->FindFieldByName(f2);
					assert(fd2);

				double y = r2->GetDouble(m2, fd2);
				double x = i;

				// set x to value of specific x variable if needed
				if(!getXAxisTime()){
					std::string xname = this->x_axis;
					// remove all vefore dot
					if(this->x_axis.find('.') != std::string::npos)
						xname = x_axis.substr(x_axis.find('.')+1);

					const Message& m2 = derivative_x ? d_refl->GetRepeatedMessage(*d_msg, fd1, i) : refl->GetRepeatedMessage(*msg, fd1, i);
					const Descriptor* d2 = m2.GetDescriptor();
					const Reflection* r2 = m2.GetReflection();
					const FieldDescriptor* xfd = d2->FindFieldByName(xname);
					x = r2->GetDouble(m2, xfd);
				}// if not time :(

				fprintf(fp, "%.10lf %.10lf\n", x, y);
			}// for
		}// if repeated field

		fprintf(fp, "e\n");
		fflush(fp);
	}// for
}

void Gnuplot::processToFile(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	fprintf(to_gnuplot, "set terminal png\n");
	fprintf(to_gnuplot, "set output \"%s\"\n", file.c_str());
	processState(msg, d_msg, time);
	fprintf(to_gnuplot, "set terminal x11\n");
	fflush(to_gnuplot);

}

void Gnuplot::saveToCsv(const std::string& file, const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	FILE* fp = fopen(file.c_str(), "wb");
	assert(fp);		// TODO: exception
	fprintf(fp, "set label \"t = %.2lf\" at graph 0.02, graph 0.95\n", time);
	printPlotCommand(fp, msg, d_msg, time);
	fclose(fp);
}

void Gnuplot::addVar(std::string var){
	serie s;
	s.derivative = var[var.size()-1]=='\'';
	s.var_name = s.derivative ? var.substr(0, var.size()-1) : var;
	series.push_back(s);
}
void Gnuplot::eraseVar(int idx){
	assert(idx >= 0 && idx <series.size());
	series.erase(series.begin()+idx);
}

void Gnuplot::writeback() {
	fprintf(to_gnuplot, "set xrange [*:*] writeback\n");
	fprintf(to_gnuplot, "set yrange [*:*] writeback\n");
	fflush(to_gnuplot);
}
void Gnuplot::restore() {
	fprintf(to_gnuplot, "set xrange restore\n");
	fprintf(to_gnuplot, "set yrange restore\n");
	fflush(to_gnuplot);
}

Gnuplot::~Gnuplot() {
	fprintf(to_gnuplot, "exit\n");
	fflush(to_gnuplot);
	fclose(to_gnuplot);
}

void Gnuplot::update_view(){
	fprintf(to_gnuplot, "set terminal x11 size %d, %d title \"%s\"\n", width, height, title.c_str());
	fprintf(to_gnuplot, "set xrange [*:*] writeback\n");
	fprintf(to_gnuplot, "set yrange [*:*] writeback\n");
	fflush(to_gnuplot);
}