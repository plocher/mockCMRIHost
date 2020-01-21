/**
 * cpNode Protocol Tester
 * ======================
 * This sketch uses a mocking framework to test
 * the cpCMRI protocol framework.
 */

#include <String.h>
#include <cpCMRI.h>
#include <I2Cexpander.h>
#include <mockCMRIHost.h>

//==============================================
//====    NODE CONFIGURATION PARAMETERS     ====
//==============================================

#define CMRINET_NODE_ID      3
#define CMRINET_SPEED      9600  // make sure this matches your speed set in JMRI

bool myOutBit;
byte myOutByte;

elapsedMillis timer_check_cmri;
#define DELAY_CHECK_CMRI  200

elapsedMillis timer_blinker;
#define DELAY_BLINKER  60
#define PIN_BLINK 25
bool blinkstate = 0;

#define PIN_BITFOLLOW 23
#define PIN_BYTEFOLLOW1 18
#define PIN_BYTEFOLLOW2 19

cpIOMap node_configuration[] = {
    // device                 pin or                              '1'/'0' = initialized output ' ' = dontcare
    // type                    addr  I/O               initilize   '+'    = input pullup, ' ' = input HiZ
    { I2Cexpander::MCP23017,     0,   "ooooooooOOOOOOOO", "1111111100000000"},
    { I2Cexpander::PCF8574,      1,   "oooooooo",         "11111111"},
//  { I2Cexpander::BIT,   &myOutBit,  "O",                "1"},
//  { I2Cexpander::BIT,   &blinkstate,"I",                " "},
//  { I2Cexpander::BYTE,  &myOutByte, "OOOOoooo",         "10101010"},
//  { I2Cexpander::BUILTIN,     25,   "I",                "+"},
//  { I2Cexpander::BUILTIN,      4,   "I",                "+"},
//  { I2Cexpander::BUILTIN,      5,   "i",                "+"},
//  { I2Cexpander::BUILTIN,     32,   "i",                "+"},
//  { I2Cexpander::BUILTIN,     33,   "i",                "+"},
//  { I2Cexpander::BUILTIN,     18,   "i",                " "},
//  { I2Cexpander::BUILTIN,     19,   "i",                " "},
//  { I2Cexpander::BUILTIN,     23,   "i",                " "}, 
_END_OF_IOMAP_LIST_
};

CMRI_Node *node;
CMRI_Packet p;                        // for CMRInet send / receive...
int txdelay = 0;                      // delay before sending packets...
int input_bits = 0;
int output_bits = 0;
mockCMRISerial SerialMock;

void gatherInputs(CMRI_Packet &p) {
      // make the RX packet
      p.clear();
      cpIOMap::collectIOMapInputs(node_configuration, p.content());
      p.set_type(CMRI_Packet::RX);
      p.set_address(CMRINET_NODE_ID);
      p.set_length( (node->get_num_input_bits() + 7) / 8);  // in bytes, rounded up
      Serial.print("POLL:==>\nRX: <== "); Serial.println(CMRI_Node::packetToString(p));
}

void distributeOutputs(CMRI_Packet &p) {
      Serial.print("TX: ==> "); Serial.println(CMRI_Node::packetToString(p));
      cpIOMap::distributeIOMapOutputs(node_configuration, p.content());
}

void setup() {

    Wire.begin();
    Serial.begin(115200);
    Serial.println("CMRI Node - Mock testing");
    Serial.print("Node address: "); Serial.println(CMRINET_NODE_ID, DEC);

    SerialMock.begin(CMRINET_SPEED, SERIAL_8N2);
    SerialMock.preload_data("I1[66,0,0,0]");
    SerialMock.preload_data("I2[66,0,0,0]");
    SerialMock.preload_data("I3[66,0,0,0]");
    SerialMock.preload_data("P3[]");
    SerialMock.preload_data("T3[0xFF,0x00,0xCC,0xCC]");
    SerialMock.preload_data("T3[0x80,0xF0,0x33,0x33]");
    SerialMock.preload_data("T3[0x40,0x0F,0xCC,0xCC]");
    SerialMock.preload_data("P3[]");
    SerialMock.preload_data("T3[0x20,0xF0,0x33,0x33]");
    SerialMock.preload_data("T3[0x10,0x0F,0xCC,0xCC]");
    SerialMock.preload_data("T3[0x08,0xF0,0x33,0x33]");
    SerialMock.preload_data("P3[]");
    SerialMock.preload_data("T3[0x04,0x0F,0xCC,0xCC]");
    SerialMock.preload_data("T3[0x02,0xF0,0x33,0x33]");
    SerialMock.preload_data("T3[0x01,0x0F,0xCC,0xCC]");
    SerialMock.preload_data("P3[]");
    SerialMock.preload_data("T3[0x00,0xF0,0x33,0x33]");
    SerialMock.preload_data("T3[0x00,0xFF,0xCC,0xCC]");

    cpIOMap::setupIOMap(node_configuration);

    node = new CMRI_Node(CMRINET_NODE_ID, SerialMock);
    node->set_num_input_bits(cpIOMap::countIOMapInputs(node_configuration));  // how many Input bits?
    node->set_num_output_bits(cpIOMap::countIOMapOutputs(node_configuration)); // how many output bits?
    node->setInputHandler(gatherInputs);
    node->setOutputHandler(distributeOutputs);

    pinMode(PIN_BLINK, OUTPUT);
    pinMode(PIN_BITFOLLOW, OUTPUT);
    pinMode(PIN_BYTEFOLLOW1, OUTPUT);
    pinMode(PIN_BYTEFOLLOW2, OUTPUT);
}


void loop() {
    if (timer_check_cmri > DELAY_CHECK_CMRI) {  // artificial slowdown of protocol handling for debugging...
        node->protocol_handler();
        timer_check_cmri = 0;
    }

    digitalWrite(PIN_BITFOLLOW, myOutBit);
    digitalWrite(PIN_BYTEFOLLOW1, (myOutByte & 0x10) ? 1 : 0);
    digitalWrite(PIN_BYTEFOLLOW2, (myOutByte & 0x20) ? 1 : 0);

    if (timer_blinker > DELAY_BLINKER) {
        blinkstate = (blinkstate ? 0 : 1);
        digitalWrite(PIN_BLINK, blinkstate);
        timer_blinker = 0;
    }
}

