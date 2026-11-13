#include <AlPlc_Opta.h>

/* opta_1.3.0
      - ArduinoIoTCloud (2.10.0)
      - Arduino_ConnectionHandler (1.3.0)
      - Arduino_NetworkConfigurator (0.5.1)
      - Arduino_CloudUtils (1.2.1)
*/

struct PLCSharedVarsInput_t
{
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


AlPlc AxelPLC(803437610, false);

#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <Arduino_NetworkConfigurator.h>
#include "configuratorAgents/agents/BLEAgent.h"
#include "configuratorAgents/agents/SerialAgent.h"

int Counter_M1;
int Counter_M2;
int Counter_M3;

bool Motor_1;
bool Motor_2;
bool Motor_3;

KVStore kvStore;
BLEAgentClass BLEAgent;
SerialAgentClass SerialAgent;

WiFiConnectionHandler ArduinoIoTPreferredConnection;
NetworkConfiguratorClass NetworkConfigurator(
  ArduinoIoTPreferredConnection
);

void initProperties()
{
  NetworkConfigurator.addAgent(BLEAgent);
  NetworkConfigurator.addAgent(SerialAgent);
  NetworkConfigurator.setStorage(kvStore);

  ArduinoCloud.setConfigurator(NetworkConfigurator);

  ArduinoCloud.addProperty(Counter_M1, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(Counter_M2, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(Counter_M3, READ, ON_CHANGE, NULL);

  ArduinoCloud.addProperty(Motor_1, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(Motor_2, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(Motor_3, READ, ON_CHANGE, NULL);
}

void setup()
{
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
	WiFi.begin("RM_interior", "RM20JUNE");
	
	AxelPLC.Run();
}

void loop()
{
	delay(1);
	
  Motor_1 = PLCOut.B1_sh;
  Motor_2 = PLCOut.B2_sh;
  Motor_3 = PLCOut.B3_sh;

  Counter_M1 = PLCOut.C1_sh;
  Counter_M2 = PLCOut.C2_sh;
  Counter_M3 = PLCOut.C3_sh
}

  ArduinoCloud.update();
}
