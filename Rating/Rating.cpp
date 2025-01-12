//
// Created by JanValiser on 12/01/2025.
//

#include "Rating.h"

Rating::Rating(): user(""), score(0) {}

Rating::Rating(const std::string &user, const unsigned int score): user(user), score(score) {}



nlohmann::json Rating::toJson() const {
    nlohmann::json j;
    j["user"] = user;
    j["score"] = score;
    return j;
}

Rating Rating::fromJson(const nlohmann::json &j) {
    return Rating(
        j.at("user").get<std::string>(),
        j.at("score").get<unsigned int>());
}

