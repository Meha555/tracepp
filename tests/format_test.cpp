#include "event.hpp"

#include <cassert>
#include <string>

using namespace trace;

int main()
{
    Event instant = Event::CreateInstant("Quote\"Name", 100, 1, 2, Event::Scope::Thread);
    instant.cat = "unit";
    instant.args.push_back({"message", "line\nvalue"});
    std::string instant_json = instant.toJSON();
    assert(instant_json.find("\"name\":\"Quote\\\"Name\"") != std::string::npos);
    assert(instant_json.find("\"ph\":\"i\"") != std::string::npos);
    assert(instant_json.find("\"cat\":\"unit\"") != std::string::npos);
    assert(instant_json.find("\"s\":\"t\"") != std::string::npos);
    assert(instant_json.find("\"args\":{") != std::string::npos);
    assert(instant_json.find("\"message\":\"line\\nvalue\"") != std::string::npos);

    Event complete = Event::CreateComplete("Complete", 200, 0, 1, 2);
    std::string complete_json = complete.toJSON();
    assert(complete_json.find("\"ph\":\"X\"") != std::string::npos);
    assert(complete_json.find("\"dur\":0") != std::string::npos);
    assert(complete_json.find("\"args\":{}") != std::string::npos);

    Event counter = Event::CreateCounter("Counter", 300, 1, 2, "value", 42.5);
    std::string counter_json = counter.toJSON();
    assert(counter_json.find("\"ph\":\"C\"") != std::string::npos);
    assert(counter_json.find("\"value\":42.5") != std::string::npos);

    Event json_arg = Event::Create("JsonArg", Event::Phase::Instant, 400, 1, 2);
    json_arg.args.push_back(Event::Arg::Json("data", "{\"nested\":true}"));
    std::string json_arg_json = json_arg.toJSON();
    assert(json_arg_json.find("\"data\":{\"nested\":true}") != std::string::npos);

    return 0;
}
