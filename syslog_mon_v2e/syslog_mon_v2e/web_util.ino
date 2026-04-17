// Web UI support utilities — event ring buffer and UEXT jumper gate.
// Used by serial STATUS from v2f.1; expanded for HTTP consumers in v2f.2+.

#define EVENT_MSG_LEN            96
#define EVENT_BUFFER_SIZE        64
#define JUMPER_PIN_SENSE         13   // UEXT pin 6
#define JUMPER_PIN_GND           16   // UEXT pin 5, driven LOW as ground ref
#define JUMPER_CHECK_INTERVAL_MS 500
#define JUMPER_DEBOUNCE_COUNT    3    // 3 × 500 ms = 1.5 s consistent to flip

struct Event {
    uint32_t seq;
    uint32_t millisAtBoot;
    uint8_t  level;
    char     msg[EVENT_MSG_LEN];
};

Event    events[EVENT_BUFFER_SIZE];
uint32_t eventSeq    = 0;
byte     eventHead   = 0;
byte     eventCount  = 0;
portMUX_TYPE eventMux = portMUX_INITIALIZER_UNLOCKED;

bool          jumperFitted      = false;
byte          jumperDebounce    = 0;
unsigned long jumperLastCheckMs = 0;

// Write one event to Serial (respecting verbosity) and ring buffer.
// DEBUG-level events skip the ring to avoid churn. Safe to call from any
// task — ring updates are protected by a portMUX spinlock.
void logEvent(uint8_t level, const char* fmt, ...) {
    char tmp[EVENT_MSG_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    if (verbosity >= level) {
        Serial.print(F("ArdMsg: "));
        Serial.println(tmp);
    }

    if (level >= VERB_DEBUG) return;

    portENTER_CRITICAL(&eventMux);
    eventSeq++;
    Event& e = events[eventHead];
    e.seq = eventSeq;
    e.millisAtBoot = millis();
    e.level = level;
    strncpy(e.msg, tmp, EVENT_MSG_LEN - 1);
    e.msg[EVENT_MSG_LEN - 1] = '\0';
    eventHead = (eventHead + 1) % EVENT_BUFFER_SIZE;
    if (eventCount < EVENT_BUFFER_SIZE) eventCount++;
    portEXIT_CRITICAL(&eventMux);
}

void setupJumper() {
    pinMode(JUMPER_PIN_GND, OUTPUT);
    digitalWrite(JUMPER_PIN_GND, LOW);
    pinMode(JUMPER_PIN_SENSE, INPUT_PULLUP);
    delay(10);  // let pullup settle
    jumperFitted      = (digitalRead(JUMPER_PIN_SENSE) == LOW);
    jumperLastCheckMs = millis();
    logEvent(VERB_NORMAL, "Jumper on boot: %s", jumperFitted ? "FITTED" : "OPEN");
}

void checkJumperPeriodic() {
    if (millis() - jumperLastCheckMs < JUMPER_CHECK_INTERVAL_MS) return;
    jumperLastCheckMs = millis();

    bool read = (digitalRead(JUMPER_PIN_SENSE) == LOW);
    if (read == jumperFitted) {
        jumperDebounce = 0;
        return;
    }
    jumperDebounce++;
    if (jumperDebounce >= JUMPER_DEBOUNCE_COUNT) {
        jumperFitted  = read;
        jumperDebounce = 0;
        logEvent(VERB_QUIET, "Jumper state change: %s (tamper)",
                 jumperFitted ? "FITTED" : "OPEN");
    }
}
