
#pragma once
#ifndef _WifiFunc_h
#define _WifiFunc_h


#include <TimerMs.h>
#include <GyverDBFile.h>
#include <LittleFS.h>

#define WIFIAPTIMER 180000


enum wifi : size_t {
    ssid = SH("wifiSsid"),
    password = SH("wifiPassword"),
    forceAP = SH("wifiForceAP"),
};

GyverDBFile* _wifiDb = nullptr;
void setDb(GyverDBFile* db = nullptr){
    _wifiDb = db;
}


TimerMs WiFiApTimer;

void wifiAPTimerHandler(){
    Serial.println("Restart ESP by AP timer");
    _wifiDb->set(wifi::forceAP, false);
    _wifiDb->update();
    ESP.reset();
}


void wifiAp(const String& deviceName = "ESP"){
  Serial.println("Starting AP mode");

  String ssid = deviceName;
  ssid += " AP";
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip.toString());

  // WiFiAP timer
  Serial.println("Starting WiFiAP timer");
  WiFiApTimer.setTime(WIFIAPTIMER);
  WiFiApTimer.setTimerMode();
  WiFiApTimer.attach(wifiAPTimerHandler);
  WiFiApTimer.start();
}


void wifiConnect(const String& deviceName = "ESP"){
    WiFi.mode(WIFI_STA);
    WiFi.hostname(deviceName);

    Serial.println("Wifi connecting...");
    WiFi.begin(_wifiDb->get(wifi::ssid), _wifiDb->get(wifi::password));

    int tries = 20;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
        if (!--tries){
            WiFi.mode(WIFI_AP);
            break;
        };
    };

    if (WiFi.status() == WL_CONNECTED){
        Serial.println();
        Serial.print("WiFi connected to SSID ");
        Serial.print(_wifiDb->get(wifi::ssid));
        Serial.println(" successfully");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
    } else {
        Serial.println();
        Serial.print("Can't connect to SSID ");
        Serial.print(_wifiDb->get(wifi::ssid));
        Serial.println(" reset to AP mode");
        _wifiDb->set(wifi::forceAP, true);
        _wifiDb->update();

        delay(3000);
        ESP.reset();
    }
}


void wifiSetup(const String& deviceName = "ESP", GyverDBFile* db = nullptr){
    Serial.println("-----------------------------");
    Serial.println("Initialize WiFi");

    setDb(db);

    _wifiDb->init(wifi::ssid, "");
    _wifiDb->init(wifi::password, "");
    _wifiDb->init(wifi::forceAP, true);

    #ifdef DEBUG_DB
        _wifiDb->dump(Serial);
        Serial.println("");
    #endif

    // Connecting WiFi
    Serial.print("WiFi force AP: ");;
    Serial.println(_wifiDb->get(wifi::forceAP));
    if (_wifiDb->get(wifi::forceAP) == true ) {
        wifiAp(deviceName);
    } else {
        wifiConnect(deviceName);
    }
};


void wifiLoop(){
    WiFiApTimer.tick();
};


void wifiReset(){
    _wifiDb->set(wifi::forceAP,false);
    ESP.reset();
}

#endif
