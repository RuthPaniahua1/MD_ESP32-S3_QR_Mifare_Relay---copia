#ifndef TimeOut_h
#define TimeOut_h

#include "Arduino.h"

#include "microTuple.h"


constexpr uint32_t sc(uint16_t x) { return x * 1000; }

constexpr uint32_t mn(uint16_t x) { return x * 60000; }

constexpr uint32_t hr(uint16_t x) { return x * 3600000; }

constexpr uint32_t timeElementToMillis(uint8_t hour, uint8_t minute,
                                       uint8_t seconde) {
  return hr(hour) + mn(minute) + sc(seconde);
}

enum class TIMEOUT {
  NORMAL,
  UNDELETABLE,
  INTERVAL
}; // timer can be overwriten or cleared , timer cannot be cleared

class TimeOut;

class TimeOutNode {
protected:
  TimeOutNode();
  TimeOutNode(unsigned long delay, unsigned long timeStamp);
  //TimeOutNode(unsigned long delay, unsigned long timeStamp, TIMEOUT type); //maybe remove if no dived ctor ?? view line 169
  virtual ~TimeOutNode() = default;
  friend class TimeOut;
  friend class Interval;
  unsigned long delay;
  unsigned long timeStamp;
  TIMEOUT type;
  TimeOutNode *next;
  virtual void callback() {}
};

template <typename Callable> class TimeOutNodeDerived : public TimeOutNode {
  friend class TimeOut;
  friend class Interval;
  TimeOutNodeDerived(unsigned long delay, unsigned long &&timeStamp,
                     Callable cb)
      : TimeOutNode(delay, timeStamp), callable(cb) {}
  Callable callable;
  void callback() override { callable(); }
  friend class TimeOut;
};

template <typename... Args> class TimeOutNodeArgs : public TimeOutNode {
friend class TimeOut;
friend class Interval;
  TimeOutNodeArgs<Args...>(unsigned long delay, unsigned long &&timeStamp,
                           Args... arg, void (*cb)(Args...))
      : TimeOutNode(delay, timeStamp),
        pack(ParamsPack<Args...>{MicroTuple<Args...>(arg...), cb}){};
  friend class TimeOut;
  ParamsPack<Args...> pack;
  void callback() override {
    if (pack)
      pack.getPack();
  };
};

class TimeOut {
public:
  friend class Interval;
  TimeOut(){};
  template <typename Callable>
  TimeOut(unsigned long delay, Callable callable)
      : node(new TimeOutNodeDerived<Callable>(delay, millis(), callable)) {
    sort(node); // place the instance into container
  }

  template <typename Callable>
  TimeOut(uint8_t hour, uint8_t minute, uint8_t seconde, Callable callable)
      : node(new TimeOutNodeDerived<Callable>(
            timeElementToMillis(hour, minute, seconde), millis(), callable)) {
    sort(node); // place the instance into container
  }

  template <typename... Args>
  TimeOut(unsigned long delay, void (*_callback)(Args... args), Args... args) 
      : node( new TimeOutNodeArgs<Args...>(delay, millis(), args..., _callback) ){
    sort(node); // place the instance into container
  };

private:
  static TimeOutNode *head;

protected:
  TimeOutNode *node = nullptr;

  static void
  sort(TimeOutNode *current) { // sort timer by time remainin to be triggered
    if (!TimeOut::head) {      // if list is emty, set it to first
      TimeOut::head = current;
      return;
    }
    unsigned long now = millis();

    TimeOutNode *it = TimeOut::head;
    TimeOutNode *previous = nullptr;

    while (it) {
      long itRemain = it->timeStamp + it->delay - now;
      long currentRemain = current->timeStamp + current->delay - now;
      if (itRemain > currentRemain) { // spot with timer a smaler than this
        // insert instance
        if (previous)
          previous->next = current;
        else
          TimeOut::head = current;
        current->next = it;
        return;
      } else if (!it->next) { // if delay is the bigger, place it at the end
        it->next = current;
        return;
      }
      previous = it;
      it = it->next; // it++
    }
  };

public:
  template <typename Callable>
  void timeOut(unsigned long delay, Callable callable) {
    node = new TimeOutNodeDerived<Callable>(delay, millis(), callable);
    sort(node); // place the instance into container
  }

  template <typename Callable>
  void timeOut(unsigned long delay, Callable callable, TIMEOUT timerType) {
    node = new TimeOutNodeDerived<Callable>(delay, millis(), callable);
    if (TIMEOUT::UNDELETABLE == timerType)
      node->type = TIMEOUT::UNDELETABLE;
    else
      node->type = timerType;
    sort(node); // place the instance into container
  }

  template <typename Callable>
  void timeOut(uint8_t hour, uint8_t minute, uint8_t seconde, Callable callable,
               TIMEOUT timerType) {
    node = new TimeOutNodeDerived<Callable>(
        timeElementToMillis(hour, minute, seconde), millis(), callable);
        node->type = timerType; //maybe add new derived ctor ??? view line 57
    sort(node); // place the instance into container
  }

  template <typename... Args>
  void timeOut(unsigned long delay, void (*_callback)(Args... args),
               Args... args) {
    node = new TimeOutNodeArgs<Args...>(delay, millis(), args..., _callback);
    sort(node); // place the instance into container
  };

  void cancel();
  static void handler();
  static void printContainer(HardwareSerial &stream);
};

class Interval : public TimeOut {
public:
  template <typename Callable>
  void interval(unsigned long delay, Callable callable) {
    node = new TimeOutNodeDerived<Callable>(delay, millis(), callable);
    node->type = TIMEOUT::INTERVAL;
    sort(node); // place the instance into container
  }

  template <typename Callable>
  void interval(uint8_t hour, uint8_t minute, uint8_t seconde,
                Callable callable) {
    node = new TimeOutNodeDerived<Callable>(
        timeElementToMillis(hour, minute, seconde), millis(),
        callable);
    node->type = TIMEOUT::INTERVAL;
    sort(node); // place the instance into container
  }

  template <typename... Args> 
  void interval(unsigned long delay, void (*callback)(Args... args),
                Args... args) {
    node = new TimeOutNodeArgs<Args...>(delay, millis(), args..., callback);
    node->type = TIMEOUT::INTERVAL;
    sort(node); // place the instance into container
  };

  constexpr static void (*handler)(){TimeOut::handler};
};

#endif
