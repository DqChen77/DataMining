#ifndef DBLP_READER_H
#define DBLP_READER_H

#include <string>
#include <vector>
#include "publication.h"

class DBLPReader {
public:
    DBLPReader(const std::string& filename);
    std::vector<Publication> readPublications();

private:
    std::string filename_;
    void processPublication(const Publication& pub);
    void saveRandomPublicationsToXML(const std::vector<Publication>& publications, int count, const std::string& outputFilename);
};

#endif // DBLP_READER_H 