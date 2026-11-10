#include <Arduino.h>
#line 1 "C:\\Repositories\\elaeniatech\\PLC\\test\\LLSketch\\LLSketch.ino"
#include <AlPlc_Opta.h>

/* opta_1.3.0
*/

struct PLCSharedVarsInput_t
{
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(424318575, false);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname

// enable direct access to local and expansion I/O variables
// #include <IOPlugin.h>

#line 24 "C:\\Repositories\\elaeniatech\\PLC\\test\\LLSketch\\LLSketch.ino"
void setup();
#line 31 "C:\\Repositories\\elaeniatech\\PLC\\test\\LLSketch\\LLSketch.ino"
void loop();
#line 24 "C:\\Repositories\\elaeniatech\\PLC\\test\\LLSketch\\LLSketch.ino"
void setup()
{


	AxelPLC.Run();
}

void loop()
{
	delay(1);
	

}

