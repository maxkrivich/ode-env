/*
 * gui_e3.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#include "gui_e3.h"

#include <gtkmm/builder.h>

#define UI_FILE_CONF "e3_config.glade"
#define UI_FILE_STATE "e3_state.glade"
#define UI_FILE_PETSC_SOLVER "e3_petsc_solver.glade"

E3ConfigWidget::E3ConfigWidget(const E3Config* cfg){
	if(cfg)
		this->config = new E3Config(*cfg);
	else
		this->config = new E3Config();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CONF);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_m", entry_m);
	b->get_widget("entry_n", entry_n);
	b->get_widget("entry_theta", entry_theta);
	b->get_widget("entry_gamma", entry_gamma);
	b->get_widget("entry_delta", entry_delta);
	b->get_widget("entry_r", entry_r);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	config_to_widget();

	entry_m->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_n->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_theta->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_gamma->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_delta->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));
	entry_r->signal_changed().connect(sigc::mem_fun(*this, &E3ConfigWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E3ConfigWidget::on_apply_cb));
}

void E3ConfigWidget::widget_to_config(){
	config->set_m(atoi(entry_m->get_text().c_str()));
	config->set_n(atof(entry_n->get_text().c_str()));
	config->set_theta_e(atof(entry_theta->get_text().c_str()));
	config->set_delta_e(atof(entry_delta->get_text().c_str()));
	config->set_gamma_0_2(atof(entry_gamma->get_text().c_str()));
	config->set_r_e(atof(entry_r->get_text().c_str()));

	button_apply->set_sensitive(false);
}
void E3ConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->m();
	entry_m->set_text(buf.str());

	buf.str("");
	buf << config->n();
	entry_n->set_text(buf.str());

	buf.str("");
	buf << config->theta_e();
	entry_theta->set_text(buf.str());

	buf.str("");
	buf << config->gamma_0_2();
	entry_gamma->set_text(buf.str());

	buf.str("");
	buf << config->delta_e();
	entry_delta->set_text(buf.str());

	buf.str("");
	buf << config->r_e();
	entry_r->set_text(buf.str());

	button_apply->set_sensitive(false);
}

void E3ConfigWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}

void E3ConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

void E3ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E3Config* ecfg = dynamic_cast<const E3Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E3Config(*ecfg);
	config_to_widget();

	m_signal_changed.emit();
}

const OdeConfig* E3ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}

E3StateWidget::E3StateWidget(const E3Config* _config, const E3State* _state){
	if(_config)
		this->config = new E3Config(*_config);
	else
		this->config = new E3Config();

	if(_state)
		this->state = new E3State(*_state);
	else
		// TODO: may be state should remember its config?!
		this->state = new E3State(this->config);

	this->d_state = new E3State();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_STATE);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_e", entry_e);
	b->get_widget("entry_phi", entry_phi);
	b->get_widget("entry_a", entry_a);

	b->get_widget("button_apply", button_apply);

	this->add(*root);

	state_to_widget();
	if(!this->state->simulated())
		generateState();

	// signals
	entry_e->signal_changed().connect(sigc::mem_fun(*this, &E3StateWidget::edit_anything_cb));
	entry_phi->signal_changed().connect(sigc::mem_fun(*this, &E3StateWidget::edit_anything_cb));
	entry_a->signal_changed().connect(sigc::mem_fun(*this, &E3StateWidget::edit_anything_cb));

	button_apply->signal_clicked().connect(sigc::mem_fun(*this, &E3StateWidget::on_apply_cb));
}

E3StateWidget::~E3StateWidget(){
	// XXX where are deletes?
}

void E3StateWidget::widget_to_state(){
	if(!state->simulated()){
		generateState();
	}
}
void E3StateWidget::state_to_widget(){
	std::ostringstream buf;

	buf.str("");
	buf << state->e();
	entry_e->set_text(buf.str());

	buf.str("");
	buf << state->phi();
	entry_phi->set_text(buf.str());

	if(state->simulated()){
		entry_a->set_text("");
	}
	else{
		buf.str("");
		buf << state->particles(0).a();
		entry_a->set_text(buf.str());
	}
}

void E3StateWidget::edit_anything_cb(){
	button_apply->set_sensitive(true);
}
void E3StateWidget::on_apply_cb(){
	generateState();
	m_signal_changed.emit();
}


void E3StateWidget::loadConfig(const OdeConfig* cfg){
	const E3Config* ecfg = dynamic_cast<const E3Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E3Config(*ecfg);

	delete this->state;
	this->state = new E3State(ecfg);
	delete this->d_state;
	this->d_state = new E3State();

	state_to_widget();
	widget_to_state();

	m_signal_changed.emit();
}
const OdeConfig* E3StateWidget::getConfig(){
	return config;
}

void E3StateWidget::loadState(const OdeState* state, const OdeState* d_state){
	const E3State* estate = dynamic_cast<const E3State*>(state);
		assert(estate);
	const E3State* d_estate = dynamic_cast<const E3State*>(d_state);
		assert(d_estate);
	delete this->state;
	delete this->d_state;
	this->state = new E3State(*estate);
	this->d_state = new E3State(*d_estate);

	state_to_widget();
	widget_to_state();

	m_signal_changed.emit();
}
const OdeState* E3StateWidget::getState(){
	widget_to_state();
	return state;
}

const OdeState* E3StateWidget::getDState(){
	return d_state;
}

void E3StateWidget::generateState(){
	double e = atof(entry_e->get_text().c_str());
	double phi = atof(entry_phi->get_text().c_str());
	double a = atof(entry_a->get_text().c_str());

	bool use_rand = false;
	double right = 0.5;
	double left = -0.5;

	int m = config->m();
	for(int i=0; i<m; i++){
		double ksi;
		if(use_rand)
			ksi = rand() / (double)RAND_MAX * (right-left) + left;
		else
			ksi = i / (double)m * (right-left) + left;

		pb::E3State::Particles p;
		p.set_ksi(ksi); p.set_a(a);
		p.set_eta(0.0);

		state->mutable_particles(i)->CopyFrom(p);
	}

	state->set_e(e);
	state->set_phi(phi);
	state->set_simulated(false);
}

E3PetscSolverConfigWidget::E3PetscSolverConfigWidget(const E3PetscSolverConfig* config){
	if(config)
		this->config = new E3PetscSolverConfig(*config);
	else
		this->config = new E3PetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_tol", entry_tol);
	b->get_widget("entry_step", entry_step);

	this->add(*root);

	config_to_widget();
}
const OdeSolverConfig* E3PetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}
void E3PetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const E3PetscSolverConfig* econfig = dynamic_cast<const E3PetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new E3PetscSolverConfig(*econfig);
	config_to_widget();
}

void E3PetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_tolerance(atof(entry_tol->get_text().c_str()));
}
void E3PetscSolverConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->init_step();
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->tolerance();
	entry_tol->set_text(buf.str());
}
