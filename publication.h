#ifndef PUBLICATION_H
#define PUBLICATION_H

#include <string>
#include <vector>

struct Publication {
    std::string key;
    std::string title;
    std::vector<std::string> authors;
    std::string year;
    std::string venue;
};

#endif // PUBLICATION_H 