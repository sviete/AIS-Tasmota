/*
  support_network.ino - Network support for Tasmota

  Copyright (C) 2020  Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************************\
 * MDNS
\*********************************************************************************************/

struct {
  uint8_t begun = 0;                  // mDNS active
} Mdns;

/*********************************************************************************************/
void MqttDiscoverServer(void) {
  AddLog_P2(LOG_LEVEL_INFO, "AIS EASY: Wykrywam MQTT Host dla bramki: %s", mqtt_client);
  HTTPClient http;
  http.begin(AIS_WS_URL);
  http.setAuthorization(AIS_WS_USER, AIS_WS_PASS);
  http.addHeader("id", String(mqtt_client));
  http.addHeader("ip", WiFi.localIP().toString().c_str());
  int httpCode = http.POST("");
  if (httpCode > 0) {
    String line = http.getString();
      int s = line.indexOf("###a###");
      int e = line.indexOf("###d###");
      int ts = line.indexOf("###t###");
      int te = line.indexOf("###e###");
      if ((s > -1) && (e > 0)){
          // Save IP from AI-Speaker only if the IP is diferend then ais-dom
          if (String(line.substring(s+9, e).c_str()) != "ais-dom") {
            SettingsUpdateText(SET_MQTT_HOST, line.substring(s+7, e).c_str());
            String user = line.substring(e+7, ts);
            if (user != ""){
              SettingsUpdateText(SET_MQTT_USER, user.c_str());
            }
            String token = line.substring(ts+7, te);
            if (token != ""){
              SettingsUpdateText(SET_MQTT_PWD, token.c_str());
            }
            AddLog_P2(LOG_LEVEL_INFO, "AIS EASY: MQTT Host dla bramki wykryty");
          } else {
            AddLog_P2(LOG_LEVEL_INFO, "AIS EASY: MQTT Host dla bramki %s NIE wykryty.", SettingsText(SET_MQTT_USER));
          }
        } else {
          snprintf_P(log_data, sizeof(log_data), line.c_str());
          AddLog(LOG_LEVEL_DEBUG_MORE);
      }
  }
  else {
      snprintf_P(log_data, sizeof(log_data), "AIS EASY: Error on HTTP request.");
      AddLog(LOG_LEVEL_INFO);
  }

  // Disconnect
  http.end();

  snprintf_P(log_data, sizeof(log_data), "AIS EASY: Rozłączono.");
  AddLog(LOG_LEVEL_INFO);
}
/*********************************************************************************************/


#ifdef USE_DISCOVERY
void StartMdns(void) {
//  static uint8_t mdns_delayed_start = Settings.param[P_MDNS_DELAYED_START];

  if (Settings.flag3.mdns_enabled) {  // SetOption55 - Control mDNS service
    if (!Mdns.begun) {
//      if (mdns_delayed_start) {
//        AddLog_P(LOG_LEVEL_INFO, PSTR(D_LOG_MDNS D_ATTEMPTING_CONNECTION));
//        mdns_delayed_start--;
//      } else {
//        mdns_delayed_start = Settings.param[P_MDNS_DELAYED_START];
        Mdns.begun = (uint8_t)MDNS.begin(TasmotaGlobal.hostname);
        AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_MDNS "%s"), (Mdns.begun) ? D_INITIALIZED : D_FAILED);
//      }
    }
  }
}

#ifdef WEBSERVER_ADVERTISE
void MdnsAddServiceHttp(void) {
  if (1 == Mdns.begun) {
    Mdns.begun = 2;
    MDNS.addService("http", "tcp", WEB_PORT);
    MDNS.addServiceTxt("http", "tcp", "devicetype", "tasmota");
  }
}

void MdnsUpdate(void) {
  if (2 == Mdns.begun) {
    MDNS.update();
    AddLog_P(LOG_LEVEL_DEBUG_MORE, D_LOG_MDNS, "MDNS.update");
  }
}
#endif  // WEBSERVER_ADVERTISE
#endif  // USE_DISCOVERY

/*********************************************************************************************\
 * Global network parameters
\*********************************************************************************************/

char* NetworkHostname(void) {
#ifdef ESP32
#ifdef USE_ETHERNET
  if (!TasmotaGlobal.global_state.eth_down) {
    return EthernetHostname();
  }
#endif
#endif
  return TasmotaGlobal.hostname;
}

IPAddress NetworkAddress(void) {
#ifdef ESP32
#ifdef USE_ETHERNET
  if (!TasmotaGlobal.global_state.eth_down) {
    return EthernetLocalIP();
  }
#endif
#endif
  return WiFi.localIP();
}

String NetworkMacAddress(void) {
#ifdef ESP32
#ifdef USE_ETHERNET
  if (!TasmotaGlobal.global_state.eth_down) {
    return EthernetMacAddress();
  }
#endif
#endif
  return WiFi.macAddress();
}
