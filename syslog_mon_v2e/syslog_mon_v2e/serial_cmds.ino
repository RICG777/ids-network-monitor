// Serial command dispatcher — handles all config/test/status commands received
// over USB serial. Extracted from loop() so the same apply* logic can later be
// shared with the HTTP API (E3 web UI).

void handleSerialCommand(const char* message) {
    if (strncmp(message, "IP:", 3) == 0) {
        ip = parseIP(message + 3);
        prefs.putString("ip", message + 3);
        if (gateway[0]==0 && gateway[1]==0 && gateway[2]==0 && gateway[3]==0)
            ETH.config(ip, IPAddress(0, 0, 0, 0), subnet);
        else
            ETH.config(ip, gateway, subnet);
        Udp.stop();
        Udp.begin(localPort);
        Serial.print(F("ArdACK:IP:"));
        Serial.println(ip);

    } else if (strncmp(message, "SUBNET:", 7) == 0) {
        subnet = parseIP(message + 7);
        prefs.putString("subnet", message + 7);
        Serial.print(F("ArdACK:SUBNET:"));
        Serial.println(subnet);

    } else if (strncmp(message, "GATEWAY:", 8) == 0) {
        gateway = parseIP(message + 8);
        prefs.putString("gateway", message + 8);
        Serial.print(F("ArdACK:GATEWAY:"));
        Serial.println(gateway);

    } else if (strncmp(message, "Relay1:", 7) == 0) {
        const char* trig = message + 7;
        int len = strlen(trig);
        if (len == 0 || len >= MAX_TRIG_STR) {
            Serial.println(F("ArdERR:Relay1:must be 1-119 chars"));
        } else {
            strncpy(r1Trigs, trig, MAX_TRIG_STR - 1);
            r1Trigs[MAX_TRIG_STR - 1] = '\0';
            prefs.putString("r1trigs", r1Trigs);
            Serial.print(F("ArdACK:Relay1:"));
            Serial.println(r1Trigs);
        }

    } else if (strncmp(message, "Relay2:", 7) == 0) {
        const char* trig = message + 7;
        int len = strlen(trig);
        if (len == 0 || len >= MAX_TRIG_STR) {
            Serial.println(F("ArdERR:Relay2:must be 1-119 chars"));
        } else {
            strncpy(r2Trigs, trig, MAX_TRIG_STR - 1);
            r2Trigs[MAX_TRIG_STR - 1] = '\0';
            prefs.putString("r2trigs", r2Trigs);
            Serial.print(F("ArdACK:Relay2:"));
            Serial.println(r2Trigs);
        }

    } else if (strncmp(message, "MODE1:", 6) == 0) {
        relay1Mode = (strncmp(message + 6, "LATCH", 5) == 0) ? MODE_LATCH : MODE_PULSE;
        prefs.putUChar("r1mode", relay1Mode);
        Serial.print(F("ArdACK:MODE1:"));
        Serial.println(relay1Mode == MODE_LATCH ? "LATCH" : "PULSE");

    } else if (strncmp(message, "MODE2:", 6) == 0) {
        relay2Mode = (strncmp(message + 6, "LATCH", 5) == 0) ? MODE_LATCH : MODE_PULSE;
        prefs.putUChar("r2mode", relay2Mode);
        Serial.print(F("ArdACK:MODE2:"));
        Serial.println(relay2Mode == MODE_LATCH ? "LATCH" : "PULSE");

    } else if (strncmp(message, "DURATION1:", 10) == 0) {
        int dur = atoi(message + 10);
        if (dur < 1 || dur > 255) { Serial.println(F("ArdERR:DURATION1:must be 1-255")); }
        else { relay1Duration = dur; prefs.putUChar("r1dur", relay1Duration);
            Serial.print(F("ArdACK:DURATION1:")); Serial.println(relay1Duration); }

    } else if (strncmp(message, "DURATION2:", 10) == 0) {
        int dur = atoi(message + 10);
        if (dur < 1 || dur > 255) { Serial.println(F("ArdERR:DURATION2:must be 1-255")); }
        else { relay2Duration = dur; prefs.putUChar("r2dur", relay2Duration);
            Serial.print(F("ArdACK:DURATION2:")); Serial.println(relay2Duration); }

    } else if (strncmp(message, "VERBOSITY:", 10) == 0) {
        int v = atoi(message + 10);
        if (v < 0 || v > 2) { Serial.println(F("ArdERR:VERBOSITY:must be 0-2")); }
        else { verbosity = v; prefs.putUChar("verb", verbosity);
            Serial.print(F("ArdACK:VERBOSITY:")); Serial.println(verbosity); }

    } else if (strncmp(message, "RESET1", 6) == 0) {
        relay1IsOn = false;
        updateRelayOutput(1);
        Serial.println(F("ArdACK:RESET1:OK"));

    } else if (strncmp(message, "RESET2", 6) == 0) {
        relay2IsOn = false;
        updateRelayOutput(2);
        Serial.println(F("ArdACK:RESET2:OK"));

    } else if (strncmp(message, "STATUS", 6) == 0) {
        sendStatus();

    } else if (strncmp(message, "PING:", 5) == 0) {
        handlePingCommand(message + 5);

    } else if (strncmp(message, "TTEST1", 6) == 0) {
        Serial.print(F("ArdACK:TTEST1:pulsing ")); Serial.print(relay1Duration); Serial.println(F("s"));
        digitalWrite(relayPin1, HIGH); delay((unsigned long)relay1Duration * 1000UL); digitalWrite(relayPin1, LOW);
        Serial.println(F("ArdMsg: Relay 1 timed test complete."));

    } else if (strncmp(message, "TTEST2", 6) == 0) {
        Serial.print(F("ArdACK:TTEST2:pulsing ")); Serial.print(relay2Duration); Serial.println(F("s"));
        digitalWrite(relayPin2, HIGH); delay((unsigned long)relay2Duration * 1000UL); digitalWrite(relayPin2, LOW);
        Serial.println(F("ArdMsg: Relay 2 timed test complete."));

    } else if (strncmp(message, "TEST1", 5) == 0) {
        Serial.println(F("ArdACK:TEST1:pulsing 3s"));
        digitalWrite(relayPin1, HIGH); delay(TEST_PULSE_MS); digitalWrite(relayPin1, LOW);
        Serial.println(F("ArdMsg: Relay 1 test complete."));

    } else if (strncmp(message, "TEST2", 5) == 0) {
        Serial.println(F("ArdACK:TEST2:pulsing 3s"));
        digitalWrite(relayPin2, HIGH); delay(TEST_PULSE_MS); digitalWrite(relayPin2, LOW);
        Serial.println(F("ArdMsg: Relay 2 test complete."));
    }
}
