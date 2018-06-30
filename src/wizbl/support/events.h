#ifndef WIZBLCOIN_SUPPORT_EVENTS_H
#define WIZBLCOIN_SUPPORT_EVENTS_H

#include <memory>
#include <ios>

#include <event2/event.h>
#include <event2/http.h>

#define MAKE_RAI2(type) \
\
struct type##_deleter {\
    void operator()(struct type* ob) {\
        type##_free(ob);\
    }\
};\
\
typedef std::unique_ptr<struct type, type##_deleter> raii_##type

MAKE_RAI2(evhttp_connection);
MAKE_RAI2(evhttp_request);
MAKE_RAI2(evhttp);
MAKE_RAI2(event);
MAKE_RAI2(event_base);

inline rai2_evhttp_connection obtain_evhttp_connection_base(struct event_base* base, std::string host, uint16_t port) {
    auto result = rai2_evhttp_connection(evhttp_connection_base_new(base, nullptr, host.c_str(), port));
    if (!result.get())
        throw std::runtime_error("create connection failed");
    return result;
}

inline rai2_evhttp_request obtain_evhttp_request(void(*cb)(struct evhttp_request *, void *), void *arg) {
    return rai2_evhttp_request(evhttp_request_new(cb, arg));
}

inline rai2_event obtain_event(struct event_base* base, evutil_socket_t s, short events, event_callback_fn cb, void* arg) {
    return rai2_event(event_new(base, s, events, cb, arg));
}

inline rai2_event_base obtain_event_base() {
    auto result = rai2_event_base(event_base_new());
    if (!result.get())
        throw std::runtime_error("cannot create event_base");
    return result;
}

inline rai2_evhttp obtain_evhttp(struct event_base* base) {
    return rai2_evhttp(evhttp_new(base));
}

#endif
                                                                