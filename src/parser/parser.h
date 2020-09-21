#ifndef __PARSER_H__
#define __PARSER_H__

#include <Arduino.h>
#include "buffer/buffer.h"

class Parser
{
public:
    typedef void (*Event)(const CommandBuffer &);
    typedef struct 
    {
        const char *command;
        Event fun;
        // used fot the interval functionality
        unsigned long lastUpdate = 0;
        size_t interval;
    } EventData;

    // Returns how many characters have been read
    size_t read_stream(Stream &stream);

    bool add_event(const char *command, Event function, size_t interval = Parser::IDLE_INTERVAL);
    bool set_interval(const char *command, size_t interval);

    bool exec_buffer();
    void exec_intervals();

    CommandBuffer &get_buff();
    EventData *get_event(const char *command);

    bool is_full();

    static constexpr int TIMEOUT = 20;
    static constexpr int MAX_EVENTS = 20;
    static constexpr size_t IDLE_INTERVAL = 0U;

private:
    CommandBuffer m_buffer;
    // arrays of events and info about them
    size_t m_current_events = 0;
    EventData m_events[MAX_EVENTS];
};

#endif // __PARSER_H
