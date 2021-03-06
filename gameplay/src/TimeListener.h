#ifndef TIMELISTENER_H_
#define TIMELISTENER_H_

namespace gameplay
{

/**
 * The TimeListener interface allows a class to be scheduled and called at a later time using Game::schedule().
 */
class TimeListener
{
public:

    /**
     * Callback method that is called when the scheduled event is fired.
     * 
     * @param timeDiff The time difference between the current game time and the target time.
     *                 The time differences will always be non-negative because scheduled events will not fire early.
     * @param cookie The cookie data that was passed when the event was scheduled.
     */
    virtual void timeEvent(long timeDiff, void* cookie) = 0;
};

}

#endif
