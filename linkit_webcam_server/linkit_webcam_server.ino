#include <wrapper_linkit_wifi.h>
#include <wrapper_linkit_cam.h>
#include <LTask.h>
#include <LWiFiServer.h>
#include <LWiFi.h>

LinkItWifi wifi;
LinkItCam cam;

void setup() {
	if (DEBUG) Serial.begin(115200);
	delay(500);
	if (DEBUG) Serial.println("begin");
	wifi.content_id = CONTENT_FRAME;
	wifi.init();
	//if (wifi.content_id == CONTENT_IMAGE) 
		cam.init();
}

String link(String url, String label) {
	String str_link = "";
	str_link += "<a href ='";
	str_link += url;
	str_link += "'>";
	str_link += label;
	str_link += "</a>";
	return str_link;
}

void loop_html() {
	wifi.html_body = "";
	wifi.html_body += "MILLIS:";
	wifi.html_body += millis();
	wifi.html_body += "<br />";
	wifi.html_body += link("/capture", "Capture Image");
}

void loop_image() {
	if (wifi.client_connected == CONTENT_IMAGE) {
		wifi.data_length = cam.pic_total_length;
		cam.capture();
		wifi.html_init();
		wifi.html_build();
		wifi.client.print(wifi.client_body);
		cam.send(wifi.client);
		wifi.client.stop();
		wifi.content_id = CONTENT_HTML;
	}
}

void loop() {
	if (wifi.content_id != CONTENT_IMAGE) {
		wifi.web();
		wifi.html_init();
		loop_html();
	} else {
		loop_image();
	}
}
