#include <Arduino.h>
#include "utilities/blink.h"
#include "utilities/vector.h"
#include "system_graph/process.h"
#include "system_graph/graph_node.h"
#include "system_graph/system_graph.h"

SystemGraph::SystemGraph() {
	lifetime = 0;
	watch.set(0);
	status.reset(0);
	nodes.reset(0);
	node_ids.reset(0);

	setup_blink();
}

void SystemGraph::collect_outputs(int index, Vector<float>* data) {
	Vector<int> ids = *nodes[index]->input_ids();
	for (int i = 0; i < ids.size(); i++) {
		int node_id = node_ids.find(ids[i]);
		Serial.printf("node id %i\n", node_id);
		if (node_id >= 0) {
			data->append(nodes[node_id]->output());
			Serial.print("output "); nodes[node_id]->output()->print();	
		}
	}
	data->print();
}

void SystemGraph::add(String proc_id, int id, int n_configs, int n_inputs, int* inputs) {
	// status.push(0);
	int index = node_ids.find(id);
	if (index == -1) {
		node_ids.push(id);
		nodes.push(new GraphNode(factory.new_proc(proc_id), n_configs, n_inputs, inputs));

		if (n_configs == 0) {
			status.push(1);
		}
		else {
			status.push(0);
		}
	}
	else {
		nodes[index]->set_config(n_configs);
		// nodes[index]->set_inputs(inputs);
		// nodes[index]->set_process(factory.new_proc(proc_id)); // Doesn't seem to reinit imu but it should... imu init takes forever though so whatever
	}
}

void SystemGraph::update_config(int id, int chunk_id, int n_configs, float* config) {
	int node_index = node_ids.find(id);
	if (node_index == -1) {
		Serial.printf("Node not found %i\n", id);
		return;
	}
	nodes[node_index]->config()->insert(config, chunk_id * n_configs, n_configs);
	if (nodes[node_index]->is_configured()) {
		status[node_index] = 1;
		nodes[node_index]->setup_proc();
	}
}

void SystemGraph::spin() {
	Vector<float> input(0);
	for (int i = 0; i < nodes.size(); i++) {
		if (status[i] == 1) {
			collect_outputs(i, &input);
			Serial.print("inputs ");
			input.print();
			// nodes[i]->run_proc(&input);
		}
	}
}

void SystemGraph::dump_all() {
	Serial.printf("lifetime: %f\n", lifetime);
	Serial.print("Node ids\n\t");
	node_ids.print();
	for (int i = 0; i < nodes.size(); i++) {
		nodes[i]->print_proc();
		nodes[i]->print_output();
	}
}

void SystemGraph::handle_hid() {
	switch (report.read()) {
		case 64:
			blink();										// only blink when connected to hid
			// Serial.printf("[%f]Report %i, %i, %i\n", lifetime, report.get(0), report.get(1), report.get(2));
			switch (report.get(0)) {
				case 255:
					// report.print();
					switch (report.get(1)) {
						case 1:
							init_process_hid();
							break;

						case 2:
							config_process_hid();
							break;

						default:
							break;
					}
					lifetime = 0;
					break;

				case 1:
					switch (report.get(1)) {
						case 1:
							// process_state_hid();
							break;

						case 2:
							// process_output_hid();
							break;

						default:
							break;
					}
					break;

				default:
					break;
			}
			break;
		
		default:
			// Serial.println("No report");
			break;
	}
	lifetime += US_2_S(watch.delay_micros(0, 1E3));
	report.put_float(60, lifetime);
	report.write();
	watch.set(0);
}

void SystemGraph::init_process_hid() {
	String key = "";
	int id = report.get(2);
	key += char(report.get(3));
	key += char(report.get(4));
	key += char(report.get(5));
	int n_config = report.get(6);
	int n_inputs = report.get(7);
	Serial.printf("new process %i: ", id);
	Serial.print(key);
	Serial.printf(" %i %i\n", n_config, n_inputs);
	int input_ids[n_inputs];
	for (int i = 0; i < n_inputs; i++) {
		input_ids[i] = report.get(i + 8);
	}
	add(key, id, n_config, n_inputs, input_ids);
}

void SystemGraph::config_process_hid() {
	int id = report.get(2);
	int chunk_num = report.get(3);
	int chunk_size = report.get(4);
	Serial.printf("configure %i, %i, %i\n",id, chunk_num, chunk_size);

	float data[chunk_size];
	for (int i = 0; i < chunk_size; i++) {
		data[i] = report.get_float((4 * i) + 5);
	}
	update_config(id, chunk_num, chunk_size, data);
}

void SystemGraph::process_state_hid() {
	int id = report.get(2);
	// Vector<float>* state = nodes[id]->context();
	// report.put(3, state->size());
	// for (int i = 0; i < state->size(); i++){
	// 	report.put_float((4 * i) + 4, (*state)[i]);
	// }
	report.put(3, 5);
	for (int i = 0; i < 5; i++){
		report.put_float((4 * i) + 4, lifetime); // debug
	}
}

void SystemGraph::process_output_hid() {
	int id = report.get(2);
	// Vector<float>* output = nodes[id]->output();
	// for (int i = 0; i < output->size(); i++){
	// 	report.put_float((4 * i) + 3, (*output)[i]);
	// }
	for (int i = 0; i < 5; i++){
		report.put_float((4 * i) + 3, lifetime); // debug
	}
}
