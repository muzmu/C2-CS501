#include <string>

#include "p2p.hpp"

#include "../libs/libdatachannel/include/rtc/rtc.hpp"

#include "../libs/nlohmann/json.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>

void sendThroughParent(rtc::DataChannel dc, std::string content) {
    dc.send(content);
}

