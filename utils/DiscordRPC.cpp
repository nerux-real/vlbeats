#include "DiscordRPC.h"
#include <iostream>

void initRPC() {
    auto& rpc = discord::RPCManager::get();

    rpc.setClientID("YOUR_APP_ID")
       .onReady([](discord::User const& user){
           std::cout << "RPC Ready: " << user.username << "\n";
       })
       .onErrored([](int code, std::string_view msg){
           std::cout << "RPC Error " << code << ": " << msg << "\n";
       })
       .initialize();
}

void updateRPC() {
    auto& rpc = discord::RPCManager::get();
    auto& presence = rpc.getPresence();

    presence.state = "Playing";
    presence.details = "In a match";
    presence.largeImageKey = "logo";

    rpc.refresh();
}

void shutdownRPC() {
    discord::RPCManager::get().shutdown();
}