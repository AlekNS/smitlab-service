
#include "AstmFrame.h"
#include "common/CharUtil.h"
#include "AstmExceptions.h"

using namespace std;


using namespace astm;
using namespace common;


//----------------------------------------------------------------------------//
AstmFrame::AstmFrame()
{
    data.reserve(256);
}


//----------------------------------------------------------------------------//
int AstmFrame::calculateChecksum()
{
    int checksum = '0' + (frameNumber & 7) + 
            (isIntermediate ? CharUtil::ETB : (isCrAtFrameEnd ? CharUtil::CR : 0) + CharUtil::ETX), 
        n = data.size();

    if(!n) return 0;
    
    while(--n >= 0) { 
        checksum += data[n]; 
    }
    
    return checksum & 0xFF;
}


//----------------------------------------------------------------------------//
void AstmFrame::read(istream &stream)
{
    isIntermediate = false;

    if(stream.eof() || stream.bad()) {
        return;
    }
    
    int sym = stream.get();
    
    if(sym == CharUtil::EOT || sym < 0) {
        return;
    }    
    
    if(sym != CharUtil::STX) {
        throw AstmFrameExceptionReceive("No [STX] symbol at start frame");
    }
    
    frameNumber = stream.get() - '0';
    if(frameNumber < 0 || frameNumber > 7) {
        throw AstmFrameExceptionReceive("Wrong frame number (not in range [0;7])");
    }
    
    int n = MAX_FRAME_SIZE + 1;

    isCrAtFrameEnd = false;
    // read frame data
    while(true) {
        
        sym = stream.get();
        
        if(sym < 0) {
            throw AstmFrameExceptionReceive("Receive stream error sym < 0");
        }

        n -= 1;
        
        switch(sym) {
            case CharUtil::CR: isCrAtFrameEnd = true; break;
            case CharUtil::ETB:
                isIntermediate = true;
                goto astmReadExitFromLoop;
                break;

            case CharUtil::ETX:
                goto astmReadExitFromLoop;
                break;
                
            default:
                data.append(1, (char)sym);
                break;
        }
        
        if(!n) {
            throw AstmFrameExceptionReceive("Frame is too large n > 240");
        }
    }

astmReadExitFromLoop:;

    if(!data.size()) {
        return;
    }
    
    // check checksum
    sym = stream.get();
    if(sym >= 'a' && sym <= 'f') { sym = sym - 'a' + 10; }
    else if(sym >= 'A' && sym <= 'F') { sym = sym - 'A' + 10; }
    else if(sym >= '0' && sym <= '9') { sym = sym - '0'; }
    else {
        throw AstmFrameExceptionReceive("Wrong high checksum symbol");
    }
    
    checksum = sym << 4;
    
    sym = stream.get();
    if(sym >= 'a' && sym <= 'f') { sym = sym - 'a' + 10; }
    else if(sym >= 'A' && sym <= 'F') { sym = sym - 'A' + 10; }
    else if(sym >= '0' && sym <= '9') { sym = sym - '0'; }
    else {
        throw AstmFrameExceptionReceive("Wrong low checksum symbol");
    }

    checksum |= sym;
    
    if(stream.get() != CharUtil::CR) {
        throw AstmFrameExceptionReceive("No [CR] symbol at end of frame");
    }
    
    if(stream.get() != CharUtil::LF) {
        throw AstmFrameExceptionReceive("No [LF] symbol at end of frame");
    }
    
    if(checksum != calculateChecksum()) {
        throw AstmFrameExceptionReceive("Wrong checksum, package is damage");
    }
}


//----------------------------------------------------------------------------//
void AstmFrame::write(ostream &stream)
{
    if(!data.size() || data.size() > MAX_FRAME_SIZE) {
        throw AstmFrameExceptionSend("Wrong frame data size (empty or > 240)");
    }
    
    stream.put(CharUtil::STX);

    stream.put('0' + (frameNumber & 7));

    stream << data;

    if(isIntermediate) {
        stream.put(CharUtil::ETB);
    }
    else {
        isCrAtFrameEnd = true;
        stream.put(CharUtil::CR);
        stream.put(CharUtil::ETX);
    }

    checksum = calculateChecksum();

    stream.put(((checksum >> 4) & 0xF) > 9 ? 'A' + ((checksum >> 4) & 0xF) - 10 : '0' + ((checksum >> 4) & 0xF));
    stream.put((checksum & 0xF) > 9 ? 'A' + (checksum & 0xF) - 10 : '0' + (checksum & 0xF));

    stream.put(CharUtil::CR);
    stream.put(CharUtil::LF);
    
    stream.flush();
}


//----------------------------------------------------------------------------//
bool AstmFrame::read(int &frameNumber, string &frameData, istream &stream)
{
    AstmFrame frame;
    frame.read(stream);

    frameNumber = frame.frameNumber;
    
    frameData.append(frame.data);
    
    return frame.isIntermediate;
}


//----------------------------------------------------------------------------//
int AstmFrame::write(const int &frameSeek, const int &frameNumber, const string &frameData, ostream &stream)
{
    int size = min((int)frameData.size() - frameSeek, (int)MAX_FRAME_SIZE);
    if(size < 1) {
        return frameData.size();
    }
    
    int newFrameSeek = frameSeek + size;
    
    AstmFrame frame;
    frame.isIntermediate = frameData.size() - newFrameSeek > 0;
    frame.frameNumber = frameNumber;
    
    frame.data.assign(frameData.substr(frameSeek, size));
    
    frame.write(stream);
    
    return newFrameSeek;
}
