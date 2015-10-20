#include <wrapper_linkit_gps.h>
#include <wrapper_linkit_wifi.h>
#include <wrapper_linkit_cam.h>
#include <LTask.h>
#include <LWiFiServer.h>
#include <LWiFi.h>
#include <LBattery.h>
#include <LGPS.h>

LinkItWifi wifi;
LinkItCam cam;
LinkItGPS globe;

#define GRAPH_BATTERY_LEVEL		0
#define GRAPH_BATTERY_CHARGING	1
#define GRAPH_BATTERY_TIME		2
#define GRAPH_BATTERY_FIX		3
#define GRAPH_BATTERY_SAT		4

#define TABLE_ROWS	100
#define TABLE_COLS	2
String table_data[TABLE_ROWS][TABLE_COLS];
int table_row = 0;
int graph_battery_level[256];
int graph_battery_charging[256];
int graph_time[256];
int graph_fix[256];

void setup_wifi() {
	wifi.html_title = "";
	wifi.html_title += "LinkIt One DASHBOARD";
	wifi.content_id = CONTENT_HTML;
	wifi.init();
	wifi.html_init();
}

void setup() {
	if (DEBUG) Serial.begin(115200);
	delay(500);
	if (DEBUG) Serial.println("begin");
	//	setup_display();
	setup_wifi();
	cam.init();
	globe.init();
	graph_battery_level[0] = 100;
	graph_battery_charging[0] = 100;
	graph_time[0] = 100;
	graph_fix[0] = 100;
}

void table_clear() {
	for (int i = 0; i < TABLE_ROWS; i++)
		for (int j = 0; j < TABLE_COLS; j++)
			table_data[i][j] = "";
	table_row = 0;
}

String table_view() {
	String temp_out = "";
	String temp_row = "";
	String temp_class = "";
	for (int i = 0; i < TABLE_ROWS; i++) {
		temp_row = "";
		for (int j = 0; j < TABLE_COLS; j++) {
			if (table_data[i][j] != "") {
				temp_class = "cell";
				if (table_data[i][1] == "*") {
					table_data[i][1] = "";
					temp_class += " bold light";
				}
				else {
					if (j == 0) {
						temp_class += " right dark";
					}
				}
				temp_row += wifi.span("", table_data[i][j], temp_class);
			}
		}
		if (temp_row != "") {
			temp_out += wifi.span("", temp_row, "row");
			temp_row = "";
		}
	}
	//	temp_out += "<br />";
	return temp_out;
}

void table_add(String label, String data) {
	table_data[table_row][0] += label;
	table_data[table_row][1] += data;
	//if (table_data[table_row][1] == "") {
	//	table_data[table_row][1] += "-";
	//}
	table_row++;
}

void loop_html() {
	String temp_out = "";
	wifi.html_body = "";
	table_clear();

	table_add("BATTERY", "*");
	table_add("Level:", (String)LBattery.level());
	table_add("Charging:", (String)LBattery.isCharging());


	table_add("GPS", "*");
	table_add("Date:", (String)globe.view[S_DATE]);
	table_add("Time:", (String)globe.view[S_TIME]);
	table_add("Fixation:", (String)globe.view[S_FIX]);
	//table_add("Satelites:",	(String)globe.view[S_SAT]);
	//table_add("Positon:",		(String)globe.view[S_POSITION]);
	table_add("Longtude:", (String)globe.view[S_LONGITUDE]);
	table_add("Latutude:", (String)globe.view[S_LATITUDE]);

	table_add("SYSTEM", "*");
	table_add("Millis:", (String)millis());

	table_add("SENSOR", "*");
	table_add("Capture Image:", wifi.link("/capture", "Serial camera"));


	temp_out += wifi.span("", table_view(), "cell");

	table_clear();
	table_add("GRAPH", "*");
	table_add("Power", graph_view(GRAPH_BATTERY_LEVEL));
	table_add("Charging", graph_view(GRAPH_BATTERY_CHARGING));
	table_add("Fix", graph_view(GRAPH_BATTERY_FIX));
	table_add("Time", graph_view(GRAPH_BATTERY_TIME));

	temp_out += wifi.span("", table_view(), "cell");
	wifi.html_body += wifi.span("", temp_out, "row");
}

void loop_image() {
	//if (wifi.client_connected == 1) {
	wifi.data_length = cam.pic_total_length;
	cam.capture();
	wifi.html_build();
	wifi.client.print(wifi.client_body);
	//if (DEBUG) {
	//	Serial.print("clientbody:");
	//	Serial.println(wifi.client_body);
	//}
	cam.send(wifi.client);
	wifi.client.stop();
	//}
	wifi.content_id = CONTENT_HTML;
	wifi.client_connected = 0;
	if (DEBUG) {
		Serial.println("content html");
	}
}

void loop_text() {
	//loop_html();
	wifi.web();
}

void graph_update() {
	for (int i = 1; i < 255; i++) {
		graph_time[i] = graph_time[i + 1];
		graph_fix[i] = graph_fix[i + 1];
		graph_battery_level[i] = graph_battery_level[i + 1];
		graph_battery_charging[i] = graph_battery_charging[i + 1];
	}
	graph_time[255] = ((globe.second * (100.0 / 60.0)) + 1);
	graph_fix[255] = (globe.fix * 99) + 1;
	graph_battery_level[255] = LBattery.level();
	graph_battery_charging[255] = (LBattery.isCharging() * 99) + 1;
	//graph_battery[255] = (100 - globe.second);
	//	graph_battery[255] = sin(globe.second / 100.0) * 100;
	//	graph_battery[255] += 50;
}

String graph_view(byte id) {
	String temp_out = "";
	String temp_style = "";
	String temp_class = "";
	for (int i = 0; i < 256; i++) {
		temp_style = "height:";
		temp_class = "graph";
		switch (id) {
		case 0:
			temp_style += graph_battery_level[i];
			if (graph_battery_level[i] < 33) {
				temp_class += " red";
			}
			else if (graph_battery_level[i] < 66) {
				temp_class += " yellow";
			}
			else {
				temp_class += " green";
			}
			break;
		case 1:
			temp_style += graph_battery_charging[i];
			if (graph_battery_charging[i] == 0) {
				temp_class += " yellow";
			}
			else {
				temp_class += " orange";
			}
			break;
		case 2:
			temp_style += graph_time[i];
			temp_class += " blue";
			break;
		case 3:
			temp_style += graph_fix[i];
			if (graph_fix[i] == 0) {
				temp_class += " magenta";
			}
			else {
				temp_class += " violet";
			}
			break;
		}
		temp_style += "px";
		temp_out += "<span class='";
		temp_out += temp_class;
		temp_out += "' style='";
		temp_out += temp_style;
		temp_out += "'>";
		temp_out += "</span>";
	}
	return wifi.span("", temp_out, "board");
}

void loop_gps() {
	globe.update();
	if (globe.second != globe.second_last) {
		if (DEBUG) {
			Serial.println(globe.view[S_TIME]);
		}
		graph_update();
		loop_html();
	}
}

void loop() {
	if (wifi.content_id != CONTENT_IMAGE) {
		loop_gps();
		wifi.web();
	}
	else {
		loop_image();
	}
}
