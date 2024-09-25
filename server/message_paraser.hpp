#pragma once

#include <boost/json.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <unordered_map>

#include "../snorlax_log.hpp"

enum MessageType {
    REGISTER,
    LOGIN,
    LOGOUT,
    TEXT_MESSAGE,
    IMAGE_MESSAGE,
    FILE_TRANSFER,
    HEARTBEAT,
    SYSTEM_MESSAGE
};

class MessageParse {
   public:
    MessageParse(const std::string& message)
        : messageType(LOGOUT), sender(""), receiver(""), content("") {
        try {
            boost::json::value jsonValue = boost::json::parse(message);
            auto jsonObject = jsonValue.as_object();
            std::string_view typeString = jsonObject.at("type").as_string();
            sender = jsonObject.at("sender").as_string();
            receiver = jsonObject.at("receiver").as_string();
            content = jsonObject.at("content").as_string();
            messageType = parseMessageType(typeString);
        } catch (const boost::json::system_error& e) {
            LOG(Level::ERROR,
                "JSON parsing error: ",
                e.what(),
                " for message: ",
                message);
        } catch (const std::exception& e) {
            LOG(Level::ERROR, "Error: ", e.what());
        }
    }

    MessageType getMessageType() const { return messageType; }
    const std::string& getSender() const { return sender; }
    const std::string& getReceiver() const { return receiver; }
    const std::string& getContent() const { return content; }

   private:
    MessageType messageType;
    std::string sender;
    std::string receiver;
    std::string content;

    MessageType parseMessageType(std::string_view type) {
        static const std::unordered_map<std::string_view, MessageType> typeMap =
            {{"REGISTER", REGISTER},
             {"LOGIN", LOGIN},
             {"LOGOUT", LOGOUT},
             {"TEXT_MESSAGE", TEXT_MESSAGE},
             {"IMAGE_MESSAGE", IMAGE_MESSAGE},
             {"FILE_TRANSFER", FILE_TRANSFER},
             {"HEARTBEAT", HEARTBEAT},
             {"SYSTEM_MESSAGE", SYSTEM_MESSAGE}};

        auto it = typeMap.find(type);
        if (it != typeMap.end()) {
            return it->second;
        } else {
            throw std::invalid_argument("Unknown message type: " +
                                        std::string(type));
        }
    }
};
