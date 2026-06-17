#pragma once
#include <string>
// #include <DiscordRPC/discord_rpc.h>
#include "../deps/discord-rpc.hpp"

class DiscordRPC {
public:
    void init(const char* appId);
    void update(const std::string& state, const std::string& details, const std::string& largeImageKey = "");
    void shutdown();
    void runCallbacks();
};