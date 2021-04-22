/*
	Copyright (C) 2021 Arc, tomichec (Tomas Stary)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "network.h"

namespace network {
	float exchange_rate;
	bool has_exchange_rate = false;
	unsigned long last_fetched_exchange_rate = 0;
	const unsigned long exchange_rate_update_interval = 5*60*1000; // 5 minutes

	bool paid = false;
	bool down = false;

	String dataId = "";

	const char* amazon_root_ca = \
		"-----BEGIN CERTIFICATE-----\n" \
		"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
		"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
		"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
		"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
		"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
		"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
		"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
		"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
		"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
		"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
		"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
		"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
		"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
		"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
		"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
		"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
		"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
		"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
		"-----END CERTIFICATE-----\n";

	const char* letsencrypt_root_ca= \
		"-----BEGIN CERTIFICATE-----\n" \
		"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n" \
		"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
		"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n" \
		"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n" \
		"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
		"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n" \
		"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n" \
		"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n" \
		"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n" \
		"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n" \
		"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n" \
		"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n" \
		"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n" \
		"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n" \
		"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n" \
		"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n" \
		"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n" \
		"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
		"-----END CERTIFICATE-----\n";

	void loop(){
		if (( ! has_exchange_rate || (millis() - last_fetched_exchange_rate) > exchange_rate_update_interval) ){
			fetchExchangeRate(config::get_currency());
		}
	}

    //////////////////OPENNODE CALL///////////////////
	void fetchExchangeRate(std::string fiatCurrency){

		WiFiClientSecure client;
		client.setCACert(amazon_root_ca);

		if (!client.connect("api.opennode.com", 443)) {
			Serial.println("failed to connect to api.opennode.com");
			return;
		}

		String url = "/v1/rates";
		client.print(String("GET ") + url + " HTTP/1.1\r\n" +
					 "Host: api.opennode.com\r\n" +
					 "User-Agent: ESP32\r\n" +
					 "Connection: close\r\n\r\n");
		while (client.connected()) {
			String line = client.readStringUntil('\n');
			if (line == "\r") {
				break;
			}
		}
		String line = client.readStringUntil('\n');
		const size_t capacity = 169*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(168) + 3800;
		DynamicJsonDocument doc(capacity);
		deserializeJson(doc, line);
		Serial.println(line);
		exchange_rate = doc["data"]["BTC" + String(fiatCurrency.c_str())][fiatCurrency.c_str()]; 
		Serial.println("Exchange rate is:");
		Serial.println(exchange_rate);

		if (exchange_rate > 0.0) {
			has_exchange_rate = true;
			last_fetched_exchange_rate = millis();
		} else {
			Serial.println("exchange rate has a weird value");
		}

	}

	double get_exchange_rate(){
		if (has_exchange_rate) {
			return exchange_rate;
		}
	}

	void connect_lnbits(){
	}

    //////////////////LNBITS CALLS///////////////////
	std::string getinvoice(String nosats) 
	{
		WiFiClientSecure client;
		client.setCACert(letsencrypt_root_ca);

		const char* lnbitsserver = config::get_lnbits_server();
		const char* invoicekey = config::get_invoice_key();
		const char* lnbitsdescription = config::get_lnbits_description();

		if (!client.connect(lnbitsserver, 443)){
			Serial.println("failed to connect to lnbits server");
			Serial.println("Make sure the server is running and you have set ssl root certificate correctly. For more info see: ");
			Serial.println("https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/examples/WiFiClientSecure/WiFiClientSecure.ino");
			down = true;
			return "false";   
		}

		String topost = "{\"out\": false,\"amount\" : " + nosats + ", \"memo\" :\""+ String(lnbitsdescription) + String(random(1,1000)) + "\"}";
		String url = "/api/v1/payments";
		client.print(String("POST ") + url +" HTTP/1.1\r\n" +
					 "Host: " + lnbitsserver + "\r\n" +
					 "User-Agent: ESP32\r\n" +
					 "X-Api-Key: "+ invoicekey +" \r\n" +
					 "Content-Type: application/json\r\n" +
					 "Connection: close\r\n" +
					 "Content-Length: " + topost.length() + "\r\n" +
					 "\r\n" + 
					 topost + "\n");
		while (client.connected()) {
			String line = client.readStringUntil('\n');
			if (line == "\r") {
				break;
			}
			if (line == "\r") {
				break;
			}
		}
  
		String line = client.readString();

		StaticJsonDocument<1000> doc;
		DeserializationError error = deserializeJson(doc, line);
		if (error) {
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.f_str());
			return "failed";
		}
		const char* payment_hash = doc["checking_id"];
		const char* payment_request = doc["payment_request"];
		dataId = payment_hash;

		return payment_request;
	}


	bool checkinvoice()
	{
		WiFiClientSecure client;
		client.setCACert(letsencrypt_root_ca);

		const char* lnbitsserver = config::get_lnbits_server();
		const char* invoicekey = config::get_invoice_key();
		if (!client.connect(lnbitsserver, 443)){
			Serial.println("failed to connect to lnbits server");
			down = true;
			return false;   
		}

		String url = "/api/v1/payments/";
		client.print(String("GET ") + url + dataId +" HTTP/1.1\r\n" +
					 "Host: " + lnbitsserver + "\r\n" +
					 "User-Agent: ESP32\r\n" +
					 "X-Api-Key:"+ invoicekey +"\r\n" +
					 "Content-Type: application/json\r\n" +
					 "Connection: close\r\n\r\n");
		while (client.connected()) {
			String line = client.readStringUntil('\n');
			if (line == "\r") {
				break;
			}
			if (line == "\r") {
				break;
			}
		}
		String line = client.readString();
		StaticJsonDocument<200> doc;
		DeserializationError error = deserializeJson(doc, line);
		if (error) {
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(error.f_str());
			return false;
		}
		bool charPaid = doc["paid"];
		return charPaid;
	}

	bool hasExchangeRate(){
		return has_exchange_rate;
	}
}
