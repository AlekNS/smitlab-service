
#ifndef ASTMFRAME_H
#define	ASTMFRAME_H


#include <string>
#include <istream>
#include <ostream>


namespace astm {

    
using std::string;
using std::istream;
using std::ostream;


class AstmFrame
{
    bool    isCrAtFrameEnd;
public:
    enum {
        MAX_FRAME_SIZE = 240
    };
    
    int     frameNumber;
    string  data;
    bool    isIntermediate;
    int     checksum;
    
    AstmFrame();
    
    int     calculateChecksum();
    
    void    read(istream &stream);
    void    write(ostream &stream);
    
    static bool read(int &frameNumber, string &frameData, istream &stream);
    static int write(const int &frameSeek, const int &frameNumber, const string &frameData, ostream &stream);
};


}

#endif
