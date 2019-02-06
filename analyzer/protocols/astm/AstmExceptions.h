
#ifndef ASTMEXCEPTIONS_H
#define	ASTMEXCEPTIONS_H


#include <exception>


namespace astm {


class AstmException: public std::exception {
    const std::string _displayMessage;
public:
    AstmException(const std::string &message): exception(), _displayMessage(message) { }
    virtual ~AstmException() throw() { }
    
    const std::string displayMessage() const { return _displayMessage; }
};


//
// Field
//
struct AstmFieldFormatException: public AstmException {
    AstmFieldFormatException(const std::string &message = ""): AstmException(message) { }
};


struct AstmFieldFormatWrongIndexException: public AstmFieldFormatException {
    AstmFieldFormatWrongIndexException(): AstmFieldFormatException() { } 
    const char* what() const throw() { return "AstmFieldFormatWrongIndexException"; }
};

struct AstmFieldFormatWrongTypeException: public AstmFieldFormatException {
    AstmFieldFormatWrongTypeException(): AstmFieldFormatException() { } 
    const char* what() const throw() { return "AstmFieldFormatWrongTypeException"; }
};

struct AstmFieldFormatWrongTypeIntegerException: public AstmFieldFormatException {
    AstmFieldFormatWrongTypeIntegerException(): AstmFieldFormatException() { } 
    const char* what() const throw() { return "AstmFieldFormatWrongTypeIntegerException"; }
};

struct AstmFieldFormatWrongTypeFloatException: public AstmFieldFormatException {
    AstmFieldFormatWrongTypeFloatException(): AstmFieldFormatException() { } 
    const char* what() const throw() { return "AstmFieldFormatWrongTypeFloatException"; }
};

struct AstmFieldFormatWrongTypeDatetimeException: public AstmFieldFormatException {
    AstmFieldFormatWrongTypeDatetimeException(): AstmFieldFormatException() { } 
    const char* what() const throw() { return "AstmFieldFormatWrongTypeDatetimeException"; }
};


//
// Frame
//
struct AstmFrameException: public AstmException {
    AstmFrameException(const std::string &message = ""): AstmException(message) { }
};

struct AstmFrameExceptionReceive: public AstmFrameException {
    AstmFrameExceptionReceive(const std::string &message): AstmFrameException(message) { } 
    const char* what() const throw() { return "AstmFrameExceptionReceive"; }
};

struct AstmFrameExceptionSend: public AstmFrameException {
    AstmFrameExceptionSend(const std::string &message): AstmFrameException(message) { } 
    const char* what() const throw() { return "AstmFrameExceptionSend"; }
};



//
// State
//
struct AstmStateException: public AstmException {
    AstmStateException(const std::string &message = ""): AstmException(message) { }
};


struct AstmStateExceptionTermination: public AstmStateException {
    AstmStateExceptionTermination(): AstmStateException() { } 
    const char* what() const throw() { return "AstmStateExceptionTermination"; }
};


struct AstmStateExceptionFrame: public AstmStateException {
    AstmStateExceptionFrame(const std::string &message): AstmStateException(message) { } 
    const char* what() const throw() { return "AstmStateExceptionFrame"; }
};


struct AstmStateExceptionTimeout: public AstmStateException {
    AstmStateExceptionTimeout(const std::string &message): AstmStateException(message) { } 
    const char* what() const throw() { return "AstmStateExceptionTimeout"; }
};



//
// Protocol
//
struct ProtocolAstmException: public AstmException {
    ProtocolAstmException(const std::string &message = ""): AstmException(message) { }
};

struct ProtocolAstmExceptionFrameNumber: public ProtocolAstmException {
    ProtocolAstmExceptionFrameNumber(): ProtocolAstmException() { }
    const char* what() const throw() { return "ProtocolAstmExceptionFrameNumber"; }
};

struct ProtocolAstmExceptionResidue: public ProtocolAstmException {
    ProtocolAstmExceptionResidue(): ProtocolAstmException() { }
    const char* what() const throw() { return "ProtocolAstmExceptionResidue"; }
};

}
    
#endif
