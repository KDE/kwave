
#ifndef _CLIP_BOARD_H_
#define _CLIP_BOARD_H_

class SignalManager;
class Signal;

class ClipBoard
{
public:
    ClipBoard();
    ClipBoard(SignalManager *signal);
    ~ClipBoard();

    void toWindow();
    void appendChannel(Signal *channel);

    SignalManager *getSignal();
    unsigned int getLength();

private:
    SignalManager *signal;
};

#endif /* _CLIP_BOARD_H_ */
