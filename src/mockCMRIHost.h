#ifndef MOCK_CMRI_HOST_H
#define MOCK_CMRI_HOST_H

/*
 * Since a cpCMRI instance reads and writes to a "serial Stream",
 * all we need to do is build something that looks like a Stream class,
 * fill it with the packets to "send", connect it and "run" the parser,
 * and we'll end up with a bunch of captured response packets.
 *
 * Note: This implementation has a limited buffer size for packets,
 * and little error cheking to see if those limits are exceeded...
 */

class mockCMRIHost : public Stream {
public:
    mockCMRIHost(void)
    : _output_buffer_len(0)
    , _read_position(0)
    , _write_position(0)
    {}

    /**
     * Take a string description of a packet and stuff the corresponding
     * real packet into the "to be read from" buffer
     * @param str
     */
    void preload_data(const char *str);

    // Stream methods
    virtual void begin(int baud, int config=SERIAL_8N1) { /* NOOP */ }
    virtual void end(void)                              { /* NOOP */ }
    virtual void flush()                                { /* NOOP */ };
    virtual int available() {
        int count = _output_buffer_len - _read_position;
        return count;
    }
    virtual int read() {
        return _read_position < _output_buffer_len ? _output_buffer[_read_position++] : -1;
    }
    virtual int peek() {
        return _read_position < _output_buffer_len ? _output_buffer[_read_position]   : -1;
    }
    virtual int availableForWrite() {
        return (sizeof(_input_buffer) / sizeof(byte) ) - _write_position;
    }
    size_t pushback(uint8_t c) {
        if (_output_buffer_len < sizeof(_output_buffer) / sizeof(byte)) {
            _output_buffer[_output_buffer_len++] = c;
            return 1;
        }
    }
    virtual size_t write(uint8_t c) {
        // Serial.print("stream write(0x");Serial.print((byte)c, HEX); Serial.print(", pos="); Serial.print(_write_position, DEC); Serial.print(") ");
        if (_write_position < sizeof(_input_buffer) / sizeof(byte)) {
            _input_buffer[_write_position++] = c;
            return 1;
        }
        return 0;
    }

    byte _output_buffer[256];   // reads come from here
    byte _input_buffer[256];    // writes go here
    int _output_buffer_len;     // how much can be read?
    int _read_position;         // how much have we read?
    int _write_position;        // how much has been written?
};


#endif

