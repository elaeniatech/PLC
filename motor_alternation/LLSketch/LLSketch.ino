#include <AlPlc_Opta.h>

/* opta_1.3.0
      - ArduinoIoTCloud (2.10.0)
      - Arduino_ConnectionHandler (1.3.0)
      - Arduino_NetworkConfigurator (0.5.1)
      - Arduino_CloudUtils (1.2.1)
      - Arduino_KVStore (1.0.1)
      - ArduinoBLE (2.1.0)
      - Arduino_SecureElement (1.0.1)
      - ArduinoECCX08 (1.4.1)
      - ArduinoMqttClient (0.1.8)
      - ArduinoBearSSL (1.8.0)
      - ArduinoHttpClient (0.6.2)
*/

struct PLCSharedVarsInput_t
{
	bool start_sh;
	bool stop_sh;
	bool C1_res_sh;
	bool C2_res_sh;
	bool C3_res_sh;
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	bool B1_sh;
	bool B2_sh;
	bool B3_sh;
	uint32_t C1_sh;
	uint32_t C2_sh;
	uint32_t C3_sh;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(453742732, false);

#include <WiFi.h>
#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// Wi-Fi credentials
const char SSID[] = "RM_interior";
const char PASS[] = "RM20JUNE";

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

// Automatic pressure commands (read-only on the dashboard)
bool Start = false;
bool Stop = false;

// Reset the counters
bool C1_res = false;
bool C2_res = false;
bool C3_res = false;

// Cloud monitoring
int Counter_M1 = 0;
int Counter_M2 = 0;
int Counter_M3 = 0;

bool Motor_1 = false;
bool Motor_2 = false;
bool Motor_3 = false;

float Pressure_bar = 7.0f;
float Consumption_CFM = 0.0f;

const float VOLUME_LITERS = 500.0f;
const float COMPRESSOR_CFM = 30.0f;
const float LOW_PRESSURE_BAR = 6.0f;
const float HIGH_PRESSURE_BAR = 8.0f;

void updatePressureSimulation();

void initProperties()
{
  ArduinoCloud.addProperty(
    C1_res, READWRITE, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    C2_res, READWRITE, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    C3_res, READWRITE, ON_CHANGE, NULL
  );

  
  // Commands: pressure simulation -> PLC and dashboard
  ArduinoCloud.addProperty(
    Start, READ, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    Stop, READ, ON_CHANGE, NULL
  );

  // Counters: PLC -> dashboard
  ArduinoCloud.addProperty(
    Counter_M1, READ, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    Counter_M2, READ, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    Counter_M3, READ, ON_CHANGE, NULL
  );

  // Motor states: PLC -> dashboard
  ArduinoCloud.addProperty(
    Motor_1, READ, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    Motor_2, READ, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
    Motor_3, READ, ON_CHANGE, NULL
  );
  
  // Dashboard consumption setting and simulated tank pressure
  ArduinoCloud.addProperty(
      Consumption_CFM, READWRITE, ON_CHANGE, NULL
  );
  ArduinoCloud.addProperty(
      Pressure_bar, READ, ON_CHANGE, NULL
  );
}

void setup()
{
  // Disable Cloud debug output during PLC USB testing.
  setDebugMessageLevel(-1);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
	WiFi.begin("RM_interior", "RM20JUNE");
	
	AxelPLC.Run();
}

void loop()
{
	delay(1);
	
	
  // Read PLC monitoring values.
  Motor_1 = PLCOut.B1_sh;
  Motor_2 = PLCOut.B2_sh;
  Motor_3 = PLCOut.B3_sh;

  Counter_M1 = PLCOut.C1_sh;
  Counter_M2 = PLCOut.C2_sh;
  Counter_M3 = PLCOut.C3_sh;

  // Exchange values with Arduino Cloud.
  ArduinoCloud.update();
  updatePressureSimulation();

  // Pass Cloud commands to the PLC.
  PLCIn.start_sh = Start;
  PLCIn.stop_sh = Stop;
  
  // Pass Cloud commands to the PLC.
  PLCIn.C1_res_sh = C1_res;
  PLCIn.C2_res_sh = C2_res;
  PLCIn.C3_res_sh = C3_res;
}

void updatePressureSimulation()
{
  static unsigned long previousMs = 0;
  static bool initialized = false;
  static bool airRequested = false;

  const unsigned long now = millis();

  if (!initialized) {
    previousMs = now;
    initialized = true;
    return;
  }

  const unsigned long elapsedMs = now - previousMs;

  // Update once per second without blocking the PLC.
  if (elapsedMs < 1000UL) {
    return;
  }

  previousMs = now;
  const float dtSeconds = elapsedMs / 1000.0f;

  // Consumption range: zero to one compressor's capacity.
  if (Consumption_CFM < 0.0f) {
    Consumption_CFM = 0.0f;
  }

  if (Consumption_CFM > COMPRESSOR_CFM) {
    Consumption_CFM = COMPRESSOR_CFM;
  }

  // Assume an active motor means the compressor delivers air.
  const int runningCompressors =
      (PLCOut.B1_sh ? 1 : 0)
    + (PLCOut.B2_sh ? 1 : 0)
    + (PLCOut.B3_sh ? 1 : 0);

  const float supplyCFM =
      runningCompressors * COMPRESSOR_CFM;

  Pressure_bar +=
      (0.478f / VOLUME_LITERS)
    * (supplyCFM - Consumption_CFM)
    * dtSeconds;

  // Gauge pressure cannot fall below zero in this model.
  if (Pressure_bar < 0.0f) {
    Pressure_bar = 0.0f;
  }

  // Hold each command until the next one-second simulation update.
  Start = false;
  Stop = false;

  if (!airRequested && Pressure_bar <= LOW_PRESSURE_BAR) {
    Start = true;
    airRequested = true;
  }
  else if (airRequested && Pressure_bar >= HIGH_PRESSURE_BAR) {
    Stop = true;
    airRequested = false;
  }
}
