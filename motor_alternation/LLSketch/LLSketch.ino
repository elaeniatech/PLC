#include <AlPlc_Opta.h>

/* opta_1.3.0
*/

struct PLCSharedVarsInput_t
{
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	bool B1;
	bool B2;
	bool B3;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(-664763463, false);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname

// enable direct access to local and expansion I/O variables
// #include <IOPlugin.h>

void setup()
{


	AxelPLC.Run();
}

void loop()
{
	delay(1);
	

}
