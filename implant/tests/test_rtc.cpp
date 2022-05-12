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

using namespace std::chrono_literals;
using std::shared_ptr;
using std::weak_ptr;
using std::chrono::milliseconds;
using std::chrono::steady_clock;
template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

using nlohmann::json;

int main() {
    rtc::InitLogger(rtc::LogLevel::Info);

    rtc::Configuration config;
    config.iceServers.emplace_back("stun.l.google.com:19302");

    rtc::PeerConnection pc(config);

    pc.onLocalDescription([](rtc::Description sdp) {
        // Send the SDP to the remote peer
        printf("onLocalDescription callback got %s", std::string(sdp));
    });

    pc.onLocalCandidate([](rtc::Candidate candidate) {
        // Send the candidate to the remote peer
        printf("onLocalCandidate callback got %s, %s", candidate.candidate(), candidate.mid());
    });

    // MY_ON_RECV_DESCRIPTION_FROM_REMOTE([&pc](std::string sdp) {
    //     pc.setRemoteDescription(rtc::Description(sdp));
    // });

    // MY_ON_RECV_CANDIDATE_FROM_REMOTE([&pc](std::string candidate, std::string mid) {
    //     pc.addRemoteCandidate(rtc::Candidate(candidate, mid));
    // });

    pc.onStateChange([](rtc::PeerConnection::State state) {
        std::cout << "State: " << state << std::endl;
    });

    pc.onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        std::cout << "Gathering state: " << state << std::endl;
    });

    auto dc = pc.createDataChannel("test");

    dc->onOpen([]() {
        std::cout << "Open" << std::endl;
    });

    dc->onMessage([](std::variant<rtc::binary, rtc::string> message) {
        if (std::holds_alternative<rtc::string>(message)) {
            std::cout << "Received: " << std::get<rtc::string>(message) << std::endl;
            // std::cout << "Received: " << message << std::endl;
        }
    });

    while(true) {}

    return 0;
}