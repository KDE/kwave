#ifndef _PLAYBACK_CONTEXT_H_
#define _PLAYBACK_CONTEXT_H_

#include "libkwave/Sample.h"

namespace Kwave 
{
  
  class PlaybackContext
  {
  public:
    PlaybackContext(); 
    void setLastTracks(unsigned int Ltracks)  { m_last_tracks = Ltracks; }
    void setLastOffset(sample_index_t Loffset) { m_last_offset = Loffset; }
    void setLastVisible(sample_index_t Lvisible) { m_last_visible = Lvisible; }
    void setLastLength(sample_index_t Llength) { m_last_length = Llength; }
    unsigned int lastTracks() { return m_last_tracks; }
    sample_index_t lastOffset() { return m_last_offset; }
    sample_index_t lastVisible() { return m_last_visible; }
    sample_index_t lastLength() { return m_last_length; }
    
  private:
	/** last number of tracks */
	unsigned int m_last_tracks;

	/** last offset of the current view */
	sample_index_t m_last_offset;

	/** last number of the visible samples */
	sample_index_t m_last_visible;

	/** last length of the signal */
	sample_index_t m_last_length;
  };
}

#endif
    
