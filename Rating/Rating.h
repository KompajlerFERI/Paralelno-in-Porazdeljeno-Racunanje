#ifndef RATING_H
#define RATING_H

#include <string>
#include <nlohmann/json.hpp>


class Rating {
public:
    std::string user;
    unsigned int score;

    Rating();
    Rating(const std::string &user, const unsigned int score);

    nlohmann::json toJson() const;

    static Rating fromJson(const nlohmann::json &j);
};



#endif //RATING_H
