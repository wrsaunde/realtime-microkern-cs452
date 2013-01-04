#include <userspace.h>


//Display information about a route to the screen
void display_route(struct track_node * track, struct train_route * route) {
	struct print_buffer pbuff;
	int i = 0, j = 0, s = 0;
	int distance;
	ap_init_buff(&pbuff);
	ap_printf(&pbuff, "PRIMARY ");
	int psdist = 0, sensor = 0;

	for (i = 0; i < route->primary_size && i < 20; i++) {
		ap_printf(&pbuff, "%s (%d:%d:%d:%d) ", track[route->primary[i].node].name, route->primary[i].distance,
				route->primary[i].edge, route->primary[i].secondary_route_index, route->primary[i].length_reserved);
	}
	if (pbuff.mem[0] != 0) {
		CommandOutput(pbuff.mem);
		ap_init_buff(&pbuff);
	}

	ap_printf(&pbuff, "SECONDARY ");
	for (i = 0; i < route->primary_size && i < 20; i++) {
		s = route->primary[i].secondary_route_index;
		if (s != -1) {
			ap_printf(&pbuff, "(%s): ", track[route->primary[i].node].name);
			distance = first_secondary_sensor_distance(track, route, route->primary[i].secondary_route_index, &sensor);
			ap_printf(&pbuff, "[%s:%d]: ", track[sensor].name,distance);
			for (j = 0; j < route->secondary_size[s]; j++) {
				ap_printf(&pbuff, "%s (%d:%d:%d) ", track[route->secondary[s][j].node].name,
						route->secondary[s][j].distance, route->secondary[s][j].edge,route->secondary[s][j].length_reserved);
			}
			ap_printf(&pbuff, "} ");
		}
	}

	if (pbuff.mem[0] != 0) {
		CommandOutput(pbuff.mem);
		ap_init_buff(&pbuff);
	}

	//Delay(500);

	struct position pos;
	pos.node = route->primary[0].node;
	pos.offset = 0;
	int sensors[10], distances[10], primary[10], numsensors = 0;

	ap_printf(&pbuff, "Guaranteed length: %d, Sensors: ", route_guaranteed_length(track, &pos, route, -1));
	numsensors = predict_sensors_quantum(track, &pos, route, sensors, distances, primary, -1);
	for (i = 0; i < numsensors; i++) {
		ap_printf(&pbuff, " %s(%d) ", track[sensors[i]].name, distances[i]);
	}

	/*
	ap_printf(&pbuff, "SENSORS ");
	for (i = 0; i < route->primary_size && i < 20; i++) {
		pos.node = route->primary[i].node;
		pos.offset = 0;
		psdist = next_primary_sensor_distance(track, &pos, route, &sensor);
		CommandOutput("%d %s %d %d", i, track[route->primary[i].node].name, psdist, sensor);
		Delay(100);
	}
	*/

	CommandOutput(pbuff.mem);
	ap_init_buff(&pbuff);
	//set_upcoming_switches(track, route, &pos, 600);
}

