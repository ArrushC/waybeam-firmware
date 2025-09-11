#include "TextMessageModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "buzz.h"
#include "configuration.h"
#include "graphics/Screen.h"
#include "power.h"
TextMessageModule *textMessageModule;

ProcessMessage TextMessageModule::handleReceived(const meshtastic_MeshPacket &mp)
{
#if defined(DEBUG_PORT) && !defined(DEBUG_MUTE)
    auto &p = mp.decoded;
    LOG_INFO("Received text msg from=0x%0x, id=0x%x, msg=%.*s", mp.from, mp.id, p.payload.size, p.payload.bytes);
#endif

    // Extract message text
    String messageText = String((char *)mp.decoded.payload.bytes, mp.decoded.payload.size);
    
    // Debug: Log all message processing
    LOG_INFO("TextMessage: Processing message '%s' from node 0x%0x", messageText.c_str(), mp.from);
    
    // Check if this is a PING message that needs automatic response
    if (messageText.startsWith("PING:")) {
        LOG_INFO("TextMessage: PING detected! Message: '%s'", messageText.c_str());
        // Parse ping format: PING:{ping_id}:{message}
        int firstColon = messageText.indexOf(':', 5); // Start after "PING:"
        LOG_INFO("TextMessage: First colon at position %d", firstColon);
        if (firstColon > 5) {
            String pingId = messageText.substring(5, firstColon);
            LOG_INFO("TextMessage: Extracted ping ID: '%s'", pingId.c_str());
            
            // Generate response with status information
            String response = "PING_ACK:" + pingId;
            LOG_INFO("TextMessage: Building response: '%s'", response.c_str());
            
            // Add battery status if available
            if (powerStatus && powerStatus->getHasBattery()) {
                response += ":" + String(powerStatus->getBatteryChargePercent()) + "%";
            } else {
                response += ":PWR"; // External power
            }
            
            // Add GPS coordinates if available  
            if (localPosition.latitude_i != 0 && localPosition.longitude_i != 0) {
                // Convert from int32 back to float (degrees * 1e7)
                float lat = localPosition.latitude_i / 1e7;
                float lon = localPosition.longitude_i / 1e7;
                response += ":" + String(lat, 6) + "," + String(lon, 6);
            } else {
                response += ":NO_GPS";
            }
            
            // Add node status
            response += ":AVAIL"; // Available for emergency response
            
            // Create and send reply packet
            auto reply = allocDataPacket();
            LOG_INFO("TextMessage: allocDataPacket returned %s", reply ? "valid packet" : "null");
            if (reply) {
                // Set the reply destination to the sender
                reply->to = mp.from;
                reply->decoded.want_response = false;
                
                // Copy response string to payload
                reply->decoded.payload.size = response.length();
                memcpy(reply->decoded.payload.bytes, response.c_str(), reply->decoded.payload.size);
                
                LOG_INFO("TextMessage: Reply packet prepared - to=0x%0x, size=%d", reply->to, reply->decoded.payload.size);
                
                // Send the reply (framework handles this automatically via myReply)
                myReply = reply;
                
                LOG_INFO("TextMessage: Auto-responding to ping %s from 0x%0x: %s", pingId.c_str(), mp.from, response.c_str());
            } else {
                LOG_ERROR("TextMessage: Failed to allocate reply packet for ping response");
            }
        } else {
            LOG_INFO("TextMessage: PING message format invalid - no second colon found");
        }
    }

    // Continue with normal message processing
    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = true;

    // Only trigger screen wake if configuration allows it
    if (shouldWakeOnReceivedMessage()) {
        powerFSM.trigger(EVENT_RECEIVED_MSG);
    }
    notifyObservers(&mp);

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

bool TextMessageModule::wantPacket(const meshtastic_MeshPacket *p)
{
    return MeshService::isTextPayload(p);
}